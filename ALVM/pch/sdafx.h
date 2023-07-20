#ifndef ALVM_PCH_H
#define ALVM_PCH_H

// STL headers
#include <iostream>
#include <memory>
#include <cstdint>
#include <chrono>
#include <thread>
#include <fstream>
#include <algorithm>
#include <utility>
#include <functional>
#include <type_traits>
#include <cstring>
#include <mutex>
#include <cstdio>
#include <exception>
#include <regex>
#include <numeric>
#include <cmath>

// STL Containers
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <bitset>
#include <array>
#include <initializer_list>

// Platform specific
#ifdef __APPLE__
#include <unistd.h>
#else
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef CreateWindow
//#include <corecrt_io.h>
#else
#include <unistd.h>
#endif
#endif

#endif // ALVM_PCH_H
