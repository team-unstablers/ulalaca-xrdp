//
// Created by Gyuhwan Park on 2022/05/06.
//

#ifndef XRDP_ULALACAMESSAGES_HPP
#define XRDP_ULALACAMESSAGES_HPP

#include <cstdint>

#define FIXME_MARK_AS_PACKED_STRUCT __attribute__ ((packed))

struct Rect {
    short x;
    short y;
    short width;
    short height;
} __attribute__ ((packed));

namespace projector {
    
    enum MessageType: uint16_t {
        IN_SCREEN_UPDATE_EVENT = 0x0101,
        IN_SCREEN_COMMIT_UPDATE = 0x0102,
        
        OUT_SCREEN_UPDATE_REQUEST = 0x0201,
        
        OUT_KEYBOARD_EVENT = 0x0311,
        
        OUT_MOUSE_MOVE_EVENT = 0x0321,
        OUT_MOUSE_BUTTON_EVENT = 0x0322,
        OUT_MOUSE_WHEEL_EVENT = 0x0323
    };
    
    struct MessageHeader {
        uint16_t messageType;
        
        uint64_t id;
        uint64_t replyTo;
    
        uint64_t timestamp;
        
        uint64_t length;
    } FIXME_MARK_AS_PACKED_STRUCT;
    
    /**
     * incoming message
     */
    struct ScreenUpdateEvent {
        uint8_t type;
    
        Rect rect;
    } FIXME_MARK_AS_PACKED_STRUCT;
    
    struct ScreenCommitUpdate {
        Rect screenRect;
        
        uint64_t bitmapLength;
    } FIXME_MARK_AS_PACKED_STRUCT;
    
    struct ScreenUpdateRequest {
        static const uint8_t TYPE_ENTIRE_SCREEN = 0;
        static const uint8_t TYPE_PARTIAL = 1;
        
        uint8_t type;
        
        Rect rect;
    };
    
    struct KeyboardEvent {
        static const uint8_t TYPE_NOOP = 0;
        static const uint8_t TYPE_KEYUP = 1;
        static const uint8_t TYPE_KEYDOWN = 2;
        
        /**
         * force releases pressed keys
         */
        static const uint8_t TYPE_RESET = 4;
        
        static const uint16_t FLAG_IGNORE_TIMESTAMP_QUEUE = 0b00000001;
        static const uint16_t FLAG_EMIT_EVENT_USING_KARABINER = 0b00010000;
        
        
        uint8_t type;
        uint32_t keyCode;
        
        uint16_t flags;
    } FIXME_MARK_AS_PACKED_STRUCT;
    
    struct MouseMoveEvent {
        static const uint16_t FLAG_IGNORE_TIMESTAMP_QUEUE = 0b0001;
        static const uint16_t FLAG_EMIT_EVENT_USING_KARABINER = 0b00010000;
        
        uint16_t x;
        uint16_t y;
        
        uint16_t flags;
    } FIXME_MARK_AS_PACKED_STRUCT;
    
    struct MouseButtonEvent {
        static const uint8_t TYPE_NOOP = 0;
        static const uint8_t TYPE_MOUSEUP = 1;
        static const uint8_t TYPE_MOUSEDOWN = 2;
    
        static const uint8_t BUTTON_LEFT = 0;
        static const uint8_t BUTTON_RIGHT = 1;
        static const uint8_t BUTTON_MIDDLE = 2;
    
        static const uint16_t FLAG_IGNORE_TIMESTAMP_QUEUE = 0b0001;
        static const uint16_t FLAG_EMIT_EVENT_USING_KARABINER = 0b00010000;
        
        // ...
        
        uint8_t type;
        uint8_t button;
        
        uint16_t flags;
    } FIXME_MARK_AS_PACKED_STRUCT;
    
    struct MouseWheelEvent {
        static const uint16_t FLAG_IGNORE_TIMESTAMP_QUEUE = 0b0001;
        static const uint16_t FLAG_EMIT_EVENT_USING_KARABINER = 0b00010000;
    
        int32_t deltaX;
        int32_t deltaY;
        
        uint16_t flags;
    } FIXME_MARK_AS_PACKED_STRUCT;
    
    
}

#endif //XRDP_ULALACAMESSAGES_HPP
