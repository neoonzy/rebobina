# Avanço sem CF — HAL e primeiro JPEG sintético do Soup

**Data:** 06/11/2026  
**Escopo:** apenas computador; nenhum acesso à Canon real ou ao CompactFlash.  
**Referência upstream Soup:** `jiijii0/soup-engine@6422c86b4530a5bdb4c5619f5287044ae7c661e3` (GPLv2).

## Entregas

1. **HAL em C** com callbacks de arquivos, memória e tarefas, sem endereços Canon, em [HAL-SIMULADA.md](HAL-SIMULADA.md).
2. **Backend RAM limitado**, com handles versionados, 4 arquivos de 4 KiB, 8 blocos de memória, 8 tarefas FIFO e injeção de falhas. Não escreve no CF ou em arquivos reais.
3. **Testes negativos** cobrindo criação sem sobrescrita, leitura/escrita em modo errado, tamanho, falta de mídia, alocação, leitura curta, gravação interrompida com bytes parciais, power cycle e descarte de tarefas.
4. **Selftests do Soup original:** compilação do `filmdev` numa cópia GPLv2 fixada, com adaptação **somente no host/main.c temporário** para a API de sistema de arquivos Linux; 13 verificações de demosaico, JPEG, LUT, perfil e parsing aprovadas.
5. **Integração inicial:** funções `se_arena` e `se_rd` reais operam sobre bytes fictícios na HAL.
6. **Primeiro teste fotográfico conectado:** o codificador JPEG real do Soup gera um **JPEG 8×8 sintético** gravado na HAL, analisado pelo parser JPEG do próprio Soup. Um caso com erro parcial de escrita confirma que o código não altera o arquivo de origem fictício.

## Verificação

- [CI principal: HAL, ARM e UBSan](https://github.com/neoonzy/rebobina/actions/runs/42227335334) — passou.
- [Soup Engine upstream e HAL](https://github.com/neoonzy/rebobina/actions/runs/42228272935) — 13 selftests sem falhas; smoke `se.c` com HAL passou.
- [Soup Engine + JPEG sintético via HAL](https://github.com/neoonzy/rebobina/actions/runs/42229454141) — JPEG sintético e falha parcial passaram.

### Reproduzir no Linux

```sh
make hal-demo
make test
make arm-objects
python3 tools/soup_host.py --fetch
```

O último comando exige rede e baixa exclusivamente uma revisão fixada do código-fonte upstream para a pasta temporária `build/`. Não baixa firmware Canon e não escreve no cartão. O código Soup permanece em uma cópia externa ao repositório; consulte [SOUP-ADAPTACAO.md](SOUP-ADAPTACAO.md).

## Não concluído

- Adaptador para processar arquivos CR2 reais, gerar JPEG de filmes usando stock `.fs`, comparar bytes com o `filmdev` e gerenciar escrita transacional robusta;
- API completa de arquivo (flush, rename e remoção segura de saídas temporárias);
- HAL real VxWorks, overlay LCD, botões e tarefas da Canon;
- Primeiro boot físico do Rebobina e validação da câmera firmware **1.1.1** (a câmera do usuário estava em 1.1.0 e ainda sem leitor CF);
- Carregamento dinâmico de módulos ARM em hardware.

**Critério de segurança:** simular um erro de escrita e verificar um JPEG sintético em RAM **não garante** integridade de fotografias reais nem segurança na câmera. A Sprint 3 física continua bloqueada.
