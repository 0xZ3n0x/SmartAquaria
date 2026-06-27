#pragma once

#include "domain/AquariaState.h"
#include "domain/Readings.h"

class IDisplay
{
public:
    virtual ~IDisplay() = default;
    virtual void display(AquariaState state, const Readings& readings) = 0;
};