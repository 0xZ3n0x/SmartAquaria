#pragma once

#include "domain/IActuator.h"

class IBuzzer;
class IHeater;

class ActuatorAdapter final : public IActuator
{
  public:
    explicit ActuatorAdapter(IBuzzer& buzzer, IHeater& heater);
    ~ActuatorAdapter() = default;

    ActuatorAdapter(const ActuatorAdapter&) = delete;
    ActuatorAdapter& operator=(const ActuatorAdapter&) = delete;
    ActuatorAdapter(ActuatorAdapter&&) = delete;
    ActuatorAdapter& operator=(ActuatorAdapter&&) = delete;

    bool buzz(bool on) override;
    bool heat(bool on) override;
    void feed() override;

  private:
    IBuzzer& m_buzzer;
    IHeater& m_heater;
};
