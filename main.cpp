#include <iostream>
#include <string>
#include <cstdlib>
#include "mpi.h"
// #include "clockcycle.h"


void create_graph_file(const std::string &filename, int rank, int size, int node_count, double p);
void create_graph_file_cuda(const std::string &filename, int rank, int size, int node_count, double p);

void cuda_init(int rank, int size, int node_count);
void generate_graph(int* h_buf, int buf_len, int* h_write_count, float h_p);

int main(int argc, char** argv) {


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

    if (device_type == "CPU" ) create_graph_file(filename, rank, size, node_count, density);
    else if (device_type == "CUDA") {
        cuda_init(rank, size, node_count);
        create_graph_file_cuda(filename, rank, size, node_count, density);
    } else {
        std::cerr << "Error: did not recognize device type." << std::endl;
        return EXIT_FAILURE;
    }


    MPI_Finalize();

    return 0;
}


// Uses Erdos-Renyi method to randomly generate graph
void create_graph_file(const std::string &filename, int rank, int size, int node_count, double p) {


    MPI_File fh;
    MPI_Status status;
    MPI_File_open(MPI_COMM_WORLD, filename.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);

    int nodes_per_rank = (node_count/size) + ((node_count % size) != 0);
    int start_node = rank * nodes_per_rank;
    int end_node = (start_node + nodes_per_rank) < node_count ? start_node + nodes_per_rank : node_count;

    // Create edges i --> j
    std::string edge_buf = "";
    for (int i = start_node; i < end_node; i++) {
        for (int j = 0; j < node_count; j++) {
            // std::cout << "edge: " << i << " --> " << j << std::endl;
            double rand = ((double)std::rand())/RAND_MAX;
            if (rand < p) {
                edge_buf += std::to_string(i) + " " + std::to_string(j) + "\n";
            }
            // std::cout << "rand: " << rand << std::endl;
        }
    }

    // Figure out cursor positions
    int cur_pos[size];
    int buf_size = edge_buf.size();
    MPI_Allgather(&buf_size, 1, MPI_INT, cur_pos, 1, MPI_INT, MPI_COMM_WORLD);
    for (int i = 1; i < size; i++) cur_pos[i] += cur_pos[i-1];

    // Write edges to file
    if (rank == 0) MPI_File_seek(fh, 0, MPI_SEEK_SET);
    else MPI_File_seek(fh, cur_pos[rank - 1], MPI_SEEK_SET);
    MPI_File_write_all(fh, edge_buf.c_str(), edge_buf.size(), MPI_CHAR, &status);

    MPI_File_close(&fh);
}

// Uses Erdos-Renyi method to randomly generate graph using cuda.
void create_graph_file_cuda(const std::string &filename, int rank, int size, int node_count, double p) {

    int nodes_per_rank = (node_count/size) + ((node_count % size) != 0);
    int start_node = rank * nodes_per_rank;
    int end_node = (start_node + nodes_per_rank) < node_count ? start_node + nodes_per_rank : node_count;

    // Allocate host memory.
    int buf_len = 2 * nodes_per_rank * node_count; /** wrong alloc length size **/
    int* h_buf = new int[buf_len]();
    int write_count;

    // Generate graph using cuda.
    generate_graph(h_buf, buf_len, &write_count, p);
    MPI_Barrier(MPI_COMM_WORLD);

    // Create edge buffer.
    std::string edge_buf = std::string();
    for (int i = 0; i < write_count; i+=2) edge_buf += std::to_string(h_buf[i]) + " " + std::to_string(h_buf[i+1]) + "\n";

    // Open up file.
    MPI_File fh;
    MPI_Status status;
    MPI_File_open(MPI_COMM_WORLD, filename.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);

    // Figure out cursor positions
    int cur_pos[size];
    int buf_size = edge_buf.size();
    MPI_Allgather(&buf_size, 1, MPI_INT, cur_pos, 1, MPI_INT, MPI_COMM_WORLD);
    for (int i = 1; i < size; i++) cur_pos[i] += cur_pos[i-1];

    // Write edges to file
    if (rank == 0) MPI_File_seek(fh, 0, MPI_SEEK_SET);
    else MPI_File_seek(fh, cur_pos[rank - 1], MPI_SEEK_SET);
    MPI_File_write_all(fh, edge_buf.c_str(), edge_buf.size(), MPI_CHAR, &status);

    MPI_File_close(&fh);

    delete [] h_buf;
}