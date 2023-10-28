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

        _dirtyRects(),
        _surface(std::make_unique<ulalaca::ULSurface>(mod)),
        _updateWaitObj(g_create_wait_obj("ulalaca_screen_update"))
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
    _dirtyRects.push_back(rect);
}

void XrdpUlalacaPrivate::commitUpdate(const uint8_t *image, size_t size, int32_t width, int32_t height) {
    auto transaction = std::make_unique<ulalaca::ULSurfaceTransaction>();
    // FIXME: TODO: timestamp should be retrieved from the sessionprojector.app
    auto now = std::chrono::steady_clock::now();
    double timestamp = std::chrono::duration<double>(now.time_since_epoch()).count();

    transaction->addRects(_dirtyRects);
    transaction->setTimestamp(timestamp);
    transaction->setBitmap(
            std::shared_ptr<uint8_t>((uint8_t *) g_malloc(size, 0), g_free),
            size, width, height
    );

    memmove(transaction->bitmap().get(), image, size);
    _dirtyRects.clear();

    {
        std::lock_guard<std::mutex> lockGuard(_updateQueueMutex);
        _updateQueue.emplace(std::move(transaction));
    }

    _updateQueueCondvar.notify_one();
}

void XrdpUlalacaPrivate::ipcDisconnected() {
    LOG(LOG_LEVEL_WARNING, "ipc disconnected");
    _error = 1;
}

void XrdpUlalacaPrivate::updateThreadLoop() {
    std::vector<ULIPCRect> skippedRects;
    std::unique_ptr<ulalaca::ULSurfaceTransaction> transaction;

    g_reset_wait_obj(_updateWaitObj);

    while (_isUpdateThreadRunning) {
        {
            using namespace std::chrono_literals;

            std::unique_lock<std::mutex> lock(_updateQueueMutex);
            if (!_updateQueueCondvar.wait_for(lock, 1s, [this] { return !_updateQueue.empty(); })) {
                continue;
            }

            transaction = std::move(_updateQueue.front());
            _updateQueue.pop();

            if (transaction == nullptr) {
                continue;
            }
        }

        try {
            _surface->beginUpdate();
            if (!_surface->submitUpdate(*transaction)) {
                // log("frame dropped");
            }
            _surface->endUpdate();
        } catch (ulalaca::ULSurfaceException &e) {
            LOG(LOG_LEVEL_ERROR, "failed to commit transaction: %s", e.what());
        }

        g_set_wait_obj(_updateWaitObj);
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


