# Rebobina 5DC.111 — análise do carregador, flags e continuação Canon

**13/11/2026 — pesquisa documental + validação e simulação offline. Nenhum boot real, .FIR, AUTOEXEC.BIN ou escrita em CompactFlash.**

## Descoberta: instalador histórico da 5D Classic

Foi localizado no código GPLv2 de autoria do **Magic Lantern Team** um diretório específico para `installer/5DC.111`, conservado no fork histórico [frantony/magic-lantern@a3454ebed3f6adcd165ee9382f451f917d989253](https://github.com/frantony/magic-lantern/tree/a3454ebed3f6adcd165ee9382f451f917d989253/installer/5DC.111) (08/08/2013).

Arquivos estudados:

- [`bootdisk.c`](https://github.com/frantony/magic-lantern/blob/a3454ebed3f6adcd165ee9382f451f917d989253/installer/5DC.111/bootdisk.c): pesquisa de flag de boot; verifica *duas assinaturas* de instruções do bootloader antes de modificar estado persistente.
- [`entry.S`](https://github.com/frantony/magic-lantern/blob/a3454ebed3f6adcd165ee9382f451f917d989253/installer/5DC.111/entry.S): salta para função que **altera flags**. Não é um bootstrap seguro para uso direto pelo Rebobina.
- [`link.script`](https://github.com/frantony/magic-lantern/blob/a3454ebed3f6adcd165ee9382f451f917d989253/installer/5DC.111/link.script): define **0x00800000** no *linker* do instalador; há comentário sobre outro endereço, por isso prevalece o código efetivo e a divergência exige investigação.
- [`installer/Makefile`](https://github.com/frantony/magic-lantern/blob/a3454ebed3f6adcd165ee9382f451f917d989253/installer/Makefile): **5DC.111 não está na lista de `SUPPORTED_INSTALLERS`**. Não supor suporte/reprodutibilidade da build geral.
- [`platform/5DC.111/Makefile.platform.default`](https://github.com/frantony/magic-lantern/blob/a3454ebed3f6adcd165ee9382f451f917d989253/platform/5DC.111/Makefile.platform.default): `RESTARTSTART=0x00980000` na **imagem realocada** do Magic Lantern; não confundir com carregamento inicial.

### Riscos documentados no instalador antigo

Ele faz leitura de configuração persistente no endereço relatado `0xF8000000`, verifica instruções da ROM do carregador e **altera a segunda word da boot flag por alternância**, redefinindo outras duas words antes de chamar rotinas de escrita na memória de boot e no CF. Uma segunda execução pode, em princípio, inverter o estado novamente; sua estrutura é inadequada para uma instalação idempotente. Não executar, compilar para câmera nem adaptar cegamente.

| Referência histórica | Endereço / instrução relatado |
| --- | --- |
| Entrada do instalador | `0x00800000` |
| Imagem ML realocada | `0x00980000` |
| Endereço mapeado das boot flags | `0xF8000000` |
| Função de leitura das flags | `0xFFFF8AE0`, prólogo `0xE52DE004` |
| Função de escrita das flags | `0xFFFF89F0`, prólogo `0xE92D41F0` |
| Escrita da flag de boot no cartão | `0xFFFF4074` |

**São observações do código de 2013, não endereços que o Rebobina mediu ou aprovou em hardware.** Sua existência não valida o bootflag do corpo da câmera do operador. O código histórico também não confirma que as flags estejam atualmente desativadas: a câmera ainda não foi sondada pelo Rebobina.

## Implementação no Rebobina

**Código novo** em `camera/5DC.111/bootstrap/boot_chain.[ch]` sob GPL-2.0-only, com atribuição à engenharia reversa GPLv2 do Magic Lantern Team. Não copia o instalador, não faz chamadas ROM nem lê/escreve memória mapeada. Possui apenas classificadores e uma decisão *fail-closed* a partir de dados injetados em testes.

**Ordem de checagem:** modelo exato → firmware exato → separação de endereços históricos → evidência de ROM coletada (ainda inexistente) → prólogos da ROM conhecidos → flags com layout conhecido → rejeição de flags já ativas → fonte identificada → RAM validada → continuação Canon validada → recuperação comprovada. Se todos os campos sintéticos forem considerados válidos, a saída continua sendo `RESEARCH_READY_NOT_AUTHORIZED`. Nenhuma saída habilita execução física.

Os três valores de flags classificados como *off* (`0, 0, 0xFFFFFFFF`) ou *on* (`0, 0xFFFFFFFF, 0xFFFFFFFF`) derivam da convenção do instalador histórico. **Não são um diagnóstico do corpo real**. Qualquer outra composição é rejeitada, sem tentar normalizar/escrever.

### Simulação do handoff/fallback

`boot_handoff_mock.[ch]` permite injetar duas funções falsas no computador: uma representando inicialização Rebobina e outra representando continuação do firmware Canon. Com evidência incompleta, a simulação ignora o módulo; com evidência sintética completa, pode executar o módulo. **Em ambos os cenários o teste exige uma chamada à continuação simulada da Canon, inclusive quando o módulo falha.** Isso exercita a política de recuperação, mas **não implementa um handoff físico nem garante que a Canon inicializaria**.

No ELF ARM de pesquisa, `rb_stage0_prepare` agora vincula o novo pré-boot e fornece evidência de ROM intencionalmente ausente. O verificador retorna `NO_ROM_OBSERVATION`; a entrada continua no loop deliberado, sem firmware handoff real. O `rb_5dc_handoff_mock_run` não é vinculado ao ELF ARM e continua exclusivo do host.

## Testes

```sh
make test
make audit-platform
make offline-bootstrap
```

Testes cobrem modelo ou firmware errado, endereços confundidos, ausência de observação ROM, instrução divergente, flags desconhecidas ou já habilitadas, proveniência, memória, handoff e recuperação não verificados, além de falhas injetadas na inicialização simulada. Um auditador compara o JSON de proveniência ao código, rejeita metadados que afirmem instalação aprovada e inspeciona o diretório para impedir caminhos de escrita da boot flag.

## Condições ainda necessárias para o boot real

1. Provar a identidade da ROM e do carregador Canon da unidade física **antes** de qualquer escrita persistente; não há ferramenta instalada de leitura segura para isso.
2. Especificar e testar um carregador independente com inicialização VxWorks, reserva de RAM, estados CPU/cache e transferência de controle **real** ao firmware original.
3. Demonstrar que falhas de boot (antes e depois de habilitar flags) podem ser revertidas com o CF separado, backup e procedimento de recuperação verificado — **dois cartões não garantem recuperação de boot flags**.
4. Resolver versão de toolchain e auditoria binária para build reproduzível; manter a PR #1 experimental isolada.
5. Nenhum instalador `.FIR` ou `AUTOEXEC.BIN` é autorizado por estes testes. A câmera permanece em firmware Canon 1.1.1 original.

## Fonte de licença e atribuição

Magic Lantern Team e colaboradores (GPLv2), preservados no fork histórico fixado acima. O código de pré-boot e de simulação do Rebobina foi escrito em 13/11/2026 com base nessa pesquisa; confira [reuso anterior](ML-5DC-REUSO.md) e [bootstrap offline](5DC-BOOTSTRAP-OFFLINE.md).
