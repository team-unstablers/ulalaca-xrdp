//
// Created by Gyuhwan Park on 10/28/23.
//

#ifndef XRDP_TUMOD_ULSURFACE_HPP
#define XRDP_TUMOD_ULSURFACE_HPP

#include "ulalaca.hpp"

namespace ulalaca {
    class ULSurfaceException: public std::exception {
    public:
        explicit ULSurfaceException(const std::string &message);
        ~ULSurfaceException() override = default;

        const std::string &getMessage() const;

        const char* what() const noexcept override;

    private:
        std::string _message;
    };

    class ULSurfaceTransaction {
    public:
        explicit ULSurfaceTransaction();

        void setTimestamp(double timestamp);

        void addRect(const ULIPCRect &rect);
        void addRects(const std::vector<ULIPCRect> &rects);

        void setBitmap(std::shared_ptr<uint8_t> bitmap, size_t bitmapSize,
                       int bitmapWidth, int bitmapHeight);

        double timestamp() const;

        const std::vector<ULIPCRect> &rects() const;

        std::shared_ptr<uint8_t> bitmap() const;
        size_t bitmapSize() const;
        int bitmapWidth() const;
        int bitmapHeight() const;

    private:
        std::vector<ULIPCRect> _rects;

        double _timestamp;

        std::shared_ptr<uint8_t> _bitmap;
        size_t _bitmapSize;
        int _bitmapWidth;
        int _bitmapHeight;
    };

    class ULSurface {
    public:
        constexpr static const int RECT_SIZE_BYPASS_CREATE = 0;

        static bool isRectOverlaps(const ULIPCRect &a, const ULIPCRect &b);
        static void mergeRect(ULIPCRect &a, const ULIPCRect &b);

        static std::vector<ULIPCRect> cleanupRects(const std::vector<ULIPCRect> &rects);
        static std::vector<ULIPCRect> createCopyRects(const std::vector<ULIPCRect> &drects,
                                                      int rectSize, int surfaceWidth, int surfaceHeight);

        static std::unique_ptr<short>allocateRectArray(const std::vector<ULIPCRect> &rects);



        /**
         * @brief force to use server_paint_rect() instead of server_paint_rects()
         */
        static const int FLAG_FORCE_RAW_BITMAP = 0x01;

        explicit ULSurface(XrdpUlalaca *mod);

        int width() const;
        int height() const;

        void setSize(int width, int height);
        void setCRectSize(int crectSize);

        void setAckFrameId(int ackFrameId);

        [[nodiscard]]
        bool canUpdate() const;

        /**
         * @brief notifies that the surface will be updated to the xrdp server.
         */
        void beginUpdate();

        /**
         * @brief notifies that the surface has been updated to the xrdp server.
         */
        void endUpdate();

        void drawBitmap(int x, int y, int width, int height,
                        const uint8_t *bitmap, size_t bitmapSize,
                        int bitmapWidth, int bitmapHeight,
                        int flags);

        /**
         * @note use submitUpdate() instead of this method when possible:
         *      - submitUpdate() drops frames when the client is not ready to receive updates
         */
        void drawBitmap(const std::vector<ULIPCRect> &rects,
                        const uint8_t *bitmap, size_t bitmapSize,
                        int bitmapWidth, int bitmapHeight,
                        int flags);


        /**
         * @return false if frame is dropped
         */
        bool submitUpdate(const ULSurfaceTransaction &transaction);

        void setForegroundColor(int color);
        void fillRect(int x, int y, int width, int height);

    protected:
        bool shouldDropFrame(const ULSurfaceTransaction &transaction) const;

    private:
        XrdpUlalaca *_mod;

        int _width;
        int _height;

        int _crectSize;

        int _frameId;
        int _ackFrameId;

        std::vector<ULIPCRect> _pendingRects;

        std::thread::id _updateThreadId;
    };
}

#endif //XRDP_TUMOD_ULSURFACE_HPP
