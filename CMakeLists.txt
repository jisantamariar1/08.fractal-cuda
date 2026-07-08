
cmake_minimum_required(VERSION 3.16)

project(FractalCuda)

enable_language(CXX CUDA)


find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS graphics window system REQUIRED)



add_executable(fractal-cuda
 main.cpp
 pallette.cpp
 kernel.cu


)

target_include_directories(fractal-cuda PRIVATE
    ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})


target_link_libraries(fractal-cuda PRIVATE
    fmt::fmt          # Conecta las funciones de texto de fmt.
    SFML::Graphics
    SFML::Window
    SFML::System

)