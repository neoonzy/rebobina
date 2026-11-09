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
- Selftest do motor Soup Engine upstream fixado (13 verificações de imagem/parsing), integração de `se.c` + HAL em host e geração de JPEG sintético 8×8 através da HAL, com teste de falha parcial; **sem CR2 real nem firmware embarcado**;
- Publicação exclusiva de JPEG em duas fases (arquivo temporário, sync, close, rename), sem sobrescrever destino, com testes de falhas e recuperação explícita no mock RAM;
- Perfil original `soup800.fs` aplicado a RGB sintético 8×8 com look real Soup e JPEG de 1.006 bytes; sem CR2 nem câmera;
- CLI Linux Rebobina para CR2→preview JPEG→stock Soup→JPEG final, com backend POSIX confinado em diretório, fsync e rename exclusivo, validado por comparação byte a byte com o Soup original em entrada sintética;
- Testes de colisão, truncamento, symlink, traversal e preservação de entrada; nenhum CR2 real testado nem firmware distribuído;
- Validação defensiva de offset/dimensões do preview CR2, inspeção host somente leitura e teste end-to-end de envelope CR2 sintético com Soup 800;
- ASan/UBSan em três stocks, achado de shift negativo indefinido no JPEG upstream, correção apenas na cópia de teste e JPEGs comparados byte a byte;
- Plano de desenvolvimento sem CompactFlash, ADRs 0002–0006 e fronteiras do Soup Engine;
- Documentação de compatibilidade e versão.

**Não há arquivos `AUTOEXEC.BIN` ou `.FIR` do Rebobina.**
