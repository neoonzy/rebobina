# Sprint 3 — Preparação para o primeiro boot Rebobina

**Estado:** bloqueada para código experimental. A câmera real já está em firmware Canon **1.1.1**, com teste básico de JPEG/RAW e reinicialização **confirmado pelo usuário em 11/11/2026**. Há um **ELF ARM ligado apenas para inspeção offline** (`make offline-bootstrap`), mas **não** há carregador Canon, linker validado em hardware nem `AUTOEXEC.BIN` Rebobina. Leia [preflight Windows/CF](5DC-WINDOWS-PREFLIGHT.md).

Este documento serve para evitar que a Sprint 3 seja declarada concluída apenas porque conseguimos compilar objetos ARM na Sprint 2.

## Marco físico de preparação (sem Rebobina)

- [x] Atualização oficial da EOS 5D Classic para 1.1.1 — informada pelo usuário.
- [x] Fotos JPEG e RAW, leitura no Windows e reinicialização normal — informados pelo usuário.
- [x] Backup das fotos relatado pelo usuário; **dois CF de 16 GB disponíveis**, sendo um reservado para testes. O formato e a identidade de cada cartão ainda precisam ser registrados.
- [x] Hash SHA-256 do ZIP Soup Engine **informado pelo operador**, coincidente com o digest do release upstream consultado; falta análise offline dos binários incluídos (não equivale a validar o Magic Lantern original).
- [ ] Base Magic Lantern original para 5DC.111, incluindo hash, procedimento de boot flag e recuperação — ainda não verificada.
- [ ] Build ARM do Boot Probe reproduzível (diferença histórica 500/504 bytes pendente).

**Não executar a PR experimental #1 nem instalar Magic Lantern somente por causa da atualização do firmware.**

## Marco da pesquisa de bootstrap (13/11/2026)

- [x] Localizado port histórico Magic Lantern **5DC.111** com fonte específica VxWorks, commit fixado e créditos GPLv2.
- [x] Criada entrada ARM e linked ELF **próprios do Rebobina**, sem depender da instalação ou de binários do Magic Lantern, com auditoria offline. Consulte [bootstrap offline](5DC-BOOTSTRAP-OFFLINE.md).
- [ ] Demonstrar que a entrada e a RAM assumidas são apropriadas **ao loader Canon real**; o ELF offline **não é firmware instalável**.
- [ ] Implementar boot, handoff para Canon e recuperação sem deixar o firmware congelado; não existem ainda.

## Condições mínimas antes de escrever código de boot

1. Identificar de maneira rastreável o binário-base e o código-fonte correspondente do Magic Lantern **5DC.111** (revisão, procedência e SHA-256).
2. Verificar com fontes e inspeção os locais de execução, as áreas de RAM, as tarefas e as convenções de chamada realmente aplicáveis à 5D Classic.
3. Definir um plano de recuperação e uma câmera **de desenvolvimento**, não de uso cotidiano, com risco explicitamente aceito.
4. Garantir que todo artefato de teste seja identificado como **experimental**, com versão, hash e incompatibilidades documentados.
5. Fazer inspeção estática e testes de simulação antes de usar hardware, além de definir quais diagnósticos podem ser observados se o display não responder.

## Critérios de aceite para primeiro boot

- [ ] Executável **Rebobina**, não um loader genérico ou build do Magic Lantern inalterado;
- [ ] Inicialização observada em Canon 5D Classic 1.1.1 real;
- [ ] Evidências de que a inicialização não sobrescreveu áreas inesperadas;
- [ ] Logs e relato das condições de teste;
- [ ] Comportamento de desligamento/reinicialização documentado;
- [ ] Procedimento de recuperação testado.

**Nenhuma dessas condições foi verificada ainda.** Compilação de `.o` ARM, menu host e `dlopen` Linux não contam como boot de câmera.
