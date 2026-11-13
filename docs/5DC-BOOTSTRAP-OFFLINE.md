# 5DC.111 — Bootstrap próprio do Rebobina (ELF offline)

**13/11/2026. Protótipo ARM ligado e auditado no computador; NÃO inicializa a Canon EOS 5D Classic.**

## Objetivo e limite

Este marco elimina a **dependência de build** de um `autoexec.bin` do Magic Lantern: o código de entrada e o núcleo ligados ao ELF são próprios do Rebobina, usando pesquisa GPLv2 do Magic Lantern como referência. **Não elimina a dependência técnica futura de compreender os mecanismos Canon de boot por cartão** nem prova a inicialização no hardware.

O alvo `make offline-bootstrap` compila um **ELF32 ARM946E-S** com entrada `rb_stage0_entry` e o linker `stage0_offline.ld`. O arquivo gerado é `build/arm/5dc-stage0-offline.elf`. **Não gera imagem raw, `AUTOEXEC.BIN`, instalador `.FIR`, configuração de boot flag ou qualquer arquivo para CF.** O ELF não possui cabeçalho/empacotamento da bootloader Canon e **não deve ser renomeado ou convertido para uso na câmera**.

## Fonte histórica e créditos

- **Magic Lantern Team**, GPL versão 2: [snapshot do port histórico `platform/5DC.111`](https://github.com/frantony/magic-lantern/tree/a3454ebed3f6adcd165ee9382f451f917d989253/platform/5DC.111), preservado em um fork no commit de **08/08/2013**.
- [`local_entry.S`](https://github.com/frantony/magic-lantern/blob/a3454ebed3f6adcd165ee9382f451f917d989253/platform/5DC.111/local_entry.S): entrada ARM histórica e bootstrap VxWorks com operações privilegiadas de baixo nível.
- [`init.c`](https://github.com/frantony/magic-lantern/blob/a3454ebed3f6adcd165ee9382f451f917d989253/platform/5DC.111/init.c): pesquisa da reserva de RAM, tarefas, patches e transição para o firmware Canon.
- [`Makefile.platform.default`](https://github.com/frantony/magic-lantern/blob/a3454ebed3f6adcd165ee9382f451f917d989253/platform/5DC.111/Makefile.platform.default): `ROMBASEADDR = 0xFF810000` e `RESTARTSTART = 0x00980000` relatados historicamente.
- [`stubs.S`](../camera/5DC.111/ml_compat/rom_stubs.S): seleção já incorporada no Rebobina, mas **não vinculada ao ELF stage0**; não existem chamadas à ROM Canon neste marco.
- **Rebobina contributors (2026-11-13):** implementaram os novos arquivos `bootstrap/*` e o auditador; são adaptações conceituais, **não uma cópia executável** do `local_entry.S`. Mantemos os créditos à pesquisa do Magic Lantern e a licença GPL-2.0-only do repositório.

## Comportamento real do protótipo

1. O ELF declara uma entrada ARM própria **`rb_stage0_entry`**, vinculada a `0x00980000` **exclusivamente como hipótese de link**.
2. O assembly prepara a pilha de 4 KiB **em memória hipotética** e chama `rb_stage0_prepare`.
3. A rotina C inicializa a área `.bss`, o registro de módulos e o modelo de interface do Rebobina.
4. Executa uma verificação estrutural *sobre um descritor sintético* (5DC.111; 1.1.1; intervalo histórico); o resultado é `RB_STAGE0_RESEARCH_MATCH_ONLY`, **nunca autorização de boot**. Também chama o novo [pré-boot offline](5DC-BOOT-CHAIN.md) com os endereços históricos separados (`0x00800000` do carregamento antigo e `0x00980000` da realocação do ML), **sem observação real da ROM**; nesse caso deve retornar `RB_5DC_BOOT_NO_ROM_OBSERVATION`.
5. A entrada ARM permanece num loop deliberado. **Não há passagem de controle para a Canon nem código de restauração de firmware. Portanto, não executar na câmera**.

Os endereços `0x00980000`–`0x00A00000` **não são uma reserva comprovadamente segura** para o bootstrap do Rebobina. Foram retirados do estudo histórico de realocação da antiga versão do Magic Lantern, onde também há patches de cache e memória. O linker aplica limite conservador ao **ELF de pesquisa**, não valida disponibilidade da RAM na Canon.

## Verificação offline

```sh
make test
make arm-objects
make offline-bootstrap
```

O `make offline-bootstrap` requer `clang`, `ld.lld` e `readelf` no Linux (ou ambiente compatível), e só cria o `.elf`. A suíte de host exercita configurações válidas/erradas da especificação e rejeita confusão entre 5D Classic, 5D Mark II, versão de firmware e limites de memória. O auditador verifica:

- ELF32 **ARM**, little-endian, tipo **EXEC** e endereço de entrada da hipótese;
- faixa dos segmentos LOAD, símbolos de BSS/pilha, vínculo ao núcleo `rb_registry_init`/`rb_ui_init`;
- ausência de símbolos globais indefinidos, de chamadas a ROM Canon e presença da marca **OFFLINE ONLY**;
- nenhum `.BIN` ou `.FIR` produzido.

Esses são **testes de ligação e consistência estática**, e não simulação de VxWorks ou boot real. Qualquer erro no CI bloqueia a PR.

## O que ainda falta para um primeiro teste físico

- **Cadeia de boot Canon:** documentar formato, entrada, modo de carregamento por CF, boot flag e requisitos de cartão para a 5DC.111; integrar o mecanismo selecionado sem assumir identidade com o binário Magic Lantern 2012 usado pelo Soup.
- **Mapa de memória e ABI:** validar qual RAM pode ser usada, estado inicial de CPU/MPU/cache e pilha, serviço de tarefas, inicialização VxWorks, concorrência e pontos de entrada Canon.
- **Handoff/fallback:** garantir que uma falha do Rebobina não interrompa a inicialização da Canon; o protótipo atual **não faz handoff**.
- **Reprodutibilidade e diagnóstico:** fixar toolchain, inspeções ELF/mapa, método de saída observável e logs no hardware.
- **Recuperação:** procedimento específico testado antes de modificar boot flags. Dois CFs e backup já foram relatados, mas não substituem recuperação do corpo.
- **Ensaios físicos de desenvolvimento:** somente após todos os critérios da [Sprint 3](SPRINT3-CHECKLIST.md) e com risco aceito. Não há atualmente release instalável.

## Decisão de produto

**Rebobina será um programa próprio, não exigirá que as pessoas instalem o Magic Lantern previamente.** A compatibilidade com a 5D Classic poderá ser implementada com reuso seletivo de código GPLv2, com atribuição e alterações declaradas, desde que a cadeia de inicialização seja comprovada.

## Pré-boot e continuação simulada (13/11/2026)

Foi acrescentada uma [auditoria do instalador histórico da 5DC](5DC-BOOT-CHAIN.md), um classificador sem efeitos colaterais das flags de inicialização e um modelo de continuação da Canon **exclusivamente em host**. O estágio ARM continua sem handoff real e sem I/O. A simulação garante apenas que a lógica de testes não pule o callback Canon quando uma etapa simulada falhar. **Não instalar nem converter o ELF**.
