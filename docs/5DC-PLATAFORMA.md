# Canon EOS 5D Classic 1.1.1 — levantamento técnico

**Sprint 1 | 05/11/2026 | Status: pesquisa estática em andamento**

> Este documento não é um guia de instalação nem atesta um firmware seguro. Os endereços apresentados são **afirmações do código do port Soup Engine** para um binário Magic Lantern específico de 2012. Não foram medidos pelo Rebobina em hardware. Nenhum endereço deve ser reutilizado sem validar a build, a finalidade e o contexto de execução.


> **Fonte histórica localizada em 12/11/2026:** [Magic Lantern 2013 com platform/5DC.111](https://github.com/frantony/magic-lantern/tree/a3454ebed3f6adcd165ee9382f451f917d989253/platform/5DC.111). Inclui código VxWorks de inicialização, eventos, endereços de ROM e configuração de memória específicos da 5D Classic. Esta árvore está num **fork comunitário com commit fixado**, sem comprovar correspondência com o executável 2012 usado pelo Soup. Parte da pesquisa foi reaproveitada com créditos e **testes offline**; consulte [reuso do Magic Lantern](ML-5DC-REUSO.md).

## 1. Identidade do equipamento e proveniência

- **Modelo:** Canon EOS 5D *Classic* (primeira 5D), firmware Canon **1.1.1**. Não é 5D Mark II.
- **Plataforma:** DIGIC II, CPU ARM946E-S e **VxWorks 5.5**. A referência histórica de 24/04/2012 descreve a obtenção de execução via boot flag e `autoexec.bin` na 5D original.
- **Fontes fixadas por commit:** `jiijii0/soup-engine@6422c86b4530a5bdb4c5619f5287044ae7c661e3` e `reticulatedpines/magiclantern_simplified@142472e95054d73640f50789ac0fd66325499d82`. Estes hashes são de **código-fonte**, não do executável de câmera.
- **Atenção à versão do Magic Lantern:** a árvore `platform/` do branch `dev` do projeto `magiclantern_simplified`, neste snapshot, **não possui `5DC.111/`**; possui modelos 5D Mark II/III/IV/5DSR, que são diferentes. **Não tratar esse repositório como uma toolchain comprovada para a 5D Classic.**
- A compilação concreta de Magic Lantern usada pelo Soup Engine é descrita como **build 2012** e requer um `vendor/magiclantern-5Dc/autoexec.bin` externo. **Seu SHA-256 ainda não foi estabelecido**. Esse é um bloqueio para compilação e testes de compatibilidade embarcada confiáveis.

### Fontes primárias ou diretas

1. [Relato de desenvolvimento da 5D Classic em 2012 — ml-devel](https://groups.google.com/g/ml-devel/c/47_XE_cwrdQ).
2. [Soup Engine: README técnico da 5D — commit fixo](https://github.com/jiijii0/soup-engine/blob/6422c86b4530a5bdb4c5619f5287044ae7c661e3/Canon-EOS-5D/src/README.md).
3. [Soup Engine: patch-film.py — endereços/hook e verificações](https://github.com/jiijii0/soup-engine/blob/6422c86b4530a5bdb4c5619f5287044ae7c661e3/Canon-EOS-5D/src/tools/patch-film.py).
4. [Soup Engine: cam.h — funções Canon/ML](https://github.com/jiijii0/soup-engine/blob/6422c86b4530a5bdb4c5619f5287044ae7c661e3/Canon-EOS-5D/src/ml/payload/cam.h).
5. [Soup Engine: start.S — entradas e tarefas](https://github.com/jiijii0/soup-engine/blob/6422c86b4530a5bdb4c5619f5287044ae7c661e3/Canon-EOS-5D/src/ml/payload/start.S).
6. [Soup Engine: payload.ld — regiões de memória sugeridas pelo port](https://github.com/jiijii0/soup-engine/blob/6422c86b4530a5bdb4c5619f5287044ae7c661e3/Canon-EOS-5D/src/ml/payload/payload.ld).
7. [Magic Lantern histórico — 5DC.111, commit 2013 fixado](https://github.com/frantony/magic-lantern/tree/a3454ebed3f6adcd165ee9382f451f917d989253/platform/5DC.111).
8. [Magic Lantern moderno: arquitetura documentada — foco em DryOS, NÃO 5D Classic](https://github.com/reticulatedpines/magiclantern_simplified/blob/142472e95054d73640f50789ac0fd66325499d82/developer_guide/03_00_architecture.md).

O inventário legível por máquina está em [`data/5dc.111-evidence.json`](data/5dc.111-evidence.json). Ele é validado por testes offline para impedir erros de edição, mas isso **não valida as hipóteses no hardware**.

## 2. Inicialização: confirmado, inferido e pendente

| Etapa | Evidência | Confiança |
| --- | --- | --- |
| Ativar modo de inicialização por cartão | Relato de 2012 mostra boot flag e execução de `autoexec.bin` na 5D Classic | Histórico primário, não reproduzido |
| Firmware Canon original | Código externo roda a partir do cartão; isso não implica regravar o firmware original | Modelo operacional, necessita verificação do instalador |
| Iniciar base Magic Lantern 2012 | O patch do Soup Engine pressupõe a imagem relocada em `0x00980000`, partindo de `AUTOEXEC.BIN` de tamanho **221.184 bytes** | Específico do port e build, não verificado com hash |
| Modificação pelo Soup Engine | Stub em `0x00980820`; redirecionamento de evento GUI e da entrada da tarefa de inicialização | Diretamente observado no script |
| Carregar `SE.BIN` | `mlstub.S` inicializa o ML original, lê `A:/SE.BIN` em `0x009D0000`, testa cabeçalho `FILM` e transfere controle | Diretamente observado em assembly |
| Inicialização Rebobina | Ainda não há loader, `AUTOEXEC.BIN`, HAL ou teste físico | **Não implementado** |

**Importante:** `magiclantern_simplified/developer_guide/03_00_architecture.md` descreve principalmente **DryOS** e informa `autoexec.bin` em `0x00800000`. Esse endereço/fluxo NÃO pode ser assumido como uma descrição precisa do bootstrap VxWorks da 5DC. O `0x00980000` aqui é o endereço da **imagem relocada do ML 2012 relatado pelo Soup**, não o endereço universal do bootloader.

As mensagens de comentário no início de `patch-film.py` e `mlstub.S` ainda mencionam versões anteriores que carregavam o módulo ao escolher um item do menu. O corpo do código atual usa **`se_boot` na tarefa de inicialização**. Para a pesquisa, priorizar o comportamento do código sobre comentários históricos contraditórios.

## 3. Pontos de integração relatados pelo Soup Engine

| Endereço (somente esta build) | Função relatada | Origem |
| --- | --- | --- |
| `0x00980820` | Região do stub dentro da imagem ML relocada | `patch-film.py` |
| `0x0098090C` | Limite final do stub; região seguinte contém veneers em uso | `patch-film.py` |
| `0x00980774` | Literal da entrada da tarefa init | `patch-film.py` |
| `0x00980440` | Tarefa init original do ML | `patch-film.py` |
| `0x00989DB4` | Chamada para handler de eventos GUI redirecionada | `patch-film.py` |
| `0x0098A0F0` | Handler original de eventos GUI do ML | `patch-film.py` |

Esses valores são úteis para entender **onde o projeto existente se integra**, mas **não são um contrato de ABI do Rebobina**. Não aplicar os patches em executáveis que não tenham procedência e hash aprovados.

## 4. Serviços Canon e limites da HAL

O [`cam.h`](https://github.com/jiijii0/soup-engine/blob/6422c86b4530a5bdb4c5619f5287044ae7c661e3/Canon-EOS-5D/src/ml/payload/cam.h) registra estes serviços específicos da ROM da **Canon 5D Classic 1.1.1**:

| Serviço | Endereço relatado | Serviço Rebobina futuro |
| --- | --- | --- |
| FIO_OpenFile / FIO_ReadFile | `0xFFAE5B1C` / `0xFFAE5B54` | HAL de arquivos |
| FIO_WriteFile / FIO_CloseFile | `0xFFAE5B6C` / `0xFFAE5B78` | Escrita/encerramento controlados |
| AllocateMemory / FreeMemory | `0xFFB2F664` / `0xFFB2F680` | Alocação e limites |
| task_create | `0xFFB18B40` | Scheduler/tarefas |
| GUI_Control | `0xFF8190D0` | Interface/eventos |
| msleep | `0xFFB187F4` | Temporização |

O port aponta um **alias uncached `0x10000000`** para buffers acessados por DMA. Isso sugere que a HAL de arquivos/captura deve tratar coerência de cache explicitamente. **Não copiar esse esquema para outro modelo sem medir e comprovar.**

Na interface, o port relata que chamadas diretas a `TurnOnDisplay`/`ShowBitmap` geraram congelamento; usar `GUI_Control` para pedir que a Canon abra seu próprio menu foi a estratégia que funcionou naquele código. Isso é evidência de engenharia do projeto existente, não promessa de segurança de uma nova interface.

## 5. Memória e tarefas: mapa de referência, não reserva comprovada

O linker script do Soup Engine define:

| Região | Início | Tamanho | Fim exclusivo |
| --- | --- | --- | --- |
| Payload | `0x009D0000` | 128 KiB | `0x009F0000` |
| Pilha da interface | `0x009F0000` | 16 KiB | `0x009F4000` |
| Pilha do watcher | `0x009F4000` | 48 KiB | `0x00A00000` |

O comentário do projeto afirma que o ML 2012 reservou `0x00980000` a `0x00A00000` diminuindo a heap Canon. **Ainda faltam um mapa de símbolos do binário-base e provas em hardware** para declarar a ocupação segura. O `start.S` coloca a interface e o watcher em pilhas separadas; o handler de botões **não troca para uma pilha usada por outra tarefa**.

## 6. Lacunas e riscos para o Rebobina

| Risco | Consequência | Verificação necessária |
| --- | --- | --- |
| Build 2012 sem hash confiável | Patch aplicado em imagem incompatível | Obter base legítima e comparar SHA-256, instruções, mapa de símbolos |
| Documentação DryOS confundida com VxWorks | Boot ou endereços incorretos | Pesquisar especificamente 5DC.111, não 5D2/5D3 |
| Endereços ROM/ML fixos | Travamentos ao trocar firmware/build | HAL amarrada a variante e teste em hardware |
| Layout de RAM e pilhas inferido | Corrupção silenciosa, falha de tarefas | Link map, probes e stack canaries no hardware |
| Display compartilhado com Canon | Travamento da interface durante captura | Simulador + testes de GUI e captura reais |
| Leitura/gravação concorrente em CF | JPEG incompleto, RAW em uso | Testes de falha de I/O, interrupção e gravação ativa |
| Módulos sem MMU isolada | Módulo pode comprometer câmera inteira | ABI restrita, limites conhecidos e ciclo de vida testado |
| Boot inválido | Nosso código pode não chegar a executar | Plano externo de recuperação, sem promessas de fallback |

## 7. Proposta de fronteira do código

- `camera/5DC.111/boot`: detalhes do bootstrap, **somente depois de fonte/firmware validados**.
- `camera/5DC.111/hal`: adaptadores de arquivos, tarefas, tela, botões e memória.
- `src/core/`: registro de serviços e ciclo de vida sem endereços absolutos.
- `modules/`: plugins com interface versionada, independentes dos endereços Canon.
- `tools/`: validação **somente leitura**, testes, simulador e ferramentas auxiliares.

O primeiro passo deve ser **reutilizar conhecimento verificável**, não copiar cegamente o binário do Magic Lantern nem definir um `AUTOEXEC.BIN` só porque a compilação passou.

## 8. Critérios que continuam pendentes

- [x] Confirmar modelo/firmware-alvo e VxWorks/DIGIC II por fontes.
- [x] Fixar snapshot das referências de código atuais.
- [x] Documentar hooks, serviços e mapa de memória **relatados** com origem.
- [x] Mapear integração e divergências entre comentários e implementação Soup.
- [x] Definir riscos e limites de evidência.
- [ ] Identificar **exatamente** o binário Magic Lantern 2012 utilizado: download verificável, SHA-256 e relação com código-fonte reproduzível.
- [ ] Obter fonte original do port 5DC.111 com procedência suficiente para validar seu boot e linker.
- [ ] Confirmar em hardware os endereços, a memória disponível e o procedimento de recuperação.

**Resultado:** pesquisa técnica e testes offline podem avançar, mas a Sprint 1 ainda não é considerada concluída para uma implementação embarcada segura.
