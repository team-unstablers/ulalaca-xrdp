//
// Created by Gyuhwan Park on 2023/01/28.
//

#include "XrdpEvent.hpp"

bool XrdpEvent::isKeyEvent() const {
    return type == KEY_DOWN || type == KEY_UP;
}

bool XrdpEvent::isMouseEvent() const {
    return type >= MOUSE_MOVE && type <= MOUSE_UNKNOWN_2;
}
