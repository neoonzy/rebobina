# Preparação da adaptação do Soup Engine ao Rebobina

**Sprint 6 — motor original e perfil Soup 800 aplicados a pixels sintéticos no host, com saída JPEG em duas fases; CR2 e câmera permanecem pendentes.**

Fonte pinada para análise: [`jiijii0/soup-engine@6422c86b...`](https://github.com/jiijii0/soup-engine/tree/6422c86b4530a5bdb4c5619f5287044ae7c661e3/Canon-EOS-5D/src). A integração existente na 5D utiliza hooks da versão de 2012 do Magic Lantern e não pode ser transferida automaticamente para o Rebobina.

## Evidência executável: motor upstream + HAL

A rotina `tools/soup_host.py` obtém o commit GPLv2 `6422c86b4530a5bdb4c5619f5287044ae7c661e3`, confirma seu hash, compila `Canon-EOS-5D/src/filmdev` e executa o selftest do próprio upstream. [CI aprovada](https://github.com/neoonzy/rebobina/actions/runs/42228272935): **13 verificações passaram**, incluindo demosaico, LUT 3D, transformação sRGB, codificação/decodificação JPEG, curva de tons, telas do Soup, arquivo de stock e dados corrompidos/truncados.

**Integração fotográfica sintética:** [CI 42229454141](https://github.com/neoonzy/rebobina/actions/runs/42229454141) também compila `tests/soup_jpeg_hal.c` com os arquivos originais `jpeg_enc.c` e `jpeg_dec.c`. Um padrão RGB8 sintético 8×8 é codificado pelo Soup e gravado através da HAL Rebobina em RAM; o cabeçalho JPEG é lido e conferido. Ao injetar uma escrita parcial de 12 bytes, o encoder retorna erro e o arquivo de origem fictício permanece intocado. **Isso não é um filtro de filme nem processamento de CR2.**

**Perfil de filme no host:** agora `tests/soup_stock_output.c` lê o arquivo original `stocks/soup800.fs`, usa o parser, o look com halation e o codificador JPEG do Soup e grava um JPEG sintético pelo fluxo `rb_output` em duas fases do Rebobina. No teste Linux passaram 8×8 pixels, **183 componentes RGB modificados** e JPEG de **1.006 bytes**. [CI](https://github.com/neoonzy/rebobina/actions/runs/42237005969). É a primeira aplicação de um stock real no protótipo host, mas **não há abertura de arquivo CR2 nem fotografia do usuário**.

**Alerta de compilação:** GCC 15 emite `-Wstringop-overflow` no código upstream de borda de halation (`engine/look.c`). O teste deixa esse aviso visível e não modifica `engine/`; é uma pendência de revisão e não pode ser tomada como prova de segurança de memória.

O Soup possui uma pequena dependência macOS no utilitário `host/main.c` (campos de `statfs`). Nosso script aplica um **shim apenas na cópia temporária baixada em `build/soup-upstream/`**, sem alterar nenhum arquivo de `engine/`, nem o projeto original remoto.

No mesmo workflow, `tests/soup_hal_smoke.c` compila as implementações reais de `se_arena` e `se_rd` (de `engine/se.c`) junto ao mock HAL Rebobina, alimentados por **oito bytes sintéticos de fixture** e testados com leitura curta e limite de arena. Esse teste não é um processamento de imagem, não gera JPEG e não prova que o Soup foi integrado ao menu ou à câmera.

Para reproduzir no Linux com rede, Git, CC, Make e Python:

```sh
make test
python3 tools/soup_host.py --fetch
```

A cópia de código Soup é temporária e obedece à GPLv2; **nenhum binário nem código-fonte de terceiros é versionado no repositório Rebobina por esse fluxo**.

## Fronteiras propostas

| Subsistema | Origem upstream | Interface necessária no Rebobina | Teste host proposto |
| --- | --- | --- | --- |
| Processamento de imagem/cores | `engine/` | API independente de ROM, callbacks de alocação | Igualdade de pixels/JPEG para dados sintéticos |
| Presets de filme | `stocks/`, parser de `engine/` | Listagem, validação e seleção de presets | Perfis válidos/inválidos e ordem estável |
| Entrada CR2 e preview JPEG | `engine/cr2.c`, `jpeg_dec.c` | Fluxo de leitura com erro explícito | Entrada truncada, metadados inválidos e CRC/checksum quando aplicável |
| Saída JPEG adicional | `engine/jpeg_enc.c` | Escrita transacional com nome distinto | Mesmo CR2 não modificado e arquivo de saída esperado |
| Tarefa e fila de trabalhos | `ml/payload/film.c` | Agendador de trabalho fora da GUI | Fila, cancelamento e falhas parciais |
| Interface de seleção | `engine/ui.c`, código de tela no `film.c` | Eventos UI abstratos e apresentação Rebobina | Navegação e status de processamento no simulador |
| ROM/hardware e boot | `ml/payload/cam.h`, `start.S`, `mlstub.S` | HAL específica e boot verificados em 5DC.111 | **Somente hardware, após validação** |

## Princípios para o port

1. Não importar `AUTOEXEC.BIN` do Soup nem misturá-lo com `SE.BIN` como se fossem plugins.
2. Processamento de imagem não deve saber os endereços ROM Canon; usa `rb_hal_file_*`, alocação e callbacks futuros.
3. GUI só enfileira comandos; processamento e gravações ficam em tarefa separada.
4. CR2 deve ser preservado (somente leitura) e o JPEG adicional não pode sobrescrever originais.
5. Falhas de I/O, arquivos truncados e memória esgotada devem ser estados recuperáveis no host.
6. Antes de integrar código GPLv2, registrar proveniência de cada arquivo, direitos autorais e testes de equivalência.
7. O menu atual **apenas mostra o texto** “Soup Engine: não integrado”, sem qualquer simulação de efeito ou promessa de suporte.

## Ordem de implementação

- [x] Especificar o contrato visual e gerenciador host para módulo Hello.
- [x] Desenhar as fronteiras de port com referência aos arquivos upstream.
- [x] Criar o contrato experimental `rb_hal` de arquivos, memória e tarefas com falhas injetáveis e testes em RAM.
- [ ] Integrar o **motor completo** `engine/` ao fluxo de processamento Rebobina, preservando GPLv2 e atribuições.
- [x] Executar selftests do motor upstream e smoke de `se_arena`/`se_rd` com HAL Rebobina.
- [x] Codificar um JPEG sintético pelo Soup com saída na HAL e testar falha de gravação.
- [x] Aplicar stock real `soup800.fs` a pixels sintéticos e gerar JPEG pelo fluxo em duas fases ([CI](https://github.com/neoonzy/rebobina/actions/runs/42237005969)).
- [ ] Revisar aviso GCC na borda de halation; executar sanitizadores adicionais na integração do look.
- [ ] Comparar saída JPEG/RAW do futuro adaptador Rebobina byte a byte com o processamento de referência.
- [ ] Implementar módulo Soup no contrato `rb_module` ou numa ABI revista, após testes.
- [ ] Validar tela, eventos e arquivos na 5D física **1.1.1** após o primeiro boot.

**Os selftests reais do motor Soup foram executados, mas nenhuma fotografia do usuário foi processada, e o Soup ainda não integra o firmware Rebobina.**
