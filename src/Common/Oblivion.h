#pragma once


#include <exception>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <chrono>
#include <functional>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <optional>
#include <thread>
#include <memory>
#include <cstring>
#include <type_traits>
#include <stdexcept>

#if defined DEBUG || _DEBUG
#include <iostream>
#else
#endif

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE


#include "platform.h"
#include "exceptions.h"
#include "ISingletone.h"


#define ARRAYSIZE(a) ((sizeof(a) / sizeof(*(a))) / static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

#define CHECK_IF_VALUE_IN_ARRAY_COMPLEX(value, arr, sufix) [&]{\
for (const auto& it : arr)\
{\
    if (it.sufix == value)\
        return true;\
}\
return false;\
}()
#define CHECK_IF_STR_IN_ARRAY_COMPLEX(value, arr, sufix) [&]{\
for (const auto& _it : arr)\
{\
    if (!strcmp(value, _it.sufix))\
        return true;\
}\
return false;\
}()


constexpr const char* APPLICATION_NAME = "Gaem";
constexpr const char* ENGINE_NAME = "Oblivion";

enum class LogType
{
    NOTE,
    WARNING,
    ERROR
};

template <class type, class arraytype>
bool valueInArray(type value, arraytype arr)
{
    for (const auto& it : arr)
    {
        if (value == it)
            return true;
    }
    return false;
}

template <class arraytype>
bool valueInArray(char value, arraytype arr)
{
    for (const auto& it : arr)
    {
        if (!strcmp(value, it))
            return true;
    }
    return false;
}


template <typename ... Args>
constexpr auto appendToString() -> std::string
{
    return "";
}

template <typename type>
constexpr auto appendToString(type arg) -> std::string
{
    std::ostringstream stream;
    stream << arg;
    return stream.str();
}

template <typename type, typename... Args>
constexpr auto appendToString(type A, Args... args) -> std::string
{
    std::string res1 = appendToString(A);
    std::string res2 = appendToString(args...);
    return res1 + res2;
}

template <typename type>
constexpr auto putSetInVector(const std::set<type>& set) -> std::vector<type>
{
    std::vector<type> ret;

    for (const auto& e : set)
        ret.push_back(e);

    return ret;
}


namespace Logger
{
    void addLog(LogType, const std::string&);
    void dumpJson(std::ostream& stream);

    template <typename ... Args>
    constexpr auto printToConsole()
    { }

    template <typename type>
    constexpr auto printToConsole(type arg)
    {
#if DEBUG || _DEBUG
        std::cout << arg;
#endif
    }

    template <typename type, typename... Args>
    constexpr auto printToConsole(type A, Args... args)
    {
#if DEBUG || _DEBUG
        std::cout << A;
        printToConsole(args...);
#endif
    }

    template <bool flush, typename type, typename... Args>
    constexpr auto printToConsole(type A, Args... args)
    {
#if DEBUG || _DEBUG
        std::cout << A;
        printToConsole(args...);
        if constexpr (flush)
        {
            std::cout.flush();
        }
#endif
    }

}


template <class string>
constexpr void NOTE(string msg)
{
    Logger::addLog(LogType::NOTE, msg);
#if DEBUG || _DEBUG
    std::cout << msg << std::endl;
#endif
}

template <class string>
constexpr void WARNING(string msg)
{
    Logger::addLog(LogType::WARNING, msg);
#if DEBUG || _DEBUG
    std::cout << msg << std::endl;
#endif
}

template <class string>
constexpr void ERROR(string msg)
{
    Logger::addLog(LogType::ERROR, msg);
#if DEBUG || _DEBUG
    std::cout << msg << std::endl;
#endif
}
