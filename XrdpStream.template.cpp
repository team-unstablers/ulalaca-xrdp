#ifndef __ULALACA_XRDPSTREAM_TEMPLATE_CPP__
#define __ULALACA_XRDPSTREAM_TEMPLATE_CPP__

#include "ulalaca.hpp"

template <>
XrdpStream &XrdpStream::operator<<(uint8_t value) {
    out_uint8(_stream, value);
}

template <>
XrdpStream &XrdpStream::operator<<(uint16_t value) {
    out_uint16_le(_stream, value);
}

template <>
XrdpStream &XrdpStream::operator<<(uint32_t value) {
    out_uint32_le(_stream, value);
}

template <>
XrdpStream &XrdpStream::operator<<(uint64_t value) {
    out_uint64_le(_stream, value);
}

#endif