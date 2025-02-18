import cv2
import numpy as np

def order_points(pts):
    """Order points in clockwise order starting from top-left."""
    rect = np.zeros((4, 2), dtype="float32")
    
    # Top-left point will have the smallest sum
    # Bottom-right point will have the largest sum
    s = pts.sum(axis=1)
    rect[0] = pts[np.argmin(s)]
    rect[2] = pts[np.argmax(s)]
    
    # Top-right point will have the smallest difference
    # Bottom-left point will have the largest difference
    diff = np.diff(pts, axis=1)
    rect[1] = pts[np.argmin(diff)]
    rect[3] = pts[np.argmax(diff)]
    
    return rect

class AreaCalibrator:
    def __init__(self, video_path):
        self.video_path = video_path
        self.points = []
        self.frame = None
        
    def click_event(self, event, x, y, flags, param):
        if event == cv2.EVENT_LBUTTONDOWN:
            if len(self.points) < 4:
                self.points.append([x, y])
                # Draw the point
                cv2.circle(self.frame, (x, y), 5, (0, 255, 0), -1)
                # Draw lines between points
                if len(self.points) > 1:
                    cv2.line(self.frame, tuple(self.points[-2]), (x, y), (0, 255, 0), 2)
                if len(self.points) == 4:
                    cv2.line(self.frame, tuple(self.points[0]), tuple(self.points[-1]), (0, 255, 0), 2)
                cv2.imshow('Frame', self.frame)

    def calibrate(self):
        cap = cv2.VideoCapture(self.video_path)
        
        # Set resolution to 1080p
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)
        
        # Read first frame
        ret, self.frame = cap.read()
        if not ret:
            print("Failed to read video")
            return None
        
        # Create window and set mouse callback
        cv2.namedWindow('Frame')
        cv2.setMouseCallback('Frame', self.click_event)
        
        print("Click 4 corner points in clockwise order starting from top-left")
        print("Press 'r' to reset points")
        print("Press 'q' to quit")
        print("Press 's' to save and continue when done")
        
        while True:
            cv2.imshow('Frame', self.frame)
            key = cv2.waitKey(1) & 0xFF
            
            if key == ord('r'):  # Reset points
                self.points = []
                self.frame = cap.read()[1].copy()
            
            elif key == ord('s') and len(self.points) == 4:  # Save points
                break
                
            elif key == ord('q'):  # Quit
                cap.release()
                cv2.destroyAllWindows()
                return None
        
        cap.release()
        cv2.destroyAllWindows()
        
        # Convert points to numpy array and order them
        pts = np.array(self.points, dtype="float32")
        rect = order_points(pts)
        
        return rect

def main():
    # Replace with your video path
    video_path = "overhead4KBlackBG.mp4"
    
    calibrator = AreaCalibrator(video_path)
    points = calibrator.calibrate()
    
    if points is not None:
        print("Calibration points:", points)
        # Save points to a file if needed
        np.save('calibration_points.npy', points)

if __name__ == "__main__":
    main()