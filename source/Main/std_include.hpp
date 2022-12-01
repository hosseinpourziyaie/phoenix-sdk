#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <Psapi.h>
#include <ShlObj.h>

#include <map>
#include <queue>
#include <vector>
#include <fstream>
#include <functional>

#include <MinHook.h>

#pragma warning(pop)
#pragma warning(disable: 4100)

using namespace std::literals;

#define PHOENIX_VERSION_STR "1.0.0 BETA"
#include <Loader/component_loader.hpp>
#include <Developer/debug_helper.hpp>
#include <Utilities/common.hpp>
#include <Utilities/string.hpp>

uintptr_t operator"" _b(const uintptr_t ptr);


#define log_debug_string(fmt, ...) diagnostic::write(0, fmt, ##__VA_ARGS__)
#define log_warn_string(fmt, ...)  diagnostic::write(2, fmt, ##__VA_ARGS__)
#define log_info_string(fmt, ...)  diagnostic::write(1, fmt, ##__VA_ARGS__)
#define log_error_string(fmt, ...) diagnostic::write(3, fmt, ##__VA_ARGS__)