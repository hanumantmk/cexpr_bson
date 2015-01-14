#include <cstdint>
#include <cstring>

#pragma once

namespace cexpr {

class data_view {
   public:
      CONSTEXPR data_view(uint8_t *b) : bytes(b) {}

      CONSTEXPR uint32_t load_le_uint32() const {
         uint32_t v = (
            bytes[0] |
            bytes[1] << 8 |
            bytes[2] << 16 |
            bytes[3] << 24
         );

         return v;
      }

      CONSTEXPR void store_le_uint32(uint32_t v) {
         bytes[0] = v & 0xFF;
         bytes[1] = (v >> 8) & 0xFF;
         bytes[2] = (v >> 16) & 0xFF;
         bytes[3] = (v >> 24) & 0xFF;
      }

      CONSTEXPR int32_t load_le_int32() const {
         uint32_t v = load_le_uint32();

         if (v < 1u << 31) {
            return v;
         } else if (v == 1u << 31) {
            return -(1 << 31);
         } else {
            return -(int32_t)(~v + 1);
         }
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

   private:
      uint8_t *bytes;
};

}
