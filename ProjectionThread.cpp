//
// Created by Gyuhwan Park on 2022/05/06.
//

#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

extern "C" {
#include "arch.h"
#include "parse.h"
#include "os_calls.h"
#include "defines.h"
#include "guid.h"
#include "xrdp_client_info.h"
};

#include "ProjectionThread.hpp"
#include "KeycodeMap.hpp"

ProjectionThread::ProjectionThread(
    XrdpUlalaca &xrdpUlalaca,
    UnixSocket &socket
):
    _xrdpUlalaca(xrdpUlalaca),
    _socket(socket),
    _isTerminated(false)
{

}

void ProjectionThread::start() {
    _projectorThread = std::thread(&ProjectionThread::mainLoop, this);
    _ioThread = std::thread(&ProjectionThread::ioLoop, this);
}

void ProjectionThread::stop() {
    _isTerminated = true;
}

void ProjectionThread::handleEvent(XrdpEvent &event) {
    using namespace projector;
    
    if (event.isKeyEvent()) {
        auto keycode = event.param3;
        auto cgKeycode = rdpKeycodeToCGKeycode(keycode);
        auto eventType = event.type == XrdpEvent::KEY_DOWN ?
            KeyboardEvent::TYPE_KEYDOWN :
            KeyboardEvent::TYPE_KEYUP;
        
        if (cgKeycode == -1) {
            return;
        }
        
        writeMessage(MessageType::OUT_KEYBOARD_EVENT, KeyboardEvent {
            eventType, (uint32_t) cgKeycode, 0
        });
    } else if (event.type == XrdpEvent::KEY_SYNCHRONIZE_LOCK) {
        auto lockStatus = event.param1;
    }
    
    if (event.isMouseEvent()) {
        switch (event.type) {
            case XrdpEvent::MOUSE_MOVE: {
                uint16_t posX = event.param1;
                uint16_t posY = event.param2;
                
                writeMessage(MessageType::OUT_MOUSE_MOVE_EVENT, MouseMoveEvent {
                    posX, posY,
                    0
                });
                return;
            }
            
            case XrdpEvent::MOUSE_BUTTON_LEFT_DOWN: {
                writeMessage(MessageType::OUT_MOUSE_BUTTON_EVENT, MouseButtonEvent {
                    MouseButtonEvent::TYPE_MOUSEDOWN,
                    MouseButtonEvent::BUTTON_LEFT,
                    0
                });
                return;
            }
            case XrdpEvent::MOUSE_BUTTON_LEFT_UP: {
                writeMessage(MessageType::OUT_MOUSE_BUTTON_EVENT, MouseButtonEvent {
                    MouseButtonEvent::TYPE_MOUSEUP,
                    MouseButtonEvent::BUTTON_LEFT,
                    0
                });
                return;
            }
    
    
            case XrdpEvent::MOUSE_BUTTON_RIGHT_DOWN: {
                writeMessage(MessageType::OUT_MOUSE_BUTTON_EVENT, MouseButtonEvent {
                    MouseButtonEvent::TYPE_MOUSEDOWN,
                    MouseButtonEvent::BUTTON_RIGHT,
                    0
                });
                return;
            }
            case XrdpEvent::MOUSE_BUTTON_RIGHT_UP: {
                writeMessage(MessageType::OUT_MOUSE_BUTTON_EVENT, MouseButtonEvent {
                    MouseButtonEvent::TYPE_MOUSEUP,
                    MouseButtonEvent::BUTTON_RIGHT,
                    0
                });
                return;
            }
            
            case XrdpEvent::MOUSE_WHEEL_DOWN: {
                LOG(LOG_LEVEL_DEBUG, "TODO: WHEEL_DOWN (%d)", (int) event.param1);
                /*
                writeMessage(MessageType::OUT_MOUSE_WHEEL_EVENT, MouseWheelEvent {
                    MouseButtonEvent::,
                    MouseButtonEvent::BUTTON_RIGHT,
                    0
                });
                 */
                return;
            }
            
            default:
                break;
        }
    
    }
    
    
    if (event.type == XrdpEvent::INVALIDATE_REQUEST) {
        uint16_t x1 = HIWORD(event.param1);
        uint16_t y1 = LOWORD(event.param1);
        uint16_t x2 = HIWORD(event.param2);
        uint16_t y2 = LOWORD(event.param2);
        
        // TODO: redraw(rect)?
    }
    
    if (event.type == XrdpEvent::CHANNEL_EVENT) {
        uint16_t channelId = LOWORD(event.param1);
        uint16_t flags = HIWORD(event.param1);
        auto size = (int) event.param2;
        auto data = (char *) event.param3;
        auto total_size = (int) event.param4;
    }
}

void ProjectionThread::mainLoop() {
    while (!_isTerminated) {
        auto header = nextHeader();
        
        switch (header->messageType) {
            case projector::IN_SCREEN_UPDATE_EVENT: {
                auto updateEvent = read<projector::ScreenUpdateEvent>(header->length);
    
                LOG(LOG_LEVEL_DEBUG, "mainLoop(): adding dirty rect");
                _xrdpUlalaca.addDirtyRect(updateEvent->rect);
                continue;
            }
            case projector::IN_SCREEN_COMMIT_UPDATE: {
                auto commitUpdate = read<projector::ScreenCommitUpdate>(header->length);
                auto bitmap = read<uint8_t>(commitUpdate->bitmapLength);
    
                LOG(LOG_LEVEL_DEBUG, "mainLoop(): commiting update");
                _xrdpUlalaca.commitUpdate(
                    bitmap.get(),
                    commitUpdate->screenRect.width,
                    commitUpdate->screenRect.height
                );
                continue;
            }
            default: {
                // ignore
                read<uint8_t>(header->length);
            }
        }
    }
}

void ProjectionThread::ioLoop() {
    const size_t MAX_READ_SIZE = 8192;
    
    size_t readBytes = 0;
    std::unique_ptr<uint8_t> _currentReadTask;
    
    while (!_isTerminated) {
        if (_writeTasks.empty() && _readTasks.empty()) {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1ms);
        }
        
        if (!_writeTasks.empty()) {
            std::scoped_lock<std::mutex> scopedWriteTasksLock(_writeTasksLock);
            auto writeTask = std::move(_writeTasks.front());
            _writeTasks.pop();
            
            if (_socket.write(writeTask.second.get(), writeTask.first) < 0) {
                throw std::runtime_error("failed to perform write()");
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

std::unique_ptr<projector::MessageHeader, MallocFreeDeleter> ProjectionThread::nextHeader() {
    return std::move(read<projector::MessageHeader>(sizeof(projector::MessageHeader)));
}

void ProjectionThread::write(const void *pointer, size_t size) {
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