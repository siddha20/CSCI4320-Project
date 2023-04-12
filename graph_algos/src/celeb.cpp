#include <string>
#include <cstdlib>
#include <vector>
#include <utility>
#include "mpi.h"

#ifndef NOT_AIMOS
    #include "clockcycle.h"
    #define CLOCK_FREQ 512000000
#endif

typedef std::vector<std::vector<char>> matrix;

void make_graph_from_file(const std::string &filename, matrix &graph);
std::pair<matrix, std::vector<int>> make_graph(int node_count, double density, int celeb_count);

int main(int argc, char** argv) {

    #ifdef NOT_AIMOS
        auto clock_now = []{ return 0; };
    #endif

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::srand(1230128093);


    std::pair<matrix, std::vector<int>> pair = make_graph(10, 1.0, 1);
    matrix graph = pair.first;
    std::vector celebs = pair.second;

    for (int i = 1; auto c : celebs) {
        std::cout << "celeb " << i++ << " :" << c << std::endl;
    }

    for (int i = 0; i < graph.size(); i++) {
        for (int j = 0; j < graph[i].size(); j++) {
            printf("edge %d --> %d: %d\n", i, j, graph[i][j]); 
        }
    }


    MPI_Finalize();

    return EXIT_SUCCESS;
}

std::pair<matrix, std::vector<int>> make_graph(int node_count, double density, int celeb_count) {

    matrix graph(node_count, std::vector<char>(node_count, 0));
    std::vector<int> celebs;

    for (int i = 0; i < celeb_count; i++) celebs.push_back(std::rand() % node_count);

    for (int i = 0; i < node_count; i++) {
        
        /* add celeb to graph*/
        if (std::find(celebs.begin(), celebs.end(), i) != celebs.end()) {
            for (int j = 0; j < node_count; j++) {
                if (j != i) graph[j][i] = 1;
            }
        } 
        
        /* add non celeb to graph*/
        else {
            for (int j = 0; j < node_count; j++) {
                double rand = ((double)std::rand())/RAND_MAX;
                if (rand <= density) {
                    if (i != j) graph[i][j] = 1;
                }
            }
        }
    }

    return std::make_pair(graph, celebs);
}