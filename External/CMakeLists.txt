add_subdirectory(glfw)

#set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
#set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
#set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
#set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

add_subdirectory(assimp)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_library(imgui)
target_include_directories(imgui PUBLIC imgui)

target_sources(imgui PRIVATE imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_vulkan.cpp
        imgui/misc/cpp/imgui_stdlib.cpp)

target_include_directories(imgui PUBLIC ${Vulkan_INCLUDE_DIRS})