#include <cstddef>
#include <cstdint>
#include <array>
#include <bson.h>
#include <cstring>
#include <iostream>

#define CONSTEXPR constexpr
//#define CONSTEXPR 

#include "cexpr/bson.hpp"

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

CONSTEXPR void store_key_val (bson &b, const char *last_key, const char *last_val, std::size_t key_len, std::size_t val_len)
{
   b.append_utf8(last_key, key_len, last_val, val_len);
}

CONSTEXPR void store_key_int (bson &b, const char *last_key, const char *last_val, std::size_t key_len, std::size_t val_len)
{
   bool is_signed = last_val[0] == '-';

   int32_t out = 0;
   int32_t power = is_signed ? -1 : 1;

   for (std::size_t i = 0; i < val_len - (is_signed ? 1 : 0); i++) {
      out += (last_val[val_len - (i + 1)] - '0') * power;
      power *= 10;
   }

   b.append_int32(last_key, key_len, out);
}

class cexpr_bson {
public:
   template <std::size_t N>
   CONSTEXPR cexpr_bson(const char (&v)[N]) : a() {
      bson b(a);

      std::size_t key_len = 0;
      std::size_t val_len = 0;

      const char *last_key = v;
      const char *last_val = nullptr;

      state s = state::key_pre;

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
               store_key_int (b, last_key, last_val, key_len, val_len);
               key_len = 0;
               val_len = 0;
               i--;
            }
         } else if (s == state::value_str) {
            if (v[i] == '"') {
               s = state::value_post;
               store_key_val (b, last_key, last_val, key_len, val_len);
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
         store_key_int (b, last_key, last_val, key_len, val_len);
      }

   }

   const uint8_t *data () const {
      return a;
   }

private:
   std::uint8_t a[256];
};

int main ()
{
   CONSTEXPR auto x = cexpr_bson("\"foo\":\"bar\",\"bar\":\"baz\",\"baz\":15715755,\"neg\":-55");

   bson_t bson;

   bson_init_static (&bson, x.data(), *(int32_t*)x.data());

   char *json;

   json = bson_as_json (&bson, NULL);
   std::cout << json << std::endl;
   bson_free (json);

   return 0;
}
