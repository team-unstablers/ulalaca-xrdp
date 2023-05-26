//
// Created by Gyuhwan Park on 2023/05/26.
//

#include <memory>

#include "catch_amalgamated.hpp"

#include "../XrdpUlalacaPrivate.hpp"
#include "../ulalaca.hpp"

std::shared_ptr<XrdpUlalaca> createMockedXrdpUlalaca() {
    auto ulalaca = std::make_shared<XrdpUlalaca>();

    // TODO: replace some methods with mocked methods

    return ulalaca;
}

TEST_CASE("size of crects must be calculated correctly", "[ulalaca]") {
    auto ulalaca = createMockedXrdpUlalaca();
    auto &impl = ulalaca->_impl;

    SECTION("RFX") {
        // inject client info;
        auto clientInfo = std::make_shared<xrdp_client_info>();
        clientInfo->rfx_codec_id = 1;

        ulalaca->lib_mod_set_param(ulalaca.get(), "client_info", (char *) clientInfo.get());

        REQUIRE(impl->decideCopyRectSize() == 64);
    }
}

TEST_CASE("crects must be created correctly", "[ulalaca]") {
    auto ulalaca = createMockedXrdpUlalaca();
    auto &impl = ulalaca->_impl;

    auto combineCopyRects = [](const std::shared_ptr<std::vector<ULIPCRect>> &rects) {
        std::pair<int, int> x { INT32_MAX, INT32_MIN };
        std::pair<int, int> y { INT32_MAX, INT32_MIN };

        for (const auto &rect: *rects) {
            x.first = std::min(x.first, (int) rect.x);
            x.second = std::max(x.second, (int) rect.x + rect.width);
            y.first = std::min(y.first, (int) rect.y);
            y.second = std::max(y.second, (int) rect.y + rect.height);
        }

        return ULIPCRect {
            (short) x.first,
            (short) y.first,
            (short) (x.second - x.first),
            (short) (y.second - y.first)
        };
    };

    SECTION("update single rect: [0, 0, 640, 640] @ 1280x720") {
        impl->setSessionSize(1280, 720);

        std::vector<ULIPCRect> dirtyRects {
            ULIPCRect { 0, 0, 640, 640 }
        };

        auto copyRects = impl->createCopyRects(dirtyRects, 64);

        int crectsWidth = std::ceil((float) 640 / 64);
        int crectsHeight = std::ceil((float) 640 / 64);

        REQUIRE(copyRects->size() == crectsWidth * crectsHeight);

        auto combinedRect = combineCopyRects(copyRects);

        REQUIRE(combinedRect.x == 0);
        REQUIRE(combinedRect.width == 640);
        REQUIRE(combinedRect.y == 0);
        REQUIRE(combinedRect.height == 640);
    }

    SECTION("update single rect: [0, 0, 1280, 720] @ 1280x720") {
        impl->setSessionSize(1280, 720);

        std::vector<ULIPCRect> dirtyRects {
            ULIPCRect { 0, 0, 1280, 720 }
        };

        auto copyRects = impl->createCopyRects(dirtyRects, 64);

        int crectsWidth = std::ceil((float) 1280 / 64);
        int crectsHeight = std::ceil((float) 720 / 64);

        REQUIRE(copyRects->size() == crectsWidth * crectsHeight);

        auto combinedRect = combineCopyRects(copyRects);

        REQUIRE(combinedRect.x == 0);
        REQUIRE(combinedRect.width == 1280);
        REQUIRE(combinedRect.y == 0);
        REQUIRE(combinedRect.height == 768);
    }

    /**
     * i am bad at math :'(
     */
    SECTION("update overlapped rects: [[240, 240, 320, 320], [280, 280, 640, 640]] @ 1280x720") {
        /*
                                768px
                       | - - - - - - - - - - |
              0: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
             64: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
            128: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
            192: 0 0 0 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0 0
            256: 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 -
            320: 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 |
            384: 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 |
            448: 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 |
            512: 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 | 576px
            576: 0 0 0 0 1 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 |
            640: 0 0 0 0 1 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 |
            704: 0 0 0 0 1 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 -
         */

        impl->setSessionSize(1280, 720);

        std::vector<ULIPCRect> dirtyRects {
            ULIPCRect { 240, 240, 320, 320 },
            ULIPCRect { 280, 280, 640, 640 },
        };

        auto copyRects = impl->createCopyRects(dirtyRects, 64);
        auto combinedRect = combineCopyRects(copyRects);

        REQUIRE(combinedRect.width == 768);
        REQUIRE(combinedRect.height == 576);
    }
}
