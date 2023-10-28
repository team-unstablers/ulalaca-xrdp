#ifndef XRDP_XRDPULALACAPRIVATE_HPP
#define XRDP_XRDPULALACAPRIVATE_HPP

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
};

#include <queue>
#include <mutex>

#include "XrdpEvent.hpp"
#include "XrdpTransport.hpp"
#include "XrdpStream.hpp"

#include "UnixSocket.hpp"

#include "ProjectionTarget.hpp"

struct XrdpUlalaca;
class ProjectorClient;

struct ScreenUpdate {
    double timestamp;

    std::shared_ptr<uint8_t> image;
    size_t size;
    int32_t width;
    int32_t height;

    std::shared_ptr<std::vector<ULIPCRect>> dirtyRects;
};

namespace ulalaca {
    class ULSurface;
}

class XrdpUlalacaPrivate: public ProjectionTarget {
public:
    // FIXME: is there TWO VERSION FIELDS??? (see ulalaca.hpp)
    static const std::string XRDP_ULALACA_VERSION;


    constexpr static const int NO_ERROR = 0;

    explicit XrdpUlalacaPrivate(XrdpUlalaca *mod);
    XrdpUlalacaPrivate(XrdpUlalacaPrivate &) = delete;
    ~XrdpUlalacaPrivate();

public:
    /* lib_mod_* */
    int libModStart(int width, int height, int bpp);
    int libModConnect();
    int libModEvent(int type, long arg1, long arg2, long arg3, long arg4);
    int libModSignal();
    int libModEnd();
    int libModSetParam(const char *cstrName, const char *cstrValue);
    int libModSessionChange(int, int);
    int libModGetWaitObjs(tbus *readObjs, int *rcount,
                          tbus *writeObjs, int *wcount, int *timeout);
    int libModCheckWaitObjs();
    int libModFrameAck(int flags, int frameId);
    int libModSuppressOutput(int suppress,
                             int left, int top, int right, int bottom);
    int libModServerMonitorResize(int width, int height);
    int libModServerMonitorFullInvalidate(int width, int height);
    int libModServerVersionMessage();

public:
    /* utility methods / lib_server_* wrappers */
    void serverMessage(const char *message, int code);

    /**
     * attach to projector session
     */
    void attachToSession(std::string sessionPath);

    int decideCopyRectSize() const;

    void addDirtyRect(ULIPCRect &rect) override;
    void commitUpdate(const uint8_t *image, size_t size, int32_t width, int32_t height) override;
    void ipcDisconnected() override;

    void updateThreadLoop();

    /**
     * TODO: add support for multiple displays
     */
    void setSessionSize(int width, int height);
    void calculateSessionSize();

    inline bool isNSCodec() const;
    inline bool isRFXCodec() const;
    inline bool isJPEGCodec() const;
    inline bool isH264Codec() const;
    inline bool isGFXH264Codec() const;
    inline bool isRawBitmap() const;

private:
    XrdpUlalaca *_mod;

    int _error = 0;
    bool _isUpdateThreadRunning;

    ULIPCRect _sessionSize;
    std::vector<ULIPCRect> _screenLayouts;

    int _bpp;

    std::string _username;
    std::string _password;
    std::string _ip;
    std::string _port;
    int _keyLayout;
    int _delayMs;
    guid _guid;
    int _encodingsMask;
    xrdp_client_info _clientInfo;


    std::unique_ptr<UnixSocket> _socket;
    std::unique_ptr<ProjectorClient> _projectorClient;
    std::unique_ptr<std::thread> _updateThread;

    std::atomic_bool _fullInvalidate;
    std::mutex _commitUpdateLock;

    std::shared_ptr<std::vector<ULIPCRect>> _dirtyRects;
    std::unique_ptr<ulalaca::ULSurface> _surface;

    std::queue<ScreenUpdate> _updateQueue;
};

#endif