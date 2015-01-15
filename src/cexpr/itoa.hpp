#pragma once

#include <cstddef>
#include <cstdint>

namespace cexpr {

class itoa {
   public:
    CONSTEXPR itoa();
    CONSTEXPR itoa(uint32_t i);
    CONSTEXPR itoa(const itoa& rhs);
    CONSTEXPR itoa& operator=(const itoa& rhs);
    CONSTEXPR itoa& operator=(uint32_t i);

    CONSTEXPR const char* c_str() const;
    CONSTEXPR std::size_t length() const;

   private:
    CONSTEXPR void init();

    uint32_t _val;
    const char* _str;
    uint8_t _len;
    char _buf[11];
};

CONSTEXPR itoa::itoa(uint32_t val) : _val(val), _str(nullptr), _len(0), _buf() { init(); }

CONSTEXPR void itoa::init() {
     int size = sizeof(_buf) - 1;
     int i = size;

     _buf[i] = '\0';

     while (_val > 0) {
         i--;
         _buf[i] = (_val % 10) + '0';
         _val = _val / 10;
     }

     _str = _buf + i;
     _len = size - i;
}

CONSTEXPR itoa::itoa() : itoa(0) {}

CONSTEXPR itoa::itoa(const itoa& rhs) : itoa(rhs._val) {}

CONSTEXPR itoa& itoa::operator=(const itoa& rhs) {
    _val = rhs._val;
    init();
    return *this;
}

CONSTEXPR itoa& itoa::operator=(uint32_t i) {
    _val = i;
    init();
    return *this;
}

CONSTEXPR const char* itoa::c_str() const { return _str; }

CONSTEXPR std::size_t itoa::length() const { return _len; }

}
