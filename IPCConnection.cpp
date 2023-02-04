//
// Created by Gyuhwan Park on 2022/05/24.
//

#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

#include <cassert>

#include <fcntl.h>
#include <poll.h>

extern "C" {
#include "parse.h"
#include "defines.h"
}

#include "IPCConnection.hpp"

IPCConnection::IPCConnection(std::string socketPath):
    _socket(socketPath),
    _isWorkerTerminated(false),
    
    _messageId(0),
    _ackId(0),
    _isGood(true)
{

}

FD IPCConnection::descriptor() {
    return _socket.descriptor();
}

void IPCConnection::connect() {
    _socket.connect();

    // enable non-blocking io
    auto flags = fcntl(_socket.descriptor(), F_GETFL, 0);
    if (fcntl(_socket.descriptor(), F_SETFL, flags | O_NONBLOCK)) {
        throw SystemCallException(errno, "fcntl");
    }


    _workerThread = std::thread(&IPCConnection::workerLoop, this);
}

void IPCConnection::disconnect() {
    _isWorkerTerminated = true;
    
    if (_workerThread.joinable()) {
        _workerThread.join();
    }
    _socket.close();
}

bool IPCConnection::isGood() const {
    return _isGood;
}

std::unique_ptr<ULIPCHeader, IPCConnection::MallocFreeDeleter> IPCConnection::nextHeader() {
    return std::move(read<ULIPCHeader>(sizeof(ULIPCHeader)));
}

void IPCConnection::write(const void *pointer, size_t size) {
    assert(pointer != nullptr);
    assert(size > 0);
    
    std::unique_ptr<uint8_t, MallocFreeDeleter> data(
        (uint8_t *) malloc(size),
        free
    );
    
    std::memcpy(data.get(), pointer, size);
    
    {
        std::scoped_lock<std::mutex> scopedWriteTasksLock(_writeTasksLock);
        _writeTasks.emplace(size, std::move(data));
    }
}

void IPCConnection::workerLoop() {
    const size_t MAX_READ_SIZE = 8192;

    size_t readPos = 0;
    std::unique_ptr<uint8_t> readBuffer;

    pollfd pollFd {
        _socket.descriptor(),
        POLLIN | POLLOUT,
        0
    };

    _isGood = true;

    while (!_isWorkerTerminated) {
        if (poll(&pollFd, 1, -1) < 0) {
            throw SystemCallException(errno, "poll");
        }

        bool canRead = (pollFd.revents & POLLIN) > 0;
        bool canWrite = (pollFd.revents & POLLOUT) > 0;

        if (canWrite && !_writeTasks.empty()) {
            _writeTasksLock.lock();
            auto writeTask = std::move(_writeTasks.front());
            _writeTasks.pop();
            _writeTasksLock.unlock();
            
            
            if (_socket.write(writeTask.second.get(), writeTask.first) < 0) {
                if (errno == EAGAIN) {
                    continue;
                }
                
                LOG(LOG_LEVEL_ERROR, "write() failed (errno=%d)", errno);
                continue;
            }
        }

        if (canRead && !_readTasks.empty()) {
            auto &readTask = _readTasks.front();

            auto &contentLength = readTask.first;
            auto &promise = readTask.second;

            if (readBuffer == nullptr) {
                readPos = 0;
                readBuffer = std::unique_ptr<uint8_t>(new uint8_t[contentLength]);
            }

            int readForBytes = std::min(
                (size_t) MAX_READ_SIZE,
                contentLength - readPos
            );

            size_t retval = _socket.read(readBuffer.get() + readPos, readForBytes);

            if (retval < 0) {
                if (errno == EAGAIN) {
                    continue;
                } else {
                    throw SystemCallException(errno, "read");
                }
            }

            if (_isGood && retval <= 0) {
                break;
            }

            readPos += retval;

            if (readPos >= contentLength) {
                promise->set_value(std::move(readBuffer));
                {
                    std::scoped_lock<std::mutex> scopedReadTasksLock(_readTasksLock);
                    _readTasks.pop();
                }

                readBuffer = nullptr;
                readPos = 0;
            }
        }
        
        if (pollFd.revents & POLLHUP) {
            LOG(LOG_LEVEL_WARNING, "POLLHUP bit set");
            _isGood = false;

            if (_readTasks.empty()) {
                LOG(LOG_LEVEL_WARNING, "POLLHUP bit set; closing connection");
                break;
            }
        }
    
        if (pollFd.revents & POLLERR) {
            LOG(LOG_LEVEL_ERROR, "POLLERR bit set; closing connection");
            break;
        }
    }

    _isGood = false;
}
