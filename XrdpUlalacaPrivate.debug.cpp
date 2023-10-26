//
// Created by Gyuhwan Park on 10/27/23.
//

#include <sstream>

#include "XrdpUlalacaPrivate.hpp"
#include "ProjectorClient.hpp"

#include "ULCGTextRenderer.hpp"

#include "ulalaca.hpp"

bool XrdpUlalacaPrivate::isDebugStatisticsEnabled() const {
    return _isDebugStatisticsEnabled;
}

void XrdpUlalacaPrivate::setDebugStatisticsEnabled(bool enabled) {
    _isDebugStatisticsEnabled = enabled;
}

void XrdpUlalacaPrivate::drawDebugText(const std::string &text, size_t fontSize, int x, int y) {
    // TODO: add multi-line support to ULCGTextRenderer
    // FIXME: x, y coordinates will be ignored since server_paint_rects() can only draw on (0, 0)

    ULCGTextRenderer textRenderer(text, fontSize);
    textRenderer.measure();
    assert(textRenderer.render() == 0);

    auto image = textRenderer.image();

    auto rects = std::vector<ULIPCRect> {
            ULIPCRect { (short) 0, (short) 0, (short) textRenderer.width(), (short) textRenderer.height() }
    };

    auto rects2 = std::vector<ULIPCRect> {
            ULIPCRect { (short) 0, (short) 0, (short) textRenderer.width(), (short) textRenderer.height() }
    };
    auto copyRects = createCopyRects(rects2, decideCopyRectSize());

    _mod->server_paint_rects(
            _mod,
            rects.size(), reinterpret_cast<short *>(rects.data()),
            copyRects->size(), reinterpret_cast<short *>(copyRects->data()),
            (char *) image.get(),

            textRenderer.width(), textRenderer.height(),
            0, (_frameId++ % INT32_MAX)
    );

}

void XrdpUlalacaPrivate::drawDebugStatistics(size_t dirtyRectsSize, double timedelta) {
    std::stringstream sstream;

    sstream << "[DEBUG STATISTICS] \n";
    sstream << _sessionSize.width << "x" << _sessionSize.height << "x" << _bpp;
    sstream << " / \n";

    sstream << "frame #" << _frameId << " / ack #" << _ackFrameId << " / \n";

    sstream << "tΔ = " << timedelta << " / \n";

    sstream << dirtyRectsSize << " rects invalidated";

    drawDebugText(sstream.str(), 12, 0, 0);
}