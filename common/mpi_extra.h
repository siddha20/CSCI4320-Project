#pragma once

template <typename T>
constexpr MPI_Datatype get_mpi_type() {

    if constexpr (std::is_same_v<T, char>) return MPI_CHAR;
    else if constexpr (std::is_same_v<T, int>) return MPI_INT;
    else if constexpr (std::is_same_v<T, signed char>) return MPI_SIGNED_CHAR;
    else if constexpr (std::is_same_v<T, unsigned char>) return MPI_UNSIGNED_CHAR;
    else if constexpr (std::is_same_v<T, wchar_t>) return MPI_WCHAR;
    else if constexpr (std::is_same_v<T, signed short>) return MPI_SHORT;
    else if constexpr (std::is_same_v<T, unsigned short>) return MPI_UNSIGNED_SHORT;
    else if constexpr (std::is_same_v<T, signed int>) return MPI_INT;
    else if constexpr (std::is_same_v<T, unsigned int>) return MPI_UNSIGNED;
    else if constexpr (std::is_same_v<T, signed long int>) return MPI_LONG;
    else if constexpr (std::is_same_v<T, unsigned long int>) return MPI_UNSIGNED_LONG;
    else if constexpr (std::is_same_v<T, signed long long int>) return MPI_LONG_LONG;
    else if constexpr (std::is_same_v<T, unsigned long long int>) return MPI_UNSIGNED_LONG_LONG;
    else if constexpr (std::is_same_v<T, float>) return MPI_FLOAT;
    else if constexpr (std::is_same_v<T, double>) return MPI_DOUBLE;
    else if constexpr (std::is_same_v<T, long double>) return MPI_LONG_DOUBLE;
    else if constexpr (std::is_same_v<T, std::int8_t>) return MPI_INT8_T;
    else if constexpr (std::is_same_v<T, std::int16_t>) return MPI_INT16_T;
    else if constexpr (std::is_same_v<T, std::int32_t>) return MPI_INT32_T;
    else if constexpr (std::is_same_v<T, std::int64_t>) return MPI_INT64_T;
    else if constexpr (std::is_same_v<T, std::uint8_t>) return MPI_UINT8_T;
    else if constexpr (std::is_same_v<T, std::uint16_t>) return MPI_UINT16_T;
    else if constexpr (std::is_same_v<T, std::uint32_t>) return MPI_UINT32_T;
    else if constexpr (std::is_same_v<T, std::uint64_t>) return MPI_UINT64_T;
    else if constexpr (std::is_same_v<T, bool>) return MPI_C_BOOL;
    else if constexpr (std::is_same_v<T, std::complex<float>>) return MPI_C_COMPLEX;
    else if constexpr (std::is_same_v<T, std::complex<double>>) return MPI_C_DOUBLE_COMPLEX;
    else if constexpr (std::is_same_v<T, std::complex<long double>>) return MPI_C_LONG_DOUBLE_COMPLEX;
    else return MPI_DATATYPE_NULL;
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
