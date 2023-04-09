all: main.cpp graph-cuda.cu
	mpicxx -std=c++20 main.cpp -c -o main-mpi.o
	nvcc graph-cuda.cu -c -o graph-cuda.o
	mpicxx main-mpi.o graph-cuda.o -lcudart
	rm main-mpi.o
	rm graph-cuda.o