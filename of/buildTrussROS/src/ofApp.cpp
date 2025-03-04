#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(0);
    receiver.setup(7777);
    // string path = ofToDataPath("../../../../../csv/State_A.csv", true);
    // loadCSVData(path);
    // loadCSVData("State_C.csv");
      //loadCSVData("helper.csv");
    // loadCSVBoxes("State_A_Boxes.csv");
    // loadCSVBoxes("State_B_Boxes.csv");

    Tag::setupFont();
}

// Populate points and lines from CSV
void ofApp::loadData(const std::string& filePath) {
    ofFile file(filePath);
    if (!file.exists()) {
        ofLogError("ofApp") << "Cannot load file: " << filePath;
        return;
    }

    ofBuffer buffer = ofBufferFromFile(filePath);
    string jsonString = buffer.getText();

    try {
        auto j = nlohmann::json::parse(jsonString);
        
        // Read points
        if (j.contains("points") && j["points"].is_array()) {
            for (const auto& pointJson : j["points"]) {
                Point p;
                p.id = pointJson["id"];
                p.x = pointJson["x"];
                p.y = 1.560 - float(pointJson["y"]);
                p.z = pointJson["z"];
                p.hasMoved = pointJson["hasMoved"];
                
                // Connected points array
                if (pointJson.contains("connected") && pointJson["connected"].is_array()) {
                    for (const auto& connectedId : pointJson["connected"]) {
                        p.connectedPoints.push_back(connectedId);
                    }
                }
                
                points.push_back(p);
            }
        }

        // Getting enclosed shapes
        findClosedShapes();

    } catch (const nlohmann::json::exception& e) {
        ofLogError("ofApp") << "JSON parsing error: " << e.what();
        return;
    }
}

// Load closed rect geometry from CSV
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
    cout << "Boxes loaded: " << boxes.size() << endl;
    for (size_t i = 0; i < boxes.size(); i++) {
        cout << "Box " << i << ":" << endl;
        cout << "  Top Left:     (" << boxes[i].topLeft.x << ", " << boxes[i].topLeft.y << ")" << endl;
        cout << "  Top Right:    (" << boxes[i].topRight.x << ", " << boxes[i].topRight.y << ")" << endl;
        cout << "  Bottom Left:  (" << boxes[i].bottomLeft.x << ", " << boxes[i].bottomLeft.y << ")" << endl;
        cout << "  Bottom Right: (" << boxes[i].bottomRight.x << ", " << boxes[i].bottomRight.y << ")" << endl;
    }
}

const string ofApp::dataPath = "test.json";
//--------------------------------------------------------------
void ofApp::update(){
    static time_t lastModified = 0;
    // Check if file exists and has been modified
    ofFile file(dataPath);
    if (file.exists()) {
        time_t currentModified = filesystem::last_write_time(file.path());
        if (currentModified > lastModified) {
            loadData(dataPath);
            lastModified = currentModified;
        }
    }

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

            vector<ofPoint> boxVertices = {
                box.topLeft,
                box.topRight,
                box.bottomRight,
                box.bottomLeft
            };
            
            // Check if point is inside box using point-in-polygon test
            if (isPointInPolygon(selectTag.getInteractionPoint(), boxVertices, true)) {
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

    // Handle INSERT tag (id 2) interactions
    tempGeo.isActive = false;
    tempGeo.previewLines.clear();

    for (const auto& insertTag : tags) {
        if (insertTag.id != 2) continue;
        cout << "Found insert tag" << endl;
        ofPoint interactionPoint = insertTag.getInteractionPoint();

        bool snapFound = false;
        // Check against all boxes, for snapping first
        for (size_t i = 0; i < boxes.size(); i++) {
            // cout << "Checking box " << i << endl;
            const auto& box = boxes[i];

            vector<ofPoint> boxVertices = {
                box.topLeft,
                box.topRight,
                box.bottomRight,
                box.bottomLeft
            };
            
            // Check if point is inside box using point-in-polygon test
            if (isPointInPolygon(interactionPoint, boxVertices, true)) {
                snapFound = true;

                // Get the actual line points for line 6
                const auto& startPoint = points[lines[6].startIndex];
                const auto& endPoint = points[lines[6].endIndex];

                // Convert interaction point to world coordinates
                ofPoint worldInteractionPoint(
                    (interactionPoint.x/ofGetWidth()) * SCREEN_WIDTH,
                    (interactionPoint.y/ofGetHeight()) * SCREEN_HEIGHT
                );

                // Calculate perpendicular intersection point
                ofPoint lineVector(endPoint.x - startPoint.x, endPoint.y - startPoint.y);
                ofPoint pointVector(worldInteractionPoint.x - startPoint.x, worldInteractionPoint.y - startPoint.y);
                float lineLengthSq = lineVector.x * lineVector.x + lineVector.y * lineVector.y;
                float dot = (pointVector.x * lineVector.x + pointVector.y * lineVector.y) / lineLengthSq;
                
                // Calculate snap point
                ofPoint snapPoint(
                    startPoint.x + dot * lineVector.x,
                    startPoint.y + dot * lineVector.y
                );

                tempGeo.isActive = true;
                tempGeo.previewPoint.x = snapPoint.x;
                tempGeo.previewPoint.y = snapPoint.y;

                // Find the shape that contains line 6
                const Shape* targetShape = nullptr;
                for (const auto& shape : closedShapes) {
                    if (find(shape.lineIndices.begin(), shape.lineIndices.end(), 6) != shape.lineIndices.end()) {
                        targetShape = &shape;
                        break;
                    }
                }

                // Draw preview lines only from points in the same shape as line 6
                if (targetShape) {
                    for (int pointIndex : targetShape->pointIndices) {
                        // Skip if point is part of line 6
                        if (pointIndex == lines[6].startIndex || pointIndex == lines[6].endIndex) {
                            continue;
                        }
                        
                        TempLine previewLine;
                        previewLine.startIndex = pointIndex;
                        previewLine.endPoint = tempGeo.previewPoint;
                        previewLine.state = Line::State::ACTIVE;
                        tempGeo.previewLines.push_back(previewLine);
                    }
                }

                // Check if confirm tag is present
                for (const auto& confirmTag : tags) {
                    if (confirmTag.id == 1 && insertTag.isPointInHitbox(confirmTag.center)) {
                        // Add the new point
                        Point newPoint;
                        newPoint.x = snapPoint.x;
                        newPoint.y = snapPoint.y;
                        points.push_back(newPoint);
                        int newPointIndex = points.size() - 1;

                        // Add the new lines
                        for (const auto& previewLine : tempGeo.previewLines) {
                            Line newLine;
                            newLine.startIndex = previewLine.startIndex;
                            newLine.endIndex = newPointIndex;
                            newLine.state = Line::State::INACTIVE;
                            lines.push_back(newLine);
                        }

                        // Clear the preview
                        tempGeo.isActive = false;
                        tempGeo.previewLines.clear();

                        // Recalculate shapes with new geometry
                        findClosedShapes();
                        break;
                    }
                }
                break;
            }
        }
        
        // Check against all closed shapes
        if (!snapFound) {
            for (auto& shape : closedShapes) {
                vector<ofPoint> shapePoints;
                // Convert shape points to screen coordinates
                for (int pointIndex : shape.pointIndices) {
                    // Not sure if this is necessary
                    if (pointIndex < points.size()) {
                        const auto& point = points[pointIndex];
                        float screenX = point.x * (ofGetWidth() / SCREEN_WIDTH);
                        float screenY = point.y * (ofGetHeight() / SCREEN_HEIGHT);
                        shapePoints.push_back(ofPoint(screenX, screenY));
                    }
                }
                
                if (isPointInPolygon(interactionPoint, shapePoints, false)) {
                    tempGeo.isActive = true;
                    tempGeo.previewPoint.x = (interactionPoint.x/ofGetWidth()) * SCREEN_WIDTH;
                    tempGeo.previewPoint.y = (interactionPoint.y/ofGetHeight()) * SCREEN_HEIGHT;

                    tempGeo.previewLines.clear();
                    for (int pointIndex : shape.pointIndices) {
                        TempLine previewLine;
                        previewLine.startIndex = pointIndex; 
                        previewLine.endPoint = tempGeo.previewPoint;  
                        previewLine.state = Line::State::ACTIVE;
                        tempGeo.previewLines.push_back(previewLine);
                    }

                    // Check if confirm tag is present
                    for (const auto& confirmTag : tags) {
                        if (confirmTag.id == 1 && insertTag.isPointInHitbox(confirmTag.center)) {
                            // Add the new point
                            Point newPoint;
                            newPoint.x = tempGeo.previewPoint.x;
                            newPoint.y = tempGeo.previewPoint.y;
                            points.push_back(newPoint);
                            int newPointIndex = points.size() - 1;

                            // Add the new lines
                            for (const auto& previewLine : tempGeo.previewLines) {
                                Line newLine;
                                newLine.startIndex = previewLine.startIndex;
                                newLine.endIndex = newPointIndex;
                                newLine.state = Line::State::INACTIVE;
                                lines.push_back(newLine);
                            }

                            // Clear the preview
                            tempGeo.isActive = false;
                            tempGeo.previewLines.clear();

                            // Recalculate shapes with new geometry
                            findClosedShapes();
                            break;
                        }
                    }

                    // cout << "Preview lines: " << tempGeo.previewLines.size() << endl;
                    break;
                }
            }
        }
    }

    // Handle MOVE tag (id 3) interactions
    for (const auto& moveTag : tags) {
        if (moveTag.id != 3) continue;
        
        // Check against all points
        for (auto& point : points) {
            ofPoint interactionPoint = moveTag.getInteractionPoint();
            if (point.isPointInHitbox(interactionPoint, SCREEN_WIDTH, SCREEN_HEIGHT)) {
                point.state = Point::State::ACTIVE;

                // Change state to confirmed if confirm tag is in hitbox
                for (const auto& confirmTag : tags) {
                    if (confirmTag.id == 1 && moveTag.isPointInHitbox(confirmTag.center)) {
                        point.state = Point::State::CONFIRMED;

                        // Convert screen (floor) coordinates back to world coordinates
                        float worldX = (interactionPoint.x / ofGetWidth()) * SCREEN_WIDTH;
                        float worldY = (interactionPoint.y / ofGetHeight()) * SCREEN_HEIGHT;
                        
                        // Update point position to follow the interaction point
                        point.x = worldX;
                        point.y = worldY;
                    }
                }
            } else {
                point.state = Point::State::INACTIVE;
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    // cout << "Draw - tempGeo.isActive: " << tempGeo.isActive << endl;
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
                    ofSetColor(30, 205, 255);  // Cyan for inactive
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

    // Draw preview lines for insert function
    if (tempGeo.isActive) {
        // Draw preview lines
        ofSetColor(255, 165, 0, 128);  // Semi-transparent orange
        ofSetLineWidth(2);
        for (const auto& previewLine : tempGeo.previewLines) {
            if (previewLine.startIndex < points.size()) {
                const auto& start = points[previewLine.startIndex];
                float startX = start.x * CM_TO_PIXELS_X;
                float startY = start.y * CM_TO_PIXELS_Y;
                float endX = previewLine.endPoint.x * CM_TO_PIXELS_X;
                float endY = previewLine.endPoint.y * CM_TO_PIXELS_Y;
                ofDrawLine(startX, startY, endX, endY);
            }
        }
        cout << "Drawing preview lines" << endl;

        // Draw preview point
        // ofSetColor(255, 165, 0);  // Orange
        // float screenX = tempGeo.previewPoint.x * CM_TO_PIXELS_X;
        // float screenY = tempGeo.previewPoint.y * CM_TO_PIXELS_Y;
        // ofDrawCircle(screenX, screenY, 7);
    }

    ofFill();

    // Draw points
    ofSetColor(255, 0, 0);
    for (const auto& point : points) {
        // Convert relative coordinates to screen coordinates
        float screenX = point.x * CM_TO_PIXELS_X;
        float screenY = point.y * CM_TO_PIXELS_Y;

        if (point.state == Point::State::ACTIVE) {
            ofSetColor(0, 255, 0);  // Green for active points
            ofDrawCircle(screenX, screenY, 7);  // Slightly larger
        } else {
            ofSetColor(255, 0, 0);  // Red for inactive points
            ofDrawCircle(screenX, screenY, 4);
        }

        switch (point.state) {
            case Point::State::CONFIRMED:
                ofSetColor(0, 255, 0);  // Green for confirmed
                ofDrawCircle(screenX, screenY, 7);  // Slightly larger
                // ofSetLineWidth(5);
                break;
            case Point::State::ACTIVE:
                ofSetColor(255, 0, 0);  // Cyan for inactive
                ofDrawCircle(screenX, screenY, 7);
                break;
            case Point::State::INACTIVE:
            default:
                ofSetColor(255, 0, 0);  // Cyan for inactive
                ofDrawCircle(screenX, screenY, 4);
                break;
        }
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

bool ofApp::isPointInPolygon(const ofPoint& point, const vector<ofPoint>& vertices, bool convertToScreen) {
    // cout << "Checking if point " << point.x << ", " << point.y << " is in box " << box.topLeft.x << ", " << box.topLeft.y << " - " << box.bottomRight.x << ", " << box.bottomRight.y << endl;
    // Implementation of point-in-polygon test using crossing number algorithm
    int cn = 0;    // crossing number counter
    
    vector<ofPoint> transformedVertices;
    if (convertToScreen) {
        const float CM_TO_PIXELS_X = ofGetWidth() / SCREEN_WIDTH;
        const float CM_TO_PIXELS_Y = ofGetHeight() / SCREEN_HEIGHT;
        
        transformedVertices.reserve(vertices.size());
        for (const auto& vertex : vertices) {
            transformedVertices.emplace_back(
                vertex.x * CM_TO_PIXELS_X,
                vertex.y * CM_TO_PIXELS_Y
            );
        }
    } else {
        transformedVertices = vertices;
    }
    
    // Loop through all edges of the polygon
    for (size_t i = 0; i < transformedVertices.size(); i++) {
        size_t next = (i + 1) % transformedVertices.size();
        
        if (((transformedVertices[i].y <= point.y) && (transformedVertices[next].y > point.y))     // upward crossing
            || ((transformedVertices[i].y > point.y) && (transformedVertices[next].y <= point.y))) { // downward crossing
            // Compute actual edge-ray intersect x-coordinate
            float vt = (float)(point.y - transformedVertices[i].y) / (transformedVertices[next].y - transformedVertices[i].y);
            if (point.x < transformedVertices[i].x + vt * (transformedVertices[next].x - transformedVertices[i].x)) {
                ++cn;   // valid crossing
            }
        }
    }
    
    return (cn & 1);    // 0 if even (outside), 1 if odd (inside)
}

void ofApp::findClosedShapes() {
    closedShapes.clear();
    
    // Create an adjacency list representation of the graph
    vector<vector<int>> adjacencyList(points.size());
    vector<vector<int>> lineIndicesMap(points.size(), vector<int>(points.size(), -1));
    
    // Build adjacency list and line indices map
    for (int i = 0; i < lines.size(); i++) {
        const Line& line = lines[i];
        adjacencyList[line.startIndex].push_back(line.endIndex);
        adjacencyList[line.endIndex].push_back(line.startIndex);
        lineIndicesMap[line.startIndex][line.endIndex] = i;
        lineIndicesMap[line.endIndex][line.startIndex] = i;
    }

    // Helper function to check if a shape contains any internal lines
    auto hasInternalLines = [&](const vector<int>& shape) {
        // Check each pair of non-adjacent vertices
        for (size_t i = 0; i < shape.size(); i++) {
            for (size_t j = i + 2; j < shape.size(); j++) {
                if (j == shape.size() - 1 && i == 0) continue; // Skip if it's first and last vertex
                
                // Check if there's a line between these points
                if (lineIndicesMap[shape[i]][shape[j]] != -1 || 
                    lineIndicesMap[shape[j]][shape[i]] != -1) {
                    return true;
                }
            }
        }
        return false;
    };

    // Helper function to check if shape has repeated vertices
    auto hasRepeatedVertices = [](const vector<int>& shape) {
        vector<int> sortedShape = shape;
        sort(sortedShape.begin(), sortedShape.end());
        return adjacent_find(sortedShape.begin(), sortedShape.end()) != sortedShape.end();
    };

    // Helper function to normalize shape
    auto normalizeShape = [](vector<int>& shape) {
        // Find position of smallest vertex
        auto minIt = min_element(shape.begin(), shape.end());
        rotate(shape.begin(), minIt, shape.end());
        
        // If second element is greater than last element, reverse the shape
        if (shape[1] > shape.back()) {
            reverse(shape.begin() + 1, shape.end());
        }
    };

    // Helper function to check if shape is already found
    auto isShapeUnique = [&normalizeShape](const vector<int>& newShape, const vector<Shape>& shapes) {
        vector<int> normalizedNew = newShape;
        normalizeShape(normalizedNew);
        
        for (const auto& existing : shapes) {
            vector<int> normalizedExisting = existing.pointIndices;
            normalizeShape(normalizedExisting);
            
            if (normalizedNew == normalizedExisting) {
                return false;
            }
        }
        return true;
    };

    // Helper function to verify all edges exist in the shape
    auto allEdgesExist = [&](const vector<int>& shape) {
        for (size_t i = 0; i < shape.size(); i++) {
            int current = shape[i];
            int next = shape[(i + 1) % shape.size()];
            if (lineIndicesMap[current][next] == -1 && lineIndicesMap[next][current] == -1) {
                return false;
            }
        }
        return true;
    };

    // Function to find minimal cycles using DFS
    function<void(int, int, vector<int>&, vector<bool>&)> findCycles = 
    [&](int current, int start, vector<int>& path, vector<bool>& visited) {
        if (path.size() >= 3) {  // Check if we can close the shape
            if (find(adjacencyList[current].begin(), 
                    adjacencyList[current].end(), 
                    start) != adjacencyList[current].end()) {
                
                // Only process valid shapes
                if (!hasInternalLines(path) && 
                    !hasRepeatedVertices(path) && 
                    allEdgesExist(path) && 
                    isShapeUnique(path, closedShapes)) {
                    Shape shape;
                    shape.pointIndices = path;
                    
                    // Add line indices
                    for (int i = 0; i < path.size(); i++) {
                        int point1 = path[i];
                        int point2 = path[(i + 1) % path.size()];
                        int lineIdx = lineIndicesMap[point1][point2];
                        if (lineIdx == -1) lineIdx = lineIndicesMap[point2][point1];
                        if (lineIdx != -1) {
                            shape.lineIndices.push_back(lineIdx);
                        }
                    }
                    
                    closedShapes.push_back(shape);
                }
                return;
            }
        }

        visited[current] = true;
        
        for (int next : adjacencyList[current]) {
            if (!visited[next] || next == start) {
                path.push_back(next);
                findCycles(next, start, path, visited);
                path.pop_back();
            }
        }
        
        visited[current] = false;
    };

    // Search for cycles starting from each point
    for (int i = 0; i < points.size(); i++) {
        vector<bool> visited(points.size(), false);
        vector<int> path = {i};
        findCycles(i, i, path, visited);
    }

    cout << "Found " << closedShapes.size() << " minimal closed shapes:" << endl;
    for (int i = 0; i < closedShapes.size(); i++) {
        cout << "Shape " << i << " points: ";
        for (int pointIdx : closedShapes[i].pointIndices) {
            cout << pointIdx << " ";
        }
        cout << endl;
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
