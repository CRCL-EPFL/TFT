#pragma once
#include "ofMain.h"

class FrameDrawer {
public:
    // Methods
    void addPoint(ofPoint point);
    void draw();
    void clear();
    
    // GUI adjustable parameters
    ofParameter<float> pointRadius;     // Radius of points
    ofParameter<float> lineThickness;   // Thickness of lines
    ofParameter<ofColor> color;         // Color for points and lines

private:
    // Stores points clicked on the canvas
    std::vector<ofPoint> points;
};
