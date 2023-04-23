#include <vector>
#include <iostream>
#include "common.h"

typedef std::vector<std::vector<char>> matrix;

void make_graph_from_file(const std::string &filename, matrix &graph);
std::pair<matrix, int> make_graph(int node_count, double density);
int find_celeb();

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int node_count, density;

    if (argc != 3) {
        std::cerr << "Usage: celeb_algo.out <node_count> <density>" << std::endl;
        return EXIT_FAILURE;
    } else {
        node_count = std::stoi(argv[1]);
        density = std::stod(argv[2]);
    }


    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::srand(1230128093);


    std::pair<matrix, int> pair = make_graph(node_count, density);
    matrix graph = pair.first;
    int celeb = pair.second;
    
    std::cout << "celeb: " << celeb << std::endl;
    

    // for (int i = 0; i < graph.size(); i++) {
    //     for (int j = 0; j < graph[i].size(); j++) {
    //         printf("edge %d --> %d: %d\n", i, j, graph[i][j]); 
    //     }
    // }


    MPI_Finalize();

    return EXIT_SUCCESS;
}

std::pair<matrix, int> make_graph(int node_count, double density) {

    matrix graph(node_count, std::vector<char>(node_count, 0));
    int celeb = std::rand() % node_count;

    for (int i = 0; i < node_count; i++) {
        
        /* add celeb to graph*/
        if (i == celeb) {
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

    return std::make_pair(graph, celeb);
}