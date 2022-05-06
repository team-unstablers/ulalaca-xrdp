//
// Created by Gyuhwan Park on 2022/05/06.
//

#include "ProjectionThread.hpp"

ProjectionThread::ProjectionThread(
    ProjectionContext &context,
    UnixSocket &socket
):
    _context(context),
    _socket(socket),
    _isTerminated(false),
{

}

void ProjectionThread::start() {
    _projectorThread = std::thread(&ProjectionThread::mainLoop, this);
    _ioThread = std::thread(&ProjectionThread::ioLoop, this);
}

void ProjectionThread::stop() {
    _isTerminated = true;
}

void ProjectionThread::mainLoop() {
    while (!_isTerminated) {
        auto header = nextHeader();
        
        switch (header->messageType) {
            case projector::IN_SCREEN_UPDATE_EVENT: {
                auto updateEvent = read<projector::ScreenUpdateEvent>(header->length);
    
                _context.addDirtyRect(updateEvent->rect);
            }
                break;
            case projector::IN_SCREEN_COMMIT_UPDATE: {
                auto commitUpdate = read<projector::ScreenCommitUpdate>(header->length);
                auto bitmap = read<uint8_t>(commitUpdate->bitmapLength);
    
                _context.commitUpdate(
                    bitmap.get(),
                    commitUpdate->screenRect.width,
                    commitUpdate->screenRect.height
                );
            }
                break;
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
            std::this_thread::yield();
        }
        
        if (!_writeTasks.empty()) {
            auto writeTask = std::move(_writeTasks.front());
            _writeTasks.pop();
            
            if (_socket.write(writeTask.second.get(), writeTask.first) < 0) {
                throw std::runtime_error("failed to perform write()");
            }
        }
        
        if (_readTasks.empty()) {
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
                contentLength -  readBytes
            );
            
            int retval = _socket.read(_currentReadTask.get() + readBytes, bytes);
            if (retval <= 0) {
                throw std::runtime_error("failed to perform read()");
            }
            
            readBytes += retval;
            
            if (readBytes >= contentLength) {
                promise.set_value(std::move(_currentReadTask));
                _readTasks.pop();
                
                _currentReadTask = nullptr;
                readBytes = 0;
            }
        }
    }
}

std::unique_ptr<projector::MessageHeader> &&ProjectionThread::nextHeader() {
    return read<projector::MessageHeader>(sizeof(projector::MessageHeader));
}

void ProjectionThread::writeMessage(const void *pointer, size_t size) {
    assert(pointer != nullptr);
    assert(size > 0);
    
    std::unique_ptr<uint8_t, std::function<void(uint8_t *)>> data(
        (uint8_t *) malloc(size),
        [](auto *ptr) { free((void *) ptr); }
    );

    std::memcpy(data.get(), pointer, size);
    
    _writeTasks.emplace(size, std::move(data));
}