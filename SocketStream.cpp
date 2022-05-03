//
// Created by cheesekun on 2/28/22.
//

#include "SocketStream.hpp"


InputSocketStream::InputSocketStream(UnixSocketBase &socketLike):
    _socket(socketLike),
    _buffer()
{

}

int InputSocketStream::underflow() {
    if (this->gptr() == this->egptr()) {
        auto read = _socket.read(_buffer, MAX_BUFFER_SIZE);
        if (read <= 0) {
            // 더 이상 읽을 수 있는 데이터가 없음
            return traits_type::eof();
        }
        
        this->setg(
            _buffer, _buffer,
            _buffer + read
        );
    }
    
    // TODO: assert(this->gptr() != this->egptr());
    return traits_type::to_int_type(*this->gptr());
}

OutputSocketStream::OutputSocketStream(UnixSocketBase &socketLike):
    _socket(socketLike),
    _buffer()
{
    this->setp(_buffer, _buffer + MAX_BUFFER_SIZE);
}

void OutputSocketStream::flush() {
    ptrdiff_t distance = this->pptr() - _buffer;
    auto written = _socket.write(_buffer, distance);
    this->setp(_buffer, _buffer + MAX_BUFFER_SIZE);
}

int OutputSocketStream::overflow(int character) {
    if (this->pptr() == this->epptr()) {
        this->flush();
    }
    
    return traits_type::to_int_type(character);
}


