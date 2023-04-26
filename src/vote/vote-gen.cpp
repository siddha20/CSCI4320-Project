#include <string>
#include <vector>
#include <iostream>
#include <numeric>
#include "common.h"
#include "probability.h"
#include "crypto.h"

using namespace probability;
using namespace crypto;

void uniform_test();
void create_vote_file(MPI_File fh, int rank, int size, int vote_count, const Distribution &dist);
std::string create_vote_sequence(const Distribution &dist);

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    std::string filename;
    int vote_count;
    double candidate_count;
    std::string distribution_type;
    Distribution dist;
    bool delete_file = false;

    std::string usage_error = "Usage: vote_gen.out <filename> <vote_count> <candidate_count> <UNIFORM or FULLBIAS ID> <OPT: DELETE>";

    if (argc < 5) {
        std::cerr << usage_error << std::endl;
        return EXIT_FAILURE;
    } else {
        filename = argv[1];
        vote_count = std::stoi(argv[2]);
        candidate_count = std::stod(argv[3]);
        distribution_type = argv[4];
    }

    /* Use a uniform distribution when sampling permutations. */
    if (distribution_type == "UNIFORM") {
        dist = Uniform(candidate_count);
        if (argc == 6) {
            if (std::string(argv[5]) == "DELETE") {
                delete_file = true;
            }
        }
    }
    /* Use a full bias distribution when sampling permutations. */
    else if (argc >= 6 && distribution_type == "FULLBIAS") {
        dist = FullBias(candidate_count, std::stoi(argv[5]) - 1);
        if (argc == 7) {
            if (std::string(argv[6]) == "DELETE") {
                delete_file = true;
            }
        }
    }
    else {
        std::cerr << usage_error << std::endl;
    }



    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_File fh;
    MPI_File_open(MPI_COMM_WORLD, filename.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);

    std::srand(1230128093 + rank << 2);

    /* Timer for total time. */
    Timer t1 = Timer(std::to_string(size) + ":" + "total_time");

    /* Create the vote file. */
    t1.start();
    create_vote_file(fh, rank, size, vote_count, dist);
    t1.end();

    if (rank == 0) t1.print_duration_cycles_label_only();

    if (rank == 0 && delete_file) {
        std::remove(filename.c_str());
    }

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
void create_vote_file(MPI_File fh, int rank, int size, int vote_count, const Distribution &dist) {

    const auto [votes_per_rank, start_voter_id, end_voter_id] = partition(vote_count, rank, size);

    size_t candidate_count = dist.get_size();

    // Create votes
    std::string vote_str = "";
    if (rank == 0) vote_str = std::to_string(vote_count) + ":" + std::to_string(candidate_count) + "\n";

    Timer t2 = Timer(std::to_string(size) + ":" + "permutation_create_time");

    t2.start();
    for (int i = start_voter_id; i < end_voter_id; i++) {
        std::string vote_sequence = create_random_permutation(dist);
        vote_str += std::to_string(i) + ":" + vote_sequence + "\n";
    }
    t2.end();

    if (rank == 0) t2.print_duration_cycles_label_only();

    const int overlap_size = votes_per_rank * OVERLAP_MULTIPLIER * (vote_count + ((candidate_count + 1) * std::to_string(candidate_count).length()));
    vote_str += std::string(std::max(0, overlap_size - (int)vote_str.size()), '#');

    MPI_Barrier(MPI_COMM_WORLD);

    Timer t3(std::to_string(size) + ":" + "encrypt_time");
    t3.start();
    size_t enc_offset = rank * 16 + 1000000;
    buf_t encrypted;
    Crypto cryptor({ ENC_IV }, { ENC_KEY_CUR }, BLOCKS_PER_HASH);
    cryptor.Encrypt((const u8 *)vote_str.data(), vote_str.length(), enc_offset, encrypted);
    t3.end();

    if (rank == 0) t3.print_duration_cycles_label_only();

    // Figure out cursor positions
    size_t cur_pos[size];
    size_t buf_size = encrypted.size();

    MPI_Datatype mpi_type = get_mpi_type<size_t>();
    MPI_Allgather(&buf_size, 1, mpi_type, cur_pos, 1, mpi_type, MPI_COMM_WORLD);
    for (size_t i = 1; i < size; i++) cur_pos[i] += cur_pos[i-1];

    // Write edges to file
    if (rank == 0) {
        MPI_File_seek(fh, 0, MPI_SEEK_SET);
        MPI_File_write(fh, (const char*)&overlap_size, sizeof(overlap_size), MPI_CHAR, NULL);
        MPI_File_seek(fh, sizeof(overlap_size), MPI_SEEK_SET);
    } else MPI_File_seek(fh, cur_pos[rank - 1] + sizeof(overlap_size), MPI_SEEK_SET);
    MPI_File_write_all(fh, encrypted.data(), encrypted.size(), MPI_CHAR, NULL);

    MPI_File_close(&fh);
}

/* Tests to test uniform distribution. */
void uniform_test(size_t size) {
    Distribution dist = Uniform(size);
    std::cout << dist.get_index(.03) << std::endl;
    std::cout << dist.get_index(.22) << std::endl;
    std::cout << dist.get_index(.29) << std::endl;
    std::cout << dist.get_index(.47) << std::endl;
    std::cout << dist.get_index(1.0) << std::endl;
    std::cout << dist.get_index(.69) << std::endl;
}