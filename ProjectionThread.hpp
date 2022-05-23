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

#include "UlalacaMessages.hpp"
#include "ulalaca.hpp"

using MallocFreeDeleter = std::function<void(void *)>;

class ProjectionThread {
public:
    explicit ProjectionThread(
        XrdpUlalaca &xrdpUlalaca,
        UnixSocket &socket
    );
    
    void start();
    void stop();
    
    void handleEvent(XrdpEvent &event);
private:
    void mainLoop();
    void ioLoop();
    
    std::unique_ptr<projector::MessageHeader, MallocFreeDeleter> nextHeader();
    
    template<typename T>
    std::unique_ptr<T, MallocFreeDeleter> read(size_t size) {
        assert(size != 0);
       
        auto promise = std::promise<std::unique_ptr<uint8_t>>();
        {
            std::scoped_lock<std::mutex> scopedReadTasksLock(_readTasksLock);
            _readTasks.emplace(size, promise);
        }
        auto pointer = promise.get_future().get();
    
        return std::move(std::unique_ptr<T, MallocFreeDeleter>(
            reinterpret_cast<T *>(pointer.release()),
            free
        ));
    }
    
    void write(const void *pointer, size_t size);
    
    template <typename T>
    void writeMessage(projector::MessageType messageType, T message) {
        auto header = projector::MessageHeader {
            (uint16_t) messageType,
            0, 0,
            0,
            sizeof(T)
        };
        
        write(&header, sizeof(header));
        write(&message, sizeof(T));
    }
    
    
    XrdpUlalaca &_xrdpUlalaca;
    UnixSocket &_socket;
    
    bool _isTerminated;
    std::thread _projectorThread;
    std::thread _ioThread;
    
    
    std::mutex _writeTasksLock;
    std::mutex _readTasksLock;
    
    std::queue<std::pair<size_t, std::unique_ptr<uint8_t, MallocFreeDeleter>>> _writeTasks;
    std::queue<std::pair<size_t, std::promise<std::unique_ptr<uint8_t>> &>> _readTasks;
};

#endif //ULALACA_PROJECTIONTHREAD_HPP
