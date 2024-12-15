import numpy as np
import sys
import cv2

DIM=(1920, 1080)
K=np.array([[1072.2416669079864, 0.0, 974.1246550909606], [0.0, 1071.6376728723455, 524.0909070617798], [0.0, 0.0, 1.0]])
D=np.array([[-0.01680471181040181], [-0.04507194951348153], [0.10890126839017801], [-0.0804898674047556]])
# K=np.array([[2113.8978121575183, 0.0, 1940.3860532394276], [0.0, 2111.3353714795057, 1078.7327869431792], [0.0, 0.0, 1.0]])
# D=np.array([[-0.03787467518808117], [0.0846315868116808], [-0.1264491080629783], [0.056945996640635654]])

# Store map results
map1, map2 = cv2.fisheye.initUndistortRectifyMap(K, D, np.eye(3), K, DIM, cv2.CV_16SC2)

def undistort():
    input_video = sys.argv[1] if len(sys.argv) > 1 else "input.mp4"
    cap = cv2.VideoCapture(input_video)

    # Set resolution to 4K (3840x2160)
    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    fps = int(cap.get(cv2.CAP_PROP_FPS))
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))

    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter('output.mp4', fourcc, fps, (width, height))

    frame_count = 0

    frame_delay = 1  # milliseconds

    while True:
        ret, img = cap.read()
        if not ret:
            print("Failed to grab frame")
            break
            
        # Create undistortion maps (moved outside loop for better performance)
        # map1, map2 = cv2.fisheye.initUndistortRectifyMap(K, D, np.eye(3), K, DIM, cv2.CV_16SC2)
        undistorted_img = cv2.remap(img, map1, map2, interpolation=cv2.INTER_LINEAR, borderMode=cv2.BORDER_CONSTANT)    

        # display_img = cv2.resize(undistorted_img, (width, height))
        cv2.imshow("undistorted", undistorted_img)
        out.write(undistorted_img)

        frame_count += 1
        if frame_count % 30 == 0:  # Update every 30 frames
            print(f"Processing: {frame_count}/{total_frames} frames ({(frame_count/total_frames)*100:.1f}%)")
        
        # Press 'q' to quit
        if cv2.waitKey(frame_delay) & 0xFF == ord('q'):
            break

    # Clean up
    cap.release()
    out.release()
    cv2.destroyAllWindows()
    
if __name__ == '__main__':
        undistort()