# Versionamento e compatibilidade

A numeração do Rebobina é **pré-alfa** até o primeiro executável validado na 5D Classic.

- **Versão de produto:** começará em `0.x.y` e seguirá SemVer quando existirem releases de software embarcado.
- **Versão da API C:** `RB_MODULE_ABI_VERSION` em `include/rebobina/module.h` é atualmente `1`, experimental e exclusiva do protótipo host. Mudanças incompatíveis exigem aumento dessa constante e atualização dos testes.
- **Manifesto:** `api_version: 1` e `version: MAJOR.MINOR.PATCH`, validados pelo utilitário `tools/modules.py`.
- **Alvo de câmera:** `5DC.111` significa exclusivamente EOS 5D Classic com firmware Canon 1.1.1. Não usar `5D2.212`, `5D3` nem outras variantes como substitutas.
- **Versão de firmware/boot:** não existe release nem identidade de binário Rebobina aprovado até este momento.
- **Compatibilidade Soup Engine:** planejada; um manifesto não equivale a um binário compatível.

## Protocolo para releases futuros

Registrar tag, hash do commit, toolchain e versão, build e arquivos verificados, hash SHA-256 de cada artefato, versão de firmware Canon e matriz de testes. Não publicar um binário para câmera com base apenas em CI host.

## Regras de alterações

Interfaces `rb_*` podem mudar durante `0.x`; toda mudança deve incluir teste, atualização de documentação e explicação no changelog. Componentes de terceiros exigem rastreabilidade e os avisos de GPLv2.
