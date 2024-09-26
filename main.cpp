#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include "tiny_obj_loader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Declare the struct to hold vertex data including texture coordinates
struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoord;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char* vertexShaderSourceWithTexture = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec2 aTexCoord;\n"
"out vec2 TexCoord;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"    TexCoord = aTexCoord;\n"
"}\0";

const char* fragmentShaderSourceWithTexture = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 TexCoord;\n"
"uniform sampler2D texture1;\n"
"void main()\n"
"{\n"
"    FragColor = texture(texture1, TexCoord);\n"  
"}\0";

// Function to load OBJ file using tinyobjloader
bool LoadOBJ(const std::string& inputfile, std::vector<Vertex>& vertices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // Load the OBJ file
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputfile.c_str());
    if (!warn.empty()) {
        std::cout << "Warning: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "Error: " << err << std::endl;
    }
    if (!ret) {
        std::cerr << "Failed to load OBJ file: " << inputfile << std::endl;
        return false;
    }

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces (polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];  // Number of vertices per face (usually 3 for triangles)

            // Loop over vertices in the face
            for (size_t v = 0; v < fv; v++) {
                // Access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                float vx = attrib.vertices[3 * idx.vertex_index + 0];
                float vy = attrib.vertices[3 * idx.vertex_index + 1];
                float vz = attrib.vertices[3 * idx.vertex_index + 2];

                // Access to texture coordinate
                float tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                float ty = 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1];

                // Create a vertex and assign position and texture coordinates
                Vertex vertex;
                vertex.position = glm::vec3(vx, vy, vz);  // Set the vertex position
                vertex.texCoord = glm::vec2(tx, ty);      // Set the texture coordinates

                // Add the vertex to the vector
                vertices.push_back(vertex);
            }
            index_offset += fv;
        }
    }
    return true;
}

int main()
{
    // Declare the scale variable
    float scale = 1.0f;

    // Rotation angles in degrees
    float rotationX = 0.0f;
    float rotationY = 0.0f;
    float rotationZ = 0.0f;

    // Rotation speeds in degrees per second
    float rotationSpeedX = 50.0f;  // X-axis
    float rotationSpeedY = 30.0f;  // Y-axis
    float rotationSpeedZ = 10.0f;  // Z-axis

    bool autoRotate = true;
    bool rotationDirectionX = true;

    // Time tracking
    float lastFrame = 0.0f;  // Time of the last frame


    
    bool wireframeMode = false;

    bool colorCycle = false;


    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    // build and compile our shader program
    // ------------------------------------
    // For the vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSourceWithTexture, NULL);  // Use texture-enabled vertex shader
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // For the fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceWithTexture, NULL);  // Use texture-enabled fragment shader
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Load OBJ file and extract vertex positions
    std::vector<Vertex> Vertices;
    if (!LoadOBJ("3D_Models/Back.obj", Vertices)) {
        std::cerr << "Failed to load model." << std::endl;
        return -1;
    }

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // Positions         // Texture coords
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,  // Bottom-left
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,  // Bottom-right
         0.0f,  0.5f, 0.0f,  0.5f, 1.0f   // Top
    };

    unsigned int objVBO, objVAO;
    glGenVertexArrays(1, &objVAO);
    glGenBuffers(1, &objVBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(objVAO);

    glBindBuffer(GL_ARRAY_BUFFER, objVBO);
    glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(Vertex), &Vertices[0], GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330"); 

    // Initialize matrices
    glm::mat4 model = glm::mat4(1.0f); // Identity matrix
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f)); // Camera view
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); // Projection


    float color[4] = { 0.8f, 0.3f, 0.02f, 1.0f };
    bool drawModel = true;


    // uncomment this call to draw in wireframe polygons.
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //-----------------------------------------------------------------------------------------
    // Load and create the texture
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load the image data
    int width, height, nrChannels;
    unsigned char* data = stbi_load("Textures/Back.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        // Create the OpenGL texture from the loaded image data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    glEnable(GL_DEPTH_TEST);
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Calculate delta time for smooth animation
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (autoRotate) {

            // Rotation on X-axis
            if (rotationX >= 360.0f) {
                rotationDirectionX = false; // Reverse direction to decreasing
            }
            else if (rotationX <= 0.0f) {
                rotationDirectionX = true;  // Reverse direction to increasing
            }

            // Automatically update rotation angles based on delta time and rotation speed
            rotationX += (rotationDirectionX ? 1 : -1) * rotationSpeedX * deltaTime;
            rotationY += rotationSpeedY * deltaTime;
            rotationZ += rotationSpeedZ * deltaTime;
        }

        if (colorCycle) {
            float time = glfwGetTime();
            color[0] = (sin(time * 0.5f) + 1.0f) / 2.0f;  // Red value
            color[1] = (sin(time * 0.7f) + 1.0f) / 2.0f;  // Green value
            color[2] = (sin(time * 1.0f) + 1.0f) / 2.0f;  // Blue value
        }

        // Update model matrix (scale based on user input)
        model = glm::mat4(1.0f); // Reset to identity
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Optional: translate if needed
        model = glm::rotate(model, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X
        model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y
        model = glm::rotate(model, glm::radians(rotationZ), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z
        model = glm::scale(model, glm::vec3(scale, scale, scale)); // Scale

        // Apply wireframe mode if enabled
        if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Enable wireframe
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // Default mode
        }


        // Set the uniform values in the shader
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


        // Draw OBJ model
        if (drawModel) {
            glUseProgram(shaderProgram);
            glUniform4f(glGetUniformLocation(shaderProgram, "color"), color[0], color[1], color[2], color[3]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
            glBindVertexArray(objVAO);
            glDrawArrays(GL_TRIANGLES, 0, Vertices.size());
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui menu
        ImGui::Begin("window");
        ImGui::Text("Test");
        ImGui::Checkbox("Draw Model", &drawModel);
        ImGui::ColorEdit4("Color", color);
        ImGui::SliderFloat("size", &scale, 0.1f, 3.0f);
        ImGui::Checkbox("Wireframe", &wireframeMode);
        ImGui::SliderFloat("Rotation X", &rotationX, 0.0f, 360.0f); // Rotation around X
        ImGui::SliderFloat("Rotation Y", &rotationY, 0.0f, 360.0f); // Rotation around Y
        ImGui::SliderFloat("Rotation Z", &rotationZ, 0.0f, 360.0f); // Rotation around Z
        ImGui::Checkbox("autoRotate", &autoRotate);
        ImGui::Checkbox("colorCycle", &colorCycle);
        // Debug print to check values
        std::cout << "Rotation X: " << rotationX << " Y: " << rotationY << " Z: " << rotationZ << std::endl;
        ImGui::End();

        glUseProgram(shaderProgram);
        glUniform4f(glGetUniformLocation(shaderProgram, "color"), color[0], color[1], color[2], color[3]);
        glUniform1f(glGetUniformLocation(shaderProgram, "scale"), scale);
       
        // Draw model if enabled
        if (drawModel) {
            glBindVertexArray(objVAO);
            glDrawArrays(GL_TRIANGLES, 0, Vertices.size() / 3);
        }


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // glBindVertexArray(0); // no need to unbind it every time 

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &objVAO);
    glDeleteBuffers(1, &objVBO);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}