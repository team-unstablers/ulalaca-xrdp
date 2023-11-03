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

    void IPCConnection::workerLoop() {
        const size_t MAX_RW_SIZE = 8192;

        const auto isWriteQueueEmpty = [this] {
            std::shared_lock<std::shared_mutex> _lock(_writeTasksMutex);
            return _writeTasks.empty();
        };

        const auto nextWriteTask = [this] {
            std::shared_lock<std::shared_mutex> _lock(_writeTasksMutex);
            auto writeTask = _writeTasks.front();
            return writeTask;
        };

        const auto popWriteTask = [this] {
            std::unique_lock<std::shared_mutex> _lock(_writeTasksMutex);
            _writeTasks.pop();
        };

        const auto isReadQueueEmpty = [this] {
            std::shared_lock<std::shared_mutex> _lock(_readTasksMutex);
            return _readTasks.empty();
        };

        const auto nextReadTask = [this] {
            std::shared_lock<std::shared_mutex> _lock(_readTasksMutex);
            auto readTask = _readTasks.front();
            return readTask;
        };

        const auto popReadTask = [this] {
            std::unique_lock<std::shared_mutex> _lock(_readTasksMutex);
            _readTasks.pop();
        };

        _isGood = true;

        while (!_isWorkerTerminated) {
            const pollfd pollFd = _socket->poll(POLLIN | POLLOUT, -1);
            const bool canRead = (pollFd.revents & POLLIN) > 0;
            const bool canWrite = (pollFd.revents & POLLOUT) > 0;

            do {
                if(!canWrite) break; /* do-while escape */
                if(isWriteQueueEmpty()) break; /* do-while escape */

                auto writeTask = nextWriteTask();
                ssize_t retval = _socket->write(writeTask->data.get(), writeTask->size);
                if (retval < 0) LOG(LOG_LEVEL_ERROR, "write() failed (errno=%d)", errno);
                else popWriteTask();
            } while(false);


            do {
                if (!canRead) break; /* do-while escape */
                if (isReadQueueEmpty()) break; /* do-while escape */

                auto readTask = nextReadTask();
                auto &size = readTask->size;
                auto &buffer = readTask->buffer;
                auto &read = readTask->read;
                auto &promise = readTask->promise;

                if (!buffer) {
                    read = 0;
                    buffer = std::shared_ptr<uint8_t>((uint8_t *) malloc(size), free);
                }

                size_t bytesToRead = std::min( (size_t) MAX_RW_SIZE, size - read);
                ssize_t retval = _socket->read(buffer.get() + read, bytesToRead);

                if (retval <= 0) {
                    if (errno != EAGAIN) {
                        LOG(LOG_LEVEL_ERROR, "read() failed (errno=%d)", errno);
                        _isGood = false;
                        break; /* do-while escape */
                    }
                }
                read += retval;
                if (read < size) break; /* do-while escape */

                popReadTask();
                promise->set_value(std::move(buffer));
                buffer = nullptr;
            } while(false);

            if (pollFd.revents & POLLHUP) {
                LOG(LOG_LEVEL_INFO, "POLLHUP bit set");
                _isGood = false;

                if (isReadQueueEmpty() || !canRead) {
                    LOG(LOG_LEVEL_INFO, "POLLHUP bit set; closing connection");
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
}