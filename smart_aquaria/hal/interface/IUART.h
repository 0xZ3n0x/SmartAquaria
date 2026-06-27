#pragma once

#include <cstddef>

class IUART
{
public:
    virtual ~IUART() = default;

    virtual int  write(const char* data, size_t len) = 0;
    virtual int  read(char* buffer, size_t len) = 0;
    virtual void flush() = 0;
    virtual bool isOpen() const = 0;
    virtual void close() = 0;
};
