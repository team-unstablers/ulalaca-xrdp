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

#include "ProjectionTarget.hpp"


class ProjectorClient {
public:
    explicit ProjectorClient(
        ProjectionTarget &target,
        const std::string &socketPath
    );
    ProjectorClient(ProjectorClient &) = delete;

    FD descriptor();

    void start();
    void stop();
    
    void handleEvent(XrdpEvent &event);
    void setViewport(ULIPCRect rect);
    
    void setOutputSuppression(bool isOutputSuppressed);

private:
    void mainLoop();

    ProjectionTarget &_target;
    IPCConnection _ipcConnection;
    
    bool _isTerminated;
    std::thread _projectorThread;
};

#endif //ULALACA_PROJECTIONTHREAD_HPP
