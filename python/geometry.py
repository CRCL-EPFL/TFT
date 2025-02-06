import Rhino.Geometry as rg
import math

class Beam:
    def __init__(self, axis, height, width):

        self.axis = axis
        self.height = height
        self.width = width
        self.uncut_polyline = self.get_uncut_polyline()
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


class Node:
    def __init__(self, position, beams):

        self.position = position
        self.connected_beams = []
        self.filter_beams(beams)

    def add_beam(self, beam):

        self.connected_beams.append(beam)

    def filter_beams(self, all_beams, tolerance=1e-6):

        for beam in all_beams:
            # Check if either endpoint of the beam's axis is close to the node's position
            if beam.axis.From.DistanceTo(self.position) < tolerance or beam.axis.To.DistanceTo(self.position) < tolerance:
                self.add_beam(beam)
    
    def organize_beams(self):

        angles = []
        for beam in self.connected_beams:
            if beam.axis.From.DistanceTo(self.position) < 1e-6:
                direction = beam.axis.To - self.position
            else:
                direction = beam.axis.From - self.position
            
            # Calculate angle using atan2
            angles.append(math.atan2(direction.Y, direction.X))

        # Sort connected beams by angle
        self.connected_beams = [x for _, x in sorted(zip(angles, self.connected_beams))]

    def cut_beams(self):
        

        for i, beam1 in enumerate(self.connected_beams):
            beam2 = self.connected_beams[(i+1)%len(self.connected_beams)]
            events = rg.Intersect.Intersection.CurveCurve(
                beam1.cut_polyline, beam2.cut_polyline, 1e-6, 1e-6
            )
            
            if events.Count>=2:
                intersection_points = [event.PointA for event in events]
                print (len(intersection_points))
                furthest_point = max(intersection_points, key=lambda pt: pt.DistanceTo(self.position))

                # Create a line from the node position to the furthest intersection point
                cutting_line = rg.Line(self.position, furthest_point)
                cutting_line.Extend(0.2,0.2)
                #cut beam1
                beam1.cut_polyline.Domain = rg.Interval(0,1)
                par = [beam1.cut_polyline.ClosestPoint(self.position)[1], beam1.cut_polyline.ClosestPoint(furthest_point)[1] ]
                split = beam1.cut_polyline.Split(par)

                if split[0].GetLength() > split[1].GetLength():
                    beam1.cut_polyline = split[0]
                else:
                    beam1.cut_polyline = split[1]

                #close curve
                pol = beam1.cut_polyline.TryGetPolyline()[1]
                new_points = [p for p in pol]
                new_points.append(new_points[0])
                beam1.cut_polyline = rg.Curve.CreateInterpolatedCurve(new_points, 1)

                #cut beam2
                beam2.cut_polyline.Domain = rg.Interval(0,1)
                par = [beam2.cut_polyline.ClosestPoint(self.position)[1], beam2.cut_polyline.ClosestPoint(furthest_point)[1] ]
                split = beam2.cut_polyline.Split(par)

                if split[0].GetLength() > split[1].GetLength():
                    beam2.cut_polyline = split[0]
                else:
                    beam2.cut_polyline = split[1]

                #close curve
                pol = beam2.cut_polyline.TryGetPolyline()[1]
                new_points = [p for p in pol]
                new_points.append(new_points[0])
                beam2.cut_polyline = rg.Curve.CreateInterpolatedCurve(new_points, 1)