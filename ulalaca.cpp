//
// Created by Gyuhwan Park on 2022/04/28.
//

#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

#include <algorithm>
#include <iostream>
#include <string>
#include <functional>
#include <chrono>

#include "ulalaca.hpp"
#include "SocketStream.hpp"

bool XrdpEvent::isKeyEvent() const {
    return type == KEY_DOWN || type == KEY_UP;
}

bool XrdpEvent::isMouseEvent() const {
    return type >= MOUSE_MOVE && type <= MOUSE_UNKNOWN_2;
}

XrdpUlalaca::XrdpUlalaca():
    size(sizeof(XrdpUlalaca)),
    version(ULALACA_VERSION),
    
    mod_start(&lib_mod_start),
    mod_connect(&lib_mod_connect),
    mod_event(&lib_mod_event),
    mod_signal(&lib_mod_signal),
    mod_end(&lib_mod_end),
    mod_set_param(&lib_mod_set_param),
    mod_session_change(&lib_mod_session_change),
    mod_get_wait_objs(&lib_mod_get_wait_objs),
    mod_check_wait_objs(&lib_mod_check_wait_objs),
    mod_frame_ack(&lib_mod_frame_ack),
    mod_suppress_output(&lib_mod_suppress_output),
    mod_server_monitor_resize(&lib_mod_server_monitor_resize),
    mod_server_monitor_full_invalidate(&lib_mod_server_monitor_full_invalidate),
    mod_server_version_message(&lib_mod_server_version_message)

{
}

int XrdpUlalaca::handleEvent(XrdpEvent &event) {
    if (event.isKeyEvent()) {
        auto keyCode = event.param2;
        return NO_ERROR;
    } else if (event.type == XrdpEvent::KEY_SYNCHRONIZE_LOCK) {
        auto lockStatus = event.param1;
        return NO_ERROR;
    }
    
    if (event.isMouseEvent()) {
        uint16_t posX = event.param1;
        uint16_t posY = event.param2;
        
        
        return NO_ERROR;
    }
    
    
    if (event.type == XrdpEvent::INVALIDATE_REQUEST) {
        uint16_t x1 = HIWORD(event.param1);
        uint16_t y1 = LOWORD(event.param1);
        uint16_t x2 = HIWORD(event.param2);
        uint16_t y2 = LOWORD(event.param2);
    
        // TODO: redraw(rect)?
        return NO_ERROR;
    }
    
    if (event.type == XrdpEvent::CHANNEL_EVENT) {
        uint16_t channelId = LOWORD(event.param1);
        uint16_t flags = HIWORD(event.param1);
        auto size = (int) event.param2;
        auto data = (char *) event.param3;
        auto total_size = (int) event.param4;
        
        return NO_ERROR;
    }
    
    return NO_ERROR;
}

int XrdpUlalaca::updateScreen() {
}

int XrdpUlalaca::lib_mod_event(XrdpUlalaca *_this, int type, long arg1, long arg2, long arg3, long arg4) {
    LOG(LOG_LEVEL_DEBUG, "lib_mod_event() called");
    
    XrdpEvent event {
        (XrdpEvent::Type) type,
        arg1, arg2, arg3, arg4
    };
    
    return _this->handleEvent(event);
}

int XrdpUlalaca::lib_mod_start(XrdpUlalaca *_this, int width, int height, int bpp) {
    constexpr const unsigned int BACKGROUND_COLOR = 0xb97e51;
    
    _this->server_begin_update(_this);
    _this->server_set_fgcolor(_this, (int) BACKGROUND_COLOR);
    _this->server_fill_rect(_this, 0, 0, width, height);
    _this->server_end_update(_this);
    
    _this->_bpp = bpp;
    // _this->updateBpp(bpp);
}

int XrdpUlalaca::lib_mod_set_param(XrdpUlalaca *_this, const char *_name, const char *_value) {
    std::string name(_name);
    std::string value(_value);
    
    if (name == "username") {
        _this->_username = value;
    } else if (name == "password") {
        _this->_password = value;
    } else if (name == "ip") {
        _this->_ip = value;
    } else if (name == "port") {
        _this->_port = value;
    } else if (name == "keylayout") {
        _this->_keyLayout = std::stoi(value);
    } else if (name == "delay_ms") {
        _this->_delayMs = std::stoi(value);
    } else if (name == "guid") {
        auto *_guid = reinterpret_cast<const guid *>(_value);
        _this->_guid = *_guid;
    } else if (name == "disabled_encodings_mask") {
        _this->_encodingsMask = ~std::stoi(value);
    } else if (name == "client_info") {
        auto *clientInfo = reinterpret_cast<const xrdp_client_info *>(_value);
        _this->_clientInfo = *clientInfo;
    }
    
    return 0;
}

int XrdpUlalaca::lib_mod_connect(XrdpUlalaca *_this) {
    _this->serverMessage("establishing connection to SessionProjector", 0);

    
    try {
        _this->_socket = std::make_unique<UnixSocket>(
            "/Users/unstabler/ulalaca-projector.socket"
        );
        _this->_socket->connect();
    } catch (SystemCallException &e) {
        _this->serverMessage(e.what(), 0);
        return 1;
    }
    
    _this->_screenUpdateThread = std::thread(&XrdpUlalaca::_screenUpdateLoop, _this);
    _this->serverMessage("welcome to the fantasy zone, get ready!", 0);
    
    return 0;
}

/**
 * TODO: 코드 정리
 * TODO: 다른 IPC 방법 찾기 (mkfifo, XPC, shm ...)
 * macOS에서 Unix Socket은 뭔 짓을 해도 8192 byte로 fragment 되기 때문에 퍼포먼스에 지장이 있진 않을까?
 */
void XrdpUlalaca::_screenUpdateLoop() {
    constexpr const int BUFFER_SIZE = 8192;
    constexpr const int IMAGE_BUFFER_SIZE = 1024 * 1024 * 4; // 더 크면 어떻게 할려구?
    
    uint8_t *buffer = new uint8_t[BUFFER_SIZE];
    ScreenUpdateMessage message; // FIXME
    uint8_t *imageBuffer = (uint8_t *) malloc(IMAGE_BUFFER_SIZE);
    
    std::vector<Rect> rects;
    rects.reserve(64);

    int pos = 0;
    
    bool isReceivingFrame = false;
    
    while (_error == 0) {
        if (!isReceivingFrame) {
            int read = _socket->read(buffer, sizeof(ScreenUpdateMessage));
    
            if (read == 0) {
                break;
            }
            
            std::memcpy(&message, buffer, sizeof(ScreenUpdateMessage));
    
            if (message.type == 0) {
                imageBuffer = new uint8_t[message.contentLength];
                pos = 0;
                isReceivingFrame = true;
            } else if (message.type == 1) {
                rects.push_back({ (short) message.x, (short) message.y, (short) message.width, (short) message.height });
            } else if (message.type == 2) {
                server_begin_update(this);
            } else if (message.type == 3) {
                server_end_update(this);
            }
        } else {
            int bytes = std::min(8192, (int) message.contentLength - pos);
            LOG(LOG_LEVEL_TRACE, "reading %d bytes (%d / %d)", bytes, pos, message.contentLength);
            
            int read = _socket->read(buffer, bytes);
            
            if (read == 0) {
                break;
            }
            
            std::memcpy(&imageBuffer[pos], buffer, read);
            pos += read;
            
            if (pos >= message.contentLength) {
                LOG(LOG_LEVEL_TRACE, "painting: %d, (%d, %d) -> (%d, %d)", rects.size(), message.x, message.y, message.width, message.height);
                if (rects.size() > 0) {
                    server_paint_rects(
                        this,
                        rects.size() * 4, reinterpret_cast<short *>(rects.data()),
                        rects.size() * 4, reinterpret_cast<short *>(rects.data()),
                        (char *) imageBuffer,
                        message.width, message.height,
                        0, _frameId
                    );
                } else {
                    // paint entire screen
                    server_paint_rect(this, 0, 0, message.width, message.height, (char *) imageBuffer, message.width, message.height, 0, 0);
                }
    
                /*
                for (auto &rect : rects2) {
                    server_paint_rect(
                        this,
                        rect.x, rect.y,
                        rect.width, rect.height,
                        (char *) imageBuffer,
                        rect.width, rect.height,
                        rect.x, rect.y
                    );
                }
                 */
                
                rects.clear();
                memset(imageBuffer, 0, message.contentLength);
    
                isReceivingFrame = false;
            }
        }
    }
    free(imageBuffer);
    delete[] buffer;
}

int XrdpUlalaca::lib_mod_signal(XrdpUlalaca *_this) {
    return 0;
}

int XrdpUlalaca::lib_mod_end(XrdpUlalaca *_this) {
    return 0;
}

int XrdpUlalaca::lib_mod_session_change(XrdpUlalaca *_this, int, int) {
    return 0;
}

int XrdpUlalaca::lib_mod_get_wait_objs(XrdpUlalaca *_this, tbus *read_objs, int *rcount, tbus *write_objs, int *wcount,
                                       int *timeout) {
    return 0;
}

int XrdpUlalaca::lib_mod_check_wait_objs(XrdpUlalaca *_this) {
    return 0;
}

int XrdpUlalaca::lib_mod_frame_ack(XrdpUlalaca *_this, int flags, int frame_id) {
    _this->_frameId++;
    return 0;
}

int XrdpUlalaca::lib_mod_suppress_output(XrdpUlalaca *_this, int suppress, int left, int top, int right, int bottom) {
    return 0;
}

int XrdpUlalaca::lib_mod_server_monitor_resize(XrdpUlalaca *_this, int width, int height) {
    return 0;
}

int XrdpUlalaca::lib_mod_server_monitor_full_invalidate(XrdpUlalaca *_this, int width, int height) {
    return 0;
}

int XrdpUlalaca::lib_mod_server_version_message(XrdpUlalaca *_this) {
    return 0;
}

void XrdpUlalaca::serverMessage(const char *message, int code) {
    this->server_msg(this, message, code);
    LOG(LOG_LEVEL_INFO, message);
}


tintptr EXPORT_CC mod_init(void) {
    auto *ulalaca = (XrdpUlalaca *) g_malloc(sizeof(XrdpUlalaca), 1);
    new (ulalaca) XrdpUlalaca();
    
    return (tintptr) ulalaca;
}

int EXPORT_CC mod_exit(tintptr handle) {
    auto *ulalaca = (XrdpUlalaca *) handle;
    LOG(LOG_LEVEL_TRACE, "Ulalaca: mod_exit()");
    
    if (ulalaca == nullptr) {
        LOG(LOG_LEVEL_WARNING, "Ulalaca: mod_exit(): handle is nullptr");
        return 0;
    }
    
    // trans_delete(ulalaca->trans);
    
    // call destructor manually
    ulalaca->~XrdpUlalaca();
    g_free(ulalaca);
    
    // TODO: cleanup
    
    return 0;
}
