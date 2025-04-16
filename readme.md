# Description

A toy renderer written with Vulkan.<br>
The renderer is only tested to work with Windows and MinGW!

# Dependencies

This project requires Vulkan SDK version 1.4 or higher.<br>
Download it here. Note that the VULKAN_SDK path variable must be configured correctly (should be done by the installer).<br>
Also, this project expects glm headers to be found with Vulkan. Make sure the corresponding box is checked when installing Vulkan SDK.

GLFW is referenced as a git submodule and should be linked automatically.
STB image and tinyobjloader are included directly.

# Build

This project uses cmake. Simply run your favorite cmake command and compile.<br>
Note that the project requires C++ 23!

# Note

The master branch should always work as intended. The dev branch on the other hand might not even compile!
