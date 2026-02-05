cmake_minimum_required(VERSION 3.16)
project(spacecraft_viewer LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

set(CONDA_PATH "$ENV{CONDA_PREFIX}")

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(scrt
    src/application.cpp
    src/utilities.cpp
    external/glad/glad.c
)

target_include_directories(scrt PRIVATE
    external/glad/include
    external/stb
    include/
    ${CONDA_PATH}/include
)

target_link_libraries(scrt PRIVATE
    ${CONDA_PATH}/lib/libimgui.so
    glfw
    glm::glm
)
