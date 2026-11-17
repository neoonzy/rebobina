# REBOBINA 0.0.3 — resultado físico na EOS 5D Classic

O desenvolvedor confirmou a execução do payload 0.0.3 na câmera e anexou RB3BOOT.TXT e RB3LATE.TXT.

## Evidência extraída

- TICK_BEFORE=00000099; TICK_AFTER=0000009B.
- MEM_256=PASS.
- WORKER_ID_HEX=0016CB0C; a tarefa posterior executou.
- READBACK=PASS (leitura do arquivo gravado no CF e comparação).
- GUI_EVENTS_HEX=00000007, sete eventos observados.
- RAW_EVENT_00000000..00000006=00000000.
- LCD=OPT_IN_DISABLED.

## Diagnóstico dos eventos

Na versão 0.0.3 o hook registrava a palavra zero da estrutura de eventos. O Soup Engine para EOS 5D Classic interpreta w[1] como parametro do botao, enquanto w[0] e o tipo. Portanto sete zeros nao significam que os botoes falharam. A 0.0.4 registra ambas as palavras e mantem retorno 1 para repassar cada evento.

## Limites

Os textos MODEL e FIRMWARE_TARGET no log sao literais do payload, nao uma sondagem de firmware. O teste nao comprova boot independente do carregador Soup nem validacao de LCD/LED, captura, RAW ou estabilidade prolongada.

Referencia GPLv2: https://github.com/jiijii0/soup-engine/blob/master/Canon-EOS-5D/src/ml/payload/film.c
