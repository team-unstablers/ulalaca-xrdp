//
// Created by Gyuhwan Park on 2023/01/28.
//

#include "XrdpUlalacaPrivate.hpp"

#include "ProjectionThread.hpp"

XrdpUlalacaPrivate::XrdpUlalacaPrivate(XrdpUlalaca *mod):
    _mod(mod),
    _error(0),

    _sessionSize(),
    _screenLayouts(),

    _bpp(0),

    _frameId(0),
    _ackFrameId(0),

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
    _projectionThread(),

    _fullInvalidate(false),
    _commitUpdateLock(),

    _dirtyRects()
{

}

void XrdpUlalacaPrivate::serverMessage(const char *message, int code) {
    _mod->server_msg(_mod, message, code);
    LOG(LOG_LEVEL_INFO, "server_msg: %s", message);
}

void XrdpUlalacaPrivate::attachToSession(std::string sessionPath) {
    if (_projectionThread != nullptr) {
        LOG(LOG_LEVEL_ERROR, "already attached to session");
        // TODO: throw runtime_error
        return;
    }

    _projectionThread = std::make_unique<ProjectionThread>(
        *this, sessionPath
    );

    _projectionThread->start();

    LOG(LOG_LEVEL_TRACE, "sessionSize: %d, %d", _sessionSize.width, _sessionSize.height);
    _projectionThread->setViewport(_sessionSize);
    _projectionThread->setOutputSuppression(false);
}

int XrdpUlalacaPrivate::decideCopyRectSize() const {
    if (isRFXCodec()) {
        return 64;
    }

    if (isH264Codec() || isGFXH264Codec()) {
        // ??? FIXME
        return RECT_SIZE_BYPASS_CREATE;
    }

    return RECT_SIZE_BYPASS_CREATE;
}

std::unique_ptr<std::vector<ULIPCRect>> XrdpUlalacaPrivate::createCopyRects(
    std::vector<ULIPCRect> &dirtyRects,
    int rectSize
) const {
    auto blocks = std::make_unique<std::vector<ULIPCRect>>();
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

                blocks->push_back(ULIPCRect {x, y, (short) rectSize, (short) rectSize });
            }
        }
    }

    return std::move(blocks);
}

void XrdpUlalacaPrivate::addDirtyRect(ULIPCRect &rect) {
    std::scoped_lock<std::mutex> scopedCommitLock(_commitUpdateLock);
    _dirtyRects.push_back(rect);
}

void XrdpUlalacaPrivate::commitUpdate(const uint8_t *image, int32_t width, int32_t height) {
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

    _mod->server_begin_update(_mod);

    ULIPCRect screenRect {0, 0, (short) width, (short) height};
    auto copyRectSize = decideCopyRectSize();

    if (_frameId > 0 || !_fullInvalidate) {
        auto copyRects = createCopyRects(_dirtyRects, copyRectSize);

        _mod->server_paint_rects(
                _mod,
                _dirtyRects.size(), reinterpret_cast<short *>(_dirtyRects.data()),
                copyRects->size(), reinterpret_cast<short *>(copyRects->data()),
                (char *) image,
                width, height,
                0, (_frameId++ % INT32_MAX)
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
                    (char *) image,
                    screenRect.width, screenRect.height,
                    0, 0
            );
        } else {
            _mod->server_paint_rects(
                    _mod,
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
    _mod->server_end_update(_mod);

    _commitUpdateLock.unlock();
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
    return !(isRFXCodec() || isJPEGCodec() || isH264Codec() || isGFXH264Codec());
}


