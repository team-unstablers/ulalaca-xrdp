//
// Created by Gyuhwan Park on 2022/05/06.
//

#ifndef ULALACA_PROJECTIONCONTEXT_HPP
#define ULALACA_PROJECTIONCONTEXT_HPP

#include <cstdint>

#include <utility>

#include "UlalacaMessages.hpp"

class ProjectionContext {
public:
    virtual void addDirtyRect(Rect &rect) = 0;
    
    virtual void commitUpdate(
        const uint8_t *image,
        int32_t width, int32_t height
    ) = 0;
};

#endif //ULALACA_PROJECTIONCONTEXT_HPP
