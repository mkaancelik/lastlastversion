#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include "Shader.h"
#include "MuseumObjectManager.h"

enum class RobotState {
    IDLE,
    MOVING_TO_TARGET,
    SCANNING,
    RETURNING_HOME
};

enum class NavigationMode {
    MANUAL,
    AUTO_PATROL,
    GOTO_OBJECT
};

// Enhanced physics and collision detection structures
struct PhysicsProperties {
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f);
    float mass = 1.0f;
    float friction = 0.8f;
    float restitution = 0.3f; // Bounciness
};

struct CollisionSphere {
    glm::vec3 center;
    float radius;
    bool isActive = true;
};

struct RobotArm {
    float baseRotation = 0.0f;        // Y-axis rotation of the arm base
    float shoulderAngle = 0.0f;       // Shoulder joint angle
    float elbowAngle = 90.0f;         // Elbow joint angle
    float wristAngle = 0.0f;          // Wrist rotation
    bool isScanning = false;          // Animation state for scanning
    float scanProgress = 0.0f;        // 0.0 to 1.0 for scan animation
    
    // Enhanced physics properties
    PhysicsProperties physics;
    std::vector<CollisionSphere> segments; // Collision spheres for each arm segment
    float maxReach = 2.0f;
    float jointLimits[4][2] = {        // [joint][min,max] in degrees
        {-180.0f, 180.0f},  // Base rotation
        {-90.0f, 120.0f},   // Shoulder
        {0.0f, 160.0f},     // Elbow
        {-180.0f, 180.0f}   // Wrist
    };
};

struct ScanResult {
    bool hasResult = false;
    std::string objectName;
    std::string objectDescription;
    glm::vec3 objectPosition;
    int objectIndex = -1;
    float scanTime = 0.0f;
};

class MobileRobot {
public:
    MobileRobot();
    ~MobileRobot();
    
    // Main update and render functions
    void update(float deltaTime, const MuseumObjectManager& objectManager);
    void render(Shader& shader);
    
    // Navigation controls
    void setPosition(const glm::vec3& position);
    void setTarget(const glm::vec3& target);
    void moveToObject(int objectIndex, const MuseumObjectManager& objectManager);
    void returnToHome();
    void setNavigationMode(NavigationMode mode);
    
    // Robot arm controls
    void setArmRotation(float rotation);
    void setArmAngles(float shoulder, float elbow, float wrist);
    void startScan();
    void stopScan();
    
    // Enhanced physics and collision methods
    void updatePhysics(float deltaTime);
    bool checkCollision(const glm::vec3& point, float radius) const;
    bool checkArmCollision(const MuseumObjectManager& objectManager) const;
    void resolveCollision(const glm::vec3& collisionPoint, const glm::vec3& normal);
    void applyForce(const glm::vec3& force);
    void updateArmCollisionSpheres();
    bool isArmPositionValid(float baseRot, float shoulder, float elbow, float wrist) const;
    
    // Advanced spotlight methods
    glm::vec3 getScanningSpotlightPosition() const;
    glm::vec3 getScanningSpotlightDirection() const;
    float getScanningSpotlightIntensity() const;
    bool hasScanningSpotlight() const;
    glm::vec3 getSecondarySpotlightPosition() const;
    glm::vec3 getSecondarySpotlightDirection() const;
    
    // Getters
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getRotation() const { return rotation; }
    RobotState getState() const { return state; }
    NavigationMode getNavigationMode() const { return navigationMode; }
    const RobotArm& getArm() const { return arm; }    const ScanResult& getLastScanResult() const { return lastScanResult; }
    void clearLastScanResult() { lastScanResult.hasResult = false; }
    bool isScanning() const { return arm.isScanning; }
    
    // Automatic tour getters
    bool isAutoMode() const { return autoMode; }
    bool isReturningHome() const { return returningHome; }
    int getCurrentTargetObjectIndex() const { return currentTargetObjectIndex; }
      // Settings
    void setMovementSpeed(float speed) { movementSpeed = speed; }
    void setRotationSpeed(float speed) { rotationSpeed = speed; }
    void setScanRange(float range) { scanRange = range; }
    
    // Automatic tour controls
    void setAutoMode(bool mode) { autoMode = mode; }
    void setReturningHome(bool returning) { returningHome = returning; }
    void setCurrentTargetObjectIndex(int index) { currentTargetObjectIndex = index; }
    
    // Spotlight system accessors
    bool isMainSpotlightActive() const;
    bool isSecondarySpotlightActive() const;
    float getMainSpotlightIntensity() const;
    float getSecondarySpotlightIntensity() const;
    glm::vec3 getMainSpotlightColor() const;
    glm::vec3 getSecondarySpotlightColor() const;
    float getSpotlightConeAngle() const;
    float getSpotlightRange() const;
    void setMainSpotlightActive(bool active);
    void setSecondarySpotlightActive(bool active);
    void setSpotlightIntensities(float main, float secondary);
    
private:
    // Robot state
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 targetPosition;
    glm::vec3 targetObjectPosition; // Position of the object we're moving towards
    glm::vec3 homePosition;
    RobotState state;
    NavigationMode navigationMode;
    
    // Robot components
    RobotArm arm;
    ScanResult lastScanResult;
    
    // Enhanced physics system
    PhysicsProperties robotPhysics;
    std::vector<CollisionSphere> robotCollisionSpheres;
    bool physicsEnabled = true;
    
    // Advanced lighting system
    struct SpotlightSystem {
        bool mainSpotlightActive = true;
        bool secondarySpotlightActive = false;
        float mainIntensity = 1.0f;
        float secondaryIntensity = 0.5f;
        glm::vec3 mainColor = glm::vec3(0.0f, 1.0f, 1.0f); // Cyan
        glm::vec3 secondaryColor = glm::vec3(1.0f, 0.5f, 0.0f); // Orange
        float spotlightConeAngle = 15.0f;
        float spotlightRange = 10.0f;
    } spotlightSystem;
    
    // Navigation parameters
    float movementSpeed;
    float rotationSpeed;
    float scanRange;
    float targetTolerance;
      // Auto patrol system
    std::vector<glm::vec3> patrolPoints;
    int currentPatrolIndex;
    float patrolWaitTime;
    float currentWaitTime;
      // Automatic tour system
    bool autoMode;
    bool returningHome;
    int currentTargetObjectIndex;
    std::vector<bool> scannedObjects;  // Track which objects have been scanned
    
    // Timing
    float scanDuration;
    float scanTimer;
    
    // OpenGL rendering data
    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
      // Internal methods
    void initializeGeometry();    void updateMovement(float deltaTime);
    void updateArm(float deltaTime);
    void updateScanning(float deltaTime, const MuseumObjectManager& objectManager);
    void updateAutoPatrol(float deltaTime, const MuseumObjectManager& objectManager);
    void updateAutomaticTour(float deltaTime, const MuseumObjectManager& objectManager);
    void generateRobotGeometry();
    void renderRobotBody(Shader& shader);
    void renderRobotArm(Shader& shader);
    void renderScanBeam(Shader& shader);    glm::mat4 getRobotMatrix() const;
    float calculateDistanceToTarget() const;
    void rotateTowardsTarget(float deltaTime);
    void rotateTowardsObject(float deltaTime);
    bool isAtTarget() const;
    int findNearestObject(const MuseumObjectManager& objectManager) const;    void setupPatrolPoints();
    glm::vec3 calculateOptimalRobotPosition(const glm::vec3& objectPos, int objectIndex) const;
    glm::vec3 calculateArmTipPosition() const;
};
