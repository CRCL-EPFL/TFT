#pragma once
#include "ofMain.h"

class Tag {
public:
    // Constructor
    Tag(int tagId = -1);

    // Visual properties
    struct Appearance {
        ofColor outlineColor;
        ofColor textColor;
        float outlineWidth;
        float fontSize;
        string label;

        Appearance(ofColor outline = ofColor(255, 0, 0),
            ofColor text = ofColor(255, 255, 255),
            float width = 2.0,
            float font = 24,
            string customLabel = "TEST");
    };

    // Core data
    int id;
    ofPoint center;
    vector<ofPoint> corners;
    Appearance appearance;

    // Methods
    void draw() const;
    void update();
    void setPosition(const ofPoint& newCenter, const vector<ofPoint>& newCorners);

    // Add custom appearance
    void setAppearance(const Appearance& newAppearance);

    // Factory method to create predefined tag types
    static Tag createPresetTag(int tagId);

    static bool setupFont();

    ofPoint getInteractionPoint() const;

    enum class State {
        INACTIVE,
        ACTIVE
    };

    bool isPointInHitbox(const ofPoint& point) const;
    State getState() const { return state; }
    void setState(State newState) { state = newState; }

private:
    static ofTrueTypeFont font;

    ofPoint getLocalInteractionPoint() const;

    State state = State::INACTIVE;
    float hitboxSize = 100;
};