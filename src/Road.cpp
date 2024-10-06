#include "Road.h"
#include "Terrain.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

Road::Road(float width) : width(width), VAO(0), VBO(0) {}

void Road::addPoint(const glm::vec3& point, const Terrain& terrain) {
    float height = terrain.getHeightAt(point.x, point.z);
    path.push_back(glm::vec3(point.x, height, point.z));
}

void Road::generateVertices() {
    vertices.clear();
    for (size_t i = 0; i < path.size(); i++) {
        glm::vec3 current = path[i];
        glm::vec3 next = (i < path.size() - 1) ? path[i + 1] : current;

        for (float t = 0; t <= 1.0f; t += 0.1f) {
            glm::vec3 point = current * (1.0f - t) + next * t;
            vertices.push_back(point.x);
            vertices.push_back(point.y);
            vertices.push_back(point.z);
        }
    }
}

void Road::setupBuffers() {
    generateVertices();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Road::render() const {
    if (VAO == 0) return; // Ensure buffers are set up

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_STRIP, 0, vertices.size() / 3);
    glBindVertexArray(0);
}

const std::vector<glm::vec3>& Road::getPath() const {
    return path;
}

float Road::getWidth() const {
    return width;
}

