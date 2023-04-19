#include <string>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <iterator>
#include <utility>
#include "mpi.h"

#define BUFFER_SIZE 2048
#define OVERLAP 100

#ifndef NOT_AIMOS
    #include "clockcycle.h"
    #define CLOCK_FREQ 512000000
#endif


std::vector<int> tokenize_line(const std::string &line, char delimitier);
int get_line(std::string &new_line, char* buffer, int buffer_size, int offset);

int main(int argc, char** argv) {

    #ifdef NOT_AIMOS
        auto clock_now = []{ return 0; };
    #endif

    MPI_Init(&argc, &argv);

    std::string input_filename;
    std::string output_filename;
    std::string device_type;

    if (argc != 4) {
        std::cerr << "Usage: vote_gen.out <input_filename> <output_filename> <'CPU' or 'CUDA'>" << std::endl;
        return EXIT_FAILURE;
    } else {
        input_filename = argv[1];
        output_filename = argv[2];
        device_type = argv[3];
    }

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::srand(1230128093 + rank << 2);

    MPI_File fh;
    MPI_Status status;
    MPI_File_open(MPI_COMM_WORLD, input_filename.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);

    MPI_Offset file_size;
    MPI_File_get_size(fh, &file_size);
    printf("file size: %d\n", file_size);

    char buffer[BUFFER_SIZE];
    MPI_File_read_all(fh, buffer, file_size, MPI_CHAR, &status);
    
    std::string line;
    bool first_line = true;
    int i;
    int vote_count;
    int candidate_count;

    // find the first line
    for (i = 0; i < file_size; i++) {
        if (buffer[i] == '\n') {
            std::vector<int> nums = tokenize_line(line, ':');
            vote_count = nums[0];
            candidate_count = nums[1];
            // std::cout << line << std::endl;
            line.clear();
            break;
        } 
        else line += buffer[i];
    }

    line.clear();

    file_size -= i;
    int bytes_per_rank = (file_size/size) + ((file_size % size) != 0);
    int start_byte = rank * bytes_per_rank;
    int end_byte = (start_byte  + bytes_per_rank) < file_size ? start_byte  + bytes_per_rank : file_size;
    start_byte += i;
    end_byte = end_byte + OVERLAP < file_size ? end_byte + i + OVERLAP : file_size + i;

    printf("start byte: %d, end byte: %d\n", start_byte, end_byte);

    // rest of the file
    std::vector<int> voters;
    for (int j = start_byte; j < end_byte; j++) {
    if (buffer[j] == '\n') {
        std::vector<int> nums = tokenize_line(line, ':');
        if (nums.size() == candidate_count + 1) {
            voters.push_back(nums[0]);
            std::cout << line << std::endl;
        }
        line.clear();
    } 
    else line += buffer[j];
}

std::sort(voters.begin(), voters.end());

MPI_Barrier(MPI_COMM_WORLD);


// this vector contains distinct voter ids for each rank.
std::vector<int> voter_diff;

// compare overlap
for (int j = 0; j < size - 1; j++) {

    std::vector<int> temp;
    int voter_size;

    if (rank == j + 1) {
        int k = voters.size();
        MPI_Send(&k, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
    }
    if (rank == j) {
        MPI_Recv(&voter_size, 1, MPI_INT, j + 1, 0, MPI_COMM_WORLD, &status);
        // std::cout << "voter_size: " << voter_size << std::endl;
        temp.resize(voter_size);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == j + 1) {
        MPI_Send(&voters[0], voters.size(), MPI_INT, j, 0, MPI_COMM_WORLD);
    }
    if (rank == j) {
        MPI_Recv(&temp[0], voter_size, MPI_INT, j + 1, 0, MPI_COMM_WORLD, &status);
        std::set_difference(voters.begin(), voters.end(), temp.begin(), temp.end(), 
            std::inserter(voter_diff, voter_diff.begin()));
    }
}






    MPI_File_close(&fh);
    MPI_Finalize();

    return EXIT_SUCCESS;
}

int get_line(std::string &new_line, char* buffer, int buffer_size, int offset) {

    new_line.clear();

    if (offset >= buffer_size) return -1;

    std::cout << "here " << new_line << std::endl;;

    for (int i = offset; i < buffer_size; i++) {
        if (buffer[i] == '\n') {
            return new_line.size() + 1;
        } else new_line += buffer[i];
    }

    std::cout << "here " << new_line << std::endl;;
    return new_line.size() + 1;
}

std::vector<int> tokenize_line(const std::string &line, char delimitier) {
    std::vector<int> nums;

    // std::cout << line << std::endl;
    std::string token;
    for (int i = 0; i < line.size() + 1; i++) {
        if (i == line.size() || line[i] == delimitier) {
            if (token.size() > 0) {
                int j = std::stoi(token);
                nums.push_back(j);
                // std::cout << j << ", ";
            }
            token.clear();
        } else token += line[i];
    }
    // std::cout << "\n";
    return nums;
}