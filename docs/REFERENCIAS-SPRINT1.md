# Fontes, evidências e limites da Sprint 1

| Referência | Revisão ou data verificada | Uso | Limitação |
| --- | --- | --- | --- |
| [Soup Engine para 5D](https://github.com/jiijii0/soup-engine/tree/6422c86b4530a5bdb4c5619f5287044ae7c661e3/Canon-EOS-5D) | \`6422c86b4530a5bdb4c5619f5287044ae7c661e3\` | Hook, ROM, heap, pilhas e simulação | Relatos do port, não medições independentes do Rebobina |
| [Magic Lantern simplificado](https://github.com/reticulatedpines/magiclantern_simplified/tree/142472e95054d73640f50789ac0fd66325499d82) | \`142472e95054d73640f50789ac0fd66325499d82\` | Regras gerais, GPLv2 e arquitetura moderna | **Não contém a pasta 5DC.111** no snapshot; guia é majoritariamente DryOS |
| [Discussão 5D Classic](https://groups.google.com/g/ml-devel/c/47_XE_cwrdQ) | 24/04/2012 | Relato histórico de boot flag e VxWorks 5.5 | Não substitui documentação ou teste da build atual |
| [Arquitetura geral ML](https://github.com/reticulatedpines/magiclantern_simplified/blob/142472e95054d73640f50789ac0fd66325499d82/developer_guide/03_00_architecture.md) | \`142472e...\` | Modelo de riscos do autoboot | Descrição de DryOS; não transplantar endereços para 5DC |
| [README da 5D no Soup](https://github.com/jiijii0/soup-engine/blob/6422c86b4530a5bdb4c5619f5287044ae7c661e3/Canon-EOS-5D/src/README.md) | \`6422c86...\` | Explicação de dependências para build e simulador | Não fornece SHA-256 do \`autoexec.bin\` original |
| [Build indicado pelo Soup](https://builds.magiclantern.fm/5DC.html) | Link de distribuição histórica | Candidato para encontrar o binário-base | **Nenhum arquivo baixado, inspecionado ou hash calculado nesta sprint** |

## Distinção entre testes

- **Host/CI:** compilação C, testes de manifesto, validação do inventário de evidências, checagem estrutural de arquivos sintéticos.
- **Simulação real da câmera:** depende de um modelo validado da 5DC e do executável-base exato; ainda não efetuada.
- **Câmera física:** exige equipamento, cartão e plano de recuperação; ainda não efetuada.

Nenhum arquivo executável nem ROM Canon foi incorporado ao repositório para a Sprint 1.
