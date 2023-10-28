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
        void drawBitmap(const std::vector<ULIPCRect> &rects,
                        const uint8_t *bitmap, size_t bitmapSize,
                        int bitmapWidth, int bitmapHeight,
                        int flags);

    private:
        XrdpUlalaca *_mod;

        int _width;
        int _height;

        int _crectSize;

        int _frameId;
        int _ackFrameId;

        std::thread::id _updateThreadId;
    };
}

#endif //XRDP_TUMOD_ULSURFACE_HPP
