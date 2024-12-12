#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0);
    receiver.setup(7777);
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
            
            currentTags.clear();
            
            for(auto& tag : j){
                TagData tagData;
                tagData.id = tag["id"].get<int>();
                cout << "Tag ID: " << tagData.id << endl;
                
                // Parse corners
                for(auto& corner : tag["corners"]){
                    ofPoint adjustedPoint(
                        corner[0].get<float>() * scaleAdjustment.x + offsetAdjustment.x,
                        corner[1].get<float>() * scaleAdjustment.y + offsetAdjustment.y
                    );
                    tagData.corners.push_back(adjustedPoint);
                }
                cout << "Tag corners: " << tagData.corners.size() << endl;
                
                // Parse center
                auto& center = tag["center"];
                tagData.center = ofPoint(
                    center[0].get<float>() * scaleAdjustment.x + offsetAdjustment.x,
                    center[1].get<float>() * scaleAdjustment.y + offsetAdjustment.y
                );
                cout << "Tag center: " << tagData.center.x << "," << tagData.center.y << endl;
                
                currentTags.push_back(tagData);
                // ofLogNotice("OSC") << "Added tag " << tagData.id << " at position " << tagData.center.x << "," << tagData.center.y;
                cout << "Added tag " << tagData.id << " at position " << tagData.center.x << "," << tagData.center.y << endl;
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    for(const auto& tag : currentTags){
        // Draw the tag outline
        ofNoFill();
        ofSetColor(255, 0, 0);
        ofBeginShape();
        for(const auto& corner : tag.corners){
            ofVertex(corner.x, corner.y);
        }
        ofEndShape(true);
        
        // Draw the tag ID
        ofSetColor(255);
        ofDrawBitmapString("Tag " + ofToString(tag.id), tag.center.x, tag.center.y);
    }
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
