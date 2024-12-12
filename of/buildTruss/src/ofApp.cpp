#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0);
    receiver.setup(7777);
    // string path = ofToDataPath("../../../../../csv/State_A.csv", true);
    // loadCSVData(path);
    loadCSVData("State_C.csv");
}

// Populate points and lines from CSV
void ofApp::loadCSVData(const std::string& filePath) {
    ofFile file(filePath);
    if (!file.exists()) {
        ofLogError("ofApp") << "Cannot load file: " << filePath;
        return;
    }

    ofBuffer buffer = ofBufferFromFile(filePath);
    bool firstPointLine = true;
    bool readingPoints = true;

    for (auto line : buffer.getLines()) {
        if (line.empty()) {
            readingPoints = false;
            continue;
        }

        vector<string> values = ofSplitString(line, ",");
        
        if (firstPointLine) {
            firstPointLine = false;
            continue;  // Skip header
        }

        if (readingPoints && values.size() >= 4) {
            Point p;
            p.x = ofToFloat(values[1]);
            p.y = 1.560 - ofToFloat(values[2]);
            p.z = ofToFloat(values[3]);
            points.push_back(p);
        } 
        else if (!readingPoints && values.size() >= 3 && values[0] != "Line_Index") {
            Line l;
            l.startIndex = ofToInt(values[1]);
            l.endIndex = ofToInt(values[2]);
            lines.push_back(l);
        }
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    while(receiver.hasWaitingMessages()){
        ofxOscMessage m;
        receiver.getNextMessage(m);
        
        if(m.getAddress() == "/tags"){
            string jsonStr = m.getArgAsString(0);
            // ofLogNotice("OSC") << "Received tags message: " << jsonStr;
            cout << "Received tags message: " << jsonStr << endl;
            
            auto j = nlohmann::json::parse(jsonStr);
            
            tags.clear();
            
            for(auto& tagJson : j){
                int tagId = tagJson["id"].get<int>();
                Tag tag(tagId);
                cout << "Tag ID: " << tagId << endl;
                
                // Parse corners
                vector<ofPoint> corners;
                for(auto& corner : tagJson["corners"]){
                    ofPoint adjustedPoint(
                        corner[0].get<float>() * scaleAdjustment.x + offsetAdjustment.x,
                        corner[1].get<float>() * scaleAdjustment.y + offsetAdjustment.y
                    );
                    corners.push_back(adjustedPoint);
                }
                // cout << "Tag corners: " << corners.size() << endl;
                
                // Parse center
                auto& center = tagJson["center"];
                ofPoint centerPoint(
                    center[0].get<float>() * scaleAdjustment.x + offsetAdjustment.x,
                    center[1].get<float>() * scaleAdjustment.y + offsetAdjustment.y
                );
                cout << "Tag center: " << centerPoint.x << "," << centerPoint.y << endl;

                tag.setPosition(centerPoint, corners);
                
                tags.push_back(tag);
                // ofLogNotice("OSC") << "Added tag " << tagData.id << " at position " << tagData.center.x << "," << tagData.center.y;
                // cout << "Added tag " << tagData.id << " at position " << tagData.center.x << "," << tagData.center.y << endl;
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    for(const auto& tag : tags){
        // Draw the tag outline
        tag.draw();
    }

    const float SCREEN_WIDTH = 2.490;
    const float SCREEN_HEIGHT = 1.560;
    const float CM_TO_PIXELS_X = ofGetWidth() / SCREEN_WIDTH;
    const float CM_TO_PIXELS_Y = ofGetHeight() / SCREEN_HEIGHT;

    ofPushStyle();
    
    float windowWidth = ofGetWidth();
    float windowHeight = ofGetHeight();
    
    // Draw lines
    ofSetColor(255, 255, 0);  // Yellow
    ofSetLineWidth(2);
    for (const auto& line : lines) {
        if (line.startIndex < points.size() && line.endIndex < points.size()) {
            const auto& start = points[line.startIndex];
            const auto& end = points[line.endIndex];
            // Convert relative coordinates to screen coordinates
            float startX = start.x * CM_TO_PIXELS_X;
            float startY = start.y * CM_TO_PIXELS_Y;
            float endX = end.x * CM_TO_PIXELS_X;
            float endY = end.y * CM_TO_PIXELS_Y;
            ofDrawLine(startX, startY, endX, endY);
        }
    }

    ofFill();

    // Draw points
    ofSetColor(255, 0, 0);
    for (const auto& point : points) {
        // Convert relative coordinates to screen coordinates
        float screenX = point.x * CM_TO_PIXELS_X;
        float screenY = point.y * CM_TO_PIXELS_Y;
        ofDrawCircle(screenX, screenY, 7);
    }
    
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    float moveStep = 1.0;     // pixels for offset
    float scaleStep = 0.01;   // scale adjustment increment
    
    switch(key) {
        // Offset adjustments
        case OF_KEY_LEFT:  adjustOffset(-moveStep, 0); break;
        case OF_KEY_RIGHT: adjustOffset(moveStep, 0);  break;
        case OF_KEY_UP:    adjustOffset(0, -moveStep); break;
        case OF_KEY_DOWN:  adjustOffset(0, moveStep);  break;
        
        // Scale adjustments
        case 'w': adjustScale(0, scaleStep);  break;
        case 's': adjustScale(0, -scaleStep); break;
        case 'a': adjustScale(-scaleStep, 0); break;
        case 'd': adjustScale(scaleStep, 0);  break;
        
        // Reset adjustments
        case 'r':
            offsetAdjustment = ofPoint(0, 0);
            scaleAdjustment = ofPoint(1.0, 1.0);
            break;
    }
    
    cout << "Current adjustments - Offset: " << offsetAdjustment << " Scale: " << scaleAdjustment << endl;
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
