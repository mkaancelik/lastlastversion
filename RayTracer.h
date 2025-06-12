#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include "MuseumObjectManager.h"

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
    float tMin = 0.001f;
    float tMax = 1000.0f;
    
    Ray(const glm::vec3& o, const glm::vec3& d) : origin(o), direction(glm::normalize(d)) {}
    
    glm::vec3 at(float t) const {
        return origin + t * direction;
    }
};

struct HitRecord {
    glm::vec3 point;
    glm::vec3 normal;
    float t;
    bool frontFace;
    glm::vec3 color;
    float reflectance = 0.0f;
    float transparency = 0.0f;
    int objectIndex = -1;
    
    void setFaceNormal(const Ray& ray, const glm::vec3& outwardNormal) {
        frontFace = glm::dot(ray.direction, outwardNormal) < 0;
        normal = frontFace ? outwardNormal : -outwardNormal;
    }
};

struct RayTracingMaterial {
    glm::vec3 albedo = glm::vec3(0.8f);
    float metallic = 0.0f;
    float roughness = 0.5f;
    float transparency = 0.0f;
    float refractiveIndex = 1.0f;
    glm::vec3 emission = glm::vec3(0.0f);
};

class RayTracer {
public:
    RayTracer();
    ~RayTracer();
    
    // Main ray tracing functions
    glm::vec3 traceRay(const Ray& ray, int depth = 0) const;
    bool hit(const Ray& ray, HitRecord& record) const;
    
    // Scene setup
    void setScene(const MuseumObjectManager* objectManager);
    void addSphere(const glm::vec3& center, float radius, const RayTracingMaterial& material);
    void addPlane(const glm::vec3& point, const glm::vec3& normal, const RayTracingMaterial& material);
    
    // Ray tracing settings
    void setMaxDepth(int depth) { maxDepth = depth; }
    void setBackgroundColor(const glm::vec3& color) { backgroundColor = color; }
    void enableGlobalIllumination(bool enable) { globalIllumination = enable; }
    void setSampleCount(int samples) { sampleCount = samples; }
    
    // Lighting
    void addLight(const glm::vec3& position, const glm::vec3& color, float intensity);
    void clearLights();
    
    // Reflections and refractions
    glm::vec3 calculateReflection(const Ray& ray, const HitRecord& hit, int depth) const;
    glm::vec3 calculateRefraction(const Ray& ray, const HitRecord& hit, int depth) const;
    
    // Advanced features
    glm::vec3 calculateGlobalIllumination(const HitRecord& hit, int depth) const;
    glm::vec3 sampleHemisphere(const glm::vec3& normal) const;
    float calculateFresnel(const glm::vec3& incident, const glm::vec3& normal, float n1, float n2) const;
    
private:
    struct Sphere {
        glm::vec3 center;
        float radius;
        RayTracingMaterial material;
    };
    
    struct Plane {
        glm::vec3 point;
        glm::vec3 normal;
        RayTracingMaterial material;
    };
    
    struct Light {
        glm::vec3 position;
        glm::vec3 color;
        float intensity;
    };
    
    // Scene objects
    std::vector<Sphere> spheres;
    std::vector<Plane> planes;
    std::vector<Light> lights;
    const MuseumObjectManager* scene = nullptr;
    
    // Ray tracing parameters
    int maxDepth = 10;
    int sampleCount = 4;
    glm::vec3 backgroundColor = glm::vec3(0.1f, 0.1f, 0.2f);
    bool globalIllumination = false;
    
    // Helper functions
    bool hitSphere(const Sphere& sphere, const Ray& ray, HitRecord& record) const;
    bool hitPlane(const Plane& plane, const Ray& ray, HitRecord& record) const;
    bool hitMuseumObject(const MuseumObject* obj, const Ray& ray, HitRecord& record) const;
    glm::vec3 calculateLighting(const HitRecord& hit) const;
    glm::vec3 randomInUnitSphere() const;
    glm::vec3 randomUnitVector() const;
    float random01() const;
};
