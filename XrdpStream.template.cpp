#ifndef __ULALACA_XRDPSTREAM_TEMPLATE_CPP__
#define __ULALACA_XRDPSTREAM_TEMPLATE_CPP__

#include "XrdpStream.hpp"

template <>
XrdpStream &XrdpStream::operator<<(uint8_t value) {
    write(value);

    return *this;
}

template <>
XrdpStream &XrdpStream::operator<<(uint16_t value) {
    write(value);

    return *this;
}

template <>
XrdpStream &XrdpStream::operator<<(uint32_t value) {
    write(value);

    return *this;
}

template <>
XrdpStream &XrdpStream::operator<<(uint64_t value) {
    write(value);

    return *this;
}

#endif