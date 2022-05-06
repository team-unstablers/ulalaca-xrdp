//
// Created by Gyuhwan Park on 2022/05/06.
//

#ifndef XRDP_PROJECTIONTHREAD_HPP
#define XRDP_PROJECTIONTHREAD_HPP

#include <memory>
#include <thread>
#include <queue>
#include <future>

#include "UnixSocket.hpp"

#include "ProjectionContext.hpp"
#include "UlalacaMessages.hpp"

class ProjectionThread {
public:
    explicit ProjectionThread(
        ProjectionContext &context,
        UnixSocket &socket
    );
    
    void start();
    void stop();
    
private:
    [[noreturn]]
    void mainLoop();
    
    [[noreturn]]
    void ioLoop();
    
    
    std::unique_ptr<projector::MessageHeader> &&nextHeader();
    
    template<typename T>
    std::unique_ptr<T> &&read(size_t size) {
        if (size == 0) {
            return nullptr;
        }
        
        auto promise = std::promise<std::unique_ptr<uint8_t>>();
        _readTasks.emplace(size, promise);
        
        auto pointer = promise.get_future().get();
        
        return std::unique_ptr<T> { static_cast<T>(pointer.release()) };
    }
    
    void writeMessage(const void *pointer, size_t size);

    ProjectionContext &_context;
    UnixSocket &_socket;
    
    bool _isTerminated;
    std::thread _projectorThread;
    std::thread _ioThread;
    
    std::queue<std::pair<size_t, std::unique_ptr<uint8_t>>> _writeTasks;
    std::queue<std::pair<size_t, std::promise<std::unique_ptr<uint8_t>> &>> _readTasks;
};

#endif //XRDP_PROJECTIONTHREAD_HPP
