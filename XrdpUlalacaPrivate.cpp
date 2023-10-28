//
// Created by Gyuhwan Park on 2023/01/28.
//

#include <cmath>

#include "XrdpUlalacaPrivate.hpp"

#include "ProjectorClient.hpp"
#include "ULSurface.hpp"

const std::string XrdpUlalacaPrivate::XRDP_ULALACA_VERSION = "0.1.1-git";

XrdpUlalacaPrivate::XrdpUlalacaPrivate(XrdpUlalaca *mod):
        _mod(mod),

        _error(0),

        _sessionSize(),
        _screenLayouts(),

        _bpp(0),

        _username(),
        _password(),
        _ip(),
        _port(),

        _keyLayout(0),
        _delayMs(0),
        _guid(),
        _encodingsMask(0),
        _clientInfo(),

        _socket(),
        _projectorClient(),

        _fullInvalidate(true),
        _commitUpdateLock(),

        _dirtyRects(std::make_shared<std::vector<ULIPCRect>>()),
        _surface(std::make_unique<ulalaca::ULSurface>(mod))
{
}

XrdpUlalacaPrivate::~XrdpUlalacaPrivate() {
    _isUpdateThreadRunning = false;
    if (_updateThread != nullptr && _updateThread->joinable()) {
        _updateThread->join();
    }
}

void XrdpUlalacaPrivate::serverMessage(const char *message, int code) {
    _mod->server_msg(_mod, message, code);
    LOG(LOG_LEVEL_INFO, "server_msg: %s", message);
}

void XrdpUlalacaPrivate::attachToSession(std::string sessionPath) {
    if (_projectorClient != nullptr) {
        LOG(LOG_LEVEL_ERROR, "already attached to session");
        // TODO: throw runtime_error
        return;
    }

    _projectorClient = std::make_unique<ProjectorClient>(
        *this, sessionPath
    );

    _projectorClient->start();

    _projectorClient->sendHello(XRDP_ULALACA_VERSION, _clientInfo);

    _isUpdateThreadRunning = true;
    _updateThread = std::make_unique<std::thread>(
        &XrdpUlalacaPrivate::updateThreadLoop, this
    );

    _projectorClient->setViewport(_sessionSize);
    _projectorClient->setOutputSuppression(false);

    _surface->setCRectSize(decideCopyRectSize());
}

int XrdpUlalacaPrivate::decideCopyRectSize() const {
    if (isRFXCodec()) {
        return 64;
    }

    if (isNSCodec() || isH264Codec() || isGFXH264Codec()) {
        // ??? FIXME
        return ulalaca::ULSurface::RECT_SIZE_BYPASS_CREATE;
    }

    return ulalaca::ULSurface::RECT_SIZE_BYPASS_CREATE;
}

void XrdpUlalacaPrivate::addDirtyRect(ULIPCRect &rect) {
    // TODO: REMOVE THIS
    /*
    for (auto &x: *_dirtyRects) {
        if (isRectOverlaps(x, rect)) {
            mergeRect(x, rect);
            return;
        }
    }


    _dirtyRects->push_back(rect);
     */
    _dirtyRects->push_back(rect);
}

void XrdpUlalacaPrivate::commitUpdate(const uint8_t *image, size_t size, int32_t width, int32_t height) {
    std::shared_ptr<uint8_t> tmp((uint8_t *) g_malloc(size, 0), g_free);
    memmove(tmp.get(), image, size);

    auto dirtyRects = _dirtyRects;
    _dirtyRects = std::make_shared<std::vector<ULIPCRect>>();

    auto now = std::chrono::steady_clock::now();
    _updateQueue.emplace(ScreenUpdate {
            std::chrono::duration<double>(now.time_since_epoch()).count(),
            tmp, size, width, height, dirtyRects
    });
}

void XrdpUlalacaPrivate::ipcDisconnected() {
    LOG(LOG_LEVEL_WARNING, "ipc disconnected");
    _error = 1;
}

void XrdpUlalacaPrivate::updateThreadLoop() {
    std::vector<ULIPCRect> skippedRects;

    while (_isUpdateThreadRunning) {
        while (_updateQueue.empty()) {
            using namespace std::chrono_literals;
            // TODO: use condition_variable
            std::this_thread::sleep_for(4ms);
        }

        if (!_isUpdateThreadRunning) {
            break;
        }

        _commitUpdateLock.lock();
        auto update = std::move(_updateQueue.front());
        _updateQueue.pop();
        _commitUpdateLock.unlock();


        auto now = std::chrono::steady_clock::now();
        auto tdelta = std::chrono::duration<double>(now.time_since_epoch()).count() - update.timestamp;

        auto width = update.width;
        auto height = update.height;
        auto dirtyRects = update.dirtyRects;
        auto image = update.image;

        if (tdelta > 1.0 / 30.0 || _updateQueue.size() > 2 || std::abs(_frameId - _ackFrameId) > 1) {
            LOG(LOG_LEVEL_INFO, "skipping frame (tdelta = %.4f)", tdelta);
            skippedRects.insert(skippedRects.end(), dirtyRects->begin(), dirtyRects->end());
            continue;
        } else {
            dirtyRects->insert(dirtyRects->end(), skippedRects.begin(), skippedRects.end());
            skippedRects.clear();
        }

        // LOG(LOG_LEVEL_TRACE, "updating screen: [%.4f] %d, %d", update.timestamp, update.width, update.height);

        if (_sessionSize.width != update.width || _sessionSize.height != update.height) {
            // server_reset(this, width, height, _bpp);
        }

        if (_frameId > 0 && dirtyRects->empty()) {
            continue;
        }

        _mod->server_begin_update(_mod);

        ULIPCRect screenRect {0, 0, (short) width, (short) height};
        auto copyRectSize = decideCopyRectSize();

#ifdef XRDP_TUMOD_ENCODER_HINTS_AVAILABLE
        int paintFlags = XRDP_ENCODER_HINT_QUALITY_LOW;
#else
        int paintFlags = 0;
#endif

        if (!_fullInvalidate) {
            auto copyRects = createCopyRects(*dirtyRects, copyRectSize);

            _mod->server_paint_rects(
                    _mod,
                    dirtyRects->size(), reinterpret_cast<short *>(dirtyRects->data()),
                    copyRects->size(), reinterpret_cast<short *>(copyRects->data()),
                    (char *) image.get(),
                    width, height,
                    paintFlags, (_frameId++ % INT32_MAX)
            );
        } else {
            // paint entire screen
            auto dirtyRects = std::vector<ULIPCRect> { screenRect } ;
            auto copyRects = createCopyRects(dirtyRects, copyRectSize);

            if (isRawBitmap()) {
                _mod->server_paint_rect(
                        _mod,
                        screenRect.x, screenRect.y,
                        screenRect.width, screenRect.height,
                        (char *) image.get(),
                        screenRect.width, screenRect.height,
                        0, 0
                );
            } else {
                _mod->server_paint_rects(
                        _mod,
                        dirtyRects.size(), reinterpret_cast<short *>(dirtyRects.data()),
                        copyRects->size(), reinterpret_cast<short *>(copyRects->data()),
                        (char *) image.get(),
                        width, height,
                        paintFlags, (_frameId++ % INT32_MAX)
                );
            }

            _fullInvalidate = false;
        }

        _mod->server_end_update(_mod);
    }

}

void XrdpUlalacaPrivate::setSessionSize(int width, int height) {
    _screenLayouts.clear();
    _screenLayouts.emplace_back(ULIPCRect {
            0, 0, (short) width, (short) height
    });
    calculateSessionSize();

    _surface->setSize(_sessionSize.width, _sessionSize.height);
    if (_projectorClient != nullptr) {
        _projectorClient->setViewport(_sessionSize);
    }
}

void XrdpUlalacaPrivate::calculateSessionSize() {
    // TODO: calculate session size to support multiple display environments
    if (_screenLayouts.empty()) {
        _sessionSize = ULIPCRect {
            0, 0, 640, 480
        };
        return;
    }

    _sessionSize = _screenLayouts.front();
}

bool XrdpUlalacaPrivate::isNSCodec() const {
    return _clientInfo.ns_codec_id != 0;
}

bool XrdpUlalacaPrivate::isRFXCodec() const {
    return _clientInfo.rfx_codec_id != 0;
}

bool XrdpUlalacaPrivate::isJPEGCodec() const {
    return _clientInfo.jpeg_codec_id != 0;
}

bool XrdpUlalacaPrivate::isH264Codec() const {
    return _clientInfo.h264_codec_id != 0;
}

bool XrdpUlalacaPrivate::isGFXH264Codec() const {
    return _clientInfo.capture_code & 3;
}

bool XrdpUlalacaPrivate::isRawBitmap() const {
    return !(isNSCodec() || isRFXCodec() || isJPEGCodec() || isH264Codec() || isGFXH264Codec());
}


