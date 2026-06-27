#pragma once

#include <memory>

#include "domain/IDisplay.h"

class IDisplayRenderer;
class IDisplayDevice;

class DisplayAdapter final : public IDisplay
{
  public:
    DisplayAdapter(std::unique_ptr<IDisplayRenderer> renderer,
                   std::unique_ptr<IDisplayDevice>   display);
    ~DisplayAdapter() override;

    DisplayAdapter(const DisplayAdapter&)            = delete;
    DisplayAdapter& operator=(const DisplayAdapter&) = delete;
    DisplayAdapter(DisplayAdapter&&)                 = delete;
    DisplayAdapter& operator=(DisplayAdapter&&)      = delete;

    void display(AquariaState state, const Readings& readings) override;

  private:
    std::unique_ptr<IDisplayRenderer> m_renderer;
    std::unique_ptr<IDisplayDevice>   m_display;
};
