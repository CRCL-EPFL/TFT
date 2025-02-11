import time

from roslibpy import Message
from roslibpy import Topic

from compas_fab.backends import RosClient

with RosClient() as client:
    talker = Topic(client, '/chatter', 'std_msgs/String')

    cnt = 0
    while client.is_connected:
        message_data = 'Hello World! %d' % cnt

        talker.publish(Message({'data': message_data}))
        print('Sending message %s' % message_data)
        time.sleep(1)
        cnt += 1

    talker.unadvertise()