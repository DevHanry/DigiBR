#pragma once
#include <Arduino.h>
#include "Config.h"

// ------------------------------------------------------------
// Estado lógico de um botão físico, já com debounce e detecção
// de "short press" (evento único) e "long press" (mantido).
// ------------------------------------------------------------
class Button {
public:
    void begin(uint8_t pin) {
        _pin = pin;
        pinMode(_pin, INPUT_PULLUP);   // nível alto = solto, baixo = pressionado
        _stableState   = HIGH;
        _lastRawState  = HIGH;
        _lastChangeMs  = 0;
        _pressStartMs  = 0;
        _longFired     = false;
    }

    // Chamar a cada loop(). Atualiza o estado interno.
    void update() {
        bool raw = digitalRead(_pin);
        uint32_t now = millis();

        _justPressed  = false;
        _justReleased = false;

        if (raw != _lastRawState) {
            _lastChangeMs = now;
            _lastRawState = raw;
        }

        // Só aceita a transição depois do tempo de debounce estabilizado
        if ((now - _lastChangeMs) > DEBOUNCE_MS && raw != _stableState) {
            _stableState = raw;

            if (_stableState == LOW) {          // pressionou agora
                _justPressed  = true;
                _pressStartMs = now;
                _longFired    = false;
            } else {                            // soltou agora
                _justReleased = true;
            }
        }

        // Long-press: dispara uma única vez enquanto mantém pressionado
        if (_stableState == LOW && !_longFired &&
            (now - _pressStartMs) >= LONG_PRESS_MS) {
            _longFired = true;
            _justLongPress = true;
        } else {
            _justLongPress = false;
        }
    }

    bool isPressed()      const { return _stableState == LOW; }
    bool wasPressed()     const { return _justPressed; }   // borda de descida (evento único)
    bool wasReleased()    const { return _justReleased; }  // borda de subida
    bool wasLongPressed() const { return _justLongPress; } // pressão longa (evento único)

private:
    uint8_t  _pin = 0;
    bool     _stableState  = HIGH;
    bool     _lastRawState = HIGH;
    uint32_t _lastChangeMs = 0;
    uint32_t _pressStartMs = 0;
    bool     _longFired     = false;

    bool _justPressed   = false;
    bool _justReleased  = false;
    bool _justLongPress = false;
};

// ------------------------------------------------------------
// Agrupa os 3 botões do brinquedo (A / B / C) num só objeto,
// com nomes semânticos para não confundir a lógica de navegação.
// ------------------------------------------------------------
class ButtonManager {
public:
    void begin() {
        _btnA.begin(PIN_BTN_A);
        _btnB.begin(PIN_BTN_B);
        _btnC.begin(PIN_BTN_C);
    }

    void update() {
        _btnA.update();
        _btnB.update();
        _btnC.update();
    }

    // A = cicla ícones do menu
    bool cyclePressed()   const { return _btnA.wasPressed(); }
    // B = confirma ação
    bool confirmPressed() const { return _btnB.wasPressed(); }
    // C = cancela / volta
    bool cancelPressed()  const { return _btnC.wasPressed(); }

    // C mantido pressionado = desligar / reset (exemplo de uso de long-press)
    bool powerOffHeld()   const { return _btnC.wasLongPressed(); }

    const Button& raw(char which) const {
        switch (which) {
            case 'A': return _btnA;
            case 'B': return _btnB;
            default:  return _btnC;
        }
    }

private:
    Button _btnA, _btnB, _btnC;
};
