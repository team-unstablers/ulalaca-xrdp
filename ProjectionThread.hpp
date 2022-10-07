//
// Created by Gyuhwan Park on 2022/05/06.
//

#ifndef ULALACA_PROJECTIONTHREAD_HPP
#define ULALACA_PROJECTIONTHREAD_HPP

#include <memory>
#include <thread>
#include <queue>
#include <future>

#include "UnixSocket.hpp"

#include "IPCConnection.hpp"
#include "messages/projector.h"

#include "ulalaca.hpp"


class ProjectionThread {
public:
    explicit ProjectionThread(
        XrdpUlalaca &xrdpUlalaca,
        const std::string &socketPath
    );
    ProjectionThread(ProjectionThread &) = delete;
    
    void start();
    void stop();
    
    void handleEvent(XrdpEvent &event);
    void setViewport(ULIPCRect rect);
    
    void setOutputSuppression(bool isOutputSuppressed);
private:
    void mainLoop();
    
    XrdpUlalaca &_xrdpUlalaca;
    IPCConnection _ipcConnection;
    
    bool _isTerminated;
    std::thread _projectorThread;
};

#endif //ULALACA_PROJECTIONTHREAD_HPP
