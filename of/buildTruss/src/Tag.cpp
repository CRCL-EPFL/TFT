#include "Tag.h"

Tag::Appearance::Appearance(ofColor outline, ofColor text, float width, float font, string customLabel)
    : outlineColor(outline)
    , textColor(text)
    , outlineWidth(width)
    , fontSize(font)
    , label(customLabel) 
{}

Tag::Tag(int tagId) : id(tagId) {
    appearance = createPresetTag(tagId).appearance;
}

void Tag::draw() const {
    // Draw the tag outline
    ofPushStyle();
    
    ofNoFill();
    ofSetColor(appearance.outlineColor);
    ofSetLineWidth(appearance.outlineWidth);
    ofBeginShape();
    for(const auto& corner : corners) {
        ofVertex(corner.x, corner.y);
    }
    ofEndShape(true);
    
    // Draw the tag ID/label
    ofSetColor(appearance.textColor);
    string displayText = appearance.label.empty() ? 
        "Tag " + ofToString(id) : appearance.label;
    ofDrawBitmapString(displayText, center.x, center.y);
    
    ofPopStyle();
}

void Tag::update() {
}

void Tag::setPosition(const ofPoint& newCenter, const vector<ofPoint>& newCorners) {
    center = newCenter;
    corners = newCorners;
}

void Tag::setAppearance(const Appearance& newAppearance) {
    appearance = newAppearance;
}

Tag Tag::createPresetTag(int tagId) {
    Tag tag(tagId);
    
    // Define preset appearances based on tag ID
    switch(tagId) {
        case 0:
            tag.appearance = Appearance(
                ofColor(255,0,0),    // Red outline
                ofColor(255,255,255), // White text
                2.0,                  // Width
                10,                   // Font size
                "Red Tag"            // Label
            );
            break;
        case 1:
            tag.appearance = Appearance(
                ofColor(0,255,0),    // Green outline
                ofColor(255,255,255),
                2.0,
                10,
                "Green Tag"
            );
            break;
        case 2:
            tag.appearance = Appearance(
                ofColor(0,0,255),    // Blue outline
                ofColor(255,255,255),
                2.0,
                10,
                "Blue Tag"
            );
            break;
        default:
            tag.appearance = Appearance(); // Default appearance
            break;
    }
    
    return tag;
}