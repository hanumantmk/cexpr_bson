#include <cstddef>
#include <cstdint>
#include <array>
#include <cstring>
#include <iostream>

#define CONSTEXPR constexpr
//#define CONSTEXPR 
//#define CONSTEXPR_OFF

#include "cexpr/bson.hpp"
#include "cexpr/atof.hpp"
#include "bson_iter.hpp"

using namespace cexpr;

int main ()
{
   CEXPR_BSON_FROM_JSON(bytes, "{\"foo\":\"bar\",\"bar\":\"baz\",\"baz\":15715755,\"neg\":-55, \"double\" : -0.012423}");
   CEXPR_BSON_FROM_JSON(bytes2, "{\"a\":1,\"b\":2,\"c\":3,\"d\" : { \"key\" : \"value\", \"2nd\" : 35, \"array\" : [1, 2, 3, true, false, 0.0, -1152921504606846976, null] } }");
   CEXPR_BSON_FROM_JSON(bytes3, "{\"only need\":\"length\"}");

   bson_iter bi(bytes.data());
   bson_iter bi2(bytes2.data());

   bi.json(std::cout);
   std::cout << std::endl;

   bi2.json(std::cout);
   std::cout << std::endl;

   std::cout << "bytes3 len: " << bytes3.len() << std::endl;

   return 0;
}
