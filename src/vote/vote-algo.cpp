#include <map>
#include <algorithm>
#include <utility>
#include <string>
#include <vector>
#include <iostream>
#include "common.h"
#include "crypto.h"

#define FIRST_LINE_BUFFER_SIZE 256
#define BUFFER_SIZE 10000

using namespace crypto;

struct RankInfo {
    int rank;
    int size;
    int vote_count;
    int candidate_count;
    MPI_Offset file_size;
    int first_line_size;
};

std::vector<int> tokenize_line(const std::string &line, char delimitier);

void read_first_line(MPI_File fh, RankInfo &rank_info, size_t overlap_size);
void get_start_and_end_bytes(const RankInfo &rank_info, size_t &start_byte, size_t &end_byte);
buf_t decrypt(const char *buffer, size_t rank, size_t overlap_size);

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    std::string input_filename;

    if (argc != 2) {
        std::cerr << "Usage: vote_algo.out <input_filename>" << std::endl;
        return EXIT_FAILURE;
    } else {
        input_filename = argv[1];
    }

    RankInfo rank_info;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_info.rank);
    MPI_Comm_size(MPI_COMM_WORLD, &rank_info.size);
    int rank = rank_info.rank;
    int size = rank_info.size;


    /* Total timer for the whole algorithm. */
    Timer total_time(std::to_string(size) + ":" + "total_time");
    total_time.start();
    
    std::srand(1230128093 + rank << 2);

    // Open up the file.
    MPI_File fh;
    MPI_Status status;
    MPI_File_open(MPI_COMM_WORLD, input_filename.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);

    size_t overlap_size;
    MPI_File_read_at_all(fh, 0, &overlap_size, sizeof(int), MPI_CHAR, &status);

    size_t num_hashes;
    MPI_File_read_at_all(fh, sizeof(int), &num_hashes, sizeof(size_t), MPI_CHAR, &status);
    overlap_size += sizeof(size_t) + 20 * num_hashes;

    // Read the first line an get data.
    read_first_line(fh, rank_info, overlap_size);

    // printf("rank %d start_byte %d end_byte %d buffer size %d\n", rank, start_byte, end_byte, buffer_size);
    char* buffer = new char[overlap_size]();
    MPI_File_read_at_all(fh, sizeof(int) + rank * overlap_size, buffer, overlap_size, MPI_CHAR, &status);
    
    /* Timer for decryption time. */
    Timer t3(std::to_string(size) + ":" + "decrypt_time");
    t3.start();
    
    buf_t decrypted = decrypt(buffer, rank, overlap_size);
    if (rank == 0)
    {
        std::string tmp;
        int after = get_line(tmp, (char*)&decrypted[0], decrypted.size(), 0);
        decrypted.erase(decrypted.begin(), decrypted.begin() + after - 1);
    }
    size_t buffer_size = decrypted.size();
    delete[] buffer;
    buffer = (char*)&decrypted[0];
    
    t3.end();
    if (rank == 0) t3.print_duration_cycles_label_only();

    /* Read in all the lines and populate the data map. */
    std::vector<int> voters;
    std::map<int, std::vector<int> > data;
    int cursor = 0;
    std::string line;
    while((cursor = get_line(line, buffer, buffer_size, cursor)) != -1) {
        //std::cout << "Line: " << line << '\n';
        std::vector<int> nums = tokenize_line(line, ':');
        if (nums.size() == rank_info.candidate_count + 1) {
            voters.push_back(nums[0]);
            data[nums[0]] = std::vector<int>(nums.begin() + 1, nums.end());
            // std::cout << line << std::endl;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Find distinct voters per rank.
    std::vector<int> voter_diff;
    if (rank == size - 1) voter_diff = voters;
    print_vec(voter_diff);
    printf("lol\n");

    for (int j = 0; j < size - 1; j++) {
        std::vector<int> temp;
        int voter_size;

        if (rank == j + 1) MPI_Send_vec(voters, j);

        if (rank == j) { 
            MPI_Recv_vec(temp, j + 1);

            // Get the difference between the two vectors. 
            std::set_difference(voters.begin(), voters.end(), temp.begin(), temp.end(), 
                std::inserter(voter_diff, voter_diff.begin()));
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Create rank only graph
    int c = rank_info.candidate_count;
    int graph_size = c * c;
    int *graph = new int[graph_size]();
    for (auto voter : voter_diff) {
        std::vector<int> candidates = data[voter];

        for (int i = 0; i < candidates.size(); i++) {
            for (int j = i + 1; j < candidates.size(); j++) {
                int row = candidates[i] - 1;
                int col = candidates[j] - 1;
                graph[row * c + col]++;
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Create final graph using reduction. 
    int* final_graph = new int[graph_size]();
    MPI_Reduce(graph, final_graph, graph_size, get_mpi_type<int>(), MPI_SUM, 0, MPI_COMM_WORLD);
    delete [] graph;

    /* Schulze Method is  performed here. We compute the candidate listings
    using the preference graph on rank 0. */
    if (rank == 0) {

        // std::cout << "Preference graph:" << std::endl;
        // print_array_2d(final_graph, c, c);
        // std::cout << std::endl;

        // int* strength_graph = new int[graph_size]();
        std::vector<int> strength_graph(graph_size, 0);

        for (int i = 0; i < c; i++) {
            for (int j = 0; j < c; j++) {
                if (i != j) {
                    if (final_graph[i * c + j] > final_graph[j * c + i]) {
                        strength_graph[i * c + j] = final_graph[i * c + j];
                    } else {
                        strength_graph[i * c + j] = 0;
                    }
                }
            }
        }

        for (int i = 0; i < c; i++) {
            for (int j = 0; j < c; j++) {
                if (i != j) {
                    for (int k = 0; k < c; k++) {
                        if (i != k && j != k) {
                            strength_graph[j * c + k] = std::max(strength_graph[j * c + k],
                                                                            std::min(strength_graph[j * c + i],
                                                                                        strength_graph[i * c + k]));
                        }
                    }
                }
            }
        }

        // std::cout << "Strength graph: " << std::endl;
        // print_vec_2d(strength_graph, c, c);
        // std::cout << std::endl;

        std::vector<std::pair<int, int>> ranking;
        for (int i = 0; i < c; i++) {
            int count = 0;
            for (int j = 0; j < c; j++) {
                if (i != j) {
                    if (strength_graph[i * c + j] > strength_graph[j * c + i]) count++;
                }
            }
            ranking.push_back(std::make_pair(i, count));
        }

        // std::cout << "Candidate ranking: " << std::endl;
        // std::sort(ranking.begin(), ranking.end(), [](auto& a, auto& b) { return a.second > b.second; });
        // for (auto [candidate, _] : ranking) {
        //     std::cout << candidate + 1 << " ";
        // }
        // std::cout << std::endl;
        // delete [] strength_graph;
    }

    delete [] final_graph;

    MPI_File_close(&fh);
    MPI_Finalize();
    
    total_time.end();
    if (rank == 0) total_time.print_duration_cycles_label_only();

    return EXIT_SUCCESS;
}



buf_t decrypt(const char *buffer, size_t rank, size_t overlap_size) {
    size_t enc_offset = rank * 16 + 1000000;
    buf_t decrypted;
    Crypto cryptor({ ENC_IV }, { ENC_KEY_CUR }, BLOCKS_PER_HASH);
    if (!cryptor.Decrypt((const u8 *)buffer, overlap_size, enc_offset, decrypted))
    {
        fprintf(stderr, "Decrypt failed rank %d, likely hash mismatch\n", rank);
        exit(1);
    }
    
    // remove padding
    size_t buffer_size = 0;
    for (; buffer_size < decrypted.size() && decrypted[buffer_size] != '#'; ++buffer_size);
    decrypted.resize(buffer_size);
    return decrypted;
}


/* Reads the first lien from the file and populates all the rank info. */
void read_first_line(MPI_File fh, RankInfo &rank_info, size_t overlap_size) {

    // Get file size.
    MPI_File_get_size(fh, &rank_info.file_size);
    // if (rank_info.rank == 0) printf("file size: %lld\n", rank_info.file_size);

    // Read in first line into buffer.
    char* first_line_buffer = new char[overlap_size]();
    MPI_Status status;
    MPI_File_read_at_all(fh, sizeof(int), first_line_buffer, overlap_size, MPI_CHAR, &status);

    buf_t decrypted = decrypt(first_line_buffer, 0, overlap_size);
    decrypted.push_back(0);

    // Get the first line from the buffer.
    std::string line;
    int cursor = 0;
    cursor = get_line(line, (char*)&decrypted[0], decrypted.size(), cursor);
    std::vector<int> nums = tokenize_line(line, ':');
    rank_info.vote_count = nums[0];
    rank_info.candidate_count = nums[1];
    rank_info.first_line_size = cursor;

    delete [] first_line_buffer;
}


/* Sets the start and end bytes to account for any missing data due to unknown chunk sizes. */
void get_start_and_end_bytes(const RankInfo &rank_info, size_t &start_byte, size_t &end_byte) {

    const int overlap_size = OVERLAP_MULTIPLIER * (rank_info.candidate_count * std::to_string(rank_info.candidate_count).length());
    size_t file_size = rank_info.file_size - rank_info.first_line_size;
    auto [bytes_per_rank, l_start_byte, l_end_byte] = partition(file_size, rank_info.rank, rank_info.size);
    // start_byte += rank_info.first_line_size;
    start_byte = l_start_byte + rank_info.first_line_size < file_size ? l_start_byte + rank_info.first_line_size : file_size;
    end_byte = l_end_byte + overlap_size < file_size ? l_end_byte + rank_info.first_line_size + overlap_size : file_size + rank_info.first_line_size;
}


/* Tokenizes a line according to some delimiter. */
std::vector<int> tokenize_line(const std::string &line, char delimitier) {
    std::vector<int> nums;

    std::string token;
    for (int i = 0; i < line.size() + 1; i++) {
        if (i == line.size() || line[i] == delimitier) {
            if (token.size() > 0) {
                int j = std::stoi(token);
                nums.push_back(j);
            }
            token.clear();
        } else token += line[i];
    }

    return nums;
}