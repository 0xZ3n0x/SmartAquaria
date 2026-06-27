#pragma once

class IActuator
{
public:
    virtual ~IActuator() = default;

    virtual bool buzz(bool on) = 0;
    virtual bool heat(bool on) = 0;
    virtual void feed() = 0;
};
