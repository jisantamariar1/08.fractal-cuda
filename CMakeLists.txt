cmake_minimum_required(VERSION 3.18) # Subido a 3.18 para mejor soporte de CUDA + componentes modernos

project(FractalCuda)

# Habilitamos CXX y CUDA
enable_language(CXX CUDA)

# Buscamos los paquetes necesarios
find_package(fmt CONFIG REQUIRED)

# CORRECCIÓN PARA SFML 3: Componentes ahora van con Mayúscula inicial
find_package(SFML COMPONENTS Graphics Window System REQUIRED)

add_executable(fractal-cuda
    main.cpp
    pallette.cpp
    kernel.cu
)

target_include_directories(fractal-cuda PRIVATE
    ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)

target_link_libraries(fractal-cuda PRIVATE
    fmt::fmt          # Conecta las funciones de texto de fmt.
    SFML::Graphics    # Conecta el módulo gráfico de SFML
    SFML::Window      # Conecta el módulo de ventanas de SFML
    SFML::System      # Conecta el módulo base de SFML
)