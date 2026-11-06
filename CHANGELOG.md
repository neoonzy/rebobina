# Changelog

O projeto ainda está em fase de prototipagem; nenhuma versão para instalação na câmera foi lançada.

## Unreleased

- Fundação do repositório Rebobina, GPLv2 e documentação técnica;
- Protótipo de registro de módulos em C e testes host;
- Sprint 1: mapeamento de VxWorks/5D, hooks relatados pelo Soup Engine e auditoria de evidências;
- Sprint 2: menu independente de hardware, simulação terminal, simulador visual web conectado ao core C e compilação de objetos ARM946E-S;
- Sprint 5 (prova host): carregamento experimental de módulos compartilhados no Linux e gerenciador de ativação/desativação individual, ainda não embarcados;
- Sprint 7 (antecipação host): 100.000 eventos de interface e UBSan, com testes aprovados na CI;
- HTTP local somente leitura com validação de eventos e testes C, Python e de interface;
- HAL em RAM com 4 arquivos, memória e tarefas limitadas, falhas de leitura/escrita, remoção de mídia e power cycle simulados;
- Selftest do motor Soup Engine upstream fixado (13 verificações de imagem/parsing), integração de `se.c` + HAL em host e geração de JPEG sintético 8×8 através da HAL, com teste de falha parcial; **sem CR2 real, processamento de filme ou firmware embarcado**;
- Plano de desenvolvimento sem CompactFlash, ADR 0002 e fronteiras do Soup Engine;
- Documentação de compatibilidade e versão.

**Não há arquivos `AUTOEXEC.BIN` ou `.FIR` do Rebobina.**
