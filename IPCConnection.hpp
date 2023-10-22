//
// Created by Gyuhwan Park on 2022/05/24.
//

#ifndef ULALACA_IPCCONNECTION_HPP
#define ULALACA_IPCCONNECTION_HPP

#include <memory>
#include <thread>
#include <queue>
#include <future>
#include <cassert>

#include "UnixSocket.hpp"

#include "messages/projector.h"

#include "ulalaca.hpp"


class IPCConnection {
public:
    using MallocFreeDeleter = std::function<void(void *)>;

    explicit IPCConnection(std::shared_ptr<UnixSocket> socket);
    explicit IPCConnection(const std::string &socketPath);
    IPCConnection(IPCConnection &) = delete;

    FD descriptor();

    /**
     * @throws SystemCallException
     */
    void connect();
    void disconnect();

    bool isGood() const;

    std::unique_ptr<ULIPCHeader, MallocFreeDeleter> nextHeader();

    template <typename T>
    void writeMessage(uint16_t messageType, T message) {
        auto header = ULIPCHeader {
            (uint16_t) messageType,
            _messageId,
            0, // FIXME
            0, // FIXME
            sizeof(T)
        };

        write(&header, sizeof(header));
        write(&message, sizeof(T));
    }

    template<typename T>
    std::unique_ptr<T, MallocFreeDeleter> read(size_t size) {
        if (size == 0) {
            return nullptr;
        }

        auto promise = std::make_shared<std::promise<std::unique_ptr<uint8_t>>>();
        {
            std::scoped_lock<std::mutex> scopedReadTasksLock(_readTasksLock);
            _readTasks.emplace(size, promise);
        }
        auto source = promise->get_future().get();
        auto destination = std::unique_ptr<T, MallocFreeDeleter>(
            (T *) malloc(size),
            free
        );

        std::memmove(destination.get(), source.get(), size);

        return std::move(destination);
    }

    void write(const void *pointer, size_t size);

private:
    void workerLoop();

    std::atomic_uint64_t _messageId;
    std::atomic_uint64_t _ackId;

    std::shared_ptr<UnixSocket> _socket;
    std::thread _workerThread;
    bool _isWorkerTerminated;

    bool _isGood;

    std::mutex _writeTasksLock;
    std::mutex _readTasksLock;

    std::queue<std::pair<size_t, std::unique_ptr<uint8_t, MallocFreeDeleter>>> _writeTasks;
    std::queue<std::pair<size_t, std::shared_ptr<std::promise<std::unique_ptr<uint8_t>>> >> _readTasks;
};


#endif //XRDP_IPCCONNECTION_HPP
