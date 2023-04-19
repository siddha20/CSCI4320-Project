#pragma once

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