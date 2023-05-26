//
// Created by Gyuhwan Park on 2023/05/26.
//

#include "MockedUnixSocket.hpp"

MockedUnixSocket::MockedUnixSocket(const std::string path):
    UnixSocket(path),

    _mockedDataLength(0),
    _writtenDataLength(0)
{

}

ssize_t MockedUnixSocket::read(void *buffer, size_t size) {
    if (_readShouldFail) {
        return -1;
    }

    if (_mockedDataLength > size) {
        memcpy(buffer, _mockedData.get(), size);
        _mockedDataLength -= size;
        return size;
    } else {
        memcpy(buffer, _mockedData.get(), _mockedDataLength);
        auto ret = _mockedDataLength;
        _mockedDataLength = 0;
        return ret;
    }
}

ssize_t MockedUnixSocket::write(const void *buffer, size_t size) {
    if (_writeShouldFail) {
        return -1;
    }

    _writtenData = std::shared_ptr<uint8_t>(new uint8_t[size]);
    memcpy(_writtenData.get(), buffer, size);
    _writtenDataLength = size;

    return size;
}

pollfd MockedUnixSocket::poll(short events, int timeout) {
    if (_pollShouldFail) {
        throw SystemCallException(EAGAIN, "poll");
    }

    return pollfd {
        descriptor(), events, events
    };
}

int MockedUnixSocket::fcntl(int cmd, int arg) {
    return 0;
}

void MockedUnixSocket::close() {

}

void MockedUnixSocket::bind() {

}

void MockedUnixSocket::listen() {

}

UnixSocketConnection MockedUnixSocket::accept() {

}

void MockedUnixSocket::connect() {
    if (_connectShouldFail) {
        throw SystemCallException(ENOENT, "connect");
    }
}

FD MockedUnixSocket::descriptor() {
    return 5;
}

void MockedUnixSocket::setMockedData(std::shared_ptr<uint8_t> data, size_t length) {
    _mockedData = data;
    _mockedDataLength = length;
}

std::shared_ptr<uint8_t> MockedUnixSocket::writtenData() const {
    return _writtenData;
}

size_t MockedUnixSocket::writtenDataLength() const {
    return _writtenDataLength;
}

void MockedUnixSocket::setConnectShouldFail(bool connectShouldFail) {
    _connectShouldFail = connectShouldFail;
}

void MockedUnixSocket::setReadShouldFail(bool readShouldFail) {
    _readShouldFail = readShouldFail;
}

void MockedUnixSocket::setWriteShouldFail(bool writeShouldFail) {
    _writeShouldFail = writeShouldFail;
}

void MockedUnixSocket::setPollShouldFail(bool pollShouldFail) {
    _pollShouldFail = pollShouldFail;
}