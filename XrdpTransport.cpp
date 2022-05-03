//
// Created by Gyuhwan Park on 2022/04/30.
//

#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

#include "XrdpTransport.hpp"

static void XrdpTransportDeleter(XrdpTransport::Transport *transport) {
    trans_delete(transport);
}

XrdpTransport::XrdpTransport(int mode, int inSize, int outSize):
    _transport {
        trans_create(mode, inSize, outSize),
        XrdpTransportDeleter
    }
{

}
