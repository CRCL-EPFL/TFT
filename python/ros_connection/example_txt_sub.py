import time

from roslibpy import Topic
from compas_fab.backends import RosClient


def receive_message(message):
    print('Heard talking: ' + message['data'])


with RosClient() as client:
    listener = Topic(client, '/chatter', 'std_msgs/String')

    listener.subscribe(receive_message)

    cnt = 0
    while client.is_connected and cnt < 5:
        time.sleep(1)
        cnt += 1