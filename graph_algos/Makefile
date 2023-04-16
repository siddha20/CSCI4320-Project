aimos: src/celeb.cpp src/vote.cpp
	mpicxx -std=c++20 -I../common src/celeb.cpp -o ../bin/aimos/celeb_algo.out
	mpicxx -std=c++20 -I../common src/vote.cpp -o ../bin/aimos/vote.out

not_aimos: src/celeb.cpp src/vote.cpp
	mpicxx -D NOT_AIMOS -std=c++20 -I../common src/celeb.cpp -o ../bin/celeb_algo.out
	mpicxx -D NOT_AIMOS -std=c++20 -I../common src/vote.cpp -o ../bin/vote_algo.out