#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);

class Model
{
public:
    // Model data
    std::vector<Texture> textures_loaded; // Stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::vector<Mesh>    meshes;
    std::string directory;
    bool gammaCorrection;

    // Constructor, expects a filepath to a 3D model.
    Model(std::string const& path, bool gamma = false);

    // Draws the model, and thus all its meshes
    void Draw(Shader& shader);

    // Get model bounding box for positioning
    glm::vec3 GetBoundingBoxMin() const { return boundingBoxMin; }
    glm::vec3 GetBoundingBoxMax() const { return boundingBoxMax; }
    glm::vec3 GetBoundingBoxCenter() const { return (boundingBoxMin + boundingBoxMax) * 0.5f; }
    glm::vec3 GetBoundingBoxSize() const { return boundingBoxMax - boundingBoxMin; }

private:
    // Bounding box for the model
    glm::vec3 boundingBoxMin;
    glm::vec3 boundingBoxMax;

    // Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(std::string const& path);

    // Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene);

    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    // Checks all material textures of a given type and loads the textures if they're not loaded yet.
    // The required info is returned as a Texture struct.
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

    // Update bounding box with vertex position
    void updateBoundingBox(const glm::vec3& position);
};
