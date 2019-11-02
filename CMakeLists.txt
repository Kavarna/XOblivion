cmake_minimum_required(VERSION 3.13)

project(XOblivion)


find_package(Vulkan REQUIRED)
find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
add_definitions(-DVK_USE_PLATFORM_XLIB_KHR)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/boost/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/Common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/headers/)


add_executable(XOblivion
    src/Common/Oblivion.cpp

    src/Core/HighResolutionTimer.cpp
    
    src/Gameplay/FirstPersonCamera.cpp

    src/Graphics/imgui/imgui_demo.cpp
    src/Graphics/imgui/imgui_draw.cpp
    src/Graphics/imgui/imgui_widgets.cpp
    src/Graphics/imgui/imgui.cpp
    
    src/Graphics/Interfaces/IGraphicsObject.cpp

    src/Graphics/Pipeline/Layout/TextureLayout.cpp
    src/Graphics/Pipeline/Layout/UIOverlayLayout.cpp

    src/Graphics/Utils/Image.cpp
    src/Graphics/Utils/Shader.cpp
    src/Graphics/Utils/stbImage.cpp
    src/Graphics/Utils/BufferUtils.cpp
    src/Graphics/Utils/ObjLoader.cpp
    src/Graphics/Utils/Samplers.cpp
    src/Graphics/Utils/VulkanAllocators.cpp

    src/Graphics/Model.cpp
    src/Graphics/UIOverlay.cpp
    src/Graphics/VulkanDebug.cpp
    src/Graphics/VulkanRenderer.cpp

    src/Scenes/SimpleScene.cpp

    src/Game.cpp
    src/main.cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glfw)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

target_link_libraries(XOblivion glfw ${GLFW_LIBRARIES})

target_link_libraries(XOblivion Vulkan::Vulkan)

