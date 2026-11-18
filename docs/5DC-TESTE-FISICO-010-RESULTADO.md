# REBOBINA 0.1.0-dev — primeiro desenho observado no LCD da Canon 5D Classic

**Data do relato:** 18/11/2026. **Câmera:** EOS 5D Classic 1.1.1, cartão CF de desenvolvimento, carregador legado Soup Engine. **Evidência:** arquivos `RB5BOOT.TXT`, `RB5TEST.TXT`, `RB5EVENT.TXT`, `RB5FINAL.TXT` fornecidos pelo autor do projeto e observação direta do LCD.

## Resultados físicos

O autor confirmou: **"vi sim, mas como era na tela do menu, vez ou outra o menu passava por cima das letras do rebobina"**. Isso resolve a incerteza anterior: os desenhos foram visíveis no LCD real. Não se trata somente de um contador de chamadas a `bmp_printf`.

Trechos relevantes dos arquivos enviados:

- **Boot:** `TICK_BEFORE_HEX=0000009A`, `TICK_AFTER_HEX=0000009C`, `MEM_256_HEX=00000001`, `MEM_4096_HEX=00000001`, `WORKER_ID_HEX=0016CB0C`, `UI_ENABLED_HEX=00000001`.
- **Diagnóstico:** `READBACK=PASS`, `READBACK_STATUS_HEX=00000001`, `TOTAL_RAM_HEX=00700000`, `FREE_RAM_HEX=00286100`, `TICK_START_HEX=0000009C`.
- **Entrada:** `TOTAL_EVENTS_HEX=0000002F`, `CAPTURED_EVENTS_HEX=0000002F`, `DROPPED_EVENTS_HEX=00000000` — **47 eventos**. Navegação, abertura de menus e outras operações foram observadas como parâmetros GUI.
- **Encerramento:** `TICK_END_HEX=0000107C`, `UI_PAGE_HEX=00000002`, `UI_DRAW_CALLS_HEX=00000020`, `DISPLAY_ON_SAMPLES_HEX=00000040`, os três arquivos de saída anteriores confirmados com `WRITE_OK=1`.

Entre os ticks `0x9C` e `0x107C` transcorreram `0xFE0 = 4064` ticks — aproximadamente **40,64 segundos a 100 Hz**. A gravação de `RB5FINAL.TXT` comprova que a tarefa chegou ao encerramento previsto.

## Problema visual descoberto

A 0.1.0 desenhava textos com a função `bmp_printf` da imagem Magic Lantern, apenas sobre o **bitmap compartilhado** usado também pelo menu Canon/ML. O worker repintava o quadro a aproximadamente 1 Hz, e um redesenho do menu podia apagar ou cobrir parte do texto.

Isto é **concorrência pela mesma camada gráfica**, não falha de gravação do CF ou de chamada `bmp_printf`.

## Nova abordagem (0.1.1-dev)

- Preservar `../preview-010` como referência do experimento físico bem-sucedido.
- Adicionar modo `FOCUS` explicitamente habilitado por `ML/RB6FOCUS.ON`, usando `bmp_fill` e desenho subsequente com guardas de bitmap e 4 Hz; manter `SHARED` e `HEADLESS`.
- Consumir apenas SET e eventos de roda enquanto `FOCUS` estiver ativo, para impedir a navegação simultânea da Canon; MENU sai do modo.
- Essa solução ainda compartilha a camada com Canon/ML, portanto **não garante exclusividade nem ausência de conflito**. O modo FOCUS precisa de teste físico próprio; não alterar ROM Canon, bootflags nem AUTOEXEC.BIN.

**Fonte histórica:** Magic Lantern Team, `platform/5DC.111`; jiijii0/Soup Engine, `Canon-EOS-5D/src/ml/payload/film.c` (GPLv2). O Soup Engine também enfrentou sobreposição de menus e documenta a estratégia de redesenho da área completa em intervalos curtos.
