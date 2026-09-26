#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Config.h"

// ------------------------------------------------------------
// Controla a posição do cursor sobre a grade de ícones (2 linhas
// x 4 colunas), o estado de "ativo/pendente" de cada ícone, e
// desenha tudo isso no TFT_eSprite (buffer off-screen) passado
// pelo loop principal — nunca desenha direto no display, para
// não piscar a tela.
// ------------------------------------------------------------
class MenuManager {
public:
    void begin() {
        _cursorIndex = ICON_FEED;
        for (uint8_t i = 0; i < ICON_COUNT; i++) _iconActive[i] = false;
    }

    // Botão A: avança o cursor para o próximo ícone (cíclico)
    void cycleNext() {
        _cursorIndex = (_cursorIndex + 1) % ICON_COUNT;
    }

    uint8_t selected() const { return _cursorIndex; }

    // Marca um ícone como "pendente de atenção" (ex: fome baixa
    // pisca o ícone de Alimentação em verde, como no original)
    void setActive(uint8_t icon, bool active) {
        if (icon < ICON_COUNT) _iconActive[icon] = active;
    }

    // Calcula o retângulo (x, y) de um ícone dado seu índice 0..7
    static void iconRect(uint8_t index, int16_t &x, int16_t &y) {
        uint8_t col = index % ICONS_PER_ROW;
        uint8_t row = index / ICONS_PER_ROW; // 0 = linha de cima, 1 = linha de baixo

        int16_t totalRowW = ICONS_PER_ROW * ICON_SIZE + (ICONS_PER_ROW - 1) * ICON_MARGIN;
        int16_t startX = (SCREEN_W - totalRowW) / 2;

        x = startX + col * (ICON_SIZE + ICON_MARGIN);
        y = (row == 0) ? ROW_TOP_Y : ROW_BOTTOM_Y;
    }

    // Desenha as duas fileiras de ícones + moldura do cursor
    // piscando sobre o ícone selecionado. `sprite` é o buffer
    // off-screen (TFT_eSprite) — chamada dentro do render loop.
    void draw(TFT_eSprite &sprite, uint32_t nowMs) {
        for (uint8_t i = 0; i < ICON_COUNT; i++) {
            int16_t x, y;
            iconRect(i, x, y);
            drawIconPlaceholder(sprite, i, x, y);
        }

        // Cursor piscante: liga/desliga a cada CURSOR_BLINK_MS
        bool blinkOn = (nowMs / CURSOR_BLINK_MS) % 2 == 0;
        if (blinkOn) {
            int16_t x, y;
            iconRect(_cursorIndex, x, y);
            sprite.drawRect(x - 2, y - 2, ICON_SIZE + 4, ICON_SIZE + 4, COLOR_CURSOR);
            sprite.drawRect(x - 1, y - 1, ICON_SIZE + 2, ICON_SIZE + 2, COLOR_CURSOR);
        }
    }

private:
    uint8_t _cursorIndex = ICON_FEED;
    bool    _iconActive[ICON_COUNT];

    // Placeholder simples (retângulo + símbolo). Troque pelo seu
    // sprite RGB565 de cada ícone quando tiver os assets prontos
    // (ver SpriteManager / bitmaps de ícone).
    void drawIconPlaceholder(TFT_eSprite &sprite, uint8_t index, int16_t x, int16_t y) {
        uint16_t color = _iconActive[index] ? COLOR_ICON_ACTIVE : COLOR_ICON;
        sprite.drawRect(x, y, ICON_SIZE, ICON_SIZE, color);

        // Pequeno glifo distinto por ícone (placeholder textual até
        // ter os bitmaps reais — facilita depurar a navegação).
        static const char* labels[ICON_COUNT] = {
            "Fd", "Tr", "Bt", "Cl", "St", "Sl", "Bk", "Cn"
        };
        sprite.setTextColor(color, COLOR_BG);
        sprite.setTextSize(1);
        sprite.setCursor(x + 1, y + 4);
        sprite.print(labels[index]);
    }
};
