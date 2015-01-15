//#define CONSTEXPR constexpr

#include <cstdint>
#include <cstring>
#include "cexpr/data_view.hpp"
#include "cexpr/jsmn.h"
#include "cexpr/atoi.hpp"
#include "cexpr/itoa.hpp"

#pragma once

#define CEXPR_BSON_FROM_JSON(name, my_str) \
struct MAGIC_IMPL_##name { \
   CONSTEXPR static const char *str() { return my_str; }; \
   CONSTEXPR static std::size_t str_len() { return sizeof(my_str); }; \
}; \
CONSTEXPR auto name = from_json<MAGIC_IMPL_##name>()

namespace cexpr {

enum class bson_type : uint8_t {
   b_double = 0x01,
   b_utf8 = 0x02,
   b_doc = 0x03,
   b_array = 0x04,
   b_int32 = 0x10,
};

class bson {
   public:
   CONSTEXPR bson() : bytes(nullptr), ptr(nullptr) {
   }

   CONSTEXPR bson(uint8_t *b) : bytes(b), ptr(b + 4) {
      update_len();
   }

   CONSTEXPR void append_int32(const char *key, std::size_t klen, int32_t v) {
      append_prefix(key, klen, bson_type::b_int32);

      data_view(ptr).store_le_int32(v);
      ptr += 4;

      update_len();
   }

   CONSTEXPR void append_bytes(const char *key, std::size_t klen, bson_type bt, uint8_t *v, std::size_t vlen) {
      append_prefix(key, klen, bt);

      for (std::size_t i = 0; i < vlen; i++) {
         *ptr++ = v[i];
      }

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

   CONSTEXPR void append_document_begin(const char *key, std::size_t klen, bson& b) {
      append_prefix(key, klen, bson_type::b_doc);

      b.bytes = ptr;
      b.ptr = b.bytes + 4;
      b.update_len();
   }

   CONSTEXPR void append_document_end(bson& b) {
      ptr = b.ptr + 1;
      update_len();
   }

   CONSTEXPR void append_array_begin(const char *key, std::size_t klen, bson& b) {
      append_prefix(key, klen, bson_type::b_array);

      b.bytes = ptr;
      b.ptr = b.bytes + 4;
      b.update_len();
   }

   CONSTEXPR void append_array_end(bson& b) {
      append_document_end(b);
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

   CONSTEXPR std::size_t length() const {
      return (ptr - bytes) + 1;
   }

   private:

   uint8_t *bytes;
   uint8_t *ptr;
};

class bson_sizer {
   public:
   CONSTEXPR bson_sizer() : len(5) {
   }

   CONSTEXPR bson_sizer(uint8_t *) : len(5) {
   }

   CONSTEXPR void append_int32(const char *key, std::size_t klen, int32_t) {
      append_prefix(key, klen, bson_type::b_int32);

      len += 4;
   }

   CONSTEXPR void append_bytes(const char *key, std::size_t klen, bson_type bt, uint8_t *, std::size_t vlen) {
      append_prefix(key, klen, bt);

      len += vlen;
   }

   CONSTEXPR void append_utf8(const char *key, std::size_t klen, const char *, std::size_t vlen) {
      append_prefix(key, klen, bson_type::b_utf8);

      len += 4;
      len += vlen;
      len++;
   }

   CONSTEXPR void append_document_begin(const char *key, std::size_t klen, bson_sizer& bs) {
      append_prefix(key, klen, bson_type::b_doc);

      bs.len = 5;
   }

   CONSTEXPR void append_document_end(bson_sizer& bs) {
      len += bs.length();
   }

   CONSTEXPR void append_array_begin(const char *key, std::size_t klen, bson_sizer& bs) {
      append_prefix(key, klen, bson_type::b_array);

      bs.len = 5;
   }

   CONSTEXPR void append_array_end(bson_sizer& bs) {
      len += bs.length();
   }

   CONSTEXPR void append_prefix(const char *, std::size_t klen, bson_type)
   {
      len++;
      len += klen;
      len++;
   }

   CONSTEXPR std::size_t length() const {
      return len;
   }

   private:
   std::size_t len;
};

template <typename T>
CONSTEXPR void append_num(T& b, const char *key, std::size_t klen, const char *v, std::size_t vlen)
{
   const char *value = v;
   const char *end = value + vlen;
   bool is_negative = false;
   bool in_fraction = false;
//   uint32_t exponent = 0;
   uint64_t integer = 0;
   uint64_t floating = 0;
   uint64_t floating_compare = 0;
   uint64_t mantissa = 0;

   if (value[0] == '-') {
      is_negative = true;
      value++;
   } else if (value[0] == '+') {
      value++;
   }

   for (;value != end;value++) {
      if (value[0] >= '0' && value[0] <= '9') {
         if (in_fraction) {
            floating_compare *= 10;
            floating *= 10;
            floating += value[0] - '0';
         } else {
            integer *= 10;
            integer += value[0] - '0';
         }
      } else if (value[0] == '.') {
         in_fraction = true;
         floating_compare = 1;
      }
   }

   if (in_fraction) {
      mantissa = integer;

      uint64_t tmp = mantissa;

      int mantissa_bits = 0;
      while (tmp) {
         mantissa_bits++;
         tmp >>= 1;
      }

      uint32_t zero_float_shifts = 0;
      bool non_zero_float_shift = integer != 0;

      while (floating && mantissa_bits < 52) {
         floating <<= 1;
         mantissa <<= 1;

         if (floating >= floating_compare) {
            non_zero_float_shift = true;
            floating -= floating_compare;
            mantissa |= 1;
         } else if (! non_zero_float_shift) {
            zero_float_shifts++;
         }

         if (non_zero_float_shift) {
            mantissa_bits++;
         }
      }

      /* TODO: this is almost certainly wrong.  Find out how rounding is
       * actually supposed to work for repeating fractions */
      if (mantissa_bits == 52) {
         mantissa |= 1;
      }

      mantissa_bits--;

      uint64_t bytes = mantissa;

      bytes = mantissa - (1ul << mantissa_bits);

      bytes <<= (52 - mantissa_bits);

      uint64_t exp = 1022;

      if (integer) {
         while (integer) {
            exp++;
            integer >>= 1;
         }
      } else {
         exp -= zero_float_shifts;
      }

      exp <<= 52;


      bytes |= exp;
      if (is_negative) {
         bytes |= 1ul << 63;
      }

      uint8_t buf[8] = {
         static_cast<uint8_t>((bytes & 0x00000000000000FFul) >> 0),
         static_cast<uint8_t>((bytes & 0x000000000000FF00ul) >> 8),
         static_cast<uint8_t>((bytes & 0x0000000000FF0000ul) >> 16),
         static_cast<uint8_t>((bytes & 0x00000000FF000000ul) >> 24),
         static_cast<uint8_t>((bytes & 0x000000FF00000000ul) >> 32),
         static_cast<uint8_t>((bytes & 0x0000FF0000000000ul) >> 40),
         static_cast<uint8_t>((bytes & 0x00FF000000000000ul) >> 48),
         static_cast<uint8_t>((bytes & 0xFF00000000000000ul) >> 56),
      };

      b.append_bytes(key, klen, bson_type::b_double, buf, 8);
   } else {
      b.append_int32(key, klen, atoi(v, vlen));
   }
}

template <typename T>
CONSTEXPR void parse_impl(T& b, jsmn::jsmntok_t *toks, const char * v, int& i, int r, bool is_array)
{
   using namespace jsmn;

   const char *key = nullptr;
   const char *value = nullptr;
   std::size_t key_len = 0;
   std::size_t value_len = 0;

   std::size_t index = 0;

   bool lf_key = true;

   itoa itoa(index);

   for (; i < r; i++) {
      if (is_array) {
         itoa = index;
         key = itoa.c_str();
         key_len = itoa.length();
      }

      switch (toks[i].type) {
         case JSMN_PRIMITIVE:
            value = v + toks[i].start;
            value_len = toks[i].end - toks[i].start;

            switch (value[0]) {
               case '+':
               case '-':
               case '0':
               case '1':
               case '2':
               case '3':
               case '4':
               case '5':
               case '6':
               case '7':
               case '8':
               case '9':
                  append_num(b, key, key_len, value, value_len);
                  break;
            }

            index++;
            lf_key = true;
            break;
         case JSMN_OBJECT:
            {
               T child;
               value = v + toks[i].start;
               value_len = toks[i].end - toks[i].start;

               std::size_t children = toks[i].size * 2;

               i++;

               b.append_document_begin(key, key_len, child);
               parse_impl(child, toks, v, i, i + children, false);
               b.append_document_end(child);

               lf_key = true;
               index++;
            }
            break;
         case JSMN_ARRAY:
            {
               T child;
               value = v + toks[i].start;
               value_len = toks[i].end - toks[i].start;

               std::size_t children = toks[i].size;

               i++;

               b.append_array_begin(key, key_len, child);
               parse_impl(child, toks, v, i, i + children, true);
               b.append_array_end(child);

               lf_key = true;
               index++;
            }
            break;
         case JSMN_STRING:
            if (! is_array && lf_key) {
               key = v + toks[i].start;
               key_len = toks[i].end - toks[i].start;

               lf_key = false;
            } else {
               value = v + toks[i].start;
               value_len = toks[i].end - toks[i].start;

               b.append_utf8(key, key_len, value, value_len);

               lf_key = true;
               index++;
            }
            break;
         default:
            break;
      }

   }
}

template <typename S>
CONSTEXPR std::size_t parse_toks()
{
   using namespace jsmn;
   jsmn_parser p = {};

   jsmn_init(&p);

   int r = jsmn_parse(&p, S::str(), S::str_len(), nullptr, 0);

   if (r < 0) {
      return 0;
   } else {
      return r;
   }
}

template <typename T, typename S>
CONSTEXPR std::size_t parse(T b)
{
   using namespace jsmn;

   const char *v = S::str();
   std::size_t len = S::str_len();

   jsmn_parser p = {};
#ifdef CONSTEXPR_OFF
   jsmntok_t toks[1000] = {};
#else
   jsmntok_t toks[parse_toks<S>()] = {};
#endif

   jsmn_init(&p);

   int r = jsmn_parse(&p, v, len, toks, sizeof(toks)/sizeof(toks[0]));

   if (r < 0) {
      return 1;
   }

   int i = 1;

   parse_impl(b, toks, v, i, r, false);

   return b.length();
}

template <typename T>
class from_json {
public:
#ifdef CONSTEXPR_OFF
   CONSTEXPR from_json() : a(), l(0) {
      l = parse<bson, T>(bson(a));
   }
#else
   CONSTEXPR from_json() : a() {
      parse<bson, T>(bson(a));
   }
#endif

   CONSTEXPR const uint8_t *data () const {
      return a;
   }

   CONSTEXPR std::size_t len() const {
#ifdef CONSTEXPR_OFF
      return l;
#else
      return sizeof(a);
#endif
   }

private:
#ifdef CONSTEXPR_OFF
   std::uint8_t a[1024];
   std::size_t l;
#else
   std::uint8_t a[parse<bson_sizer, T>(bson_sizer())];
#endif
};

}
