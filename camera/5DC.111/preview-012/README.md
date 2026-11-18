# REBOBINA 0.1.2-dev — correção da cobertura vertical do LCD

## Resultado real anterior
O usuário executou a 0.1.1-dev na Canon EOS 5D Classic e **viu o modo FOCUS cobrindo somente a metade superior**. Os dois arquivos enviados (`RB6BOOT.TXT`, `RB6TEST.TXT`) confirmam inicialização, dois testes de RAM e leitura de CF, com `UI_MODE=FOCUS_EXPERIMENTAL`. Não foram enviados RB6EVENT/RB6FINAL nessa sessão: não presumir conclusão da janela de eventos.

## Causa e correção
Magic Lantern 5DC (CONFIG_VXWORKS): coordenadas lógicas 720x480, framebuffer físico 360 bytes/linha × 240 linhas com 4 bits por pixel. Em `src/bmp.c`, `bmp_fill` usa mapeamento vertical `y/2` (macro BM). A 0.1.1 chamava `bmp_fill(2,0,0,720,240)`, preenchendo metade da altura. A 0.1.2 corrige para `720x480` e dobra as coordenadas verticais dos textos, ocupando o painel inteiro. Não aloca nem modifica VRAM além do limite fornecido pelo próprio Magic Lantern.

A mesma build mantém HEADLESS (sem LCD), SHARED (texto sobre o menu Canon) e FOCUS (fundo inteiro + texto 4 Hz, SET/roda sob foco, MENU sai). Mantém cinco páginas, 48 eventos, memória, relógio, tarefas, arquivos e relatórios. **Esta versão ainda não passou por teste físico**; CI apenas compila e simula.

## Testar com CF já habilitado para Soup (Windows D:)
```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\prepare-test.ps1 -CardRoot D:\ -EnableFocusUI
```

Com a câmera desligada, copie do staging apenas `SE.BIN` e o marcador `ML/RB7FOCUS.ON`. Preserve `AUTOEXEC.BIN`, firmware, bootflags, pasta ML e fotografias. Faça backup de `SE.BIN` anterior. Antigos marcadores RB5/RB6 são ignorados pela versão nova. MENU Canon liga a tela; navegue no REBOBINA com roda/SET. MENU sai do foco. Após cerca de 40 segundos, examine `RB7BOOT.TXT`, `RB7TEST.TXT`, `RB7EVENT.TXT`, `RB7FINAL.TXT`. O script de preparo **não escreve no CF**. Em caso de erro/travamento, interrompa o teste e restaure o `SE.BIN` do backup.

## Pesquisa e licença
Magic Lantern Team GPLv2: [bmp.h](https://github.com/frantony/magic-lantern/blob/a3454ebed3f6adcd165ee9382f451f917d989253/src/bmp.h) e [bmp.c](https://github.com/frantony/magic-lantern/blob/a3454ebed3f6adcd165ee9382f451f917d989253/src/bmp.c); jiijii0 / Soup Engine GPLv2: carregador e pesquisa da 5DC. Para a 0.1.1, veja `../preview-011/README.md`. Não é firmware Canon independente e não é indicado para uso cotidiano.
