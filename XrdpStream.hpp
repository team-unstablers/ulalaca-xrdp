//
// Created by Gyuhwan Park on 2022/04/30.
//

#ifndef XRDP_XRDPSTREAM_HPP
#define XRDP_XRDPSTREAM_HPP

#include <memory>
#include <functional>

extern "C" {
#include "arch.h"
#include "parse.h"
#include "os_calls.h"
#include "defines.h"
};

class XrdpStream {
public:
    using Stream = stream;
    using StreamDeleter = std::function<void(Stream *)>;
    
    XrdpStream(size_t size);
    
    explicit operator Stream const *() const;
    
    template<typename T>
    XrdpStream &operator<< (T value);

private:
    std::unique_ptr<Stream, StreamDeleter> _stream;
};

#endif //XRDP_XRDPSTREAM_HPP
