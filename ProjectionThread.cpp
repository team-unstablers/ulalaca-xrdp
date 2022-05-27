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
}

#include "ProjectionThread.hpp"
#include "KeycodeMap.hpp"

ProjectionThread::ProjectionThread(
    XrdpUlalaca &xrdpUlalaca,
    const std::string &socketPath
):
    _xrdpUlalaca(xrdpUlalaca),
    _ipcConnection(socketPath),
    _isTerminated(false)
{

}

void ProjectionThread::start() {
    _ipcConnection.connect();
    _projectorThread = std::thread(&ProjectionThread::mainLoop, this);
}

void ProjectionThread::stop() {
    _isTerminated = true;
    
    if (_projectorThread.joinable()) {
        _projectorThread.join();
    }
    
    _ipcConnection.disconnect();
}

void ProjectionThread::handleEvent(XrdpEvent &event) {
    if (_isTerminated) {
        return;
    }
    
    if (event.isKeyEvent()) {
        auto keycode = event.param3;
        auto cgKeycode = rdpKeycodeToCGKeycode(keycode);
        auto eventType = event.type == XrdpEvent::KEY_DOWN ?
            KEYBOARD_EVENT_TYPE_KEYDOWN :
            KEYBOARD_EVENT_TYPE_KEYUP;
        
        if (cgKeycode == -1) {
            return;
        }
        
        _ipcConnection.writeMessage(TYPE_EVENT_KEYBOARD, ULIPCKeyboardEvent {
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
                
                _ipcConnection.writeMessage(TYPE_EVENT_MOUSE_MOVE, ULIPCMouseMoveEvent {
                    posX, posY,
                    0
                });
                return;
            }
            
            case XrdpEvent::MOUSE_BUTTON_LEFT_DOWN: {
                _ipcConnection.writeMessage(TYPE_EVENT_MOUSE_BUTTON, ULIPCMouseButtonEvent {
                    MOUSE_EVENT_TYPE_DOWN,
                    MOUSE_EVENT_BUTTON_LEFT,
                    0
                });
                return;
            }
            case XrdpEvent::MOUSE_BUTTON_LEFT_UP: {
                _ipcConnection.writeMessage(TYPE_EVENT_MOUSE_BUTTON, ULIPCMouseButtonEvent {
                    MOUSE_EVENT_TYPE_UP,
                    MOUSE_EVENT_BUTTON_LEFT,
                    0
                });
                return;
            }
    
    
            case XrdpEvent::MOUSE_BUTTON_RIGHT_DOWN: {
                _ipcConnection.writeMessage(TYPE_EVENT_MOUSE_BUTTON, ULIPCMouseButtonEvent {
                    MOUSE_EVENT_TYPE_DOWN,
                    MOUSE_EVENT_BUTTON_RIGHT,
                    0
                });
                return;
            }
            case XrdpEvent::MOUSE_BUTTON_RIGHT_UP: {
                _ipcConnection.writeMessage(TYPE_EVENT_MOUSE_BUTTON, ULIPCMouseButtonEvent {
                    MOUSE_EVENT_TYPE_UP,
                    MOUSE_EVENT_BUTTON_RIGHT,
                    0
                });
                return;
            }
    
            case XrdpEvent::MOUSE_WHEEL_UP: {
                _ipcConnection.writeMessage(TYPE_EVENT_MOUSE_WHEEL, ULIPCMouseWheelEvent {
                    0, -56, 0
                });
                return;
            }
            case XrdpEvent::MOUSE_WHEEL_DOWN: {
                _ipcConnection.writeMessage(TYPE_EVENT_MOUSE_WHEEL, ULIPCMouseWheelEvent {
                    0, 56, 0
                });
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
        auto header = _ipcConnection.nextHeader();
        
        switch (header->messageType) {
            case TYPE_SCREEN_UPDATE_NOTIFY: {
                auto notification = _ipcConnection.read<ULIPCScreenUpdateNotify>(header->length);
    
                LOG(LOG_LEVEL_DEBUG, "mainLoop(): adding dirty rect");
                _xrdpUlalaca.addDirtyRect(notification->rect);
                continue;
            }
            case TYPE_SCREEN_UPDATE_COMMIT: {
                auto commit = _ipcConnection.read<ULIPCScreenUpdateCommit>(header->length);
                auto bitmap = _ipcConnection.read<uint8_t>(commit->bitmapLength);
    
                LOG(LOG_LEVEL_DEBUG, "mainLoop(): commiting update");
                _xrdpUlalaca.commitUpdate(
                    bitmap.get(),
                    commit->screenRect.width,
                    commit->screenRect.height
                );
                continue;
            }
            default: {
                // ignore
                _ipcConnection.read<uint8_t>(header->length);
            }
        }
    }
}