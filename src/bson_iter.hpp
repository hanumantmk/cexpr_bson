#pragma once

#include <iostream>
#include <cstring>
#include <cstdint>

class bson_iter {
   public:
   bson_iter (const uint8_t* b) : bytes(b), ptr(b) {
      uint32_t x = 0;
      std::memcpy (&x, b, 4);

      l = x;
   }

   const char * utf8() const {
      return (const char *)ptr + name_skip() + 4;
   }

   int32_t int32() const {
      int32_t i32 = 0;
      std::memcpy(&i32, ptr + name_skip(), 4);

      return i32;
   }

   int64_t int64() const {
      int64_t i64 = 0;
      std::memcpy(&i64, ptr + name_skip(), 8);

      return i64;
   }

   double dbl() const {
      double d = 0;
      std::memcpy(&d, ptr + name_skip(), 8);

      return d;
   }

   bool bl() const {
      return ptr[name_skip()];
   }

   bson_iter recurse() const {
      return bson_iter(ptr + name_skip());
   }

   uint8_t type() const {
      return ptr[0];
   }

   const char * key() const {
      return (const char *)ptr + 1;
   }

   bool next() {
      if (ptr == bytes) {
         ptr += 4;
      } else {
         uint32_t ns = name_skip();

         switch (*ptr) {
            case 0x01:
               ns += 8;
               break;
            case 0x02:
            case 0x03:
            case 0x04:
            {
               const uint8_t *p = ptr + ns;
               uint32_t x = 0;
               std::memcpy (&x, p, 4);

               ns += x + 4;

               break;
            }
            case 0x07:
            case 0x08:
               ns++;
               break;
            case 0x0a:
               break;
            case 0x10:
               ns += 4;
               break;
            case 0x12:
               ns += 8;
               break;
         }

         ptr += ns;
      }

      return *ptr != 0;
   }

   uint32_t len() const {
      return l;
   }

   void json(std::ostream& os, bool is_array = false) const {
      bool first = true;

      bson_iter bi = *this;

      if (is_array) {
         os << "[";
      } else {
         os << "{";
      }

      while (bi.next()) {
         if (first) {
            first = false;
         } else {
            os << ", ";
         }

         if (! is_array) {
            os << "\"" << bi.key() << "\" : ";
         }

         switch (bi.type()) {
            case 0x01:
               os << bi.dbl();
               break;
            case 0x02:
               os << "\"" << bi.utf8() << "\"";
               break;
            case 0x03:
            {
               bson_iter child = bi.recurse();
               child.json(os, false);

               break;
            }
            case 0x04:
            {
               bson_iter child = bi.recurse();
               child.json(os, true);

               break;
            }
            case 0x07:
            case 0x08:
               os << (bi.bl() ? "true" : "false");
               break;
            case 0x0a:
               os << "null";
               break;
            case 0x10:
               os << bi.int32();
               break;
            case 0x12:
               os << bi.int64();
               break;
         }
      }

      if (is_array) {
         os << "]";
      } else {
         os << "}";
      }
   }

   const uint8_t *bytes;
   private:

   uint32_t name_skip() const {
      const uint8_t *p;

      for (p = ptr + 1; *p; p++) {}

      return (1 + (p - ptr));
   }

   const uint8_t *ptr;
   uint32_t l;
};
