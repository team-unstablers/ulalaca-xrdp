//
// Created by Gyuhwan Park on 2022/04/28.
//

#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

#include "ulalaca.hpp"
#include "SocketStream.hpp"

#include "ProjectionThread.hpp"

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
    mod_server_version_message(&lib_mod_server_version_message),
    
    si(nullptr),
    
    _sessionSize { 0, 0, 640, 480 },
    
    _bpp(0),
    _encodingsMask(0),
    _keyLayout(0),
    _delayMs(0)
{
}

int XrdpUlalaca::lib_mod_event(XrdpUlalaca *_this, int type, long arg1, long arg2, long arg3, long arg4) {
    LOG(LOG_LEVEL_DEBUG, "lib_mod_event() called: %d", type);
    
    XrdpEvent event {
        (XrdpEvent::Type) type,
        arg1, arg2, arg3, arg4
    };
    
    if (_this->_projectionThread != nullptr) {
        _this->_projectionThread->handleEvent(event);
    }
    
    return 0;
}

int XrdpUlalaca::lib_mod_start(XrdpUlalaca *_this, int width, int height, int bpp) {
    constexpr const unsigned int BACKGROUND_COLOR = 0xb97e51;

    _this->_screenLayouts.clear();
    _this->_screenLayouts.emplace_back(Rect {
        0, 0, (short) width, (short) height
    });
    _this->calculateSessionSize();


    _this->server_begin_update(_this);
    _this->server_set_fgcolor(_this, (int) BACKGROUND_COLOR);
    _this->server_fill_rect(_this, 0, 0, width, height);
    _this->server_end_update(_this);
    
    _this->_bpp = bpp;
    // _this->updateBpp(bpp);
    
    return 0;
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
            _this->getSessionSocketPath(_this->_username)
        );
        _this->_socket->connect();
    } catch (SystemCallException &e) {
        _this->serverMessage(e.what(), 0);
        return 1;
    }
    
    _this->_password.clear();
    
    _this->_projectionThread = std::make_unique<ProjectionThread>(
        *_this, *(_this->_socket)
    );
    _this->_projectionThread->start();
    _this->serverMessage("welcome to the fantasy zone, get ready!", 0);
    
    return 0;
}

int XrdpUlalaca::lib_mod_signal(XrdpUlalaca *_this) {
    LOG(LOG_LEVEL_INFO, "lib_mod_signal() called");
    return 0;
}

int XrdpUlalaca::lib_mod_end(XrdpUlalaca *_this) {
    LOG(LOG_LEVEL_INFO, "lib_mod_end() called");
    return 0;
}

int XrdpUlalaca::lib_mod_session_change(XrdpUlalaca *_this, int, int) {
    return 0;
}

int XrdpUlalaca::lib_mod_get_wait_objs(XrdpUlalaca *_this, tbus *read_objs, int *rcount, tbus *write_objs, int *wcount,
                                       int *timeout) {
    
    LOG(LOG_LEVEL_INFO, "lib_mod_get_wait_objs() called");
    return 0;
}

int XrdpUlalaca::lib_mod_check_wait_objs(XrdpUlalaca *_this) {
    LOG(LOG_LEVEL_INFO, "lib_mod_check_wait_objs() called");
    return 0;
}

int XrdpUlalaca::lib_mod_frame_ack(XrdpUlalaca *_this, int flags, int frame_id) {
    LOG(LOG_LEVEL_INFO, "lib_mod_frame_ack() called: %d", frame_id);
    _this->_ackFrameId = frame_id;
    
    return 0;
}

int XrdpUlalaca::lib_mod_suppress_output(XrdpUlalaca *_this, int suppress, int left, int top, int right, int bottom) {
    LOG(LOG_LEVEL_INFO, "lib_mod_suppress_output() called");
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

std::string XrdpUlalaca::getSessionSocketPath(std::string &username) {
    std::stringstream sstream;
    sstream << "/Users/" << username << "/.ulalaca_projector.sock";
    
    return sstream.str();
}

int XrdpUlalaca::decideCopyRectSize() const {
    bool isRFXCodec = _clientInfo.rfx_codec_id != 0;
    bool isJPEGCodec = _clientInfo.jpeg_codec_id != 0;
    bool isH264Codec = _clientInfo.h264_codec_id != 0;

    if (isRFXCodec || isJPEGCodec) {
        return 64;
    }

    if (isH264Codec) {
        // return 256;
        return RECT_SIZE_BYPASS_CREATE;
    }

    return RECT_SIZE_BYPASS_CREATE;
}

std::unique_ptr<std::vector<Rect>> XrdpUlalaca::createCopyRects(std::vector<Rect> &dirtyRects, int rectSize) const {
    auto blocks = std::make_unique<std::vector<Rect>>();

    if (rectSize == RECT_SIZE_BYPASS_CREATE) {
        std::copy(dirtyRects.begin(), dirtyRects.end(), std::back_insert_iterator(*blocks));
        return std::move(blocks);
    }

    for (auto &dirtyRect : dirtyRects) {
        if (_sessionSize.width <= dirtyRect.x ||
            _sessionSize.height <= dirtyRect.y) {
            continue;
        }

        auto width = std::min(dirtyRect.width, (short) (_sessionSize.width - dirtyRect.x));
        auto height = std::min(dirtyRect.height, (short) (_sessionSize.height - dirtyRect.y));

        auto baseX = dirtyRect.x - (dirtyRect.x % rectSize);
        auto baseY = dirtyRect.y - (dirtyRect.y % rectSize);

        auto blockCountX = ((width + dirtyRect.x % rectSize) + (rectSize - 1)) / rectSize;
        auto blockCountY = ((height + dirtyRect.y % rectSize) + (rectSize - 1)) / rectSize;

        for (int j = 0; j < blockCountY; j++) {
            for (int i = 0; i < blockCountX; i++) {
                short x = baseX + (rectSize * i);
                short y = baseY + (rectSize * j);

                blocks->push_back(Rect {x, y, (short) rectSize, (short) rectSize });
            }
        }
    }

    return std::move(blocks);
}

void XrdpUlalaca::addDirtyRect(Rect &rect) {
    _dirtyRects.push_back(rect);
}

void XrdpUlalaca::commitUpdate(const uint8_t *image, int32_t width, int32_t height) {
    LOG(LOG_LEVEL_TRACE, "painting: %d, %d", width, height);
    
    std::scoped_lock<std::mutex> scopedCommitLock(_commitUpdateLock);
    
    if ((_frameId - _ackFrameId) > 4) {
        _dirtyRects.clear();
        return;
    }
    
    if (_frameId > 0 && _dirtyRects.empty()) {
        return;
    }
    
    server_begin_update(this);
    
    Rect screenRect = {0, 0, (short) width, (short) height};
    auto copyRectSize = decideCopyRectSize();
    
    if (_frameId > 0) {
        auto copyRects = createCopyRects(_dirtyRects, copyRectSize);
        
        server_paint_rects(
            this,
            _dirtyRects.size(), reinterpret_cast<short *>(_dirtyRects.data()),
            copyRects->size(), reinterpret_cast<short *>(copyRects->data()),
            (char *) image,
            width, height,
            0, (_frameId++ % INT32_MAX)
        );
    } else {
        // paint entire screen
        auto dirtyRects = std::vector<Rect>{screenRect};
        auto copyRects = createCopyRects(dirtyRects, copyRectSize);
        
        server_paint_rects(
            this,
            dirtyRects.size(), reinterpret_cast<short *>(dirtyRects.data()),
            copyRects->size(), reinterpret_cast<short *>(copyRects->data()),
            (char *) image,
            width, height,
            0, (_frameId++ % INT32_MAX)
        );
    }
    
    _dirtyRects.clear();
    server_end_update(this);
}

void XrdpUlalaca::calculateSessionSize() {
    // TODO: calculate session size to support multiple display environments
    if (_screenLayouts.empty()) {
        _sessionSize = Rect {
            0, 0, 640, 480
        };
        return;
    }

    _sessionSize = _screenLayouts.front();
}


void XrdpUlalaca::serverMessage(const char *message, int code) {
    this->server_msg(this, message, code);
    LOG(LOG_LEVEL_INFO, "%s", message);
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
