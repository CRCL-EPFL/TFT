import cv2 as cv
from robotpy_apriltag import AprilTagDetector, AprilTagPoseEstimator
import sys
import numpy as np

input_video = sys.argv[1] if len(sys.argv) > 1 else "input.mp4"
cap = cv.VideoCapture(input_video)

width = int(cap.get(cv.CAP_PROP_FRAME_WIDTH))
height = int(cap.get(cv.CAP_PROP_FRAME_HEIGHT))
fps = int(cap.get(cv.CAP_PROP_FPS))
total_frames = int(cap.get(cv.CAP_PROP_FRAME_COUNT))

fourcc = cv.VideoWriter_fourcc(*'mp4v')
out = cv.VideoWriter('annotatedOverhead2.mp4', fourcc, 30.0, (width, height))

frame_count = 0
frame_delay = 1  # milliseconds

detector = AprilTagDetector()
# detector.setConfig()
config = detector.getConfig()

config.quadDecimate = 1.0  # Decimation factor (1.0 = full resolution, 2.0 = half resolution)
config.quadSigma = 0.0     # Gaussian blur sigma (0.0 = no blur)
config.refineEdges = True  # Refine edges for better accuracy
config.decodeSharpening = 0.5

detector.setConfig(config)
# estimator = AprilTagPoseEstimator()
detector.addFamily('tagStandard41h12')

roi_x, roi_y = 400, 300  # Top-left corner
roi_width, roi_height = 400, 300  # ROI dimensions


while True:
    ret, img = cap.read()
    if not ret:
        print("Failed to grab frame")
        break

    # Create ROI
    roi = img[roi_y:roi_y+roi_height, roi_x:roi_x+roi_width]
    
    # grayFrame = cv.cvtColor(img, cv.COLOR_BGR2GRAY)

    # Convert ROI to grayscale
    gray_roi = cv.cvtColor(roi, cv.COLOR_BGR2GRAY)
    
    # Apply threshold to get binary image
    _, binary = cv.threshold(gray_roi, 127, 255, cv.THRESH_BINARY)
    
    # Find contours in the ROI
    contours, _ = cv.findContours(binary, cv.RETR_EXTERNAL, cv.CHAIN_APPROX_SIMPLE)
    
    # Draw ROI boundary
    cv.rectangle(img, (roi_x, roi_y), (roi_x+roi_width, roi_y+roi_height), (0, 255, 0), 2)
    
    # Process each contour
    for contour in contours:
        # Approximate the contour to a polygon
        epsilon = 0.02 * cv.arcLength(contour, True)
        approx = cv.approxPolyDP(contour, epsilon, True)
        
        # If the polygon has 4 vertices, it's likely a rectangle
        if len(approx) == 4:
            # Adjust contour coordinates to global image space
            adjusted_contour = approx + [roi_x, roi_y]
            # Draw the rectangle
            cv.drawContours(img, [adjusted_contour], 0, (0, 0, 255), 2)

    # cv.imshow("grayscale", grayFrame)

    # tags = detector.detect(grayFrame)

    # print(f"Number of tags detected: {len(tags)}")

    # for i, tag in enumerate(tags):
    #     corners = (0, 0, 0, 0, 0, 0, 0, 0)

    #     colors = [
    #         (0, 0, 255),    # Red
    #         (0, 255, 0),    # Green
    #         (255, 0, 0),    # Blue
    #         (0, 255, 255)   # Yellow
    #     ]

    #     print(f"Tag ID: {tag.getId()}")
    #     print(f"Tag center: {tag.getCenter()}")
    #     print(f"Tag corners: {tag.getCorners(corners)}")

    #     tagId = tag.getId()
    #     tagCenter = tag.getCenter()
    #     corners = tag.getCorners(corners)

    #     # Draw lines connecting the corners
    #     cornersShaped = np.array(corners).reshape(-1, 2).astype(np.int32)

    #     # Draw lines connecting the corners
    #     for j in range(4):
    #         pt1 = tuple(cornersShaped[j])
    #         pt2 = tuple(cornersShaped[(j + 1) % 4])
    #         cv.line(img, pt1, pt2, colors[j], 2)

    #     # Draw the tag ID on the frame
    #     cv.putText(img, "SELECT", (int(tag.getCenter().x+10), int(tag.getCenter().y+10)), cv.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)

    # display_img = cv.resize(undistorted_img, (width, height))
    cv.imshow("annotated", img)
    out.write(img)

    frame_count += 1
    if frame_count % 30 == 0:  # Update every 30 frames
        print(f"Processing: {frame_count}/{total_frames} frames ({(frame_count/total_frames)*100:.1f}%)")
    
    # Press 'q' to quit
    if cv.waitKey(frame_delay) & 0xFF == ord('q'):
        break

# Clean up
cap.release()
out.release()
cv.destroyAllWindows()