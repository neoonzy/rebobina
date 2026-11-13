# Sprints — Rebobina

> **Produto:** software embarcado próprio para a Canon EOS 5D Classic (firmware 1.1.1), inspirado no Magic Lantern, com interface própria e módulos extensíveis. Não é um instalador de Magic Lantern.
>
> **Planejamento inicial, em 5 de novembro de 2026.** Sprints são ciclos de entrega orientados por objetivos e critérios verificáveis, não promessas de prazo. Sugestão de cadência: duas semanas para desenvolvimento host/pesquisa; as atividades em hardware ficam condicionadas a evidências e disponibilidade de uma câmera de teste.
>
> **Regra:** executar testes em host e simulação não significa validar na câmera. Não distribuir `AUTOEXEC.BIN`, `.FIR` ou payloads como seguros enquanto não houver testes físicos e procedimento de recuperação.

## Visão geral

| Sprint | Foco | Entrega principal | Estado |
| --- | --- | --- | --- |
| **0** | Fundação | Repositório, API protótipo, manifesto, CI e documentação | **Concluída no escopo host** |
| **1** | Engenharia reversa e especificação | Mapa de boot, APIs, memória e decisões de reaproveitamento GPLv2 da 5D | **Em andamento: levantamento e testes offline realizados; binário-base ainda não identificado** |
| **2** | Toolchain e simulação ARM | Build reproduzível ARM e ambiente mínimo de testes do port | **Objetos ARM e ELF stage0 offline agora testáveis; boot físico/port Canon pendentes** |
| **3** | Primeiro executável embarcado | Rebobina mínimo inicializando de forma controlada na 5D de teste | **ELF ARM próprio ligado offline; cadeia de boot Canon, handoff, recuperação e hardware pendentes** |
| **4** | HAL e interface própria | Interface inicial, botões, arquivos, logs e serviços comuns | **Prova visual no host concluída; LCD/HAL em câmera pendentes** |
| **5** | Módulos embarcados | Carregador versionado e primeiro módulo simples em câmera | **Gerenciador host e loader .so Linux testados; ARM/câmera pendentes** |
| **6** | Integração Soup Engine | Soup Engine adaptado ao Rebobina, sem patch individual de `AUTOEXEC.BIN` | **CLI host CR2→preview→Soup→JPEG e HAL POSIX, saída comparada ao upstream; CR2 real/câmera pendentes** |
| **7** | Confiabilidade | Testes de interrupção, memória, compatibilidade e recuperação | **Testes host antecipados; testes reais pendentes** |
| **8** | Distribuição | Releases por firmware e instalador auxiliar seguro | A fazer |
| **9** | Ecossistema | SDK público, módulos extras e estudo de outras Canon | Futuro |

## Sprint 0 — Fundação do projeto

**Objetivo:** criar uma base de desenvolvimento que não se confunda com firmware utilizável.

- [x] Criar README, arquitetura, segurança, contribuição e diretrizes para agentes.
- [x] Implementar registro de módulos em C para **host** e módulo de demonstração.
- [x] Definir manifesto JSON experimental, CLI de criação e validação.
- [x] Adicionar testes C/Python e CI; a execução inicial no GitHub Actions passou.
- [x] Documentar versionamento do núcleo, ABI, módulos e releases em [VERSIONAMENTO.md](VERSIONAMENTO.md).
- [x] Criar `CHANGELOG.md` e templates de issue/PR.

**Concluída quando:** política de versões e rastreabilidade documentadas; CI host verde. **Conclusão: cumprida para a fundação host**, sem qualquer entrega embarcada.

## Sprint 1 — Entender a EOS 5D e o Magic Lantern

**Progresso (05/11/2026):** levantamento estático publicado em [5DC-PLATAFORMA.md](5DC-PLATAFORMA.md), [inventário de evidências](data/5dc.111-evidence.json), [ADR 0001](decisions/0001-boot-e-reuso.md) e [relatório de testes](SPRINT1-TESTES.md). Fontes fixadas por commit do Soup Engine e do ML moderno. **Não houve validação de binário-base nem teste físico.**

**Objetivo:** ter informações confiáveis para desenvolver o Rebobina embarcado sem partir de suposições perigosas.

- [ ] Fixar a compilação/fonte exata do Magic Lantern para **5DC.111**, registrando procedência e hashes quando disponíveis.
- [x] Documentar o fluxo conhecido por fontes históricas e pelo Soup Engine, **separando o que ainda falta verificar no bootstrap VxWorks**.
- [x] Mapear preliminarmente VxWorks/DIGIC II: tarefas, pilhas, memória, GUI e arquivos a partir do código Soup; **endereços ainda não validados em hardware**.
- [x] Identificar APIs e oportunidades preliminares de reaproveitamento GPLv2, com limites de compatibilidade e fonte.
- [x] Comparar hooks e carregamento do Soup Engine com a HAL e as necessidades do Rebobina.
- [x] Criar matriz de riscos distinguindo host, simulação e câmera.
- [x] Registrar ADR 0001, **provisória**, sobre reutilização, adaptação e limites do boot.

**Entregáveis:** `docs/5DC-PLATAFORMA.md`, `docs/decisions/0001-boot-e-reuso.md` e lista de recursos com suas fontes.

**Aceite:** cada endereço/hook conhecido tem origem e versão explícitas; limitações e incertezas ficam documentadas; nenhuma alegação de segurança depende apenas de compilação. **Pendente:** localizar e validar o binário-base exato de 2012 e a fonte correlata. A sprint não está encerrada.

## Sprint 2 — Toolchain e ambiente de testes ARM

**Progresso:** compilação `arm-none-eabi` via Clang apenas de **objetos relocáveis ARM946E-S**, menu portável e simulador de interface host. CI verificada em [execução 42129417751](https://github.com/neoonzy/rebobina/actions/runs/42129417751). Consulte [SPRINT2-BASE.md](SPRINT2-BASE.md). Não há linker executável nem simulação do sistema Canon.

**Objetivo:** compilar o projeto para a arquitetura da 5D sem ainda declarar suporte ao hardware.

- [x] Criar alvos de compilação de objetos ARM946E-S no host; **linker script base permanece pendente** até validação de memória.
- [x] Separar os protótipos host de `camera/5DC.111`, explicitando que só objetos são compilados.
- [x] Implementar **mock de arquivos, memória e tarefas** para host (não chamadas Canon): [HAL SIMULADA](HAL-SIMULADA.md).
- [ ] Validar dependências externas, tamanho de imagem, seções, ABI e limites de memória na compilação.
- [x] Automatizar testes de interface e ciclo de vida sob host; HAL Canon ainda pendente.
- [x] Registrar Clang/CPU alvo e verificação ELF; **a reprodutibilidade binária total da câmera ainda está pendente**.

**Entregável:** primeiro artefato ARM **experimental**, junto a relatórios de compilação e verificações automáticas.

**Aceite:** build determinístico e verificações automatizadas; nenhuma alegação de que o artefato é seguro para colocar no cartão.

### Desenvolvimento paralelo sem CompactFlash (06/11/2026)

A câmera real ainda estava em firmware **1.1.0** e não havia leitor CF para atualizar/testar. Portanto a [PR experimental de boot #1](https://github.com/neoonzy/rebobina/pull/1) **não foi validada em hardware nem mesclada como firmware pronto**.

Foram iniciadas, sem depender do CF:

- [x] Simulador visual de interface no navegador, comandado pelo core C ([manual](SIMULADOR-VISUAL.md)).
- [x] Servidor local somente leitura, protocolos de eventos e testes HTTP.
- [x] Gerenciador C de módulos individuais no host, com Hello demonstrativo.
- [x] ADR de simulação e fronteiras do Soup Engine ([adaptação](SOUP-ADAPTACAO.md)).
- [x] HAL simulada de arquivos, memória e tarefas com falhas injetáveis.
- [x] Executar selftests originais de processamento do Soup Engine no host ([CI](https://github.com/neoonzy/rebobina/actions/runs/42228272935)).
- [x] Gerar JPEG sintético do Soup por callback de escrita da HAL em RAM e validar erros de gravação ([CI](https://github.com/neoonzy/rebobina/actions/runs/42229454141)).
- [x] Aplicar `soup800.fs` real a pixels RGB sintéticos, publicar JPEG com rename exclusivo e testar recuperação de temporários ([SAIDA-SEGURA](SAIDA-SEGURA.md)).
- [x] Validar offset, tamanho e cabeçalho de JPEG no envelope CR2 sintético e produzir JPEG com o `previewcheck` original ([CR2-INSPECAO-HOST](CR2-INSPECAO-HOST.md)).
- [x] Criar CLI Linux de processamento completo do preview CR2, com HAL de saída exclusiva e comparação byte a byte com `previewcheck` no teste sintético ([guia](CR2-DESENVOLVIMENTO-HOST.md)).
- [x] Rodar três stocks e comparar JPEG byte a byte sob ASan/UBSan, com correção temporária de UB ([SOUP-SANITIZERS](SOUP-SANITIZERS.md)).
- [ ] Integrar o Soup ao fluxo Rebobina e comparar resultados JPEG/RAW **de fotos reais**.

[Plano de execução completo](SPRINT-SEM-CF.md). Esses componentes antecipam Sprints 4–6 **apenas no host**, sem alterar seus critérios físicos.

## Sprint 3 — Primeiro Rebobina na câmera

**Avanço offline em 13/11/2026:** o [bootstrap próprio](5DC-BOOTSTRAP-OFFLINE.md) possui entrada ARM946E-S, inicialização mínima e linker de pesquisa que geram **somente ELF**. Não usa instalação prévia nem binário do Magic Lantern para ligar, mas **não é carregável pela câmera** e não inclui handoff da Canon. O uso de pesquisa GPLv2 histórica está documentado com créditos; segurança, boot e recuperação continuam bloqueados.


**Objetivo:** atingir o primeiro marco real do produto: **um programa próprio executando na Canon EOS 5D Classic**.

- [ ] Definir procedimento de teste e recuperação em hardware de desenvolvimento.
- [ ] Integrar um bootstrap mínimo compatível com o firmware conhecido.
- [ ] Inicializar um serviço pequeno com diagnóstico visível ou verificável.
- [ ] Validar entrada, saída/desligamento e comportamento com módulo ausente.
- [ ] Capturar logs e resultados, registrando versão do firmware, cartão e hashes.
- [ ] Documentar falhas e procedimento de retorno à configuração anterior.

**Dependências:** Sprints 1 e 2 concluídas; uma 5D Classic de teste e um cartão de recuperação/preparação.

**Aceite:** inicialização e desligamento observados fisicamente em hardware de teste, com evidência documentada; falhas conhecidas e limitações expostas.

## Sprint 4 — HAL e interface Rebobina

**Objetivo:** ter uma interface própria, sem depender do menu do Magic Lantern como produto final.

- [ ] Encapsular botões, tela, temporização, arquivos e memória da 5D na HAL.
- [ ] Criar um menu Rebobina com navegação simples e contraste adequado.
- [ ] Exibir informações sobre versão, memória e erros.
- [ ] Garantir que as tarefas de interface não interrompam captura e gravação.
- [ ] Testar entrada/saída do menu, eventos de desligamento e tempos de resposta.

**Aceite:** navegação funcional em câmera de desenvolvimento com registro de falhas e critérios de desempenho definidos.

## Sprint 5 — Sistema de extensões embarcadas

**Progresso antecipado:** prova no Linux host de carregamento independente via `dlopen`, além de gerenciador C com ativação/desativação individual, erro de inicialização e teste pela UI visual, com testes positivos e negativos de ABI. **Não é o carregador da câmera.** Consulte [SPRINT5-PROTOTIPO.md](SPRINT5-PROTOTIPO.md).

**Objetivo:** adicionar funcionalidades sem alterar o executável principal a cada integração.

- [ ] Definir ABI embarcada versionada, APIs compartilhadas e convenções de recursos.
- [ ] Implementar carregamento validado de módulos, caso seja compatível com as restrições da 5D.
- [ ] Implementar verificação de compatibilidade e ciclo de vida seguro para módulos.
- [ ] Integrar ao menu ações de ativar, desativar e visualizar erros.
- [ ] Criar módulo `hello` para câmera, separado do exemplo host.
- [ ] Testar módulos ausentes, incompatíveis e falhas de inicialização.

**Aceite:** instalar ou trocar um módulo de teste no cartão **sem recompilar o núcleo**, com resultados documentados em hardware. Se o carregamento dinâmico não couber na plataforma, registrar ADR e alternativa compatível.

## Sprint 6 — Soup Engine como extensão

**Objetivo:** transformar o Soup Engine em um módulo do Rebobina, não apenas iniciar seu binário antigo.

- [ ] Identificar as dependências de inicialização, interface, tarefas, arquivos e memória do Soup Engine da 5D.
- [ ] Isolar o motor de processamento das adaptações específicas do Magic Lantern.
- [ ] Mapear chamadas para a HAL/API de módulos Rebobina.
- [ ] Construir interface de seleção dos perfis `.fs`.
- [ ] Validar geração de JPEG adicional, preservação de CR2 e comportamento JPEG/RAW.
- [ ] Documentar atribuições, GPLv2 e condições de redistribuição.

**Aceite:** módulo integrado ao menu Rebobina, processando amostras em simulação e depois em câmera, com comparação de saída e integridade dos RAWs.

## Sprint 7 — Estabilidade e recuperação

**Progresso antecipado:** testes de fronteira e 100 mil eventos de interface no host, testes negativos de plugins e job UBSan aprovados na [CI 42129417751](https://github.com/neoonzy/rebobina/actions/runs/42129417751). [Plano de testes](SPRINT7-TESTES.md). Ainda sem hardware.

**Objetivo:** reduzir regressões e tornar o software adequado a testes repetidos.

- [ ] Testar interrupção de energia, desligamento, remoção de cartão e arquivos incompletos, dentro de procedimentos controlados.
- [ ] Auditar tamanhos de pilha, arena, corrupção de entrada e concorrência com gravação de fotos.
- [ ] Adicionar logs de falha, marcadores de versão e testes de recuperação de módulos.
- [ ] Validar que alterações de um módulo não introduzam regressões no núcleo.
- [ ] Criar matriz de compatibilidade por firmware, build e cartão.

**Aceite:** suíte repetível, limites conhecidos, incidentes documentados e critérios mínimos de uso experimental satisfeitos.

## Sprint 8 — Releases e instalação opcional

**Objetivo:** distribuir um programa embarcado experimental de modo reprodutível, com instruções seguras.

- [ ] Automatizar build e empacotamento por câmera/firmware.
- [ ] Gerar checksums, notas de versão e guia de recuperação.
- [ ] Criar instalador auxiliar que **verifique primeiro**, faça backup e prepare um cartão sem escrita silenciosa.
- [ ] Exibir incompatibilidades e exigir confirmação antes de operações em mídia.
- [ ] Testar reversão e erros de preparação.

**Aceite:** pacote versionado e documentado, com validação física do binário correspondente; o instalador é uma conveniência, não o centro do projeto.

## Sprint 9 — Novos módulos e câmeras (backlog futuro)

- [ ] Criar modelo de contribuição e SDK estável para terceiros.
- [ ] Experimentar intervalômetro e outras ferramentas fotográficas.
- [ ] Avaliar porte para outro corpo Canon somente com documentação e hardware disponíveis.
- [ ] Avaliar usabilidade e acessibilidade da interface na câmera.

**Aceite:** novos recursos com testes próprios e sem degradar o suporte da 5D.

## Política de trabalho

1. Uma sprint tem um objetivo verificável, escopo fechado durante a execução e uma demonstração ao final.
2. Mudanças não concluídas voltam ao backlog — não são marcadas como prontas por existirem arquivos de código.
3. Começar uma sprint dependente de hardware requer os resultados das anteriores e um plano de recuperação.
4. Cada PR deve indicar sprint, testes executados e limitações.
5. Mudanças de ABI e inicialização exigem revisão de compatibilidade e atualização dos documentos.
6. CI verde demonstra apenas que os testes definidos passaram no ambiente usado; não prova estabilidade em câmera.

## Próxima ação

**Sem leitor CF:** o pipeline [CR2→preview→Soup→JPEG no Linux](CR2-DESENVOLVIMENTO-HOST.md) já tem saída protegida. Próximo marco: testar **CR2 real autorizado da EOS 5D Classic**, estudar streaming com memória reduzida e revisar halation. Boot físico continua condicionado à atualização para firmware 1.1.1, leitor CF, fonte de boot verificada e procedimento de recuperação.

## Marco técnico adicional (13/11/2026)

Localizamos também [o instalador histórico GPLv2 da 5D Classic 1.1.1](5DC-BOOT-CHAIN.md) e distinguimos seus endereços de link dos endereços do Magic Lantern realocado. O Rebobina agora possui um *preflight* puramente funcional, auditado em CI, vinculado ao ELF offline, e testes com simulação da continuação Canon mesmo após falhas. **Não temos ainda boot/flash/write real, nem handoff de firmware ou plano de recuperação testado em hardware.**
