# Arquitetura do sistema

## Fluxo de operação

1. A cada 60 ms o ESP32 inicia um ciclo de leitura
2. Os 4 sensores são lidos sequencialmente com 10 ms de pausa entre cada um
3. Cada leitura passa pelo filtro de média móvel (3 amostras)
4. O sensor com menor distância é identificado como sensor crítico
5. O intervalo de bipe é calculado com base na distância mínima
6. O display OLED é atualizado com barras posicionais e distância mínima
7. O buzzer é gerenciado de forma não bloqueante via millis()

## Por que leitura sequencial?

Leituras simultâneas causam crosstalk: o pulso de um sensor é captado
pelo ECHO do vizinho, gerando falsas leituras. A leitura sequencial
com pausa de 10 ms entre sensores elimina completamente esse problema.

## Filtro de média móvel

Cada sensor mantém buffer circular de 3 leituras. A distância reportada
é a média das 3 últimas. Atenua picos de ruído sem latência perceptível
(~180 ms de atraso máximo).
