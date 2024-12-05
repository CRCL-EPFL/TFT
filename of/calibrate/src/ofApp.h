#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);

    void drawGrid();

    float aspectRatio; // Current aspect ratio
    float gridScale;   // Scale of the grid
    bool showGrid;     // Toggle grid visibility
};
