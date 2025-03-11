try:
    import Rhino.Geometry as rg
    RHINO_AVAILABLE = True
except ImportError:
    RHINO_AVAILABLE = False

import json
import numpy as np
from typing import List, Tuple

class WoodPiece:
    def __init__(self, corners: List[List[int]], center: Tuple[int, int], width: float, height: float):
        self.corners = np.array(corners, dtype=int)
        self.center = tuple(center)
        self.width = width
        self.height = height

    def to_dict(self):
        """Convert the WoodPiece object to a dictionary."""
        return {
            'corners': self.corners.tolist(),
            'center': self.center,
            'width': self.width,
            'height': self.height
        }

    @staticmethod
    def from_dict(data: dict):
        """Deserialize a dictionary into a WoodPiece object."""
        return WoodPiece(data['corners'], tuple(data['center']), data['width'], data['height'])
    
    def to_json(self) -> str:
        """Serialize the WoodPiece object to JSON format."""
        return json.dumps(self.to_dict())
    
    @staticmethod
    def from_json(json_str: str):
        """Deserialize a JSON string into a WoodPiece object."""
        data = json.loads(json_str)
        return WoodPiece.from_dict(data)
    
    if RHINO_AVAILABLE:
        def to_rhino_box(self) -> rg.Box:
            """Convert the WoodPiece to a Rhino Geometry Box."""
            plane = rg.Plane(rg.Point3d(self.center[0], self.center[1], 0), rg.Vector3d.ZAxis)
            box = rg.Box(plane, rg.Interval(-self.width / 2, self.width / 2), rg.Interval(-self.height / 2, self.height / 2), rg.Interval(0, 5))
            return box