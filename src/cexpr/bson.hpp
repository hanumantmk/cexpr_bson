//#define CONSTEXPR constexpr

#include <cstdint>
#include <cstring>
#include "cexpr/data_view.hpp"
#include "cexpr/jsmn.h"
#include "cexpr/atoi.hpp"
#include "cexpr/atof.hpp"
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
   b_bool = 0x08,
   b_null = 0x0a,
   b_int32 = 0x10,
   b_int64 = 0x12,
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

   CONSTEXPR void append_int64(const char *key, std::size_t klen, int64_t v) {
      append_prefix(key, klen, bson_type::b_int64);

      data_view(ptr).store_le_int64(v);
      ptr += 8;

      update_len();
   }

   CONSTEXPR void append_double(const char *key, std::size_t klen, double f) {
      append_prefix(key, klen, bson_type::b_double);

      data_view(ptr).store_le_double(f);
      ptr += 8;

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

   CONSTEXPR void append_bool(const char *key, std::size_t klen, bool v) {
      append_prefix(key, klen, bson_type::b_bool);

      *ptr++ = v ? 0x01 : 0x00;

      update_len();
   }

   CONSTEXPR void append_null(const char *key, std::size_t klen) {
      append_prefix(key, klen, bson_type::b_null);

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

   CONSTEXPR void append_int64(const char *key, std::size_t klen, int64_t) {
      append_prefix(key, klen, bson_type::b_int64);

      len += 8;
   }

   CONSTEXPR void append_double(const char *key, std::size_t klen, double) {
      append_prefix(key, klen, bson_type::b_double);

      len += 8;
   }

   CONSTEXPR void append_utf8(const char *key, std::size_t klen, const char *, std::size_t vlen) {
      append_prefix(key, klen, bson_type::b_utf8);

      len += 4;
      len += vlen;
      len++;
   }

   CONSTEXPR void append_bool(const char *key, std::size_t klen, bool v) {
      append_prefix(key, klen, bson_type::b_bool);

      len++;
   }

   CONSTEXPR void append_null(const char *key, std::size_t klen) {
      append_prefix(key, klen, bson_type::b_null);
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
   bool is_fraction = false;

   for (std::size_t i = 0; i < vlen; i++) {
      if (v[i] == '.') {
         is_fraction = true;
      }
   }

   if (is_fraction) {
      double f = atof(v, vlen);

      b.append_double(key, klen, f);
   } else {
      int64_t x = atoi(v, vlen);
      int64_t y = x;

      if (y < 0) {
         y *= -1;
      }

      if (y >= (1ul << 32)) {
         b.append_int64(key, klen, x);
      } else {
         b.append_int32(key, klen, x);
      }
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
               case 't':
               case 'T':
                  b.append_bool(key, key_len, true);
                  break;
               case 'f':
               case 'F':
                  b.append_bool(key, key_len, false);
                  break;
               case 'n':
               case 'N':
                  b.append_null(key, key_len);
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
