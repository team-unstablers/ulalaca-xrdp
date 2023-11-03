//
// Created by Gyuhwan Park on 2022/05/24.
//

#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

#include <cassert>
#include <utility>

#include <fcntl.h>

extern "C" {
#include "parse.h"
#include "defines.h"
}

#include "IPCConnection.hpp"

namespace ulalaca::ipc {

    IPCConnection::IPCConnection(std::shared_ptr<UnixSocket> socket) :
            _socket(std::move(socket)),
            _isWorkerTerminated(false),

            _workerThread(),
            _messageId(0),
            _ackId(0),
            _isGood(true) {

    }

    IPCConnection::IPCConnection(const std::string &socketPath) :
            IPCConnection(std::make_shared<UnixSocket>(socketPath)) {

    }

    FD IPCConnection::descriptor() {
        return _socket->descriptor();
    }

    void IPCConnection::connect() {
        _socket->connect();

        // enable non-blocking io
        auto flags = _socket->fcntl(F_GETFL, 0);
        if (_socket->fcntl(F_SETFL, flags | O_NONBLOCK)) {
            throw SystemCallException(errno, "fcntl");
        }

        _workerThread = std::thread(&IPCConnection::workerLoop, this);
    }

    void IPCConnection::disconnect() {
        _isWorkerTerminated = true;

        if (_workerThread.joinable()) {
            _workerThread.join();
        }
        _socket->close();
    }

    bool IPCConnection::isGood() const {
        return _isGood;
    }

    IPCDataBlockPtr IPCConnection::readBlock(size_t size) {
        if (!size) {
            return nullptr;
        }

        auto readTask = std::make_shared<IPCReadTask>(IPCReadTask {
            size,
            std::make_shared<IPCReadPromise>(),

            0,
            nullptr
        });

        {
            std::unique_lock<std::shared_mutex> _lock(_readTasksMutex);
            _readTasks.emplace(readTask);
        }

        auto future = readTask->promise->get_future();
        auto retval = future.get();

        return std::move(retval);
    }

    void IPCConnection::writeBlock(const void *pointer, size_t size) {
        assert(pointer != nullptr);
        assert(size > 0);

        auto writeTask = std::make_shared<IPCWriteTask>(IPCWriteTask {
                size,
                std::shared_ptr<uint8_t>(new uint8_t[size])
        });


        memcpy(writeTask->data.get(), pointer, size);

        {
            _writeTasks.emplace(std::move(writeTask));
        }
    }

    std::shared_ptr<ULIPCHeader> IPCConnection::nextHeader() {
        auto header = std::move(this->readBlock<ULIPCHeader>(sizeof(ULIPCHeader)));

        _ackId = header->id;
        // TODO: check timestamp or std::max(_ackId, header->id)

        return std::move(header);
    }
}
