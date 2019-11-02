#pragma once


#include <type_traits>

#define CREATE_CHECK_CLASS(METHOD_NAME) \
template<typename, typename T>\
struct has_##METHOD_NAME {\
    static_assert(\
        std::integral_constant<T, false>::value,\
        "Second template parameter needs to be of function type.");\
};\
\
template<typename C, typename Ret, typename... Args>\
struct has_##METHOD_NAME<C, Ret(Args...)> {\
private:\
    template<typename T>\
    static constexpr auto check(T*)\
        -> typename\
        std::is_same<\
        decltype(std::declval<T>().##METHOD_NAME(std::declval<Args>()...)),\
        Ret\
        >::type;\
\
    template<typename>\
    static constexpr std::false_type check(...);\
\
    using type = decltype(check<C>(nullptr));\
\
public:\
    static constexpr bool value = type::value;\
};

