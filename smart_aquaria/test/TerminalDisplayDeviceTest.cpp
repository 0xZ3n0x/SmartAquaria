#include "TerminalDisplayDevice.h"

#include <fstream>
#include <sstream>

#include <gtest/gtest.h>

TEST(TerminalDisplayDeviceTest, PrintWritesToFile)
{
    TerminalDisplayDevice display("/tmp/test_display.log");

    display.print("hello");
    display.flush();

    std::ifstream     file("/tmp/test_display.log");
    std::stringstream buffer;
    buffer << file.rdbuf();

    EXPECT_NE(buffer.str().find("hello"), std::string::npos);
}

TEST(TerminalDisplayDeviceTest, ClearWritesSeparator)
{
    TerminalDisplayDevice display("/tmp/test_display2.log");

    display.clear();

    std::ifstream     file("/tmp/test_display2.log");
    std::stringstream buffer;
    buffer << file.rdbuf();

    EXPECT_FALSE(buffer.str().empty());
}
