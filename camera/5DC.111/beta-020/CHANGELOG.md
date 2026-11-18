# REBOBINA 0.2.0-beta

## Novidades desde 0.1.2

- Primeiro runtime embarcado interativo e persistente; não encerra aos ~40 segundos.
- Redesenho do LCD **até 10 Hz** com altura completa 720×480 lógica e guarda para display/bitmap pronto.
- Tela inicial + Film, Roll, Tools, Modules, Preferences, About. Menu via roda/SET; MENU volta, lixeira esconde/exibe. Shutter e demais eventos passam para Canon.
- Seletor de quatro perfis com nomes Soup; configuração funciona, **filtros de fotos ainda somente no host**.
- Caderno de rolos com contagem manual, novo rolo, desfazer, reset e salvamento.
- Configuração persistente com dois slots, checksums e recuperação após corrupção; auto-save com debounce.
- Sistema de módulos estáticos Roll e Diagnostics com toggles funcionais; module ABI dinâmico ainda host-only.
- Teste de memória de 4 KiB e CF com readback; diagnóstico exportável sob demanda.
- Adaptador host separado para desenvolver CR2 com o perfil salvo na 5D, usando o executável Linux do Rebobina.
- CI, validação binária, mocks e testes host (incluindo recuperação e falhas).

## Limites

O beta ainda depende do AUTOEXEC.BIN previamente preparado e validado do Soup Engine, não altera Canon ROM ou bootflags, não transforma JPEG/CR2 na câmera e não é uma distribuição para uso profissional. O **binário 0.2.0-beta ainda não foi testado em hardware**. O 0.1.2 foi observado cobrindo a tela toda, mas recebeu feedback de flicker a 4 Hz, daí a mudança para 10 Hz.
