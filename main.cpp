#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <cuda_runtime.h>
#include "palette.h"

#include <complex>

// Pamarametro img
#define ANCHO 1600
#define ALTO 900

// Parameteros
int max_iteraciones = 10;
double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;

// Complejo que almacena dobles
std::complex<double> c(-0.71, 0.27015);
uint32_t *host_pixel_buffer = nullptr;
uint32_t *device_pixel_buffer = nullptr;
extern void copiar_paleta(unsigned int *pallete_host);

extern void julia_gpu(double centro_real, double centro_imag, int num_iteraciones, double x_min, double y_min, double x_max, double y_max, uint32_t width, uint32_t height, uint32_t *pixel_buffer);

#define CHECK(expr)                                                                                                               \
    {                                                                                                                             \
        auto internal_error = (expr);                                                                                             \
        if (internal_error != cudaSuccess)                                                                                        \
        {                                                                                                                         \
            fmt::println("{}: {} in {} at line {}", (int)internal_error, cudaGetErrorString(internal_error), __FILE__, __LINE__); \
            exit(EXIT_FAILURE);                                                                                                   \
        }                                                                                                                         \
    }

int main()
{
    int deviceId = 0;
    cudaSetDevice(deviceId);
    cudaDeviceProp deviceProp;
    cudaGetDeviceProperties(&deviceProp, deviceId);
    fmt::print("Device Name: {}\n", deviceProp.name);
    fmt::println("Total memory: {} MB", deviceProp.totalGlobalMem / (1024 * 1024));
    
    
    size_t buffer_size = ANCHO * ALTO * sizeof(uint32_t);
    host_pixel_buffer = (uint32_t *)malloc(buffer_size);

    std::memset(host_pixel_buffer, 0, buffer_size);
    CHECK(cudaMalloc(&device_pixel_buffer, buffer_size));
    copiar_paleta(color_ramp.data());
    // graifica

    sf::RenderWindow window(sf::VideoMode({ANCHO, ALTO}), "Julia");

#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE); // Maximizar Ventana
#endif
    sf::Vector2u windowSize = {ANCHO, ALTO};
    sf::Texture texture(windowSize);
    sf::Sprite sprite(texture);

    sf::Font font("arial.ttf");
    sf::Text text(font, "Julia Set", 24);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    std::string options = "Up/Sown change iterations";
    sf::Text textoptions(font, options, 20);

    textoptions.setStyle(sf::Text::Bold);
    textoptions.setPosition({10, window.getView().getSize().y - 40});
    // FPS
    int frames = 0;
    int fps = 0;
    sf::Clock clock;

    while (window.isOpen())
    {
        // Process events
        while (const std::optional event = window.pollEvent())
        {
            // Close window: exit
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (event->is<sf::Event::KeyReleased>())
            {
                auto evt = event->getIf<sf::Event::KeyReleased>();

                switch (evt->scancode)
                {
                case sf::Keyboard::Scan::Up:
                    max_iteraciones += 10;
                    break;
                case sf::Keyboard::Scan::Down:
                    max_iteraciones -= 10;
                    if (max_iteraciones < 10)
                        max_iteraciones = 10;
                    break;
                }
            }
        }
            std::string mode = "";
            //---------------------------
            // Dibuja Fractal dependera de la velocidad
            mode = "GPU CUDA";
            // dibujar en la gpu
            // invocar kernel
            // copiar la imagen gpu-cpu
            julia_gpu(
                c.real(), c.imag(), 
                max_iteraciones, 
                x_min, y_min, x_max, y_max,
                 ANCHO, ALTO, 
                 device_pixel_buffer);
            CHECK(cudaGetLastError());
            CHECK(cudaMemcpy(host_pixel_buffer, device_pixel_buffer, buffer_size, cudaMemcpyDeviceToHost));
            //----------------------

            texture.update((const uint8_t *)host_pixel_buffer);

            frames++;

            if (clock.getElapsedTime().asSeconds() >= 1.0f)
            {
                fps = frames;
                frames = 0;
                clock.restart();
            }
            auto msg = fmt::format("julia: iteraciones; {}.fps{}, Mode: {}", max_iteraciones, fps, mode);
            text.setString(msg);

            // Clear screen
            window.clear();
            {
                window.draw(sprite);
                window.draw(text);
                window.draw(textoptions);
            }

            // Update the window
            window.display();
        }
free(host_pixel_buffer);
    cudaFree(device_pixel_buffer);
      return 0;
    }
      