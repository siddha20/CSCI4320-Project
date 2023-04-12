aimos: src/main.cpp src/graph-cuda.cu
	mpicxx -std=c++20 src/main.cpp -c -o main-mpi.o
	nvcc src/graph-cuda.cu -c -o graph-cuda.o
	mpicxx main-mpi.o graph-cuda.o -o bin/aimos/graph_gen.out -lcudart
	rm main-mpi.o
	rm graph-cuda.o

not_aimos: src/main.cpp src/graph-cuda.cu
	mpicxx -D NOT_AIMOS -std=c++20 src/main.cpp -c -o main-mpi.o
	nvcc src/graph-cuda.cu -c -o graph-cuda.o
	mpicxx main-mpi.o graph-cuda.o -o bin/graph_gen.out -lcudart
	rm main-mpi.o
	rm graph-cuda.o