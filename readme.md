# Description

A toy renderer written with Vulkan.<br>
The renderer is only tested to work with Windows and MinGW!

# Dependencies

This project requires Vulkan SDK version 1.4 or higher.<br>
Download it <a href=https://www.lunarg.com/vulkan-sdk/>here</a>. Note that the VULKAN_SDK path variable must be configured correctly (should be done by the installer).<br>
Also, this project expects glm headers to be found with Vulkan. Make sure the corresponding box is checked when installing Vulkan SDK.

<a href=https://www.glfw.org/>GLFW</a> is referenced as a git submodule and should be linked automatically.
<a href=https://github.com/nothings/stb>STB image</a>, <a href=https://github.com/tinyobjloader/tinyobjloader>tinyobjloader</a> and <a href=https://github.com/assimp/assimp>assimp</a> are included directly.

# Build

This project uses cmake. Create a folder ```build``` in the root folder of the repo, navigate there and run ```cmake .. -G "MinGW Makefiles"``` to generate MinGW makefiles.
After that, compile using MinGW.<br>
Note that the project requires C++ 23!

# Running the Application

After building, navigate to ```build/VulkanRendere``` and run ```VulkanRenderer.exe```. Make sure to run it from this folder, as otherwise the asset files are not found!

# Note

The master branch should always work as intended. The dev branch on the other hand might not even compile!
