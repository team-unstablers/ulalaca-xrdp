//
// Created by Gyuhwan Park on 10/27/23.
//

#ifndef ULALACA_CGTEXTRENDERER_HPP
#define ULALACA_CGTEXTRENDERER_HPP

#include <string>
#include <memory>

#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>

template <typename TypeRef>
struct ULCFTypeRefHolder {
    TypeRef ref;
};

template <typename T>
using ULCFSharedPtr = std::shared_ptr<ULCFTypeRefHolder<T>>;

template <typename T>
void ULCFRelease(ULCFTypeRefHolder<T> *holder) {
    if (holder == nullptr) {
        // ???
        return;
    }

    if (holder->ref != nullptr) {
        CFRelease(holder->ref);
        holder->ref = nullptr;
    }

    free(holder);
}

template <typename T>
inline ULCFSharedPtr<T> ULCFWrapRef(T ref) {
    return std::shared_ptr<ULCFTypeRefHolder<T>>(new ULCFTypeRefHolder<T>{ref}, ULCFRelease<T>);
}

using ULCFDictionaryPrototype = std::unordered_map<CFStringRef, CFTypeRef>;

inline ULCFSharedPtr<CFStringRef> ULCFCreateString(const std::string &text);
inline ULCFSharedPtr<CFDictionaryRef> ULCFCreateDictionary(const ULCFDictionaryPrototype &prototype);

inline ULCFSharedPtr<CFAttributedStringRef> ULCFCreateAttributedString(const ULCFSharedPtr<CFStringRef> &string, const ULCFSharedPtr<CFDictionaryRef> &attributes);

inline ULCFSharedPtr<CTFontRef> ULCTCreateFont(const std::string &fontName, size_t fontSize);



/**
 * @brief a class for rendering text as BGRA32 image.
 */
class ULCGTextRenderer {
public:
    ULCGTextRenderer(const std::string &text, size_t fontSize);
    ULCGTextRenderer(ULCGTextRenderer &) = delete;

    ~ULCGTextRenderer() = default;


    int width() const;
    int height() const;

    void setWidth(int width);
    void setHeight(int height);

    /**
     * @brief measure the text size. this method must be called before render() if width or height is not set.
     * @return OSStatus
     */
    OSStatus measure();

    /**
     * @brief render the text as BGRA32 image.
     * @return
     */
    OSStatus render();

    /**
     * @brief get the rendered image.
     * @return
     */
    std::shared_ptr<uint8_t> image() const;
    size_t size() const;

private:
    ULCFSharedPtr<CFStringRef> _text;
    ULCFSharedPtr<CTFontRef> _font;
    size_t _fontSize;

    int _width;
    int _height;

    size_t _imageSize;
    std::shared_ptr<uint8_t> _image;
};

#endif //ULALACA_CGTEXTRENDERER_HPP
