//
// Created by Gyuhwan Park on 2022/05/07.
//

#include "KeycodeMap.hpp"

#include <Carbon/Carbon.h>

static const int16_t RDP_CGKEYCODE_MAP[] = {
    -1,
    kVK_Escape,
    kVK_ANSI_1,
    kVK_ANSI_2,
    kVK_ANSI_3,
    kVK_ANSI_4,
    kVK_ANSI_5,
    kVK_ANSI_6,
    kVK_ANSI_7,
    kVK_ANSI_8,
    kVK_ANSI_9,
    kVK_ANSI_0,
    kVK_ANSI_Minus,
    kVK_ANSI_Equal,
    kVK_Delete, // 14
    
    kVK_Tab, // 15
    kVK_ANSI_Q,
    kVK_ANSI_W,
    kVK_ANSI_E,
    kVK_ANSI_R,
    kVK_ANSI_T,
    kVK_ANSI_Y,
    kVK_ANSI_U,
    kVK_ANSI_I,
    kVK_ANSI_O,
    kVK_ANSI_P,
    kVK_ANSI_LeftBracket,
    kVK_ANSI_RightBracket, // 27
    
    kVK_Return, // 28
    
    
    kVK_Control, // 29
    
    kVK_ANSI_A,
    kVK_ANSI_S,
    kVK_ANSI_D,
    kVK_ANSI_F,
    kVK_ANSI_G,
    kVK_ANSI_H,
    kVK_ANSI_J,
    kVK_ANSI_K,
    kVK_ANSI_L,
    kVK_ANSI_Semicolon,
    kVK_ANSI_Quote, // 40
    
    kVK_ANSI_Grave,
    kVK_Shift, // 42
    kVK_ANSI_Backslash,
    kVK_ANSI_Z, // 44
    kVK_ANSI_X,
    kVK_ANSI_C,
    kVK_ANSI_V,
    kVK_ANSI_B,
    kVK_ANSI_N,
    kVK_ANSI_M,
    kVK_ANSI_Comma,
    kVK_ANSI_Period,
    kVK_ANSI_Slash,
    kVK_RightShift, // 54
    
    -1,
    kVK_Option, // 56
    kVK_Space, // 57
    -1,
    kVK_F1,
    kVK_F2, // 60
    
    kVK_F3, // 61
    kVK_F4,
    kVK_F5,
    kVK_F6,
    kVK_F7,
    kVK_F8,
    kVK_F9,
    kVK_F10,
    kVK_F11,
    kVK_F12, // 70
    
    kVK_Home, // 71
    kVK_UpArrow,
    kVK_PageUp,
    -1,
    kVK_LeftArrow,
    -1,
    kVK_RightArrow,
    -1,
    kVK_End,
    kVK_DownArrow, // 80
    
    kVK_PageDown,
    -1, // FIXME: INSERT KEY
    kVK_ForwardDelete,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1, // 90
    
    kVK_Command
};

int16_t rdpKeycodeToCGKeycode(uint16_t keycode) {
    if (keycode <= 91) {
        return RDP_CGKEYCODE_MAP[keycode];
    }
    
    return -1;
}