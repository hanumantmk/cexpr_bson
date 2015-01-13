#include <cstddef>
#include <cstdint>
#include <array>
#include <bson.h>
#include <cstring>
#include <iostream>

#define CONSTEXPR constexpr
//#define CONSTEXPR 

enum class state {
   key_pre,
   key_mid,
   key_post,
   value_pre,
   value_int,
   value_str,
   value_post,
};

enum class type : uint8_t {
   b_string = 0x02,
   b_int32 = 0x10,
};

template <typename T>
CONSTEXPR void store_int (T t, std::uint8_t *out)
{
   for (std::size_t i = 0; i < sizeof(T); i++) {
      out[i] = t & 0xFF;
      t>>=8;
   }
}

CONSTEXPR void store_key_val (std::uint8_t *a, std::size_t &x, const char *last_key, const char *last_val, std::size_t key_len, std::size_t val_len)
{
   a[x] = 2;
   
   for (std::size_t j = 0; j < key_len; j++) {
      x++;

      a[x] = last_key[j];
   }

   x++;
   a[x] = 0;

   x++;

   store_int(std::uint32_t(val_len + 1), a + x);

   x += 3;

   for (std::size_t j = 0; j < val_len; j++) {
      x++;

      a[x] = last_val[j];
   }

   x++;
   a[x] = 0;

   x++;
}

CONSTEXPR void store_key_int (std::uint8_t *a, std::size_t &x, const char *last_key, const char *last_val, std::size_t key_len, std::size_t val_len)
{
   a[x] = static_cast<std::uint8_t>(type::b_int32);
   
   for (std::size_t j = 0; j < key_len; j++) {
      x++;

      a[x] = last_key[j];
   }

   x++;
   a[x] = 0;

   x++;

   uint32_t out = 0;
   uint32_t power = 1;

   for (std::size_t i = 0; i < val_len; i++) {
      out += (last_val[val_len - (i + 1)] - '0') * power;
      power *= 10;
   }

   store_int(out, a + x);

   x += 4;
}

class cexpr_bson {
public:
   template <std::size_t N>
   CONSTEXPR cexpr_bson(const char (&v)[N]) : a() {
      std::size_t x = 0;
      std::size_t key_len = 0;
      std::size_t val_len = 0;

      const char *last_key = v;
      const char *last_val = nullptr;

      state s = state::key_pre;

      /* skip the beginning */
      x += 4;

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
            } else if (v[i] >= '0' && v[i] <= '9') {
               s = state::value_int;
               last_val = v + i;
               i--;
            }
         } else if (s == state::value_int) {
            if (v[i] >= '0' && v[i] <= '9') {
               val_len++;
            } else {
               s = state::value_post;
               store_key_int (a, x, last_key, last_val, key_len, val_len);
               key_len = 0;
               val_len = 0;
               i--;
            }
         } else if (s == state::value_str) {
            if (v[i] == '"') {
               s = state::value_post;
               store_key_val (a, x, last_key, last_val, key_len, val_len);
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
         store_key_int (a, x, last_key, last_val, key_len, val_len);
      }

      store_int (uint32_t(x+1), a);
   }

   const uint8_t *data () const {
      return a;
   }

private:
   std::uint8_t a[256];
};


int main ()
{
   CONSTEXPR auto x = cexpr_bson("\"foo\":\"bar\",\"bar\":\"baz\",\"baz\":15715755");

   char *json;

   bson_t bson;

   bson_init_static (&bson, x.data(), *(int32_t*)x.data());

   json = bson_as_json (&bson, NULL);
   std::cout << json << std::endl;
   bson_free (json);

   return 0;
}
