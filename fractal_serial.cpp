#include "fractal_serial.h" // Incluimos las declaraciones para que el compilador verifique las firmas.
#include <complex>          // Librería estándar para manejar números complejos y funciones como std::abs.
#include "palette.h"       // Nuestra paleta de colores predefinida (opcional).
// EXTERN: Estas variables existen físicamente en main.cpp. 
// Aquí solo le decimos al compilador que confíe en que el "Linker" las encontrará.
extern int max_iteraciones;
extern std::complex<double> c;

// --- VERSIÓN 1: USANDO LA LIBRERÍA STANDARD <complex> ---

// Función que determina si un punto individual pertenece al conjunto o "escapa" al infinito.
uint32_t acotado_1(std::complex<double> z0) {
    int iter = 1;               // Contador de iteraciones.
    std::complex<double> z = z0; // El punto inicial de la iteración es la coordenada del píxel.

    // Bucle de escape: se detiene si superamos el límite de intentos o si la magnitud de z > 2.0.
    while (iter < max_iteraciones && std::abs(z) < 2.0) {
        z = z * z + c;          // Zn+1 = Zn^2 + c (La fórmula de Julia).
        iter++;
    }

    // Si terminó antes de llegar al máximo, es porque escapó (está fuera del conjunto).
    if(iter < max_iteraciones){
        //return 0xFF0000FF;      // Devolvemos color ROJO (Formato RGBA: 0x RR GG BB AA).
        int index = iter % PALETTE_SIZE; // Usamos el contador de iteraciones para elegir un color de la paleta.
        return color_ramp[index]; // Devolvemos el color correspondiente de la paleta.
    }
    return 0xFF000000;          // Si nunca escapó, devolvemos NEGRO.
}

// Función principal para la versión 1.
void julia_serial_1(double x_min, double y_min, double x_max, double y_max, uint32_t width, uint32_t height, uint32_t* pixel_buffer) {

    // Calculamos el "paso" matemático que representa cada píxel.
    double dx = (x_max - x_min) / width;   
    double dy = (y_max - y_min) / height;  

    // Doble bucle para recorrer la rejilla de píxeles (Imagen 2D).
    for(int i = 0; i < width; i++) {       // i recorre columnas (eje X).
        for(int j = 0; j < height; j++) {  // j recorre filas (eje Y).
            
            // MAPEO: Convertimos el índice del píxel (i, j) a coordenadas reales (x, y).
            double x = x_min + i * dx;
            double y = y_min + j * dy;

            // Creamos el número complejo z0 a partir de las coordenadas mapeadas.
            std::complex<double> z(x, y);

            // Obtenemos el color llamando a la función de cálculo.
            auto color = acotado_1(z);

            // Guardamos el color en el buffer plano usando la fórmula de índice 1D: (Fila * Ancho + Columna).
            pixel_buffer[j * width + i] = color;
        }
    }
}

// --- VERSIÓN 2: OPTIMIZACIÓN MANUAL SIN <complex> ---

// Aquí hacemos la matemática de complejos "a mano" para ganar velocidad.
uint32_t acotado_2(double x, double y) {
    int iter = 1;
    double zr = x; // Parte real de z.
    double zi = y; // Parte imaginaria de z.

    // OPTIMIZACIÓN: (zr*zr + zi*zi) < 4.0 es equivalente a abs(z) < 2.0.
    // Se usa 4.0 porque así evitamos calcular la raíz cuadrada interna de abs().
    while (iter < max_iteraciones && (zr * zr + zi * zi) < 4.0) {
        
        // Aplicamos binomio al cuadrado: (a + bi)^2 = (a^2 - b^2) + (2ab)i.
        double dr = zr * zr - zi * zi + c.real(); // Nueva parte real.
        double di = 2.0 * zr * zi + c.imag();     // Nueva parte imaginaria.
        
        zr = dr; // Actualizamos para la siguiente iteración.
        zi = di;
        iter++;
    }

    if(iter < max_iteraciones){
        int index = iter % PALETTE_SIZE; // Usamos el contador de iteraciones para elegir un color de la paleta.
        return color_ramp[index]; // Devolvemos el color correspondiente de la paleta.
        //return 0xFF0000FF; // Rojo si escapa.
    }
    return 0xFF000000; // Negro si no escapa.
}

// Función principal para la versión 2.
void julia_serial_2(double x_min, double y_min, double x_max, double y_max, uint32_t width, uint32_t height, uint32_t* pixel_buffer) {

    double dx = (x_max - x_min) / width;   
    double dy = (y_max - y_min) / height;  

    for(int i = 0; i < width; i++) {
        for(int j = 0; j < height; j++) {
            
            double x = x_min + i * dx;
            double y = y_min + j * dy;

            // Llamamos a la versión optimizada que recibe doubles directamente.
            auto color = acotado_2(x, y);

            pixel_buffer[j * width + i] = color;
        }
    }
}