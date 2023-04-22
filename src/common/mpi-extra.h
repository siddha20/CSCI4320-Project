#pragma once
#include <complex>
#include <vector>
#include <iterator>
#include <type_traits>


template <typename T>
constexpr MPI_Datatype get_mpi_type() {

    if constexpr (std::is_same<T, char>::value) return MPI_CHAR;
    else if constexpr (std::is_same<T, int>::value) return MPI_INT;
    else if constexpr (std::is_same<T, signed char>::value) return MPI_SIGNED_CHAR;
    else if constexpr (std::is_same<T, unsigned char>::value) return MPI_UNSIGNED_CHAR;
    else if constexpr (std::is_same<T, wchar_t>::value) return MPI_WCHAR;
    else if constexpr (std::is_same<T, signed short>::value) return MPI_SHORT;
    else if constexpr (std::is_same<T, unsigned short>::value) return MPI_UNSIGNED_SHORT;
    else if constexpr (std::is_same<T, signed int>::value) return MPI_INT;
    else if constexpr (std::is_same<T, unsigned int>::value) return MPI_UNSIGNED;
    else if constexpr (std::is_same<T, signed long int>::value) return MPI_LONG;
    else if constexpr (std::is_same<T, unsigned long int>::value) return MPI_UNSIGNED_LONG;
    else if constexpr (std::is_same<T, signed long long int>::value) return MPI_LONG_LONG;
    else if constexpr (std::is_same<T, unsigned long long int>::value) return MPI_UNSIGNED_LONG_LONG;
    else if constexpr (std::is_same<T, float>::value) return MPI_FLOAT;
    else if constexpr (std::is_same<T, double>::value) return MPI_DOUBLE;
    else if constexpr (std::is_same<T, long double>::value) return MPI_LONG_DOUBLE;
    else if constexpr (std::is_same<T, std::int8_t>::value) return MPI_INT8_T;
    else if constexpr (std::is_same<T, std::int16_t>::value) return MPI_INT16_T;
    else if constexpr (std::is_same<T, std::int32_t>::value) return MPI_INT32_T;
    else if constexpr (std::is_same<T, std::int64_t>::value) return MPI_INT64_T;
    else if constexpr (std::is_same<T, std::uint8_t>::value) return MPI_UINT8_T;
    else if constexpr (std::is_same<T, std::uint16_t>::value) return MPI_UINT16_T;
    else if constexpr (std::is_same<T, std::uint32_t>::value) return MPI_UINT32_T;
    else if constexpr (std::is_same<T, std::uint64_t>::value) return MPI_UINT64_T;
    else if constexpr (std::is_same<T, bool>::value) return MPI_C_BOOL;
    else if constexpr (std::is_same<T, std::complex<float>>::value) return MPI_C_COMPLEX;
    else if constexpr (std::is_same<T, std::complex<double>>::value) return MPI_C_DOUBLE_COMPLEX;
    else if constexpr (std::is_same<T, std::complex<long double>>::value) return MPI_C_LONG_DOUBLE_COMPLEX;
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
