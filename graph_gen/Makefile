aimos: src/main.cpp src/graph-cuda.cu
	mpicxx -std=c++20 -I../common src/main.cpp -c -o src/main-mpi.o
	nvcc src/graph-cuda.cu -c -o src/graph-cuda.o
	mpicxx src/main-mpi.o src/graph-cuda.o -o bin/aimos/graph_gen.out -lcudart
	rm src/main-mpi.o
	rm src/graph-cuda.o

not_aimos: src/main.cpp src/graph-cuda.cu
	mpicxx -D NOT_AIMOS -std=c++20 -I../common src/main.cpp -c -o src/main-mpi.o
	nvcc src/graph-cuda.cu -c -o src/graph-cuda.o
	mpicxx src/main-mpi.o src/graph-cuda.o -o bin/graph_gen.out -lcudart