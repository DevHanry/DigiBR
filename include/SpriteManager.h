#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Config.h"

constexpr uint16_t PET_SPRITE_W = 32;
constexpr uint16_t PET_SPRITE_H = 32;
constexpr uint8_t  PET_IDLE_FRAME_COUNT = 2;

// Aponta para as variáveis criadas no SpriteData.cpp (sem inline e sem { ... })
extern const uint16_t PET_IDLE_FRAME_0[PET_SPRITE_W * PET_SPRITE_H] PROGMEM;
extern const uint16_t PET_IDLE_FRAME_1[PET_SPRITE_W * PET_SPRITE_H] PROGMEM;

class SpriteManager {
public:
    void begin() {
        _frames[0] = PET_IDLE_FRAME_0;
        _frames[1] = PET_IDLE_FRAME_1;
        _currentFrame = 0;
        _lastFrameSwapMs = millis();
    }

    void update(uint32_t nowMs) {
        if (nowMs - _lastFrameSwapMs >= IDLE_FRAME_INTERVAL_MS) {
            _lastFrameSwapMs = nowMs;
            _currentFrame = (_currentFrame + 1) % PET_IDLE_FRAME_COUNT;
        }
    }

    void draw(TFT_eSprite &sprite) {
        int16_t x = (SCREEN_W - PET_SPRITE_W) / 2;
        int16_t y = PET_AREA_Y + (PET_AREA_H - PET_SPRITE_H) / 2;

        sprite.setSwapBytes(true);
        // O const_cast remove o 'const' exigido pela TFT_eSPI
        sprite.pushImage(x, y, PET_SPRITE_W, PET_SPRITE_H, const_cast<uint16_t*>(_frames[_currentFrame]), COLOR_BG);
        sprite.setSwapBytes(false);
    }

private:
    const uint16_t* _frames[PET_IDLE_FRAME_COUNT];
    uint8_t  _currentFrame = 0;
    uint32_t _lastFrameSwapMs = 0;

    static constexpr uint32_t IDLE_FRAME_INTERVAL_MS = 500;
};