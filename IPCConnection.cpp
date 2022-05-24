//
// Created by Gyuhwan Park on 2022/05/24.
//

#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

#include "IPCConnection.hpp"

IPCConnection::IPCConnection(std::string socketPath):
    _socket(socketPath),
    _isWorkerTerminated(false),
    
    _messageId(0),
    _ackId(0)
{

}

void IPCConnection::connect() {
    _socket.connect();
    
    _workerThread = std::thread(&IPCConnection::workerLoop, this);
}

void IPCConnection::disconnect() {
    _isWorkerTerminated = true;
    
    if (_workerThread.joinable()) {
        _workerThread.join();
    }
    _socket.close();
}

std::unique_ptr<ULIPCHeader, IPCConnection::MallocFreeDeleter> IPCConnection::nextHeader() {
    return std::move(read<ULIPCHeader>(sizeof(ULIPCHeader)));
}

void IPCConnection::write(const void *pointer, size_t size) {
    assert(pointer != nullptr);
    assert(size > 0);
    
    std::scoped_lock<std::mutex> scopedWriteTasksLock(_writeTasksLock);
    
    std::unique_ptr<uint8_t, MallocFreeDeleter> data(
        (uint8_t *) malloc(size),
        free
    );
    
    std::memcpy(data.get(), pointer, size);
    
    _writeTasks.emplace(size, std::move(data));
}

void IPCConnection::workerLoop() {
    const size_t MAX_READ_SIZE = 8192;
    
    size_t readBytes = 0;
    std::unique_ptr<uint8_t> _currentReadTask;
    
    while (!_isWorkerTerminated) {
        if (!_writeTasks.empty()) {
            std::scoped_lock<std::mutex> scopedWriteTasksLock(_writeTasksLock);
            auto writeTask = std::move(_writeTasks.front());
            _writeTasks.pop();
            
            if (_socket.write(writeTask.second.get(), writeTask.first) < 0) {
                throw std::runtime_error("could not perform write()");
            }
        }
        
        if (!_readTasks.empty()) {
            auto &readTask = _readTasks.front();
            
            auto &contentLength = readTask.first;
            auto &promise = readTask.second;
            
            if (_currentReadTask == nullptr) {
                readBytes = 0;
                _currentReadTask = std::unique_ptr<uint8_t>(
                    new uint8_t[readTask.first]
                );
            }
            
            int bytes = std::min(
                (size_t) MAX_READ_SIZE,
                contentLength - readBytes
            );
            
            size_t retval = _socket.read(_currentReadTask.get() + readBytes, bytes);
            if (retval < 0) {
                throw std::runtime_error("failed to perform read()");
            }
            
            readBytes += retval;
            
            if (readBytes >= contentLength) {
                promise.set_value(std::move(_currentReadTask));
                
                {
                    std::scoped_lock<std::mutex> scopedReadTasksLock(_readTasksLock);
                    _readTasks.pop();
                }
                
                _currentReadTask = nullptr;
                readBytes = 0;
            }
        }
    }
}
