import cv2
import numpy as np
from typing import List, Tuple

def crop_frame(frame: np.ndarray, coords: List[Tuple[int, int]]) -> np.ndarray:
    """
    Crop frame using a list of coordinates and return a perspective-corrected minimum frame
    """
    # Convert coordinates to numpy array
    pts = np.array(coords, dtype=np.float32)
    
    # Calculate the width and height of the new image
    # Width: maximum distance between left and right points
    width = max(
        np.linalg.norm(pts[1] - pts[0]),  # top width
        np.linalg.norm(pts[2] - pts[3])   # bottom width
    )
    
    # Height: maximum distance between top and bottom points
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
    
    # Calculate perspective transform matrix
    matrix = cv2.getPerspectiveTransform(pts, dst_pts)
    
    # Apply perspective transform
    result = cv2.warpPerspective(frame, matrix, (int(width), int(height)))
    
    return result

def segment_wood(frame: np.ndarray) -> List[np.ndarray]:
    """
    Segment wooden pieces from white background
    Only keeps approximately rectangular (4-sided) contours
    """
    # Convert to grayscale and threshold
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    blur = cv2.GaussianBlur(gray, (5, 5), 0)
    _, thresh = cv2.threshold(blur, 127, 255, cv2.THRESH_BINARY_INV)
    
    # Find contours
    contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    wood_contours = []
    for cnt in contours:
        # Filter by area first
        if cv2.contourArea(cnt) < 1000:  # Adjust min_area based on your needs
            continue
            
        # Approximate the contour to reduce number of points
        epsilon = 0.02 * cv2.arcLength(cnt, True)  # 2% of perimeter
        approx = cv2.approxPolyDP(cnt, epsilon, True)
        
        # Only keep contours with 4 vertices (rectangular)
        if len(approx) == 4:
            wood_contours.append(cnt)
    
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

def process_video(video_path: str, crop_coords: List[Tuple[int, int]]):
    """
    Main function to process video
    """
    cap = cv2.VideoCapture(video_path)
    
    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            break
            
        # Crop frame
        cropped = crop_frame(frame, crop_coords)
        
        # Segment wood
        wood_contours = segment_wood(cropped)
        
        # Get centerlines
        centerlines = get_centerlines(wood_contours)
        
        # Draw results
        result = cropped.copy()
        
        # Draw contours
        cv2.drawContours(result, wood_contours, -1, (0, 255, 0), 2)
        
        # Draw centerlines
        for centerline, angle in centerlines:
            cv2.line(result, tuple(centerline[0]), tuple(centerline[1]), (0, 0, 255), 2)
        
        # Show result
        cv2.imshow('Processed Frame', result)
        
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    
    cap.release()
    cv2.destroyAllWindows()

# Example usage
if __name__ == "__main__":
    # Define your crop coordinates (example)
    crop_coords = [
        (1185, 778),
        (2385, 740),
        (2402, 1344),
        (1201, 1379)
    ]
    
    video_path = "overheadAnnotatedTags.mp4"
    process_video(video_path, crop_coords)
