# Rebobina

**Um novo software embarcado, livre e modular para expandir as possibilidades de câmeras digitais clássicas.**

O **Rebobina será um programa próprio, executado na câmera**, desenvolvido a partir do conhecimento, dos mecanismos e, quando viável e legalmente compatível, de componentes de código aberto do [Magic Lantern](https://magiclantern.fm). O foco inicial é a **Canon EOS 5D Classic (firmware 1.1.1)**. O objetivo é oferecer uma alternativa modular ao Magic Lantern para esse modelo: interface própria, recursos fotográficos próprios e um sistema que permita adicionar extensões como o [Soup Engine](https://github.com/neoonzy/soup-engine) sem reescrever o executável principal a cada módulo.

**O produto principal não é um instalador do Magic Lantern nem um gerenciador de plugins para o Magic Lantern existente.** O Rebobina pretende executar no lugar da extensão Magic Lantern no cartão, preservando o firmware Canon na ROM. Um eventual instalador será somente uma ferramenta auxiliar. A decisão sobre quais partes do Magic Lantern serão reutilizadas ou adaptadas depende da pesquisa da implementação para 5D Classic.

> [!IMPORTANT]
> **Estado: protótipo de arquitetura no computador.** O Rebobina **ainda não funciona na câmera**, não instala Magic Lantern, não modifica firmware/boot flags e não gera um `AUTOEXEC.BIN` utilizável. Nenhum binário do Soup Engine está integrado ou homologado. Não copie os executáveis deste repositório para o CompactFlash.

## Saída protegida e primeiro perfil real de filme (host)

O Rebobina agora testa a publicação de JPEG em **duas fases**: arquivo temporário `.TMP`, sincronização simulada, fechamento e renomeação **sem substituir arquivo existente**. O código tem recuperação explícita de temporários após interrupção, porém **não há garantia de atomicidade no CompactFlash real**.

Uma integração host aplica o arquivo original `stocks/soup800.fs` através do motor Soup Engine a **pixels RGB sintéticos 8×8**, gerando um JPEG pelo codificador original e salvando-o nessa HAL em RAM. O teste alterou 183 componentes de cor e gerou um JPEG de 1.006 bytes. Não são fotos reais nem um CR2 da câmera.

Veja [saída segura: contrato e limites](docs/SAIDA-SEGURA.md) e [ADR 0004: publicação exclusiva](docs/decisions/0004-publicacao-exclusiva.md).

## HAL simulada e processamento de referência do Soup Engine

O Rebobina possui um **contrato C** de arquivos, memória e fila de tarefas e um mock **inteiramente em RAM**. Ele permite injetar falhas, leituras curtas, gravações parcialmente interrompidas e simular reinicialização **sem escrever no cartão CF**.

```sh
make hal-demo
make test
```

O motor original do Soup Engine para a 5D também pode ser compilado e testado **separadamente** no computador, numa revisão GPLv2 fixada. O fluxo baixa o upstream em `build/soup-upstream/`, adapta **apenas o utilitário desktop** ao Linux dentro dessa cópia temporária, roda seus 13 selftests e compila um teste de ligação entre `se_arena/se_rd` e a HAL Rebobina:

```sh
python3 tools/soup_host.py --fetch
```

Esse comando requer Git, compilador C, Make, Python e **acesso à internet**. Os testes incluem a **geração de um JPEG sintético de 8×8 pixels** pelo codificador real do Soup Engine, gravado somente na HAL em RAM, com teste de gravação interrompida e preservação de um arquivo fictício. Não importa o motor para o firmware Rebobina nem processa fotografias reais do usuário. Consulte [HAL SIMULADA](docs/HAL-SIMULADA.md), [Soup Engine no host](docs/SOUP-ADAPTACAO.md), [saída segura](docs/SAIDA-SEGURA.md) e [ADR 0003](docs/decisions/0003-hal-deterministica.md).

## Simulador visual — desenvolvimento sem leitor CF

O projeto agora oferece um **laboratório visual no navegador**. A interface é responsiva e suas ações são calculadas por `src/core/ui.c` e `src/core/manager.c`, executados no computador.

```sh
make web-sim
```

Abra **http://127.0.0.1:8765/** no navegador do **mesmo computador**. Pressione a lixeira para abrir o menu, use ↑/↓ e SET; em **Módulos**, SET ativa/desativa o Hello **somente no host**. É possível reiniciar a simulação a qualquer momento. Não precisa de câmera, leitor CF, arquivo `.FIR` ou Magic Lantern.

Leia o [manual do simulador visual](docs/SIMULADOR-VISUAL.md) e o [plano de desenvolvimento sem CF](docs/SPRINT-SEM-CF.md). **O simulador não emula o processador, o sistema VxWorks nem o LCD físico.**

## O que queremos construir

- **Programa embarcado completo:** inicialização a partir do CompactFlash, ciclo de vida de tarefas, interface própria e ferramentas fotográficas integradas ao firmware Canon em execução.
- **Núcleo modular:** carregamento, ativação e desativação de recursos por uma API versionada, com gerenciamento de memória e erros.
- **Sistema de extensões:** recursos como Soup Engine instaláveis e atualizáveis sem aplicar patches individuais no executável principal (meta futura, ainda não implementada).
- **Camada por câmera:** abstração de serviços específicos do hardware, começando pela Canon 5D Classic.
- **Ferramentas de desenvolvimento:** simulador, testes e validação automatizada para reduzir os riscos de usar código novo no equipamento.
- **Instalação assistida (secundária):** um utilitário para facilitar a preparação e recuperação do cartão quando existir uma versão embarcada validada.

O Magic Lantern já possui um sistema de módulos em várias versões. O Rebobina vai estudar e reutilizar esse legado quando fizer sentido, mantendo atribuição e GPLv2, mas terá **produto, interface, organização e evolução próprios**. Não é necessário reescrever do zero o que já foi resolvido pela comunidade.

## O que já funciona nesta versão inicial

- Um **protótipo do registro de módulos em C**, executado no computador;
- Um **simulador de interface em terminal** e um **simulador visual no navegador**, ambos conectados ao núcleo C;
- Um **gerenciador host de módulos** para ativar, desativar e testar falhas de módulos individualmente;
- Uma **HAL simulada C** para arquivos, tarefas e memória, com falhas injetáveis;
- Execução em CI dos **13 selftests do Soup Engine original**, sem incorporação ao firmware;
- Objetos relocáveis para **ARM946E-S**, sem firmware instalável;
- Uma **prova de módulos externos `.so` no Linux host**, sem suporte a câmera;
- Contrato inicial de API com verificação de versão, identificador e ciclo de vida;
- Exemplo `hello`, que demonstra inicialização e encerramento de módulo;
- Manifestos JSON validados por ferramenta Python, incluindo um *placeholder* do Soup Engine;
- Testes do registro e dos manifestos, com CI no GitHub Actions.

**Ainda não existe um executável embarcado do Rebobina:** não há inicialização própria na câmera, menu na câmera, carregador ARM de módulos, suporte a `.mo`, isolamento, instalação ou integração funcional com o Soup Engine. Há apenas carregamento experimental de `.so` no **Linux host**. Os manifestos descrevem intenções e compatibilidade planejada, não permissões efetivamente impostas.

## Estrutura

```text
include/rebobina/     Contrato inicial de módulos em C
src/core/             Registro, UI portável e gerenciador de módulos (host)
src/host/             Demos, simulador CLI e ponte C do simulador visual
modules/hello/        Módulo de exemplo executável no host
modules/soup-engine/  Manifesto de integração planejada
simulator/web/        UI visual local (HTML/CSS/JS, sem dependências remotas)
tools/simulator_server.py  Servidor HTTP somente localhost + ponte para C
tools/modules.py      Validação e criação de manifestos
tests/                Testes automatizados
docs/                 Arquitetura, segurança e roteiro
```

## Desenvolvimento atual e próximo marco

A Sprint 0 foi concluída como fundação host e a Sprint 2 agora inclui o laboratório visual e o gerenciador de módulos host. A Sprint 1 permanece aberta por dependências históricas: já existe um mapa preliminar dos hooks, da ROM e da memória da EOS 5D Classic, mas o executável Magic Lantern 2012 de referência continua sem hash verificado. O protótipo host foi criado para testar a API de módulos. **Ele não é o produto final**. Sem leitor CF e com a câmera ainda em firmware 1.1.0, o próximo passo é construir uma HAL simulada e integrar progressivamente o motor de processamento do Soup Engine, mantendo o primeiro boot físico bloqueado até a atualização e a validação da câmera 1.1.1. Um instalador desktop não é requisito para iniciar essa etapa.

## Primeiros comandos

Requisitos: compilador C11, `make` e Python 3.9 ou superior. Os comandos **não acessam a câmera**.

```sh
make demo          # compila e executa o protótipo C no computador
make hal-demo      # demonstra a HAL com dados somente em RAM
make test          # testa o registro C e a ferramenta Python
make check-modules # valida manifestos
make audit-platform # verifica consistência do inventário estático da 5D
make arm-objects    # compila objetos ARM (NÃO firmware)
make sim-ui         # simula navegação no terminal
make web-sim        # inicia interface visual em http://127.0.0.1:8765/
make plugin-demo    # carrega um .so confiável no Linux host
python3 tools/modules.py list
python3 tools/modules.py new minha-extensao --name "Minha extensão"
```

O último comando cria um manifesto e um README de rascunho para uma nova extensão; **não cria um binário de câmera**.

## Primeiro dispositivo

| Campo | Alvo inicial |
| --- | --- |
| Câmera | Canon EOS 5D Classic (2005), não Mark II/III/IV |
| Firmware Canon | 1.1.1 |
| Sistema embarcado | VxWorks / DIGIC II |
| Referência de integração | Magic Lantern específico da EOS 5D Classic |
| Primeiro módulo planejado | Soup Engine |

Não há promessa de compatibilidade com outros modelos. Cada adaptação futura precisará de pesquisa, validação e testes próprios.

## Documentação

- [Arquitetura proposta](docs/ARQUITETURA.md)
- [HAL simulada: contrato, falhas e testes](docs/HAL-SIMULADA.md)
- [JPEG em duas fases e recuperação de temporários](docs/SAIDA-SEGURA.md)
- [ADR 0004: publicação sem sobrescrita](docs/decisions/0004-publicacao-exclusiva.md)
- [ADR 0003: HAL determinística e tratamento de falhas](docs/decisions/0003-hal-deterministica.md)
- [Simulador visual: usar, arquitetura e testes](docs/SIMULADOR-VISUAL.md)
- [Plano de continuidade sem leitor CF](docs/SPRINT-SEM-CF.md)
- [ADR 0002: interface web conectada ao core C](docs/decisions/0002-simulador-host.md)
- [Preparação da adaptação do Soup Engine](docs/SOUP-ADAPTACAO.md)
- [Sprint 1: levantamento da Canon 5D Classic](docs/5DC-PLATAFORMA.md)
- [Sprint 2: compilação ARM e interface host](docs/SPRINT2-BASE.md)
- [Sprint 3: condições para primeiro boot](docs/SPRINT3-CHECKLIST.md)
- [Sprint 5: protótipo de módulos no Linux host](docs/SPRINT5-PROTOTIPO.md)
- [Sprint 6: plano de integração Soup Engine](docs/SPRINT6-SOUP-PORT.md)
- [Sprint 7: testes de estabilidade](docs/SPRINT7-TESTES.md)
- [Relatório de avanço nas sprints 2, 5 e 7](docs/RELATORIO-AVANCO.md)
- [Sprint 8: distribuição futura](docs/SPRINT8-DISTRIBUICAO.md)
- [Sprint 1: relatório de testes](docs/SPRINT1-TESTES.md)
- [Sprint 1: evidências e fontes](docs/REFERENCIAS-SPRINT1.md)
- [Contrato e criação de módulos](docs/MODULOS.md)
- [Segurança, boot e limites](docs/SEGURANCA.md)
- [Sprints e critérios de entrega](docs/SPRINTS.md)
- [Roadmap de alto nível](docs/ROADMAP.md)
- [Como contribuir](CONTRIBUTING.md)
- [Orientações para agentes e colaboradores](AGENTS.md)

## Créditos e licença

O Rebobina não é afiliado à Canon nem à equipe Magic Lantern. Reconhecemos o trabalho da comunidade do [Magic Lantern](https://github.com/reticulatedpines/magiclantern_simplified) e dos autores do [Soup Engine](https://github.com/jiijii0/soup-engine). **O simulador, a HAL e o gerenciador host são implementações originais do Rebobina; o código do Soup Engine é baixado em uma cópia temporária externa para executar testes do upstream fixado. Nenhum código Soup foi incluído no firmware Rebobina.** A pesquisa de integração utiliza referências externas e mantém as obrigações GPLv2. Integrações futuras deverão respeitar direitos autorais, atribuições e licenças dos componentes reutilizados.

Código original do Rebobina licenciado sob **GPL-2.0-only** — veja [LICENSE](LICENSE). Projeto experimental, **sem garantias**.
