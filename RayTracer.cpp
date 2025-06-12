#include "RayTracer.h"
#include <random>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

RayTracer::RayTracer() {
    // Initialize with default scene
    // Add a floor plane
    RayTracingMaterial floorMaterial;
    floorMaterial.albedo = glm::vec3(0.7f, 0.7f, 0.7f);
    floorMaterial.roughness = 0.8f;
    addPlane(glm::vec3(0, -0.1f, 0), glm::vec3(0, 1, 0), floorMaterial);
    
    // Add default lighting
    addLight(glm::vec3(0, 10, 0), glm::vec3(1, 1, 1), 1.0f);
    addLight(glm::vec3(5, 5, 5), glm::vec3(0.8f, 0.9f, 1.0f), 0.7f);
}

RayTracer::~RayTracer() {
    // Cleanup if needed
}

glm::vec3 RayTracer::traceRay(const Ray& ray, int depth) const {
    if (depth >= maxDepth) {
        return backgroundColor;
    }
    
    HitRecord record;
    if (hit(ray, record)) {
        glm::vec3 color = calculateLighting(record);
        
        // Add reflections
        if (record.reflectance > 0.0f) {
            glm::vec3 reflection = calculateReflection(ray, record, depth);
            color = glm::mix(color, reflection, record.reflectance);
        }
        
        // Add refractions for transparent objects
        if (record.transparency > 0.0f) {
            glm::vec3 refraction = calculateRefraction(ray, record, depth);
            color = glm::mix(color, refraction, record.transparency);
        }
        
        // Global illumination
        if (globalIllumination) {
            glm::vec3 gi = calculateGlobalIllumination(record, depth);
            color += gi * 0.3f;
        }
        
        return color;
    }
    
    return backgroundColor;
}

bool RayTracer::hit(const Ray& ray, HitRecord& record) const {
    bool hitAnything = false;
    float closestSoFar = ray.tMax;
    HitRecord tempRecord;
    
    // Check spheres
    for (const auto& sphere : spheres) {
        if (hitSphere(sphere, ray, tempRecord) && tempRecord.t < closestSoFar) {
            hitAnything = true;
            closestSoFar = tempRecord.t;
            record = tempRecord;
        }
    }
    
    // Check planes
    for (const auto& plane : planes) {
        if (hitPlane(plane, ray, tempRecord) && tempRecord.t < closestSoFar) {
            hitAnything = true;
            closestSoFar = tempRecord.t;
            record = tempRecord;
        }
    }
    
    // Check museum objects (simplified as bounding spheres)
    if (scene) {
        for (size_t i = 0; i < scene->getObjectCount(); ++i) {
            const MuseumObject* obj = scene->getObject(i);
            if (obj && hitMuseumObject(obj, ray, tempRecord) && tempRecord.t < closestSoFar) {
                hitAnything = true;
                closestSoFar = tempRecord.t;
                record = tempRecord;
                record.objectIndex = static_cast<int>(i);
            }
        }
    }
    
    return hitAnything;
}

void RayTracer::setScene(const MuseumObjectManager* objectManager) {
    scene = objectManager;
}

void RayTracer::addSphere(const glm::vec3& center, float radius, const RayTracingMaterial& material) {
    spheres.push_back({center, radius, material});
}

void RayTracer::addPlane(const glm::vec3& point, const glm::vec3& normal, const RayTracingMaterial& material) {
    planes.push_back({point, glm::normalize(normal), material});
}

void RayTracer::addLight(const glm::vec3& position, const glm::vec3& color, float intensity) {
    lights.push_back({position, color, intensity});
}

void RayTracer::clearLights() {
    lights.clear();
}

glm::vec3 RayTracer::calculateReflection(const Ray& ray, const HitRecord& hit, int depth) const {
    glm::vec3 reflected = glm::reflect(ray.direction, hit.normal);
    
    // Add some roughness to reflection
    if (hit.reflectance < 1.0f) {
        glm::vec3 randomDir = randomInUnitSphere() * (1.0f - hit.reflectance);
        reflected = glm::normalize(reflected + randomDir);
    }
    
    Ray reflectedRay(hit.point, reflected);
    return traceRay(reflectedRay, depth + 1);
}

glm::vec3 RayTracer::calculateRefraction(const Ray& ray, const HitRecord& hit, int depth) const {
    float refractiveIndex = 1.5f; // Default glass
    float etaRatio = hit.frontFace ? (1.0f / refractiveIndex) : refractiveIndex;
    
    float cosTheta = std::min(glm::dot(-ray.direction, hit.normal), 1.0f);
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    
    bool cannotRefract = etaRatio * sinTheta > 1.0f;
    glm::vec3 direction;
    
    if (cannotRefract || calculateFresnel(-ray.direction, hit.normal, 1.0f, refractiveIndex) > random01()) {
        direction = glm::reflect(ray.direction, hit.normal);
    } else {
        direction = glm::refract(ray.direction, hit.normal, etaRatio);
    }
    
    Ray refractedRay(hit.point, direction);
    return traceRay(refractedRay, depth + 1);
}

glm::vec3 RayTracer::calculateGlobalIllumination(const HitRecord& hit, int depth) const {
    if (depth >= maxDepth - 2) return glm::vec3(0.0f);
    
    glm::vec3 color(0.0f);
    int samples = std::max(1, sampleCount / (depth + 1));
    
    for (int i = 0; i < samples; ++i) {
        glm::vec3 randomDir = sampleHemisphere(hit.normal);
        Ray giRay(hit.point, randomDir);
        color += traceRay(giRay, depth + 1) * glm::dot(hit.normal, randomDir);
    }
    
    return color / float(samples);
}

glm::vec3 RayTracer::sampleHemisphere(const glm::vec3& normal) const {
    glm::vec3 randomDir = randomUnitVector();
    if (glm::dot(randomDir, normal) < 0.0f) {
        randomDir = -randomDir;
    }
    return randomDir;
}

float RayTracer::calculateFresnel(const glm::vec3& incident, const glm::vec3& normal, float n1, float n2) const {
    float cosI = -glm::dot(incident, normal);
    float sinT2 = (n1 / n2) * (n1 / n2) * (1.0f - cosI * cosI);
    
    if (sinT2 > 1.0f) return 1.0f; // Total internal reflection
    
    float cosT = sqrt(1.0f - sinT2);
    float r0rth = (n1 * cosI - n2 * cosT) / (n1 * cosI + n2 * cosT);
    float rPar = (n2 * cosI - n1 * cosT) / (n2 * cosI + n1 * cosT);
    
    return (r0rth * r0rth + rPar * rPar) / 2.0f;
}

bool RayTracer::hitSphere(const Sphere& sphere, const Ray& ray, HitRecord& record) const {
    glm::vec3 oc = ray.origin - sphere.center;
    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0f * glm::dot(oc, ray.direction);
    float c = glm::dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4 * a * c;
    
    if (discriminant < 0) return false;
    
    float sqrtd = sqrt(discriminant);
    float root = (-b - sqrtd) / (2.0f * a);
    
    if (root < ray.tMin || ray.tMax < root) {
        root = (-b + sqrtd) / (2.0f * a);
        if (root < ray.tMin || ray.tMax < root) {
            return false;
        }
    }
    
    record.t = root;
    record.point = ray.at(record.t);
    glm::vec3 outwardNormal = (record.point - sphere.center) / sphere.radius;
    record.setFaceNormal(ray, outwardNormal);
    record.color = sphere.material.albedo;
    record.reflectance = sphere.material.metallic;
    record.transparency = sphere.material.transparency;
    
    return true;
}

bool RayTracer::hitPlane(const Plane& plane, const Ray& ray, HitRecord& record) const {
    float denom = glm::dot(plane.normal, ray.direction);
    if (abs(denom) < 1e-6) return false; // Ray parallel to plane
    
    float t = glm::dot(plane.point - ray.origin, plane.normal) / denom;
    if (t < ray.tMin || t > ray.tMax) return false;
    
    record.t = t;
    record.point = ray.at(t);
    record.setFaceNormal(ray, plane.normal);
    record.color = plane.material.albedo;
    record.reflectance = plane.material.metallic;
    record.transparency = plane.material.transparency;
    
    return true;
}

bool RayTracer::hitMuseumObject(const MuseumObject* obj, const Ray& ray, HitRecord& record) const {
    // Simplified collision as bounding sphere
    float radius = 1.5f; // Approximate object size
    glm::vec3 oc = ray.origin - obj->position;
    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0f * glm::dot(oc, ray.direction);
    float c = glm::dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    
    if (discriminant < 0) return false;
    
    float sqrtd = sqrt(discriminant);
    float root = (-b - sqrtd) / (2.0f * a);
    
    if (root < ray.tMin || ray.tMax < root) {
        root = (-b + sqrtd) / (2.0f * a);
        if (root < ray.tMin || ray.tMax < root) {
            return false;
        }
    }
    
    record.t = root;
    record.point = ray.at(record.t);
    glm::vec3 outwardNormal = glm::normalize(record.point - obj->position);
    record.setFaceNormal(ray, outwardNormal);
    record.color = obj->materialDiffuse;
    record.reflectance = 0.3f; // Museum objects have some reflectance
    record.transparency = 0.0f;
    
    return true;
}

glm::vec3 RayTracer::calculateLighting(const HitRecord& hit) const {
    glm::vec3 color = hit.color * 0.1f; // Ambient
    
    for (const auto& light : lights) {
        glm::vec3 lightDir = glm::normalize(light.position - hit.point);
        float distance = glm::length(light.position - hit.point);
        float attenuation = 1.0f / (1.0f + 0.1f * distance + 0.01f * distance * distance);
          // Check for shadows
        Ray shadowRay(hit.point + hit.normal * 0.001f, lightDir);
        shadowRay.tMax = distance - 0.001f;
        HitRecord shadowHit;
        bool inShadow = this->hit(shadowRay, shadowHit);
        
        if (!inShadow) {
            // Diffuse
            float diff = std::max(glm::dot(hit.normal, lightDir), 0.0f);
            color += hit.color * light.color * light.intensity * diff * attenuation;
        }
    }
    
    return color;
}

glm::vec3 RayTracer::randomInUnitSphere() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
    
    glm::vec3 p;
    do {
        p = glm::vec3(dis(gen), dis(gen), dis(gen));
    } while (glm::length(p) >= 1.0f);
    
    return p;
}

glm::vec3 RayTracer::randomUnitVector() const {
    return glm::normalize(randomInUnitSphere());
}

float RayTracer::random01() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    return dis(gen);
}
