//
// Created by Gyuhwan Park on 2022/05/06.
//

#ifndef ULALACA_PROJECTIONCONTEXT_HPP
#define ULALACA_PROJECTIONCONTEXT_HPP

#include <cstdint>
#include <utility>

#include "messages/projector.h"

class ProjectionTarget {
public:
    virtual void addDirtyRect(ULIPCRect &rect) = 0;
    
    virtual void commitUpdate(
        const uint8_t *image,
        size_t size,
        int32_t width, int32_t height
    ) = 0;

    virtual void streamWillStart(uint8_t type, uint64_t resourceId) = 0;
    virtual void streamDataReceived(uint8_t type, uint64_t resourceId, const uint8_t *data, size_t size) = 0;
    virtual void streamWillEnd(uint8_t type, uint64_t resourceId) = 0;

    virtual void ipcConnected() = 0;
    virtual void ipcDisconnected() = 0;
    virtual void ipcError(int reason) = 0;
};

#endif //ULALACA_PROJECTIONCONTEXT_HPP
