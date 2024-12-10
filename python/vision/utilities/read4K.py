import cv2
import time

# Open the camera (device index might need adjustment)
camera_index = 1  # Default camera. Change if needed (e.g., 1 for another camera)
cap = cv2.VideoCapture(camera_index, cv2.CAP_DSHOW)  # CAP_DSHOW for Windows, optional for others

# Set resolution to 4K (3840x2160)
width = 3840
height = 2160
# width = 1920
# height = 1080
cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)

if not cap.isOpened():
    print("Error: Could not open the camera.")
    exit()

print(f"Camera resolution set to {width}x{height}. Press 'q' to exit.")
print(f"Actual resolution: {cap.get(cv2.CAP_PROP_FRAME_WIDTH)}x{cap.get(cv2.CAP_PROP_FRAME_HEIGHT)}")

# Get screen size to scale down for display
screen_width = 1920  # Example screen width
screen_height = 1080  # Example screen height

# OR create a full-size window
# cv2.namedWindow("4K Camera Output", cv2.WINDOW_NORMAL)
# cv2.resizeWindow("4K Camera Output", width, height)

image_counter = 0
last_capture_time = time.time()

while True:
    # Capture a frame
    ret, frame = cap.read()
    if not ret:
        print("Error: Failed to grab a frame.")
        break

    # Resize frame for display
    display_frame = cv2.resize(frame, (screen_width, screen_height))

    # Display the resized frame
    cv2.imshow("4K Camera Output (Resized)", display_frame)

    # Automatic capture every second
    # current_time = time.time()
    # if current_time - last_capture_time >= 1.0:  # Check if 1 second has passed
    #     filename = f"capture_{image_counter}.png"
    #     cv2.imwrite(filename, frame)
    #     print(f"Image captured and saved as '{filename}'.")
    #     image_counter += 1
    #     last_capture_time = current_time

    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'):
        break
    elif key == ord('s'):
        filename = f"capture_{image_counter}.png"
        cv2.imwrite(filename, frame)
        print(f"Image captured and saved as '{filename}'.")
        image_counter += 1

# Release the camera and close the window
cap.release()
cv2.destroyAllWindows()
