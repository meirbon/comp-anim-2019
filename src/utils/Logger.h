#pragma once

#include <cstdarg>
#include <iostream>
#include <cstring>

#include <vector>

/**
 * Logging functions; mostly for debug purposes
 */
namespace utils
{
namespace logger
{
static void debug(const char *format, ...)
{
	std::vector<char> buffer(4096, 0);
	memset(buffer.data(), 0, sizeof(char) * buffer.size());
	va_list arg;
	va_start(arg, format);
	vsprintf(buffer.data(), format, arg);
	va_end(arg);
	std::cout << "DEBUG: " << buffer.data() << std::endl;
}

static void debug(const char *context, const char *format, ...)
{
	std::vector<char> buffer(4096, 0);
	memset(buffer.data(), 0, sizeof(char) * buffer.size());
	va_list arg;
	va_start(arg, format);
	vsprintf(buffer.data(), format, arg);
	va_end(arg);
	std::cout << "DEBUG: " << context << ": " << buffer.data() << std::endl;
}

static void debug(const char *file, int line, const char *format, ...)
{
	std::vector<char> buffer(4096, 0);
	memset(buffer.data(), 0, sizeof(char) * buffer.size());
	va_list arg;
	va_start(arg, format);
	vsprintf(buffer.data(), format, arg);
	va_end(arg);
	std::cout << "DEBUG: " << file << ":" << line << ": " << buffer.data() << std::endl;
}

static void warning(const char *format, ...)
{
	std::vector<char> buffer(4096, 0);
	memset(buffer.data(), 0, sizeof(char) * buffer.size());
	va_list arg;
	va_start(arg, format);
	vsprintf(buffer.data(), format, arg);
	va_end(arg);
	std::cerr << "WARNING: " << buffer.data() << std::endl;
}

static void warning(const char *context, const char *format, ...)
{
	std::vector<char> buffer(4096, 0);
	memset(buffer.data(), 0, sizeof(char) * buffer.size());
	va_list arg;
	va_start(arg, format);
	vsprintf(buffer.data(), format, arg);
	va_end(arg);
	std::cerr << "WARNING: " << context << ": " << buffer.data() << std::endl;
}

static void warning(const char *file, int line, const char *format, ...)
{
	std::vector<char> buffer(4096, 0);
	memset(buffer.data(), 0, sizeof(char) * buffer.size());
	va_list arg;
	va_start(arg, format);
	vsprintf(buffer.data(), format, arg);
	va_end(arg);
	std::cerr << "WARNING: " << file << ":" << line << ": " << buffer.data() << std::endl;
}

static void log(const char *format, ...)
{
	std::vector<char> buffer(4096, 0);
	memset(buffer.data(), 0, sizeof(char) * buffer.size());
	va_list arg;
	va_start(arg, format);
	vsprintf(buffer.data(), format, arg);
	va_end(arg);
	std::cout << "LOG: " << buffer.data() << std::endl;
}

static void log(const char *context, const char *format, ...)
{
	std::vector<char> buffer(4096, 0);
	memset(buffer.data(), 0, sizeof(char) * buffer.size());
	va_list arg;
	va_start(arg, format);
	vsprintf(buffer.data(), format, arg);
	va_end(arg);
	std::cout << context << ": " << buffer.data() << std::endl;
}

static void err(const char *format, ...)
{
	std::vector<char> buffer(4096, 0);
	memset(buffer.data(), 0, sizeof(char) * buffer.size());
	va_list arg;
	va_start(arg, format);
	vsprintf(buffer.data(), format, arg);
	va_end(arg);
	std::cerr << "ERROR: " << buffer.data() << std::endl;
	exit(EXIT_FAILURE);
}

static void err(const char *context, const char *format, ...)
{
	std::vector<char> buffer(4096, 0);
	memset(buffer.data(), 0, sizeof(char) * buffer.size());
	va_list arg;
	va_start(arg, format);
	vsprintf(buffer.data(), format, arg);
	va_end(arg);
	std::cerr << context << ": " << buffer.data() << std::endl;
	exit(EXIT_FAILURE);
}

static void err(const char *file, int line, const char *format, ...)
{
	std::vector<char> buffer(4096, 0);
	memset(buffer.data(), 0, sizeof(char) * buffer.size());
	va_list arg;
	va_start(arg, format);
	vsprintf(buffer.data(), format, arg);
	va_end(arg);
	std::cout << "ERROR: " << file << ":" << line << ": " << buffer.data() << std::endl;
	exit(EXIT_FAILURE);
}
} // namespace logger
} // namespace utils
/**
 * Defines a DEBUG macro to log only when the application is build in debug mode.
 */
#ifndef NDEBUG
#define DEBUG(format, ...) utils::logger::debug(__FILE__, __LINE__, format, ##__VA_ARGS__)
#else
#define DEBUG(format, ...)
#endif

#define WARNING(format, ...) utils::logger::warning(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define ERROR(format, ...) utils::logger::err(__FILE__, __LINE__, format, ##__VA_ARGS__)
