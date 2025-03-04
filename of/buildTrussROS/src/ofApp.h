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

		// NODES
		struct Point {
			int id;
			float x, y, z;
			enum class State {
				INACTIVE,
				ACTIVE,
				CONFIRMED
			} state = State::INACTIVE;

			vector<int> connectedPoints;
			bool hasMoved = false;

			static constexpr float HITBOX_RADIUS = 30.0f;
			
			// Check if something is in the hitbox of this point
			bool isPointInHitbox(const ofPoint& testPoint, float screenWidth, float screenHeight) const {
				return ofDist(x * ofGetWidth() / screenWidth, 
							 y * ofGetHeight() / screenHeight,
							 testPoint.x, 
							 testPoint.y) < HITBOX_RADIUS;
			}
		};

		vector<Point> points;

		struct Line {
			int startIndex;
			int endIndex;
			enum class State { INACTIVE, ACTIVE, CONFIRMED };
			State state = State::INACTIVE;
		};

		// Closed shapes, for checking insert points
		struct Shape {
			vector<int> pointIndices;
			vector<int> lineIndices;
		};

		vector<Shape> closedShapes;
		void findClosedShapes();

		struct Box {
			ofPoint topLeft;
			ofPoint topRight;
			ofPoint bottomLeft;
			ofPoint bottomRight;
		};

    	vector<Line> lines;

    	void loadCSVData(const std::string& filePath);
		// Function for checking if point is in Box object
		bool isPointInPolygon(const ofPoint& point, const vector<ofPoint>& vertices, bool convertToScreen = true);

		struct TempLine {
			int startIndex;      // Index into existing points array
			Point endPoint;      // Actual coordinates of the preview point
			Line::State state;
		};

		struct TempGeo {
			Point previewPoint;
			vector<TempLine> previewLines;
			bool isActive = false;
			
			void clear() {
				previewLines.clear();
				isActive = false;
			}
		};

		TempGeo tempGeo;

	private:
		// Screen size in meters
		const float SCREEN_WIDTH = 2.490;
		const float SCREEN_HEIGHT = 1.560;
		
		// Path to JSON file for nodes
		static const string dataPath;

		vector<Tag> tags;

		vector<Box> boxes;
		void loadCSVBoxes(const std::string& filePath);
};
