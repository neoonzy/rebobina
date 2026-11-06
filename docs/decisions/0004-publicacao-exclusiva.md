# ADR 0004 — Publicação exclusiva de JPEG e recuperação explícita

**Data:** 06/11/2026  
**Estado:** aceita para protótipo host, **não** para Canon física.

## Decisão

Adotar geração de arquivo temporário seguido de `sync`, fechamento e `rename_noreplace`, com retorno de erro explicitamente representado na API. A publicação do nome final não pode sobrescrever um arquivo existente. O módulo deve marcar a saída como falha após qualquer escrita inconclusiva.

A HAL oferece `sync`, `rename_noreplace` e `remove` como **contrato**, deixando a implementação e suas garantias dependentes do backend. O mock aplica o fluxo em memória e injeta falhas em cada etapa. Recuperação de temporários órfãos é uma **ação explícita**, sem apagar arquivos do usuário automaticamente.

## Alternativas consideradas

- Gravar diretamente em `FILME.JPG`: rejeitado, pois nomes finais podem representar saídas incompletas.
- Apagar o arquivo existente e gravar outro: rejeitado, pois destrói o resultado anterior e pode perder dados.
- Supor que `rename` e `fsync` têm semântica POSIX na câmera Canon: rejeitado até verificar os serviços VxWorks/FAT da 5D.
- Apagar temporários automaticamente no boot: adiado, pois pode ocultar causas de falha e eliminar dados recuperáveis.

## Consequências

A operação pode deixar `.TMP` depois de falta de mídia, falha de limpeza ou desligamento. O nome final só aparece após commit bem-sucedido **no mock**. A garantia física permanece **desconhecida**. O código caller precisa inicializar `rb_output` antes do primeiro uso, abortar erros quando possível e nunca reutilizar uma transação interrompida sem recuperação.

Ver [SAIDA-SEGURA.md](../SAIDA-SEGURA.md) e [HAL-SIMULADA.md](../HAL-SIMULADA.md).
