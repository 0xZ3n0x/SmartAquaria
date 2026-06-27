#pragma once

#include "IDisplayRenderer.h"

class DisplayRenderer final : public IDisplayRenderer
{
  public:
    void render(AquariaState state, const Readings& readings, IDisplayDevice& device) const override;

  private:
    static DisplayStyle toStyle(AquariaState state);
};
