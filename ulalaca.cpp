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

#include "messages/broker.h"
#include "ulalaca.hpp"

#include "XrdpUlalacaPrivate.hpp"
#include "SocketStream.hpp"

#include "ProjectionThread.hpp"



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

    _impl(std::make_unique<XrdpUlalacaPrivate>(this))
{
}

int XrdpUlalaca::lib_mod_event(XrdpUlalaca *_this, int type, long arg1, long arg2, long arg3, long arg4) {
    return _this->_impl->libModEvent(type, arg1, arg2, arg3, arg4);
}

int XrdpUlalaca::lib_mod_start(XrdpUlalaca *_this, int width, int height, int bpp) {
    return _this->_impl->libModStart(width, height, bpp);
}

int XrdpUlalaca::lib_mod_set_param(XrdpUlalaca *_this, const char *name, const char *value) {
    return _this->_impl->libModSetParam(name, value);
}

int XrdpUlalaca::lib_mod_connect(XrdpUlalaca *_this) {
    return _this->_impl->libModConnect();
}

int XrdpUlalaca::lib_mod_signal(XrdpUlalaca *_this) {
    return _this->_impl->libModSignal();
}

int XrdpUlalaca::lib_mod_end(XrdpUlalaca *_this) {
    return _this->_impl->libModEnd();
}

int XrdpUlalaca::lib_mod_session_change(XrdpUlalaca *_this, int arg1, int arg2) {
    return _this->_impl->libModSessionChange(arg1, arg2);
}

int XrdpUlalaca::lib_mod_get_wait_objs(XrdpUlalaca *_this, tbus *read_objs, int *rcount, tbus *write_objs, int *wcount,
                                       int *timeout) {
    return _this->_impl->libModGetWaitObjs(read_objs, rcount, write_objs, wcount, timeout);
}

int XrdpUlalaca::lib_mod_check_wait_objs(XrdpUlalaca *_this) {
    return _this->_impl->libModCheckWaitObjs();
}

int XrdpUlalaca::lib_mod_frame_ack(XrdpUlalaca *_this, int flags, int frame_id) {
    return _this->_impl->libModFrameAck(flags, frame_id);
}

int XrdpUlalaca::lib_mod_suppress_output(XrdpUlalaca *_this, int suppress, int left, int top, int right, int bottom) {
    return _this->_impl->libModSuppressOutput(suppress, left, top, right, bottom);
}

int XrdpUlalaca::lib_mod_server_monitor_resize(XrdpUlalaca *_this, int width, int height) {
    return _this->_impl->libModServerMonitorResize(width, height);
}

int XrdpUlalaca::lib_mod_server_monitor_full_invalidate(XrdpUlalaca *_this, int width, int height) {
    return _this->_impl->libModServerMonitorFullInvalidate(width, height);
}

int XrdpUlalaca::lib_mod_server_version_message(XrdpUlalaca *_this) {
    return _this->_impl->libModServerVersionMessage();
}

std::string XrdpUlalaca::getSessionSocketPathUsingCredential(
    const std::string &username,
    const std::string &password
) {
    std::string socketPath;
    
    IPCConnection connection("/var/run/ulalaca_broker.sock");
    connection.connect();
    {
        using namespace std::chrono_literals;
        
        ULIPCSessionRequest request {};
        std::strncpy(&request.username[0], username.c_str(), sizeof(request.username));
        std::strncpy(&request.password[0], password.c_str(), sizeof(request.password));
        
        connection.writeMessage(TYPE_SESSION_REQUEST, request);
        std::memset(&request.password, 0x00, sizeof(request.password));
        
        auto responseHeader = connection.nextHeader();
        
        switch (responseHeader->messageType) {
            case TYPE_SESSION_REQUEST_RESOLVED: {
                auto response = connection.read<ULIPCSessionRequestResolved>(
                    sizeof(ULIPCSessionRequestResolved)
                );
                
                socketPath = std::string(response->path);
            }
            break;
            
            case TYPE_SESSION_REQUEST_REJECTED:
            default: {
                this->serverMessage("invalid credential", 0);
            }
            break;
            
        }
    }
    connection.disconnect();
    
    
    return socketPath;
}

bool XrdpUlalaca::isRFXCodec() const {
    return _clientInfo.rfx_codec_id != 0;
}

bool XrdpUlalaca::isJPEGCodec() const {
    return _clientInfo.jpeg_codec_id != 0;
}

bool XrdpUlalaca::isH264Codec() const {
    return _clientInfo.h264_codec_id != 0;
}

bool XrdpUlalaca::isGFXH264Codec() const {
    return _clientInfo.capture_code & 3;
}

bool XrdpUlalaca::isRawBitmap() const {
    return !(isRFXCodec() || isJPEGCodec() || isH264Codec() || isGFXH264Codec());
}

int XrdpUlalaca::decideCopyRectSize() const {
    if (isRFXCodec()) {
        return 64;
    }
    
    if (isH264Codec() || isGFXH264Codec()) {
        return RECT_SIZE_BYPASS_CREATE;
    }

    return RECT_SIZE_BYPASS_CREATE;
}

std::unique_ptr<std::vector<XrdpUlalaca::Rect>> XrdpUlalaca::createCopyRects(
    std::vector<Rect> &dirtyRects,
    int rectSize
) const {
    auto blocks = std::make_unique<std::vector<Rect>>();
    blocks->reserve(128);

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
    std::scoped_lock<std::mutex> scopedCommitLock(_commitUpdateLock);
    _dirtyRects.push_back(rect);
}

void XrdpUlalaca::commitUpdate(const uint8_t *image, int32_t width, int32_t height) {
    LOG(LOG_LEVEL_DEBUG, "updating screen: %d, %d", width, height);

    if (!_commitUpdateLock.try_lock()) {
        _dirtyRects.clear();
        return;
    }

    if (_sessionSize.width != width || _sessionSize.height != height) {
        // server_reset(this, width, height, _bpp);
    }
    
    if (_frameId > 0 && _dirtyRects.empty()) {
        return;
    }
    
    server_begin_update(this);

    Rect screenRect = {0, 0, (short) width, (short) height};
    auto copyRectSize = decideCopyRectSize();
    
    if (_frameId > 0 || !_fullInvalidate) {
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
        
        if (isRawBitmap()) {
            server_paint_rect(
                this,
                screenRect.x, screenRect.y,
                screenRect.width, screenRect.height,
                (char *) image,
                screenRect.width, screenRect.height,
                0, 0
            );
        } else {
            server_paint_rects(
                this,
                dirtyRects.size(), reinterpret_cast<short *>(dirtyRects.data()),
                copyRects->size(), reinterpret_cast<short *>(copyRects->data()),
                (char *) image,
                width, height,
                0, (_frameId++ % INT32_MAX)
            );
        }
        
        _fullInvalidate = false;
    }
    
    _dirtyRects.clear();
    server_end_update(this);

    _commitUpdateLock.unlock();
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
