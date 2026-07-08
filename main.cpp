#include <iostream>
#include <complex>
#include <cstdint>
#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <cuda_runtime.h>
#ifdef _WIN32
    #include <windows.h>
#endif

// Definimos la resolución de nuestra área de dibujo.
#define WIDTH 1600
#define HEIGHT 900
std::complex<double> c(-0.7, 0.27015); // La semilla que da forma al fractal de Julia.

// Definimos los límites del plano complejo donde queremos "mirar".
double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;

int max_iteraciones = 10; // Parámetro dinámico: puede cambiar mientras el programa corre.

// buffer host
uint32_t* host_pixel_buffer = nullptr;
// buffer device
uint32_t* device_pixel_buffer = nullptr;

#define CHECK(expr) {                               \
        auto internal_error = (expr);               \
        if (internal_error!=cudaSuccess) {          \
            fmt::println("{}: {} in {} at line {}", (int)internal_error, cudaGetErrorString(internal_error), __FILE__, __LINE__);    \
            exit(EXIT_FAILURE);                     \
        }                                           \
    }

// Declaración externa de la función CUDA (Se añadió el ';' faltante)
extern void julia_gpu(
    double x_min, double y_min, double x_max, double y_max, 
    uint32_t width, 
    uint32_t height, 
    uint32_t* pixel_buffer
);

int main() {
    int deviceId = 0; // Seleccionamos el primer dispositivo CUDA disponible.
    cudaSetDevice(deviceId);
    cudaDeviceProp deviceProp;
    cudaGetDeviceProperties(&deviceProp, deviceId);

    fmt::println("Device: {}", deviceProp.name);
    fmt::println("Total memory: {} MB", deviceProp.totalGlobalMem / (1024 * 1024));

    // 1. ASIGNACIÓN Y RESERVA DE MEMORIA
    size_t buffer_size = WIDTH * HEIGHT * sizeof(uint32_t);
    
    // Reservar memoria en el Host (CPU) y limpiarla a 0 (Color Negro ARGB/RGBA)
    host_pixel_buffer = (uint32_t*)malloc(buffer_size);
    std::memset(host_pixel_buffer, 0, buffer_size);

    // Reservar memoria en el Device (GPU)
    CHECK(cudaMalloc(&device_pixel_buffer, buffer_size));

    // 2. INICIALIZAR UI Y MOTOR GRÁFICO (SFML)
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Julia Set - CUDA SFML");

    // Si es Windows, forzamos que la ventana se abra maximizada usando el handle nativo.
#ifdef _WIN32
    HWND hwnd = window.getNativeHandle(); 
    ShowWindow(hwnd, SW_MAXIMIZE);        
#endif

    sf::Vector2u windowSize = {WIDTH, HEIGHT};
    sf::Texture texture(windowSize); 
    // El 'sprite' es el objeto que permite "dibujar" la textura en la ventana.
    sf::Sprite sprite(texture); 

    // Cargamos una fuente y configuramos el texto que mostrará los FPS.
    sf::Font font("arial.ttf"); 
    sf::Text text(font, "Julia Set", 24); 
    text.setFillColor(sf::Color::White); 
    text.setPosition({10, 10}); 
    text.setStyle(sf::Text::Bold); 

    std::string options = "Up/Down: Change iterations";
    sf::Text textOptions(font, options, 20);
    textOptions.setFillColor(sf::Color::White);
    textOptions.setStyle(sf::Text::Bold);
    textOptions.setPosition({10, static_cast<float>(window.getSize().y - 40)}); // Posicionar en la parte inferior.

    // Variables para medir el rendimiento (Frames Per Second).
    int frames = 0;
    int fps = 0;
    sf::Clock clock; 

    std::string mode = "GPU CUDA (Inactivo)";

    // 3. BUCLE PRINCIPAL (Game Loop): Se ejecuta hasta que se cierre la ventana.
    while (window.isOpen())
    {
        // A. PROCESAR EVENTOS: Entrada del usuario.
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close(); // Cerrar el programa.
            } else if(event->is<sf::Event::KeyReleased>()) {
                auto evt = event->getIf<sf::Event::KeyReleased>();
                // Controlamos las iteraciones con las flechas del teclado.
                switch(evt->scancode) {
                    case sf::Keyboard::Scan::Up:
                        max_iteraciones += 10; // Más detalle.
                        break;
                    case sf::Keyboard::Scan::Down:
                        max_iteraciones -= 10; // Menos detalle.
                        if(max_iteraciones < 10) max_iteraciones = 10;
                        break;
                    default:
                        break;
                }
            }
        }

        // B. CÁLCULO DEL FRACTAL / LOGICA CUDA (Comentado de momento para mantener ventana en negro)
        /*
        julia_gpu(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, device_pixel_buffer);
        CHECK(cudaGetLastError()); // Corregido typo: cudaGeetLastError -> cudaGetLastError
        CHECK(cudaMemcpy(host_pixel_buffer, device_pixel_buffer, buffer_size, cudaMemcpyDeviceToHost)); // Corregido typo: cudaMencpy
        */

        // C. ACTUALIZAR TEXTURA: 
        // Como el buffer de la CPU está lleno de ceros por el memset inicial, la textura se actualizará en negro.
        texture.update((const uint8_t*)host_pixel_buffer);
        frames++;

        // D. CÁLCULO DE FPS: Cada vez que pase 1 segundo, actualizamos el contador.
        if (clock.getElapsedTime().asSeconds() >= 1.0f){
            fps = frames;
            frames = 0;
            clock.restart();
        }

        // E. ACTUALIZAR HUD: Formateamos el mensaje de texto.
        auto msg = fmt::format("Julia Set: Iterations: {}, FPS: {}, Mode: {}", max_iteraciones, fps, mode);
        text.setString(msg);

        // F. RENDERIZADO:
        window.clear(sf::Color::Black); // Limpiar la pantalla con fondo negro estricto.
        window.draw(sprite);            // Dibujar el buffer texturizado (actualmente negro).
        window.draw(text);              // Dibujar el contador de FPS encima.
        window.draw(textOptions);       // Dibujar las opciones de control.
        window.display();               // Intercambiar buffers para mostrar en pantalla.
    }

    // 4. LIMPIEZA DE MEMORIA: Evita fugas de memoria al cerrar la app
    free(host_pixel_buffer);
    CHECK(cudaFree(device_pixel_buffer));

    return 0;
}