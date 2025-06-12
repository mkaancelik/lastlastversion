#include "MuseumObjectManager.h"
#include <iostream>
#include <algorithm>
#include <cmath>

MuseumObjectManager::MuseumObjectManager()
{
}

MuseumObjectManager::~MuseumObjectManager()
{
}

void MuseumObjectManager::addObject(const std::string& modelPath, const glm::vec3& position, 
                                   const std::string& name, const std::string& description,
                                   const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular)
{
    auto obj = std::make_unique<MuseumObject>(modelPath, position, name, description, ambient, diffuse, specular);
    
    if (obj->model) {
        // Auto-scale the object to a reasonable size
        autoScaleObject(obj.get());
        objects.push_back(std::move(obj));
        std::cout << "Added museum object: " << name << " at position (" 
                  << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    } else {
        std::cout << "Failed to add museum object: " << name << " (model loading failed)" << std::endl;
    }
}

void MuseumObjectManager::removeObject(size_t index)
{
    if (index < objects.size()) {
        objects.erase(objects.begin() + index);
    }
}

MuseumObject* MuseumObjectManager::getObject(size_t index)
{
    if (index < objects.size()) {
        return objects[index].get();
    }
    return nullptr;
}

const MuseumObject* MuseumObjectManager::getObject(size_t index) const
{
    if (index < objects.size()) {
        return objects[index].get();
    }
    return nullptr;
}

void MuseumObjectManager::drawAll(Shader& shader)
{
    for (auto& obj : objects) {
        if (obj->model) {
            // Set the model matrix uniform
            glm::mat4 modelMatrix = obj->getModelMatrix();
            shader.setMat4("model", modelMatrix);
            
            // Set object-specific material properties
            shader.setVec3("material.ambient", obj->materialAmbient);
            shader.setVec3("material.diffuse", obj->materialDiffuse);
            shader.setVec3("material.specular", obj->materialSpecular);
            
            // Draw the model
            obj->model->Draw(shader);
        }
    }
}

void MuseumObjectManager::loadDefaultObjects()
{
    // Clear existing objects
    objects.clear();
    
    // Add different museum objects in strategic positions around the room
    // Assuming the room is roughly 20x20 units
      // Object 1: Center-left - Male Sculpture (Bronze color)
    addObject("models/erkek_heykeli.glb", glm::vec3(-6.0f, 0.0f, 0.0f), 
              "Erkek Heykeli | Man Statue", "Tunç | Bronze\nRoma Dönemi | Roman Period\nMS 1. Yüzyil | 1st Century AD\nBulunma Yeri | Finding Place: Adana Karatas",              glm::vec3(0.25f, 0.15f, 0.05f),  // Bronze ambient
              glm::vec3(0.70f, 0.45f, 0.20f),  // Bronze diffuse
              glm::vec3(0.8f, 0.6f, 0.4f));    // Bronze specular
    
    // Object 2: Center-right - Tombstones with Figure (Stone color)
    addObject("models/kadın.glb", glm::vec3(6.0f, 0.0f, 0.0f), 
              "Figurlu Mezar Tasi | Tombstones with Figure", "Tas | Stone\nRoma Dönemi | Roman Period\nMS 2-3. Yüzyil | 2nd-3rd Century AD",
              glm::vec3(0.28f, 0.25f, 0.22f),  // Stone ambient
              glm::vec3(0.80f, 0.75f, 0.70f),  // Stone diffuse
              glm::vec3(0.4f, 0.4f, 0.4f));    // Stone specular
    
    // Reset female statue orientation and turn it to face opposite direction (180° Y-axis rotation)
    if (objects.size() >= 2) {
        objects[1]->rotation = glm::vec3(0.0f, 180.0f, 0.0f);
        std::cout << "Female sculpture orientation reset and rotated 180° on Y-axis" << std::endl;
    }
      // Object 3: Back-left corner - Sarcophagus of Achilles (Dark stone color)
    addObject("models/Akhilleus Lahdi.glb", glm::vec3(-6.0f, 0.0f, -6.0f), 
              "Akhilleus Lahdi | Sarcophagus of Achilles", "It is from the second group of Achilles tombs of Attica type from the Roman Imperial Period.\nThe left and short façade and its front façade are allocated to the figures.\nThere is a sphinx in the right short face of the work and opposing Gryphons on its rear long face.\nAlthough the work bears the characteristics of Late Antonines Period, it may be dated to between AD 170 and 190.",              glm::vec3(0.15f, 0.15f, 0.15f),  // Dark stone ambient
              glm::vec3(0.45f, 0.45f, 0.45f),  // Dark stone diffuse
              glm::vec3(0.2f, 0.2f, 0.2f));    // Dark stone specular
              
    // Object 4: Back-right corner - Tarhunda Sculpture with Chariot (Stone color)
    addObject("models/Arabalı Tarhunda Heykeli.glb", glm::vec3(6.0f, 0.0f, -6.0f), 
              "Arabali Tarhunda Heykeli | Tarhunta in Cart Sculpture", "Bazalt, Kalker | Basalt, Limestone\nGeç Hitit Dönemi | Late Hittite Period\nMÖ 8. Yüzyil | 8th Century BC",              glm::vec3(0.15f, 0.15f, 0.15f),  // Dark stone ambient
              glm::vec3(0.45f, 0.45f, 0.45f),  // Dark stone diffuse
              glm::vec3(0.2f, 0.2f, 0.2f));    // Dark stone specular
              
    // Object 5: Front center - Sarcophagus (Marble color)
    addObject("models/Lahit.glb", glm::vec3(0.0f, 0.0f, 6.0f), 
              "Lahit | Sarcophagus", "Mermer | Marble\nRoma Dönemi | Roman Period\nMS 3. Yüzyil | 3rd Century AD",
              glm::vec3(0.30f, 0.28f, 0.25f),  // Marble ambient
              glm::vec3(0.80f, 0.77f, 0.75f),  // Marble diffuse
              glm::vec3(0.5f, 0.5f, 0.5f));    // Marble specular
    
    // Initialize spotlight positions for all objects
    for (auto& obj : objects) {
        calculateSpotlightPosition(obj.get());
    }
    
    std::cout << "Loaded " << objects.size() << " different museum objects with realistic materials and spotlights" << std::endl;
}

std::vector<std::string> MuseumObjectManager::getObjectNames() const
{
    std::vector<std::string> names;
    for (size_t i = 0; i < objects.size(); ++i) {
        const auto& obj = objects[i];
        std::string displayName = obj->name.empty() ? ("Object " + std::to_string(i + 1)) : obj->name;
        names.push_back(displayName);
    }
    return names;
}

int MuseumObjectManager::findClosestObject(const glm::vec3& position, float maxDistance) const
{
    int closestIndex = -1;
    float closestDistance = maxDistance;
    
    for (size_t i = 0; i < objects.size(); ++i) {
        const auto& obj = objects[i];
        float distance = glm::length(position - obj->position);
        
        if (distance < closestDistance) {
            closestDistance = distance;
            closestIndex = static_cast<int>(i);
        }
    }
    
    return closestIndex;
}

void MuseumObjectManager::autoScaleObject(MuseumObject* obj, float targetSize)
{
    if (!obj || !obj->model) return;
    
    // Get the bounding box size of the model
    glm::vec3 boundingBoxSize = obj->model->GetBoundingBoxSize();
    
    // Find the largest dimension
    float maxDimension = std::max({boundingBoxSize.x, boundingBoxSize.y, boundingBoxSize.z});
    
    // Calculate scale factor to make the largest dimension equal to targetSize
    if (maxDimension > 0.0f) {
        float scaleFactor = targetSize / maxDimension;
        obj->scale = glm::vec3(scaleFactor);
        
        // Adjust Y position so the object sits on the ground
        glm::vec3 boundingBoxMin = obj->model->GetBoundingBoxMin();
        obj->position.y = -boundingBoxMin.y * scaleFactor;
        
        std::cout << "Auto-scaled object " << obj->name << " with scale factor: " << scaleFactor << std::endl;
    }
}

// Museum Object Spotlight Management Implementation
void MuseumObjectManager::updateObjectSpotlights(const glm::vec3& robotPosition, float deltaTime)
{
    int nearestObject = getNearestObjectToRobot(robotPosition, spotlightDeactivationDistance);
    
    // Check if we should change the active object
    if (nearestObject != currentActiveObject) {
        // Deactivate current spotlight if any
        if (currentActiveObject >= 0 && currentActiveObject < objects.size()) {
            objects[currentActiveObject]->spotlight.active = false;
        }
        
        // Activate new spotlight if object is close enough
        if (nearestObject >= 0) {
            float distance = glm::length(robotPosition - objects[nearestObject]->position);
            if (distance <= spotlightActivationDistance) {
                objects[nearestObject]->spotlight.active = true;
                currentActiveObject = nearestObject;
                std::cout << "Activated spotlight for: " << objects[nearestObject]->name << std::endl;
            } else {
                currentActiveObject = -1;
            }
        } else {
            currentActiveObject = -1;
        }
    }
    
    // Update spotlight intensities with smooth transitions
    for (auto& obj : objects) {
        float targetIntensity = obj->spotlight.active ? obj->spotlight.maxIntensity : 0.0f;
        
        if (obj->spotlight.intensity != targetIntensity) {
            float intensityChange = obj->spotlight.fadeSpeed * deltaTime;
            
            if (obj->spotlight.intensity < targetIntensity) {
                obj->spotlight.intensity = std::min(targetIntensity, obj->spotlight.intensity + intensityChange);
            } else {
                obj->spotlight.intensity = std::max(targetIntensity, obj->spotlight.intensity - intensityChange);
            }
        }
    }
}

void MuseumObjectManager::setObjectSpotlightActive(size_t objectIndex, bool active)
{
    if (objectIndex < objects.size()) {
        objects[objectIndex]->spotlight.active = active;
    }
}

void MuseumObjectManager::setObjectSpotlightIntensity(size_t objectIndex, float intensity)
{
    if (objectIndex < objects.size()) {
        objects[objectIndex]->spotlight.intensity = std::max(0.0f, std::min(intensity, objects[objectIndex]->spotlight.maxIntensity));
    }
}

std::vector<size_t> MuseumObjectManager::getActiveSpotlightObjects() const
{
    std::vector<size_t> activeObjects;
    for (size_t i = 0; i < objects.size(); ++i) {
        if (objects[i]->spotlight.intensity > 0.01f) {
            activeObjects.push_back(i);
        }
    }
    return activeObjects;
}

int MuseumObjectManager::getNearestObjectToRobot(const glm::vec3& robotPosition, float maxDistance) const
{
    int nearestIndex = -1;
    float nearestDistance = maxDistance;
    
    for (size_t i = 0; i < objects.size(); ++i) {
        float distance = glm::length(robotPosition - objects[i]->position);
        if (distance < nearestDistance) {
            nearestDistance = distance;
            nearestIndex = static_cast<int>(i);
        }
    }
    
    return nearestIndex;
}

std::vector<MuseumObjectManager::SpotlightData> MuseumObjectManager::getActiveSpotlights() const
{
    std::vector<SpotlightData> spotlights;
    
    for (const auto& obj : objects) {
        if (obj->spotlight.intensity > 0.01f) {
            SpotlightData data;
            data.position = obj->spotlight.position;
            data.direction = obj->spotlight.direction;
            data.color = obj->spotlight.color;
            data.intensity = obj->spotlight.intensity;
            data.cutOff = obj->spotlight.cutOff;
            data.outerCutOff = obj->spotlight.outerCutOff;
            spotlights.push_back(data);
        }
    }
    
    return spotlights;
}

void MuseumObjectManager::calculateSpotlightPosition(MuseumObject* obj)
{
    if (!obj) return;
    
    // Position spotlight above and slightly forward of the object
    glm::vec3 spotlightOffset(0.0f, 4.5f, 1.5f);
    obj->spotlight.position = obj->position + spotlightOffset;
    
    // Point the spotlight down towards the object
    obj->spotlight.direction = glm::normalize(obj->position - obj->spotlight.position);
    
    // Set object-specific spotlight colors based on object type
    if (obj->name.find("Male") != std::string::npos || obj->name.find("Female") != std::string::npos) {
        // Cool white for sculptures
        obj->spotlight.color = glm::vec3(0.9f, 0.95f, 1.0f);
    } else if (obj->name.find("Grave") != std::string::npos) {
        // Slightly blue-tinted for grave monument
        obj->spotlight.color = glm::vec3(0.8f, 0.9f, 1.0f);
    } else if (obj->name.find("Tarhunda") != std::string::npos) {
        // Warm golden for bronze objects
        obj->spotlight.color = glm::vec3(1.0f, 0.9f, 0.7f);
    } else {
        // Default warm white
        obj->spotlight.color = glm::vec3(1.0f, 0.95f, 0.85f);
    }
}
