import compas_rrc as rrc
from compas.geometry import Scale, Frame
import math
from compas_rhino.geometry import RhinoMesh
from compas_fab.robots import AttachedCollisionMesh, CollisionMesh
from compas_ghpython import draw_frame
from compas_rhino.conversions import plane_to_compas_frame
import Rhino.Geometry as rg
import uuid
from production_data import Action

def generate_default_tolerances(joints):
    DEFAULT_TOLERANCE_METERS = .001
    DEFAULT_TOLERANCE_RADIANS = math.radians(1)

    return [
        DEFAULT_TOLERANCE_METERS if j.is_scalable()
        else DEFAULT_TOLERANCE_RADIANS
        for j in joints
    ]


class TFTProcess(object):
    def __init__(self, robot, TRANSFORM_ROBOT_A_TO_ROBOT_B = rg.Transform(), TRANSFORM_ROBOT_B_TO_ROBOT_A = rg.Transform(), speed = 100):
        self.robot = robot
        self.speed = speed
        self.actions = []
        self.TRANSFORM_ROBOT_A_TO_ROBOT_B = TRANSFORM_ROBOT_A_TO_ROBOT_B
        self.TRANSFORM_ROBOT_B_TO_ROBOT_A = TRANSFORM_ROBOT_B_TO_ROBOT_A
        self.S = Scale.from_factors([1000] * 3)

    def setup(self):

        self.actions.append(Action('SetTool', dict(tool_name='tool0')))
        self.actions.append(Action('SetWorkObject', dict(wobj_name='wobj0')))

    def pick(self, APPROACH_PICK_CONFIG, pick_plane, approach_distance):

        configurations = []

        #send to APPROACH_PICK_CONFIG
        joints = [math.degrees(j) for j in APPROACH_PICK_CONFIG.joint_values]
        self.actions.append(Action('MoveToJoints', dict(joints = joints, ext_axes = [], speed=self.speed, zone= rrc.Zone.FINE, feedback_level=rrc.FeedbackLevel.DONE), action_id = len(self.actions)))
        configurations.append(APPROACH_PICK_CONFIG)

        #open gripper
        self.actions.append(Action('SetDigital', dict(io_name = 'Local_IO_0_DO1', value = 1),action_id = len(self.actions)))

        #send to approach_pick_plane
        approach_pick_plane = pick_plane + rg.Vector3d(0, 0, approach_distance)
        approach_pick_plane.Transform(self.TRANSFORM_ROBOT_A_TO_ROBOT_B)
        frame = plane_to_compas_frame(approach_pick_plane)
        frame_rob = self.robot.from_tcf_to_t0cf([frame])[0]
        scaled_frame = frame_rob.transformed(self.S)
        self.actions.append(Action('MoveToFrame', dict(frame= scaled_frame, speed=self.speed, zone=rrc.Zone.FINE, motion_type='rrc.Motion.LINEAR',feedback_level=rrc.FeedbackLevel.DONE), action_id = len(self.actions)))

        #send to pick_plane

        pick_plane.Transform(self.TRANSFORM_ROBOT_A_TO_ROBOT_B)
        frame = plane_to_compas_frame(pick_plane)
        frame_rob = self.robot.from_tcf_to_t0cf([frame])[0]
        scaled_frame = frame_rob.transformed(self.S)
        self.actions.append(Action('MoveToFrame', dict(frame= scaled_frame, speed=self.speed, zone=rrc.Zone.FINE, motion_type='rrc.Motion.LINEAR',feedback_level=rrc.FeedbackLevel.DONE), action_id = len(self.actions)))

        #wait
        self.actions.append(Action('WaitTime', dict(time = 1,  feedback_level=rrc.FeedbackLevel.DONE), action_id = len(self.actions)))

        #close gripper
        self.actions.append(Action('SetDigital', dict(io_name = 'Local_IO_0_DO1', value = 0),action_id = len(self.actions)))

        #text
        self.actions.append(Action('PrintText', dict(text='Screw the beam to the gripper!'), action_id = len(self.actions)))
        self.actions.append(Action('Stop', dict(), action_id = len(self.actions)))

        #send to approach_pick_plane
        frame = plane_to_compas_frame(approach_pick_plane)
        frame_rob = self.robot.from_tcf_to_t0cf([frame])[0]
        scaled_frame = frame_rob.transformed(self.S)
        self.actions.append(Action('MoveToFrame', dict(frame= scaled_frame, speed=self.speed, zone=rrc.Zone.FINE, motion_type='rrc.Motion.LINEAR',feedback_level=rrc.FeedbackLevel.DONE), action_id = len(self.actions)))

        #text
        self.actions.append(Action('PrintText', dict(text='Robot goes to cut'), action_id = len(self.actions)))
        self.actions.append(Action('Stop', dict(), action_id = len(self.actions)))

        #send to APPROACH_PICK_CONFIG
        joints = [math.degrees(j) for j in APPROACH_PICK_CONFIG.joint_values]
        self.actions.append(Action('MoveToJoints', dict(joints = joints, ext_axes = [], speed=self.speed, zone= 10, feedback_level=rrc.FeedbackLevel.DONE), action_id = len(self.actions)))

        return configurations

    def cut_side_A(self):

        pass

    def cut_side_B(self):

        pass

    def place(self):

        pass
    
    def motion_start_config_target_config(self, start_configuration, target_configuration, attached_mesh, group):

        """
        Generates a motion plan from a start configuration to a target configuration 
        while considering attached objects.

        Parameters:
        - start_configuration: The initial configuration of the robot's joints (12 values)
        - target_configuration: The desired final configuration of the robot's joints. (6 values)
        - attached_mesh: A mesh representing any object attached to the robot, affecting motion planning.
        - group: The name of the joint group to be used for motion planning.

        Returns:
        - configurations: A list of the planned configurations
        """

        tolerance_above = generate_default_tolerances(self.robot.get_configurable_joints(group))
        tolerance_below = generate_default_tolerances(self.robot.get_configurable_joints(group))

        #get goal constraints for this target config 
        goal_constraints = self.robot.constraints_from_configuration(
                        configuration=target_configuration,
                        tolerances_above=tolerance_above,
                        tolerances_below=tolerance_below,
                        group=group
                    )

        #get attached collision mesh
        identifier = str(uuid.uuid4())
        compas_mesh = RhinoMesh.from_geometry(attached_mesh).to_compas()
        collision_mesh = CollisionMesh(compas_mesh, identifier)
        link_name = group + "_tool0"
        touch_links = [group + "_tool0", group + "_link_6"]
        attached_collision_mesh = AttachedCollisionMesh(collision_mesh, link_name, touch_links)
        attached_collision_meshes = [attached_collision_mesh]

        #plan motion
        planner_id = 'RRTConnect'
        attached_collision_meshes = list(attached_collision_meshes) if attached_collision_meshes else None

        #check if start_configuration has 12 values. If not add 0 configuration for other planning group
        if len(start_configuration.joint_values) == 6:
            config_A = self.robot.zero_configuration(group = "robotA")
            config_B = self.robot.zero_configuration(group = "robotB")
            if group == "robotB":
                config_A.joint_values = start_configuration.joint_values
            else:
                config_A.joint_values = start_configuration.joint_values
            start_configuration = config_A.merged(config_B)

        trajectory = self.robot.plan_motion(goal_constraints,
                                                start_configuration=start_configuration,
                                                group=group,
                                                options=dict(
                                                    attached_collision_meshes=attached_collision_meshes,
                                                    path_constraints=None,
                                                    planner_id=planner_id,
                                                ))

        merged_configurations = []
        planes = []
        for c in trajectory.points:
            merged_configurations.append(self.robot.merge_group_with_full_configuration(c, trajectory.start_configuration, group))
            frame = self.robot.forward_kinematics(c, group, options=dict(solver='model'))
            planes.append(draw_frame(frame))

        configurations = []

        for conf in merged_configurations:
            config = self.robot.zero_configuration(group)

            # Extract joint values in the correct order
            joint_values = []
            for i in range(1, 7):  # Ensure order is maintained
                joint_name = "{}_joint_{}".format(group, i)
                for n, j in zip(conf.joint_names, conf.joint_values):
                    if n == joint_name:
                        joint_values.append(j)
                        break  # Stop searching once we find the correct value

            config.joint_values = joint_values
            configurations.append(config)

            for config in configurations:
                joints = [math.degrees(j) for j in config.joint_values]
                self.actions.append(Action('MoveToJoints', dict(joints = joints, ext_axes = [], speed=self.speed, zone= 5, feedback_level=rrc.FeedbackLevel.DONE), action_id = len(self.actions)))


        return configurations
