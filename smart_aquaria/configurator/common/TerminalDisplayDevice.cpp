#include "TerminalDisplayDevice.h"

#include <stdexcept>

TerminalDisplayDevice::TerminalDisplayDevice(const char* filename)
    : m_file(filename, std::ios::out | std::ios::trunc)
{
    if (!m_file.is_open())
    {
        throw std::runtime_error(std::string("Failed to open display log: ") + filename);
    }
}

void TerminalDisplayDevice::clear()
{
    m_file << '\n' << std::flush;
}

void TerminalDisplayDevice::print(const char* text)
{
    m_file << text << '\n';
}

void TerminalDisplayDevice::flush()
{
    m_file.flush();
}

void TerminalDisplayDevice::printStyled(const char* text, DisplayStyle style)
{
    const char* code = "";
    switch (style)
    {
    case DisplayStyle::Bold:   code = "\033[1m";    break;
    case DisplayStyle::Dim:    code = "\033[2m";    break;
    case DisplayStyle::Red:    code = "\033[1;31m"; break;
    case DisplayStyle::Yellow: code = "\033[1;33m"; break;
    case DisplayStyle::Green:  code = "\033[1;32m"; break;
    case DisplayStyle::Cyan:   code = "\033[1;36m"; break;
    default: break;
    }
    m_file << code << text << "\033[0m\n";
}
