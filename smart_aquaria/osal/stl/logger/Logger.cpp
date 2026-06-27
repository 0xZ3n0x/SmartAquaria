#include "Logger.h"

#include <stdexcept>

#include <fcntl.h>
#include <unistd.h>

Logger::Logger(const char* filename)
    : m_fd(::open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644))
{
    if (m_fd < 0)
    {
        throw std::runtime_error(std::string("Failed to open log file: ") + filename);
    }
}

Logger::~Logger()
{
    if (m_fd >= 0)
    {
        ::close(m_fd);
    }
}

bool Logger::log(const char* prefix, const char* message)
{
    if (m_fd < 0)
    {
        return false;
    }
    std::string line{prefix};
    line += ": ";
    line += message;
    line += '\n';
    return ::write(m_fd, line.c_str(), line.size()) == static_cast<ssize_t>(line.size());
}
