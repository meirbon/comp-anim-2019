#pragma once

#include <fstream>
#include <iostream>
#include <string>

/**
 * File helper functions
 */
namespace utils
{
namespace file
{

static std::string read(const std::string &path)
{
	std::ifstream file(path.c_str());
	std::string line;
	std::string text;

	if (!file.is_open())
		std::cout << path << " not found" << std::endl;

	while (std::getline(file, line))
		text += line + "\n";

	return text;
}

static bool exists(const std::string &path)
{
	std::ifstream file(path.c_str());
	return file.is_open();
}

} // namespace file
} // namespace utils