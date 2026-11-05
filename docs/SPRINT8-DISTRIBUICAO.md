# Sprint 8 — Distribuição e instalação (planejada)

O instalador é uma ferramenta auxiliar para o **Rebobina, software próprio na câmera**. Não será um instalador do Magic Lantern como produto principal.

## Política de distribuição futura

- Publicar binários **somente após** validação de hardware por modelo e versão de firmware.
- Vincular artefatos a código e toolchain rastreáveis, com hashes SHA-256.
- Nunca misturar arquivos de câmeras diferentes.
- Tornar obrigatórios backup, verificação de compatibilidade e recuperação documentada.
- Evitar operações no boot flag ou no cartão sem confirmação explícita.
- Exibir informações sobre risco e condição experimental no instalador.

## Estado atual

- `build/rebobina-host`: protótipo de computador.
- `build/rebobina-ui-host`: simulação de menu em terminal.
- `build/rebobina-plugin-host`: protótipo Linux de extensão via `dlopen`.
- `build/arm/*.o`: objetos ARM **não executáveis e não instaláveis**.

**Não existe `AUTOEXEC.BIN` ou release Rebobina para a 5D.**
