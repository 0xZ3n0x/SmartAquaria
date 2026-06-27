#pragma once

#include <cstddef>
#include <cstdint>

class ISPI
{
public:
    virtual ~ISPI() = default;

    virtual bool transfer(const uint8_t* tx, uint8_t* rx, size_t len) = 0;
    virtual bool isOpen() const = 0;
    virtual void close() = 0;
};
