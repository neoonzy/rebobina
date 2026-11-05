# Sprint 7 — Testes de estabilidade (fundação antecipada)

Os testes atuais ainda são **host**, não testes de estresse numa Canon real.

## Testes adicionados desde a Sprint 2

- Navegação normal, menu fechado e seleção por botões.
- **100.000 eventos determinísticos** de UI, incluindo entradas inválidas, com invariantes `opened ∈ {0,1}` e `selected < RB_UI_ITEMS`.
- Carregamento host de módulo independente `.so`, incluindo **rejeição** de ABI errada e ausência do símbolo exportado.
- Auditoria offline de endereços de documentação e regiões de RAM sem sobreposição.
- Compilação freestanding para objetos ARM946E-S (não é boot).

## Futuras verificações obrigatórias

- [ ] Valgrind/ASan/UBSan e cobertura dos componentes host onde suportados.
- [ ] Interrupções de I/O, card cheio, leitura parcial, falhas de alocação, suspend/resume (simulado).
- [ ] Isolamento de filas de eventos de câmera e menu, sem trabalho de I/O em callback de botões.
- [ ] Medição de pilhas e memória de módulos em hardware.
- [ ] Testes de recuperação com procedimento controlado e documentação de incidentes.
- [ ] Critérios de aceitação com evidências fotográficas, logs e versões dos binários.

**Condição:** só declarar Sprint 7 concluída quando o firmware estiver operando na 5D e os casos críticos tiverem testes reais. Testes positivos host não demonstram segurança em câmera.
