# Reutilização do Magic Lantern histórico na Canon 5D Classic

**12/11/2026. Código integrado ao Rebobina para testes no computador; NÃO é firmware de câmera.**

## Proveniência e créditos

- Autores originais: **Magic Lantern Team e contribuidores históricos**.
- Snapshot usado: [frantony/magic-lantern @ a3454ebed3f6adcd165ee9382f451f917d989253](https://github.com/frantony/magic-lantern/tree/a3454ebed3f6adcd165ee9382f451f917d989253/platform/5DC.111), datado de 08/08/2013.
- O fork histórico contém o port real **platform/5DC.111**: init.c, local_entry.S, stubs.S, gui.c, gui.h, consts.h e configuração da plataforma.
- Licença no repositório histórico: **GPL versão 2**. O material selecionado mantém copyright do Magic Lantern Team, licença GPL-2.0-only na versão incorporada e avisos de alterações em 12/11/2026. O Rebobina permanece sob GPL-2.0-only.
- A fonte de 2013 foi localizada em um fork comunitário. **Não demonstramos que corresponde exatamente ao AUTOEXEC.BIN Magic Lantern 2012 de 221.184 bytes utilizado como base pelo Soup Engine.**

## O que foi incorporado

| Arquivo no Rebobina | Fonte ML original | Conteúdo |
| --- | --- | --- |
| [rom_stubs.S](../camera/5DC.111/ml_compat/rom_stubs.S) | platform/5DC.111/stubs.S e include/stub.h | Seleção de **10 símbolos absolutos ARM** para arquivos, memória, tarefas e GUI |
| [input.h](../camera/5DC.111/ml_compat/input.h) | platform/5DC.111/gui.h | **7 códigos físicos de botões/roda** |
| [input.c](../camera/5DC.111/ml_compat/input.c) | Adaptação nova dos códigos documentados | Tradução pura de códigos ML 5DC para eventos da interface Rebobina |
| [source.json](../camera/5DC.111/ml_compat/source.json) | Snapshot fixado | Registro verificável de origem, licença, constantes e limitações |

Os aliases da ROM apenas criam símbolos **ABS em um objeto ELF ARM relocável**. Não fazem nenhuma chamada Canon, não produzem AUTOEXEC.BIN, não instalam Magic Lantern e não mexem na boot flag.

Os eventos são traduzidos em memória, sem interceptar a câmera. Uma integração embarcada futura deve preservar os eventos do firmware Canon quando o Rebobina não estiver ativo.

## Como testar sem câmera

```sh
make test
make audit-platform
make arm-objects
```

Os testes validam números, licença/proveniência, eventos desconhecidos e os 10 endereços absolutos da tabela de símbolos. O compilador ARM só produz arquivos .o, **não binários executáveis de câmera**.

## O que aprendemos sobre o boot

- **Makefile.platform.default:** ROMBASEADDR 0xFF810000; RESTARTSTART 0x00980000 na configuração histórica.
- **local_entry.S e init.c:** caminhos próprios da 5D VxWorks, inicialização, reserva de RAM, cache e tarefas. O código histórico também contém rotinas de depuração e patches de ROM que não devem ser copiados cegamente.
- **gui.c e gui.h:** substituição/interceptação da tarefa GUI e parâmetros dos eventos. Interceptar essa tarefa incorretamente pode impedir o uso normal da câmera.
- **stubs.S:** endereços de funções VxWorks/Canon para o firmware 1.1.1, agora reconciliados com parte dos endereços já levantados via Soup.
- O [Magic Lantern moderno](https://github.com/reticulatedpines/magiclantern_simplified/tree/142472e95054d73640f50789ac0fd66325499d82) não contém 5DC.111 nesse snapshot e **não** deve fornecer endereços para esse corpo.

## O que continua bloqueado

1. Obter a procedência e o hash exato do **AUTOEXEC.BIN original do Magic Lantern 2012**; não confundir com o executável já modificado pelo Soup.
2. Determinar diferenças entre código 2013 e binário 2012 e verificar layout real de RAM, ABI, hooks de boot, cache e tarefas.
3. Desenvolver bootstrap próprio do Rebobina que não exija instalação prévia do Magic Lantern, mas possa reaproveitar seu código GPLv2.
4. Validar procedimento de recuperação **antes** de modificar boot flags ou executar qualquer payload experimental.
5. Só então testar em equipamento físico, começando por diagnóstico mínimo sem interferir em fotos ou firmware Canon.

**Status: pesquisa e integração offline. Não copiar artefatos gerados por este port para o CF.**
