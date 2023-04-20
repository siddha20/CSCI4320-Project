#include "common.h"

#define FIRST_LINE_BUFFER_SIZE 1024
#define BUFFER_SIZE 10000
#define OVERLAP_MULTIPLIER 2

std::vector<int> tokenize_line(const std::string &line, char delimitier);

int main(int argc, char** argv) {

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
    if (rank == 0) printf("file size: %lld\n", file_size);


    // Read in first line into buffer.
    char* first_line_buffer = new char[FIRST_LINE_BUFFER_SIZE]();
    MPI_File_read_all(fh, first_line_buffer, FIRST_LINE_BUFFER_SIZE, MPI_CHAR, &status);


    // Get the first line from the buffer.
    std::string line;
    int cursor = 0;
    cursor = get_line(line, first_line_buffer, file_size, cursor);
    std::vector<int> nums = tokenize_line(line, ':');
    int vote_count = nums[0];
    int candidate_count = nums[1];
    int first_line_size = cursor;
    delete [] first_line_buffer;


    // Calculate start and end bytes.
    const int overlap_size = OVERLAP_MULTIPLIER * (candidate_count * std::to_string(candidate_count).length());
    file_size -= first_line_size;
    auto [bytes_per_rank, start_byte, end_byte] = partition(file_size, rank, size);
    start_byte += first_line_size;
    end_byte = end_byte + overlap_size < file_size ? end_byte + first_line_size + overlap_size : file_size + first_line_size;

    // Collect all the data from the file.
    int buffer_size = end_byte - start_byte; 
    char* buffer = new char[buffer_size]();
    MPI_File_read_at_all(fh, start_byte, buffer, buffer_size, MPI_CHAR, &status);

    std::vector<int> voters;
    std::map<int, std::vector<int> > data;
    cursor = 0;
    while((cursor = get_line(line, buffer, buffer_size, cursor)) != -1) {
        std::vector<int> nums = tokenize_line(line, ':');
        if (nums.size() == candidate_count + 1) {
            voters.push_back(nums[0]);
            data[nums[0]] = std::vector<int>(nums.begin() + 1, nums.end());
            // std::cout << line << std::endl;
        }
    }
    delete [] buffer;


    // Very important to sort voters.
    std::sort(voters.begin(), voters.end());
    MPI_Barrier(MPI_COMM_WORLD);

    // Find distinct voters per rank.
    std::vector<int> voter_diff;
    if (rank == size - 1) voter_diff = voters;

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

    // Create graph
    std::vector<std::vector<int>> graph(candidate_count, std::vector<int>(candidate_count, 0));
    for (auto voter : voter_diff) {
        std::vector<int> candidates = data[voter];
        std::cout << voter << ": ";
        print_vec(candidates);
        std::cout << std::endl;

        for (int i = 0; i < candidates.size(); i++) {
            for (int j = i + 1; j < candidates.size(); j++) {
                graph[candidates[i] - 1][candidates[j] - 1]++;
            }
        }
    }

    if (rank == 0) {
        print_vec(voter_diff);
        std::cout << std::endl;
        for (const auto &row : graph) {
            print_vec(row);
            std::cout << std::endl;
        }
    }


    MPI_File_close(&fh);
    MPI_Finalize();


    return EXIT_SUCCESS;
}

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