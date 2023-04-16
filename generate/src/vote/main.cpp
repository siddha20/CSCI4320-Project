#include <iostream>
#include <string>
#include <cstdlib>
#include "mpi.h"

#ifndef NOT_AIMOS
    #include "clockcycle.h"
    #define CLOCK_FREQ 512000000
#endif


int main(int argc, char** argv) {

    #ifdef NOT_AIMOS
        auto clock_now = []{ return 0; };
    #endif

    MPI_Init(&argc, &argv);

    std::string filename;
    int node_count;
    double density;
    std::string device_type;

    if (argc != 5) {
        std::cerr << "Usage: a.out <filename> <node_count> <density> <'CPU' or 'CUDA'>" << std::endl;
        return EXIT_FAILURE;
    } else {
        filename = argv[1];
        node_count = std::stoi(argv[2]);
        density = std::stod(argv[3]);
        device_type = argv[4];
    }

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::srand(1230128093 + rank << 2);


    MPI_Finalize();

    return 0;
}