aimos: src/celeb.cpp src/vote.cpp
	mpicxx -std=c++20 src/celeb.cpp -c -o bin/aimos/celeb_algo.out
	mpicxx -std=c++20 vote.cpp -c -o bin/aimos/vote.out

not_aimos: src/celeb.cpp src/vote.cpp
	mpicxx -D NOT_AIMOS -std=c++20 src/celeb.cpp -c -o bin/celeb_algo.out
	mpicxx -D NOT_AIMOS -std=c++20 src/vote.cpp -c -o bin/vote_algo.out