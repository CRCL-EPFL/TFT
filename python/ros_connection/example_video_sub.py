import base64
import logging
import time
import cv2
import numpy as np
import roslibpy

# Configure logging
fmt = '%(asctime)s %(levelname)8s: %(message)s'
logging.basicConfig(format=fmt, level=logging.INFO)
log = logging.getLogger(__name__)

client = roslibpy.Ros(host='127.0.0.1', port=9090)

def receive_image(msg):
    log.info('Received image seq=%d', msg['header']['seq'])
    base64_bytes = msg['data'].encode('ascii')
    image_bytes = base64.b64decode(base64_bytes)

    image_bytes_np = np.frombuffer(image_bytes, dtype=np.uint8)
    cv_img = cv2.imdecode(image_bytes_np, cv2.IMREAD_COLOR)
    cv2.imwrite('received-image.jpg', cv_img)


subscriber = roslibpy.Topic(client, '/camera/image/compressed', 'sensor_msgs/CompressedImage')
subscriber.subscribe(receive_image)

client.run_forever()