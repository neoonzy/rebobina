# Saída de imagem em duas fases — protótipo host

**Status:** testes C e Soup Engine no computador. **Sem gravação em CF físico, sem garantia de atomicidade ou durabilidade na Canon.** O arquivo original nunca é aberto para escrita pelo fluxo descrito aqui.

## Por que isso existe

Fotografias RAW da Canon não podem ser usadas como área de trabalho de um filtro. Uma falha de gravação, um cartão cheio ou o desligamento entre gerar e salvar um JPEG pode deixar uma saída incompleta. O Rebobina precisa distinguir **dados produzidos**, **dados sincronizados** e **saída publicada**, sem substituir fotos existentes.

## Fluxo implementado

A API em `include/rebobina/output.h` e `src/core/output.c` segue esta sequência:

1. `rb_output_init(&out)`: inicializa o contexto local, uma vez, antes da primeira tentativa.
2. `rb_output_begin(&out, hal, "FILME.JPG")`: cria **`FILME.JPG.TMP`**, exclusivo, em vez de tocar na saída final. Um temporário que já exista impede novo `begin` até recuperação explícita.
3. `rb_output_append(&out, data, n)`: escreve em loop enquanto a HAL retorna sucesso com **contagem positiva de bytes**. Uma escrita parcial sem erro é completada. Falha, contagem zero ou contagem inválida marcam a transação como não publicável.
4. `rb_output_commit(&out)`: solicita `sync`, fecha o temporário e faz `rename_noreplace` — **não substitui** um destino existente. Se qualquer etapa falhar, o nome final **pode não ser publicado**; no backend POSIX, a falha de `fsync` **após** o rename pode produzir erro mesmo que o nome final já exista. Nunca apagar o destino automaticamente nesse caso.
5. `rb_output_abort(&out)`: tenta fechar e remover **somente o temporário**, preservando o destino e o RAW. Se a mídia estiver indisponível, pode falhar e deixar um temporário órfão.
6. Depois de reinicialização simulada, `rb_output_discard_orphan(hal, "FILME.JPG")` permite remover **explicitamente** o temporário, e `rb_output_init` prepara um **novo** contexto da aplicação.

Ações de recuperação não são automáticas no boot. Em um produto real, a pessoa deve poder inspecionar/recuperar dados; nada deve apagar fotos existentes silenciosamente.

## Backend Linux POSIX: saída real de teste

Além do mock RAM, `src/host/hal_posix.c` implementa um backend **apenas para Linux** com diretório de saída explícito, `O_EXCL`, `O_NOFOLLOW`, `fsync` de arquivo e diretório, e `renameat2(RENAME_NOREPLACE)`. O programa `rebobina-cr2-develop` usa essa HAL para criar JPEGs em arquivos reais, sem substituição de destino. A implementação recusa symlinks e traversal. [Detalhes e limitações](CR2-DESENVOLVIMENTO-HOST.md), [ADR 0006](decisions/0006-hal-posix-segura.md).

**Essa implementação não representa o filesystem da câmera Canon** e não substitui a validação futura em FAT/VxWorks.

## Semântica do mock em RAM

- `sync` tem retorno de sucesso/falha injetável, mas **não executa fsync físico** nem garante a persistência de dados em caso de perda de energia;
- `rename_noreplace` recusa sobrescrever um destino. A simulação aplica a mudança de nome de forma indivisível em RAM, **não prova que FAT/VxWorks oferecem a mesma operação**;
- `remove` rejeita remover um arquivo ainda aberto e só remove o nome solicitado;
- As operações recusam mídia ausente, nomes fora do contrato e estados inválidos;
- O mock permite falhas em `sync`, `rename` e `remove`, além de escrita curta e gravação parcial seguida de erro;
- `rb_sim_hal_power_cycle` invalida handles e tarefas, mas preserva os bytes que o mock já escreveu para testar órfãos.

O simulador **não tem** log de filesystem, cache de escrita, corrupção de FAT, desconexão elétrica, concorrência VxWorks ou políticas de recuperação de firmware.

## Testes executáveis

```sh
make test
make arm-objects
python3 tools/soup_host.py --fetch
```

Em `tests/test_output.c` são cobertos: arquivo original preservado, publicação exclusiva, sucesso em escrita curta, falha de criação/IO, falha de sync, falha de rename, falha de remoção, falta de mídia, reinicialização e remoção explícita de temporários órfãos.

Em `tests/soup_jpeg_hal.c`, um JPEG **sintético** gerado pelo codificador real do Soup percorre o mesmo fluxo; falhas de gravação e renomeação não publicam arquivos finais truncados.

Em `tests/soup_stock_output.c`, o perfil upstream **`soup800.fs`** é lido e aplicado pelo código original do Soup a **8×8 pixels RGB sintéticos**, com JPEG produzido via HAL em duas fases. No primeiro teste verificado, o perfil alterou **183 componentes de cor** e gerou um JPEG com **1.006 bytes**. Isso não demonstra a mesma aparência ou desempenho numa fotografia real, muito menos numa Canon.

A compilação sob GCC 15 ainda imprime um aviso upstream `-Wstringop-overflow` na rotina de borda da halation em `engine/look.c`. Não foi alterado o motor original e o aviso não deve ser confundido com validação de segurança de memória.

## Dependências para gravar no CF real

- Implementação de `sync`, `rename_noreplace` e `remove` **com semântica documentada e observada na EOS 5D Classic 1.1.1**;
- Verificar se FAT/Canon suporta rename sem sobrescrita; se não, redesenhar o protocolo e registrar ADR;
- Confirmar capacidade de recuperar temporários após perda de energia e após cartão cheio;
- Escolher política de nomes e tratamento de colisões sem sobrescrever originais;
- Avaliar pilhas, consumo de RAM, latência e concorrência com captura de fotos;
- Testar com hardware descartável e protocolo de recuperação; nenhum teste host comprova segurança do CF real.

O fluxo de **preview JPEG de CR2 → Soup Engine → JPEG** existe agora no **host Linux**, com arquivo final real e publicação exclusiva. A validação automatizada usa um envelope CR2 sintético; não existe decodificação RAW Bayer nem suporte físico à Canon. A execução na câmera permanece pendente.
