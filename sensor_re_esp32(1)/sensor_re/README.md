# Sensor de Ré Automotivo com ESP32

> Sistema embarcado de detecção de obstáculos traseiros utilizando 4 sensores ultrassônicos HC-SR04, display OLED SSD1306 e alerta sonoro via buzzer PWM.

---

## Visão geral

Este projeto implementa um sensor de estacionamento traseiro próximo dos sistemas utilizados comercialmente em automóveis. O sistema detecta obstáculos em quatro posições distintas na traseira do veículo — esquerda, centro-esquerda, centro-direita e direita — e informa o motorista por meio de alertas visuais no display OLED e alertas sonoros com frequência e cadência variáveis conforme a distância.

---

## Funcionalidades

- 4 sensores ultrassônicos HC-SR04 com leitura sequencial (evita interferência cruzada)
- Filtro de média móvel por sensor (3 amostras)
- Display OLED 128×64 com barras indicadoras posicionais e distância mínima
- Alerta sonoro via buzzer passivo com cadência e frequência dinâmicas
- Comunicação Serial estruturada para debug
- Código modular e documentado (padrão Doxygen)

---

## Hardware

| Componente | Quantidade | Observação |
|---|---|---|
| ESP32 DevKit | 1 | Qualquer variante com I2C e PWM |
| HC-SR04 | 4 | Sensor ultrassônico 2–400 cm |
| OLED SSD1306 128×64 | 1 | Interface I2C, endereço 0x3C |
| Buzzer passivo | 1 | 3–5V, conectado ao GPIO 17 |
| Resistor 1kΩ | 4 | Divisor de tensão pino ECHO |
| Resistor 2kΩ | 4 | Divisor de tensão pino ECHO |

---

## Pinagem

| Sensor | TRIG | ECHO |
|---|---|---|
| Esquerda | GPIO 5 | GPIO 18 |
| Centro-Esq | GPIO 4 | GPIO 19 |
| Centro-Dir | GPIO 26 | GPIO 25 |
| Direita | GPIO 27 | GPIO 14 |

| Periférico | Pino |
|---|---|
| Buzzer PWM | GPIO 17 |
| OLED SDA | GPIO 21 |
| OLED SCL | GPIO 22 |

> **Atenção:** Os pinos ECHO do HC-SR04 operam em 5V. É obrigatório o uso de divisor resistivo (1kΩ + 2kΩ) para adequar o sinal a 3,3V aceito pelo ESP32.

---

## Bibliotecas necessárias

Instalar via Arduino IDE > Library Manager:

- `Adafruit GFX Library`
- `Adafruit SSD1306`

---

## Como usar

1. Clone este repositório
2. Abra `firmware/sensor_re_v3/sensor_re_v3.ino` no Arduino IDE
3. Instale as bibliotecas listadas acima
4. Selecione a placa: **ESP32 Dev Module**
5. Faça o upload e abra o Serial Monitor a 115200 baud

---

## Estrutura do repositório

```
sensor-re-esp32/
├── docs/               Documentação técnica
├── firmware/           Código-fonte do firmware
│   └── sensor_re_v3/   Versão atual (4 sensores)
├── hardware/
│   ├── esquematicos/   Esquemáticos elétricos
│   ├── pcb/            Arquivos de layout de PCB
│   └── simulacoes/     Simulações (Wokwi, Proteus)
├── imagens/            Fotos do projeto montado
├── diagramas/          Diagramas de blocos e fluxogramas
├── testes/             Relatórios e scripts de teste
├── bibliotecas/        Bibliotecas externas (se necessário)
├── apresentacoes/      Slides de apresentação
├── videos/             Links ou arquivos de demonstração
├── README.md
├── CHANGELOG.md
└── LICENSE
```

---

## Versões

Consulte o [CHANGELOG.md](CHANGELOG.md) para o histórico completo de versões.

---

## Licença

MIT License — veja [LICENSE](LICENSE) para detalhes.
