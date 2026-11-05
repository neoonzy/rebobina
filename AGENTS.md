# Instruções de desenvolvimento para colaboradores e agentes

Projeto: **Rebobina** — arquitetura modular experimental para Canon EOS 5D Classic 1.1.1.

1. **Ler** `README.md`, `docs/ARQUITETURA.md`, `docs/SEGURANCA.md` e `docs/ROADMAP.md` antes de propor mudanças profundas.
2. **Não afirmar suporte real à câmera.** O protótipo atual roda apenas no computador. `module.json` é descritivo, não um binário instalável.
3. **Não escrever firmware/boot flags/CF**, não criar ou distribuir binários prontos para câmera sem solicitação explícita, revisão, testes e documentação de recuperação.
4. **Não incorporar binários ou código proprietário** de terceiros. Respeitar a GPLv2 e os créditos Magic Lantern/Soup Engine.
5. **Testar primeiro no host** com `make test`, `make check-modules`, `make audit-platform` e `make arm-objects`. Reportar resultados e limitações sem fabricar aprovação.
6. **Manter os componentes separados:** `src/core` independente de hardware; futuras chamadas Canon sob HAL específico por câmera; extensões não devem embutir endereços e hooks na lógica de negócio.
7. **Não confundir o formato `.mo` do Magic Lantern** com a API experimental `rb_module`. Adaptação exige trabalho real de engenharia.
8. Toda funcionalidade nova deve atualizar testes e, se mudar design, a documentação correspondente.
9. Evitar refatoração massiva sem razão. Preferir commits pequenos e compatíveis com testes reprodutíveis.
