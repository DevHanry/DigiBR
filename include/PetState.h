#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "Config.h"

enum class EvoStage : uint8_t {
    EGG = 0,
    BABY,
    IN_TRAINING,
    ROOKIE,
    CHAMPION,
    ULTIMATE,
    MEGA
};

struct PetData {
    uint8_t  hunger        = 4;   // 0..4 corações
    uint8_t  strength      = 4;   // 0..4 corações
    uint16_t weight        = 10;  // peso
    uint32_t ageSeconds    = 0;   // idade acumulada em segundos
    uint8_t  careMistakes  = 0;   // erros de cuidado
    uint16_t trainingWins  = 0;   // vitórias
    EvoStage stage         = EvoStage::EGG;
    bool     isSleeping    = false;
    bool     isSick        = false;
};

class PetManager {
public:
    void begin() {
        load();
        _lastTickMs = millis();
        _lastAutoSaveMs = millis();
    }

    void update() {
        uint32_t now = millis();

        if (now - _lastTickMs >= 1000) {
            _lastTickMs += 1000;
            tickOneSecond();
        }

        if (now - _lastAutoSaveMs >= AUTO_SAVE_INTERVAL_MS) {
            _lastAutoSaveMs = now;
            save();
        }
    }

    void feed() {
        if (_data.hunger < 4) _data.hunger++;
        markDirty();
    }

    void train() {
        if (_data.strength < 4) {
            _data.strength++;
            _data.trainingWins++;
        }
        markDirty();
    }

    void clean() {
        _data.isSick = false;
        markDirty();
    }

    void toggleSleep() {
        _data.isSleeping = !_data.isSleeping;
        markDirty();
    }

    bool resolveBattle(uint8_t opponentPower) {
        uint8_t myPower = _data.strength * 20 + (_data.weight % 40);
        bool win = myPower >= opponentPower;

        if (win) {
            if (_data.trainingWins < 65535) _data.trainingWins++;
            if (_data.weight < 999) _data.weight++;
        } else {
            if (_data.strength > 0) _data.strength--;
        }

        markDirty();
        checkEvolution();
        return win;
    }

    void saveNow() { save(); }

    const PetData& data() const { return _data; }
    bool needsAttention() const {
        return _data.hunger == 0 || _data.strength == 0 || _data.isSick;
    }

private:
    PetData     _data;
    Preferences _prefs;
    uint32_t    _lastTickMs = 0;
    uint32_t    _lastAutoSaveMs = 0;
    uint32_t    _neglectSeconds = 0; // Guardado na classe para poder reiniciar
    bool        _dirty = false;

    static constexpr uint32_t AUTO_SAVE_INTERVAL_MS = 30000;

    void markDirty() { _dirty = true; }

    void tickOneSecond() {
        _data.ageSeconds++;

        constexpr uint32_t DEGRADE_INTERVAL_S = 300;
        if (_data.ageSeconds % DEGRADE_INTERVAL_S == 0) {
            if (_data.hunger > 0)   _data.hunger--;
            if (_data.strength > 0) _data.strength--;
        }

        // Lógica de erros de cuidado com reset correto
        if (needsAttention()) {
            _neglectSeconds++;
            if (_neglectSeconds >= 600) { // 10 min sem atender
                _data.careMistakes++;
                _neglectSeconds = 0;
                markDirty();
            }
        } else {
            _neglectSeconds = 0; // Reinicia o contador se o pet foi atendido
        }

        checkEvolution();
    }

    void checkEvolution() {
        struct EvoRule { EvoStage stage; uint32_t minAgeS; uint16_t minWins; };
        static const EvoRule rules[] = {
            { EvoStage::BABY,        60,     0 },
            { EvoStage::IN_TRAINING, 3600,   0 },
            { EvoStage::ROOKIE,      21600,  3 },
            { EvoStage::CHAMPION,    86400,  10 },
            { EvoStage::ULTIMATE,    172800, 25 },
            { EvoStage::MEGA,        345600, 50 },
        };

        for (auto &r : rules) {
            if (_data.ageSeconds >= r.minAgeS &&
                _data.trainingWins >= r.minWins &&
                (uint8_t)r.stage > (uint8_t)_data.stage) {
                _data.stage = r.stage;
                markDirty();
            }
        }
    }

    void load() {
        _prefs.begin(PREFS_NAMESPACE, false);
        _data.hunger       = _prefs.getUChar("hunger", 4);
        _data.strength     = _prefs.getUChar("strength", 4);
        _data.weight       = _prefs.getUShort("weight", 10);
        _data.ageSeconds   = _prefs.getUInt("age", 0);
        _data.careMistakes = _prefs.getUChar("mistakes", 0);
        _data.trainingWins = _prefs.getUShort("wins", 0);
        _data.stage        = (EvoStage)_prefs.getUChar("stage", (uint8_t)EvoStage::EGG);
        _data.isSleeping   = _prefs.getBool("sleeping", false);
        _data.isSick       = _prefs.getBool("sick", false);
        _prefs.end();
    }

    void save() {
        if (!_dirty) return;
        _prefs.begin(PREFS_NAMESPACE, false);
        _prefs.putUChar("hunger", _data.hunger);
        _prefs.putUChar("strength", _data.strength);
        _prefs.putUShort("weight", _data.weight);
        _prefs.putUInt("age", _data.ageSeconds);
        _prefs.putUChar("mistakes", _data.careMistakes);
        _prefs.putUShort("wins", _data.trainingWins);
        _prefs.putUChar("stage", (uint8_t)_data.stage);
        _prefs.putBool("sleeping", _data.isSleeping);
        _prefs.putBool("sick", _data.isSick);
        _prefs.end();
        _dirty = false;
    }
};