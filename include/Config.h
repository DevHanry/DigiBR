#pragma once
#include <Arduino.h>

// ============================================================
//  PINOS DOS BOTÕES (ESP32-C3)
// ============================================================
// A = cicla ícones | B = confirma | C = cancela / volta
constexpr uint8_t PIN_BTN_A = 0;
constexpr uint8_t PIN_BTN_B = 1;
constexpr uint8_t PIN_BTN_C = 5; // GPIO 5 (segura para o boot do ESP32-C3)

constexpr uint32_t DEBOUNCE_MS   = 35;   // tempo de debounce
constexpr uint32_t LONG_PRESS_MS = 800;  // pressão longa (ex: desligar)

// ============================================================
//  TELA (ST7735 128x160)
// ============================================================
constexpr int16_t SCREEN_W = 128;
constexpr int16_t SCREEN_H = 160;

// ============================================================
//  LAYOUT DO MENU
// ============================================================
enum MenuIcon : uint8_t {
    ICON_FEED = 0,    // Alimentação
    ICON_TRAIN,       // Treinamento
    ICON_BATTLE,      // Batalha
    ICON_CLEAN,       // Limpeza
    ICON_STATUS,      // Status
    ICON_SLEEP,       // Sono
    ICON_BOOK,        // Livro / Agenda
    ICON_CONNECT,     // Conexão
    ICON_COUNT
};

constexpr uint8_t ICONS_PER_ROW = 4;
constexpr uint8_t ICON_SIZE     = 16;
constexpr uint8_t ICON_MARGIN   = 4;

constexpr int16_t ROW_TOP_Y    = 4;
constexpr int16_t ROW_BOTTOM_Y = SCREEN_H - ICON_SIZE - 4;

constexpr int16_t PET_AREA_Y = ROW_TOP_Y + ICON_SIZE + 8;
constexpr int16_t PET_AREA_H = ROW_BOTTOM_Y - PET_AREA_Y - 8;

// ============================================================
//  CORES (RGB565)
// ============================================================
constexpr uint16_t COLOR_BG          = 0x0000; // Preto
constexpr uint16_t COLOR_ICON        = 0xFFFF; // Branco
constexpr uint16_t COLOR_ICON_ACTIVE = 0x07E0; // Verde
constexpr uint16_t COLOR_CURSOR      = 0xFFE0; // Amarelo
constexpr uint16_t COLOR_TEXT        = 0xFFFF; // Branco

constexpr uint32_t CURSOR_BLINK_MS = 300;
constexpr const char* PREFS_NAMESPACE = "digipet";