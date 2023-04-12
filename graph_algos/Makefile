aimos: main.cpp
	mpicxx -std=c++20 main.cpp -c -o main-mpi.o
	mpicxx main-mpi.o -o graph_algo.out
	rm main-mpi.o

not_aimos: main.cpp
	mpicxx -D NOT_AIMOS -std=c++20 main.cpp -c -o main-mpi.o
	mpicxx main-mpi.o -o graph_algo.out
	rm main-mpi.o