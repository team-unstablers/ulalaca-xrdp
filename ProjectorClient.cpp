//
// Created by Gyuhwan Park on 2022/05/06.
//

#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

#include <cstring>

extern "C" {
#include "arch.h"
#include "parse.h"
#include "os_calls.h"
#include "defines.h"
#include "guid.h"
#include "xrdp_client_info.h"
}

#include "ProjectorClient.hpp"
#include "KeycodeMap.hpp"

ProjectorClient::ProjectorClient(
    ProjectionTarget &target,
    const std::string &socketPath
):
    _target(target),
    _ipcConnection(socketPath),
    _isTerminated(false)
{

}

FD ProjectorClient::descriptor() {
    return _ipcConnection.descriptor();
}

void ProjectorClient::start() {
    _ipcConnection.connect();
    _projectorThread = std::thread(&ProjectorClient::mainLoop, this);
}

void ProjectorClient::stop() {
    _isTerminated = true;
    
    if (_projectorThread.joinable()) {
        _projectorThread.join();
    }
    
    _ipcConnection.disconnect();
}

void ProjectorClient::sendHello(const std::string &xrdpUlalacaVersion, const xrdp_client_info &clientInfo) {
    if (_isTerminated) {
        return;
    }

    if (!_ipcConnection.isGood()) {
        _target.ipcDisconnected();
        this->stop();
        return;
    }

    ULIPCProjectionHello message {};

    strncpy((char *) &message.xrdpUlalacaVersion, xrdpUlalacaVersion.c_str(), sizeof(message.xrdpUlalacaVersion));
    strncpy((char *) &message.clientAddress, clientInfo.client_ip, sizeof(message.clientAddress));
    strncpy((char *) &message.clientDescription, clientInfo.client_description, sizeof(message.clientDescription));
    strncpy((char *) &message.program, clientInfo.program, sizeof(message.program));

    message.clientOSMajor = clientInfo.client_os_major;
    message.clientOSMinor = clientInfo.client_os_minor;
    message.codec = 0; // FIXME
    message.flags = 0;

    _ipcConnection.writeMessage(TYPE_PROJECTION_HELLO, message);
}

void ProjectorClient::handleEvent(XrdpEvent &event) {
    if (_isTerminated) {
        return;
    }

    if (!_ipcConnection.isGood()) {
        _target.ipcDisconnected();
        this->stop();
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
        [[maybe_unused]] auto lockStatus = event.param1;
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
}

void ProjectorClient::setViewport(ULIPCRect rect) {
    _ipcConnection.writeMessage(TYPE_PROJECTION_SET_VIEWPORT, ULIPCProjectionSetViewport {
        0, (uint16_t) rect.width, (uint16_t) rect.height, 0
    });
}

void ProjectorClient::setOutputSuppression(bool isOutputSuppressed) {
    if (isOutputSuppressed) {
        _ipcConnection.writeMessage(TYPE_PROJECTION_STOP, ULIPCProjectionStop {
            0
        });
    } else {
        _ipcConnection.writeMessage(TYPE_PROJECTION_START, ULIPCProjectionStart {
            0
        });
    }
}

void ProjectorClient::mainLoop() {
    while (!_isTerminated && _ipcConnection.isGood()) {
        auto header = _ipcConnection.nextHeader();
        
        switch (header->messageType) {
            case TYPE_SCREEN_UPDATE_NOTIFY: {
                auto notification = _ipcConnection.read<ULIPCScreenUpdateNotify>(header->length);
    
                LOG(LOG_LEVEL_DEBUG, "mainLoop(): adding dirty rect");
                _target.addDirtyRect(notification->rect);
                continue;
            }
            case TYPE_SCREEN_UPDATE_COMMIT: {
                auto commit = _ipcConnection.read<ULIPCScreenUpdateCommit>(header->length);
                auto bitmap = _ipcConnection.read<uint8_t>(commit->bitmapLength);
    
                LOG(LOG_LEVEL_DEBUG, "mainLoop(): commiting update");
                _target.commitUpdate(
                    bitmap.get(),
                    commit->bitmapLength,
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