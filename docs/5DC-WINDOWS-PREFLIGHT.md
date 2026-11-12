# Canon EOS 5D Classic 1.1.1 — preparação física no Windows

**Atualizado em 11/11/2026. Estado: firmware Canon atualizado; primeiro boot Rebobina NÃO autorizado.**

Este registro serve ao primeiro teste de hardware da Sprint 3. **Não cria cartão inicializável, não ativa boot flags, não instala Magic Lantern/Soup/Rebobina e não executa nenhum arquivo na câmera.**

## Estado confirmado pelo operador

Na conversa de 11/11/2026, o usuário informou que concluiu a atualização da Canon EOS 5D **original (Classic)** e confirmou:

- [x] Menu da câmera indica versão **1.1.1**.
- [x] Desliga e liga normalmente.
- [x] Fotografa, salva e exibe **JPEG**.
- [x] Fotografa em **RAW (CR2)** e transfere arquivos para o Windows.
- [x] Sem comportamento incomum nos testes básicos relatados.

**Fonte da evidência:** relato do operador, não logs, fotografias ou testes executados por terceiros. Isso **não valida o Rebobina** e não substitui testes físicos de boot.

Ainda é necessário confirmar backup independente e disponibilidade de **cartão CF exclusivo de testes** (idealmente diferente do cartão usado para fotos importantes), além de registrar capacidade e formato do CF.

## Preparação segura no computador Windows — SEM escrever no CF

1. Com a câmera desligada e o LED de acesso apagado, conecte o CF ao leitor.
2. Copie as fotos e pastas **sem mover ou apagar** e verifique arquivos JPEG/CR2 copiados. Guarde uma segunda cópia se possível.
3. **Não** formate o cartão nem prepare flags de boot até termos um plano de recuperação validado.
4. No GitHub do upstream, selecione **somente o pacote \`Canon-EOS-5D.zip\`**, não \`Canon-EOS-5D-Half-Frame\`, 5D Mark II ou qualquer outro modelo.
5. Antes de extrair ou copiar arquivos, confirme o hash do ZIP **no Windows** e inspeção estrutural **somente leitura** conforme a próxima seção.

### Pacote de referência do Soup Engine

Fonte: [Soup Engine — Canon EOS 5D Classic](https://github.com/jiijii0/soup-engine/tree/master/Canon-EOS-5D) e [releases upstream](https://github.com/jiijii0/soup-engine/releases/tag/folders).

**Snapshot observado pela API oficial do GitHub em 11/11/2026, não um hash permanente do projeto:**

| Item | Valor |
| --- | --- |
| Release tag | \`folders\` (publicada em 04/11/2026; arquivos atualizados automaticamente) |
| Asset | \`Canon-EOS-5D.zip\` |
| Tamanho do ZIP informado | **968.459 bytes** |
| SHA-256 do ZIP informado | \`0f8cc374374d6692ec77836cd979ee75d2c77ae8a7a5bdd8da1cd5e6d0c9c993\` |
| Data do asset na API | 11/11/2026 22:51:31 UTC |
| GitHub source tree | \`Canon-EOS-5D/card/AUTOEXEC.BIN\`, \`card/SE.BIN\`, \`card/ML/\` |
| Tamanho do \`AUTOEXEC.BIN\` no source tree | 221.184 bytes |

**Atenção:** o upstream publica o pacote automaticamente; se o SHA-256 do arquivo baixado for diferente, **pare**. Não desative a verificação nem suponha que a mudança seja inofensiva; compare a nova versão com a API oficial e revise novamente. **Hash do ZIP prova identidade com o asset publicado, não autenticidade do código-base histórico do Magic Lantern.**

Abra o **PowerShell** na pasta do projeto Rebobina. Para apenas calcular o hash do ZIP baixado:

~~~powershell
Get-FileHash -LiteralPath "$env:USERPROFILE\Downloads\Canon-EOS-5D.zip" -Algorithm SHA256
~~~

O hash deve coincidir exatamente com o snapshot acima para este conjunto de instruções. Para conferir o hash e a estrutura **sem extrair nem gravar nada**, use:

~~~powershell
.\tools\inspect-5dc-soup-release.ps1 -ZipPath "$env:USERPROFILE\Downloads\Canon-EOS-5D.zip"
~~~

O script **não pede letra de unidade CF, não cria arquivos e não modifica cartões**. Requer PowerShell 5.1 ou 7+ e permissão local para executar o script. Ele valida o hash do ZIP, presença de \`AUTOEXEC.BIN\`/ \`SE.BIN\` / \`ML/FILM/*.fs\`, o tamanho esperado do executável legado e os marcadores \`gaonisoy\`/\`FILM\`. Essas checagens são **estruturais** e **não** comprovam boot seguro, endereços corretos, compatibilidade do loader ou origem do Magic Lantern de 2012.

**Não coloque os arquivos \`AUTOEXEC.BIN\`, \`SE.BIN\` nem o payload experimental do Rebobina no cartão neste estágio.**

## O que falta provar antes do primeiro boot experimental

- [ ] Obter/identificar a distribuição **Magic Lantern 5D Classic para firmware 1.1.1**, com origem verificável, notas específicas e executável-base associado. A página indicada pelo Soup é https://builds.magiclantern.fm/5DC.html; **a página não pôde ser recuperada automaticamente nesta revisão**, portanto disponibilidade e instruções atuais exigem conferência manual.
- [ ] Identificar **hash SHA-256 exato do AUTOEXEC.BIN de origem** e compatibilidade com o patch, e não apenas assinatura \`gaonisoy\` e tamanho.
- [ ] Resolver diferença conhecida de build do Boot Probe (**500 versus 504 bytes**) fixando versões da toolchain.
- [ ] Revisar endereços ROM, memória, init e hooks de GUI próprios da **5DC.111**, incluindo alterações no upstream Soup.
- [ ] Fazer teste offline do carregador **exato** e suas regras de fallback/rollback.
- [ ] Confirmar **CF dedicado**, cópia das fotos, bateria carregada e um procedimento realista de recuperação.
- [ ] Revalidar a PR [#1](https://github.com/neoonzy/rebobina/pull/1) e manter explícito \`OFFLINE-ONLY\` até cumprir todos os critérios.

O pacote upstream atual do Soup inclui um \`AUTOEXEC.BIN\` derivado de um Magic Lantern antigo. A base de origem não pode ser deduzida do hash do ZIP ou do tamanho. O [script upstream de patch](https://github.com/jiijii0/soup-engine/blob/master/Canon-EOS-5D/src/tools/patch-film.py) contém variantes e comentários históricos, e a PR #1 exige análise de compatibilidade com a versão efetivamente instalada.

## Plano de recuperação a preparar (não executar agora)

1. Guardar fora do CF uma cópia íntegra do **estado anterior**, incluindo fotos, \`AUTOEXEC.BIN\` original de Magic Lantern, \`SE.BIN\` e perfis, **se/quando** instalados.
2. Documentar se e como o **boot flag** foi ativado. Não inferir reversão somente ao formatar o CF ou apagar arquivos.
3. **Nunca** usar um cartão marcado como inicializável sem um \`AUTOEXEC.BIN\` válido. Isso pode fazer a câmera travar ao ligar.
4. Se um futuro teste experimental congelar a câmera: parar imediatamente, desligar, remover bateria e cartão antes de tentar reiniciar sem o CF experimental; não repetir tentativas em sequência. Verificar a inicialização Canon e só depois investigar.
5. Não instalar em uma câmera de uso indispensável sem aceitar o risco de falha que pode exigir reparo especializado.

Fontes: [instruções e alertas do Soup Engine para a 5D](https://github.com/jiijii0/soup-engine/tree/master/Canon-EOS-5D) e [alertas históricos do Magic Lantern](https://magiclantern.fandom.com/wiki/Unified/Install).

## Registro da sessão física futura

| Campo | Valor |
| --- | --- |
| Corpo | Canon EOS 5D Classic (2005) |
| Firmware | **1.1.1 — confirmado pelo operador** |
| Testes JPEG e RAW | **Passaram — informado pelo operador** |
| CF de teste (capacidade/formato) | **Pendente** |
| Fotos salvas em backup fora do cartão | **A reconfirmar** |
| Build/commit do loader ML | **Pendente** |
| SHA-256 do AUTOEXEC do Magic Lantern original | **Pendente** |
| SHA-256 do AUTOEXEC do Soup efetivamente usado | **Pendente** |
| SHA-256 do SE.BIN Rebobina candidato | **Pendente** |
| Teste de inicialização experimental | **NÃO REALIZADO** |
| Recuperação testada | **NÃO REALIZADA** |

**Critério de parada:** qualquer falha de checksum, origem incerta, arquivos de modelo/firmware diferente, comportamento anormal da câmera ou ausência de plano de recuperação. O avanço ao teste físico exige revisão explícita desses bloqueadores; esta documentação **não é autorização automática para executar o payload**.
