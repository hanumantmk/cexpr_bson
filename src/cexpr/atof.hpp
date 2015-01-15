//#define CONSTEXPR constexpr

#include <cstdint>
#include <cstring>

#pragma once

namespace cexpr {

CONSTEXPR double atof(const char *str, std::size_t len)
{
   double out = 0.0;
   const char *end = str + len;
   bool in_fraction = false;
   int fractional_digits = 0;
   bool is_negative = false;

   if (str[0] == '-') {
      is_negative = true;
      str++;
   } else if (str[0] == '+') {
      str++;
   }

   for (;str != end;str++) {
      if (str[0] >= '0' && str[0] <= '9') {
         if (in_fraction) {
            fractional_digits++;
         }

         out *= 10;
         out += str[0] - '0';
      } else if (str[0] == '.') {
         in_fraction = true;
      }
   }

   while (fractional_digits) {
      out /= 10.0;
      fractional_digits--;
   }

   return is_negative ? -out : out;
}

}
