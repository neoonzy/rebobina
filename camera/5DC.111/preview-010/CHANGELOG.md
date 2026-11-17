# Alterações do REBOBINA embarcado

## 0.1.0-dev — sessão integrada (não validada fisicamente)

- HAL separada para Canon EOS 5D Classic 1.1.1, com mocks testáveis.
- Relatórios de boot, memória, gravação/leitura do CF, task VxWorks, eventos e resumo final em uma sessão.
- Extensão do buffer de eventos de 12 para 48 entradas e identificação do código na segunda palavra `event->param`.
- Cinco páginas de interface (`Status`, `Memória`, `Input`, `Storage`, `About`) e navegação passiva SET/roda.
- Modo LCD opt-in via arquivo `ML/RB5UI.ON` no cartão, sem forçar display ou MMIO.
- Quatro arquivos no CF: `RB5BOOT.TXT`, `RB5TEST.TXT`, `RB5EVENT.TXT`, `RB5FINAL.TXT`.
- Snapshot de RAM total/livre via API Canon histórica, teste de alocação de 256 e 4096 bytes.
- Preparação no Windows somente no computador, rollback, parser de logs, testes C/Python, validador de ABI.

## Marcos físicos anteriores

- `0.0.1`: criação confirmada de `REBOBINA.TXT` na 5D Classic.
- `0.0.3`: memória, criação de tarefa, leitura e gravação no CF confirmadas.
- `0.0.4`: nove eventos GUI com parâmetros como MENU (`0x03`), INFO (`0x04`), SET (`0x02`), TRASH (`0x07`) confirmados no log.

Nem os marcos acima nem o build 0.1.0 representam firmware independente do Soup Engine.
