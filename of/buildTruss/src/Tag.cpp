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

ofPoint Tag::getLocalInteractionPoint() const {
    float size = 40;
    switch(id) {
        case 0: // SELECT
            return ofPoint(-size-11, -size-11);
        case 1: // CONFIRM
            return ofPoint(0, 0);  // Center point, or return empty optional since it has no interaction point
        case 2: // INSERT
            return ofPoint(0, size+17);
        case 3: // MOVE
            return ofPoint(0, -size-18);
        default:
            return ofPoint(0, 0);
    }
}

ofPoint Tag::getInteractionPoint() const {
    ofPoint localPoint = getLocalInteractionPoint();
    
    // Calculate rotation
    float dx = corners[1].x - corners[0].x;
    float dy = corners[1].y - corners[0].y;
    float angleRadians = atan2(dy, dx);
    
    // Transform point from local to world space
    ofPoint worldPoint;
    worldPoint.x = center.x + (localPoint.x * cos(angleRadians) - localPoint.y * sin(angleRadians));
    worldPoint.y = center.y + (localPoint.x * sin(angleRadians) + localPoint.y * cos(angleRadians));
    
    return worldPoint;
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
            ofVertex(-size/3, -size);             // Top R
            ofVertex(-size, -size/3);         // Bottom L
            ofEndShape(true);

            ofFill();
            ofDrawCircle(getLocalInteractionPoint(), 3);

            
            // Draw hitbox
            ofNoFill();
            // Change color based on state
            if (state == State::ACTIVE) {
                ofSetColor(0, 255, 0, 180); // Yellow highlight
                ofSetLineWidth(appearance.outlineWidth);
            } else if (state == State::INACTIVE) {
                ofSetColor(255, 255, 255, 100); // Semi-transparent
                ofSetLineWidth(1);
            }
            ofDrawCircle(0, 0, hitboxSize);

            
            break;
            
        case 1: // CONFIRM - Rect
            ofDrawRectangle(-size*0.75, -size*0.75, size*1.5, size*1.5);
            break;
            
        case 2: // INSERT - Triangle
            ofBeginShape();
            ofVertex(-size/2, 2*size/3);        // Top left
            ofVertex(0, size+8);             // Bottom middle 
            ofVertex(size/2, 2*size/3);         // Top right
            ofEndShape(true);

            ofFill();
            ofDrawCircle(getLocalInteractionPoint(), 3);

            // Draw hitbox
            ofNoFill();
            // Change color based on state
            if (state == State::ACTIVE) {
                ofSetColor(0, 255, 0, 180); // Yellow highlight
                ofSetLineWidth(appearance.outlineWidth);
            }
            else if (state == State::INACTIVE) {
                ofSetColor(255, 255, 255, 100); // Semi-transparent
                ofSetLineWidth(1);
            }
            ofDrawCircle(0, 0, hitboxSize);
            break;
            
        case 3: // MOVE - Diamond
            ofBeginShape();
            ofVertex(0, -size);             // Top
            ofVertex(size, 0);              // Right
            ofVertex(0, size);              // Bottom
            ofVertex(-size, 0);             // Left
            ofEndShape(true);

            // vector<float> pattern = {10, 5}; // First number is dash length, second is gap length
            // ofSetDashStyle(pattern);
            // ofDrawRectangle(-size*0.75, -size*0.75+60, size*1.5, size*1.5);

            ofFill();
            ofDrawCircle(getLocalInteractionPoint(), 3);

            // Draw hitbox
            ofNoFill();
            // Change color based on state
            if (state == State::ACTIVE) {
                ofSetColor(0, 255, 0, 180); // Yellow highlight
                ofSetLineWidth(appearance.outlineWidth);
            }
            else if (state == State::INACTIVE) {
                ofSetColor(255, 255, 255, 100); // Semi-transparent
                ofSetLineWidth(1);
            }
            ofDrawCircle(0, 0, hitboxSize);
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

bool Tag::isPointInHitbox(const ofPoint& point) const {
    // Only tags 0, 2, and 3 have hitboxes
    if (id == 1) return false;
    
    // Get interaction point in world space
    ofPoint interactionPoint = getInteractionPoint();
    
    // Calculate distance from point to interaction point
    float distance = ofDist(point.x, point.y, 
                          center.x, center.y);

    cout << "Distance for id " << id << ": " << distance << endl;
    
    // Check if point is within hitbox radius
    return distance < hitboxSize;
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