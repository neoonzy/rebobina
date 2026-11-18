# Estado da 5DC — marco de novembro de 2026

### Evidência física preservada

- `0.0.1`: SE.BIN executou e criou `REBOBINA.TXT` no CompactFlash.
- `0.0.3`: RAM de 256 B, VxWorks worker, leitura/gravação do CF e eventos de botões.
- `0.0.4`: leitura correta de `event->param` e eventos MENU/INFO/SET/Lixeira.
- `0.1.0`: cinco páginas experimentais apareceram **visivelmente** no LCD; menu Canon passava por cima do texto.
- `0.1.1`: fundo preto no modo FOCUS visível, mas cobriu somente metade superior do LCD.
- `0.1.2`: usuário confirmou fundo cobrindo **o LCD todo**, mas percebeu piscadas/flicker a aproximadamente 4 Hz; `RB7BOOT` e `RB7TEST` confirmam boot, RAM e CF. Não foram fornecidos `RB7FINAL` ou eventos da sessão. A 0.2.0 deve repintar a 10 Hz.

### Interfaces e camadas

O foco experimental usa `bmp_fill` e `bmp_printf` de Magic Lantern, carregados em memória como parte do `AUTOEXEC.BIN` Soup previamente validado. Isso não constitui camada independente: o menu Canon/ML disputa pixels e pode reaparecer entre quadros. A abordagem de filme Soup completo desenha num buffer dedicado e copia para a VRAM; é uma oportunidade para versão futura, mas não foi copiada de forma improvisada sem um teste de integração e orçamento de memória.

### Recursos host que ainda não estão no payload

- `src/core/{manager,registry,ui}.c`: modelo de UI/serviços no host; beta porta semanticamente o menu e os módulos simples, sem alegar carregar `.so`.
- `src/host/cr2_develop.c`: revelador real do **JPEG embutido em CR2** usando engine Soup e saída protegida. Beta exporta configuração e integra um lançador host; o binário Canon não processa fotos.
- `modules/soup-engine`: status planned. Próxima grande fase é integrar o motor de imagens na 5D, com buffers e tratamento de arquivos independentes do hook.

### Critérios da próxima validação única

1. O novo beta liga e permanece ativo após 40 segundos.
2. Menu ocupa toda a altura do LCD, opera à taxa alvo de 10 ciclos por segundo quando o corpo dá conta (CPU e CF podem reduzir a taxa real).
3. Navegação da tela Film/Roll, alterações persistentes após desligar e religar, rollback se cfg corrompida.
4. `RB8BOOT.TXT`, `RB8TEST.TXT`, `RB8SESSION.TXT` e ao menos um `RB8CFG*.BIN` gerados quando se faz salvar/exportar.
5. A Canon permanece capaz de fotografar normalmente: esta build não modifica nenhuma fotografia e não deve engolir eventos do disparador.

### Pontos que NÃO foram homologados

Runtime contínuo em standby/shutdown, integridade do CF em quedas de energia, latência de interação sob fotografia, ausência de flicker e adaptação de módulo JPEG/CR2. Se o corpo emitir Err ou congelar, interromper o teste. Recuperar com o antigo `SE.BIN`; nunca modificar `AUTOEXEC.BIN` durante a recuperação.
