# Componentes e especificações

## HC-SR04

| Parâmetro | Valor |
|---|---|
| Tensão | 5V DC |
| Alcance | 2 – 400 cm |
| Ângulo de detecção | ~15° |
| Fórmula | distância (cm) = duração_echo (µs) / 58 |

ATENÇÃO: Pino ECHO em 5V. Use divisor 1kΩ/2kΩ para proteger GPIO ESP32.

## ESP32

| Parâmetro | Valor |
|---|---|
| CPU | Xtensa LX6 dual-core 240 MHz |
| RAM | 520 KB |
| Tensão lógica | 3,3V |
| LEDC PWM | 16 canais |

## OLED SSD1306

| Parâmetro | Valor |
|---|---|
| Resolução | 128 × 64 px |
| Interface | I2C — 0x3C |
| Tensão | 3,3V – 5V |
