#include <iostream>
#include <string>
#include <cstdlib>
#include "mpi.h"

#ifndef NOT_AIMOS
    #include "clockcycle.h"
    #define CLOCK_FREQ 512000000
#endif



void create_vote_file(const std::string &filename, int rank, int size, int vote_count, int candidate_count);
std::string create_vote_sequence(int candidate_count);

int main(int argc, char** argv) {

    #ifdef NOT_AIMOS
        auto clock_now = []{ return 0; };
    #endif

    MPI_Init(&argc, &argv);

    std::string filename;
    int vote_count;
    double candidate_count;
    std::string device_type;

    if (argc != 5) {
        std::cerr << "Usage: vote_gen.out <filename> <vote_count> <candidate_count> <'CPU' or 'CUDA'>" << std::endl;
        return EXIT_FAILURE;
    } else {
        filename = argv[1];
        vote_count = std::stoi(argv[2]);
        candidate_count = std::stod(argv[3]);
        device_type = argv[4];
    }

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::srand(1230128093 + rank << 2);

    if (device_type == "CPU" ) create_vote_file(filename, rank, size, vote_count, candidate_count);
    else {
        std::cerr << "Error: did not recognize device type." << std::endl;
        return EXIT_FAILURE;
    }

    MPI_Finalize();

    return 0;
}


/* Uses Fisher-Yates shuffle algorithm: https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle */
std::string create_vote_sequence(int candidate_count) {

    std::string output = "";
    int map[candidate_count];
    for (int i = 1; i <= candidate_count; i++) map[i-1] = i;

    for (int i = candidate_count; i >= 1; i--) {
        int j = 1 + std::rand() % i;
        output += std::to_string(map[j-1]) + ":";
        map[j-1] = map[i-1];
    }
    if (!output.empty()) output.pop_back();
    return output;
}


/* Create file containing a ton of votes. */
/* Candidates go from 1..candidate_count, voters go from 0..vote_count*/
void create_vote_file(const std::string &filename, int rank, int size, int vote_count, int candidate_count) {

    MPI_File fh;
    MPI_Status status;
    MPI_File_open(MPI_COMM_WORLD, filename.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);

    int votes_per_rank = (vote_count/size) + ((vote_count % size) != 0);
    int start_voter_id= rank * votes_per_rank;
    int end_voter_id = (start_voter_id + votes_per_rank) < vote_count ? start_voter_id + votes_per_rank : vote_count;

    // Create votes
    std::string vote_buffer = "";
    for (int i = start_voter_id; i < end_voter_id; i++) {
        std::string vote_sequence = create_vote_sequence(candidate_count);
        vote_buffer += std::to_string(i) + ":" + vote_sequence + "\n";
    }

    // Figure out cursor positions
    int cur_pos[size];
    int buf_size = vote_buffer.size();
    MPI_Allgather(&buf_size, 1, MPI_INT, cur_pos, 1, MPI_INT, MPI_COMM_WORLD);
    for (int i = 1; i < size; i++) cur_pos[i] += cur_pos[i-1];

    // Write edges to file
    if (rank == 0) MPI_File_seek(fh, 0, MPI_SEEK_SET);
    else MPI_File_seek(fh, cur_pos[rank - 1], MPI_SEEK_SET);
    MPI_File_write_all(fh, vote_buffer.c_str(), vote_buffer.size(), MPI_CHAR, &status);

    MPI_File_close(&fh);
}