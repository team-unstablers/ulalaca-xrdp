//
// Created by Gyuhwan Park on 2023/05/26.
//

#ifndef XRDP_TUMOD_MOCKEDUNIXSOCKET_HPP
#define XRDP_TUMOD_MOCKEDUNIXSOCKET_HPP

#include "../UnixSocket.hpp"

class MockedUnixSocket: public UnixSocket {
public:
    explicit MockedUnixSocket(const std::string path);

    ssize_t read(void *buffer, size_t size) override;
    ssize_t write(const void *buffer, size_t size) override;
    pollfd poll(short events, int timeout) override;
    int fcntl(int cmd, int arg) override;

    void close() override;
    void bind() override;
    void listen() override;
    UnixSocketConnection accept() override;
    void connect() override;
    FD descriptor() override;

    void setMockedData(std::shared_ptr<uint8_t> data, size_t length);

    std::shared_ptr<uint8_t> writtenData() const;
    size_t writtenDataLength() const;

    void setConnectShouldFail(bool connectShouldFail);
    void setReadShouldFail(bool readShouldFail);
    void setWriteShouldFail(bool writeShouldFail);
    void setPollShouldFail(bool pollShouldFail);

private:
    std::shared_ptr<uint8_t> _mockedData;
    size_t _mockedDataLength;

    std::shared_ptr<uint8_t> _writtenData;
    size_t _writtenDataLength;

    bool _connectShouldFail;
    bool _readShouldFail;
    bool _writeShouldFail;
    bool _pollShouldFail;
};



#endif //XRDP_TUMOD_MOCKEDUNIXSOCKET_HPP
