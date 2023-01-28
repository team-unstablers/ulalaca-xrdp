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

#include <mutex>

#include "XrdpEvent.hpp"
#include "XrdpTransport.hpp"
#include "XrdpStream.hpp"

#include "UnixSocket.hpp"

#include "messages/projector.h"

struct XrdpUlalaca;
class ProjectionThread;

class XrdpUlalacaPrivate {
public:
    constexpr static const int RECT_SIZE_BYPASS_CREATE = 0;

    constexpr static const int NO_ERROR = 0;

    explicit XrdpUlalacaPrivate(XrdpUlalaca *mod);
    XrdpUlalacaPrivate(XrdpUlalacaPrivate &) = delete;

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

    /* utility methods / lib_server_* wrappers */
    void serverMessage(const char *message, int code);

    /* session-broker related */
    inline std::string getSessionSocketPathUsingCredential(
            const std::string &username,
            const std::string &password
    );

    /* paint related */
    inline int decideCopyRectSize() const;
    inline std::unique_ptr<std::vector<Rect>> createCopyRects(std::vector<Rect> &dirtyRects, int rectSize) const;

    void addDirtyRect(Rect &rect);
    void commitUpdate(const uint8_t *image, int32_t width, int32_t height);

    void calculateSessionSize();

    inline bool isRFXCodec() const;
    inline bool isJPEGCodec() const;
    inline bool isH264Codec() const;
    inline bool isGFXH264Codec() const;
    inline bool isRawBitmap() const;

private:
    XrdpUlalaca *_mod;
    int _error = 0;

    ULIPCRect _sessionSize;
    std::vector<ULIPCRect> _screenLayouts;

    int _bpp;

    std::atomic_int64_t _frameId;
    std::atomic_int64_t _ackFrameId;

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
    std::unique_ptr<ProjectionThread> _projectionThread;

    std::atomic_bool _fullInvalidate;
    std::mutex _commitUpdateLock;

    std::vector<ULIPCRect> _dirtyRects;
};

#endif