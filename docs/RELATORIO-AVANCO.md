# Relatório de avanço — Sprints 2, 5 e 7

**Data:** 05/11/2026  
**Repositório:** `neoonzy/rebobina`  
**Commit testado:** `92eadd481ab71362ae6b2167edf4781acafcd2e7`  
**Workflow:** [GitHub Actions 42129417751](https://github.com/neoonzy/rebobina/actions/runs/42129417751)  
**Resultado:** `build-and-test` = **success**, `undefined-behavior` = **success**.

## Entregas

| Sprint | Entregue | O que ainda não foi entregue |
| --- | --- | --- |
| 0 — Fundação | Política de versões, changelog, templates e código host com CI | Nenhum firmware de câmera faz parte da Sprint 0 |
| 1 — Pesquisa | Mapa de fontes, funções da ROM e hooks relatados; ADR e auditoria offline | Falta identificar o binário-base 2012 e validar em hardware |
| 2 — Toolchain | Objetos ARM ELF32 relocáveis para ARM946E-S; modelo de interface e simulação terminal | Sem linker de câmera, carregamento real, emulação VxWorks ou boot |
| 3 — Primeiro boot | Checklist e condições de teste | **Nenhum executável embarcado inicializado na câmera** |
| 4 — Interface | Modelo genérico de navegação C testado no host | Sem LCD, botões ou renderização reais da Canon |
| 5 — Módulos | `dlopen/dlsym` no Linux host e testes de rejeição de ABI/export | Sem loader ARM, ABI estável em câmera ou `.mo` |
| 6 — Soup Engine | Mapa de integração e dependências por arquivo | Nenhum processamento Soup dentro do Rebobina |
| 7 — Qualidade | Testes de estresse, negativos de plugins e UBSan no host | Sem testes de energia, CF, captura e recuperação no hardware |
| 8 — Distribuição | Política documentada | Sem instalador e sem pacotes instaláveis |
| 9 — Expansão | Backlog | Nenhum novo corpo Canon suportado |

## Evidências da CI

A execução em `ubuntu-latest` confirmou:

- Compilação do protótipo C e execução dos testes de registro e interface;
- **100.000 eventos simulados** de interface sem violar os invariantes testados;
- Plugin `.so` host carregado, iniciado e encerrado;
- Plugin com versão ABI errada **rejeitado** e biblioteca sem símbolo exportado **rejeitada**;
- **13 testes Python aprovados**;
- Dois manifestos JSON validados;
- Inventário estático consistente: 3 fontes, 6 hooks, 9 funções de ROM, 3 regiões de memória;
- `base_sha_known: false`: o hash de referência ainda não foi identificado;
- Compilação de objetos ELF32 relocáveis para ARM946E-S;
- Execução do job com **UndefinedBehaviorSanitizer**, sem erro reportado nos testes executados.

## Como reproduzir no Linux

```sh
make demo
make sim-ui
make plugin-demo
make test
make check-modules
make audit-platform
make arm-objects
```

Para usar UBSan em host:

```sh
make clean
make test CC=clang CFLAGS='-std=c11 -O1 -g -Wall -Wextra -Werror -pedantic -Iinclude -fsanitize=undefined -fno-sanitize-recover=all'
```

Os executáveis host e os arquivos `build/arm/*.o` **não devem ser copiados para cartões de memória**. Não há `AUTOEXEC.BIN` ou release embarcado do Rebobina.

## Restrições de segurança e próximos passos

A Sprint 1 continua aberta por ausência de correspondência verificada entre código-fonte 5DC.111 e binário-base histórico do Magic Lantern. Isso impede declarar os endereços como homologados para um boot próprio. Na Sprint 2 restam o linker comprovado, a HAL de hardware e a simulação específica da plataforma. **O primeiro boot real é a meta da Sprint 3 e requer uma câmera de desenvolvimento, planejamento de recuperação e testes físicos.**

Sprints 4–8 têm somente preparação parcial em host/documentação, não homologação em câmera. Não publicar ou distribuir firmware experimental como seguro enquanto esses bloqueios permanecerem.
