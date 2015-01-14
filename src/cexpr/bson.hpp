//#define CONSTEXPR constexpr

#include <cstdint>
#include <cstring>
#include "cexpr/data_view.hpp"

#pragma once

namespace cexpr {

enum class bson_type : uint8_t {
   b_utf8 = 0x02,
   b_int32 = 0x10,
};

class bson {
   public:
   CONSTEXPR bson(uint8_t *b) : bytes(b), ptr(b + 4) {
      update_len();
   }

   CONSTEXPR void append_int32(const char *key, std::size_t klen, int32_t v) {
      append_prefix(key, klen, bson_type::b_int32);

      data_view(ptr).store_le_int32(v);
      ptr += 4;

      update_len();
   }

   CONSTEXPR void append_utf8(const char *key, std::size_t klen, const char *v, std::size_t vlen) {
      append_prefix(key, klen, bson_type::b_utf8);

      data_view(ptr).store_le_uint32(vlen + 1);
      ptr += 4;

      for (std::size_t i = 0; i < vlen; i++) {
         *ptr++ = v[i];
      }

      *ptr++ = 0;

      update_len();
   }

   CONSTEXPR void append_prefix(const char *key, std::size_t klen, bson_type bt)
   {
      *ptr++ = (uint8_t)bt;

      for (std::size_t i = 0; i < klen; i++) {
         *ptr++ = key[i];
      }

      *ptr++ = 0;
   }

   CONSTEXPR void update_len()
   {
      *ptr = 0;
      data_view(bytes).store_le_uint32((ptr - bytes) + 1);
   }

   private:

   uint8_t *bytes;
   uint8_t *ptr;
};

}
