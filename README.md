# Virtual Museum Simulation

A 3D interactive virtual museum application created with OpenGL and C++, featuring a navigable environment with 3D models of museum artifacts, an automatic tour robot guide, and advanced lighting and rendering techniques.

## Project Overview

This Virtual Museum Simulation allows users to explore a 3D museum environment containing various archaeological artifacts. The application features:

- Third-person navigation through a virtual museum space
- Interactive museum robot guide that can provide information about exhibits
- Automatic tour functionality with the robot visiting exhibits sequentially
- Advanced lighting effects including directional and spotlights
- ImGui-based control panel for museum, light and robot interaction

## Project Structure

### Core Components

- **Main Application (`Main.cpp`)**: Entry point and main loop handling rendering, input, and UI
- **Museum Room (`MuseumRoom.cpp/.h`)**: Manages the 3D environment of the museum
- **Museum Object Manager (`MuseumObjectManager.cpp/.h`)**: Handles loading and managing museum artifacts
- **Mobile Robot (`MobileRobot.cpp/.h`)**: Controls the robot guide behavior and movement
- **Camera System (`Camera.cpp/.h`)**: Handles view and navigation in the 3D space
- **Shaders (`shader.vert`, `shader.frag`)**: OpenGL shaders for 3D rendering
- **Ray Tracer (`RayTracer.cpp/.h`)**: Provides realistic rendering effects

### External Libraries

- **GLFW**: Window management and OpenGL context creation
- **GLAD**: OpenGL function loader
- **GLM**: Mathematics library for 3D operations
- **Assimp**: 3D model loading
- **ImGui**: User interface components
- **stb_image**: Image loading for textures

### Models and Textures

- Various 3D archaeological artifact models in GLB format located in the `models/` directory
- Texture files in the `textures/` directory

## Building the Project

### Prerequisites

- Visual Studio 2022 (or compatible version)
- Windows 10/11
- Graphics card supporting OpenGL 3.3 or higher

### Build Instructions

1. Open `Project1.sln` in Visual Studio
2. Select the build configuration (Debug/Release) and platform (x64 recommended)
3. Build the solution (Ctrl+Shift+B or Build > Build Solution)
4. The executable will be created in `x64/Debug/` or `x64/Release/` depending on your configuration

## Running the Application

1. Make sure you are in the directory containing the executable (`x64/Debug/Project1.exe`)
2. Run the application
3. If any errors occur regarding missing DLLs, ensure the required DLLs (particularly assimp-vc143-mt.dll) are in the executable directory

## Controls

### Navigation Controls

- **W, A, S, D**: Move forward, left, backward, right
- **Mouse Movement**: Look around
- **Mouse Scroll**: Zoom in/out
- **ESC**: Exit application
- **M**: Toggle mouse capture (for using UI elements)

### Robot Controls

The Virtual Museum features an interactive robot guide that can navigate between exhibits and provide information:

- Use the UI control panel (visible when mouse is freed with M key) to:
  - Start an automatic tour of all exhibits
  - Command the robot to return to its home position
  - Select specific artifacts for the robot to visit
  - View robot position and status information

## Features

### 3D Environment

- Detailed museum room with textured walls, floor, and ceiling
- Multiple 3D archaeological artifact models
- Dynamic camera with collision detection to prevent moving through walls
- Simulated lighting with day/night cycle option

### Robot Guide

- Automatic pathfinding between exhibits
- Artifact scanning and information display
- Manual and automatic tour modes
- Return-to-home functionality

### Advanced Rendering

- Phong lighting model with ambient, diffuse, and specular components
- Ray tracing effects for reflective and transparent surfaces
- Normal mapping for detailed surface textures
- Dynamic spotlight system that follows the robot

### User Interface

- ImGui-based control panel for:
  - Controlling the robot guide
  - Selecting specific exhibits
  - Adjusting lighting parameters
  - Viewing artifact information when scanned

### Robot Arm

- The robot guide is equipped with a mobile arm for scanning artifacts:

*   **Advanced Scanning Physics:** The scanning mechanism simulates a sensor sweep. When activated, the robot's arm moves in a predefined pattern. During this sweep, the system checks for proximity to designated scannable objects within a certain range and cone of view. If an object is detected, its information is "captured" and displayed. This involves ray casting or sphere casting from the arm's tip to detect objects.
*   **Scanning Beam:** A visual effect, such as a colored beam or a spotlight cone, emanates from the robot's arm tip during the scanning process. This beam visually indicates the scanning direction and area of effect. The beam might pulse or change intensity to signify active scanning.
*   **Object Interaction:** Can be programmed to "scan" specific objects in the museum, retrieving and displaying their information.

## Customization and Extension

The Virtual Museum application can be extended in several ways:

1. **Adding New Exhibits**: Place new 3D models in the `models/` directory and update the `MuseumObjectManager::loadDefaultObjects()` method to include them
2. **Modifying Room Layout**: Edit the `MuseumRoom::generateRoomGeometry()` method to change room dimensions or features
3. **Custom Lighting**: Adjust lighting parameters in the UI or directly in the shader parameters
4. **Adding Interactivity**: Extend the robot's capabilities or add new interactive elements

## Troubleshooting

- **Missing textures or models**: Ensure all texture and model files are in their respective directories
- **OpenGL errors**: Verify your graphics drivers are updated and support OpenGL 3.3+
- **Robot navigation issues**: The robot might get stuck in complex layouts; adjust museum object positions for better clearance

## License

This project is provided for educational purposes.

## Acknowledgments

- 3D models from various archaeological collections
- OpenGL and related libraries for 3D rendering capabilities
