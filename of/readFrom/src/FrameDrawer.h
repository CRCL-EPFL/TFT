#pragma once
#include "ofMain.h"

class FrameDrawer {
public:
    // Methods
    void addPoint(ofPoint point);
    void setPoints(const std::vector<ofPoint>& newPoints);
    void draw();
    void clear();
    void drawGrid();
    
    // GUI adjustable parameters
    ofParameter<float> pointRadius;     // Radius of points
    ofParameter<float> lineThickness;   // Thickness of lines
    ofParameter<ofColor> color;         // Color for points and lines
    
    ofParameter<float> translateX;
    ofParameter<float> translateY;
    ofParameter<float> scale;
    
    bool showGrid = true;

private:
    // Stores points clicked on the canvas
    std::vector<ofPoint> points;
};
