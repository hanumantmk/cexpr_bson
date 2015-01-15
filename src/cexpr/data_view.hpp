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

      CONSTEXPR void store_le_double(double v) {
         uint64_t bytes = 0;

         if (v != 0.0) {
            bool is_negative = v < 0;

            if (is_negative) {
               v = -v;
            }

            uint64_t mantissa = 0;
            {
               double tmp = v;

               while (double(int64_t(tmp)) != tmp) {
                  tmp *= 2;
               }

               while (double(int64_t(tmp)) == tmp) {
                  tmp /= 2;
               }
               tmp *= 2;

               mantissa = tmp;
            }

            uint64_t exp = 1023;
            {
               double tmp = v;

               while (tmp > 1) {
                  tmp /= 2;
                  exp++;
               }

               while (tmp < 1) {
                  tmp *= 2;
                  exp--;
               }
            }

            int mantissa_bits = 0;
            {
               uint64_t tmp = mantissa;

               while (tmp) {
                  mantissa_bits++;
                  tmp >>= 1;
               }
            }

            mantissa_bits--;

            bytes = mantissa - (1ul << mantissa_bits);

            bytes <<= (52 - mantissa_bits);

            exp <<= 52;

            bytes |= exp;

            if (is_negative) {
               bytes |= 1ul << 63;
            }
         }

         store_le_uint64(bytes);
      }

   private:
      uint8_t *bytes;
};

}
