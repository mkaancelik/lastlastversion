#pragma once

#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Model.h"
#include "Shader.h"

struct MuseumObject {
    std::unique_ptr<Model> model;
    glm::vec3 position;
    glm::vec3 rotation; // Euler angles in degrees
    glm::vec3 scale;
    std::string name;
    std::string description;
    
    // Material properties for realistic coloring
    glm::vec3 materialAmbient;
    glm::vec3 materialDiffuse;
    glm::vec3 materialSpecular;
    
    // Scanning state for automatic tour
    bool scanned;    MuseumObject(const std::string& modelPath, const glm::vec3& pos, 
                 const std::string& objName = "", const std::string& desc = "",
                 const glm::vec3& ambient = glm::vec3(0.2f, 0.15f, 0.1f),
                 const glm::vec3& diffuse = glm::vec3(0.8f, 0.7f, 0.6f),
                 const glm::vec3& specular = glm::vec3(0.3f, 0.3f, 0.3f))
        : position(pos), rotation(0.0f), scale(1.0f), name(objName), description(desc),
          materialAmbient(ambient), materialDiffuse(diffuse), materialSpecular(specular), scanned(false)
    {
        try {
            model = std::make_unique<Model>(modelPath);
        } catch (const std::exception& e) {
            std::cout << "Failed to load model: " << modelPath << " - " << e.what() << std::endl;
        }
    }
    
    glm::mat4 getModelMatrix() const {
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = glm::scale(modelMatrix, scale);
        return modelMatrix;
    }
    
    // Spotlight system for this object
    struct ObjectSpotlight {
        bool active = false;
        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 color = glm::vec3(1.0f, 0.9f, 0.8f); // Warm white
        float intensity = 0.0f;
        float maxIntensity = 1.2f;
        float cutOff = 25.0f;       // Inner cone angle
        float outerCutOff = 35.0f;  // Outer cone angle
        float fadeSpeed = 2.0f;     // How fast to fade in/out
    } spotlight;
};

class MuseumObjectManager {
public:
    MuseumObjectManager();
    ~MuseumObjectManager();
      // Add a museum object
    void addObject(const std::string& modelPath, const glm::vec3& position, 
                   const std::string& name = "", const std::string& description = "",
                   const glm::vec3& ambient = glm::vec3(0.2f, 0.15f, 0.1f),
                   const glm::vec3& diffuse = glm::vec3(0.8f, 0.7f, 0.6f),
                   const glm::vec3& specular = glm::vec3(0.3f, 0.3f, 0.3f));
    
    // Remove an object by index
    void removeObject(size_t index);
    
    // Get object count
    size_t getObjectCount() const { return objects.size(); }
    
    // Get object by index
    MuseumObject* getObject(size_t index);
    const MuseumObject* getObject(size_t index) const;
    
    // Draw all objects
    void drawAll(Shader& shader);
    
    // Load default museum objects
    void loadDefaultObjects();
    
    // Get object information for UI
    std::vector<std::string> getObjectNames() const;
    
    // Find closest object to a position (for robot interaction)
    int findClosestObject(const glm::vec3& position, float maxDistance = 5.0f) const;

    // Museum object spotlight management
    void updateObjectSpotlights(const glm::vec3& robotPosition, float deltaTime);
    void setObjectSpotlightActive(size_t objectIndex, bool active);
    void setObjectSpotlightIntensity(size_t objectIndex, float intensity);
    std::vector<size_t> getActiveSpotlightObjects() const;
    int getNearestObjectToRobot(const glm::vec3& robotPosition, float maxDistance = 8.0f) const;
    
    // Get spotlight data for rendering
    struct SpotlightData {
        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 color;
        float intensity;
        float cutOff;
        float outerCutOff;
    };
    std::vector<SpotlightData> getActiveSpotlights() const;
    
private:
    std::vector<std::unique_ptr<MuseumObject>> objects;
    
    // Helper function to auto-scale objects based on their bounding box
    void autoScaleObject(MuseumObject* obj, float targetSize = 2.0f);
    
    // Spotlight management
    int currentActiveObject = -1;  // Currently lit object
    float spotlightActivationDistance = 6.0f;  // Distance to activate spotlight
    float spotlightDeactivationDistance = 8.0f; // Distance to deactivate spotlight
    
    // Helper methods
    void calculateSpotlightPosition(MuseumObject* obj);
};
