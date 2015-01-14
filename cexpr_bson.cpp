#include <cstddef>
#include <cstdint>
#include <array>
#include <bson.h>
#include <cstring>
#include <iostream>

#define CONSTEXPR constexpr
//#define CONSTEXPR 

#include "cexpr/bson.hpp"
#include "cexpr/atoi.hpp"

#define MAKE(name, my_str) \
struct { \
   CONSTEXPR static const char *str() { return my_str; }; \
} MAGIC_IMPL_##name; \
CONSTEXPR auto name = cexpr_bson<decltype(MAGIC_IMPL_##name)>()

using namespace cexpr;

enum class state {
   key_pre,
   key_mid,
   key_post,
   value_pre,
   value_int,
   value_str,
   value_post,
};

template <typename T>
CONSTEXPR std::size_t cexpr_parse(const char *v, uint8_t *a)
{
   T b(a);

   std::size_t key_len = 0;
   std::size_t val_len = 0;

   const char *last_key = v;
   const char *last_val = nullptr;

   state s = state::key_pre;

   std::size_t N = 0;
   const char *z = v;

   for (; *z; z++, N++) {}

   for (std::size_t i = 0; i < N; i++) {
      if (s == state::key_pre) {
         if (v[i] == '"') {
            last_key = v + i + 1;
            s = state::key_mid;
         }
      } else if (s == state::key_mid) {
         if (v[i] == '"') {
            s = state::key_post;
         } else {
            key_len++;
         }
      } else if (s == state::key_post) {
         if (v[i] == ':') {
            s = state::value_pre;
         }
      } else if (s == state::value_pre) {
         if (v[i] == '"') {
            s = state::value_str;
            last_val = v + i + 1;
         } else if (v[i] == '-' || (v[i] >= '0' && v[i] <= '9')) {
            s = state::value_int;
            last_val = v + i;
            val_len++;
         }
      } else if (s == state::value_int) {
         if (v[i] >= '0' && v[i] <= '9') {
            val_len++;
         } else {
            s = state::value_post;
            b.append_int32(last_key, key_len, cexpr::atoi(last_val, val_len));
            key_len = 0;
            val_len = 0;
            i--;
         }
      } else if (s == state::value_str) {
         if (v[i] == '"') {
            s = state::value_post;
            b.append_utf8(last_key, key_len, last_val, val_len);
            key_len = 0;
            val_len = 0;
         } else {
            val_len++;
         }
      } else if (s == state::value_post) {
         if (v[i] == ',') {
            s = state::key_pre;
         }
      }
   }

   if (s == state::value_int) {
      b.append_int32(last_key, key_len, cexpr::atoi(last_val, val_len));
   }

   return b.length();
}

template <typename T>
class cexpr_bson {
public:
   CONSTEXPR cexpr_bson() : a() {
      cexpr_parse<bson>(T::str(), a);
   }

   CONSTEXPR const uint8_t *data () const {
      return a;
   }

   CONSTEXPR std::size_t len() const {
      return sizeof(a);
   }

private:
   std::uint8_t a[cexpr_parse<bson_sizer>(T::str(), nullptr)];
};

int main ()
{
   bson_t bson;
   char *json;

   MAKE(bytes, "\"foo\":\"bar\",\"bar\":\"baz\",\"baz\":15715755,\"neg\":-55");
   MAKE(bytes2, "\"a\":1,\"b\":2,\"c\":3");
   MAKE(bytes3, "\"only need\":\"length\"");

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
