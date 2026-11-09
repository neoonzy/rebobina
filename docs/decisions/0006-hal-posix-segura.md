# ADR 0006 — Backend HAL POSIX no Linux, isolado em diretório

**Data:** 08/11/2026  
**Status:** aceita apenas para a CLI host; não faz parte de firmware embarcado.

## Problema

A HAL em RAM tinha limite de 4 KiB por arquivo e não servia para gerar JPEGs de fotografias reais no computador. Precisamos exercitar o contrato `rb_output` com arquivos reais de teste no Linux, **sem sobrescrever os arquivos do usuário** e sem confundir o comportamento do host com FAT/VxWorks.

## Decisão

- A implementação `rb_posix_hal` recebe um **diretório de saída existente**, aberto com `O_NOFOLLOW | O_DIRECTORY` e mantido por descritor.
- Aceita somente **basenames ASCII** limitados; recusa travessias de diretório, barras e nomes ocultos.
- Criar temporário: `openat(..., O_CREAT | O_EXCL | O_NOFOLLOW, 0600)`, nunca abre/trunca arquivo existente.
- Publicar: `fsync` do temporário, fechar, `renameat2(RENAME_NOREPLACE)` e `fsync` do diretório. Não existe fallback que possa sobrescrever destino se a chamada não for suportada.
- Recuperação de temporários é **explícita**, via `rb_output_discard_orphan`. A CLI nunca remove o resultado final automaticamente.
- Limitado a **Linux**: não usar o backend POSIX no build ARM nem atribuir a esses serviços semântica de chamadas Canon.

## Consequências e casos difíceis

O processo pode deixar um arquivo `.TMP` depois de falhas, e uma falha de `fsync` do diretório depois de renomear pode ser **ambígua** (destino já existe mesmo com erro). Isso exige inspeção do diretório e não justifica excluir arquivos finais.

`renameat2` com `RENAME_NOREPLACE` depende de suporte do kernel/filesystem; se não houver, a operação falha de forma segura em vez de substituir um arquivo. O teste host não prova persistência após perda de energia física ou suportabilidade em FAT da câmera.

Esse backend é parte do **ambiente de testes** do Rebobina, não da sua portabilidade para ARM. Considere [CR2-DESENVOLVIMENTO-HOST.md](../CR2-DESENVOLVIMENTO-HOST.md) e [SAIDA-SEGURA.md](../SAIDA-SEGURA.md).
