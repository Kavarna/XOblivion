#pragma once

#include "Oblivion.h"


namespace exceptions
{
	class Exception : public std::exception
	{
	protected:
		std::string m_data;
	public:
		virtual const char* what() const noexcept
		{
			return m_data.c_str();
		}
	};

	class InitializationException : public Exception
	{
	public:
		InitializationException(const char* message, int line, const char* file)
		{
			m_data = std::string("Error: \"") + std::string(message) + std::string("\" on line ") + std::to_string(line) +
				" in file " + std::string(file);
		}
	};

	class MemoryAllocationException : public Exception
	{
	public:
		MemoryAllocationException(int allocateSize, int line, const char* file)
		{
			m_data = std::string("Error: \"Couldn't allocate ") + std::to_string(allocateSize) +
				std::string("\" on line ") + std::to_string(line) +
				" in file" + std::string(file);
		}
	};
}

#define THROW_ERROR(...) {char message[1024];\
sprintf(message, __VA_ARGS__);\
std::runtime_error error(&message[0]);\
throw error;}
#define THROW_INITIALIZATION_EXCEPTION(...) {\
char message[1024];\
sprintf(message, __VA_ARGS__);\
throw exceptions::InitializationException(message, __LINE__, __FILE__)\
;}
#define THROW_ALLOCATION_EXCEPTION(size) {\
throw exceptions::MemoryAllocationException(size, __LINE__, __FILE__);\
}

#define EVALUATE(expression, expected_result, op, ...) { auto value = (expression); if ( value op decltype(value)(expected_result) ) THROW_ERROR(__VA_ARGS__); }
#if DEBUG || _DEBUG
#define EVALUATE_DEBUG(expression, expected_result, op, ...) if ( (expression) op (expected_result)) THROW_ERROR(__VA_ARGS__);
#else
#define EVALUATE_DEBUG(expression, expected_result, op, ...)
#endif
