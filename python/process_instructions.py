import itertools

import compas_rrc as rrc
from compas.geometry import Frame, Vector

from production_data import Action


class TFTProcess(object):
    def __init__(self, namespace='/rob1'):
        self.namespace = namespace

    def setup(self):
        actions = []

        actions.append(Action('SetTool', dict(tool_name='tool0')))
        actions.append(Action('SetWorkObject', dict(wobj_name='wobj0')))

        return actions

    def pick(self):

        actions = []
        return actions

    def cut_side_A(self):

        actions = []
        return actions

    def cut_side_B(self):

        actions = []
        return actions

    def place(self):

        actions = []
        return actions