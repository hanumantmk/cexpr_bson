cexpr_bson: cexpr_bson.cpp Makefile src/cexpr/data_view.hpp src/cexpr/bson.hpp
	clang++ -std=c++1y -Wall -Werror -ggdb3 -O0 -Isrc `pkg-config libbson-1.0 --cflags` `pkg-config libbson-1.0 --libs` cexpr_bson.cpp -o cexpr_bson

clean:
	rm -f cexpr_bson
