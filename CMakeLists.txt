cmake_minimum_required(VERSION 3.24)
project(VulkanRenderer)

set(CMAKE_CXX_STANDARD 23)

#IF (CMAKE_COMPILER_IS_MINGW)
#ENDIF ()

#list(APPEND CMAKE_PREFIX_PATH "C:/Program Files (x86)/GLFW")
#set(glfw3_DIR "External/glfw-3.4/build/src")

find_package(Vulkan REQUIRED)
#find_package(Slang REQUIRED)
#find_package(glfw3 3.4 REQUIRED)
#find_package(glm REQUIRED)

find_library(Slang_LIBRARY NAMES slang HINTS ${Vulkan_LIBRARY}/../ NO_DEFAULT_PATH)

include_directories(${CMAKE_SOURCE_DIR}/External/glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/External/assimp/include)
include_directories(${CMAKE_SOURCE_DIR}/External/assimp)
#include_directories(${CMAKE_CURRENT_BINARY_DIR}/External/assimp/include)
include_directories(${CMAKE_SOURCE_DIR}/External/imgui)
include_directories(${CMAKE_SOURCE_DIR}/External/stb)
include_directories(${CMAKE_SOURCE_DIR}/External/tinyobjloader)

add_subdirectory(VulkanRenderer)
add_subdirectory(External)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -Wl,-allow-multiple-definition -mbig-obj")

target_link_libraries(${PROJECT_NAME} PUBLIC imgui Vulkan::Vulkan glfw ${GLFW_LIBRARIES} ${Slang_LIBRARY} assimp) # GLM::GLM)
