const int PALETTE_SIZE = 16;
__constant__
unsigned int color_ramp[PALETTE_SIZE];

__device__
uint32_t acotado(double x, double y, double cr, double ci, int max_iteraciones) {
    int iter = 1;
    double zr = x; // Parte real de z.
    double zi = y; // Parte imaginaria de z.

    // OPTIMIZACIÓN: (zr*zr + zi*zi) < 4.0 es equivalente a abs(z) < 2.0.
    // Se usa 4.0 porque así evitamos calcular la raíz cuadrada interna de abs().
    while (iter < max_iteraciones && (zr * zr + zi * zi) < 4.0) {
        
        // Aplicamos binomio al cuadrado: (a + bi)^2 = (a^2 - b^2) + (2ab)i.
        double dr = zr * zr - zi * zi + cr; // Nueva parte real.
        double di = 2.0 * zr * zi + ci;     // Nueva parte imaginaria.
        
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


__global__
void julia_kernel(
    double centro_real, double centro_imag,
    int num_iteraciones,
    double x_min, double y_min, double x_max, double y_max, 
    uint32_t width,
     uint32_t height, 
     uint32_t* pixel_buffer) {

        double dx = (x_max - x_min) / width;
        double dy = (y_max - y_min) / height;

        int index = blockIdx.x * blockDim.x + threadIdx.x;


        if (index<width * height){
            int i = index / width;
            int j = index % width;

            double x = x_min + i * dx;
            double y = y_min + j * dy;

            auto color = acotado(
                x,y, centro_real, centro_imag, num_iteraciones); // Default color if max iterations reached
            
            pixel_buffer[j*width + i] = color;
        }

    }


    void copiar_paleta(unsigned int* h_palette) {
        //copiar la paleta desde la cpu a la gpu
        cudaMemcpyToSymbol(color_ramp, h_palette, PALETTE_SIZE * sizeof(unsigned int));
    }


void julia_gpu(
    double centro_real, double centro_imag,
    int num_iteraciones,
    double x_min, double y_min, double x_max, double y_max, 
    uint32_t width,
     uint32_t height, 
     uint32_t* pixel_buffer) {

        int threads_per_block = 1024;
        int blocks_per_grid = std::ceil((width * height)*1.0 / threads_per_block);

        julia_kernel<<<blocks_per_grid, threads_per_block>>>(
            centro_real, centro_imag,
            num_iteraciones, 
            x_min, y_min, x_max, y_max, 
            width, height, 
            pixel_buffer);

     }