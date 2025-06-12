#define _CRT_SECURE_NO_WARNINGS


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Add ImGui headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Add OpenGL Mathematics headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Add our custom headers
#include "Shader.h"
#include "Camera.h"
#include "MuseumRoom.h"
#include "MuseumObjectManager.h"
#include "MobileRobot.h"
#include "RayTracer.h"

// Global variables for camera and input
Camera camera(glm::vec3(0.0f, 3.0f, 5.0f));
float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Mouse capture state
bool mouseCaptured = true;
bool mKeyPressed = false;

// Callback function to adjust the viewport when the window size changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Mouse callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!mouseCaptured) return; // Don't process mouse movement if not captured
    
    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top

    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// Scroll callback
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// Process keyboard input
void processInput(GLFWwindow *window) {
    // Close window on pressing ESC
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // Toggle mouse capture with M key
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !mKeyPressed) {
        mouseCaptured = !mouseCaptured;
        if (mouseCaptured) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true; // Reset first mouse to avoid camera jump
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        mKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) {
        mKeyPressed = false;
    }
    
    // Camera movement (only when mouse is captured)
    if (mouseCaptured) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    
    // Configure GLFW: OpenGL version 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Virtual Museum Assignment", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }    glfwMakeContextCurrent(window);
    
    // Set callback for window resize events
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    // Tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLAD to load OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }
    
    // Set the viewport dimensions
    glViewport(0, 0, 800, 600);
    
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
      // Enable depth testing for 3D rendering
    glEnable(GL_DEPTH_TEST);
    
    // Create shader program
    Shader ourShader("shader.vert", "shader.frag");
      // Create museum room
    MuseumRoom room;
      // Create museum object manager and load default objects
    MuseumObjectManager objectManager;
    objectManager.loadDefaultObjects();    // Create mobile robot
    MobileRobot robot;
    
    // Create and initialize ray tracer
    RayTracer rayTracer;
    rayTracer.setScene(&objectManager);
    rayTracer.setMaxDepth(10);
    rayTracer.setSampleCount(4);
    rayTracer.setBackgroundColor(glm::vec3(0.1f, 0.1f, 0.2f));
    
    // Add some reflective spheres for ray tracing demonstration
    RayTracingMaterial glassMaterial;
    glassMaterial.albedo = glm::vec3(0.9f, 0.9f, 1.0f);
    glassMaterial.transparency = 0.8f;
    glassMaterial.refractiveIndex = 1.5f;
    rayTracer.addSphere(glm::vec3(3.0f, 2.0f, 3.0f), 0.8f, glassMaterial);
    
    RayTracingMaterial metalMaterial;
    metalMaterial.albedo = glm::vec3(0.7f, 0.7f, 0.8f);
    metalMaterial.metallic = 0.9f;
    metalMaterial.roughness = 0.1f;
    rayTracer.addSphere(glm::vec3(-3.0f, 2.0f, -3.0f), 0.8f, metalMaterial);
    
    // Set camera boundaries to keep it inside the museum room
    // Room dimensions: 20x8x20 (width x height x depth)
    // Add small margins to prevent camera from going through walls
    float roomMargin = 0.5f;
    camera.SetRoomBoundaries(
        -10.0f + roomMargin,  // minX 
         10.0f - roomMargin,  // maxX
         1.0f,                // minY (keep camera above floor)
         7.0f,                // maxY (keep camera below ceiling)
        -10.0f + roomMargin,  // minZ
         10.0f - roomMargin   // maxZ
    );    // Museum state variables
    bool show_control_panel = true;
    bool show_scan_result_popup = false;
    float robot_position[3] = {0.0f, 0.0f, 0.0f};
    float robot_arm_angle = 0.0f;
    float ambient_light = 0.3f;
    bool directional_light = true;
      // Enhanced Lighting parameters
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 lightPos(5.0f, 5.0f, 5.0f);
    
    // Enhanced lighting controls
    float pointLightIntensity = 1.0f;
    bool enableWarmLighting = true;
    float atmosphericIntensity = 0.15f;
      // Main render loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // Poll and handle events
        glfwPollEvents();
        // Process input
        processInput(window);        // Update robot
        robot.update(deltaTime, objectManager);
        
        // Update museum object spotlights based on robot position
        objectManager.updateObjectSpotlights(robot.getPosition(), deltaTime);
          // Check if we have a new scan result
        const ScanResult& scanResult = robot.getLastScanResult();
        if (scanResult.hasResult && !show_scan_result_popup) {
            show_scan_result_popup = true;
            std::cout << "Detected scan result! Opening popup for: " << scanResult.objectName << std::endl;
        }
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // ImGui UI Elements for Virtual Museum Controls
        if (show_control_panel) {
            ImGui::Begin("Virtual Museum Control Panel", &show_control_panel);
            ImGui::Text("Welcome to the Virtual Museum!");
            ImGui::Separator();
            
            if (ImGui::CollapsingHeader("Robot Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
                // Display robot status
                glm::vec3 robotPos = robot.getPosition();
                ImGui::Text("Robot Position: (%.1f, %.1f, %.1f)", robotPos.x, robotPos.y, robotPos.z);
                
                // Automatic Tour Controls
                ImGui::Separator();
                ImGui::Text("Automatic Tour:");
                  if (ImGui::Button("Start Automatic Tour (1-5 & Return)")) {
                    robot.setAutoMode(true);
                    robot.setReturningHome(false);
                    robot.setCurrentTargetObjectIndex(-1);
                    // Robot will reset its internal scanned tracking automatically
                }
                  if (ImGui::Button("Stop Robot / Return Home")) {
                    robot.setAutoMode(false);
                    robot.setReturningHome(true);
                    robot.setCurrentTargetObjectIndex(-1);
                    robot.returnToHome();
                    // Update UI to reflect center position
                    robot_position[0] = robot_position[1] = robot_position[2] = 0.0f;
                }
                
                // Display current automatic tour status
                if (robot.isAutoMode()) {
                    ImGui::Text("Auto Mode: ACTIVE");
                    if (robot.isReturningHome()) {
                        ImGui::Text("Status: Returning Home");
                    } else {
                        int targetIndex = robot.getCurrentTargetObjectIndex();
                        if (targetIndex >= 0) {
                            ImGui::Text("Current Target: Object %d", targetIndex + 1);
                        } else {
                            ImGui::Text("Status: Selecting next target");
                        }
                    }
                } else {
                    ImGui::Text("Auto Mode: OFF");
                }
                
                ImGui::Separator();
                ImGui::Text("Manual Object Selection:");
                  std::vector<std::string> objectNames = objectManager.getObjectNames();
                for (int i = 0; i < objectNames.size(); ++i) {
                    ImGui::PushID(i);
                    if (ImGui::Button(objectNames[i].c_str())) {
                        robot.setAutoMode(false);
                        robot.setReturningHome(false);
                        robot.setCurrentTargetObjectIndex(i);
                        robot.moveToObject(i, objectManager);
                        // Update UI position for display
                        const MuseumObject* obj = objectManager.getObject(i);
                        if (obj) {
                            robot_position[0] = obj->position.x + 2.0f;
                            robot_position[1] = obj->position.y;
                            robot_position[2] = obj->position.z;
                        }
                    }
                    ImGui::PopID();
                }
                
                ImGui::Separator();
                
                // Robot arm controls
                ImGui::Text("Robot Arm Controls:");
                const RobotArm& arm = robot.getArm();
                ImGui::Text("Base Rotation: %.1f°", arm.baseRotation);
                ImGui::Text("Shoulder Angle: %.1f°", arm.shoulderAngle);
                ImGui::Text("Elbow Angle: %.1f°", arm.elbowAngle);
                ImGui::Text("Wrist Angle: %.1f°", arm.wristAngle);
                
                if (robot.isScanning()) {
                    ImGui::Text("Status: SCANNING...");
                    ImGui::ProgressBar(arm.scanProgress, ImVec2(0.0f, 0.0f));
                } else {
                    const char* stateNames[] = { "IDLE", "MOVING", "SCANNING", "RETURNING" };
                    ImGui::Text("Status: %s", stateNames[(int)robot.getState()]);
                }
                
                // Manual scanning controls
                ImGui::Separator();
                if (ImGui::Button("Start Scan") && !robot.isScanning()) {
                    robot.startScan();
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop Scan")) {
                    robot.stopScan();
                }
                  if (ImGui::Button("Return to Base")) {
                    robot.setAutoMode(false);
                    robot.returnToHome();
                    // Update UI to reflect center position (home base)
                    robot_position[0] = robot_position[1] = robot_position[2] = 0.0f;
                    robot_arm_angle = 0.0f;
                }
            }            if (ImGui::CollapsingHeader("Lighting Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::SliderFloat("Ambient Light", &ambient_light, 0.0f, 1.0f);
                ImGui::Checkbox("Directional Light", &directional_light);
                
                ImGui::Spacing();
                ImGui::Text("Museum Object Spotlights:");
                
                // Display active spotlights
                auto activeSpotlights = objectManager.getActiveSpotlights();
                auto activeObjects = objectManager.getActiveSpotlightObjects();
                
                if (activeObjects.empty()) {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No object spotlights active");
                    ImGui::Text("Move robot closer to objects to activate their spotlights");
                } else {
                    ImGui::Text("Active spotlights: %zu", activeObjects.size());
                    for (size_t i = 0; i < activeObjects.size(); ++i) {
                        const MuseumObject* obj = objectManager.getObject(activeObjects[i]);
                        if (obj) {
                            ImGui::Text("- %s (Intensity: %.2f)", obj->name.c_str(), obj->spotlight.intensity);
                        }
                    }                }
                
                ImGui::Spacing();
                ImGui::Text("Enhanced Lighting Controls:");
                ImGui::SliderFloat("Point Light Intensity", &pointLightIntensity, 0.0f, 2.0f);
                ImGui::Checkbox("Warm Museum Lighting", &enableWarmLighting);
                ImGui::SliderFloat("Atmospheric Lighting", &atmosphericIntensity, 0.0f, 0.5f);
                
                ImGui::Spacing();
                ImGui::Text("Lighting Presets:");
                if (ImGui::Button("Classic Museum")) {
                    ambient_light = 0.25f;
                    pointLightIntensity = 0.8f;
                    enableWarmLighting = true;
                    atmosphericIntensity = 0.2f;
                    directional_light = true;
                }
                ImGui::SameLine();
                if (ImGui::Button("Dramatic")) {
                    ambient_light = 0.1f;
                    pointLightIntensity = 1.2f;
                    enableWarmLighting = false;
                    atmosphericIntensity = 0.05f;
                    directional_light = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Bright Exhibition")) {
                    ambient_light = 0.4f;
                    pointLightIntensity = 1.0f;
                    enableWarmLighting = true;
                    atmosphericIntensity = 0.3f;
                    directional_light = true;
                }
                
                if (ImGui::Button("Reset Lighting")) {
                    ambient_light = 0.3f;
                    directional_light = true;
                    pointLightIntensity = 1.0f;
                    enableWarmLighting = true;
                    atmosphericIntensity = 0.15f;
                }
            }if (ImGui::CollapsingHeader("Camera Controls")) {
                ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", camera.Position.x, camera.Position.y, camera.Position.z);
                ImGui::Text("Camera Zoom: %.1f", camera.Zoom);
                ImGui::Separator();
                
                // Mouse capture status
                if (mouseCaptured) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Mouse Captured");
                    ImGui::Text("Press M to release mouse");
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Mouse Released");
                    ImGui::Text("Press M to capture mouse");
                }
                ImGui::Separator();
                
                ImGui::Text("Controls:");
                ImGui::Text("- Mouse: Look around (when captured)");
                ImGui::Text("- WASD: Move camera (when captured)"); 
                ImGui::Text("- Mouse wheel: Zoom");
                ImGui::Text("- M: Toggle mouse capture/release");
                ImGui::Text("- ESC: Exit");            if (ImGui::Button("Reset Camera")) {
                camera.Position = glm::vec3(0.0f, 3.0f, 5.0f);
                camera.Yaw = -90.0f;
                camera.Pitch = 0.0f;
                camera.Zoom = 45.0f;
                firstMouse = true;
            }
        }        
        ImGui::End();
    }

    // Display scan result popup if we have a new scan result
    if (show_scan_result_popup) {
        // Center the popup on the screen
        ImVec2 center = ImGui::GetIO().DisplaySize;
        center.x *= 0.5f;
        center.y *= 0.5f;
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        
        // Create the popup window
        ImGui::Begin("Scan Result", &show_scan_result_popup, 
                    ImGuiWindowFlags_AlwaysAutoResize | 
                    ImGuiWindowFlags_NoCollapse);
        
        // Get the scan result from the robot
        const ScanResult& scanResult = robot.getLastScanResult();
        
        if (scanResult.hasResult) {
            // Display object information
            ImGui::Text("Object: %s", scanResult.objectName.c_str());
            ImGui::Separator();
            
            // Display position information
            ImGui::Text("Position: (%.1f, %.1f, %.1f)", 
                    scanResult.objectPosition.x, 
                    scanResult.objectPosition.y, 
                    scanResult.objectPosition.z);
            
            // Display scan time
            float timeSinceScan = static_cast<float>(glfwGetTime()) - scanResult.scanTime;
            ImGui::Text("Scan Time: %.1f seconds ago", timeSinceScan);
            
            // Display object description with proper wrapping
            ImGui::Separator();
            ImGui::TextWrapped("Description:");
            ImGui::TextWrapped("%s", scanResult.objectDescription.c_str());
            
            // Close button
            ImGui::Spacing();
            if (ImGui::Button("Close", ImVec2(120, 0))) {
                show_scan_result_popup = false;
                robot.clearLastScanResult();
            }
        }
        else {
            // This should not happen, but just in case
            ImGui::Text("No scan data available!");
            if (ImGui::Button("Close")) {
                show_scan_result_popup = false;
            }
        }
        
        ImGui::End();
    }
        
    // --- RENDERING ---
        // Enhanced atmospheric background based on lighting settings
        glm::vec3 backgroundColor = enableWarmLighting ? 
            glm::vec3(0.12f, 0.10f, 0.08f) : glm::vec3(0.08f, 0.10f, 0.12f);
        backgroundColor += glm::vec3(atmosphericIntensity * 0.3f);
        glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ourShader.use();
            
            // Camera/view transformation
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();
            ourShader.setMat4("projection", projection);
            ourShader.setMat4("view", view);
            ourShader.setVec3("viewPos", camera.Position);
              // Material properties
            ourShader.setVec3("material.ambient", 0.2f, 0.2f, 0.2f);
            ourShader.setVec3("material.diffuse", 0.5f, 0.5f, 0.5f);
            ourShader.setVec3("material.specular", 1.0f, 1.0f, 1.0f);
            ourShader.setFloat("material.shininess", 64.0f);
            ourShader.setBool("hasTexture", false); // Default to no texture for room
             // Enhanced directional light with atmospheric adjustment
            glm::vec3 dirLightColor = enableWarmLighting ? 
                glm::vec3(1.0f, 0.95f, 0.85f) : glm::vec3(1.0f, 1.0f, 1.0f);
            
            ourShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
            ourShader.setVec3("dirLight.ambient", 
                ambient_light + atmosphericIntensity, 
                ambient_light + (enableWarmLighting ? atmosphericIntensity * 0.95f : atmosphericIntensity), 
                ambient_light + (enableWarmLighting ? atmosphericIntensity * 0.85f : atmosphericIntensity));
            ourShader.setVec3("dirLight.diffuse", 
                directional_light ? dirLightColor.x * 0.5f : 0.0f, 
                directional_light ? dirLightColor.y * 0.5f : 0.0f, 
                directional_light ? dirLightColor.z * 0.5f : 0.0f);
            ourShader.setVec3("dirLight.specular", 
                directional_light ? dirLightColor.x : 0.0f, 
                directional_light ? dirLightColor.y : 0.0f, 
                directional_light ? dirLightColor.z : 0.0f);
            
            // Enhanced point lights with warm/cool lighting and intensity control
            glm::vec3 pointLightPositions[] = {
                glm::vec3( 7.0f,  6.0f,  7.0f),
                glm::vec3(-7.0f,  6.0f,  7.0f),
                glm::vec3( 7.0f,  6.0f, -7.0f),
                glm::vec3(-7.0f,  6.0f, -7.0f)
            };
            
            // Calculate point light colors based on warm lighting setting
            glm::vec3 pointLightColor = enableWarmLighting ? 
                glm::vec3(1.0f, 0.9f, 0.8f) : glm::vec3(0.9f, 0.95f, 1.0f);
            
            for (int i = 0; i < 4; i++) {
                std::string index = std::to_string(i);
                ourShader.setVec3("pointLights[" + index + "].position", pointLightPositions[i]);
                ourShader.setVec3("pointLights[" + index + "].ambient", 
                    0.05f * pointLightIntensity * pointLightColor.x, 
                    0.05f * pointLightIntensity * pointLightColor.y, 
                    0.05f * pointLightIntensity * pointLightColor.z);
                ourShader.setVec3("pointLights[" + index + "].diffuse", 
                    0.8f * pointLightIntensity * pointLightColor.x, 
                    0.8f * pointLightIntensity * pointLightColor.y, 
                    0.8f * pointLightIntensity * pointLightColor.z);
                ourShader.setVec3("pointLights[" + index + "].specular", 
                    1.0f * pointLightIntensity * pointLightColor.x, 
                    1.0f * pointLightIntensity * pointLightColor.y, 
                    1.0f * pointLightIntensity * pointLightColor.z);
                ourShader.setFloat("pointLights[" + index + "].constant", 1.0f);
                ourShader.setFloat("pointLights[" + index + "].linear", 0.09f);
                ourShader.setFloat("pointLights[" + index + "].quadratic", 0.032f);
            }// Spotlights (museum object spotlights + robot spotlights)
            glm::vec3 spotlightPositions[4];
            glm::vec3 spotlightDirections[4];
            glm::vec3 spotlightColors[4];
            float spotlightIntensities[4];
            
            // Get active museum object spotlights
            auto activeSpotlights = objectManager.getActiveSpotlights();
            
            // Fill first slots with museum object spotlights (up to 2 slots)
            int spotlightIndex = 0;
            for (size_t i = 0; i < std::min(activeSpotlights.size(), size_t(2)); ++i) {
                spotlightPositions[spotlightIndex] = activeSpotlights[i].position;
                spotlightDirections[spotlightIndex] = activeSpotlights[i].direction;
                spotlightColors[spotlightIndex] = activeSpotlights[i].color;
                spotlightIntensities[spotlightIndex] = activeSpotlights[i].intensity;
                spotlightIndex++;
            }
            
            // Fill remaining slots with robot spotlights
            while (spotlightIndex < 4) {
                if (spotlightIndex == 2 && robot.hasScanningSpotlight()) {
                    spotlightPositions[spotlightIndex] = robot.getScanningSpotlightPosition();
                    spotlightDirections[spotlightIndex] = robot.getScanningSpotlightDirection();
                    spotlightColors[spotlightIndex] = robot.getMainSpotlightColor();
                    spotlightIntensities[spotlightIndex] = robot.getScanningSpotlightIntensity();
                } else if (spotlightIndex == 3 && robot.isSecondarySpotlightActive()) {
                    spotlightPositions[spotlightIndex] = robot.getSecondarySpotlightPosition();
                    spotlightDirections[spotlightIndex] = robot.getSecondarySpotlightDirection();
                    spotlightColors[spotlightIndex] = robot.getSecondarySpotlightColor();
                    spotlightIntensities[spotlightIndex] = robot.getSecondarySpotlightIntensity();
                } else {
                    // Empty spotlight slot
                    spotlightPositions[spotlightIndex] = glm::vec3(0.0f);
                    spotlightDirections[spotlightIndex] = glm::vec3(0.0f, -1.0f, 0.0f);
                    spotlightColors[spotlightIndex] = glm::vec3(1.0f);
                    spotlightIntensities[spotlightIndex] = 0.0f;
                }
                spotlightIndex++;
            }
              for (int i = 0; i < 4; i++) {
                std::string index = std::to_string(i);
                ourShader.setVec3("spotLights[" + index + "].position", spotlightPositions[i]);
                ourShader.setVec3("spotLights[" + index + "].direction", spotlightDirections[i]);
                ourShader.setVec3("spotLights[" + index + "].ambient", 0.0f, 0.0f, 0.0f);
                ourShader.setVec3("spotLights[" + index + "].diffuse", spotlightColors[i] * spotlightIntensities[i]);
                ourShader.setVec3("spotLights[" + index + "].specular", spotlightColors[i] * spotlightIntensities[i]);
                ourShader.setFloat("spotLights[" + index + "].constant", 1.0f);
                ourShader.setFloat("spotLights[" + index + "].linear", 0.09f);
                ourShader.setFloat("spotLights[" + index + "].quadratic", 0.032f);
                
                // Set cut-off angles based on spotlight type
                if (i < activeSpotlights.size()) {
                    // Museum object spotlights use their own parameters
                    ourShader.setFloat("spotLights[" + index + "].cutOff", glm::cos(glm::radians(activeSpotlights[i].cutOff)));
                    ourShader.setFloat("spotLights[" + index + "].outerCutOff", glm::cos(glm::radians(activeSpotlights[i].outerCutOff)));
                } else if (i >= 2) {
                    // Robot spotlights use tighter beam
                    ourShader.setFloat("spotLights[" + index + "].cutOff", glm::cos(glm::radians(robot.getSpotlightConeAngle())));
                    ourShader.setFloat("spotLights[" + index + "].outerCutOff", glm::cos(glm::radians(robot.getSpotlightConeAngle() + 5.0f)));
                } else {
                    // Default spotlight parameters for empty slots
                    ourShader.setFloat("spotLights[" + index + "].cutOff", glm::cos(glm::radians(25.0f)));
                    ourShader.setFloat("spotLights[" + index + "].outerCutOff", glm::cos(glm::radians(35.0f)));
                }}
            
            // Render the museum room
            glm::mat4 model = glm::mat4(1.0f);
            ourShader.setMat4("model", model);
            ourShader.setBool("hasTexture", false);
            room.render();
              // Render museum objects
            ourShader.setBool("hasTexture", true);
            objectManager.drawAll(ourShader);
              // Render mobile robot
            robot.render(ourShader);
        
        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // Swap buffers
        glfwSwapBuffers(window);
    }
    
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    // Clean up and exit
    glfwTerminate();
    return 0;
}