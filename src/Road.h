#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

class Terrain;

class Road {
public:
    Road(float width);
    void addPoint(const glm::vec3& point, const Terrain& terrain);
    void setupBuffers();
    void render() const;
    const std::vector<glm::vec3>& getPath() const;
    float getWidth() const;

private:
    std::vector<glm::vec3> path;
    std::vector<float> vertices;
    float width;
    GLuint VAO, VBO, EBO;
    void generateVertices();
};