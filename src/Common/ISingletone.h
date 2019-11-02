#pragma once

#include <memory>
#include <mutex>
#include "CountParameters.h"

template <typename type>
class ISingletone
{
public:
    ISingletone() {};
    virtual ~ISingletone() { reset(); };

public:
    template <typename ...Args>
    static type* Get(Args ... args)
    {
        if constexpr (countParameters<Args...>::value == 0)
        {
            if constexpr (std::is_default_constructible<type>::value)
            {
                std::call_once(m_singletoneFlags, [&]
                {
                    m_singletoneInstance = new type(args...);
                });
                return m_singletoneInstance;
            }
            else
                return m_singletoneInstance;
        }
        else
        {
            std::call_once(m_singletoneFlags, [&]
            {
                m_singletoneInstance = new type(args...);
            });
            return m_singletoneInstance;
        }
    }

    static void reset()
    {
        if (m_singletoneInstance)
        {
            auto ptr = m_singletoneInstance; // If we do it this way, we can safely call reset() from destructor too
            m_singletoneInstance = nullptr;
            delete ptr;
        }
    };

private:
    static type* m_singletoneInstance;
    static std::once_flag m_singletoneFlags;
};

template <typename type>
type* ISingletone<type>::m_singletoneInstance = nullptr;
template <typename type>
std::once_flag ISingletone<type>::m_singletoneFlags;