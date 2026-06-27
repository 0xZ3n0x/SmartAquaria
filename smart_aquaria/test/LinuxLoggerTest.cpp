#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "Logger.h"

static std::string readFile(const std::string& path)
{
    std::ifstream      f(path);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

class LoggerTest : public ::testing::Test
{
  protected:
    const std::string testFile = "test_logger.log";

    void TearDown() override { std::filesystem::remove(testFile); }
};

TEST_F(LoggerTest, ThrowsOnInvalidPath)
{
    EXPECT_THROW(Logger("/nonexistent/path/test.log"), std::runtime_error);
}

TEST_F(LoggerTest, LogPrefixMessageWritesCorrectFormat)
{
    Logger logger(testFile.c_str());

    EXPECT_TRUE(logger.log("INFO", "something happened"));

    const auto content = readFile(testFile);
    EXPECT_EQ(content, "INFO: something happened\n");
}

TEST_F(LoggerTest, MultipleLogsAppendInOrder)
{
    Logger logger(testFile.c_str());

    logger.log("INFO", "first");
    logger.log("INFO", "second");
    logger.log("INFO", "third");

    const auto content = readFile(testFile);
    EXPECT_EQ(content, "INFO: first\nINFO: second\nINFO: third\n");
}

TEST_F(LoggerTest, TruncatesFileOnReopen)
{
    {
        Logger logger(testFile.c_str());
        logger.log("INFO", "old content");
    }

    {
        Logger logger(testFile.c_str());
        logger.log("INFO", "new content");
    }

    const auto content = readFile(testFile);
    EXPECT_EQ(content, "INFO: new content\n");
}
