#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
#include <iterator>
#include <utility>
#include <type_traits>
#include "mpi.h"
#include "mpi_type.h"

#ifndef NOT_AIMOS
    #include "clockcycle.h"
    #define CLOCK_FREQ 512000000
#endif

#ifdef NOT_AIMOS
    auto clock_now = []{ return 0; };
#endif

int get_line(std::string &new_line, char* buffer, int buffer_size, int offset) {

    new_line.clear();
    new_line = "";

    if (offset >= buffer_size) return -1;

    for (int i = offset; i < buffer_size; i++) {
        if (buffer[i] == '\n') {
            return offset + new_line.size() + 1;
        } else new_line += buffer[i];
    }
    return offset + new_line.size() + 1;
}

struct ParitionData {
    int size_per_rank;
    int start;
    int end;
};

ParitionData partition(int total_size, int rank, int size) {
    ParitionData data;
    data.size_per_rank = (total_size/size) + ((total_size % size) != 0);
    data.start = rank * data.size_per_rank;
    data.end = (data.start + data.size_per_rank) < total_size ? data.start + data.size_per_rank : total_size;
    return data;
}

template <typename T>
void print_vec(const std::vector<T> &vec) {
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>(std::cout," "));
    std::cout << std::flush;
}

template <typename T> 
void MPI_Send_vec(const std::vector<T> &src, int dst_rank) {

        MPI_Datatype mpi_type = get_mpi_type<T>();

        int i = src.size();
        MPI_Send(&i, 1, mpi_type, dst_rank, 0, MPI_COMM_WORLD);
        MPI_Send(&src[0], src.size(), mpi_type, dst_rank, 0, MPI_COMM_WORLD);
}

template <typename T> 
void MPI_Recv_vec(std::vector<T> &dst, int src_rank) {

        MPI_Datatype mpi_type = get_mpi_type<T>();
        MPI_Status status;

        int size;
        MPI_Recv(&size, 1, mpi_type, src_rank, 0, MPI_COMM_WORLD, &status);
        dst.resize(size);

        MPI_Recv(&dst[0], size, mpi_type, src_rank, 0, MPI_COMM_WORLD, &status);
}