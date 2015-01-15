#include <cstddef>
#include <cstdint>
#include <array>
#include <bson.h>
#include <cstring>
#include <iostream>

#define CONSTEXPR constexpr
//#define CONSTEXPR 
//#define CONSTEXPR_OFF

#include "cexpr/bson.hpp"
#include "cexpr/atof.hpp"

using namespace cexpr;

int main ()
{
   bson_t bson;
   char *json;

   CEXPR_BSON_FROM_JSON(bytes, "{\"foo\":\"bar\",\"bar\":\"baz\",\"baz\":15715755,\"neg\":-55, \"double\" : -0.012423}");
   CEXPR_BSON_FROM_JSON(bytes2, "{\"a\":1,\"b\":2,\"c\":3,\"d\" : { \"key\" : \"value\", \"2nd\" : 35, \"array\" : [1, 2, 3, true, false, -1152921504606846976, null] } }");
   CEXPR_BSON_FROM_JSON(bytes3, "{\"only need\":\"length\"}");

   bson_init_static (&bson, bytes.data(), bytes.len());
   json = bson_as_json (&bson, NULL);
   std::cout << "bytes1: " << json << std::endl;
   bson_free (json);

   bson_init_static (&bson, bytes2.data(), bytes2.len());
   json = bson_as_json (&bson, NULL);
   std::cout << "bytes2: " << json << std::endl;
   bson_free (json);

   std::cout << "bytes3 len: " << bytes3.len() << std::endl;

   return 0;
}
