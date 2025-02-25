import cv2
import numpy as np
from typing import List, Tuple

def crop_frame(frame: np.ndarray, coords: List[Tuple[int, int]]) -> np.ndarray:

    # Convert coordinates to numpy array
    pts = np.array(coords, dtype=np.float32)

    width = max(
        np.linalg.norm(pts[1] - pts[0]),  # top width
        np.linalg.norm(pts[2] - pts[3])   # bottom width
    )

    height = max(
        np.linalg.norm(pts[3] - pts[0]),  # left height
        np.linalg.norm(pts[2] - pts[1])   # right height
    )
    
    # Create target points for perspective transform
    dst_pts = np.array([
        [0, 0],
        [width-1, 0],
        [width-1, height-1],
        [0, height-1]
    ], dtype=np.float32)
    
    matrix = cv2.getPerspectiveTransform(pts, dst_pts)
    
    result = cv2.warpPerspective(frame, matrix, (int(width), int(height)))
    
    return result

def segment_wood(frame: np.ndarray) -> List[np.ndarray]:
    """
    Segment wooden pieces from black background
    Only keeps exactly rectangular (4-sided) contours
    """
    # Convert to grayscale and threshold
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    blur = cv2.GaussianBlur(gray, (5, 5), 0)
    
    # Thresholding (normal, not inverted)
    _, thresh = cv2.threshold(blur, 100, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    
    # Clean up the mask
    kernel = np.ones((5,5), np.uint8)
    thresh = cv2.morphologyEx(thresh, cv2.MORPH_OPEN, kernel)
    
    # FIND CONTOURS
    contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    wood_contours = []
    debug_image = frame.copy()  # debug image
    
    for cnt in contours:
        # Filter by area
        area = cv2.contourArea(cnt)
        if area < 5000 or area > frame.shape[0] * frame.shape[1] * 0.5:
            continue
        
        epsilon = 0.01 * cv2.arcLength(cnt, True)
        approx = cv2.approxPolyDP(cnt, epsilon, True)
        
        M = cv2.moments(cnt)
        if M['m00'] != 0:
            cx = int(M['m10']/M['m00'])
            cy = int(M['m01']/M['m00'])
            cv2.putText(debug_image, f'Vertices: {len(approx)}', 
                       (cx, cy), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 0, 0), 2)
        
        # Exact 4 vertices
        if len(approx) == 4:
            # Get angles between sides
            angles = []
            for i in range(4):
                pt1 = approx[i][0]
                pt2 = approx[(i+1)%4][0]
                pt3 = approx[(i+2)%4][0]
                
                # 2D vectors to 3D by adding z=0
                v1_3d = np.append(pt2 - pt1, 0)
                v2_3d = np.append(pt3 - pt2, 0)
                
                angle = abs(np.degrees(np.arctan2(np.linalg.norm(np.cross(v1_3d, v2_3d)), np.dot(v1_3d, v2_3d))))
                angles.append(angle)
            
            # 90 degrees (within tolerance)
            if all(abs(angle - 90) < 20 for angle in angles):
                wood_contours.append(cnt)
    
    # Show debug image
    cv2.imshow('Debug - Contour Analysis', debug_image)
    cv2.imshow('Threshold', thresh)
    
    return wood_contours

def get_centerlines(contours: List[np.ndarray]) -> List[Tuple[np.ndarray, float]]:
    """
    Get centerline for each wood piece
    Returns list of (centerline_points, angle)
    """
    centerlines = []
    
    for contour in contours:
        # Fit rectangle
        rect = cv2.minAreaRect(contour)
        box = cv2.boxPoints(rect)
        box = np.int8(box)
        
        # Get center, width, height, and angle
        center = rect[0]
        width = rect[1][0]
        height = rect[1][1]
        angle = rect[2]
        
        # Ensure width is the longer side
        if width < height:
            width, height = height, width
            angle += 90
        
        # Calculate centerline points
        angle_rad = np.deg2rad(angle)
        dx = (width/2) * np.cos(angle_rad)
        dy = (width/2) * np.sin(angle_rad)
        
        start_point = (int(center[0] - dx), int(center[1] - dy))
        end_point = (int(center[0] + dx), int(center[1] + dy))
        
        centerlines.append((np.array([start_point, end_point]), angle))
    
    return centerlines

def process_video(camera: int, crop_coords: List[Tuple[int, int]]):
    """
    Main function to process video
    """
    cap = cv2.VideoCapture(camera)

    # Get video properties for output video
    frame_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    frame_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    fps = int(cap.get(cv2.CAP_PROP_FPS))
    
    # Get first frame to determine cropped dimensions
    ret, first_frame = cap.read()
    if not ret:
        print("Error: Could not read video file")
        return
    
    cropped_first = crop_frame(first_frame, crop_coords)
    # output_width = cropped_first.shape[1]
    # output_height = cropped_first.shape[0]
    
    # cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
    
    # output_path = video_path.rsplit('.', 1)[0] + '_processed.mp4'
    # fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    # out = cv2.VideoWriter(output_path, fourcc, fps, (output_width, output_height))
    
    pts = np.array(crop_coords, dtype=np.float32)
    width_pixels = max(
        np.linalg.norm(pts[1] - pts[0]),  # top width
        np.linalg.norm(pts[2] - pts[3])   # bottom width
    )
    PIXELS_TO_CM = 160.0 / width_pixels  # 160 cm is the known width
    
    while True:
        ret, frame = cap.read()
        if not ret:
            break
            
        cropped = crop_frame(frame, crop_coords)
        
        wood_contours = segment_wood(cropped)
        
        centerlines = get_centerlines(wood_contours)
        
        result = cropped.copy()
        
        # Draw contours and dimensions
        for cnt in wood_contours:
            cv2.drawContours(result, [cnt], -1, (0, 255, 0), 2)
            
            rect = cv2.minAreaRect(cnt)
            box = cv2.boxPoints(rect)
            box = np.int8(box)
            
            width_cm = rect[1][0] * PIXELS_TO_CM
            height_cm = rect[1][1] * PIXELS_TO_CM
            
            if width_cm < height_cm:
                width_cm, height_cm = height_cm, width_cm
            
            x, y, w, h = cv2.boundingRect(cnt)
            text_x = x + w//2  # center of bounding box
            text_y = y + h//2  # center of bounding box
            
            dimensions_text = f"{width_cm:.1f} x {height_cm:.1f} cm"
            
            font = cv2.FONT_HERSHEY_SIMPLEX
            font_scale = 0.7
            thickness = 2
            (text_width, text_height), _ = cv2.getTextSize(dimensions_text, font, font_scale, thickness)
            
            # Adjust position to ensure text is within frame
            text_x = min(max(text_x, text_width//2), result.shape[1] - text_width//2)
            text_y = min(max(text_y, text_height), result.shape[0] - 10)
            
            # Display dimensions with background for better visibility
            cv2.putText(result, dimensions_text, 
                       (int(text_x - text_width//2), int(text_y)),
                       font, font_scale, (0, 0, 255), thickness)
        
        # Draw centerlines
        for centerline, angle in centerlines:
            cv2.line(result, tuple(centerline[0]), tuple(centerline[1]), (0, 0, 255), 2)
        
        out.write(result)

        # Show result
        cv2.imshow('Processed Frame', result)
        
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    
    cap.release()
    # out.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    crop_coords = [
        (1185, 778),
        (2385, 740),
        (2402, 1344),
        (1201, 1379)
    ]
    
    video_path = "overhead4KBlackBG.mp4"
    process_video(video_path, crop_coords)
