# Sprint 2 — Base de compilação ARM e simulação host

**Status:** parcialmente implementada. Nenhum binário instalável foi criado. CI normal e UBSan aprovadas na [execução 42129417751](https://github.com/neoonzy/rebobina/actions/runs/42129417751).

## Entregue

- Compilação **somente de objetos ARM relocáveis** para a CPU ARM946E-S, sem link, `AUTOEXEC.BIN`, arquivo `.FIR` ou modificação de cartão.
- Modelo de navegação de menu escrito em C portável, desacoplado da tela via callbacks `rb_display`.
- Uma HAL **em memória** de arquivos, memória e fila de tarefas com falhas injetáveis ([documentação](HAL-SIMULADA.md)).
- Um simulador de **interface** em terminal (`make sim-ui`) com botões `t` (lixeira), `u` (cima), `d` (baixo), `s` (SET) e `b` (voltar).
- Testes de limites, navegação, menu fechado e seleção, executados em host.
- Regras de validação do header ELF para rejeitar artefatos errados.

## Limites importantes

A simulação **não emula a Canon**, nem VxWorks, LCD real, bootloader, USB, RAW, sistema de arquivos ou tarefas. O fluxo atual é uma **prova de arquitetura de interface**, não a validação da Sprint 3 nem da Sprint 4 no hardware.

```sh
make sim-ui
make test
make arm-objects
```

## Pendências para concluir a Sprint 2

- Identificar o binário e código de origem do Magic Lantern **5DC.111** da Sprint 1;
- Determinar, com evidências, linker script, entry point, layout de memória e convenções de chamada;
- Validar correspondência com os serviços Canon reais: **o mock de tarefas/arquivos foi criado e testado, mas não comprova a HAL da câmera**;
- Simular a execução de integração com a plataforma e produzir build reproduzível e verificável.

O Clang permite compilar C para ARM mesmo sem uma `arm-none-eabi-gcc` instalada. Isto **não comprova compatibilidade binária** com a Canon EOS 5D.
