# Teste físico da 0.0.4 — entradas GUI identificadas na Canon 5D Classic

**Evidência:** dois arquivos `RB4BOOT.TXT` e `RB4LATE.TXT` enviados pelo desenvolvedor, produzidos pela câmera no teste da versão 0.0.4. A data exata do teste não foi inserida nos arquivos.

## Conteúdo observado: RB4BOOT.TXT
```text
REBOBINA BOOT DIAGNOSTICO 0.0.4
MODEL=EOS_5D_CLASSIC
FIRMWARE_TARGET=1.1.1
LOADER=SOUP_LEGACY_5DC
TICK_BEFORE=00000099
TICK_AFTER=0000009B
MEM_256=PASS
WORKER_ID_HEX=0016CB0C
EVENT_HOOK=PASS_THROUGH
```

## Conteúdo observado: RB4LATE.TXT
```text
REBOBINA DIAGNOSTICO 0.0.4 / PASSIVE KEY TEST
READBACK=PASS
GUI_EVENTS_HEX=00000009
RAW_EVENT_00000000=00000000
EVENT_PARAM_00000000=00000003
RAW_EVENT_00000001=00000000
EVENT_PARAM_00000001=0000000B
RAW_EVENT_00000002=00000000
EVENT_PARAM_00000002=00000009
RAW_EVENT_00000003=00000000
EVENT_PARAM_00000003=00000004
RAW_EVENT_00000004=00000000
EVENT_PARAM_00000004=0000000B
RAW_EVENT_00000005=00000000
EVENT_PARAM_00000005=00000009
RAW_EVENT_00000006=00000000
EVENT_PARAM_00000006=00000002
RAW_EVENT_00000007=00000000
EVENT_PARAM_00000007=00000002
RAW_EVENT_00000008=00000000
EVENT_PARAM_00000008=00000007
LCD=OPT_IN_DISABLED
```

## Resultado e interpretação

- O programa gravou o relatório de boot, a memória alocada de 256 bytes passou no teste, o tick avançou e a tarefa posterior executou, produzindo um segundo arquivo. A leitura/comparação desse arquivo deu `READBACK=PASS`.
- **Nove eventos** passaram pelo hook da GUI e tiveram o tipo `w[0]=0`, consistente com a categoria de evento de botão.
- Parâmetros `w[1]` em ordem: `03, 0B, 09, 04, 0B, 09, 02, 02, 07`.
- Segundo `platform/5DC.111/gui.h` do Magic Lantern: `03` MENU; `04` INFO; `02` SET; `07` TRASH. `09` corresponde ao rótulo histórico **UNPRESS_ZOOMIN_MAYBE**, `0B` a **UNPRESS_ZOOMOUT_MAYBE**. Os dois últimos são **hipóteses históricas**, e a sequência física de pressionamentos não foi fornecida para confirmação.
- `LCD=OPT_IN_DISABLED` — não houve teste de desenho no painel.
- O programa sempre retornava 1 no hook; os eventos continuaram para a interface Canon/ML, sem interceptação intencional.
- `MODEL`, `FIRMWARE_TARGET` e `LOADER` são strings do próprio programa e não provam identificação dinâmica da ROM.

**Este marco comprova acesso passivo aos parâmetros de botões**, não navegação ou interface LCD própria.

## Ferramenta de leitura no computador

```sh
python3 camera/5DC.111/diagnostic-004/tools/decode_events.py RB4LATE.TXT
```

Leitura somente, sem modificar o CF. Código histórico: [Magic Lantern 5DC.111/gui.h](https://github.com/frantony/magic-lantern/blob/a3454ebed3f6adcd165ee9382f451f917d989253/platform/5DC.111/gui.h) — GPLv2, Magic Lantern Team.

## Próximo experimento

Separar a prova de saída gráfica da prova de navegação (não ativar LCD/LED/consumo de eventos de uma só vez). O hook passivo atual é uma base válida para uma API de navegação; os códigos provisórios de zoom devem permanecer classificados como não confirmados. O primeiro teste de LCD deve manter rollback do `SE.BIN` e não modificar `AUTOEXEC.BIN`.
