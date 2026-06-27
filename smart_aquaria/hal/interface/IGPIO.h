#pragma once

class IGPIO
{
public:
    virtual ~IGPIO() = default;

    virtual bool write(bool active) = 0;
    virtual bool isOpen() const = 0;
    virtual void close() = 0;
};
