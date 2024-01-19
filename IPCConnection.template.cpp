//
// Created by Gyuhwan Park on 10/29/23.
//

#ifndef ULALACA_IPCCONNECTION_TEMPLATE_HPP
#define ULALACA_IPCCONNECTION_TEMPLATE_HPP

#include "IPCConnection.hpp"

namespace ulalaca::ipc {
    template <typename T>
    std::shared_ptr<T> IPCConnection::readBlock(size_t size) {
        auto dataBlock = this->readBlock(size);

        return std::reinterpret_pointer_cast<T>(dataBlock);
    }

    template <typename T>
    void IPCConnection::writeMessage(const ULIPCHeader &header, const T &message) {
        std::unique_lock<std::shared_mutex> lock(_writeTasksMutex);

        auto _header = header;
        _header.id = _messageId++;
        _header.length = sizeof(message);
        // _header.timestamp = now();

        writeBlock(&_header, sizeof(_header));
        writeBlock(&message, sizeof(message));
    }

    template<typename T>
    void IPCConnection::writeMessage(uint16_t messageType, const T &message) {
        ULIPCHeader header {
            messageType,
            0,
            0,
            0,
            sizeof(message)
        };
        writeMessage(header, message);
    }
}
#endif
