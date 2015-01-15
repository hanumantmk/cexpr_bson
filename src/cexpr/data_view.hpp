#include <cstdint>
#include <cstring>

#pragma once

namespace cexpr {

class data_view {
   public:
      CONSTEXPR data_view(uint8_t *b) : bytes(b) {}

      CONSTEXPR void store_le_uint32(uint32_t v) {
         bytes[0] = v & 0xFF;
         bytes[1] = (v >> 8) & 0xFF;
         bytes[2] = (v >> 16) & 0xFF;
         bytes[3] = (v >> 24) & 0xFF;
      }

      CONSTEXPR void store_le_uint64(uint64_t v) {
         bytes[0] = v & 0xFFul;
         bytes[1] = (v >> 8) & 0xFFul;
         bytes[2] = (v >> 16) & 0xFFul;
         bytes[3] = (v >> 24) & 0xFFul;
         bytes[4] = (v >> 32) & 0xFFul;
         bytes[5] = (v >> 40) & 0xFFul;
         bytes[6] = (v >> 48) & 0xFFul;
         bytes[7] = (v >> 56) & 0xFFul;
      }

      CONSTEXPR void store_le_int32(int32_t v) {
         uint32_t b = 0;

         if (v > 0) {
            b = v;
         } else {
            b = 0xFFFFFFFF + v + 1;
         }

         store_le_uint32(b);
      }

      CONSTEXPR void store_le_int64(int64_t v) {
         uint64_t b = 0;

         if (v > 0) {
            b = v;
         } else {
            b = 0xFFFFFFFFFFFFFFFFUL + v + 1;
         }

         store_le_uint64(b);
      }
   private:
      uint8_t *bytes;
};

}
