# Sprint 6 — Plano de adaptação do Soup Engine

**Estado:** análise/planejamento. O motor do Soup Engine ainda não foi incorporado, compilado nem executado no Rebobina.

Fonte de referência: [Soup Engine 5D Classic](https://github.com/jiijii0/soup-engine/tree/6422c86b4530a5bdb4c5619f5287044ae7c661e3/Canon-EOS-5D/src) (commit fixo).

## Estratégia: separar motor e integração

| Parte do Soup Engine | Origem | Destino planejado no Rebobina | Bloqueio |
| --- | --- | --- | --- |
| Processamento de cor, filmes, JPEG/CR2 | `engine/` | Biblioteca interna/módulo opcional | Testes de equivalência e orçamento de RAM |
| Seleção de filmes `.fs` | `stocks/`, parser do `engine/` | API de perfis do módulo | Compatibilidade da sintaxe e dos recursos |
| Monitor de `.CR2` e geração de `.FLM.JPG` | `ml/payload/film.c` | Serviço de arquivos/tarefas da HAL | Encerramento de escrita, concorrência e I/O |
| Botões, menu e overlay | `film.c`, `cam.h` | Interface Rebobina com callbacks | Sem driver LCD validado |
| Chamada direta a funções Canon | `cam.h` | HAL específica da 5DC.111 | Exige confirmar ROM e convenções de chamada |
| Stub, hooks e boot de 2012 | `mlstub.S`, `start.S`, `patch-film.py` | **Não transplantar diretamente** | Acoplamento ao binário Magic Lantern 2012 |
| Simulador e testes desktop | `tools/camsim`, `host/` | Reutilização/adaptação de testes | Dependências de amostras externas e licença |

## Contrato mínimo que o port deverá consumir

- Abrir, ler, escrever e fechar arquivos com resultados explícitos e limites.
- Agendar trabalho fora do callback de botões.
- Mostrar UI sem assumir endereços da interface Magic Lantern.
- Gerenciar alocação/liberação e condições de pouca memória.
- Interromper e recuperar processamentos incompletos.
- Descrever versão do módulo, dependências e suporte a modos JPEG/RAW.

## O que provar primeiro

1. **No host:** produzir um JPEG com a mesma saída do motor Soup de referência para uma entrada controlada, preservando a licença e autoria.
2. **Na simulação:** exercitar erros de leitura, gravação e falta de memória, com amostras explicitamente fornecidas.
3. **Na câmera de desenvolvimento:** integrar o menu, provar preservação de `.CR2` e inspecionar a saída gerada.
4. **Na distribuição:** compatibilidade por firmware, testes e créditos verificáveis.

O `module.json` atual é apenas **planejamento**. A prova do loader `.so` Linux na Sprint 5 não carrega `SE.BIN` e não executa os efeitos do Soup Engine.
