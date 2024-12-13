#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "json.hpp"
#include "ofxCsv.h"
#include "Tag.h"
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

		// Fine-tuning
    	ofPoint offsetAdjustment{5, -8};
    	ofPoint scaleAdjustment{1.0, 1.0};
		void adjustOffset(float x, float y) { offsetAdjustment += ofPoint(x, y); }
    	void adjustScale(float x, float y) { scaleAdjustment += ofPoint(x, y); }

		struct Point {
			float x, y, z;
			enum class State {
				INACTIVE,
				ACTIVE,
				CONFIRMED
			} state = State::INACTIVE;

			static constexpr float HITBOX_RADIUS = 30.0f;

			bool isPointInHitbox(const ofPoint& testPoint, float screenWidth, float screenHeight) const {
				return ofDist(x * ofGetWidth() / screenWidth, 
							 y * ofGetHeight() / screenHeight,
							 testPoint.x, 
							 testPoint.y) < HITBOX_RADIUS;
			}
		};

		struct Line {
			int startIndex;
			int endIndex;
			enum class State { INACTIVE, ACTIVE, CONFIRMED };
			State state = State::INACTIVE;
		};

		struct Box {
			ofPoint topLeft;
			ofPoint topRight;
			ofPoint bottomLeft;
			ofPoint bottomRight;
		};

		vector<Point> points;
    	vector<Line> lines;
    	void loadCSVData(const std::string& filePath);
		bool isPointInBox(const ofPoint& point, const Box& box);

	private:
		// Screen size in meters
		const float SCREEN_WIDTH = 2.490;
		const float SCREEN_HEIGHT = 1.560;

		vector<Tag> tags;

		vector<Box> boxes;
		void loadCSVBoxes(const std::string& filePath);
};
