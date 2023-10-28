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

        ulalaca::ULSurfaceTransaction transaction;
        transaction.addRects(*update.dirtyRects);
        transaction.setBitmap(update.image, update.size, update.width, update.height);
        transaction.setTimestamp(update.timestamp);

        _surface->beginUpdate();
        if (!_surface->submitUpdate(transaction)) {
            // log("frame dropped");
        }
        _surface->endUpdate();
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


