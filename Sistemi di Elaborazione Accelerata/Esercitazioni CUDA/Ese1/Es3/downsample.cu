#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <string.h>
#include <time.h>

// Include STB image libraries
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Error checking macro
#define CHECK(call) \
{ \
    const cudaError_t error = call; \
    if (error != cudaSuccess) \
    { \
        fprintf(stderr, "Error: %s:%d, ", __FILE__, __LINE__); \
        fprintf(stderr, "code: %d, reason: %s\n", error, cudaGetErrorString(error)); \
        exit(1); \
    } \
}

// Kernel downsampling 2×2 → 1
__global__ void downsample2x2(unsigned char* d_input, unsigned char* d_output,
                               int input_width, int input_height, int channels)
{
    
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x < input_width/2 && y < input_height/2) {
        //printf("x: %i,y: %i\n",x,y);
        int index_output = (y * (input_width/2) + x) * channels;
        for(int c = 0; c < channels; c++){
            int result = 0;
            for(int i = 0; i < 2; i++){
                for(int j = 0; j < 2; j++){
                    int index_input = ((y*2+i) * input_width + (x*2+j)) * channels;
                    //printf("index input: %i\n",index_input);
                    result +=  d_input[index_input+c];
                    
                }
            }
            d_output[index_output+c] = result/4;
        }
    }
}

// Versione CPU naive per confronto
void downsample2x2_cpu(unsigned char* input, unsigned char* output,
                       int input_width, int input_height, int channels)
{
    
    for(int y = 0; y < input_height/2; y++){
        for(int x = 0; x < input_width/2; x++){
            //printf("x: %i,y: %i\n",x,y);
            int index_output = (y * (input_width/2) + x) * channels;
            for(int c = 0; c < channels; c++){
                int result = 0;
                for(int i = 0; i < 2; i++){
                    for(int j = 0; j < 2; j++){
                        int index_input = ((y*2+i) * input_width + (x*2+j)) * channels;
                        //printf("index input: %i\n",index_input);
                        result +=  input[index_input+c];
                    }
                }
                output[index_output+c] = result/4;
            }
        }
    }

}

// Funzione per verificare correttezza
bool verifyResults(unsigned char* cpu_result, unsigned char* gpu_result, 
                   int size)
{
    int errors = 0;
    for (int i = 0; i < size; i++) {
        // Tolleriamo differenze di ±1 dovute ad arrotondamenti
        int diff = abs((int)cpu_result[i] - (int)gpu_result[i]);
        if (diff > 1) {
            errors++;
            if (errors <= 5) {
                printf("Mismatch at index %d: CPU=%d, GPU=%d (diff=%d)\n", 
                       i, cpu_result[i], gpu_result[i], diff);
            }
        }
    }
    
    if (errors > 0) {
        printf("Total errors: %d / %d (%.2f%%)\n", 
               errors, size, 100.0f * errors / size);
    }
    
    return errors == 0;
}

int main(int argc, char **argv) 
{
    if (argc < 3) {
        printf("Usage: %s <input_image> <block_size>\n", argv[0]);
        printf("Example: %s input.png 16\n", argv[0]);
        printf("Note: Input image dimensions must be even numbers\n");
        return 1;
    }

    const char* inputFile = argv[1];
    int blockSize = atoi(argv[2]);

    // ========== Caricamento immagine ==========
    int width, height, channels;
    unsigned char* h_input = stbi_load(inputFile, &width, &height, &channels, 0);
    if (!h_input) {
        printf("Error loading image %s\n", inputFile);
        return 1;
    }
    printf("Image loaded: %dx%d with %d channels\n", width, height, channels);

    // Verifica che le dimensioni siano pari
    if (width % 2 != 0 || height % 2 != 0) {
        printf("Warning: Image dimensions should be even for 2x downsampling\n");
        printf("Output will use dimensions: %dx%d\n", width/2, height/2);
    }

    // ========== Allocazione memoria ==========
    
    int size = width * height * channels;
    int size_reduced = (width/2) * (height/2) * channels;

    unsigned char* h_output_cpu = (unsigned char*) malloc(size_reduced*sizeof(unsigned char));
    unsigned char* h_output_gpu = (unsigned char*) malloc(size_reduced*sizeof(unsigned char));

    unsigned char* d_input; // Dichiarazione di un puntatore per la memoria sulla GPU (device)
    cudaMalloc((void**)&d_input, size*sizeof(unsigned char)); // Allocazione della memoria sulla GPU

    // Copia dei dati dalla memoria dell'host (CPU) alla memoria del device (GPU)
    cudaError_t err = cudaMemcpy(d_input, h_input, size*sizeof(unsigned char), cudaMemcpyHostToDevice);
    // Controlla se la copia è avvenuta con successo
    if (err != cudaSuccess) {
        // Se c'è un errore, stampa un messaggio di errore e termina il programma
        fprintf(stderr, "Errore nella copia H2D: %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    unsigned char* d_output; 
    cudaMalloc((void**)&d_output, size_reduced*sizeof(unsigned char));


    // ========== Configurazione kernel ==========
    
    int grid_x,grid_y;
    grid_x = width/2/blockSize+1;
    grid_y = height/2/blockSize+1;
    dim3 grid_size(grid_x,grid_y);
    dim3 block_size(blockSize,blockSize);


    // ========== Esecuzione GPU ==========
    
    clock_t gpu_start = clock();
    downsample2x2<<<grid_size, block_size>>>(d_input,d_output,width,height,channels);
    cudaDeviceSynchronize();
    err = cudaMemcpy(h_output_gpu,d_output, size_reduced*sizeof(char), cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) {
        // Se c'è un errore, stampa un messaggio di errore e termina il programma
        fprintf(stderr, "Errore nella copia D2H: %s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    clock_t gpu_end = clock();
    double gpu_time = ((double)(gpu_end - gpu_start)) / CLOCKS_PER_SEC * 1000.0;
    printf("Tempo GPU: %.3f ms\n", gpu_time);


    // ========== Esecuzione CPU ==========
    printf("\nEsecuzione CPU...\n");
    
    clock_t cpu_start = clock();
    downsample2x2_cpu(h_input, h_output_cpu, width, height, channels);
    clock_t cpu_end = clock();
    
    double cpu_time = ((double)(cpu_end - cpu_start)) / CLOCKS_PER_SEC * 1000.0;
    printf("Tempo CPU: %.3f ms\n", cpu_time);

    // ========== Verifica correttezza ==========
    printf("\nVerifica correttezza...\n");
    bool correct = verifyResults(h_output_cpu, h_output_gpu, size_reduced);
    
    if (correct) {
        printf("✓ Test PASSATO: GPU e CPU producono lo stesso risultato\n");
    } else {
        printf("✗ Test FALLITO: GPU e CPU producono risultati diversi\n");
    }

    // ========== Salvataggio immagini ==========
    stbi_write_png("output_gpu.png", width/2, height/2, channels, 
                   h_output_gpu, (width/2) * channels);
    stbi_write_png("output_cpu.png", width/2, height/2, channels, 
                   h_output_cpu, (width/2) * channels);
    printf("\nImmagini salvate: output_gpu.png, output_cpu.png\n");
    printf("Dimensioni output: %dx%d (%.1f%% dell'originale)\n", 
           width/2, height/2, 
           100.0 * size_reduced / size);

    // ========== Cleanup ==========
    
    // TODO


    return 0;
}
