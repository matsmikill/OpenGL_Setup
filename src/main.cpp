#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "tiny_obj_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Camera.h"
#include "Model.h"
#include "Shader.h"
#include "BloomEffect.h"

#include "Terrain.h"
//#include "Road.h"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Global variables or constants
glm::vec3 lightPos(5.0f, 5.0f, 5.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 objectColor(0.5f, 0.5f, 0.5f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//void processInput(GLFWwindow* window);

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 400, lastY = 300;
bool firstMouse = true;

void processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(0, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(1, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(2, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(3, deltaTime);
}

bool rightMousePressed = false;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (rightMousePressed) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
        lastX = xpos;
        lastY = ypos;
        camera.processMouseMovement(xoffset, yoffset);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            rightMousePressed = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;  // Reset firstMouse when right-click is pressed
        }
        else if (action == GLFW_RELEASE) {
            rightMousePressed = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

// settings
const unsigned int SCR_WIDTH = 1100;
const unsigned int SCR_HEIGHT = 900;

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

    bool autoRotate = false;
    bool rotationDirectionX = true;

    bool wireframeMode = false;
    bool wireframeModePoints = false;
    float pointSize = 0.0f;

    bool colorCycle = false;

    float terrainSize = 10.0f;

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

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    Terrain terrain(terrainSize, terrainSize);
    terrain.generate();

    // Create shader program
    Shader shader("Shaders/shader.vert", "Shaders/shader.frag");

    Shader bloomShader("Shaders/bloom.vert", "Shaders/bloom.frag");
    Shader blurShader("Shaders/blur.vert", "Shaders/blur.frag");
    Shader finalShader("Shaders/final.vert", "Shaders/final.frag");

    Texture terrainTexture("Textures/Grass.png");

    // Load models
    std::vector<Model> models;
    models.emplace_back("3D_Models/Back.obj", "Textures/Back.png");
    //models.emplace_back("3D_Models/Horn.obj", "Textures/Horn_Texture.png");
    // Add more models as needed

    // Initialize matrices
    glm::mat4 model = glm::mat4(1.0f); // Identity matrix
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f)); // Camera view
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); // Projection

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330"); 

    float color[4] = { 0.8f, 0.3f, 0.02f, 1.0f };
    bool drawModel = true;

    // uncomment this call to draw in wireframe polygons.
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    glEnable(GL_PROGRAM_POINT_SIZE);

    //-----------------------------------------------------------------------------------------

    glEnable(GL_DEPTH_TEST);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {

        // Calculate delta time for smooth animation
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window, deltaTime);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        // Use your main shader for rendering
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("lightColor", lightColor);
        shader.setVec3("objectColor", objectColor);
        shader.setBool("useTexture", true);

        // Render models
        for (const auto& model : models)
        {
            glm::mat4 modelMatrix = glm::mat4(1.0f);

            modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(rotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(scale, scale, scale));

            shader.setMat4("model", modelMatrix);
            model.Draw();
        }

        // Render terrain
        terrainTexture.bind(0);
        shader.setInt("texture1", 0);
        shader.setMat4("model", glm::mat4(1.0f));
        terrain.draw();

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

        // Apply wireframe mode if enabled
        if (wireframeModePoints) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glPointSize(pointSize);
        }
        else if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);   
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
        ImGui::Checkbox("Wireframe_Points", &wireframeModePoints);
        ImGui::SliderFloat("Point_Size", &pointSize, 0.0f, 100.0f);
        ImGui::SliderFloat("Rotation X", &rotationX, 0.0f, 360.0f); // Rotation around X
        ImGui::SliderFloat("Rotation Y", &rotationY, 0.0f, 360.0f); // Rotation around Y
        ImGui::SliderFloat("Rotation Z", &rotationZ, 0.0f, 360.0f); // Rotation around Z
        ImGui::Checkbox("autoRotate", &autoRotate);
        ImGui::Checkbox("colorCycle", &colorCycle);
        ImGui::End();

        ImGui::Begin("Terrain");
        ImGui::SliderFloat("Size", &terrainSize, 10.0f, 300.0f);
        ImGui::End();

        ImGui::Begin("Bloom Debug");
        static float bloomThreshold = 1.0f;
        static float bloomIntensity = 1.0f;
        ImGui::SliderFloat("Bloom Threshold", &bloomThreshold, 0.0f, 5.0f);
        ImGui::SliderFloat("Bloom Intensity", &bloomIntensity, 0.0f, 2.0f);
        ImGui::End();

        if (terrainSize) {
            terrain = Terrain(terrainSize, terrainSize);
            terrain.generate();
        }
     
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

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