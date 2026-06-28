# Changelog

Todas as alterações relevantes deste projeto serão documentadas neste arquivo.

Formato baseado em [Keep a Changelog](https://keepachangelog.com/pt-BR/1.0.0/).

---

## [3.0.0] — 2025

### Adicionado
- Suporte a 4 sensores HC-SR04 com leitura sequencial
- Indicador posicional no display OLED (4 barras verticais)
- Identificação do sensor crítico (mais próximo)
- Log Serial estruturado com todos os valores por ciclo
- Pausa entre leituras para evitar interferência cruzada (crosstalk)

### Alterado
- `tone()`/`noTone()` substituídos por `ledcWriteTone()` (LEDC nativo ESP32)
- Display reorganizado: barras indicadoras + painel de distância mínima

---

## [2.0.0] — 2025

### Adicionado
- Filtro de média móvel (3 amostras) por sensor
- Constantes nomeadas (sem magic numbers)
- Cabeçalho Doxygen completo
- Proteção de underflow na frequência do buzzer

### Corrigido
- `for(;;)` silencioso substituído por pisca-LED + log Serial
- Frequência dinâmica do buzzer agora usa `constrain()` + `map()`

---

## [1.0.0] — 2025

### Adicionado
- Versão inicial com 1 sensor HC-SR04
- Display OLED com radar gráfico
- Alerta sonoro com cadência variável
- Lógica não bloqueante com `millis()`
