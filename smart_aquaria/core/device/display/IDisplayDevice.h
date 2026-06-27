#pragma once

enum class DisplayStyle { Normal, Bold, Dim, Red, Yellow, Green, Cyan };

class IDisplayDevice
{
public:
    virtual ~IDisplayDevice() = default;

    virtual void clear() = 0;
    virtual void print(const char* text) = 0;
    virtual void printStyled(const char* text, DisplayStyle style) = 0;
    virtual void flush() = 0;
};
