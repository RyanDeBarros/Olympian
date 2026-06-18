#pragma once

#if defined(_WIN32)
#define OLY_OS_WINDOWS 1
#elif defined(__APPLE__)
#define OLY_OS_MAC 1
#elif defined(__linux__)
#define OLY_OS_LINUX 1
#else
#error "Unsupported platform"
#endif
