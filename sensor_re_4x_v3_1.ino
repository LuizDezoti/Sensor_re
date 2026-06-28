/**
 * @file    sensor_re_4x_v3_1.ino
 * @brief   Sistema de sensor de ré automotivo — 4 sensores HC-SR04 / JSN-SR04T
 * @version 3.1.0 (Revisado)
 * @date    2025
 * @author  [Seu Nome]
 * @license MIT
 *
 * Correções da v3.1.0:
 *   - Redução do timeout do pulseIn para 12ms (evita bloqueio do buzzer)
 *   - Filtro de média móvel agora ignora falhas de leitura (timeout)
 *   - Compatibilidade automática com ESP32 Core v2.x e v3.x (API LEDC)
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ============================================================
//  DISPLAY
// ============================================================
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define OLED_I2C_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ============================================================
//  NÚMERO DE SENSORES E PINAGEM
// ============================================================
#define NUM_SENSORES 4

#define S_ESQ        0
#define S_CENTRO_ESQ 1
#define S_CENTRO_DIR 2
#define S_DIR        3

const uint8_t PIN_TRIG[NUM_SENSORES] = { 5,  4, 26, 27 };
const uint8_t PIN_ECHO[NUM_SENSORES] = {18, 19, 25, 14 };

const char* NOME_SENSOR[NUM_SENSORES] = {
  "ESQ", "C-ESQ", "C-DIR", "DIR"
};

// ============================================================
//  BUZZER (PWM — GPIO 17)
// ============================================================
const uint8_t  PIN_BUZZER       = 17;
const uint8_t  LEDC_CANAL       = 0;
const uint32_t LEDC_FREQ_BASE   = 2000;
const uint8_t  LEDC_RESOLUCAO   = 8;

// ============================================================
//  TEMPORIZAÇÃO E SENSOR ULTRASSÔNICO
// ============================================================
const uint16_t INTERVALO_CICLO_MS = 60;
const uint8_t  DIVISOR_CM         = 58;

// Reduzido para 12.000us (alcance máx. de ~200cm).
// Isso garante que o tempo de bloqueio máximo seja baixo o suficiente
// para não interferir com a velocidade do buzzer.
const uint32_t TIMEOUT_US         = 12000; 

// ============================================================
//  CONSTANTES DE DISTÂNCIA E ÁUDIO
// ============================================================
const uint16_t DIST_CRITICA      =  10;
const uint16_t DIST_MUITO_PERTO  =  20;
const uint16_t DIST_PERTO        =  40;
const uint16_t DIST_MEDIA        =  60;
const uint16_t DIST_LONGE        =  80;
const uint16_t DIST_MUITO_LONGE  = 100;
const uint16_t DIST_FORA_ALCANCE = 200;
const uint16_t DIST_SEM_ECO      = 999;

const uint16_t FREQ_ALARME_CONTINUO = 2000;
const uint16_t FREQ_BIP_MINIMA      = 1400;
const uint16_t FREQ_BIP_MAXIMA      = 1900;

const uint16_t BIP_CRITICO     =   1;
const uint16_t BIP_MUITO_PERTO =  80;
const uint16_t BIP_PERTO       = 120;
const uint16_t BIP_MEDIO       = 180;
const uint16_t BIP_LENTO       = 240;
const uint16_t BIP_MUITO_LENTO = 300;
const uint16_t BIP_SILENCIO    =   0;

// ============================================================
//  FILTRO DE MÉDIA MÓVEL E ESTADOS GLOBAIS
// ============================================================
const uint8_t AMOSTRAS = 3;

static uint16_t s_distFiltrada[NUM_SENSORES];
static uint16_t s_bufferAmostras[NUM_SENSORES][AMOSTRAS];
static uint8_t  s_indiceBuffer[NUM_SENSORES];

static unsigned long s_tempoAnteriorCiclo  = 0;
static unsigned long s_tempoAnteriorBuzzer = 0;
static bool          s_estadoBuzzer        = false;
static uint16_t      s_intervaloBip        = BIP_SILENCIO;

static uint8_t  s_sensorCritico  = 0;
static uint16_t s_distMinima     = DIST_SEM_ECO;

// ============================================================
//  PROTÓTIPOS
// ============================================================
uint16_t lerDistanciaRaw(uint8_t idx);
uint16_t filtrarLeitura(uint8_t idx, uint16_t novaMedida);
void     executarCicloLeitura();
uint16_t calcularIntervaloSonoro(uint16_t dist);
void     gerenciarSom(unsigned long tempoAtual);
void     atualizarDisplay();
void     desenharBarrasIndicadoras();
void     desenharInfoCentral();
void     tocarBuzzer(uint16_t freq);
void     pararBuzzer();

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  Serial.println(F("[BOOT] Sensor de ré v3.1 — 4 sensores"));

  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    pinMode(PIN_TRIG[i], OUTPUT);
    pinMode(PIN_ECHO[i], INPUT);
    digitalWrite(PIN_TRIG[i], LOW);
    
    for (uint8_t j = 0; j < AMOSTRAS; j++) {
      s_bufferAmostras[i][j] = DIST_SEM_ECO;
    }
    s_distFiltrada[i] = DIST_SEM_ECO;
    s_indiceBuffer[i] = 0;
  }

  // Inicializa o PWM de forma compatível com ESP32 Core v2 e v3
  #if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcAttach(PIN_BUZZER, LEDC_FREQ_BASE, LEDC_RESOLUCAO);
  #else
    ledcSetup(LEDC_CANAL, LEDC_FREQ_BASE, LEDC_RESOLUCAO);
    ledcAttachPin(PIN_BUZZER, LEDC_CANAL);
    ledcWrite(LEDC_CANAL, 0);
  #endif

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println(F("[ERRO] OLED nao inicializado. Verifique I2C."));
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 10);
  display.print(F("SENSOR DE RE v3.1"));
  display.setCursor(10, 26);
  display.print(F("SISTEMA SEGURO"));
  display.setCursor(10, 42);
  display.print(F("INICIALIZANDO..."));
  display.display();
  delay(1500);

  Serial.println(F("[OK] Sistema pronto."));
}

// ============================================================
//  LOOP PRINCIPAL
// ============================================================
void loop() {
  unsigned long tempoAtual = millis();

  if (tempoAtual - s_tempoAnteriorCiclo >= INTERVALO_CICLO_MS) {
    s_tempoAnteriorCiclo = tempoAtual;
    executarCicloLeitura();
    atualizarDisplay();
  }

  gerenciarSom(tempoAtual);
}

// ============================================================
//  LEITURA DOS SENSORES
// ============================================================
void executarCicloLeitura() {
  s_distMinima  = DIST_SEM_ECO;
  s_sensorCritico = 0;

  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    uint16_t raw      = lerDistanciaRaw(i);
    s_distFiltrada[i] = filtrarLeitura(i, raw);

    if (s_distFiltrada[i] < s_distMinima) {
      s_distMinima    = s_distFiltrada[i];
      s_sensorCritico = i;
    }

    delay(10); // Pausa anti-crosstalk
  }

  s_intervaloBip = calcularIntervaloSonoro(s_distMinima);

  Serial.printf("[CICLO] ESQ:%3u C-E:%3u C-D:%3u DIR:%3u | MIN:%3u (%s) | BIP:%u\n",
    s_distFiltrada[S_ESQ],
    s_distFiltrada[S_CENTRO_ESQ],
    s_distFiltrada[S_CENTRO_DIR],
    s_distFiltrada[S_DIR],
    s_distMinima,
    NOME_SENSOR[s_sensorCritico],
    s_intervaloBip
  );
}

uint16_t lerDistanciaRaw(uint8_t idx) {
  digitalWrite(PIN_TRIG[idx], LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG[idx], HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG[idx], LOW);

  long duracao = pulseIn(PIN_ECHO[idx], HIGH, TIMEOUT_US);
  if (duracao == 0) return DIST_SEM_ECO;
  return (uint16_t)(duracao / DIVISOR_CM);
}

uint16_t filtrarLeitura(uint8_t idx, uint16_t novaMedida) {
  // Correção: Se o sensor der timeout falhando a leitura, utilizamos
  // o valor da última leitura válida no buffer para evitar falsos positivos
  if (novaMedida == DIST_SEM_ECO) {
    uint8_t indiceAnterior = (s_indiceBuffer[idx] == 0) ? (AMOSTRAS - 1) : (s_indiceBuffer[idx] - 1);
    novaMedida = s_bufferAmostras[idx][indiceAnterior];
  }

  s_bufferAmostras[idx][s_indiceBuffer[idx]] = novaMedida;
  s_indiceBuffer[idx] = (s_indiceBuffer[idx] + 1) % AMOSTRAS;

  uint32_t soma = 0;
  for (uint8_t j = 0; j < AMOSTRAS; j++) {
    soma += s_bufferAmostras[idx][j];
  }
  return (uint16_t)(soma / AMOSTRAS);
}

// ============================================================
//  ÁUDIO E COMPATIBILIDADE PWM
// ============================================================
void tocarBuzzer(uint16_t freq) {
  #if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWriteTone(PIN_BUZZER, freq);
  #else
    ledcWriteTone(LEDC_CANAL, freq);
  #endif
}

void pararBuzzer() {
  #if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWriteTone(PIN_BUZZER, 0);
  #else
    ledcWriteTone(LEDC_CANAL, 0);
  #endif
}

uint16_t calcularIntervaloSonoro(uint16_t dist) {
  if (dist <= DIST_CRITICA)     return BIP_CRITICO;
  if (dist > DIST_MUITO_LONGE)  return BIP_SILENCIO;
  if (dist <= DIST_MUITO_PERTO) return BIP_MUITO_PERTO;
  if (dist <= DIST_PERTO)       return BIP_PERTO;
  if (dist <= DIST_MEDIA)       return BIP_MEDIO;
  if (dist <= DIST_LONGE)       return BIP_LENTO;
  return BIP_MUITO_LENTO;
}

void gerenciarSom(unsigned long tempoAtual) {
  if (s_intervaloBip == BIP_SILENCIO) {
    pararBuzzer();
    s_estadoBuzzer = false;
    return;
  }

  if (s_intervaloBip == BIP_CRITICO) {
    tocarBuzzer(FREQ_ALARME_CONTINUO);
    s_estadoBuzzer = true;
    return;
  }

  if (tempoAtual - s_tempoAnteriorBuzzer >= s_intervaloBip) {
    s_tempoAnteriorBuzzer = tempoAtual;
    s_estadoBuzzer = !s_estadoBuzzer;

    if (s_estadoBuzzer) {
      uint16_t distClamped = constrain(s_distMinima, DIST_CRITICA, DIST_MEDIA);
      uint16_t frequencia  = map(distClamped, DIST_MEDIA, DIST_CRITICA,
                                 FREQ_BIP_MINIMA, FREQ_BIP_MAXIMA);
      tocarBuzzer(frequencia);
    } else {
      pararBuzzer();
    }
  }
}

// ============================================================
//  DISPLAY OLED
// ============================================================
void atualizarDisplay() {
  display.clearDisplay();
  desenharBarrasIndicadoras();
  desenharInfoCentral();
  display.display();
}

void desenharBarrasIndicadoras() {
  const uint8_t BAR_W   = 26;
  const uint8_t BAR_GAP = 4;
  const uint8_t BAR_MAX_H = 30;
  const uint8_t BAR_Y_BASE = 35;

  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    uint8_t x = 2 + i * (BAR_W + BAR_GAP);
    uint16_t dist = s_distFiltrada[i];
    uint8_t altBarra = 0;
    
    if (dist < DIST_MUITO_LONGE) {
      uint16_t distClamped = constrain(dist, DIST_CRITICA, DIST_MUITO_LONGE);
      altBarra = (uint8_t)map(distClamped, DIST_MUITO_LONGE, DIST_CRITICA, 1, BAR_MAX_H);
    }

    display.drawRect(x, BAR_Y_BASE - BAR_MAX_H, BAR_W, BAR_MAX_H, SSD1306_WHITE);

    if (altBarra > 0) {
      display.fillRect(x, BAR_Y_BASE - altBarra, BAR_W, altBarra, SSD1306_WHITE);
    }

    display.setTextSize(1);
    display.setCursor(x + 2, BAR_Y_BASE + 2);
    if (i == S_ESQ)        display.print(F("ESQ"));
    else if (i == S_CENTRO_ESQ) display.print(F("C-E"));
    else if (i == S_CENTRO_DIR) display.print(F("C-D"));
    else                   display.print(F("DIR"));

    if (i == s_sensorCritico && s_distMinima < DIST_MUITO_LONGE) {
      if ((millis() / 300) % 2 == 0) {
        display.drawRect(x - 1, BAR_Y_BASE - BAR_MAX_H - 1,
                         BAR_W + 2, BAR_MAX_H + 2, SSD1306_WHITE);
      }
    }
  }
}

void desenharInfoCentral() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("DIST MIN:"));

  display.setTextSize(2);
  display.setCursor(0, 10);
  if (s_distMinima >= DIST_FORA_ALCANCE) {
    display.print(F("---"));
  } else {
    display.print(s_distMinima);
    display.setTextSize(1);
    display.print(F(" cm"));
  }

  display.setTextSize(1);
  display.setCursor(0, 56);
  if (s_distMinima <= DIST_CRITICA) {
    if ((millis() / 200) % 2 == 0) display.print(F("!! PERIGO !!"));
    else                            display.print(F("            "));
  } else if (s_distMinima <= 30) {
    display.print(F("ATENCAO MAX"));
  } else if (s_distMinima <= DIST_MEDIA) {
    display.print(F("APROXIMACAO"));
  } else if (s_distMinima < DIST_MUITO_LONGE) {
    display.print(F("SEGURO"));
  } else {
    display.print(F("LIVRE"));
  }
}
