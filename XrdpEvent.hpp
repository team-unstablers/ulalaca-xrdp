//
// Created by Gyuhwan Park on 2022/04/30.
//

#ifndef ULALACA_XRDPEVENT_HPP
#define ULALACA_XRDPEVENT_HPP

struct XrdpEvent {
public:
    enum Type {
        KEY_DOWN = 15,
        KEY_UP = 16,
        KEY_SYNCHRONIZE_LOCK = 17,
        
        MOUSE_MOVE = 100,
        MOUSE_BUTTON_LEFT_UP = 101,
        MOUSE_BUTTON_LEFT_DOWN = 102,
        MOUSE_BUTTON_RIGHT_UP = 103,
        MOUSE_BUTTON_RIGHT_DOWN = 104,
        MOUSE_BUTTON_MIDDLE_UP = 105,
        MOUSE_BUTTON_MIDDLE_DOWN = 106,
        
        MOUSE_WHEEL_UP = 107,
        MOUSE_UNKNOWN_1 = 108,
        MOUSE_WHEEL_DOWN = 109,
        MOUSE_UNKNOWN_2 = 110,
        
        INVALIDATE_REQUEST = 200,
        
        CHANNEL_EVENT = 0x5555
    };
    
    bool isKeyEvent() const;
    bool isMouseEvent() const;
    
    Type type;
    
    long param1;
    long param2;
    long param3;
    long param4;
};


#endif //ULALACA_XRDPEVENT_HPP
