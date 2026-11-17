# REBOBINA 0.1.0-dev — sessão integrada na Canon EOS 5D Classic

**Status: compilado e validado offline, ainda não testado na câmera.**

Este é um salto de desenvolvimento depois dos testes **0.0.1**, **0.0.3** e **0.0.4** executados com sucesso na 5D Classic 1.1.1. Em vez de editar e reiniciar para cada subsistema, o novo pacote reúne tudo em um único `SE.BIN`, acessível a partir do carregador legado Soup Engine (`AUTOEXEC.BIN`) que já funciona no cartão de testes.

**Não é firmware independente, não instala bootflags, não altera ROM Canon e não traz o Soup Engine como plugin.** O software é um payload próprio REBOBINA, licenciado sob GPLv2, carregado pela infraestrutura aberta já existente.

## Recursos da sessão única

| Recurso | Implementação | Prova na câmera |
|---|---|---|
| Boot ARM946E-S | Assinatura FILM, entradas +0x04 e +0x44 | `RB5BOOT.TXT` |
| HAL 5D.111 | Funções Canon ROM separadas do núcleo | `RB5BOOT.TXT`, `RB5TEST.TXT` |
| Arquivos | Criar, escrever, fechar, abrir, ler e comparar | `READBACK=PASS` |
| Memória | Alocar/verificar/liberar blocos de 256 e 4.096 bytes | `MEM_256_HEX=1`, `MEM_4096_HEX=1` |
| Informações do sistema | Tick VxWorks, total/livre em RAM via API Canon | `TOTAL_RAM_HEX`, `FREE_RAM_HEX` |
| Concorrência | Uma tarefa de diagnóstico, prioridade 31 e stack 8 KiB | `WORKER_ID_HEX`, relatório tardio |
| Eventos GUI | 48 primeiras entradas (tipo + parâmetro), contador total e descartes | `RB5EVENT.TXT` |
| Mapeamento de botões | SET, MENU, INFO, PLAY, lixeira, roda, direcionais | `tools/read-results.py` |
| Menu REBOBINA | Cinco páginas: status, memória, botões, arquivos, projeto | LCD quando habilitado |
| Navegação | SET/roda alteram a página interna, **sem consumir eventos Canon** | `UI_PAGE_HEX` |
| Tela | `bmp_printf` do ML histórico, apenas se LCD **já** estiver ligado | `UI_DRAW_CALLS_HEX`, observação visual |
| Diagnóstico final | Integridade, eventos, desenho, monitoramento e arquivo final | `RB5FINAL.TXT` |
| Modos no mesmo binário | **HEADLESS** (sem desenho) / **FULL UI** (marcador) | `UI_ENABLED_HEX` |

### Limitações deliberadas

- Não liga a tela por conta própria: o programa tenta desenhar somente com a tela já ligada pela Canon/ML.
- **A interface gráfica da versão 0.1.0-dev ainda não foi validada em hardware e pode travar a câmera.** Quando ativada, ela pode competir visualmente com os menus Canon/ML. O modo HEADLESS não chama `bmp_printf`.
- A navegação é **passiva** e os botões continuam exercendo suas funções na interface Canon, portanto não é ainda um menu independente com controle exclusivo.
- A tarefa de demonstração opera por cerca de **39 segundos** e depois produz o resumo; a interface não permanece executando indefinidamente. Isso não é ainda um sistema de módulos carregados dinamicamente.
- Não acessa o pipeline de captura, CR2/JPEG ou informações privadas da ROM. Não modifica fotos nem arquivos de configuração de terceiros.
- O tamanho do payload e o layout da RAM são verificados estaticamente, mas só a execução na câmera poderá confirmar estabilidade.

## Executar apenas UMA sessão de teste

Use o **CF de desenvolvimento que já inicializa o Soup**, com a câmera desligada e backup preservado. Não reinstale Magic Lantern e **não substitua `AUTOEXEC.BIN`**.

### Windows — preparar sem alterar o CompactFlash

Extraia o ZIP em uma pasta e abra PowerShell nela. Supondo que o CF esteja como `D:`:

**Modo completo (menu/LCD incluídos):**

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\prepare-test.ps1 -CardRoot D:\ -EnableUI
```

**Modo diagnóstico sem LCD (caso queira reduzir o risco, mesmo SE.BIN):**

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\prepare-test.ps1 -CardRoot D:\
```

Para voltar de FULL UI para HEADLESS, **apague manualmente `ML/RB5UI.ON` do CF com a câmera desligada**; deixar o marcador no cartão continua habilitando o desenho. O script recusa preparar HEADLESS se detectar esse marcador.

O script **não escreve no cartão**. Ele verifica o carregador e a integridade do novo binário e cria em `Downloads/REBOBINA-010-TESTE/` as pastas `BACKUP/` e `COPIAR-PARA-CF/`. Não apague essa pasta de backups. Se o diretório já existir, o script recusa sobrescrevê-lo; renomeie o anterior antes de preparar outra cópia.

1. Faça backup das fotos e do `SE.BIN` que está no CF (a ferramenta também copia o SE antigo para uma pasta no computador).
2. Com a câmera **desligada**, copie **somente o conteúdo** de `COPIAR-PARA-CF/` para a raiz do CF. No modo completo, esse conteúdo terá `SE.BIN` e `ML/RB5UI.ON`. Preserve os demais arquivos.
3. Ejete o CF com segurança e ligue a câmera. Se ativou o modo completo, abra um menu Canon com MENU para que o LCD fique ligado; tente navegar pelo menu REBOBINA com SET ou roda durante a sessão. Os comandos também continuam sendo processados pela Canon.
4. Aguarde o término do diagnóstico (aproximadamente 40 segundos); desligue a câmera normalmente. Não tire fotos nem entre em menus de apagar arquivos durante este experimento.
5. Remova o CF, conecte ao PC e leia os resultados:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\read-results.ps1 -CardRoot D:\
```

Ou use `python tools/read-results.py D:\` para os códigos dos botões traduzidos. Envie os quatro relatórios, e uma foto do LCD se a interface apareceu.

### Arquivos esperados na raiz do CF

- `RB5BOOT.TXT` — entrada, memória, tick e tarefa; criado imediatamente.
- `RB5TEST.TXT` — teste de leitura de volta do arquivo de boot e memória disponível; criado cedo, **antes do desenho**.
- `RB5EVENT.TXT` — captura de botões; criado depois da janela de ~38s.
- `RB5FINAL.TXT` — estado geral e número de tentativas de desenho; criado por último.

`UI_DRAW_CALLS_HEX` diferente de zero **não garante texto visível**. Se `RB5TEST` existir mas `RB5FINAL` não, a tarefa pode ter parado antes de terminar, inclusive por chamada gráfica experimental; não deduza automaticamente a causa.

### Rollback

Se a câmera congelar ou exibir `Err`, desligue, retire bateria e CF se necessário, **não force novos testes**. Com o cartão no computador, restaure `BACKUP/SE-ANTERIOR.BIN` da pasta de preparo como `SE.BIN` na raiz. Não mexa no `AUTOEXEC.BIN`, nas bootflags ou no firmware. Se a câmera não iniciar, remova o CF de teste e interrompa os experimentos.

## Compilar e testar no computador

Linux/WSL com `clang`, `lld`, `llvm-objcopy`, `readelf`, Python 3.9+:

```sh
make clean all test
```

Gera `dist/SE.BIN` e `dist/manifest.json`. Os testes de host simulam o ciclo completo, navegação, falhas de alocação, tarefa, arquivos e saturação dos 48 slots. O validador inspeciona formato FILM, branches ARM, mapa ELF, ausência de `.bss` e de relocações dinâmicas. **Nenhuma dessas verificações substitui o teste físico.**

## Estrutura

- `boot/entry.S`: contrato Soup (`FILM`, jump `+0x04`, hook `+0x44`).
- `boot/payload.ld`: endereço histórico `0x009D0000` e restrições de layout.
- `boot/hal_5dc.c`: funções ROM específicas da 5D 1.1.1 e renderizador ML histórico.
- `include/rb_hal.h`: API de serviços desacoplada do core.
- `core/rb_core.c`: estado, inicialização, cinco páginas, captura e relatório.
- `tests/`: HAL simulada em C e testes de validação e interpretação Python.
- `tools/`: staging seguro, leitura do relatório e validador binário.

## Crédito e licença

**Magic Lantern Team**: engenharia reversa, mapeamento de eventos e a imagem histórica da Canon 5D Classic (GPLv2). **jiijii0 / Soup Engine**: carregador, pesquisa das chamadas de firmware, coerência de cache e ABI de `SE.BIN` (GPLv2). Código novo do REBOBINA: GPL-2.0-only, sem afiliação à Canon, Magic Lantern ou Soup Engine.

Referências:
- https://github.com/jiijii0/soup-engine/tree/master/Canon-EOS-5D
- https://github.com/jiijii0/soup-engine/blob/master/Canon-EOS-5D/src/ml/payload/cam.h
- https://github.com/frantony/magic-lantern/blob/a3454ebed3f6adcd165ee9382f451f917d989253/platform/5DC.111/gui.h

**Experimento sem garantia. Não é versão de uso cotidiano.**
