import time
import base64
import cv2

import roslibpy
from roslibpy import Message
from roslibpy import Topic

client = roslibpy.Ros(host='127.0.0.1', port=9090)

publisher = roslibpy.Topic(client, '/camera/image/compressed', 'sensor_msgs/CompressedImage')
publisher.advertise()

# open webcam
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print('Error: Could not open webcam.')
    exit()

def publish_frame():
    while(True):
        ret, frame = cap.read()
        if not ret:
            print('Error: Could not read frame.')
            return
        
        print('Publishing frame...')
        cv2.imwrite('test_input.jpg', frame)

        encoded_frame = cv2.imencode('.jpg', frame)[1].tobytes()
        encoded = base64.b64encode(encoded_frame).decode('ascii')

        publisher.publish(dict(format='jpeg', data=encoded))
        time.sleep(0.1)

    # publisher.publish(dict(format='jpeg', data=encoded))

client.on_ready(publish_frame)
client.run_forever()
    