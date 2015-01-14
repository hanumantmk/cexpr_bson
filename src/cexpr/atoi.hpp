#include <cstdint>
#include <cstring>

#pragma once

namespace cexpr {

CONSTEXPR int64_t atoi(const char *str, std::size_t len)
{
   int64_t out = 0;
   int64_t power = str[0] == '-' ? -1 : 1;

   for (std::size_t i = 0; i < len - ((str[0] == '-' || str[0] == '+') ? 1 : 0); i++) {
      out += (str[len - (i + 1)] - '0') * power;
      power *= 10;
   }

   return out;
}

}
