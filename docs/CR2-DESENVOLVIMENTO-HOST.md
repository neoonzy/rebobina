# Desenvolver o preview de CR2 com Soup Engine — CLI Linux

**Data:** 08/11/2026  
**Estado:** protótipo para Linux, com testes automatizados e saída em diretório explicitamente indicado. **Não é firmware, não acessa a Canon física e não lê os dados RAW do sensor.**

## O que faz

A ferramenta `rebobina-cr2-develop` lê o arquivo de entrada CR2 em memória no **computador** (limite 64 MiB), localiza e valida o **JPEG de pré-visualização embutido** com `rb_cr2_host_find_preview`, decodifica com o Soup Engine, aplica um perfil `.fs` e produz um **novo JPEG**, preservando o CR2 de entrada.

Os arquivos de filmes são os originais da revisão fixada do Soup Engine, em `build/soup-upstream/Canon-EOS-5D/src/stocks/` após download do source. O processamento **não decodifica o RAW Bayer**, portanto a qualidade/tonalidade vem do preview JPEG já processado pela Canon.

## Preparação

Requer **Linux** (não macOS), Git, Make, um compilador C, Clang e Python 3 com conexão para obter o código GPLv2 do Soup Engine:

```sh
python3 tools/soup_host.py --fetch
```

O comando deixa `build/rebobina-cr2-develop` e `build/rebobina-cr2-inspect` prontos para o ambiente host, além de executar testes automatizados.

## Utilização

Crie **um diretório de saída dedicado** antes da operação:

```sh
mkdir -p ./rebobina-saidas
./build/rebobina-cr2-develop \
  /caminho/para/foto.CR2 \
  ./build/soup-upstream/Canon-EOS-5D/src/stocks/portra.fs \
  ./rebobina-saidas \
  FOTO-PORTRA.JPG \
  2
```

Argumentos: `<entrada.CR2> <perfil.fs> <diretorio-existente> <nome.JPG> [escala:1|2]`.

- Escala `1`: tamanho original do preview JPEG embutido; escala `2`: metade da largura e da altura (padrão).
- Nome de saída: basename ASCII sem barras, com até 43 caracteres úteis, permitindo extensão `.TMP` de transação. Nomes que contenham diretórios ou iniciem com ponto são recusados pelo backend.
- A ferramenta **não substitui um arquivo existente**. Se `FOTO-PORTRA.JPG` existir, a publicação falha e o arquivo anterior fica intocado.
- A ferramenta cria temporário `FOTO-PORTRA.JPG.TMP` com exclusividade, escreve dados e solicita `fsync` do arquivo, fecha, chama `renameat2(RENAME_NOREPLACE)` e sincroniza o diretório. O fluxo de saída utiliza o **mesmo `rb_output` do Rebobina**, por meio do backend Linux `src/host/hal_posix.c`.

O diretório de saída é aberto uma única vez com `O_DIRECTORY | O_NOFOLLOW`. Os basenames são verificados e as operações usam `openat`/`unlinkat` e `renameat2` relativos ao descritor de diretório, evitando traversal e substituição de symlink de destino. É uma medida de segurança para o programa Linux, **não uma propriedade comprovada do firmware Canon/VxWorks/FAT**.

**Atenção:** já existir um arquivo temporário de execução anterior bloqueia uma nova operação. O utilitário não apaga arquivos de terceiros automaticamente. Inspecione os temporários antes de remover qualquer um.

## Evidência dos testes

O workflow `tools/soup_host.py` prepara um envelope TIFF/CR2 **inteiramente sintético**, sem dados RAW da Canon, que contém um JPEG gerado com o encoder Soup. Em seguida:

1. Valida os metadados e o intervalo de bytes do preview;
2. Executa `out/previewcheck` (utilitário de referência original do Soup Engine) com `soup800.fs`;
3. Executa `rebobina-cr2-develop` na **mesma entrada** e no mesmo perfil/escala, com saída exclusiva em um diretório temporário;
4. Compara **todos os bytes** do JPEG resultante entre as duas implementações;
5. Confirma que o SHA-256 do CR2 sintético de origem não mudou;
6. Tenta criar a mesma saída novamente, uma entrada truncada, um caminho com `../` e um diretório que é link simbólico — todos devem falhar sem substituir imagens.

O teste do backend POSIX (`tests/test_hal_posix.c`) verifica adicionalmente operações de leitura/escrita, handles fechados, colisão, renomeação exclusiva, recuperação explícita de temporário e recusa de symlinks. Os testes só escrevem em **diretórios temporários da CI**, separados das suas fotos.

## Limitações e riscos

- **Nenhum CR2 real da EOS 5D Classic foi fornecido/validado nesta etapa.** Um CR2 com preview JPEG de formato diferente, metadados não suportados ou imagens maiores pode exigir adaptação.
- Só o computador Linux usa `src/host/hal_posix.c`. O comportamento FAT/VxWorks da câmera não foi validado.
- Em falhas entre `renameat2` e o `fsync` do diretório, o nome final **pode já ter sido criado**, mesmo se o comando retornar erro; portanto a recuperação nunca remove automaticamente o nome final.
- O fluxo usa até 64 MiB de entrada e uma arena de 32 MiB, **incompatíveis com o orçamento de RAM da EOS 5D Classic**. O caminho embarcado exigirá leitura em streaming e gerenciamento dos buffers.
- A revisão original do Soup Engine tem um UB confirmado no `jpeg_enc.c` (left shift de número negativo) e aviso GCC na halation (`look.c`). A correção de shift usada nos testes ASan/UBSan permanece **somente na cópia temporária de testes**, não nesta ferramenta de referência nem no firmware da câmera. Veja [SOUP-SANITIZERS.md](SOUP-SANITIZERS.md).
- A operação preserva o CR2 original na lógica do software; isso **não é garantia contra falhas físicas de disco**, permissões externas ou software malicioso com acesso ao mesmo diretório.

## Próximas etapas

- [x] Adaptar processamento do preview real do Soup para uma CLI Rebobina com saída exclusiva.
- [x] Testar sem sobrescrita e validar equivalência com o código de referência em um envelope sintético.
- [ ] Processar amostras **reais e autorizadas** da EOS 5D Classic, verificando qualidade, integridade e metadados EXIF.
- [ ] Substituir a leitura em memória por pipeline em streaming compatível com os limites de RAM da câmera.
- [ ] Estabelecer backend real da 5D Classic 1.1.1 com APIs VxWorks verificadas fisicamente e política de recuperação no CF.

Veja também [inspeção CR2](CR2-INSPECAO-HOST.md), [saída segura](SAIDA-SEGURA.md), [adaptação Soup](SOUP-ADAPTACAO.md) e [ADR 0006](decisions/0006-hal-posix-segura.md).
