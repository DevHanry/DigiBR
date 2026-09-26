#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Config.h"
#include "SpriteManager.h"
#include "ButtonManager.h"
#include "MenuManager.h"
#include "PetState.h"

// ------------------------------------------------------------
// Estados gerais da UI (bem mais simples que a máquina de
// estados do pet — isso aqui é só "onde estamos na navegação").
// ------------------------------------------------------------
enum class UiState {
    IDLE_MAIN,      // tela principal, cursor livre sobre os ícones
    ACTION_FEED,    // instantâneo
    ACTION_TRAIN,   // instantâneo
    ACTION_CLEAN,   // instantâneo
    ACTION_STATUS,  // tela dedicada, sai com B/C
    ACTION_SLEEP,   // alterna dormindo/acordado, sai com B/C
    ACTION_BATTLE,  // sub-máquina de estados própria (ver BattlePhase)
    ACTION_BOOK,    // tela dedicada, sai com B/C
    ACTION_CONNECT, // tela dedicada, sai com B/C ou timeout
};

// Sub-estados da tela de Batalha (bem mais simples que o combate
// real do Digimon Color — troque pela lógica de ataques/turnos
// original quando fizer a engenharia reversa dessa tela).
enum class BattlePhase {
    VS_SCREEN,   // mostra "VS oponente", aguarda B para atacar
    RESULT,      // mostra vitória/derrota, aguarda B/C para sair
};
BattlePhase battlePhase = BattlePhase::VS_SCREEN;
bool        lastBattleWon = false;
uint8_t     currentOpponentPower = 50;

// Timeout automático da tela de Conexão (simula "procurando outro
// aparelho..." e desiste depois de alguns segundos)
uint32_t connectStartMs = 0;
constexpr uint32_t CONNECT_TIMEOUT_MS = 4000;

TFT_eSPI    tft = TFT_eSPI();
TFT_eSprite frameBuffer = TFT_eSprite(&tft); // buffer off-screen p/ evitar flicker

ButtonManager buttons;
MenuManager   menu;
PetManager    pet;
SpriteManager petSprite;

UiState uiState = UiState::IDLE_MAIN;

// ------------------------------------------------------------
// Desenha o HUD de fome/força (corações) — chamada dentro do
// render principal, usando o mesmo buffer off-screen.
// ------------------------------------------------------------
void drawHearts(TFT_eSprite &sprite, int16_t x, int16_t y, uint8_t value, uint16_t color) {
    for (uint8_t i = 0; i < 4; i++) {
        uint16_t c = (i < value) ? color : TFT_DARKGREY;
        sprite.fillCircle(x + i * 8, y, 3, c);
    }
}

// ------------------------------------------------------------
// Tela principal (idle): ícones + pet + HUD de corações.
// ------------------------------------------------------------
void drawMainScreen(TFT_eSprite &sprite, uint32_t now) {
    menu.draw(sprite, now);
    petSprite.draw(sprite);
    drawHearts(sprite, 6, ROW_BOTTOM_Y - 10, pet.data().hunger, TFT_RED);
    drawHearts(sprite, SCREEN_W - 6 - 24, ROW_BOTTOM_Y - 10, pet.data().strength, TFT_YELLOW);
}

// ------------------------------------------------------------
// Tela de Status (Livro/Status): dados brutos do pet em texto.
// ------------------------------------------------------------
const char* stageName(EvoStage s) {
    switch (s) {
        case EvoStage::EGG:         return "Ovo";
        case EvoStage::BABY:        return "Baby";
        case EvoStage::IN_TRAINING: return "In-Training";
        case EvoStage::ROOKIE:      return "Rookie";
        case EvoStage::CHAMPION:    return "Champion";
        case EvoStage::ULTIMATE:    return "Ultimate";
        case EvoStage::MEGA:        return "Mega";
    }
    return "?";
}

void drawStatusScreen(TFT_eSprite &sprite) {
    const PetData &d = pet.data();
    sprite.setTextColor(COLOR_TEXT, COLOR_BG);
    sprite.setTextSize(1);

    int16_t y = 10;
    sprite.setCursor(6, y); sprite.print("STATUS"); y += 14;
    sprite.setCursor(6, y); sprite.print("Fase: ");   sprite.print(stageName(d.stage)); y += 12;
    sprite.setCursor(6, y); sprite.print("Idade(s): "); sprite.print(d.ageSeconds); y += 12;
    sprite.setCursor(6, y); sprite.print("Peso: ");   sprite.print(d.weight); y += 12;
    sprite.setCursor(6, y); sprite.print("Fome: ");   sprite.print(d.hunger); sprite.print("/4"); y += 12;
    sprite.setCursor(6, y); sprite.print("Forca: ");  sprite.print(d.strength); sprite.print("/4"); y += 12;
    sprite.setCursor(6, y); sprite.print("Vitorias: "); sprite.print(d.trainingWins); y += 12;
    sprite.setCursor(6, y); sprite.print("Erros cuid.: "); sprite.print(d.careMistakes); y += 12;
    sprite.setCursor(6, y); sprite.print(d.isSick ? "Doente!" : "Saudavel"); y += 16;

    sprite.setCursor(6, SCREEN_H - 12);
    sprite.print("B/C: voltar");
}

// ------------------------------------------------------------
// Tela de Sono: apaga a maior parte da UI (o pet "dorme"), só
// deixa um indicador. Sair alterna de volta e desliga o sono.
// ------------------------------------------------------------
void drawSleepScreen(TFT_eSprite &sprite) {
    sprite.setTextColor(COLOR_TEXT, COLOR_BG);
    sprite.setTextSize(1);
    sprite.setCursor(SCREEN_W / 2 - 12, SCREEN_H / 2 - 4);
    sprite.print("Zzz...");
    sprite.setCursor(6, SCREEN_H - 12);
    sprite.print("B/C: acordar");
}

// ------------------------------------------------------------
// Tela de Batalha: VS -> resultado. Sub-máquina própria via
// battlePhase, independente do uiState geral.
// ------------------------------------------------------------
void drawBattleScreen(TFT_eSprite &sprite) {
    sprite.setTextColor(COLOR_TEXT, COLOR_BG);
    sprite.setTextSize(1);

    if (battlePhase == BattlePhase::VS_SCREEN) {
        sprite.setCursor(SCREEN_W / 2 - 9, SCREEN_H / 2 - 20);
        sprite.print("VS");
        sprite.setCursor(6, SCREEN_H / 2);
        sprite.print("Poder inimigo: ");
        sprite.print(currentOpponentPower);
        sprite.setCursor(6, SCREEN_H - 20);
        sprite.print("B: atacar  C: fugir");
    } else { // RESULT
        sprite.setCursor(SCREEN_W / 2 - 24, SCREEN_H / 2 - 4);
        sprite.print(lastBattleWon ? "VITORIA!" : "DERROTA");
        sprite.setCursor(6, SCREEN_H - 12);
        sprite.print("B/C: voltar");
    }
}

// ------------------------------------------------------------
// Tela de Conexão: simula busca por outro aparelho (IR/BLE).
// Sai sozinha após CONNECT_TIMEOUT_MS, ou com B/C.
// ------------------------------------------------------------
void drawConnectScreen(TFT_eSprite &sprite, uint32_t now) {
    sprite.setTextColor(COLOR_TEXT, COLOR_BG);
    sprite.setTextSize(1);
    sprite.setCursor(6, SCREEN_H / 2 - 8);
    sprite.print("Procurando...");

    uint32_t elapsed = now - connectStartMs;
    uint8_t dots = (elapsed / 400) % 4;
    sprite.setCursor(6, SCREEN_H / 2 + 6);
    for (uint8_t i = 0; i < dots; i++) sprite.print(".");

    sprite.setCursor(6, SCREEN_H - 12);
    sprite.print("C: cancelar");
}

// ------------------------------------------------------------
// Monta o frame completo no buffer off-screen conforme o estado
// atual da UI, e só então manda para o display de uma vez.
// Isso é o que garante ausência de flicker.
// ------------------------------------------------------------
void renderFrame() {
    uint32_t now = millis();

    frameBuffer.fillSprite(COLOR_BG);

    switch (uiState) {
        case UiState::ACTION_STATUS:
            drawStatusScreen(frameBuffer);
            break;
        case UiState::ACTION_SLEEP:
            drawSleepScreen(frameBuffer);
            break;
        case UiState::ACTION_BATTLE:
            drawBattleScreen(frameBuffer);
            break;
        case UiState::ACTION_BOOK:
            drawStatusScreen(frameBuffer); // reaproveita o mesmo layout de texto
            break;
        case UiState::ACTION_CONNECT:
            drawConnectScreen(frameBuffer, now);
            break;
        default:
            // IDLE_MAIN e ações instantâneas (feed/train/clean, que
            // retornam ao IDLE_MAIN no mesmo tick) usam a tela principal
            drawMainScreen(frameBuffer, now);
            break;
    }

    // Envia o buffer inteiro para o TFT de uma só vez (push, sem
    // clear intermediário na tela real -> sem flicker)
    frameBuffer.pushSprite(0, 0);
}

// ------------------------------------------------------------
// Processa os botões de acordo com o estado atual da UI.
// A = cicla ícones | B = confirma | C = cancela/volta
// ------------------------------------------------------------
void handleInput() {
    switch (uiState) {

        case UiState::IDLE_MAIN:
            if (buttons.cyclePressed()) {
                menu.cycleNext();
            }
            if (buttons.confirmPressed()) {
                switch (menu.selected()) {
                    case ICON_FEED:    uiState = UiState::ACTION_FEED;   break;
                    case ICON_TRAIN:   uiState = UiState::ACTION_TRAIN;  break;
                    case ICON_CLEAN:   uiState = UiState::ACTION_CLEAN;  break;
                    case ICON_STATUS:  uiState = UiState::ACTION_STATUS; break;
                    case ICON_SLEEP:
                        pet.toggleSleep();          // entra dormindo
                        uiState = UiState::ACTION_SLEEP;
                        break;
                    case ICON_BATTLE:
                        battlePhase = BattlePhase::VS_SCREEN;
                        currentOpponentPower = 30 + (millis() % 60); // placeholder de RNG
                        uiState = UiState::ACTION_BATTLE;
                        break;
                    case ICON_BOOK:
                        uiState = UiState::ACTION_BOOK;
                        break;
                    case ICON_CONNECT:
                        connectStartMs = millis();
                        uiState = UiState::ACTION_CONNECT;
                        break;
                    default: break;
                }
            }
            if (buttons.powerOffHeld()) {
                pet.saveNow();      // grava antes de desligar
                // TODO: entrar em deep sleep / desligar tela, etc.
            }
            break;

        case UiState::ACTION_FEED:
            pet.feed();
            uiState = UiState::IDLE_MAIN;   // ação instantânea, volta ao menu
            break;

        case UiState::ACTION_TRAIN:
            pet.train();
            uiState = UiState::IDLE_MAIN;
            break;

        case UiState::ACTION_CLEAN:
            pet.clean();
            uiState = UiState::IDLE_MAIN;
            break;

        case UiState::ACTION_STATUS:
        case UiState::ACTION_BOOK:
            // Tela só de leitura; qualquer botão B/C volta ao menu
            if (buttons.confirmPressed()) {
                pet.saveNow();      // bom momento pra forçar save
                uiState = UiState::IDLE_MAIN;
            }
            break;

        case UiState::ACTION_SLEEP:
            // B ou C acordam o pet e voltam ao menu principal.
            if (buttons.confirmPressed() || buttons.cancelPressed()) {
                if (pet.data().isSleeping) pet.toggleSleep();
                uiState = UiState::IDLE_MAIN;
            }
            break;

        case UiState::ACTION_BATTLE:
            if (battlePhase == BattlePhase::VS_SCREEN) {
                if (buttons.confirmPressed()) {
                    lastBattleWon = pet.resolveBattle(currentOpponentPower);
                    battlePhase = BattlePhase::RESULT;
                }
            } else { // RESULT
                if (buttons.confirmPressed() || buttons.cancelPressed()) {
                    uiState = UiState::IDLE_MAIN;
                }
            }
            break;

        case UiState::ACTION_CONNECT:
            if (millis() - connectStartMs >= CONNECT_TIMEOUT_MS) {
                uiState = UiState::IDLE_MAIN; // desistiu de procurar
            }
            break;
    }

    // C sempre pode cancelar/voltar, independente do estado (exceto
    // no IDLE_MAIN, onde não há "para onde voltar")
    if (uiState != UiState::IDLE_MAIN && buttons.cancelPressed()) {
        uiState = UiState::IDLE_MAIN;
    }
}

void setup() {
    Serial.begin(115200);

    tft.init();
    tft.setRotation(0);
    tft.fillScreen(COLOR_BG);

    frameBuffer.setColorDepth(16);
    frameBuffer.createSprite(SCREEN_W, SCREEN_H);

    buttons.begin();
    menu.begin();
    pet.begin();
    petSprite.begin();
}

void loop() {
    uint32_t now = millis();

    buttons.update();
    pet.update();
    petSprite.update(now);

    handleInput();

    // Sinaliza no ícone de Alimentação/Treinamento quando o pet
    // precisa de atenção (como o "piscar verde" do original)
    menu.setActive(ICON_FEED, pet.data().hunger == 0);
    menu.setActive(ICON_TRAIN, pet.data().strength == 0);
    menu.setActive(ICON_CLEAN, pet.data().isSick);

    renderFrame();

    delay(10); // ritmo de loop leve; ajuste conforme necessidade de energia
}