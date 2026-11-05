# Sprints — Rebobina

> **Produto:** software embarcado próprio para a Canon EOS 5D Classic (firmware 1.1.1), inspirado no Magic Lantern, com interface própria e módulos extensíveis. Não é um instalador de Magic Lantern.
>
> **Planejamento inicial, em 5 de novembro de 2026.** Sprints são ciclos de entrega orientados por objetivos e critérios verificáveis, não promessas de prazo. Sugestão de cadência: duas semanas para desenvolvimento host/pesquisa; as atividades em hardware ficam condicionadas a evidências e disponibilidade de uma câmera de teste.
>
> **Regra:** executar testes em host e simulação não significa validar na câmera. Não distribuir `AUTOEXEC.BIN`, `.FIR` ou payloads como seguros enquanto não houver testes físicos e procedimento de recuperação.

## Visão geral

| Sprint | Foco | Entrega principal | Estado |
| --- | --- | --- | --- |
| **0** | Fundação | Repositório, API protótipo, manifesto, CI e documentação | Parcialmente concluída |
| **1** | Engenharia reversa e especificação | Mapa de boot, APIs, memória e decisões de reaproveitamento GPLv2 da 5D | A fazer — próxima |
| **2** | Toolchain e simulação ARM | Build reproduzível ARM e ambiente mínimo de testes do port | A fazer |
| **3** | Primeiro executável embarcado | Rebobina mínimo inicializando de forma controlada na 5D de teste | A fazer; depende de hardware |
| **4** | HAL e interface própria | Interface inicial, botões, arquivos, logs e serviços comuns | A fazer |
| **5** | Módulos embarcados | Carregador versionado e primeiro módulo simples em câmera | A fazer |
| **6** | Integração Soup Engine | Soup Engine adaptado ao Rebobina, sem patch individual de `AUTOEXEC.BIN` | A fazer |
| **7** | Confiabilidade | Testes de interrupção, memória, compatibilidade e recuperação | A fazer |
| **8** | Distribuição | Releases por firmware e instalador auxiliar seguro | A fazer |
| **9** | Ecossistema | SDK público, módulos extras e estudo de outras Canon | Futuro |

## Sprint 0 — Fundação do projeto

**Objetivo:** criar uma base de desenvolvimento que não se confunda com firmware utilizável.

- [x] Criar README, arquitetura, segurança, contribuição e diretrizes para agentes.
- [x] Implementar registro de módulos em C para **host** e módulo de demonstração.
- [x] Definir manifesto JSON experimental, CLI de criação e validação.
- [x] Adicionar testes C/Python e CI; a execução inicial no GitHub Actions passou.
- [ ] Especificar versionamento do núcleo, ABI, módulos e releases.
- [ ] Criar `CHANGELOG.md` e templates de issue/PR.

**Concluída quando:** política de versões e rastreabilidade documentadas; CI host verde. **A situação atual é parcialmente concluída.**

## Sprint 1 — Entender a EOS 5D e o Magic Lantern

**Objetivo:** ter informações confiáveis para desenvolver o Rebobina embarcado sem partir de suposições perigosas.

- [ ] Fixar a compilação/fonte exata do Magic Lantern para **5DC.111**, registrando procedência e hashes quando disponíveis.
- [ ] Documentar o fluxo do bootloader Canon, boot flag e início da extensão no CompactFlash.
- [ ] Mapear VxWorks/DIGIC II: memória, tarefas, pilhas, botões, LCD, sistema de arquivos e captura.
- [ ] Identificar APIs e funções GPLv2 reaproveitáveis e seus limites de compatibilidade.
- [ ] Comparar os hooks e o carregamento atuais do Soup Engine com as necessidades do Rebobina.
- [ ] Criar matriz de riscos: o que pode ser testado em host, o que exige simulação e o que exige câmera.
- [ ] Redigir uma decisão arquitetural (ADR) sobre reutilizar, adaptar ou substituir cada componente.

**Entregáveis:** `docs/5DC-PLATAFORMA.md`, `docs/decisions/0001-boot-e-reuso.md` e lista de recursos com suas fontes.

**Aceite:** cada endereço/hook conhecido tem origem e versão explícitas; limitações e incertezas ficam documentadas; nenhuma alegação de segurança depende apenas de compilação.

## Sprint 2 — Toolchain e ambiente de testes ARM

**Objetivo:** compilar o projeto para a arquitetura da 5D sem ainda declarar suporte ao hardware.

- [ ] Criar alvos de build ARM946E-S e linker script base, derivados de estudos verificados da plataforma.
- [ ] Separar `host` de `camera/5DC.111` e explicitar o que é simulado.
- [ ] Implementar stubs ou simulador mínimo para chamadas da HAL.
- [ ] Validar dependências externas, tamanho de imagem, seções, ABI e limites de memória na compilação.
- [ ] Automatizar testes da HAL e do ciclo de vida sob host.
- [ ] Registrar ferramentas e versões para builds reproduzíveis.

**Entregável:** primeiro artefato ARM **experimental**, junto a relatórios de compilação e verificações automáticas.

**Aceite:** build determinístico e verificações automatizadas; nenhuma alegação de que o artefato é seguro para colocar no cartão.

## Sprint 3 — Primeiro Rebobina na câmera

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

**Iniciar Sprint 1:** levantamento da inicialização, memória e APIs do Magic Lantern na Canon EOS 5D Classic 1.1.1. O primeiro executável embarcado fica como meta da Sprint 3, após pesquisa e toolchain adequados.
