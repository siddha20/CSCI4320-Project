#pragma once
#include <assert.h>
#include <iterator>
#include <vector>
#include "mpi.h"
#include "mpi-extra.h"

#ifndef NOT_AIMOS
    #include "clockcycle.h"
    #define CLOCK_FREQ 512000000
#endif

#ifdef NOT_AIMOS
    auto clock_now = []{ return 123; };
    #define CLOCK_FREQ 1
#endif


struct ParitionData {
    size_t size_per_rank;
    size_t start;
    size_t end;
};

class Timer {
    private:
        int start_cycles = 0;
        int end_cycles = 0;
        std::string label;

        double get_duration_sec() {
            return ((double)(end_cycles - start_cycles))/CLOCK_FREQ;
        }

        double get_duration_cycles() {
            return end_cycles - start_cycles;
        }

    public:
        Timer() = default;
        Timer(const std::string &label) : label(label) {}

        void start() {
            start_cycles = clock_now();
        }

        void end() {
            end_cycles = clock_now();
        }

        void print_duration_sec() {
            if (label.size() == 0) {
            std::cout << "Completed in " << this->get_duration_sec() << "s." << std::endl;
            }
            else {
                std::cout << "Completed " << label << " in " << this->get_duration_sec() << "s." << std::endl;
            }
        }

        void print_duration_cycles() {
            if (label.size() == 0) {
            std::cout << "Completed in " << this->get_duration_cycles() << " cycles." << std::endl;
            }
            else {
                std::cout << "Completed " << label << " in " << this->get_duration_cycles() << " cycles." << std::endl;
            }
        }
};

int get_line(std::string &new_line, char* buffer, int buffer_size, int offset) {

    new_line.clear();
    new_line = "";

    if (offset >= buffer_size) return -1;

    for (int i = offset; i < buffer_size; i++) {
        if (buffer[i] == '\n' || buffer[i] == '\r') {
            return offset + new_line.size() + 1;
        } else new_line += buffer[i];
    }
    return offset + new_line.size() + 1;
}

ParitionData partition(size_t total_size, size_t rank, size_t size) {
    ParitionData data;
    data.size_per_rank = (total_size/size) + ((total_size % size) != 0);
    data.start = rank * data.size_per_rank;
    data.end = (data.start + data.size_per_rank) < total_size ? data.start + data.size_per_rank : total_size;
    return data;
}

template <typename T>
void print_array(T const * array, const size_t size) {
    for (int i = 0; i < size; i++) std::cout << array[i] << " ";
    std::cout << std::flush;
}

template <typename T>
void print_array_2d(T const * array, const size_t rows, const size_t cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            std::cout << array[i * cols + j] << " ";
        }
        std::cout << std::endl;
    }     
    std::cout << std::flush;
}

template <typename T>
void print_vec(const std::vector<T> &vec) {
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>(std::cout," "));
    std::cout << std::flush;
}

template <typename T>
void print_vec_2d(const std::vector<T> &vec, const size_t rows, const size_t cols) {
    for (int i = 0; i < rows; i++) {
        std::copy(vec.begin() + (i * cols), vec.begin() + (i * cols) + cols, std::ostream_iterator<T>(std::cout," "));
        std::cout << std::endl;
    }
    std::cout << std::flush;
}