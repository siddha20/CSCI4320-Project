#pragma once

template <typename T>
MPI_Datatype get_mpi_type() {

    if (std::is_same_v<T, char>) return MPI_CHAR;
    if (std::is_same_v<T, int>) return MPI_INT;

    return MPI_DATATYPE_NULL;
}
