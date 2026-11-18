# REBOBINA 0.1.1-dev

- Resultado 0.1.0: desenhos observados em LCD real; o menu Canon/ML por vezes cobre o texto. Os quatro arquivos RB5 do teste físico foram gerados.
- Três modos no mesmo payload: HEADLESS, SHARED e FOCUS, configurados por marcadores.
- FOCUS: chamada opcional a bmp_fill sobre região do overlay existente, guarda da tela bitmap, redesenho 4 Hz por 38s.
- Navegação (SET, roda) consumida somente enquanto FOCUS está ativo; MENU sai do modo e restaura pass-through. Nenhuma chamada de arquivo na GUI.
- Novos indicadores de foco, preenchimento, bitmap e eventos, arquivos isolados RB6*.TXT.
- Mantém HAL Canon 5DC.111 e validação offline, sem alteração da ROM ou AUTOEXEC.
- O teste físico do modo FOCUS continua pendente. Não é framebuffer próprio.
