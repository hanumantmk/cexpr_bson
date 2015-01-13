cexpr_bson: cexpr_bson.cpp Makefile
	clang++ -std=c++1y -Wall -Werror -ggdb3 -O2 `pkg-config libbson-1.0 --cflags` `pkg-config libbson-1.0 --libs` cexpr_bson.cpp -o cexpr_bson

clean:
	rm -f cexpr_bson
