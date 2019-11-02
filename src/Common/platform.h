#pragma once

#ifdef _WIN32
#define _WINDOWS_
#ifdef _WIN64
#define _WINDOWS64_
#else
#define _WINDOWS32_
#endif

#elif __APPLE__
#define _APPLE_
#elif __linux__
#define _LINUX_
#elif __unix__
#define _UNIX_
#else
#error "Not platform ready"
#endif