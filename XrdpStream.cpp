//
// Created by Gyuhwan Park on 2022/04/30.
//

#include "ulalaca.hpp"

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
