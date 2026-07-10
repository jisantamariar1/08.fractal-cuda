const int PALETTE_SIZE = 16;

__constant__ //variable de solo lectura en la memoria de la GPU
unsigned int color_ramp[PALETTE_SIZE];

//****************************** */
__device__ //funcion en la GPU, solo accesible desde la GPU
uint32_t acotado(double x, double y, 
    double c_r, double c_i, 
    int max_iteraciones){
    /*
    dados: c, z0
    Zn+1 = Zn^2 + c
    */

    int iter = 1;

    double zr = x;
    double zi = y;

    while (iter < max_iteraciones && (zr*zr+zi*zi) < 4.0)
    {
        double dr = zr*zr-zi*zi+c_r;
        double di = 2.0*zr*zi+c_i;
        zr = dr;
        zi = di;

        iter ++;

    }
    if(iter < max_iteraciones){
        // nomras > 2
        int index = iter % PALETTE_SIZE;
        return color_ramp[index];
    }
    
    // los bits esta alreves en cuanto a los colores
    return 0xFF000000; // negro

}

//******************************* */

__global__ //funcion en la GPU
void julia_kernel(
    double centro_real, double centro_imag,
    int num_iteraciones,
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width,
    uint32_t heigt,
    uint32_t *pixel_buffer)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / heigt;
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < width * heigt){
          // z = x+yi = (x,y)
          int i =index % width;
          int j = index / width;
            double x = x_min + i * dx;
            double y = y_max - j * dy;

            //std::complex<double> z(x, y);

            // similar al var
            auto color = acotado(x,y, centro_real, centro_imag, num_iteraciones);

            //index j*w + i
            pixel_buffer[j * width + i] = color;

    }
          



}


//---------------cpu
void copiar_paleta(unsigned int *pallete_host)
{
    //forma de copiar variables del host a la gpu  cudaMemcpyToSymbol-- copiar la paleta desde la cpu a la gpu
    cudaMemcpyToSymbol(color_ramp, pallete_host, sizeof(unsigned int) * PALETTE_SIZE);
}


void julia_gpu(
    double centro_real, double centro_imag,
    int num_iteraciones,
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width,
    uint32_t heigt,
    uint32_t *pixel_buffer)
{
    int threads_per_block = 1024;
    int blocks_per_grid = std::ceil((width * heigt) * 1.0 / threads_per_block);
    
    julia_kernel<<<blocks_per_grid, threads_per_block>>>(
        centro_real, centro_imag, 
        num_iteraciones, 
        x_min, y_min, x_max, y_max, 
        width, heigt, 
        pixel_buffer);
}