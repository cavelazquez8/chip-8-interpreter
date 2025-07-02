#ifndef CHIP8_VERSION_H
#define CHIP8_VERSION_H

#include <string_view>

namespace Chip8Version {
    constexpr int VERSION_MAJOR = 1;
    constexpr int VERSION_MINOR = 2;
    constexpr int VERSION_PATCH = 0;
    
    constexpr std::string_view VERSION_STRING = "1.2.0";
    constexpr std::string_view BUILD_DATE = __DATE__;
    constexpr std::string_view BUILD_TIME = __TIME__;
    
    constexpr std::string_view APPLICATION_NAME = "CHIP-8 Interpreter";
    constexpr std::string_view APPLICATION_DESCRIPTION = "Professional CHIP-8 Emulator with GUI";
    constexpr std::string_view COPYRIGHT = "Copyright (c) 2024";
}

#endif // CHIP8_VERSION_H