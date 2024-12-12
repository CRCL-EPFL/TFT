#include "Tag.h"

ofTrueTypeFont Tag::font;

Tag::Appearance::Appearance(ofColor outline, ofColor text, float width, float font, string customLabel)
    : outlineColor(outline)
    , textColor(text)
    , outlineWidth(width)
    , fontSize(font)
    , label(customLabel) 
{}

bool Tag::setupFont() {
    return font.load("Roboto.ttf", 20); // Default size, adjust as needed
}

Tag::Tag(int tagId) : id(tagId) {
    switch(tagId) {
        case 0:
            appearance = Appearance(
                ofColor(255,0,0),    // Red outline
                ofColor(255,255,255), // White text
                2.0,                  // Width
                20,                   // Font size
                "SELECT"             // Label
            );
            break;
        case 1:
            appearance = Appearance(
                ofColor(0,255,0),    // Green outline
                ofColor(255,255,255),
                2.0,
                20,
                "CONFIRM"
            );
            break;
        case 2:
            appearance = Appearance(
                ofColor(255,255,0),    // Yellow outline
                ofColor(255,255,255),
                2.0,
                20,
                "INSERT"
            );
            break;
        case 3:
            appearance = Appearance(
                ofColor(255,65,0),    // Orange outline
                ofColor(255,255,255),
                2.0,
                20,
                "MOVE"
            );
            break;
        default:
            appearance = Appearance(); // Default appearance
            break;
    }
}

void Tag::draw() const {
    // Draw the tag outline
    ofPushStyle();
    
    ofNoFill();
    ofSetColor(appearance.outlineColor);
    // ofSetLineWidth(appearance.outlineWidth);
    float size = 40;

    ofPoint centroid = ofPoint(center.x, center.y);
    // float scale = 4;
    // ofBeginShape();
    // for(const auto& corner : corners) {
    //     // Scale the point from the center
    //     ofPoint scaledPoint;
    //     scaledPoint.x = centroid.x + (corner.x - centroid.x) * scale;
    //     scaledPoint.y = centroid.y + (corner.y - centroid.y) * scale;
    //     ofVertex(scaledPoint.x, scaledPoint.y);
    // }
    // ofEndShape(true);

    // Rotation angle of tag
    float dx = corners[1].x - corners[0].x;
    float dy = corners[1].y - corners[0].y;
    float angleRadians = atan2(dy, dx);
    float angleDegrees = ofRadToDeg(angleRadians);

    ofPushMatrix();
    ofTranslate(center.x, center.y);
    ofRotateDeg(angleDegrees);

    switch(id) {
        case 0: // SELECT - Right Triangle
            ofBeginShape();
            ofVertex(-size, -size);        // Top L
            ofVertex(size/3, -size);             // Top R
            ofVertex(-size, size/3);         // Bottom L
            ofEndShape(true);
            break;
            
        case 1: // CONFIRM - Rect
            ofDrawRectangle(-size/2, -size/2, size, size);
            break;
            
        case 2: // INSERT - Triangle
            ofBeginShape();
            ofVertex(-size, 2*size/3);        // Top left
            ofVertex(0, size+5);             // Bottom middle 
            ofVertex(size, 2*size/3);         // Top right
            ofEndShape(true);
            break;
            
        case 3: // MOVE - Diamond
            ofBeginShape();
            ofVertex(0, -size);             // Top
            ofVertex(size, 0);              // Right
            ofVertex(0, size);              // Bottom
            ofVertex(-size, 0);             // Left
            ofEndShape(true);
            break;
            
        default: // Default - Square
            ofDrawRectangle(-size/2, -size/2, size, size);
            break;
    }
    
    // Draw the tag ID/label
    ofSetColor(appearance.textColor);
    string displayText = appearance.label.empty() ? 
        "Tag " + ofToString(id) : appearance.label;
    // ofDrawBitmapString(displayText, center.x, center.y);
    font.drawString(displayText, 40, 0);
    
    ofPopMatrix();
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
            tag.setAppearance(Appearance(
                ofColor(255,0,0),    // Red outline
                ofColor(255,255,255), // White text
                2.0,                  // Width
                10,                   // Font size
                "SELECT"            // Label
            ));
            break;
        case 1:
            tag.setAppearance(Appearance(
                ofColor(0,255,0),    // Green outline
                ofColor(255,255,255),
                2.0,
                10,
                "CONFIRM"
            ));
            break;
        case 2:
            tag.setAppearance(Appearance(
                ofColor(0,0,255),    // Blue outline
                ofColor(255,255,255),
                2.0,
                10,
                "INSERT"
            ));
            break;
        default:
            tag.setAppearance(Appearance()); // Default appearance
            break;
    }
    
    return tag;
}