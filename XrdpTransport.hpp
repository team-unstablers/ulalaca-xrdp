//
// Created by Gyuhwan Park on 2022/04/30.
//

#ifndef ULALACA_XRDPTRANSPORT_HPP
#define ULALACA_XRDPTRANSPORT_HPP

#include <memory>
#include <vector>

extern "C" {
#include "trans.h"
};

#include "XrdpStream.hpp"


class XrdpTransport {
public:
    using Transport = trans;
    using TransportDeleter = std::function<void(Transport *)>;
    
    using TransportBus = tbus;
    
    template <typename T>
    using List = std::vector<T>;
    
    XrdpTransport(int mode, int inSize, int outSize);
    
    std::unique_ptr<List<TransportBus>> getWaitObjs();
    
    int forceReadS(XrdpStream &inStream, size_t size);
    int forceWriteS(XrdpStream &outStream);
    
    int forceRead(size_t size);
    int forceWrite();
    
    int writeCopy();
    int writeCopyS(XrdpStream &outStream);
    
    int connect(std::string server, std::string port, int timeout);
    int listenAddress(std::string port, std::string address);
    
    XrdpStream &getInStream();
    XrdpStream &getOutStream(size_t size);
    
    int setTLSMode(std::string key, std::string cert, long sslProtocols, std::string tlsCiphers);
    int shutdownTLSMode();
    
    int tcpForceReadS(XrdpStream &inStream, size_t size);
    
private:
    std::unique_ptr<Transport, TransportDeleter> _transport;
};

#endif //ULALACA_XRDPTRANSPORT_HPP
