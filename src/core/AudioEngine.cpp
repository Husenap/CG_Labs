#include "AudioEngine.hpp"

#define MINIAUDIO_IMPLEMENTATION

#include "miniaudio.h"

AudioEngine::AudioEngine() {
    ma_result result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        throw std::runtime_error("Failed to initialize audio engine!");
    }
}

AudioEngine::~AudioEngine() {
    ma_engine_uninit(&engine);
}

void AudioEngine::play(const std::string& path) {
    ma_result result = ma_engine_play_sound(&engine, path.c_str(), NULL);
    if(result != MA_SUCCESS){
        throw std::runtime_error(std::string("Failed to play audio: ") + path);
    }
}
