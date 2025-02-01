cmake_minimum_required(VERSION 3.24)
project(VulkanRenderer)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_PREFIX_PATH "C:/Program Files (x86)/GLFW")
#set(glfw3_DIR "External/glfw-3.4/build/src")

find_package(Vulkan REQUIRED)
#find_package(glfw3 3.4 REQUIRED)
#find_package(glm REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/External/glfw-3.4/include)

add_subdirectory(External)
add_subdirectory(VulkanRenderer)

target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)# GLM::GLM)
