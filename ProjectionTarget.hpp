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

    virtual void ipcDisconnected() = 0;
};

#endif //ULALACA_PROJECTIONCONTEXT_HPP
