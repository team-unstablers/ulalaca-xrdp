//
// Created by Gyuhwan Park on 10/28/23.
//

#include "ULSurface.hpp"

namespace ulalaca {
    ULSurfaceException::ULSurfaceException(const std::string &message):
        _message(message)
    {

    }

    const std::string &ULSurfaceException::getMessage() const {
        return _message;
    }

    const char *ULSurfaceException::what() const noexcept {
        return _message.c_str();
    }

    ULSurfaceTransaction::ULSurfaceTransaction() {

    }

    void ULSurfaceTransaction::setTimestamp(double timestamp) {
        _timestamp = timestamp;
    }

    void ULSurfaceTransaction::addRect(const ULIPCRect &rect) {
        _rects.push_back(rect);
    }

    void ULSurfaceTransaction::addRects(const std::vector<ULIPCRect> &rects) {
        _rects.insert(_rects.end(), rects.begin(), rects.end());
    }

    void ULSurfaceTransaction::setBitmap(std::shared_ptr<uint8_t> bitmap, size_t bitmapSize, int bitmapWidth,
                                         int bitmapHeight) {
        _bitmap = std::move(bitmap);
        _bitmapSize = bitmapSize;
        _bitmapWidth = bitmapWidth;
        _bitmapHeight = bitmapHeight;
    }

    double ULSurfaceTransaction::timestamp() const {
        return _timestamp;
    }

    const std::vector<ULIPCRect> &ULSurfaceTransaction::rects() const {
        return _rects;
    }

    std::shared_ptr<uint8_t> ULSurfaceTransaction::bitmap() const {
        return _bitmap;
    }

    size_t ULSurfaceTransaction::bitmapSize() const {
        return _bitmapSize;
    }

    int ULSurfaceTransaction::bitmapWidth() const {
        return _bitmapWidth;
    }

    int ULSurfaceTransaction::bitmapHeight() const {
        return _bitmapHeight;
    }

    bool ULSurface::isRectOverlaps(const ULIPCRect &a, const ULIPCRect &b) {
        int16_t a_x1 = a.x;
        int16_t a_x2 = a.x + a.width;
        int16_t a_y1 = a.y;
        int16_t a_y2 = a.y + a.height;
        int16_t b_x1 = b.x;
        int16_t b_x2 = b.x + b.width;
        int16_t b_y1 = b.y;
        int16_t b_y2 = b.y + b.height;

        return (
                (a_x1 >= b_x1 && a_x1 <= b_x2 && a_y1 >= b_y1 && a_y1 <= b_y2) ||
                (a_x2 >= b_x1 && a_x2 <= b_x2 && a_y1 >= b_y1 && a_y1 <= b_y2) ||
                (a_x1 >= b_x1 && a_x1 <= b_x2 && a_y2 >= b_y1 && a_y2 <= b_y2) ||
                (a_x2 >= b_x1 && a_x2 <= b_x2 && a_y2 >= b_y1 && a_y2 <= b_y2)
        );
    }

    void ULSurface::mergeRect(ULIPCRect &a, const ULIPCRect &b) {
        int16_t a_x1 = a.x;
        int16_t a_x2 = a.x + a.width;
        int16_t a_y1 = a.y;
        int16_t a_y2 = a.y + a.height;
        int16_t b_x1 = b.x;
        int16_t b_x2 = b.x + b.width;
        int16_t b_y1 = b.y;
        int16_t b_y2 = b.y + b.height;

        a.x = std::min(a_x1, b_x1);
        a.y = std::min(a_y1, b_y1);
        a.width  = std::max(a_x2, b_x2) - a.x;
        a.height = std::max(a_y2, b_y2) - a.y;
    }

    std::vector<ULIPCRect> ULSurface::createCopyRects(
            const std::vector<ULIPCRect> &drects,
            int rectSize,
            int surfaceWidth, int surfaceHeight
    ) {
        if (rectSize == RECT_SIZE_BYPASS_CREATE) {
            return drects;
        }

        std::vector<ULIPCRect> blocks;

        blocks.reserve((surfaceWidth * surfaceHeight) / (rectSize * rectSize));

        /*
         * sorry, i'm not good at math. XDD
         */

        int mapWidth  = std::ceil((float) surfaceWidth / (float) rectSize);
        int mapHeight = std::ceil((float) surfaceHeight / (float) rectSize);
        int mapSize = mapWidth * mapHeight;
        std::unique_ptr<uint8_t> rectMap(new uint8_t[mapSize]);
        memset(rectMap.get(), 0x00, mapSize);

        for (auto &dirtyRect : drects) {
            if (surfaceWidth <= dirtyRect.x ||
                surfaceHeight <= dirtyRect.y) {
                continue;
            }

            int mapX1 = (int) std::floor((float) dirtyRect.x / rectSize);
            int mapY1 = (int) std::floor((float) dirtyRect.y / rectSize);
            int mapX2 = std::min(
                    (int) std::ceil((float) (dirtyRect.x + dirtyRect.width) / rectSize),
                    mapWidth
            ) - 1;
            int mapY2 = std::min(
                    (int) std::ceil((float) (dirtyRect.y + dirtyRect.height) / rectSize),
                    mapHeight
            ) - 1;

            for (int y = mapY1; y <= mapY2; y++) {
                for (int x = mapX1; x <= mapX2; x++) {
                    rectMap.get()[(y * mapWidth) + x] = 0x01;
                }
            }
        }

        for (int y = 0; y < mapHeight; y++) {
            for (int x = 0; x < mapWidth; x++) {
                if (rectMap.get()[(y * mapWidth) + x] == 0x01) {
                    int rectX = x * rectSize;
                    int rectY = y * rectSize;

                    blocks.emplace_back(ULIPCRect{(short) rectX, (short) rectY, (short) rectSize, (short) rectSize});
                }
            }
        }

        return std::move(blocks);
    }

    std::vector<ULIPCRect> ULSurface::cleanupRects(const std::vector<ULIPCRect> &rects) {
        std::vector<ULIPCRect> result;
        result.reserve(rects.size());

        for (const ULIPCRect &rect: rects) {
            bool merged = false;

            for (ULIPCRect &resultRect: result) {
                if (isRectOverlaps(resultRect, rect)) {
                    mergeRect(resultRect, rect);
                    merged = true;
                    break;
                }
            }

            if (!merged) {
                result.push_back(rect);
            }
        }

        return std::move(result);
    }

    std::unique_ptr<short> ULSurface::allocateRectArray(const std::vector<ULIPCRect> &rects) {
        static_assert(sizeof(ULIPCRect) == (sizeof(short) * 4));

        std::unique_ptr<short> result(new short[rects.size() * 4]);
        memcpy(result.get(), rects.data(), sizeof(ULIPCRect) * rects.size());

        return std::move(result);
    }

    ULSurface::ULSurface(XrdpUlalaca *mod):
        _mod(mod),

        _width(640),
        _height(480),

        _crectSize(RECT_SIZE_BYPASS_CREATE),

        _frameId(0),
        _ackFrameId(0),

        _updateThreadId(),

        __HACK__mstsc_workaround(true)
    {

    }

    int ULSurface::width() const {
        return _width;
    }

    int ULSurface::height() const {
        return _height;
    }

    void ULSurface::setSize(int width, int height) {
        _width = width;
        _height = height;
    }

    void ULSurface::setCRectSize(int crectSize) {
        _crectSize = crectSize;
    }

    void ULSurface::setAckFrameId(int ackFrameId) {
        _ackFrameId = ackFrameId;
    }

    bool ULSurface::canUpdate() const {
        return _updateThreadId == std::thread::id();
    }

    void ULSurface::beginUpdate() {
        if (_updateThreadId != std::thread::id()) {
            throw ULSurfaceException("another thread is updating the surface");
        }

        _updateThreadId = std::this_thread::get_id();
        _mod->server_begin_update(_mod);
    }

    void ULSurface::endUpdate() {
        if (_updateThreadId == std::thread::id()) {
            LOG(LOG_LEVEL_WARNING, "endUpdate() called without beginUpdate()");
            return;
        }

        _mod->server_end_update(_mod);
        _updateThreadId = std::thread::id();
    }

    void ULSurface::drawBitmap(int x, int y, int width, int height,
                               const uint8_t *bitmap, size_t bitmapSize,
                               int bitmapWidth, int bitmapHeight, int flags) {
        std::vector rects {
            ULIPCRect { (short) x, (short) y, (short) width, (short) height }
        };

        this->drawBitmap(rects, bitmap, bitmapSize, bitmapWidth, bitmapHeight, flags);
    }

    void ULSurface::drawBitmap(const std::vector<ULIPCRect> &rects, const uint8_t *bitmap, size_t bitmapSize,
                               int bitmapWidth, int bitmapHeight, int flags) {
        if (_updateThreadId != std::this_thread::get_id()) {
            throw ULSurfaceException("this thread is not holding the update lock");
        }

        if (rects.empty()) {
            return;
        }

        if (__HACK__mstsc_workaround) {
            // we should draw entire screen at least once to make mstsc.exe happy
            __HACK__mstsc_workaround = false;

            _mod->server_paint_rect(
                    _mod,
                    0, 0,
                    _width, _height,
                    (char *) bitmap,
                    bitmapWidth, bitmapHeight,
                    0, (_frameId++ % INT32_MAX)
            );

            return;
        }

        const std::vector<ULIPCRect> &cleanedRects = cleanupRects(rects);
        const std::vector<ULIPCRect> &copyRects = createCopyRects(cleanedRects, _crectSize, _width, _height);

        std::unique_ptr<short> drects = std::move(allocateRectArray(cleanedRects));
        std::unique_ptr<short> crects = std::move(allocateRectArray(copyRects));

        _mod->server_paint_rects(
            _mod,
            cleanedRects.size(), drects.get(),
            copyRects.size(), crects.get(),
            (char *) bitmap,
            bitmapWidth, bitmapHeight,
            0, (_frameId++ % INT32_MAX)
        );
    }

    void ULSurface::setForegroundColor(int color) {
        _mod->server_set_fgcolor(_mod, color);
    }

    void ULSurface::fillRect(int x, int y, int width, int height) {
        _mod->server_fill_rect(_mod, x, y, width, height);
    }

    bool ULSurface::submitUpdate(const ULSurfaceTransaction &transaction) {
        _pendingRects.insert(
                _pendingRects.end(),
                transaction.rects().begin(), transaction.rects().end()
        );

        if (shouldDropFrame(transaction)) {
            return false;
        }

        this->drawBitmap(
            _pendingRects,
            transaction.bitmap().get(), transaction.bitmapSize(),
            transaction.bitmapWidth(), transaction.bitmapHeight(),
            0
        );


        _pendingRects.clear();

        return true;
    }

    bool ULSurface::shouldDropFrame(const ULSurfaceTransaction &transaction) const {
        constexpr static const int MAX_FRAME_DELAY = 2;

        int fdelta = std::abs(_frameId - _ackFrameId);

        return (
            fdelta > MAX_FRAME_DELAY
        );
    }


}
