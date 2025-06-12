#ifndef MUSEUM_ROOM_H
#define MUSEUM_ROOM_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class MuseumRoom {
public:
    MuseumRoom();
    ~MuseumRoom();
    
    void setupRoom();
    void render();
    
private:
    unsigned int VAO, VBO;
    std::vector<float> vertices;
    
    void generateRoomGeometry();
};

#endif
