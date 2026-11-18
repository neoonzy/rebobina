# REBOBINA 0.1.2-dev

- Correção após teste físico 0.1.1: fundo FOCUS cobria só a metade superior do LCD.
- Chamada `bmp_fill` alterada de 720x240 para **720x480 coordenadas lógicas** (não se modifica o tamanho físico de 360x240 do VRAM 5DC).
- Textos reposicionados em y=34,96,150,206,262,318,406, usando a metade inferior.
- Teste host agora exige altura lógica 480 e verifica desenho de texto abaixo de y=240.
- Permanecem HEADLESS/SHARED/FOCUS, SET/roda, MENU, RAM, CF, VxWorks e logs.
- Sem alteração no `AUTOEXEC.BIN`, bootflags ou firmware. Teste físico 0.1.2 pendente.
