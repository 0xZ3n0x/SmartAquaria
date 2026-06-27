#include "DisplayAdapter.h"

#include "device/display/IDisplayDevice.h"
#include "renderer/IDisplayRenderer.h"

DisplayAdapter::DisplayAdapter(std::unique_ptr<IDisplayRenderer> renderer,
                               std::unique_ptr<IDisplayDevice>   display)
    : m_renderer(std::move(renderer)), m_display(std::move(display))
{
}

DisplayAdapter::~DisplayAdapter() = default;

void DisplayAdapter::display(AquariaState state, const Readings& readings)
{
    if (m_renderer == nullptr || m_display == nullptr)
    {
        return;
    }
        
    m_renderer->render(state, readings, *m_display);
}
