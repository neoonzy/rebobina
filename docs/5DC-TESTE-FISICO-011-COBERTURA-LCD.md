# Teste físico REBOBINA 0.1.1 — cobertura da metade superior
Na EOS 5D Classic com firmware Canon 1.1.1, o usuário observou em pessoa o modo FOCUS funcionando mas cobrindo do meio para cima, não a tela inteira.

Arquivos fornecidos: `RB6BOOT.TXT` (boot, RAM 256 B e 4096 B PASS, worker id 0x0016CB0C, UI enabled) e `RB6TEST.TXT` (readback PASS, UI_MODE=FOCUS_EXPERIMENTAL, RAM total 0x00700000 e livre 0x00286100).
Não foram enviados RB6EVENT ou RB6FINAL nessa ocasião.

**Causa na implementação:** `rb_hal_fill(2,0,0,720,240)` cobria 240 de 480 unidades verticais lógicas. No VxWorks de 5DC o bitmap físico tem 240 linhas, mas o endereçamento interno usa `y/2`, então o comando percorria apenas metade.

A 0.1.2 altera o preenchimento para 720x480 unidades lógicas, sem ampliar o framebuffer, e reposiciona as legendas nas duas metades do painel. Ainda precisa de validação física.

Fontes GPLv2: Magic Lantern Team [bmp.h](https://github.com/frantony/magic-lantern/blob/a3454ebed3f6adcd165ee9382f451f917d989253/src/bmp.h), [bmp.c](https://github.com/frantony/magic-lantern/blob/a3454ebed3f6adcd165ee9382f451f917d989253/src/bmp.c); jiijii0 / Soup Engine.
