import cv2 as cv
import numpy as np

# View untransformed image and select corner points

# FISHEYE CORRECTION
# DIM=(1920, 1080)
DIM=(3840, 2160)
# K=np.array([[715.0845213313972, 0.0, 649.466032885757], [0.0, 714.7508174031162, 349.07092982998165], [0.0, 0.0, 1.0]])
# D=np.array([[-0.018535944017385605], [-0.036164616733474465], [0.0996012486725898], [-0.08137886264463776]])
K=np.array([[2113.8978121575183, 0.0, 1940.3860532394276], [0.0, 2111.3353714795057, 1078.7327869431792], [0.0, 0.0, 1.0]])
D=np.array([[-0.03787467518808117], [0.0846315868116808], [-0.1264491080629783], [0.056945996640635654]])

# store map results
map1, map2 = cv.fisheye.initUndistortRectifyMap(K, D, np.eye(3), K, DIM, cv.CV_16SC2)

def undistort(img, map1, map2):    
    undistorted_img = cv.remap(img, map1, map2, interpolation=cv.INTER_LINEAR, borderMode=cv.BORDER_CONSTANT)    
    

    
    return undistorted_img

# img = cv.imread('')
video = cv.VideoCapture(1, cv2.CAP_DSHOW)

width = 3840
height = 2160

cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)

input_points = []

map1, map2 = cv2.fisheye.initUndistortRectifyMap(K, D, np.eye(3), K, DIM, cv2.CV_16SC2)


convertedPoints= np.float32([[0, 0], [width, 0], [0, height], [width, height]])

matrix = None
# img_output = cv.warpPerspective(img, matrix, (width, height))

def onMouse(event, x, y, flags, param):
    global input_points, convertedPoints, matrix
    if event == cv.EVENT_LBUTTONDOWN:
       if len(input_points) < 5:
           input_points.append((x, y))
           print('x = %d, y = %d'%(x, y))
           if len(input_points) == 4:
               input_points = np.array(input_points, np.float32)
               print(input_points)
            #    matrix = cv.getPerspectiveTransform(input_points, convertedPoints)
               
while True:
    success, frame = video.read()

    # frame = cv.rotate(frame, cv.ROTATE_180)
    frame = undistort(frame, map1, map2)

    if len(input_points) == 4:
        matrix = cv.getPerspectiveTransform(input_points, convertedPoints)

        transformedFrame = cv.warpPerspective(frame, matrix, (width, height))
        cv.imshow('transformed', transformedFrame)

    for point in input_points:
        # print(type(point))
        cv.circle(frame, (int(point[0]), int(point[1])), 3, (255, 0, 0), -1)

    cv.imshow('original', frame)
    
    cv.setMouseCallback('original', onMouse)                                               

    key = cv.waitKey(1) & 0xFF

    if key == ord("q"):
        break

# cv.imshow('original', img)
# cv.imshow('warped', img_output)

# cv.waitKey(0)

cv.destroyAllWindows()
video.release()