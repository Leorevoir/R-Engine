#include "../Test.hpp"

#include <R-Engine/Core/Logger.hpp>

#include <iostream>
#include <sstream>
#include <string>

Test(Logger, test_logger_debug)
{
    std::stringstream buffer;
    std::streambuf *old_buf = std::cout.rdbuf(buffer.rdbuf());

    r::Logger::debug("Test debug message", "testfile.cpp", 42);

    std::cout.rdbuf(old_buf);

    std::string output = buffer.str();

    cr_assert(output.find("DEBUG") != std::string::npos, "Expected DEBUG level in output");
    cr_assert(output.find("Test debug message") != std::string::npos, "Expected message in output");
    cr_assert(output.find("testfile.cpp:42") != std::string::npos, "Expected file:line in output");
}

Test(Logger, test_logger_info)
{
    std::stringstream buffer;
    std::streambuf *old_buf = std::cout.rdbuf(buffer.rdbuf());

    r::Logger::info("Info message", "file.cpp", 123);

    std::cout.rdbuf(old_buf);

    std::string output = buffer.str();
    cr_assert(output.find("INFO") != std::string::npos);
    cr_assert(output.find("Info message") != std::string::npos);
    cr_assert(output.find("file.cpp:123") != std::string::npos);
}

Test(Logger, test_logger_warn)
{
    std::stringstream buffer;
    std::streambuf *old_buf = std::cout.rdbuf(buffer.rdbuf());

    r::Logger::warn("Warning message");

    std::cout.rdbuf(old_buf);

    std::string output = buffer.str();
    cr_assert(output.find("WARN") != std::string::npos);
    cr_assert(output.find("Warning message") != std::string::npos);
}

Test(Logger, test_logger_error)
{
    std::stringstream buffer;
    std::streambuf *old_buf = std::cout.rdbuf(buffer.rdbuf());

    r::Logger::error("Error message");

    std::cout.rdbuf(old_buf);

    std::string output = buffer.str();
    cr_assert(output.find("ERROR") != std::string::npos);
    cr_assert(output.find("Error message") != std::string::npos);
}
