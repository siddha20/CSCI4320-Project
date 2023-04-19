aimos: src/graph/main.cpp src/graph/graph-cuda.cu src/vote/main.cpp
	mpicxx -std=c++20 -I../common src/graph/main.cpp -c -o src/graph/main-mpi.o
	nvcc src/graph/graph-cuda.cu -c -o src/graph/graph-cuda.o
	mpicxx src/graph/main-mpi.o src/graph/graph-cuda.o -o ../bin/aimos/graph_gen.out -lcudart
	rm src/graph/main-mpi.o
	rm src/graph/graph-cuda.o
	mpicxx src/vote/main.cpp -I../common -o ../bin/vote_gen.out

not_aimos: src/graph/main.cpp src/graph/graph-cuda.cu src/vote/main.cpp
	mpicxx -D NOT_AIMOS -std=c++20 -I../common src/graph/main.cpp -c -o src/graph/main-mpi.o
	nvcc src/graph/graph-cuda.cu -c -o src/graph/graph-cuda.o
	mpicxx src/graph/main-mpi.o src/graph/graph-cuda.o -o ../bin/graph_gen.out -lcudart
	rm src/graph/main-mpi.o
	rm src/graph/graph-cuda.o
	mpicxx -D NOT_AIMOS src/vote/main.cpp -I../common -o ../bin/vote_gen.out
