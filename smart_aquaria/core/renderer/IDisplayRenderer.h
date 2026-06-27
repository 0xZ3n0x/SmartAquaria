#pragma once

enum class AquariaState;
enum class DisplayStyle;
struct Readings;
class IDisplayDevice;

class IDisplayRenderer
{
  public:
    virtual ~IDisplayRenderer() = default;
    virtual void render(AquariaState state, const Readings& readings, IDisplayDevice& device) const = 0;
};
