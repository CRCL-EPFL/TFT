#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0);
    receiver.setup(7777);
    // string path = ofToDataPath("../../../../../csv/State_A.csv", true);
    // loadCSVData(path);
    loadCSVData("State_A.csv");
    loadCSVBoxes("State_A_Boxes.csv");

    Tag::setupFont();
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

void ofApp::loadCSVBoxes(const std::string& filePath) {
    ofFile file(filePath);
    if (!file.exists()) {
        // ofLogError("ofApp") << "Cannot load file: " << filePath;
        cout << "Cannot load file: " << filePath << endl;
        return;
    }

    ofBuffer buffer = ofBufferFromFile(filePath);
    bool firstLine = true;

    for (auto line : buffer.getLines()) {
        if (line.empty()) continue;

        if (firstLine) {
            firstLine = false;
            continue;  // Skip header
        }

        vector<string> values = ofSplitString(line, ",");
        
        if (values.size() >= 12) {  // 4 points × 3 coordinates (x,y,z)
            Box box;
            // Convert coordinates and flip y (1.560 - y) as in loadCSVData
            box.topLeft = ofPoint(
                ofToFloat(values[1]),
                1.560 - ofToFloat(values[2])
            );
            box.topRight = ofPoint(
                ofToFloat(values[4]),
                1.560 - ofToFloat(values[5])
            );
            // Flipped these two
            box.bottomRight = ofPoint(
                ofToFloat(values[7]),
                1.560 - ofToFloat(values[8])
            );
            box.bottomLeft = ofPoint(
                ofToFloat(values[10]),
                1.560 - ofToFloat(values[11])
            );
            boxes.push_back(box);
        }
    }
    // cout << "Boxes loaded: " << boxes.size() << endl;
    // for (size_t i = 0; i < boxes.size(); i++) {
    //     cout << "Box " << i << ":" << endl;
    //     cout << "  Top Left:     (" << boxes[i].topLeft.x << ", " << boxes[i].topLeft.y << ")" << endl;
    //     cout << "  Top Right:    (" << boxes[i].topRight.x << ", " << boxes[i].topRight.y << ")" << endl;
    //     cout << "  Bottom Left:  (" << boxes[i].bottomLeft.x << ", " << boxes[i].bottomLeft.y << ")" << endl;
    //     cout << "  Bottom Right: (" << boxes[i].bottomRight.x << ", " << boxes[i].bottomRight.y << ")" << endl;
    // }
}

//--------------------------------------------------------------
void ofApp::update(){
    while(receiver.hasWaitingMessages()){
        ofxOscMessage m;
        receiver.getNextMessage(m);
        
        if(m.getAddress() == "/tags"){
            string jsonStr = m.getArgAsString(0);
            // ofLogNotice("OSC") << "Received tags message: " << jsonStr;
            // cout << "Received tags message: " << jsonStr << endl;
            
            auto j = nlohmann::json::parse(jsonStr);
            
            tags.clear();
            
            for(auto& tagJson : j){
                int tagId = tagJson["id"].get<int>();
                Tag tag(tagId);
                // cout << "Tag ID: " << tagId << endl;
                
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
                // cout << "Tag center: " << centerPoint.x << "," << centerPoint.y << endl;

                tag.setPosition(centerPoint, corners);
                
                tags.push_back(tag);
                // ofLogNotice("OSC") << "Added tag " << tagData.id << " at position " << tagData.center.x << "," << tagData.center.y;
                // cout << "Added tag " << tagData.id << " at position " << tagData.center.x << "," << tagData.center.y << endl;
            }
        }
    }

    // Handle SELECT tag (id 0) interactions
    for (const auto& selectTag : tags) {
        if (selectTag.id != 0) continue;
        
        // Check against all boxes
        for (size_t i = 0; i < boxes.size(); i++) {
            // cout << "Checking box " << i << endl;
            const auto& box = boxes[i];
            
            // Check if point is inside box using point-in-polygon test
            if (isPointInBox(selectTag.getInteractionPoint(), box)) {
                // cout << "Tag 0 in box " << i << endl;
                if (i < lines.size() && lines[i].state != Line::State::CONFIRMED) {
                    lines[i].state = Line::State::ACTIVE;

                    // Check if confirm tag is also present
                    for (const auto& confirmTag : tags) {
                        if (confirmTag.id == 1 && selectTag.isPointInHitbox(confirmTag.center)) {
                            lines[i].state = Line::State::CONFIRMED;
                        }
                    }
                }
            } else {
                if (i < lines.size() && lines[i].state == Line::State::ACTIVE) {
                    lines[i].state = Line::State::INACTIVE;
                }
            }
        }
    }

    // Handle CONFIRM tag (id 1) interactions
    for (const auto& confirmTag : tags) {
        if (confirmTag.id != 1) continue;
        // cout << "Confirm tag " << confirmTag.id << " at position " << confirmTag.center.x << "," << confirmTag.center.y << endl;
        
        // Check against all other tags
        for (auto& targetTag : tags) {
            if (targetTag.id == 1) continue; // Skip other confirm tags
            // cout << "Target tag " << targetTag.id << " at position " <
            // Check if confirm tag's center is in target's hitbox
            if (targetTag.isPointInHitbox(confirmTag.center)) {
                // cout << "Target tag " << targetTag.id << " in hitbox" << endl;
                if (targetTag.getState() == Tag::State::INACTIVE) {
                    targetTag.setState(Tag::State::ACTIVE);
                    // cout << "Target tag " << targetTag.id << " activated" << endl;
                }
            } else {
                // cout << "Target tag " << targetTag.id << " not in hitbox" << endl;
                if (targetTag.getState() == Tag::State::ACTIVE) {
                    targetTag.setState(Tag::State::INACTIVE);
                }
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
            // Set color based on state
            switch (line.state) {
                case Line::State::CONFIRMED:
                    ofSetColor(0, 255, 0);  // Green for permanent active
                    ofSetLineWidth(5);
                    break;
                case Line::State::ACTIVE:
                    ofSetColor(255, 165, 0);  // Orange for temporary active
                    ofSetLineWidth(5);
                    break;
                case Line::State::INACTIVE:
                default:
                    ofSetColor(255, 255, 0);  // Yellow for inactive
                    ofSetLineWidth(2);
                    break;
            }

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

    // Draw boxes
    // ofSetColor(255, 255, 255, 128);  
    // for (const auto& box : boxes) {
    //     ofBeginShape();
    //     ofVertex(box.topLeft.x * CM_TO_PIXELS_X, box.topLeft.y * CM_TO_PIXELS_Y);
    //     ofVertex(box.topRight.x * CM_TO_PIXELS_X, box.topRight.y * CM_TO_PIXELS_Y);
    //     ofVertex(box.bottomRight.x * CM_TO_PIXELS_X, box.bottomRight.y * CM_TO_PIXELS_Y);
    //     ofVertex(box.bottomLeft.x * CM_TO_PIXELS_X, box.bottomLeft.y * CM_TO_PIXELS_Y);
    //     ofEndShape(true);
    // }
    
    ofPopStyle();
}

bool ofApp::isPointInBox(const ofPoint& point, const Box& box) {
    cout << "Checking if point " << point.x << ", " << point.y << " is in box " << box.topLeft.x << ", " << box.topLeft.y << " - " << box.bottomRight.x << ", " << box.bottomRight.y << endl;
    // Implementation of point-in-polygon test using crossing number algorithm
    int cn = 0;    // crossing number counter
    
    // Create array of vertices for easier processing
    const float CM_TO_PIXELS_X = ofGetWidth() / SCREEN_WIDTH;
    const float CM_TO_PIXELS_Y = ofGetHeight() / SCREEN_HEIGHT;
    
    // Create array of vertices with converted coordinates
    vector<ofPoint> vertices = {
        ofPoint(box.topLeft.x * CM_TO_PIXELS_X, box.topLeft.y * CM_TO_PIXELS_Y),
        ofPoint(box.topRight.x * CM_TO_PIXELS_X, box.topRight.y * CM_TO_PIXELS_Y),
        ofPoint(box.bottomRight.x * CM_TO_PIXELS_X, box.bottomRight.y * CM_TO_PIXELS_Y),
        ofPoint(box.bottomLeft.x * CM_TO_PIXELS_X, box.bottomLeft.y * CM_TO_PIXELS_Y)
    };
    
    // Loop through all edges of the polygon
    for (size_t i = 0; i < vertices.size(); i++) {
        size_t next = (i + 1) % vertices.size();
        
        if (((vertices[i].y <= point.y) && (vertices[next].y > point.y))     // upward crossing
            || ((vertices[i].y > point.y) && (vertices[next].y <= point.y))) { // downward crossing
            // Compute actual edge-ray intersect x-coordinate
            float vt = (float)(point.y - vertices[i].y) / (vertices[next].y - vertices[i].y);
            if (point.x < vertices[i].x + vt * (vertices[next].x - vertices[i].x)) {
                ++cn;   // valid crossing
            }
        }
    }
    
    return (cn & 1);    // 0 if even (outside), 1 if odd (inside)
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
    
    // cout << "Current adjustments - Offset: " << offsetAdjustment << " Scale: " << scaleAdjustment << endl;
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
