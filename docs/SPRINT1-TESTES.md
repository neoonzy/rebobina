# Sprint 1 — Relatório de testes

**Data:** 05/11/2026  
**Escopo:** host Linux (GitHub Actions), manifesto de evidências e simulação **sintética de estrutura de arquivos**, sem ROM Canon, sem instalar Magic Lantern, sem cartão CF e sem hardware físico.

## Execução verificável

- **Workflow:** [Validação host — execução 42127791844](https://github.com/neoonzy/rebobina/actions/runs/42127791844).
- **Resultado da execução:** **success** (job build-and-test).
- **Commit testado:** `af8f74ff09fd0ce802617f6737d609129a9d5941`.
- **Comandos executados:** `make demo`, `make test`, `make check-modules`, `make audit-platform`.
- **C:** compilação e demonstração host executadas; teste do registro e ciclo de vida passou.
- **Python:** **13 testes** passaram (4 testes de manifestos e 9 testes de auditoria de plataforma).
- **Manifestos:** 2 manifestos válidos (apenas metadados).
- **Auditoria:** 3 fontes, 6 hooks, 9 funções de ROM e 3 regiões coerentes **no arquivo JSON**, com `base_sha_known: false`.

## O que os testes da Sprint 1 cobrem

| Grupo | Teste | Resultado |
| --- | --- | --- |
| Inventário | Modelo, firmware, CPU/OS e referências fixadas por commit | Passou |
| Inventário | Rejeição de plataforma errada ou fonte sem commit fixo | Passou |
| Inventário | Rejeição de regiões de memória sobrepostas | Passou |
| Inventário | Rejeição de hooks duplicados e endereços ROM fora do padrão | Passou |
| Integridade documental | Binário sem SHA-256 **não** pode ser marcado como identificado | Passou |
| Binário sintético | Aceita somente tamanho, marcador e SHA-256 esperados | Passou |
| Binário sintético | Rejeita tamanho, marcador ou SHA incorretos | Passou |
| Imutabilidade | Inspeção local não altera o arquivo de teste | Passou |
| Núcleo host | Registro e encerramento de módulos | Passou |

O teste do `AUTOEXEC.BIN` usa uma **sequência de bytes construída pelo próprio teste**. Passar nessa verificação **não comprova** que o executável real foi encontrado ou que seria seguro na câmera.

## Reproduzir

A partir da raiz do repositório, com C11, Make e Python:

```sh
make demo
make test
make check-modules
make audit-platform
```

Se você **já possuir legalmente uma cópia do executável Magic Lantern** e conhecer seu SHA-256 esperado por fonte confiável, poderá realizar uma inspeção **somente leitura**:

```sh
python3 tools/platform_audit.py inspect-autoexec \
  --file /caminho/para/autoexec.bin \
  --expected-sha256 HASH_SHA256_CONFIAVEL_DE_64_CARACTERES
```

O script exige hash explícito e confere tamanho (221184 bytes), marcador `gaonisoy` na posição indicada pelo port e igualdade do hash. Isso **não instala nem modifica nada**, nem determina por si só a compatibilidade de firmware.

## Testes que **não** foram feitos

- Inicialização da câmera e comportamento do bootloader 5DC;
- Build ARM do Rebobina, linker final e validação do mapa real de RAM;
- Uso de binário original Magic Lantern 2012;
- Simulação completa de firmware Canon / Magic Lantern na 5D;
- Teste de LCD, botões, cartão CF, gravação CR2, DMA e desligamento em hardware.

**Conclusão:** base host e evidências estruturais testadas; Sprint 1 **parcialmente concluída**. A identidade do binário-base 2012 ainda impede declarar prontidão para iniciar bootstrap de câmera.
