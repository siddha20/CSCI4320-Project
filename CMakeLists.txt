cmake_minimum_required(VERSION 3.18 )

project(final-project LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MPI REQUIRED)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
    add_compile_definitions(NOT_AIMOS)
endif()



add_library(crypto STATIC
    src/crypto/cryptompi.cpp
    src/crypto/ctrmode.cpp
    src/crypto/rijndael.cpp
)
add_library(probability STATIC
    src/probability/distribution.cpp
    src/probability/uniform.cpp
    src/probability/fullbias.cpp
    src/probability/custom.cpp
)

add_executable(celeb src/celeb/celeb.cpp)
add_executable(vote-algo src/vote/vote-algo.cpp)
add_executable(vote-gen src/vote/vote-gen.cpp)

add_executable(graph-gen
    src/graph/graph-gen.cpp
    src/graph/graph-gen.cu
)

set(TARGETS celeb vote-algo vote-gen graph-gen)

set_target_properties( ${TARGETS}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)


foreach (TARGET IN ITEMS ${TARGETS})
    target_include_directories(${TARGET} PRIVATE src/common src/probability)
    target_link_libraries(${TARGET} PRIVATE MPI::MPI_CXX crypto probability)

    set_target_properties(${TARGET} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
endforeach()