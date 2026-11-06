# HAL simulada do Rebobina

**Sprints 2, 4 e 7 — implementação e testes apenas no computador.** Esta camada NÃO emula VxWorks, o CF real ou o DIGIC II.

## Implementação

O contrato portável está em `include/rebobina/hal.h`, implementado por um backend em RAM em `src/host/hal_sim.c`. A interface usa contexto e callbacks em C para arquivos (`create`, `open_read`, `read`, `write`, `close`), memória (`alloc` e `release`) e tarefas (`enqueue`). Cada operação de arquivo devolve um status e uma contagem de bytes, permitindo distinguir sucesso, leitura curta e gravação parcial.

```sh
make hal-demo
make test
make arm-objects
```

`make hal-demo` grava sete bytes sintéticos **apenas em RAM**. Os testes C em `tests/test_hal_sim.c` validam limites, estados inválidos, escrita parcial, interrupção de mídia, memória e fila. A compilação ARM produz **objeto relocável**, não firmware de câmera.

## Restrições deliberadas

| Elemento | Limite | Semântica |
| --- | --- | --- |
| Arquivos | 4 | Apenas em memória; nomes sem diretórios |
| Tamanho por arquivo | 4 KiB | Sem FAT, ROM, DMA ou disco |
| Fila | 8 tarefas | FIFO cooperativa, sem threads |
| Memória | 8 blocos de até 1024 bytes | Não corresponde ao heap Canon |
| Handles | Token com geração | Fechados/invalidados após reinício |
| Criação | Não sobrescreve arquivo existente | `RB_HAL_EXISTS` |

**Uma escrita parcialmente concluída continua presente após `rb_sim_hal_power_cycle`.** A simulação exige que o código de processamento detecte esse estado, e não assuma rollback mágico.

## Injeção de falhas

`rb_sim_hal_inject_fault` aceita os modos `RB_SIM_FAULT_CREATE`, `OPEN`, `READ`, `WRITE`, `ALLOC`, `QUEUE`. Essas falhas pontuais são consumidas por sua operação. `rb_sim_hal_short_read_once` devolve menos bytes sem indicar erro; `rb_sim_hal_cut_write_once` grava parcialmente e retorna erro de I/O; `rb_sim_hal_set_media(sim, 0)` simula cartão indisponível.

A função `rb_sim_hal_run_one` executa uma tarefa pendente; `rb_sim_hal_power_cycle` descarta trabalhos e invalida handles. O mock não mede latência, concorrência, vazão, consumo de energia ou coerência do cache.

## Limitações para módulos fotográficos

- Ainda não há `flush`, `rename`, transação nem recuperação automática: antes de gravar JPEG em CF real, precisamos definir como preservar o CR2 e evitar arquivos incompletos com nomes finais.
- O código não chama endereços ROM da Canon. Um backend físico para 5DC.111 será outra implementação, testada separadamente.
- A ABI deste mock é experimental. Um arquivo que passa nos testes host não é garantia de segurança física.

## Relação com Soup Engine

O teste `tests/soup_hal_smoke.c` compila a implementação real `engine/se.c` do upstream pinado, usa `se_arena` e `se_rd` com dados sintéticos atravessando a HAL Rebobina e testa limites. **Não processa um CR2 nem gera um JPEG real.**

Além disso, `tests/soup_jpeg_hal.c` **gera um JPEG real de 8×8 pixels sintéticos** com `se_jpeg_begin`/`se_jpeg_write_rows`/`se_jpeg_end`, usa callbacks de escrita da HAL em RAM, verifica SOI/EOI e lê o cabeçalho com `se_jpeg_dec_info`. Um segundo cenário injeta uma escrita JPEG parcial que falha, preservando um arquivo de origem **fictício** denominado `ORIGINAL.CR2`.

O selftest independente do upstream executa casos de demosaico, LUT, JPEG, curva, tela e parsing, descritos em [SOUP-ADAPTACAO.md](SOUP-ADAPTACAO.md).

**Evidência:** [GitHub Actions da HAL](https://github.com/neoonzy/rebobina/actions/runs/42227335334), [selftest Soup + JPEG sintético via HAL](https://github.com/neoonzy/rebobina/actions/runs/42229454141). Consulte também [ADR 0003](decisions/0003-hal-deterministica.md).
