import cv2 as cv
from robotpy_apriltag import AprilTagDetector, AprilTagPoseEstimator
import numpy as np
import json
from pythonosc import udp_client

# FISHEYE CORRECTION
DIM=(3840, 2160)

K=np.array([[2113.8978121575183, 0.0, 1940.3860532394276], [0.0, 2111.3353714795057, 1078.7327869431792], [0.0, 0.0, 1.0]])
D=np.array([[-0.03787467518808117], [0.0846315868116808], [-0.1264491080629783], [0.056945996640635654]])

# Store map results
map1, map2 = cv.fisheye.initUndistortRectifyMap(K, D, np.eye(3), K, DIM, cv.CV_16SC2)

fourcc = cv.VideoWriter_fourcc(*'mp4v')
out = cv.VideoWriter('output5.mp4', fourcc, 30.0, (1510, 943))

def undistort(img, map1, map2):    
    undistorted_img = cv.remap(img, map1, map2, interpolation=cv.INTER_LINEAR, borderMode=cv.BORDER_CONSTANT)    
    
    return undistorted_img

cap = cv.VideoCapture(0, cv.CAP_DSHOW)

width = 3840
height = 2160

ASPECT_RATIO = 16/10

cap.set(cv.CAP_PROP_FRAME_WIDTH, width)
cap.set(cv.CAP_PROP_FRAME_HEIGHT, height)

input_points = np.float32([[1108, 658], [2618, 682], [1110, 1610], [2594, 1608]])

map1, map2 = cv.fisheye.initUndistortRectifyMap(K, D, np.eye(3), K, DIM, cv.CV_16SC2)

matrix = None

detector = AprilTagDetector()
# estimator = AprilTagPoseEstimator()
detector.addFamily('tagStandard41h12')

osc_client = udp_client.SimpleUDPClient("127.0.0.1", 7777)

while True:
    success, frame = cap.read()

    frame = undistort(frame, map1, map2)
    # Scale down to 1920x1080 for display
    scaledFrame = cv.resize(frame, (1510, 943))

    outputWidth, outputHeight = 1510, 943

    # Update converted points to match output dimensions
    convertedPoints = np.float32([[0, 0], [outputWidth, 0], [0, outputHeight], [outputWidth, outputHeight]])
    
    matrix = cv.getPerspectiveTransform(input_points, convertedPoints)

    transformedFrame = cv.warpPerspective(frame, matrix, (outputWidth, outputHeight))

    grayFrame = cv.cvtColor(transformedFrame, cv.COLOR_BGR2GRAY)

    tags = detector.detect(grayFrame)
    # print(f"Type of tags: {type(tags)}")

    allCorners = []

    for i, tag in enumerate(tags):
        corners = (0, 0, 0, 0, 0, 0, 0, 0)

        colors = [
            (0, 0, 255),    # Red
            (0, 255, 0),    # Green
            (255, 0, 0),    # Blue
            (0, 255, 255)   # Yellow
        ]

        # print(f"Tag ID: {tag.getId()}")
        # print(f"Tag center: {tag.getCenter()}")
        # print(f"Tag corners: {tag.getCorners(corners)}")

        tagId = tag.getId()
        tagCenter = tag.getCenter()
        corners = tag.getCorners(corners)

        # Scale factors for 1920x1200 output
        scale_x = 1920 / 1510
        scale_y = 1200 / 943

        # Draw lines connecting the corners
        cornersShaped = np.array(corners).reshape(-1, 2).astype(np.int32)

        scaled_corners = cornersShaped * np.array([scale_x, scale_y])
        scaled_center = np.array([float(tagCenter.x) * scale_x, float(tagCenter.y) * scale_y])

        allCorners.append({
            'id': int(tagId),
            'corners': scaled_corners.tolist(),
            'center': scaled_center.tolist()
        })

        if allCorners:
            print("Sending OSC message:", json.dumps(allCorners))
            osc_client.send_message("/tags", json.dumps(allCorners))
            # osc_client.send_message("/tags", "TESTING")

        # Draw lines connecting the corners
        for j in range(4):
            pt1 = tuple(cornersShaped[j])
            pt2 = tuple(cornersShaped[(j + 1) % 4])
            # cv.line(transformedFrame, pt1, pt2, colors[j], 2)

        # Draw the tag ID on the frame
        # cv.putText(transformedFrame, str(tag.getId()), (int(tag.getCenter().x+10), int(tag.getCenter().y+10)), cv.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)

    cv.imshow('transformed', transformedFrame)

    out.write(transformedFrame)

    # cv.imshow('original', scaledFrame)                                         

    key = cv.waitKey(1) & 0xFF

    if key == 27:  # ESC key
        break

cv.destroyAllWindows()
out.release()
cap.release()