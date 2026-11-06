# Preparação da adaptação do Soup Engine ao Rebobina

**Sprint 6 — fase de decomposição técnica, sem importação de código nem execução do motor.**

Fonte pinada para análise: [`jiijii0/soup-engine@6422c86b...`](https://github.com/jiijii0/soup-engine/tree/6422c86b4530a5bdb4c5619f5287044ae7c661e3/Canon-EOS-5D/src). A integração existente na 5D utiliza hooks da versão de 2012 do Magic Lantern e não pode ser transferida automaticamente para o Rebobina.

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
- [ ] Definir interface `rb_hal_file`, `rb_hal_task` e `rb_hal_memory` com injeção de erros para testes.
- [ ] Integrar **somente** `engine/` numa biblioteca host, preservando GPLv2 e atribuições.
- [ ] Reproduzir testes de cor/JPEG em fixtures sintéticas e comparar com upstream.
- [ ] Implementar módulo Soup no contrato `rb_module` ou numa ABI revista, após testes.
- [ ] Validar tela, eventos e arquivos na 5D física **1.1.1** após o primeiro boot.

Nenhum processamento de imagem do Soup Engine foi executado nesta entrega.
