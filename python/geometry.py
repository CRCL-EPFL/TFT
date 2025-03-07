import Rhino.Geometry as rg
import math
import Rhino
import json
import os


class Truss:
    def __init__(self):

        self.nodes = []
        self.beams = []

    def add_node(self, position):
        """Adds a node"""

        new_node = Node(len(self.nodes), position)
        self.nodes.append(new_node)
        return new_node

    def add_beam(self, axis, height, width, isnew=False, fabricated = False):
        """Creates a beam between two existing nodes and adds it to the truss."""

        tolerance = 1e-6

        # find which nodes are connected to this beam
        for i, node in enumerate(self.nodes):
            # Check if either endpoint of the beam's axis is close to the node's position
            if axis.From.DistanceTo(node.position) < tolerance:
                start_node_index = i
            if axis.To.DistanceTo(node.position) < tolerance:
                end_node_index = i

        new_beam = Beam(len(self.beams), start_node_index, end_node_index, axis, height, width, isnew, fabricated)

        self.beams.append(new_beam)

        self.nodes[start_node_index].add_beam(new_beam)
        self.nodes[end_node_index].add_beam(new_beam)

    def cut_all_beams(self):

        for beam in self.beams:
            if not beam.fabricated:
                beam.cut_polyline = beam.uncut_polyline
        for n in self.nodes:
            n.cut_beams()

    def to_dict(self):
        """Serializes the truss into a dictionary format."""
        return {
            "nodes": [node.to_dict() for node in self.nodes],
            "beams": [beam.to_dict() for beam in self.beams]
        }

    def to_json(self, file_path=None):
        """Converts the dictionary representation to a JSON string or saves it to a file.
        """
        json_data = json.dumps(self.to_dict(), indent=4)

        if file_path:
            with open(file_path, "w") as file:
                file.write(json_data)
            print(f"Truss data saved to: {file_path}")
        else:
            return json_data

    @classmethod
    def from_json(cls, file_path):
        """Loads a Truss object from a JSON file."""
        if not os.path.exists(file_path):
            raise FileNotFoundError(f"File not found: {file_path}")

        with open(file_path, "r") as file:
            data = json.load(file)

        truss = cls()

        # Reconstruct nodes
        node_map = {}  # Maps node ID to actual node objects
        for node_data in data["nodes"]:
            position = rg.Point3d(*node_data["position"])
            node = Node(node_data["id"], position)
            node.has_moved = node_data.get("has_moved", False)
            truss.nodes.append(node)
            node_map[node_data["id"]] = node  # Store mapping for later use

        cut_polylines = []
        # Reconstruct beams
        for beam_data in data["beams"]:

            axis = rg.Line(
                rg.Point3d(*beam_data["axis"]["from"]),
                rg.Point3d(*beam_data["axis"]["to"]),
            )

            # Reconstruct the cut polyline
            cut_polyline_points = [rg.Point3d(*pt) for pt in beam_data["cut_polyline"]]
            cut_polyline = rg.PolylineCurve(cut_polyline_points)
            cut_polylines.append(cut_polyline)

            truss.add_beam(axis, beam_data["height"], beam_data["width"], beam_data["is_new"], beam_data["fabricated"])
            truss.beams[-1].reference_width = beam_data["reference_width"]

        for beam, ctp in zip(truss.beams, cut_polylines):
            beam.cut_polyline = ctp

        return truss

    def __repr__(self):
        return f"Truss(Nodes={len(self.nodes)}, Beams={len(self.beams)})"


class Beam:
    def __init__(self, id, start_node_index, end_node_index, axis, height, width, isnew=False, fabricated=False):

        self.id = id
        # a line defining the main axis of the beam
        self.axis = axis
        self.start_node, self.end_node = start_node_index, end_node_index
        self.height = height
        self.width = width
        self.reference_width = width
        self.uncut_polyline = self.get_uncut_polyline()
        self.centroid = rg.AreaMassProperties.Compute(self.uncut_polyline.ToNurbsCurve()).Centroid
        self.xaxis = self.axis.PointAt(0) - self.axis.PointAt(1)
        self.yaxis = rg.Vector3d.CrossProduct(self.xaxis, rg.Vector3d(0, 0, 1))
        self.plane = rg.Plane(self.centroid, self.xaxis, self.yaxis)
        self.fabricated = fabricated
        self.is_new = isnew

        if not self.fabricated:
            self.cut_polyline = self.uncut_polyline

    def get_uncut_polyline(self):

        line_direction = self.axis.Direction
        line_direction.Unitize()

        # Create a perpendicular vector for width
        perp = rg.Vector3d.CrossProduct(line_direction, rg.Vector3d.ZAxis)
        perp.Unitize()
        perp *= (0.5 * self.width)

        # Define the corners of the rectangular profile
        corners = [
            self.axis.From + perp,
            self.axis.From - perp,
            self.axis.To - perp,
            self.axis.To + perp
        ]

        # Create the rectangle polyline
        rectangle = rg.Polyline(corners + [corners[0]])
        return rg.PolylineCurve(rectangle)

    def get_cut_planes(self, blade_side_A, blade_side_B):

        def angle_from_center_clockwise(pt, center):
            vector = rg.Vector3d(pt.X - center.X, pt.Y - center.Y, 0)
            angle = math.atan2(vector.Y, vector.X)
            # Rotate reference so positive Y-axis is 0 radians
            adjusted_angle = (angle - math.pi / 2) % (2 * math.pi)
            return adjusted_angle

        def angle_from_center_anticlockwise(pt, center):
            vector = rg.Vector3d(pt.X - center.X, pt.Y - center.Y, 0)
            angle = math.atan2(vector.Y, vector.X)
            # Rotate reference so positive Y-axis is 0 radians and reverse for CCW
            adjusted_angle = (2 * math.pi - (angle - math.pi / 2)) % (2 * math.pi)
            return adjusted_angle

        def get_plane(pt1, pt2, blade_plane):
            yaxis = pt2 - pt1
            xaxis = rg.Vector3d.CrossProduct(yaxis, rg.Vector3d(0, 0, -1))
            source = rg.Plane(pt1, xaxis, yaxis)
            trans = rg.Transform.PlaneToPlane(source, blade_plane)
            temp = rg.Plane(self.plane.Origin, self.plane.XAxis, self.plane.YAxis)
            temp.Transform(trans)
            return temp

        moved_centroid = self.centroid - 0.5 * (self.width*self.yaxis)
        moved_ref_plane = rg.Plane(moved_centroid, self.xaxis, self.yaxis)

        trans = rg.Transform.PlaneToPlane(moved_ref_plane, rg.Plane.WorldXY)
        points = list(self.cut_polyline.ToPolyline())[:-1]
        for p in points:
            p.Transform(trans)

        center = rg.Point3d(0, 0, 0)
        positive_x = [pt for pt in points if pt.X > 0]
        negative_x = [pt for pt in points if pt.X < 0]

        Positive_X_Sorted = sorted(positive_x, key=lambda pt: angle_from_center_anticlockwise(pt, center))
        Negative_X_Sorted = sorted(negative_x, key=lambda pt: angle_from_center_clockwise(pt, center))

        inverse_trans = trans.TryGetInverse()[1]

        for p in Positive_X_Sorted:
            p.Transform(inverse_trans)
        for p in Negative_X_Sorted:
            p.Transform(inverse_trans)

        self.numbersA = Positive_X_Sorted
        self.numbersB = Negative_X_Sorted

        in_cut_plane_side_a1 = get_plane(Positive_X_Sorted[0], Positive_X_Sorted[1], blade_side_A)
        in_cut_plane_side_a2 = get_plane(Positive_X_Sorted[1], Positive_X_Sorted[2], blade_side_A)

        in_cut_plane_side_b1 = get_plane(Negative_X_Sorted[0], Negative_X_Sorted[1], blade_side_B)
        in_cut_plane_side_b2 = get_plane(Negative_X_Sorted[1], Negative_X_Sorted[2], blade_side_B)

        return in_cut_plane_side_a1, in_cut_plane_side_a2, in_cut_plane_side_b1, in_cut_plane_side_b2

    def add_labels(self):
        # Create a label 'A' on the right side and 'B' on the left side
        plane_A = rg.Plane(self.numbersA[1], -self.plane.XAxis, self.plane.YAxis)
        plane_B = rg.Plane(self.numbersB[1], -self.plane.XAxis, self.plane.YAxis)

        text_A = rg.TextEntity()
        text_A.Plane = plane_A
        text_A.TextHeight = 0.05
        text_A.Text = "A"

        text_B = rg.TextEntity()
        text_B.Plane = plane_B
        text_B.TextHeight = 0.05
        text_B.Text = "B"

        # Convert the text to a 2D curve
        curve_A = rg.Curve.JoinCurves(text_A.Explode())[0]
        curve_B = rg.Curve.JoinCurves(text_B.Explode())[0]

        extrusion_A = rg.Extrusion.Create(curve_A, plane_A, 0.01, True)  # Extrude along the Z-axis
        extrusion_B = rg.Extrusion.Create(curve_B, plane_B, 0.01, True)  # Same for B

        # Convert extrusions to Breps
        brep_A = extrusion_A.ToBrep()
        brep_B = extrusion_B.ToBrep()

        return [brep_A, brep_B]

    def get_visual_attached_mesh(self):
        pass

    def to_dict(self):
        """Serializes the beam into a dictionary format."""
        return {
            "id": self.id,
            "start_node": self.start_node,
            "end_node": self.end_node,
            "axis": {
                "from": [self.axis.From.X, self.axis.From.Y, self.axis.From.Z],
                "to": [self.axis.To.X, self.axis.To.Y, self.axis.To.Z]
            },
            "height": self.height,
            "width": self.width,
            "fabricated": self.fabricated,
            "is_new": self.is_new,
            "reference_width": self.reference_width,
            "cut_polyline": [[pt.X, pt.Y, pt.Z] for pt in self.cut_polyline.ToPolyline()]
        }

    def __repr__(self):
        return f"Beam(ID={self.id}, Start={self.start_node}, End={self.end_node}, Fabricated={self.fabricated})"


class Node:
    def __init__(self, id, position):

        self.id = id
        self.position = position
        self.connected_beams = []
        self.connected_beams_ids = []
        self.has_moved = False

    def add_beam(self, beam):

        self.connected_beams.append(beam)
        self.connected_beams_ids.append(beam.id)

    def organize_beams(self):

        angles = []
        for beam in self.connected_beams:
            if beam.axis.From.DistanceTo(self.position) < 1e-6:
                direction = beam.axis.To - self.position
            else:
                direction = beam.axis.From - self.position

            beam.axis = rg.Line(self.position, direction)

            # Calculate angle using atan2
            angles.append(math.atan2(direction.Y, direction.X))

        # Sort connected beams by angle
        return [x for _, x in sorted(zip(angles, self.connected_beams))]
    
    def cut_beamA_with_beamB(self, beam1, beam2, cutting_points):

        # cut beam1
        if not beam1.fabricated:
            beam1.cut_polyline.Domain = rg.Interval(0,1)
            par = [beam1.cut_polyline.ClosestPoint(p)[1] for p in cutting_points]
            split = beam1.cut_polyline.Split(par)

            if split[0].GetLength() > split[1].GetLength():
                beam1.cut_polyline = split[0]
            else:
                beam1.cut_polyline = split[1]

            # close curve
            pol = beam1.cut_polyline.TryGetPolyline()[1]
            new_points = [p for p in pol]
            new_points.append(new_points[0])
            beam1.cut_polyline = rg.Curve.CreateInterpolatedCurve(new_points, 1)

            # if the other beam is already fabricated and if the current width is larger than the reference_width
            if beam2.fabricated and beam1.width > beam1.reference_width:
                # find the already fabricated corner and add it
                candidate_points = [p for p in beam2.cut_polyline.ToPolyline()]
                for pt in candidate_points:
                    if beam1.cut_polyline.Contains(pt, rg.Plane.WorldXY, 1e-6) == rg.PointContainment.Inside:
                        new_points = new_points[:-1] + [pt] + new_points[-1:]
                        beam1.cut_polyline = rg.Curve.CreateInterpolatedCurve(new_points, 1)

    def cut_beams(self):
        self.helper = []

        organized_beams = self.organize_beams()
        for i, beam1 in enumerate(organized_beams):
            
            beam2 = organized_beams[(i+1) % len(organized_beams)]

            pt_beam1 = beam1.axis.PointAtLength(0.05)
            dir_beam1 = pt_beam1 - self.position
            pt_beam2 = beam2.axis.PointAtLength(0.05)
            dir_beam2 = pt_beam2 - self.position

            events = rg.Intersect.Intersection.CurveCurve(
                beam1.cut_polyline, beam2.cut_polyline, 1e-6, 1e-6
            )

            if events.Count >= 2:
                intersection_points = [event.PointA for event in events]

                # we want the point that is furthest from the node
                furthest_point = max(intersection_points, key=lambda pt: pt.DistanceTo(self.position))
                cutting_points = [self.position, furthest_point]
                # cut beam1
                self.cut_beamA_with_beamB(beam1, beam2, cutting_points)

                # cut beam2
                self.cut_beamA_with_beamB(beam2, beam1, cutting_points)

        #fix those that have >180 degrees angle
        for i, beam1 in enumerate(organized_beams):

            beam2 = organized_beams[(i+1) % len(organized_beams)]

            pt_beam1 = beam1.axis.PointAtLength(0.05)
            dir_beam1 = pt_beam1 - self.position
            pt_beam2 = beam2.axis.PointAtLength(0.05)
            dir_beam2 = pt_beam2 - self.position

            if math.degrees(rg.Vector3d.VectorAngle(dir_beam1, dir_beam2, rg.Plane.WorldXY))> 180:
                
                bisector_direction = (pt_beam1 + pt_beam2) / 2 - self.position
                bisector_line = rg.Line(self.position, self.position - bisector_direction * 10)
                
                # Find the second closest point to self.position on beam1's cut polyline
                polyline_points = [p for p in beam1.cut_polyline.ToPolyline()]
                polyline_points.sort(key=lambda pt: pt.DistanceTo(self.position))
                second_closest_point = polyline_points[2]
                
                # Project this point along dir_beam1
                start = second_closest_point
                end = second_closest_point - 0.25 * (dir_beam1/dir_beam1.Length)
                line = rg.Line(start, end)
                par = rg.Intersect.Intersection.LineLine(line, bisector_line)[1]
                projected_point = line.PointAt(par)
                # Update the point's location in the polyline
                polyline_points = [p for p in beam1.cut_polyline.ToPolyline()]
                index = polyline_points.index(second_closest_point)
                polyline_points = polyline_points[:index] + [projected_point] + polyline_points[index+1:]
                
                # Update beam1.cut_polyline
                beam1.cut_polyline = rg.Curve.CreateInterpolatedCurve(polyline_points, 1)

                # Find the second closest point to self.position on beam2's cut polyline
                polyline_points = [p for p in beam2.cut_polyline.ToPolyline()]
                polyline_points.sort(key=lambda pt: pt.DistanceTo(self.position))
                second_closest_point = polyline_points[1]
                self.helper = second_closest_point

                #Update the point's location in the polyline
                polyline_points = [p for p in beam2.cut_polyline.ToPolyline()]
                index = polyline_points.index(second_closest_point)
                polyline_points = polyline_points[:index] + [projected_point] + polyline_points[index+1:]
                
                # Update beam1.cut_polyline
                beam2.cut_polyline = rg.Curve.CreateInterpolatedCurve(polyline_points, 1)


    def to_dict(self):
        """Serializes the node into a dictionary format."""
        return {
            "id": self.id,
            "position": [self.position.X, self.position.Y, self.position.Z],
            "connected_beams_ids": [id for id in self.connected_beams_ids],
            "has_moved": self.has_moved
        }

    def __repr__(self):
        return f"Node(ID={self.id}, Position={self.position}, connected_beams_ids={self.connected_beams_ids}, has_moved={self.has_moved})"
