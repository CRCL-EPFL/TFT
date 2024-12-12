#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "json.hpp"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		ofxOscReceiver receiver;
        struct TagData {
            int id;
            vector<ofPoint> corners;
            ofPoint center;
        };
        vector<TagData> currentTags;
        
        void updateTags();

		// Fine-tuning
    	ofPoint offsetAdjustment{5, -8};
    	ofPoint scaleAdjustment{1.0, 1.0};
		void adjustOffset(float x, float y) { offsetAdjustment += ofPoint(x, y); }
    	void adjustScale(float x, float y) { scaleAdjustment += ofPoint(x, y); }
};