# Instruções de desenvolvimento para colaboradores e agentes

Projeto: **Rebobina** — arquitetura modular experimental para Canon EOS 5D Classic 1.1.1.

1. **Ler** `README.md`, `docs/ARQUITETURA.md`, `docs/SEGURANCA.md` e `docs/ROADMAP.md` antes de propor mudanças profundas.
2. **Não afirmar suporte real à câmera.** O simulador visual em `simulator/web/` é guiado pelo core C em localhost, mas não emula DIGIC II/VxWorks nem testa a Canon. `module.json` é descritivo, não um binário instalável.
3. **Não escrever firmware/boot flags/CF**, não criar ou distribuir binários prontos para câmera sem solicitação explícita, revisão, testes e documentação de recuperação.
4. **Não incorporar binários ou código proprietário** de terceiros. Respeitar a GPLv2 e os créditos Magic Lantern/Soup Engine.
5. **Testar primeiro no host** com `make hal-demo`, `make test`, `make check-modules`, `make audit-platform` e `make arm-objects`; usar `make web-sim` para inspeção manual do simulador. Reportar resultados e limitações sem fabricar aprovação.
6. **Manter os componentes separados:** `src/core` independente de hardware; futuras chamadas Canon sob HAL específico por câmera; extensões não devem embutir endereços e hooks na lógica de negócio.
7. **Não confundir o formato `.mo` do Magic Lantern** com a API experimental `rb_module`. Adaptação exige trabalho real de engenharia.
8. Toda funcionalidade nova deve atualizar testes e, se mudar design, a documentação correspondente.
9. Evitar refatoração massiva sem razão. Preferir commits pequenos e compatíveis com testes reprodutíveis.

10. A HAL em `src/host/hal_sim.c` é **somente RAM** e não representa o filesystem CF. Não inventar serviços Canon nem declarar falha simulada como teste de hardware.
11. O Soup Engine upstream deve ser testado na revisão GPLv2 pinada; o script `tools/soup_host.py` aplica portabilidade Linux **somente no checkout temporário**. Não commitar binários/Soup vendor sem revisão de licença e origem.
12. `rb_output` exige inicialização antes do primeiro uso e recuperação explícita de `.TMP` após falhas; não sobrescrever CR2 ou JPEG existentes. A semântica de `sync`/`rename_noreplace` em RAM **não comprova** suporte Canon FAT/VxWorks. Não executar limpeza automática em cartão físico.
13. O código `engine/look.c` do Soup Engine emite aviso `-Wstringop-overflow` no GCC 15; registrar o aviso e não mascarar possíveis defeitos de memória na câmera.
14. A função upstream `se_cr2_find_preview` não comprova sozinha que o comprimento JPEG cabe no arquivo: usar o wrapper defensivo `rb_cr2_host_find_preview` nos testes host e **não** declarar validação de RAW com um envelope TIFF sintético.
15. UBSan encontra deslocamento negativo indefinido em `engine/jpeg_enc.c`; `tools/soup_host.py` gera correção SOMENTE na cópia de sanitizador. Não aplicar silenciosamente no upstream ou no firmware. Ver `docs/SOUP-SANITIZERS.md`.
