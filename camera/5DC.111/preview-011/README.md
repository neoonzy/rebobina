# REBOBINA 0.1.1-dev — desenho em foco + subsistemas integrados

**O REBOBINA 0.1.0-dev apareceu no LCD real da EOS 5D Classic.** O desenvolvedor relatou que o menu Canon/ML por vezes passava por cima das letras do REBOBINA. A versão anterior tinha apenas texto desenhado sobre a mesma camada gráfica, a 1 Hz. A 0.1.1-dev introduz um segundo modo experimental de desenho e mantém a compatibilidade.

## Modos e uso no Windows

O modo HEADLESS mantém os testes de boot/RAM/CF/eventos sem interface. O modo SHARED desenha texto sobre a interface Canon, preservando todos os botões, com o problema de sobreposição esperado. **O modo FOCUS é explicitamente opcional**: enquanto o LCD estiver ligado e a tela bitmap estiver pronta, chama a função de preenchimento `bmp_fill` do Magic Lantern para cobrir a área 720x240 e redesenha a interface a **4 quadros por segundo**, por 38 segundos; SET/roda são temporariamente consumidos e MENU sai do foco e volta a passar para Canon.

**Ainda compartilha a mesma camada de bitmap com a Canon; não é um framebuffer independente.** A abordagem pode reduzir a sobreposição, mas não garante eliminá-la. As chamadas gráficas novas podem congelar a câmera. Não há gravação de ROM, bootflags ou alteração de AUTOEXEC.BIN.

Abra PowerShell na pasta extraída, e com CF em D::

```powershell
# Novo modo foco — maior risco, experimental
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\prepare-test.ps1 -CardRoot D:\ -EnableFocusUI

# Modo anterior (texto sobre o menu Canon)
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\prepare-test.ps1 -CardRoot D:\ -EnableUI

# Diagnósticos sem LCD
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\prepare-test.ps1 -CardRoot D:\
```

O script apenas cria staging e backup no computador. Se já existir a pasta de saída, renomeie antes de repetir. Com a câmera desligada, copie só `SE.BIN` e o marcador `ML/RB6FOCUS.ON` (ou `ML/RB6UI.ON`) gerados para o CF. **Preserve o AUTOEXEC.BIN existente**, a pasta ML e todos os arquivos de fotos. Remova do CF qualquer marcador `RB6*` que não faça parte do modo escolhido, para evitar configurações misturadas. Marcadores antigos `RB5UI.ON` são ignorados pelo novo payload.

Abra o MENU Canon para ligar o LCD e, se estiver no modo FOCUS, use a roda ou SET para navegar; MENU devolve o controle ao menu Canon. Após a sessão de cerca de 40 segundos, verifique `RB6BOOT.TXT`, `RB6TEST.TXT`, `RB6EVENT.TXT` e `RB6FINAL.TXT`. O último registra `UI_BACKGROUND_FILLS_HEX`, `BITMAP_READY_SAMPLES_HEX` e `UI_NAV_CONSUMED_HEX`, mas **não prova ausência de sobreposição**.

Se congelar, desligue, retire bateria/CF se necessário e recopie o `SE.BIN` anterior guardado em `BACKUP/` antes de voltar a usar o cartão.

## O que permanece integrado

Cinco páginas Status/Memória/Entrada/Armazenamento/Sobre; botões, 48 eventos, alocação de 256 B e 4096 B, memória disponível, criação de tarefa, timestamps, teste de leitura/escrita e relatório final. Nenhuma funcionalidade de fotografia (CR2/JPEG) é habilitada nesta build. A 0.1.0-dev permanece intacta em `../preview-010/`.

**Documentação técnica e créditos:** veja `../preview-010/README.md`. Pesquisa e endereços a partir de Magic Lantern Team e jiijii0/Soup Engine GPLv2, especialmente `Canon-EOS-5D/src/ml/payload/film.c`, `cam.h` e `tools/romtool.py`. O preenchimento usa o entrypoint `bmp_fill` em `0x00981390` do ML 5DC 2012; botão MENU é tratado sem chamadas de CF no hook. O novo modo FOCUS ainda não foi validado na câmera.
