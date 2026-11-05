# Sprint 3 — Preparação para o primeiro boot Rebobina

**Estado:** bloqueada. Não há um bootloader, linker de câmera validado ou `AUTOEXEC.BIN` Rebobina.

Este documento serve para evitar que a Sprint 3 seja declarada concluída apenas porque conseguimos compilar objetos ARM na Sprint 2.

## Condições mínimas antes de escrever código de boot

1. Identificar de maneira rastreável o binário-base e o código-fonte correspondente do Magic Lantern **5DC.111** (revisão, procedência e SHA-256).
2. Verificar com fontes e inspeção os locais de execução, as áreas de RAM, as tarefas e as convenções de chamada realmente aplicáveis à 5D Classic.
3. Definir um plano de recuperação e uma câmera **de desenvolvimento**, não de uso cotidiano, com risco explicitamente aceito.
4. Garantir que todo artefato de teste seja identificado como **experimental**, com versão, hash e incompatibilidades documentados.
5. Fazer inspeção estática e testes de simulação antes de usar hardware, além de definir quais diagnósticos podem ser observados se o display não responder.

## Critérios de aceite para primeiro boot

- [ ] Executável **Rebobina**, não um loader genérico ou build do Magic Lantern inalterado;
- [ ] Inicialização observada em Canon 5D Classic 1.1.1 real;
- [ ] Evidências de que a inicialização não sobrescreveu áreas inesperadas;
- [ ] Logs e relato das condições de teste;
- [ ] Comportamento de desligamento/reinicialização documentado;
- [ ] Procedimento de recuperação testado.

**Nenhuma dessas condições foi verificada ainda.** Compilação de `.o` ARM, menu host e `dlopen` Linux não contam como boot de câmera.
