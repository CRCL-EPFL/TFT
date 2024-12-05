#include "FrameDrawer.h"

void FrameDrawer::addPoint(ofPoint point) {
    points.push_back(point); // Add the new point to the vector
}

void FrameDrawer::draw() {
    drawGrid();
    
    ofSetColor(color); // Set color from parameter
    
    // Draw points
    for (const auto& point : points) {
        float x = point.x * scale + translateX;
        float y = point.y * scale + translateY;
        ofDrawCircle(ofPoint(x, y), pointRadius);
    }
    
    // Draw lines between points
    if (points.size() > 1) {
        ofSetLineWidth(lineThickness); // Set line thickness from parameter
        for (size_t i = 0; i < points.size() - 1; ++i) {
            float x1 = points[i].x * scale + translateX;
            float y1 = points[i].y * scale + translateY;
            float x2 = points[i + 1].x * scale + translateX;
            float y2 = points[i + 1].y * scale + translateY;
            ofDrawLine(ofPoint(x1, y1), ofPoint(x2, y2));
        }
    }
}

void FrameDrawer::clear(){
    points.clear();
}

void FrameDrawer::setPoints(const std::vector<ofPoint>& newPoints) {
    points = newPoints; // Replace the current points with the new set
}

void FrameDrawer::drawGrid() {
    if (!showGrid) return;
    
    ofSetColor(150); // Light grey color for the grid
    ofSetLineWidth(1.0f); // Set a fixed line width for the grid

    // Get the scaled size of each square
    float gridSize = 100; // Default grid size is 100, scaled by the scale factor

    // Draw vertical lines
    for (float x = 0; x < ofGetWidth(); x += gridSize) {
        ofDrawLine(x, 0, x, ofGetHeight());
    }

    // Draw horizontal lines
    for (float y = 0; y < ofGetHeight(); y += gridSize) {
        ofDrawLine(0, y, ofGetWidth(), y);
    }
}
