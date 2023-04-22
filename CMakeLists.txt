cmake_minimum_required(VERSION 3.20 )

project(final-project LANGUAGES CXX C CUDA)

set(${CMAKE_CXX_STANDARD} 17)
set(${CMAKE_CXX_STANDARD_REQUIRED} ON)

find_package(MPI REQUIRED)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
    add_compile_definitions(-DNOT_AIMOS)
endif()

add_library(crypto STATIC
    src/crypto/cryptompi.cpp
    src/crypto/ctrmode.cpp
    src/crypto/rijndael.cpp
)

add_executable(celeb src/celeb/celeb.cpp)
add_executable(vote-algo src/vote/vote-algo.cpp)
add_executable(vote-gen src/vote/vote-gen.cpp)

add_executable(graph-gen
    src/graph/graph-gen.cpp
    src/graph/graph-gen.cu
)

foreach (TARGET IN ITEMS celeb vote-algo vote-gen)
    target_include_directories(${TARGET} PRIVATE src/common)
    target_link_libraries(${TARGET} PRIVATE MPI crypto)
endforeach()