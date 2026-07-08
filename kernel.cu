const int PALETTE_SIZE = 16;
__constant__
unsigned int color_ramp[PALETTE_SIZE];

__global__
void julia_kernel(
    double centro_real, double centro_imag,
    int num_iteraciones,
    double x_min, double y_min, double x_max, double y_max, 
    uint32_t width,
     uint32_t height, 
     uint32_t* pixel_buffer) {
        int idx = blockIdx.x * blockDim.x + threadIdx.x;
        if (idx >= width * height) return;

        int x = idx % width;
        int y = idx / width;

        double real = x_min + (x_max - x_min) * x / width;
        double imag = y_min + (y_max - y_min) * y / height;

        int max_iter = num_iteraciones;
        int iter = 0;
        while (real * real + imag * imag < 4.0 && iter < max_iter) {
            double temp_real = real * real - imag * imag + centro_real;
            imag = 2.0 * real * imag + centro_imag;
            real = temp_real;
            iter++;
        }

        pixel_buffer[idx] = iter == max_iter ? 0 : iter; // Store the iteration count
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