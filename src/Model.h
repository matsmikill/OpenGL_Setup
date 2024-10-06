#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Texture.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

class Model {
public:
    Model(const std::string& objPath, const std::string& texturePath);
    ~Model();

    void Draw() const;

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;
    Texture texture;

    void loadModel(const std::string& objPath);
    void setupMesh();
};

#endif // MODEL_H
