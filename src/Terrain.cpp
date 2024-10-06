#include "Terrain.h"
#include <glm/gtc/noise.hpp>
#include <glm/gtc/type_ptr.hpp>

Terrain::Terrain(int width, int height) : width(width), height(height) {}

void Terrain::generate() {
    vertices.clear();
    indices.clear();

    for (int z = 0; z < height; z++) {
        for (int x = 0; x < width; x++) {
            float y = glm::simplex(glm::vec2(x * 0.1f, z * 0.1f)) * 1.0f;
            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            // Normal (will be calculated later)
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
            // Texture coordinates
            vertices.push_back(static_cast<float>(x) / width);
            vertices.push_back(static_cast<float>(z) / height);
        }
    }

    for (int z = 0; z < height - 1; z++) {
        for (int x = 0; x < width - 1; x++) {
            int topLeft = z * width + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * width + x;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    calculateNormals();
    setupMesh();
}

void Terrain::calculateNormals() {
    std::vector<glm::vec3> normals(width * height, glm::vec3(0.0f));

    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        glm::vec3 v0(vertices[i0 * 8], vertices[i0 * 8 + 1], vertices[i0 * 8 + 2]);
        glm::vec3 v1(vertices[i1 * 8], vertices[i1 * 8 + 1], vertices[i1 * 8 + 2]);
        glm::vec3 v2(vertices[i2 * 8], vertices[i2 * 8 + 1], vertices[i2 * 8 + 2]);

        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        normals[i0] += normal;
        normals[i1] += normal;
        normals[i2] += normal;
    }

    for (size_t i = 0; i < normals.size(); i++) {
        normals[i] = glm::normalize(normals[i]);
        vertices[i * 8 + 3] = normals[i].x;
        vertices[i * 8 + 4] = normals[i].y;
        vertices[i * 8 + 5] = normals[i].z;
    }
}

void Terrain::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Terrain::draw() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Terrain::carveRoad(const std::vector<glm::vec3>& roadPath, float roadWidth) {
    for (size_t i = 0; i < vertices.size(); i += 3) {
        glm::vec3 vertex(vertices[i], vertices[i + 1], vertices[i + 2]);
        float minDistance = std::numeric_limits<float>::max();

        for (size_t j = 0; j < roadPath.size() - 1; j++) {
            glm::vec3 roadStart = roadPath[j];
            glm::vec3 roadEnd = roadPath[j + 1];

            float distance = glm::length(glm::cross(roadEnd - roadStart, vertex - roadStart)) / glm::length(roadEnd - roadStart);
            minDistance = std::min(minDistance, distance);
        }

        if (minDistance < roadWidth / 2.0f) {
            vertices[i + 1] -= 0.5f * (1.0f - minDistance / (roadWidth / 2.0f)); // Lower the vertex
        }
    }

    calculateNormals(); // Recalculate normals after modifying the terrain

    // Update the terrain's vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
}

float Terrain::getHeightAt(float x, float z) const {
    int gridX = static_cast<int>(x);
    int gridZ = static_cast<int>(z);

    gridX = std::max(0, std::min(gridX, width - 1));
    gridZ = std::max(0, std::min(gridZ, height - 1));

    return vertices[(gridZ * width + gridX) * 8 + 1];
}