cmake_minimum_required(VERSION 3.18)

project(final-project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -O2")

option(VERBOSE "Print outs" OFF)
if (VERBOSE)
    message(STATUS "Verbosity enabled")
    add_compile_definitions(VERBOSE)
endif()

find_package(MPI REQUIRED)
find_package(Threads REQUIRED)

if (APPLE OR CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)|(arm64)")
    add_compile_definitions(NOT_AIMOS)
endif()

add_library(crypto STATIC
    src/crypto/crypto.cpp
    src/crypto/ctrmode.cpp
    src/crypto/rijndael.cpp
    src/crypto/sha1.cpp
)
target_include_directories(crypto PUBLIC src/crypto)

add_executable(cryptotest src/crypto/cryptotest.cpp)

add_library(probability STATIC
    src/probability/distribution.cpp
    src/probability/uniform.cpp
    src/probability/fullbias.cpp
    src/probability/custom.cpp
)
target_include_directories(probability PUBLIC src/probability)

add_executable(celeb src/celeb/celeb.cpp)
add_executable(vote-algo src/vote/vote-algo.cpp)
add_executable(vote-gen src/vote/vote-gen.cpp)

set(TARGETS celeb vote-algo vote-gen cryptotest)

set_target_properties( ${TARGETS}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

foreach (TARGET IN ITEMS ${TARGETS})
    target_include_directories(${TARGET} PRIVATE src/common)
    target_link_libraries(${TARGET} PRIVATE MPI::MPI_CXX crypto probability Threads::Threads)

    set_target_properties(${TARGET} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
endforeach()