#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    ofSetWindowTitle("Projector Calibration");
    ofSetBackgroundColor(0);
    aspectRatio = 16.0f / 10.0f; // Default aspect ratio
    gridScale = 1.0f;
    showGrid = true;
}

//--------------------------------------------------------------
void ofApp::update() {
}

//--------------------------------------------------------------
void ofApp::draw() {
    if (showGrid) {
        drawGrid();
    }

    // Draw aspect ratio text
    ofSetColor(255);
    std::string ratioText = "Aspect Ratio: " + ofToString(aspectRatio, 2);
    ofDrawBitmapString(ratioText, 20, 20);
}

//--------------------------------------------------------------
void ofApp::drawGrid() {
    ofSetColor(255);
    float width = ofGetWidth();
    float height = width / aspectRatio;

    // Center the grid vertically
    float yOffset = (ofGetHeight() - height) / 2.0f;

    // Draw the grid
    int numLines = 10; // Number of grid lines
    float stepX = width / numLines;
    float stepY = height / numLines;

    for (int i = 0; i <= numLines; i++) {
        // Vertical lines
        float x = i * stepX;
        ofDrawLine(x, yOffset, x, yOffset + height);

        // Horizontal lines
        float y = yOffset + i * stepY;
        ofDrawLine(0, y, width, y);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    switch (key) {
    case 'f': // Toggle full screen
        ofToggleFullscreen();
        break;
    case '+': // Increase aspect ratio (stretch vertically)
        aspectRatio += 0.1f;
        break;
    case '-': // Decrease aspect ratio (shrink vertically)
        aspectRatio -= 0.1f;
        if (aspectRatio < 0.1f) aspectRatio = 0.1f; // Avoid negative aspect ratio
        break;
    case 'g': // Toggle grid visibility
        showGrid = !showGrid;
        break;
    }
}
