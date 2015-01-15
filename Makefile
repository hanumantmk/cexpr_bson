cexpr_bson: cexpr_bson.cpp Makefile src/cexpr/data_view.hpp src/cexpr/bson.hpp src/bson_iter.hpp
	clang++ -std=c++1y -Wall -Werror -ggdb3 -O0 -Isrc cexpr_bson.cpp -o cexpr_bson

clean:
	rm -f cexpr_bson
