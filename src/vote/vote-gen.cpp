#include <string>
#include <vector>
#include <iostream>
#include <numeric>
#include "common.h"
#include "probability.h"

using namespace probability;

void uniform_test();
void create_vote_file(const std::string &filename, int rank, int size, int vote_count, const Distribution &dist);
std::string create_vote_sequence(const Distribution &dist);

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    std::string filename;
    int vote_count;
    double candidate_count;
    std::string distribution_type;
    Distribution dist;

    std::string usage_error = "Usage: vote_gen.out <filename> <vote_count> <candidate_count> <UNIFORM or FULLBIAS ID>";

    if (argc < 5) {
        std::cerr << usage_error << std::endl;
        return EXIT_FAILURE;
    } else {
        filename = argv[1];
        vote_count = std::stoi(argv[2]);
        candidate_count = std::stod(argv[3]);
        distribution_type = argv[4];
    }

    if (distribution_type == "UNIFORM") {
        dist = Uniform(candidate_count);
    }
    else if (argc == 6 && distribution_type == "FULLBIAS") {
        dist = FullBias(candidate_count, std::stoi(argv[5]) - 1);
    }
    else {
        std::cerr << usage_error << std::endl;
    }

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::srand(1230128093 + rank << 2);

    Timer t1 = Timer("file creation");

    t1.start();
    create_vote_file(filename, rank, size, vote_count, dist);
    t1.end();

    if (rank == 0) t1.print_duration_sec();

    MPI_Finalize();

    return 0;
}


/* Uses Fisher-Yates shuffle algorithm: https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle */
std::string create_random_permutation(const Distribution &dist) {

    size_t permutation_size = dist.get_size();
    std::string output = "";
    int map[permutation_size];
    for (int i = 1; i <= permutation_size; i++) map[i-1] = i;

    for (int i = permutation_size; i >= 1; i--) {
        int j = 1 + std::rand() % i;

        // Only change first index
        if (i == permutation_size) {
            double f1 = ((double)std::rand())/RAND_MAX;
            j = (int)dist.get_index(f1) + 1;
        }
        output += std::to_string(map[j-1]) + ":";
        map[j-1] = map[i-1];
    }
    if (!output.empty()) output.pop_back();
    return output;
}

/* Create file containing a ton of votes. */
/* Candidates go from 1..candidate_count, 
   voters go from 0..vote_count */
void create_vote_file(const std::string &filename, int rank, int size, int vote_count, const Distribution &dist) {

    MPI_File fh;
    MPI_Status status;
    MPI_File_open(MPI_COMM_WORLD, filename.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);

    const auto [votes_per_rank, start_voter_id, end_voter_id] = partition(vote_count, rank, size);

    size_t candidate_count = dist.get_size();

    // Create votes
    std::string vote_buffer = "";
    if (rank == 0) vote_buffer = std::to_string(vote_count) + ":" + std::to_string(candidate_count) + "\n";

    for (int i = start_voter_id; i < end_voter_id; i++) {
        std::string vote_sequence = create_random_permutation(dist);
        vote_buffer += std::to_string(i) + ":" + vote_sequence + "\n";
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Figure out cursor positions
    size_t cur_pos[size];
    size_t buf_size = vote_buffer.size();

    MPI_Datatype mpi_type = get_mpi_type<size_t>();
    MPI_Allgather(&buf_size, 1, mpi_type, cur_pos, 1, mpi_type, MPI_COMM_WORLD);
    for (int i = 1; i < size; i++) cur_pos[i] += cur_pos[i-1];

    // Write edges to file
    if (rank == 0) MPI_File_seek(fh, 0, MPI_SEEK_SET);
    else MPI_File_seek(fh, cur_pos[rank - 1], MPI_SEEK_SET);
    MPI_File_write_all(fh, vote_buffer.c_str(), vote_buffer.size(), MPI_CHAR, &status);

    MPI_File_close(&fh);
}


void uniform_test(size_t size) {
    Distribution dist = Uniform(size);
    std::cout << dist.get_index(.03) << std::endl;
    std::cout << dist.get_index(.22) << std::endl;
    std::cout << dist.get_index(.29) << std::endl;
    std::cout << dist.get_index(.47) << std::endl;
    std::cout << dist.get_index(1.0) << std::endl;
    std::cout << dist.get_index(.69) << std::endl;
}