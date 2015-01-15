SOURCES = \
	src/bson_iter.hpp \
	src/cexpr/atoi.hpp \
	src/cexpr/data_view.hpp \
	src/cexpr/atof.hpp \
	src/cexpr/itoa.hpp \
	src/cexpr/bson.hpp \
	src/cexpr/jsmn.h

cexpr_bson: Makefile cexpr_bson.cpp $(SOURCES)
	clang++ -std=c++1y -Wall -Werror -ggdb3 -O2 -Isrc cexpr_bson.cpp -o cexpr_bson

clean:
	rm -f cexpr_bson
