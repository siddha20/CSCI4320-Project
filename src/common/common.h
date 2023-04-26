#pragma once
#include <assert.h>
#include <iterator>
#include <vector>
#include <iostream>
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

#define OVERLAP_MULTIPLIER 2

// For testing
#define ENC_KEY_128 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0xd, 0xe, 0xf
#define ENC_KEY_192 KEY_128, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
#define ENC_KEY_256 KEY_192, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
#define ENC_KEY_CUR ENC_KEY_128
#define ENC_IV 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66
#define BLOCKS_PER_HASH 2048

struct ParitionData {
    size_t size_per_rank;
    size_t start;
    size_t end;
};

class Timer {
    private:
        unsigned long long start_cycles = 0;
        unsigned long long end_cycles = 0;
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

        void print_duration_sec_label_only() {
            std::cout << label << " " << this->get_duration_sec() << " (seconds)" << std::endl;
        }

        void print_duration_cycles_label_only() {
            std::cout << label << " " << this->get_duration_cycles() << " (cycles)" << std::endl;
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
