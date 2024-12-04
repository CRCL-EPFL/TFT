#include "FrameDrawer.h"

void FrameDrawer::addPoint(ofPoint point) {
    points.push_back(point); // Add the new point to the vector
}

void FrameDrawer::draw() {
    ofSetColor(color); // Set color from parameter
    
    // Draw points
    for (const auto& point : points) {
        ofDrawCircle(point, pointRadius); // Draw a small circle at each point
    }
    
    // Draw lines between points
    if (points.size() > 1) {
        ofSetLineWidth(lineThickness); // Set line thickness from parameter
        for (size_t i = 0; i < points.size() - 1; ++i) {
            ofDrawLine(points[i], points[i + 1]);
        }
    }
}

void FrameDrawer::clear(){
    points.clear();
}
