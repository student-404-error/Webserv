#ifndef CONFIGEXCEPTION_HPP
#define CONFIGEXCEPTION_HPP

#include <stdexcept>
#include <string>

class ConfigException : public std::runtime_error
{
	public:
		explicit ConfigException(const std::string &msg) : std::runtime_error(msg) {}
};

class ConfigFileException : public ConfigException
{
	public:
		explicit ConfigFileException(const std::string &msg) : ConfigException(msg) {}
};

class ConfigSyntaxException : public ConfigException
{
	public:
		explicit ConfigSyntaxException(const std::string &msg) : ConfigException(msg) {}
};

class ConfigSemanticException : public ConfigException
{
	public:
		explicit ConfigSemanticException(const std::string &msg) : ConfigException(msg) {}
};

#endif

