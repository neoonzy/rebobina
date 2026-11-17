# REBOBINA 0.0.3 — teste integrado da 5D Classic

**Candidato embarcado experimental.** Build ARM compilado e verificado estruturalmente; **ainda não testado na câmera**. É uma continuação do Boot Probe **0.0.1 que criou `REBOBINA.TXT` na Canon EOS 5D Classic 1.1.1 do desenvolvedor em 16/11/2026**. O novo 0.0.3 não significa boot independente: depende do `AUTOEXEC.BIN` modificado do Soup Engine no cartão já inicializável.

## O que testa em UMA sessão

| Capacidade | Evidência gerada | Observação |
|---|---|---|
| Entrada ARM | `RB3BOOT.TXT` | Estágio mais importante: boot entrou no novo payload |
| Temporizador VxWorks | `TICK_BEFORE`, `TICK_AFTER` | Leitura de clock + pausa curta |
| Memória da Canon | `MEM_256=PASS/FAIL` | Aloca 256 bytes, testa padrão e libera |
| Criação de tarefa | `WORKER_ID_HEX` | Tarefa de diagnóstico com espera de 8 segundos; ID `FFFFFFFF` indica erro |
| Gravação no CF | `RB3BOOT.TXT` | Cria/grava/fecha arquivo próprio, não toca fotografias |
| Leitura do CF | `RB3LATE.TXT: READBACK` | Relê o primeiro arquivo usando o alias não-cacheado do VxWorks |
| Eventos de botões | `GUI_EVENTS_HEX`, `RAW_EVENT_*` no `RB3LATE.TXT` | Registra até 12 palavras brutas; **todos os eventos são repassados** à Canon/ML |
| LCD (opcional) | `LCD=CALL_ATTEMPTED` | Apenas se arquivo `ML/RB3LCD.ON` existir **e** painel já estiver ligado; não comprova desenho visível |
| LED (opcional) | Pisca no início/fim | Apenas se `ML/RB3LED.ON` existir; MMIO experimental não homologado |

**Não testa:** captura de fotografias, troca de firmware, processamento RAW, Soup como plugin, menu Rebobina ou carregamento independente. Nenhuma alteração em bootflags, ROM, `AUTOEXEC.BIN` ou `DCIM` é implementada neste payload.

## Para testar no seu Windows (cartão CF já funcional)

1. **Preserve os dois cartões e seus backups.** Câmera desligada, faça backup das fotos e do `SE.BIN` atualmente funcional; não execute instaladores `.FIR` nem mude firmware/bootflags.
2. Extraia o ZIP deste diretório. Com o CF no PC como `D:`, abra PowerShell na pasta extraída e rode:

   ```powershell
   powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\prepare-test.ps1 -CardRoot D:\
   ```

   Ele examina o `AUTOEXEC.BIN` e gera uma pasta em `Downloads\REBOBINA-003-TESTE` com backup e candidato. **Não escreve no CF**. Se a checagem falhar, não force.
3. Copie **somente** `COPIAR-APENAS-SE.BIN\SE.BIN` para a raiz do CF como `SE.BIN`, substituindo o anterior. **Não substitua o `AUTOEXEC.BIN`.** Os marcadores LCD/LED são **opcionais e desativados por padrão**; não os use no primeiro teste por serem experimentais.
4. Ejetar CF com segurança, ligar a câmera. Durante os primeiros **8 segundos**, pressione alguns botões não relacionados a disparo, como MENU, INFO, SET e direcionais, observando se a câmera continua respondendo. Evite disparar fotos ou apagar arquivos nesse primeiro teste.
5. Desligue a câmera normalmente. Retire o CF e examine `RB3BOOT.TXT` e `RB3LATE.TXT` na raiz. Para exibir ambos no PC:

   ```powershell
   powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\read-results.ps1 -CardRoot D:\
   ```

6. Envie o conteúdo dos dois arquivos para análise. **Se travar, surgir `Err` ou não responder**, desligue, remova bateria e CF. Restaure `BACKUP\SE-ANTERIOR.BIN` como `SE.BIN` no CF com a câmera desligada. Se persistir, retire o CF e não continue os testes.

Se `RB3BOOT.TXT` existir e `RB3LATE.TXT` não, o payload iniciou, mas a tarefa tardia pode não ter sido criada, pode ter falhado ou pode ter ocorrido erro de I/O. **Não atribua uma causa única sem dados.** O campo `WORKER_ID_HEX` ajuda a separar as possibilidades.

### LCD e LED — opt-in, não homologados

O teste-padrão **não mexe no LCD nem no LED**. Para preparar um teste adicional com chamada gráfica, adicione o marcador `ML/RB3LCD.ON` no cartão. A rotina só tenta `bmp_printf` se detectar o display ligado na janela de observação; chamar a função não garante visibilidade. Para LED, marcador `ML/RB3LED.ON`. Não inclua esses dois testes sem aceitar que a câmera pode congelar, exigir retirada da bateria ou apresentar `Err`. A ferramenta de staging aceita as opções `-EnableLCD -EnableLED`, mas apenas gera arquivos na pasta do computador.

## Build e testes locais

```sh
make clean all test
```

Requisitos: `clang` com `--target=arm-none-eabi`, `lld`, `llvm-objcopy`, `readelf`, `python3` e compilador host. Gera `dist/SE.BIN`, `dist/manifest.json`. O validador confere `FILM`, branches ARM em `+0x04/+0x44`, símbolos e ausência de `.bss`/relocações. Testes host fazem injeção de falhas; **não são testes de hardware**.

O carregador Soup tem limite de leitura de 160 KiB. O payload é ligado em `0x009D0000`, como definido pelo Soup, e os endereços de funções Canon são específicos da **5DC.111**. Mesmo um binário estruturado corretamente pode travar a câmera.

## Arquitetura e atribuições

- `boot/entry.S` e `boot/payload.ld`: contrato do carregador histórico Soup para 5DC.111.
- `boot/probe.c`: diagnóstico próprio REBOBINA com I/O, relógio, memória, tarefa e eventos passivos.
- `tools/validate_probe.py`: validação offline e manifesto de integridade.
- `tools/prepare-test.ps1`: staging e backups sem escrever no CF.
- `tools/read-results.ps1`: leitura dos relatórios.
- `tests/`: testes de simulação e falhas.

Baseado em pesquisa GPLv2 de [Magic Lantern Team](https://github.com/reticulatedpines/magiclantern_simplified) e [Soup Engine de jiijii0](https://github.com/jiijii0/soup-engine), principalmente `Canon-EOS-5D/src/ml/payload/cam.h`, `start.S` e `mlstub.S`. Nenhum binário de terceiros ou firmware Canon é redistribuído. Código REBOBINA sob GPL-2.0-only. **Distribuição experimental, sem garantia.**
