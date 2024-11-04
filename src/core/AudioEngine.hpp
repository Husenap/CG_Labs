#pragma once

#include <string>
#include <stdexcept>
#include "miniaudio.h"

class AudioEngine {
    ma_engine engine{};
public:
    AudioEngine();
    ~AudioEngine();
    void play(const std::string& path);
};
