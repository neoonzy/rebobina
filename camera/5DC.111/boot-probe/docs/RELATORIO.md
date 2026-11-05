# Sprint 3 — Boot Probe 0.0.1: relatório de preparação

**Status:** compilado e validado offline; **execução em câmera física não testada**.

| Verificação | Resultado |
| --- | --- |
| Compilação freestanding para ARM946E-S | Aprovada no ambiente local |
| Link ELF32 ARM / sem relocação pendente | Aprovado |
| Binário `SE.BIN` com cabeçalho `FILM` | Aprovado |
| Entradas `+0x04` e `+0x44` | Aprovadas por testes estruturais |
| 10 testes Python com fixtures sintéticas | Aprovados localmente |
| Validação conjunta com `AUTOEXEC.BIN` real do Soup | **Pendente** |
| Teste de boot na EOS 5D Classic 1.1.1 | **Pendente** |
| Exibição de interface própria | **Não implementada** |

**Relevância:** pela primeira vez há um candidato de payload Rebobina em código ARM que segue o protocolo do carregador da 5D relatado no [Soup Engine](https://github.com/jiijii0/soup-engine/tree/6422c86b4530a5bdb4c5619f5287044ae7c661e3/Canon-EOS-5D/src). Ele não constitui firmware Rebobina independente.

**Limitações:** o teste de arquivo de diagnóstico utiliza `FIO_CreateFile`, `FIO_WriteFile` e `FIO_CloseFile` em endereços documentados pelo port para a Canon 5D Classic 1.1.1. A compatibilidade do `AUTOEXEC.BIN` real ainda não foi testada pelo projeto. Um arquivo de diagnóstico pode não aparecer por falha de gravação mesmo que o boot tenha sido alcançado; uma falha física pode travar a câmera.

**Critério de aceite em hardware:** confirmar versão Canon 1.1.1, hash do `AUTOEXEC.BIN` de origem, teste com CF de desenvolvimento, cópia do arquivo `REBOBINA.TXT`, observações de boot e desligamento, procedimento de recuperação realizado e registrável.

**Atenção:** não copiar `SE.BIN` para um CF com fotos importantes. Não apagar `AUTOEXEC.BIN` de cartão marcado como inicializável.


## Diferença de toolchain observada

- **Clang/LLD 17 local:** `SE.BIN` de **504 bytes**, SHA-256 `f50b41ef6d70be894117277a8f7c1984e571ddc2a10ff471b0371805e81a8834`.
- **Build Ubuntu/GitHub Actions:** `SE.BIN` de **500 bytes**, SHA-256 `369aae5f3eab3ba07b246fdb07292c7714b02daf8839efd19a6608ecc2fdf94a` ([CI 42131517911](https://github.com/neoonzy/rebobina/actions/runs/42131517911)).
- **Os dois passaram nos testes offline**, mas NÃO são bit a bit idênticos; o processo de build ainda precisa fixar toolchain/versões para reprodutibilidade.
- O ZIP experimental fornecido na conversa corresponde ao **build local de 504 bytes** e traz o respectivo manifesto. Não misturar um `SE.BIN` de outro build com seu manifesto.
