//
// Created by cheesekun on 2/28/22.
//

#ifndef ULALACA_SOCKETSTREAM_HPP
#define ULALACA_SOCKETSTREAM_HPP

#include <streambuf>

#include "UnixSocket.hpp"

class InputSocketStream: public std::basic_streambuf<uint8_t, std::char_traits<uint8_t>> {
public:
    explicit InputSocketStream(UnixSocketBase &socketLike);

protected:
    static const size_t MAX_BUFFER_SIZE = 8192;
    
    int underflow() override;
    
    uint8_t _buffer[MAX_BUFFER_SIZE];
    UnixSocketBase &_socket;
};

class OutputSocketStream: public std::streambuf {
public:
    explicit OutputSocketStream(UnixSocketBase &socketLike);
    
    void flush();
protected:
    static const size_t MAX_BUFFER_SIZE = 8192;
    
    int overflow(int character) override;
    
    char _buffer[MAX_BUFFER_SIZE];
    UnixSocketBase &_socket;
};


#endif //ULALACA_SOCKETSTREAM_HPP
