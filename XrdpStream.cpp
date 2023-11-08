//
// Created by Gyuhwan Park on 2022/04/30.
//

#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

#include "ulalaca.hpp"

static inline XrdpStream::Stream *createXrdpStreamInternal(size_t size) {
    XrdpStream::Stream *stream;
    int p = (int)size;

#pragma clang diagnostic push /*임시로 경고 단계 조정*/
#pragma clang diagnostic ignored "-Wsign-conversion"

    make_stream(stream);
    init_stream(stream, p);

#pragma clang diagnostic pop /*경고단계 복구*/

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
