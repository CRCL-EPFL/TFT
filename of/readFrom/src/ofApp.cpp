#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0);
    
    // Initialize GUI and add sliders
    gui.setup(); // Setup the panel
    instructions.set("displayFrame", "Click to add points, press 'C' to clear.");
    gui.add(instructions);
    gui.add(pointRadius.set("Point Radius", 5.0, 1.0, 20.0));  // Radius slider
    gui.add(lineThickness.set("Line Thickness", 2.0, 1.0, 10.0)); // Line thickness slider
    gui.add(color.set("Color", ofColor(255, 255, 255), ofColor(0, 0, 0), ofColor(255, 255, 255))); // Color slider
    
    gui.add(translateX.set("Translate X", 0.0, -500.0, 1000.0));
    gui.add(translateY.set("Translate Y", 0.0, -500.0, 1000.0));
    gui.add(scale.set("Scale", 10.0, 1.0, 20.0));
}

//--------------------------------------------------------------
void ofApp::update(){
    // Link PathDrawer parameters to GUI
    frameDrawer.pointRadius = pointRadius;
    frameDrawer.lineThickness = lineThickness;
    frameDrawer.color = color;
    
    frameDrawer.translateX = translateX;
    frameDrawer.translateY = translateY;
    frameDrawer.scale = scale;
}

//--------------------------------------------------------------
void ofApp::draw(){
    frameDrawer.draw(); // Draw points and lines
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'l' || key == 'L') { // Press 'L' to load points from CSV
            loadPointsFromCsv("../../csv/example.csv"); // Replace with your CSV file path
        }
    if (key == 'c' || key == 'C') {
        frameDrawer.clear(); // Clear all points when 'C' key is pressed
    }
    if (key == 'g' || key == 'G') {
        frameDrawer.showGrid = !frameDrawer.showGrid; // Toggle grid visibility
    }
}

void ofApp::loadPointsFromCsv(const std::string& filePath) {
    ofxCsv csv;
    if (csv.load(filePath)) {
        std::vector<ofPoint> points;
        for (int i = 0; i < csv.getNumRows(); i++) {
            float x = ofToFloat(csv[i][0]);
            float y = ofToFloat(csv[i][1]);
            points.push_back(ofPoint(x, y)); // Ignore the z-value
        }
        frameDrawer.setPoints(points); // Set the points in PathDrawer
    } else {
        ofLogError() << "Failed to load CSV file: " << filePath;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
