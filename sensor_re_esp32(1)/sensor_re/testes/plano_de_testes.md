# Plano de testes

## Teste 1 — Sensor individual
Posicionar obstáculo a 30, 50 e 80 cm de cada sensor.
Critério: variação máxima ±3 cm.

## Teste 2 — Interferência cruzada
Obstáculos a distâncias diferentes por sensor.
Critério: nenhum sensor reporta distância do vizinho.

## Teste 3 — Filtro de média móvel
Superfície inclinada a ~50 cm.
Critério: variação filtrada não ultrapassa ±5 cm.

## Teste 4 — Display posicional
Aproximar objeto de um sensor por vez.
Critério: apenas a barra do sensor ativo cresce.

## Teste 5 — Alerta sonoro

| Distância | Comportamento esperado |
|---|---|
| > 100 cm | Silêncio |
| 80–100 cm | Bipe lento |
| 60–80 cm | Bipe médio |
| 40–60 cm | Bipe rápido |
| 20–40 cm | Bipe muito rápido |
| 10–20 cm | Bipe super rápido |
| < 10 cm | Tom contínuo 2000 Hz |
