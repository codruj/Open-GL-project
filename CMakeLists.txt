cmake_minimum_required(VERSION 3.29)
project(proiectgp)

set(CMAKE_CXX_STANDARD 20)

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
link_libraries(${GLEW_LIBRARIES})

find_package(glfw3 REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})

# Add GLM using an absolute path
set(GLM_INCLUDE_DIR "/Users/codrutajucan/Desktop/AN3/GP")
include_directories(${GLM_INCLUDE_DIR})

add_executable(proiectgp main.cpp Camera.cpp Camera.hpp Mesh.cpp Model3D.cpp Model3D.hpp Shader.cpp Shader.hpp stb_image.cpp stb_image.h tiny_obj_loader.cpp tiny_obj_loader.h Window.cpp Window.h
)

target_link_libraries(proiectgp PRIVATE glfw GLEW::GLEW
        "-framework AudioToolbox"
)
