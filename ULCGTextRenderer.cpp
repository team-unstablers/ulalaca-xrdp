//
// Created by Gyuhwan Park on 10/27/23.
//


#include "ULCGTextRenderer.hpp"

ULCFSharedPtr<CFStringRef> ULCFCreateString(const std::string &text) {
    CFStringRef stringRef = CFStringCreateWithCString(kCFAllocatorDefault, text.c_str(), kCFStringEncodingUTF8);

    return ULCFWrapRef(stringRef);
}

ULCFSharedPtr<CFDictionaryRef> ULCFCreateDictionary(const ULCFDictionaryPrototype &prototype) {
    CFMutableDictionaryRef dictionaryRef = CFDictionaryCreateMutable(
            kCFAllocatorDefault, (long) prototype.size(),
            &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks
    );

    for (const auto &pair: prototype) {
        CFDictionarySetValue(dictionaryRef, pair.first, pair.second);
    }

    return ULCFWrapRef((CFDictionaryRef) dictionaryRef);
}

ULCFSharedPtr<CFAttributedStringRef>
ULCFCreateAttributedString(const ULCFSharedPtr<CFStringRef> &string, const ULCFSharedPtr<CFDictionaryRef> &attributes) {
    CFAttributedStringRef attributedStringRef = CFAttributedStringCreate(
            kCFAllocatorDefault, string->ref, attributes->ref
    );

    return ULCFWrapRef(attributedStringRef);
}

ULCFSharedPtr<CTFontRef> ULCTCreateFont(const std::string &fontName, size_t fontSize) {
    auto cfFontName = ULCFCreateString(fontName);
    CTFontRef fontRef = CTFontCreateWithName(cfFontName->ref, (CGFloat) fontSize, nullptr);

    return ULCFWrapRef(fontRef);
}


ULCGTextRenderer::ULCGTextRenderer(const std::string &text, size_t fontSize):
    _text(ULCFCreateString(text)),
    _font(ULCTCreateFont("Monaco", fontSize)),
    _fontSize(fontSize),

    _width(-1), _height(-1),

    _imageSize(0),
    _image(nullptr)
{
}

int ULCGTextRenderer::width() const {
    return _width;
}

int ULCGTextRenderer::height() const {
    return _height;
}

void ULCGTextRenderer::setWidth(int width) {
    _width = width;
}

void ULCGTextRenderer::setHeight(int height) {
    _height = height;
}

OSStatus ULCGTextRenderer::measure() {
    auto attributes = ULCFCreateDictionary({
            { kCTFontAttributeName, _font->ref }
    });

    auto attributedString = ULCFCreateAttributedString(_text, attributes);
    auto line = ULCFWrapRef(CTLineCreateWithAttributedString(attributedString->ref));

    CGFloat ascent, descent, leading;
    double lineWidth = CTLineGetTypographicBounds(line->ref, &ascent, &descent, &leading);

    _width = (int) std::ceil(lineWidth);
    _height = (int) std::ceil(ascent + descent + leading);

    return noErr;
}

OSStatus ULCGTextRenderer::render() {
    if (_width <= 0 || _height <= 0) {
        return -1;
    }

    // FIXME: CGColorSpaceRelease() 써야 함
    auto colorSpace = ULCFWrapRef(CGColorSpaceCreateDeviceRGB());
    // FIXME: CGBitmapContextRelease() 써야 함
    auto context = ULCFWrapRef(CGBitmapContextCreate(
            nullptr,
            _width, _height, 8, _width * 4,
            colorSpace->ref,
            kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Little
    ));

    if (context->ref == nullptr) {
        return -2;
    }

    auto attributes = ULCFCreateDictionary({
           { kCTFontAttributeName, _font->ref },
           { kCTForegroundColorAttributeName, CGColorGetConstantColor(kCGColorWhite) }
    });

    auto attributedString = ULCFCreateAttributedString(_text, attributes);
    auto line = ULCFWrapRef(CTLineCreateWithAttributedString(attributedString->ref));

    CGContextSetTextPosition(context->ref, 0.0, (CGFloat) (_height - _fontSize));
    CTLineDraw(line->ref, context->ref);

    void *destPtr = CGBitmapContextGetData(context->ref);
    size_t destSize = CGBitmapContextGetBytesPerRow(context->ref) * (_height);

    if (destPtr == nullptr) {
        return -2;
    }

    uint8_t *destCopy = (uint8_t *) malloc(destSize);
    memcpy(destCopy, destPtr, destSize);

    _image = std::shared_ptr<uint8_t>(destCopy, free);
    _imageSize = destSize;

    return 0;
}

std::shared_ptr<uint8_t> ULCGTextRenderer::image() const {
    return _image;
}

size_t ULCGTextRenderer::size() const {
    return _imageSize;
}
