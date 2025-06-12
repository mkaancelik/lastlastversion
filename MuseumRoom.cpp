#include "MuseumRoom.h"

MuseumRoom::MuseumRoom() {
    setupRoom();
}

MuseumRoom::~MuseumRoom() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void MuseumRoom::setupRoom() {
    generateRoomGeometry();
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(vertices.size() * sizeof(float)), &vertices[0], GL_STATIC_DRAW);
    
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

void MuseumRoom::render() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / 8));
    glBindVertexArray(0);
}

void MuseumRoom::generateRoomGeometry() {
    vertices.clear();
    
    // Room dimensions
    float width = 20.0f;
    float height = 8.0f;
    float depth = 20.0f;
    
    // Floor (y = 0)
    // Triangle 1
    vertices.insert(vertices.end(), {
        -width/2, 0.0f, -depth/2,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,  // Bottom-left
         width/2, 0.0f, -depth/2,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,  // Bottom-right
         width/2, 0.0f,  depth/2,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f   // Top-right
    });
    // Triangle 2
    vertices.insert(vertices.end(), {
         width/2, 0.0f,  depth/2,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,  // Top-right
        -width/2, 0.0f,  depth/2,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,  // Top-left
        -width/2, 0.0f, -depth/2,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f   // Bottom-left
    });
    
    // Ceiling (y = height)
    // Triangle 1
    vertices.insert(vertices.end(), {
        -width/2, height, -depth/2,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,  // Bottom-left
         width/2, height,  depth/2,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,  // Top-right
         width/2, height, -depth/2,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f   // Bottom-right
    });
    // Triangle 2
    vertices.insert(vertices.end(), {
         width/2, height,  depth/2,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,  // Top-right
        -width/2, height, -depth/2,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,  // Bottom-left
        -width/2, height,  depth/2,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f   // Top-left
    });
    
    // Front wall (z = depth/2)
    // Triangle 1
    vertices.insert(vertices.end(), {
        -width/2, 0.0f, depth/2,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,  // Bottom-left
         width/2, 0.0f, depth/2,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,  // Bottom-right
         width/2, height, depth/2,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f   // Top-right
    });
    // Triangle 2
    vertices.insert(vertices.end(), {
         width/2, height, depth/2,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,  // Top-right
        -width/2, height, depth/2,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,  // Top-left
        -width/2, 0.0f, depth/2,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f   // Bottom-left
    });
    
    // Back wall (z = -depth/2)
    // Triangle 1
    vertices.insert(vertices.end(), {
        -width/2, 0.0f, -depth/2,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,  // Bottom-left
         width/2, height, -depth/2,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // Top-right
         width/2, 0.0f, -depth/2,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f   // Bottom-right
    });
    // Triangle 2
    vertices.insert(vertices.end(), {
         width/2, height, -depth/2,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,  // Top-right
        -width/2, 0.0f, -depth/2,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,  // Bottom-left
        -width/2, height, -depth/2,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f   // Top-left
    });
    
    // Left wall (x = -width/2)
    // Triangle 1
    vertices.insert(vertices.end(), {
        -width/2, 0.0f, -depth/2,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,  // Bottom-left
        -width/2, 0.0f,  depth/2,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,  // Bottom-right
        -width/2, height,  depth/2,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f   // Top-right
    });
    // Triangle 2
    vertices.insert(vertices.end(), {
        -width/2, height,  depth/2,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,  // Top-right
        -width/2, height, -depth/2,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,  // Top-left
        -width/2, 0.0f, -depth/2,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f   // Bottom-left
    });
    
    // Right wall (x = width/2)
    // Triangle 1
    vertices.insert(vertices.end(), {
         width/2, 0.0f, -depth/2,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,  // Bottom-left
         width/2, height,  depth/2,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,  // Top-right
         width/2, 0.0f,  depth/2,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f   // Bottom-right
    });
    // Triangle 2
    vertices.insert(vertices.end(), {
         width/2, height,  depth/2,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,  // Top-right
         width/2, 0.0f, -depth/2,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,  // Bottom-left
         width/2, height, -depth/2,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f   // Top-left
    });
}
