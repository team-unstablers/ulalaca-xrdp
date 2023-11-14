//
// Created by Gyuhwan Park on 2022/05/24.
//

#ifndef ULALACA_IPCCONNECTION_HPP
#define ULALACA_IPCCONNECTION_HPP

#include <cassert>

#include <memory>
#include <thread>
#include <queue>
#include <future>
#include <mutex>
#include <shared_mutex>

#include "UnixSocket.hpp"

#include "messages/projector.h"

#include "ulalaca.hpp"

namespace ulalaca::ipc {

    using IPCDataBlockPtr = std::shared_ptr<uint8_t>;
    using IPCReadPromise = std::promise<IPCDataBlockPtr>;

    struct IPCWriteTask {
        size_t size;
        IPCDataBlockPtr data;
    };

    struct IPCReadTask {
        size_t size;
        std::shared_ptr<IPCReadPromise> promise;

        size_t read;
        IPCDataBlockPtr buffer;
    };


    class IPCConnection {
    public:
        explicit IPCConnection(std::shared_ptr<UnixSocket> socket);
        explicit IPCConnection(const std::string &socketPath);

        IPCConnection(IPCConnection &) = delete;

        FD descriptor();

        /** @throws SystemCallException */
        void connect();
        /** @throws SystemCallException */
        void disconnect();

        bool isGood() const;

        std::shared_ptr<ULIPCHeader> nextHeader();

        IPCDataBlockPtr readBlock(size_t size);
        void writeBlock(const void *pointer, size_t size);

        template <typename T>
        std::shared_ptr<T> readBlock(size_t size);

        template <typename T>
        void writeMessage(const ULIPCHeader &header, const T &message);

        /** @deprecated use writeMessage(const ULIPCHeader &header, const T &message) instead */
        template <typename T>
        void writeMessage(uint16_t messageType, const T &message);

    private:
        void workerLoop();

        std::shared_ptr<UnixSocket> _socket;
        std::atomic_uint64_t _messageId;
        std::atomic_uint64_t _ackId;
        std::thread _workerThread;
        bool _isWorkerTerminated;
        bool _isGood;

        std::shared_mutex _writeTasksMutex;
        std::queue<std::shared_ptr<IPCWriteTask>> _writeTasks;

        std::shared_mutex _readTasksMutex;
        std::queue<std::shared_ptr<IPCReadTask>>  _readTasks;
    };
}

/** @deprecated use ulalaca::ipc::IPCConnection instead */
using IPCConnection = ulalaca::ipc::IPCConnection;

#include "IPCConnection.template.cpp"

#endif //XRDP_IPCCONNECTION_HPP
