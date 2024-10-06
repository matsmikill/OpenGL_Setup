#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Terrain {
public:
    Terrain(int width, int height);
    void generate();
    void draw();
    void carveRoad(const std::vector<glm::vec3>& roadPath, float roadWidth);
    float getHeightAt(float x, float z) const;

private:
    int width, height;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;

    void setupMesh();
    void calculateNormals();
};
