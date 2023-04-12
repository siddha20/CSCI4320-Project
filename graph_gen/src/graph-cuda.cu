#include <stdio.h>
#include <iostream>
#include "curand_kernel.h"

#define WORD_LENGTH 5

int node_count;
int size;
int rank;

__device__ int mutex = 0;

void cuda_init(int l_rank, int l_size, int l_node_count) {

    node_count = l_node_count;
    size = l_size;
    rank = l_rank;

    int device_count;
    cudaError_t cE;
    
    // Get cuda device count
    if((cE = cudaGetDeviceCount(&device_count)) != cudaSuccess) {
        printf(" Unable to determine cuda device count, error is %d, count is %d\n", cE, device_count);
        exit(-1);
    }

    // Assign rank to cuda device.
    if((cE = cudaSetDevice(rank % device_count)) != cudaSuccess) {
        printf(" Unable to have rank %d set to cuda device %d, error is %d \n", rank, (rank % device_count), cE);
        exit(-1);
    }
    
    printf("Rank %d attached to CUDA device %d.\n", rank, (rank % device_count));

}

__global__ void test_write_kernel(int* data) {
    data[threadIdx.x] = threadIdx.x;
}

__global__ void setup_kernel(curandState* state, int seed) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    curand_init(seed, idx, 0, state + idx);
}

__global__ void generate_graph_kernel(int* data, int* write_count, int buf_len, curandState* state, float p, 
                                      int node_count, int nodes_per_rank, int start_node, int end_node) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int i = start_node + idx;

    // printf("write count: %d, buf_len %d\n", *write_count, buf_len);
    if (*write_count >= buf_len) return;

    if (i < end_node) {
        // Create edges i --> j
        for (int j = 0; j < node_count; j++) {
            float rand = curand_uniform(state + idx);
            if (rand <= p) {
                int old = 1;
                while (old) {
                    old = atomicCAS(&mutex, 0, 1); // lock
                    if (old == 0){
                        data[(*write_count)] = i;
                        data[(*write_count) + 1] = j;
                        // printf("edge %d --> %d, %f\n", data[(*write_count)], data[(*write_count) + 1], rand);
                        (*write_count) += 2;
                        // printf("write count %d\n", *write_count);
                        __threadfence();

                        atomicExch(&mutex, 0); // unlock
                    }
                }
            }
        }
        __syncthreads();
    }
}


void generate_graph(int* h_buf, int buf_len, int* h_write_count, float h_p) {

    int nodes_per_rank = (node_count/size) + ((node_count % size) != 0);
    int start_node = rank * nodes_per_rank;
    int end_node = (start_node + nodes_per_rank) < node_count ? start_node + nodes_per_rank : node_count;

    // Allocate memory on device. 
    int* d_buf;
    cudaMalloc(&d_buf, buf_len * sizeof(int));
    cudaMemset(d_buf, 0, buf_len * sizeof(int));

    int* d_write_count;
    cudaMalloc(&d_write_count, sizeof(int));
    cudaMemset(d_write_count, 0, sizeof(int));

    curandState* devStates;
    cudaMalloc (&devStates, nodes_per_rank * sizeof(curandState));

    test_write_kernel<<<1, buf_len>>>(d_buf);

    int threads = 1024;
    int blocks = (nodes_per_rank + threads - 1)/threads;
    int seed = 39483 + 2 << rank;
    float p = h_p;

    // Setup the random number generator thing idk. 
    setup_kernel<<<blocks, threads>>>(devStates, seed);

    // Generate the graph.
    generate_graph_kernel<<<blocks, threads>>>(d_buf, d_write_count, buf_len, devStates, p, 
        node_count, nodes_per_rank, start_node, end_node);

    // Copy device memory to host memory.
    cudaMemcpy(h_buf, d_buf, buf_len * sizeof(int), cudaMemcpyDeviceToHost);
    cudaMemcpy(h_write_count, d_write_count, sizeof(int), cudaMemcpyDeviceToHost);
    cudaDeviceSynchronize();



    cudaFree(d_buf);
    cudaFree(devStates);
}
