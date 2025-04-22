add_subdirectory(glfw)

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