# Digimon Color Port (ESP32-C3) 🦖✨

[![Version](https://img.shields.io/badge/version-0.0.1-blue.svg)](https://github.com/Hanry/digimon_color_port/releases/tag/v0.0.1)
[![Framework](https://img.shields.io/badge/Framework-Arduino_ESP32-green.svg)](https://www.arduino.cc/)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-Supported-orange.svg)](https://platformio.org/)

Port open-source e recriação das mecânicas do lendário **Digimon Color (Virtual Pet)** desenvolvido para o microcontrolador **ESP32-C3** com ecrã a cores SPI.

---

## 📌 Funcionalidades (v0.0.1)

- 🖼️ **Renderização sem Flicker:** Sistema de *Double Buffering* utilizando `TFT_eSprite` para animações fluidas.
- 🕹️ **Menu de Ações:** Interface com seleção circular de ícones (Alimentar, Treinar, Limpar, Status, Sono, Batalha, Livro e Conexão).
- 🧬 **Motor de Estados do Pet:** Sistema de atributos em tempo real (Fome, Força, Peso, Idade, Erros de Cuidado, Doença e Fases de Evolução).
- 💾 **Persistência de Dados:** Guardado automático do progresso na memória NVS do ESP32 através da biblioteca `Preferences`.
- ⚔️ **Ecrãs Dedicados:** Sub-máquinas de estado para telas de Batalha (VS/Resultado), Status detalhado e Simulação de Conexão.

---

## 🛠️ Hardware Necessário

| Componente | Especificação Recomendada |
| :--- | :--- |
| **Microcontrolador** | ESP32-C3 DevKit (ex: ESP32-C3-DevKitM-1) |
| **Ecrã** | Display TFT SPI 128x128 ou 240x240 (ST7789 / ILI9341) |
| **Botões** | 3x Push Buttons (A: Alternar \| B: Confirmar \| C: Cancelar) |

---

## 📁 Estrutura do Projeto

```text
digimon_color_port/
├── include/
│   ├── ButtonManager.h   # Leitura e debouncing dos botões
│   ├── Config.h          # Configurações globais de hardware e pinos
│   ├── MenuManager.h     # Lógica e navegação dos ícones do menu
│   ├── PetState.h        # Regras de negócio do Pet e evolução
│   └── SpriteManager.h   # Gestor de animações e renderização
├── src/
│   ├── main.cpp          # Loop principal e máquina de estados da UI
│   └── SpriteData.cpp    # Arrays de dados de pixels dos sprites (RGB565)
├── platformio.ini        # Configurações de build do PlatformIO
└── .gitignore
```

---

## 🚀 Como Compilar e Enviar

### Pré-requisitos
- [Visual Studio Code](https://code.visualstudio.com/)
- Extensão [PlatformIO IDE](https://platformio.org/platformio-ide)

### Compilação
1. Clona o repositório:
   ```bash
   git clone [https://github.com/TEU_USUARIO/digimon_color_port.git](https://github.com/TEU_USUARIO/digimon_color_port.git)
   cd digimon_color_port
   ```
2. Compila o código:
   ```bash
   platformio run
   ```
3. Para gravar na placa (liga o ESP32-C3 via USB):
   ```bash
   platformio run -t upload
   ```

---

## 📄 Licença

Este projeto é desenvolvido para fins educacionais e de preservação da cultura dos *Virtual Pets*. Os direitos sobre a marca Digimon pertencem à **Bandai Namco**.