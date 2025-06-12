#include "MobileRobot.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

MobileRobot::MobileRobot() 
    : position(0.0f, 0.0f, 0.0f), rotation(0.0f), targetPosition(0.0f), 
      targetObjectPosition(0.0f), homePosition(0.0f, 0.0f, 0.0f), state(RobotState::IDLE), 
      navigationMode(NavigationMode::MANUAL), currentPatrolIndex(0),
      patrolWaitTime(3.0f), currentWaitTime(0.0f), autoMode(false),
      returningHome(false), currentTargetObjectIndex(-1), scanDuration(2.0f), 
      scanTimer(0.0f), movementSpeed(2.0f), rotationSpeed(90.0f), 
      scanRange(3.0f), targetTolerance(0.2f)
{
    // Initialize robot arm to default position
    arm.baseRotation = 0.0f;
    arm.shoulderAngle = 45.0f;
    arm.elbowAngle = 90.0f;
    arm.wristAngle = 0.0f;
    arm.isScanning = false;
    arm.scanProgress = 0.0f;
    
    // Setup patrol points around the museum
    setupPatrolPoints();
    
    // Initialize OpenGL geometry
    initializeGeometry();
      std::cout << "Mobile robot initialized at home (center) position (" 
              << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
}

MobileRobot::~MobileRobot() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void MobileRobot::setupPatrolPoints() {
    // Define strategic patrol points around the museum room with center as home
    patrolPoints.clear();
    patrolPoints.push_back(glm::vec3(0.0f, 0.0f, 0.0f));   // Home/Center of room
    patrolPoints.push_back(glm::vec3(-6.0f, 0.0f, 2.0f));  // Near male sculpture
    patrolPoints.push_back(glm::vec3(6.0f, 0.0f, 2.0f));   // Near female sculpture
    patrolPoints.push_back(glm::vec3(-6.0f, 0.0f, -4.0f)); // Near Achilles grave
    patrolPoints.push_back(glm::vec3(6.0f, 0.0f, -4.0f));  // Near Tarhunda sculpture
    patrolPoints.push_back(glm::vec3(0.0f, 0.0f, 6.0f));   // Near sarcophagus
}

void MobileRobot::initializeGeometry() {
    generateRobotGeometry();
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)), indices.data(), GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}

void MobileRobot::generateRobotGeometry() {
    vertices.clear();
    indices.clear();
    
    // Simple robot body geometry (cuboid base + cylindrical top)
    // Robot base (rectangular base)
    float baseWidth = 0.8f, baseHeight = 0.3f, baseDepth = 1.0f;
    
    // Base vertices (bottom box)
    std::vector<glm::vec3> baseVertices = {
        // Bottom face
        glm::vec3(-baseWidth/2, 0.0f, -baseDepth/2),
        glm::vec3(baseWidth/2, 0.0f, -baseDepth/2),
        glm::vec3(baseWidth/2, 0.0f, baseDepth/2),
        glm::vec3(-baseWidth/2, 0.0f, baseDepth/2),
        // Top face
        glm::vec3(-baseWidth/2, baseHeight, -baseDepth/2),
        glm::vec3(baseWidth/2, baseHeight, -baseDepth/2),
        glm::vec3(baseWidth/2, baseHeight, baseDepth/2),
        glm::vec3(-baseWidth/2, baseHeight, baseDepth/2),
    };
    
    // Add base vertices with normals and texture coordinates
    for (const auto& vertex : baseVertices) {
        vertices.push_back(vertex.x);
        vertices.push_back(vertex.y);
        vertices.push_back(vertex.z);
        vertices.push_back(0.0f); // Normal X
        vertices.push_back(1.0f); // Normal Y  
        vertices.push_back(0.0f); // Normal Z
        vertices.push_back(0.0f); // Texture U
        vertices.push_back(0.0f); // Texture V
    }
    
    // Robot body indices (cube)
    std::vector<unsigned int> baseIndices = {
        // Bottom face
        0, 1, 2, 2, 3, 0,
        // Top face
        4, 5, 6, 6, 7, 4,
        // Front face
        0, 1, 5, 5, 4, 0,
        // Back face
        2, 3, 7, 7, 6, 2,
        // Left face
        3, 0, 4, 4, 7, 3,
        // Right face
        1, 2, 6, 6, 5, 1
    };
    
    indices.insert(indices.end(), baseIndices.begin(), baseIndices.end());
    
    // Robot head/sensor (cylinder on top)
    float headRadius = 0.25f;
    float headHeight = 0.4f;
    float headY = baseHeight;
    int segments = 12;
    
    int headStartIndex = vertices.size() / 8;
    
    // Head center top
    vertices.insert(vertices.end(), {0.0f, headY + headHeight, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f});
    // Head center bottom
    vertices.insert(vertices.end(), {0.0f, headY, 0.0f, 0.0f, -1.0f, 0.0f, 0.5f, 0.5f});
    
    // Head circumference vertices
    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float x = headRadius * cos(angle);
        float z = headRadius * sin(angle);
        
        // Top ring
        vertices.insert(vertices.end(), {x, headY + headHeight, z, x/headRadius, 0.0f, z/headRadius, 
                                       (cos(angle) + 1.0f) * 0.5f, (sin(angle) + 1.0f) * 0.5f});
        // Bottom ring
        vertices.insert(vertices.end(), {x, headY, z, x/headRadius, 0.0f, z/headRadius,
                                       (cos(angle) + 1.0f) * 0.5f, (sin(angle) + 1.0f) * 0.5f});
    }
      // Head indices
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        
        // Top face triangles
        indices.insert(indices.end(), {
            static_cast<unsigned int>(headStartIndex),
            static_cast<unsigned int>(headStartIndex + 2 + i * 2),
            static_cast<unsigned int>(headStartIndex + 2 + next * 2)
        });
        
        // Bottom face triangles
        indices.insert(indices.end(), {
            static_cast<unsigned int>(headStartIndex + 1),
            static_cast<unsigned int>(headStartIndex + 3 + next * 2),
            static_cast<unsigned int>(headStartIndex + 3 + i * 2)
        });
        
        // Side faces (quads as triangles)
        indices.insert(indices.end(), {
            static_cast<unsigned int>(headStartIndex + 2 + i * 2),     // top current
            static_cast<unsigned int>(headStartIndex + 3 + i * 2),     // bottom current
            static_cast<unsigned int>(headStartIndex + 3 + next * 2),  // bottom next
            
            static_cast<unsigned int>(headStartIndex + 2 + i * 2),     // top current
            static_cast<unsigned int>(headStartIndex + 3 + next * 2),  // bottom next
            static_cast<unsigned int>(headStartIndex + 2 + next * 2)   // top next
        });
    }
}

void MobileRobot::update(float deltaTime, const MuseumObjectManager& objectManager) {
    if (physicsEnabled) {
        updatePhysics(deltaTime);
    }
    updateMovement(deltaTime);
    updateArm(deltaTime);
    updateScanning(deltaTime, objectManager);
    updateArmCollisionSpheres();
    
    if (navigationMode == NavigationMode::AUTO_PATROL) {
        updateAutoPatrol(deltaTime, objectManager);
    }
    
    // Handle automatic tour mode
    if (autoMode) {
        updateAutomaticTour(deltaTime, objectManager);
    }
}

void MobileRobot::updateMovement(float deltaTime) {
    if (state == RobotState::MOVING_TO_TARGET || state == RobotState::RETURNING_HOME) {
        // Rotate towards target
        rotateTowardsTarget(deltaTime);
        
        // Move towards target
        glm::vec3 direction = glm::normalize(targetPosition - position);
        position += direction * movementSpeed * deltaTime;
        
        // Check if reached target
        if (isAtTarget()) {
            if (state == RobotState::RETURNING_HOME) {
                std::cout << "Robot returned to home base (center)" << std::endl;
            } else {
                std::cout << "Robot reached target position" << std::endl;
            }
            state = RobotState::IDLE;
        }
    }
      // If idle and positioned for object viewing, ensure proper facing
    if (state == RobotState::IDLE && navigationMode == NavigationMode::GOTO_OBJECT) {
        rotateTowardsObject(deltaTime);
    }
}

void MobileRobot::updateArm(float deltaTime) {
    if (arm.isScanning) {
        // Animate scanning motion
        arm.scanProgress += deltaTime / scanDuration;
        
        // Oscillate arm for scanning animation
        float oscillation = sin(arm.scanProgress * M_PI * 4.0f) * 15.0f; // ±15 degrees
        arm.baseRotation = oscillation;
        arm.shoulderAngle = 30.0f + sin(arm.scanProgress * M_PI * 2.0f) * 10.0f;
        
        if (arm.scanProgress >= 1.0f) {
            arm.isScanning = false;
            arm.scanProgress = 0.0f;
            arm.baseRotation = 0.0f;
            arm.shoulderAngle = 45.0f;
            state = RobotState::IDLE;
            std::cout << "Scanning completed" << std::endl;
        }
    }
}

void MobileRobot::updateScanning(float deltaTime, const MuseumObjectManager& objectManager) {    if (arm.isScanning && arm.scanProgress > 0.5f && !lastScanResult.hasResult) {
        // Check for nearby objects to scan with increased range for better detection
        int nearestObject = objectManager.findClosestObject(position, scanRange * 2.0f);
        
        if (nearestObject >= 0) {
            const MuseumObject* obj = objectManager.getObject(nearestObject);
            if (obj) {
                lastScanResult.hasResult = true;
                lastScanResult.objectName = obj->name;
                lastScanResult.objectDescription = obj->description;
                lastScanResult.objectPosition = obj->position;
                lastScanResult.objectIndex = nearestObject;
                lastScanResult.scanTime = glfwGetTime();
                
                std::cout << "Scanned object: " << obj->name << std::endl;
                std::cout << "Scan result set, popup should appear!" << std::endl;
            }
        } else {
            std::cout << "No object found within scan range. Try moving closer to an object." << std::endl;
        }
    }
}

void MobileRobot::updateAutoPatrol(float deltaTime, const MuseumObjectManager& objectManager) {
    if (state == RobotState::IDLE) {
        currentWaitTime += deltaTime;
        
        if (currentWaitTime >= patrolWaitTime) {
            // Move to next patrol point
            currentPatrolIndex = (currentPatrolIndex + 1) % patrolPoints.size();
            setTarget(patrolPoints[currentPatrolIndex]);
            currentWaitTime = 0.0f;
            
            // Start scanning if near an object
            if (findNearestObject(objectManager) >= 0) {
                startScan();
            }
        }
    }
}

void MobileRobot::updateAutomaticTour(float deltaTime, const MuseumObjectManager& objectManager) {
    // Ensure scannedObjects vector matches object count
    if (scannedObjects.size() != objectManager.getObjectCount()) {
        scannedObjects.resize(objectManager.getObjectCount(), false);
    }
    
    if (returningHome) {
        // Robot is returning home, let normal movement handle it
        if (state == RobotState::IDLE && isAtTarget()) {
            // Reached home, stop automatic tour
            autoMode = false;
            returningHome = false;
            currentTargetObjectIndex = -1;
            scannedObjects.clear();
            std::cout << "Robot returned home. Automatic tour complete." << std::endl;
        }
        return;
    }
    
    // If robot is currently moving or scanning, wait for it to finish
    if (state == RobotState::MOVING_TO_TARGET || state == RobotState::SCANNING) {
        return;
    }
    
    // Robot is idle, check if we need to move to next object or scan current one
    if (state == RobotState::IDLE) {
        // If we have a current target object, check if we're close enough to scan
        if (currentTargetObjectIndex >= 0 && currentTargetObjectIndex < scannedObjects.size()) {
            const MuseumObject* currentObj = objectManager.getObject(currentTargetObjectIndex);
            if (currentObj && !scannedObjects[currentTargetObjectIndex]) {
                // Check if we're close enough to the object to scan
                float distanceToObject = glm::length(position - currentObj->position);
                if (distanceToObject <= scanRange + 1.0f) {
                    // Start scanning this object
                    startScan();
                    // Mark object as scanned in our internal tracking
                    scannedObjects[currentTargetObjectIndex] = true;
                    std::cout << "Starting scan of object: " << currentObj->name << std::endl;
                    return;
                }
            }
        }
        
        // Find next unscanned object
        int nextObjectIndex = -1;
        for (size_t i = 0; i < objectManager.getObjectCount(); ++i) {
            if (i < scannedObjects.size() && !scannedObjects[i]) {
                nextObjectIndex = static_cast<int>(i);
                break;
            }
        }
        
        if (nextObjectIndex >= 0) {
            // Move to next unscanned object
            currentTargetObjectIndex = nextObjectIndex;
            moveToObject(nextObjectIndex, objectManager);
            std::cout << "Moving to next object for automatic tour: " << nextObjectIndex << std::endl;
        } else {
            // All objects scanned, return home
            returningHome = true;
            currentTargetObjectIndex = -1;
            returnToHome();
            std::cout << "All objects scanned. Returning home..." << std::endl;
        }
    }
}

void MobileRobot::render(Shader& shader) {
    // Set robot material properties
    shader.setVec3("material.ambient", 0.2f, 0.2f, 0.3f);
    shader.setVec3("material.diffuse", 0.4f, 0.4f, 0.6f);
    shader.setVec3("material.specular", 0.8f, 0.8f, 0.9f);
    shader.setFloat("material.shininess", 32.0f);
    shader.setBool("hasTexture", false);
    
    renderRobotBody(shader);
    renderRobotArm(shader);
    
    // Scan beam removed - robot will scan without visual beam
}

void MobileRobot::renderRobotBody(Shader& shader) {
    glm::mat4 modelMatrix = getRobotMatrix();
    shader.setMat4("model", modelMatrix);
      glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void MobileRobot::renderRobotArm(Shader& shader) {
    // Arm base position on top of robot
    glm::vec3 armBasePos = position + glm::vec3(0.0f, 0.7f, 0.0f);
    
    // Render arm segments with joint rotations
    glm::mat4 baseMatrix = glm::translate(glm::mat4(1.0f), armBasePos);
    baseMatrix = glm::rotate(baseMatrix, glm::radians(rotation.y + arm.baseRotation), glm::vec3(0, 1, 0));
    
    // Shoulder joint
    glm::mat4 shoulderMatrix = baseMatrix;
    shoulderMatrix = glm::rotate(shoulderMatrix, glm::radians(arm.shoulderAngle), glm::vec3(1, 0, 0));
    shoulderMatrix = glm::translate(shoulderMatrix, glm::vec3(0, 0, 0.3f));
    shoulderMatrix = glm::scale(shoulderMatrix, glm::vec3(0.1f, 0.1f, 0.3f));
    
    shader.setMat4("model", shoulderMatrix);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0); // Draw first 36 indices (cube)
    
    // Elbow joint
    glm::mat4 elbowMatrix = baseMatrix;
    elbowMatrix = glm::rotate(elbowMatrix, glm::radians(arm.shoulderAngle), glm::vec3(1, 0, 0));
    elbowMatrix = glm::translate(elbowMatrix, glm::vec3(0, 0, 0.6f));
    elbowMatrix = glm::rotate(elbowMatrix, glm::radians(arm.elbowAngle), glm::vec3(1, 0, 0));
    elbowMatrix = glm::translate(elbowMatrix, glm::vec3(0, 0, 0.25f));
    elbowMatrix = glm::scale(elbowMatrix, glm::vec3(0.08f, 0.08f, 0.25f));
    
    shader.setMat4("model", elbowMatrix);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
}

void MobileRobot::renderScanBeam(Shader& shader) {
    // Render a small glowing sphere at the arm tip to show scanning is active
    shader.setVec3("material.ambient", 0.3f, 1.0f, 0.3f);
    shader.setVec3("material.diffuse", 0.0f, 1.0f, 0.0f);
    shader.setVec3("material.specular", 0.8f, 1.0f, 0.8f);
    
    // Get the exact arm tip position
    glm::vec3 armTipPos = calculateArmTipPosition();
    
    // Create a small glowing sphere at the arm tip
    glm::mat4 scanIndicatorMatrix = glm::translate(glm::mat4(1.0f), armTipPos);
    
    // Make it pulsate based on scan progress for visual feedback
    float pulseScale = 0.05f + 0.03f * sin(arm.scanProgress * M_PI * 8.0f);
    scanIndicatorMatrix = glm::scale(scanIndicatorMatrix, glm::vec3(pulseScale));
    
    shader.setMat4("model", scanIndicatorMatrix);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

glm::mat4 MobileRobot::getRobotMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    return model;
}

void MobileRobot::setPosition(const glm::vec3& pos) {
    position = pos;
    position.y = 0.0f; // Keep robot on ground
}

void MobileRobot::setTarget(const glm::vec3& target) {
    targetPosition = target;
    targetPosition.y = 0.0f; // Keep target on ground
    state = RobotState::MOVING_TO_TARGET;
    lastScanResult.hasResult = false; // Clear previous scan result
}

void MobileRobot::moveToObject(int objectIndex, const MuseumObjectManager& objectManager) {
    const MuseumObject* obj = objectManager.getObject(objectIndex);
    if (obj) {
        glm::vec3 objectPos = obj->position;
        glm::vec3 robotTargetPos = calculateOptimalRobotPosition(objectPos, objectIndex);
        
        // Store object position for proper facing direction
        targetObjectPosition = objectPos;
        
        setTarget(robotTargetPos);
        navigationMode = NavigationMode::GOTO_OBJECT;
        
        std::cout << "Moving robot to object: " << obj->name 
                  << " at optimal position (" << robotTargetPos.x 
                  << ", " << robotTargetPos.y << ", " << robotTargetPos.z << ")" << std::endl;
    }
}

void MobileRobot::returnToHome() {
    setTarget(homePosition);
    targetObjectPosition = glm::vec3(0.0f); // Clear object target
    state = RobotState::RETURNING_HOME;
    navigationMode = NavigationMode::MANUAL;
}

void MobileRobot::setNavigationMode(NavigationMode mode) {
    navigationMode = mode;
    if (mode == NavigationMode::AUTO_PATROL) {
        currentPatrolIndex = 0;
        currentWaitTime = 0.0f;
        setTarget(patrolPoints[0]);
    }
}

void MobileRobot::startScan() {
    if (!arm.isScanning) {
        arm.isScanning = true;
        arm.scanProgress = 0.0f;
        state = RobotState::SCANNING;
        lastScanResult.hasResult = false;
        std::cout << "Starting object scan..." << std::endl;
    }
}

void MobileRobot::stopScan() {
    arm.isScanning = false;
    arm.scanProgress = 0.0f;
    state = RobotState::IDLE;
}

void MobileRobot::setArmRotation(float rotation) {
    arm.baseRotation = rotation;
}

void MobileRobot::setArmAngles(float shoulder, float elbow, float wrist) {
    arm.shoulderAngle = shoulder;
    arm.elbowAngle = elbow;
    arm.wristAngle = wrist;
}

float MobileRobot::calculateDistanceToTarget() const {
    return glm::length(targetPosition - position);
}

void MobileRobot::rotateTowardsTarget(float deltaTime) {
    glm::vec3 direction = glm::normalize(targetPosition - position);
    float targetAngle = atan2(direction.x, direction.z) * 180.0f / M_PI;
    
    float angleDiff = targetAngle - rotation.y;
      // Normalize angle difference to [-180, 180]
    while (angleDiff > 180.0f) angleDiff -= 360.0f;
    while (angleDiff < -180.0f) angleDiff += 360.0f;
    
    if (std::abs(angleDiff) > 5.0f) {
        float rotationStep = rotationSpeed * deltaTime;
        if (angleDiff > 0) {
            rotation.y += rotationStep;
        } else {
            rotation.y -= rotationStep;
        }
    }
}

void MobileRobot::rotateTowardsObject(float deltaTime) {
    // Only rotate if we have a valid target object position
    if (targetObjectPosition != glm::vec3(0.0f)) {
        glm::vec3 direction = glm::normalize(targetObjectPosition - position);
        float targetAngle = atan2(direction.x, direction.z) * 180.0f / M_PI;
        
        float angleDiff = targetAngle - rotation.y;
        
        // Normalize angle difference to [-180, 180]
        while (angleDiff > 180.0f) angleDiff -= 360.0f;
        while (angleDiff < -180.0f) angleDiff += 360.0f;
        
        if (std::abs(angleDiff) > 2.0f) { // Smaller tolerance for object facing
            float rotationStep = rotationSpeed * deltaTime;
            if (angleDiff > 0) {
                rotation.y += rotationStep;
            } else {
                rotation.y -= rotationStep;
            }
        }
    }
}

bool MobileRobot::isAtTarget() const {
    return calculateDistanceToTarget() < targetTolerance;
}

int MobileRobot::findNearestObject(const MuseumObjectManager& objectManager) const {
    return objectManager.findClosestObject(position, scanRange);
}

glm::vec3 MobileRobot::calculateOptimalRobotPosition(const glm::vec3& objectPos, int objectIndex) const {
    // Define optimal robot positions for each object based on their locations and characteristics
    glm::vec3 robotPos;
    
    switch (objectIndex) {
        case 0: // Male Sculpture at (-6.0f, 0.0f, 0.0f) - center-left
            // Position robot to the right of the sculpture, facing west (towards sculpture)
            robotPos = glm::vec3(-3.0f, 0.0f, 0.0f);
            break;
            
        case 1: // Female Sculpture at (6.0f, 0.0f, 0.0f) - center-right
            // Position robot to the left of the sculpture, facing east (towards sculpture)
            robotPos = glm::vec3(3.0f, 0.0f, 0.0f);
            break;
            
        case 2: // Achilles Grave at (-6.0f, 0.0f, -6.0f) - back-left corner
            // Position robot south-east of the grave, facing north-west (towards grave)
            robotPos = glm::vec3(-3.5f, 0.0f, -3.5f);
            break;
            
        case 3: // Tarhunda with Chariot at (6.0f, 0.0f, -6.0f) - back-right corner
            // Position robot south-west of the chariot, facing north-east (towards chariot)
            robotPos = glm::vec3(3.5f, 0.0f, -3.5f);
            break;
            
        case 4: // Ancient Sarcophagus at (0.0f, 0.0f, 6.0f) - front center
            // Position robot north of the sarcophagus, facing south (towards sarcophagus)
            robotPos = glm::vec3(0.0f, 0.0f, 3.5f);
            break;
            
        default:
            // Fallback: position robot 2.5 units in front of object (negative Z direction)
            robotPos = objectPos + glm::vec3(0.0f, 0.0f, 2.5f);
            break;
    }
    
    // Ensure robot stays within room boundaries (room is 20x8x20 centered at origin)
    robotPos.x = glm::clamp(robotPos.x, -9.5f, 9.5f);
    robotPos.z = glm::clamp(robotPos.z, -9.5f, 9.5f);
    robotPos.y = 0.0f; // Keep robot on ground
    
    return robotPos;
}

glm::vec3 MobileRobot::calculateArmTipPosition() const {
    // Calculate arm tip position by following the exact same transformation chain as renderRobotArm
    
    // Start from arm base position on top of robot
    glm::vec3 armBasePos = position + glm::vec3(0.0f, 0.7f, 0.0f);
    
    // Apply robot's rotation and arm base rotation
    glm::mat4 baseMatrix = glm::translate(glm::mat4(1.0f), armBasePos);
    baseMatrix = glm::rotate(baseMatrix, glm::radians(rotation.y + arm.baseRotation), glm::vec3(0, 1, 0));
    
    // Follow the exact same transformation as renderRobotArm
    // Shoulder transformation
    glm::mat4 shoulderMatrix = baseMatrix;
    shoulderMatrix = glm::rotate(shoulderMatrix, glm::radians(arm.shoulderAngle), glm::vec3(1, 0, 0));
    shoulderMatrix = glm::translate(shoulderMatrix, glm::vec3(0, 0, 0.3f));
    
    // Elbow transformation (matching renderRobotArm exactly)
    glm::mat4 elbowMatrix = baseMatrix;
    elbowMatrix = glm::rotate(elbowMatrix, glm::radians(arm.shoulderAngle), glm::vec3(1, 0, 0));
    elbowMatrix = glm::translate(elbowMatrix, glm::vec3(0, 0, 0.6f)); // Same as renderRobotArm
    elbowMatrix = glm::rotate(elbowMatrix, glm::radians(arm.elbowAngle), glm::vec3(1, 0, 0));
    elbowMatrix = glm::translate(elbowMatrix, glm::vec3(0, 0, 0.25f)); // End of elbow segment
    
    // Add a small extension for the tip (scanner/end effector)
    elbowMatrix = glm::translate(elbowMatrix, glm::vec3(0, 0, 0.1f));
    
    // Extract the position from the final transformation matrix
    glm::vec4 tipPosition = elbowMatrix * glm::vec4(0, 0, 0, 1.0f);
    
    return glm::vec3(tipPosition.x, tipPosition.y, tipPosition.z);
}

// Enhanced Physics and Collision Detection Implementation
void MobileRobot::updatePhysics(float deltaTime) {
    // Apply friction to robot movement
    robotPhysics.velocity *= (1.0f - robotPhysics.friction * deltaTime);
    
    // Apply gravity and other forces
    robotPhysics.acceleration.y = -9.81f; // Gravity
    
    // Update velocity based on acceleration
    robotPhysics.velocity += robotPhysics.acceleration * deltaTime;
    
    // Prevent robot from falling through ground
    if (position.y < 0.0f) {
        position.y = 0.0f;
        robotPhysics.velocity.y = -robotPhysics.velocity.y * robotPhysics.restitution;
    }
    
    // Reset acceleration for next frame
    robotPhysics.acceleration = glm::vec3(0.0f);
}

bool MobileRobot::checkCollision(const glm::vec3& point, float radius) const {
    for (const auto& sphere : robotCollisionSpheres) {
        if (sphere.isActive) {
            float distance = glm::length(point - sphere.center);
            if (distance < (radius + sphere.radius)) {
                return true;
            }
        }
    }
    return false;
}

bool MobileRobot::checkArmCollision(const MuseumObjectManager& objectManager) const {
    // Check arm collision with museum objects
    for (size_t i = 0; i < objectManager.getObjectCount(); ++i) {
        const MuseumObject* obj = objectManager.getObject(i);
        if (obj) {
            for (const auto& armSphere : arm.segments) {
                if (armSphere.isActive) {
                    float distance = glm::length(obj->position - armSphere.center);
                    if (distance < (armSphere.radius + 1.5f)) { // Object collision radius
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void MobileRobot::resolveCollision(const glm::vec3& collisionPoint, const glm::vec3& normal) {
    // Calculate relative velocity
    glm::vec3 relativeVelocity = robotPhysics.velocity;
    
    // Calculate collision impulse
    float velocityAlongNormal = glm::dot(relativeVelocity, normal);
    
    if (velocityAlongNormal > 0) return; // Objects separating
    
    float e = robotPhysics.restitution;
    float j = -(1 + e) * velocityAlongNormal;
    j /= (1.0f / robotPhysics.mass);
    
    glm::vec3 impulse = j * normal;
    robotPhysics.velocity += impulse / robotPhysics.mass;
}

void MobileRobot::applyForce(const glm::vec3& force) {
    robotPhysics.acceleration += force / robotPhysics.mass;
}

void MobileRobot::updateArmCollisionSpheres() {
    arm.segments.clear();
    
    // Calculate collision spheres for each arm segment
    glm::vec3 armBase = position + glm::vec3(0.0f, 0.7f, 0.0f);
    
    // Base segment
    CollisionSphere baseSphere;
    baseSphere.center = armBase;
    baseSphere.radius = 0.15f;
    arm.segments.push_back(baseSphere);
    
    // Shoulder segment
    glm::mat4 shoulderTransform = glm::translate(glm::mat4(1.0f), armBase);
    shoulderTransform = glm::rotate(shoulderTransform, glm::radians(rotation.y + arm.baseRotation), glm::vec3(0, 1, 0));
    shoulderTransform = glm::rotate(shoulderTransform, glm::radians(arm.shoulderAngle), glm::vec3(1, 0, 0));
    shoulderTransform = glm::translate(shoulderTransform, glm::vec3(0, 0, 0.3f));
    
    CollisionSphere shoulderSphere;
    shoulderSphere.center = glm::vec3(shoulderTransform * glm::vec4(0, 0, 0, 1));
    shoulderSphere.radius = 0.12f;
    arm.segments.push_back(shoulderSphere);
    
    // Elbow segment
    glm::mat4 elbowTransform = shoulderTransform;
    elbowTransform = glm::rotate(elbowTransform, glm::radians(arm.elbowAngle), glm::vec3(1, 0, 0));
    elbowTransform = glm::translate(elbowTransform, glm::vec3(0, 0, 0.4f));
    
    CollisionSphere elbowSphere;
    elbowSphere.center = glm::vec3(elbowTransform * glm::vec4(0, 0, 0, 1));
    elbowSphere.radius = 0.10f;
    arm.segments.push_back(elbowSphere);
    
    // End effector
    glm::vec3 tipPos = calculateArmTipPosition();
    CollisionSphere tipSphere;
    tipSphere.center = tipPos;
    tipSphere.radius = 0.08f;
    arm.segments.push_back(tipSphere);
}

bool MobileRobot::isArmPositionValid(float baseRot, float shoulder, float elbow, float wrist) const {
    // Check joint limits
    if (baseRot < arm.jointLimits[0][0] || baseRot > arm.jointLimits[0][1]) return false;
    if (shoulder < arm.jointLimits[1][0] || shoulder > arm.jointLimits[1][1]) return false;
    if (elbow < arm.jointLimits[2][0] || elbow > arm.jointLimits[2][1]) return false;
    if (wrist < arm.jointLimits[3][0] || wrist > arm.jointLimits[3][1]) return false;
    
    // Check if end effector is within reach
    // Temporarily set angles to check reach
    float oldBase = arm.baseRotation;
    float oldShoulder = arm.shoulderAngle;
    float oldElbow = arm.elbowAngle;
    float oldWrist = arm.wristAngle;
    
    const_cast<RobotArm&>(arm).baseRotation = baseRot;
    const_cast<RobotArm&>(arm).shoulderAngle = shoulder;
    const_cast<RobotArm&>(arm).elbowAngle = elbow;
    const_cast<RobotArm&>(arm).wristAngle = wrist;
    
    glm::vec3 tipPos = calculateArmTipPosition();
    glm::vec3 armBase = position + glm::vec3(0.0f, 0.7f, 0.0f);
    float reach = glm::length(tipPos - armBase);
    
    // Restore original angles
    const_cast<RobotArm&>(arm).baseRotation = oldBase;
    const_cast<RobotArm&>(arm).shoulderAngle = oldShoulder;
    const_cast<RobotArm&>(arm).elbowAngle = oldElbow;
    const_cast<RobotArm&>(arm).wristAngle = oldWrist;
    
    return reach <= arm.maxReach;
}

// Advanced Spotlight System Implementation
glm::vec3 MobileRobot::getScanningSpotlightPosition() const {
    return calculateArmTipPosition();
}

glm::vec3 MobileRobot::getScanningSpotlightDirection() const {
    glm::vec3 armTip = calculateArmTipPosition();
    glm::vec3 armBase = position + glm::vec3(0.0f, 0.7f, 0.0f);
    
    // Direction from arm base to tip, then extended forward
    glm::vec3 armDirection = glm::normalize(armTip - armBase);
    
    // If scanning, point towards nearest object
    if (arm.isScanning && targetObjectPosition != glm::vec3(0.0f)) {
        return glm::normalize(targetObjectPosition - armTip);
    }
    
    return armDirection;
}

float MobileRobot::getScanningSpotlightIntensity() const {
    if (!spotlightSystem.mainSpotlightActive) return 0.0f;
    
    float baseIntensity = spotlightSystem.mainIntensity;
    
    if (arm.isScanning) {
        // Pulsing effect during scanning
        float pulse = 0.7f + 0.3f * sin(arm.scanProgress * M_PI * 8.0f);
        return baseIntensity * pulse;
    }
    
    return baseIntensity * 0.5f; // Dimmed when not scanning
}

bool MobileRobot::hasScanningSpotlight() const {
    return spotlightSystem.mainSpotlightActive;
}

glm::vec3 MobileRobot::getSecondarySpotlightPosition() const {
    // Secondary spotlight from robot body center
    return position + glm::vec3(0.0f, 0.5f, 0.0f);
}

glm::vec3 MobileRobot::getSecondarySpotlightDirection() const {
    // Point in the direction robot is facing
    float yaw = glm::radians(rotation.y);
    return glm::vec3(sin(yaw), -0.2f, cos(yaw));
}

bool MobileRobot::isMainSpotlightActive() const { return spotlightSystem.mainSpotlightActive; }
bool MobileRobot::isSecondarySpotlightActive() const { return spotlightSystem.secondarySpotlightActive; }
float MobileRobot::getMainSpotlightIntensity() const { return spotlightSystem.mainIntensity; }
float MobileRobot::getSecondarySpotlightIntensity() const { return spotlightSystem.secondaryIntensity; }
glm::vec3 MobileRobot::getMainSpotlightColor() const { return spotlightSystem.mainColor; }
glm::vec3 MobileRobot::getSecondarySpotlightColor() const { return spotlightSystem.secondaryColor; }
float MobileRobot::getSpotlightConeAngle() const { return spotlightSystem.spotlightConeAngle; }
float MobileRobot::getSpotlightRange() const { return spotlightSystem.spotlightRange; }
void MobileRobot::setMainSpotlightActive(bool active) { spotlightSystem.mainSpotlightActive = active; }
void MobileRobot::setSecondarySpotlightActive(bool active) { spotlightSystem.secondarySpotlightActive = active; }
void MobileRobot::setSpotlightIntensities(float main, float secondary) { spotlightSystem.mainIntensity = main; spotlightSystem.secondaryIntensity = secondary; }
