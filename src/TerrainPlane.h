#ifndef TERRAIN_PLANE_H
#define TERRAIN_PLANE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class TerrainPlane {
private:
    GLuint VAO, VBO, EBO;
    int width, height;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    void generateTerrain();
    void setupBuffers();

public:
    TerrainPlane(int width, int height);
    ~TerrainPlane();

    void render();
};

#endif // TERRAIN_PLANE_H
