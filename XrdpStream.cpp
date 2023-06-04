//
// Created by Gyuhwan Park on 2022/04/30.
//

#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

#include "ulalaca.hpp"
#include "XrdpStream.hpp"


static inline XrdpStream::Stream *createXrdpStreamInternal(size_t size) {
    XrdpStream::Stream *stream;
    make_stream(stream);
    init_stream(stream, size);
    
    return stream;
}

static void XrdpStreamDeleter(XrdpStream::Stream *stream) {
    free_stream(stream);
}

XrdpStream::XrdpStream(size_t size):
    _stream { createXrdpStreamInternal(size), XrdpStreamDeleter }
{

}

XrdpStream::operator Stream const *() const {
    return _stream.get();
}

void XrdpStream::write(uint8_t value) {
    out_uint8(_stream.get(), value);
}

void XrdpStream::write(uint16_t value) {
    out_uint16_le(_stream.get(), value);
}

void XrdpStream::write(uint32_t value) {
    out_uint32_le(_stream.get(), value);
}

void XrdpStream::write(uint64_t value) {
    out_uint64_le(_stream.get(), value);
}

uint8_t XrdpStream::readUInt8() {
    uint8_t value = 0;
    in_uint8(_stream.get(), value);

    return value;
}

uint16_t XrdpStream::readUInt16() {
    uint16_t value = 0;
    in_uint16_le(_stream.get(), value);

    return value;
}

uint32_t XrdpStream::readUInt32() {
    uint32_t value = 0;
    in_uint32_le(_stream.get(), value);

    return value;
}

uint64_t XrdpStream::readUInt64() {
    uint64_t value = 0;
    in_uint64_le(_stream.get(), value);

    return value;
}

void XrdpStream::write(const uint8_t *data, size_t size) {
    out_uint8a(_stream.get(), data, size);
}

ptrdiff_t XrdpStream::size() const {
    return _stream->end - _stream->data;
}

ptrdiff_t XrdpStream::distance() const {
    return _stream->end - _stream->p;
}

void XrdpStream::seekTo(ptrdiff_t offset) {
    if (offset < 0) {
        _stream->p = _stream->data;
    } else if (offset > size()) {
        _stream->p = _stream->end;
    } else {
        _stream->p = _stream->data + offset;
    }
}

void XrdpStream::markChannelHeader(int offset) {
    s_push_layer(_stream.get(), channel_hdr, offset);
}

void XrdpStream::seekToChannelHeader() {
    s_pop_layer(_stream.get(), channel_hdr);
}

const char *XrdpStream::begin() const {
    return _stream->data;
}

void XrdpStream::seekToBegin() {
    _stream->p = _stream->data;
}

const char *XrdpStream::end() const {
    return _stream->end;
}

void XrdpStream::markEnd() {
    s_mark_end(_stream.get());
}

