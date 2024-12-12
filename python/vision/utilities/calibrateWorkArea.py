import cv2 as cv
import numpy as np

# View untransformed image and select corner points

# FISHEYE CORRECTION
DIM=(3840, 2160)

K=np.array([[2113.8978121575183, 0.0, 1940.3860532394276], [0.0, 2111.3353714795057, 1078.7327869431792], [0.0, 0.0, 1.0]])
D=np.array([[-0.03787467518808117], [0.0846315868116808], [-0.1264491080629783], [0.056945996640635654]])

# Store map results
map1, map2 = cv.fisheye.initUndistortRectifyMap(K, D, np.eye(3), K, DIM, cv.CV_16SC2)

def undistort(img, map1, map2):    
    undistorted_img = cv.remap(img, map1, map2, interpolation=cv.INTER_LINEAR, borderMode=cv.BORDER_CONSTANT)    
    
    return undistorted_img

# img = cv.imread('')
cap = cv.VideoCapture(0, cv.CAP_DSHOW)

width = 3840
height = 2160

ASPECT_RATIO = 16/10

cap.set(cv.CAP_PROP_FRAME_WIDTH, width)
cap.set(cv.CAP_PROP_FRAME_HEIGHT, height)

input_points = []

map1, map2 = cv.fisheye.initUndistortRectifyMap(K, D, np.eye(3), K, DIM, cv.CV_16SC2)

matrix = None
# img_output = cv.warpPerspective(img, matrix, (width, height))

def onMouse(event, x, y, flags, param):
    global input_points, convertedPoints, matrix
    if event == cv.EVENT_LBUTTONDOWN:
       if len(input_points) < 5:
           # Scale up the coordinates from 1080p to 4K
           scaled_x = int(x * (width/1920))
           scaled_y = int(y * (height/1080))
           input_points.append((scaled_x, scaled_y))
           print(f'1080p coords: x={x}, y={y}')
           print(f'4K coords: x={scaled_x}, y={scaled_y}')
           if len(input_points) == 4:
               input_points = np.array(input_points, np.float32)
               print(input_points)

def calculate_output_dimensions(input_points):
    pts = np.array(input_points, dtype=np.float32)
    
    # Calculate width and height of the selected quadrilateral
    width = max(
        np.linalg.norm(pts[1] - pts[0]),  # top edge
        np.linalg.norm(pts[3] - pts[2])   # bottom edge
    )
    height = max(
        np.linalg.norm(pts[2] - pts[0]),  # left edge
        np.linalg.norm(pts[3] - pts[1])   # right edge
    )
    
    # Calculate dimensions based on the smallest side
    if width/ASPECT_RATIO < height:
        # Width is the limiting factor
        outputWidth = int(width)
        outputHeight = int(width/ASPECT_RATIO)
    else:
        # Height is the limiting factor
        outputHeight = int(height)
        outputWidth = int(height * ASPECT_RATIO)
    print(f"Output dimensions: {outputWidth}x{outputHeight} (16:10)")
    return outputWidth, outputHeight
               
while True:
    success, frame = cap.read()

    # frame = cv.rotate(frame, cv.ROTATE_180)
    frame = undistort(frame, map1, map2)
    # Scale down to 1920x1080 for display
    scaledFrame = cv.resize(frame, (1920, 1080))

    if len(input_points) == 4:
        outputWidth, outputHeight = calculate_output_dimensions(input_points)

        # Update converted points to match output dimensions
        convertedPoints = np.float32([[0, 0], [outputWidth, 0], [0, outputHeight], [outputWidth, outputHeight]])
        
        matrix = cv.getPerspectiveTransform(input_points, convertedPoints)

        transformedFrame = cv.warpPerspective(frame, matrix, (outputWidth, outputHeight))

        # Scale down transformed frame for display
        # Calculate scaling factor to fit within 1920x1080 display
        scale = min(1920/outputWidth, 1080/outputHeight)
        displayWidth = int(outputWidth * scale)
        displayHeight = int(outputHeight * scale)
        
        scaledTransformedFrame = cv.resize(transformedFrame, (displayWidth, displayHeight))
        cv.imshow('transformed', scaledTransformedFrame)

    for point in input_points:
        # Scale down the points for display on 1080p
        displayX = int(point[0] * (1920/width))
        displayY = int(point[1] * (1080/height))
        cv.circle(scaledFrame, (displayX, displayY), 3, (255, 0, 0), -1)


    cv.imshow('original', scaledFrame)
    
    cv.setMouseCallback('original', onMouse)                                               

    key = cv.waitKey(1) & 0xFF

    if key == ord("q"):
        break

# cv.imshow('original', img)
# cv.imshow('warped', img_output)

# cv.waitKey(0)

cv.destroyAllWindows()
cap.release()