# Primeiro teste físico do Rebobina — 5D Classic / firmware Canon 1.1.1

**Status:** candidato experimental da PR #1, ainda SEM validação física. Esta é a via *mais curta de desenvolvimento* aproveitando um loader conhecido, **não a arquitetura final independente**.

## O que vamos demonstrar

Executar código Rebobina no corpo da câmera **uma vez** e tentar gravar `A:/REBOBINA.TXT`. Se o arquivo existir e contiver `REBOBINA BOOT PROBE 0.0.1`, avançamos do simulador para a primeira evidência real de execução ARM. A ausência dele não prova que nada executou: o boot ou a escrita podem falhar.

## Você já concluiu

- Firmware Canon 1.1.1 atualizado e fotografando JPEG/CR2 normalmente.
- Dois cartões CF de 16 GB e backup das fotos.
- Pacote Soup `Canon-EOS-5D.zip` SHA-256 `0f8cc374374d6692ec77836cd979ee75d2c77ae8a7a5bdd8da1cd5e6d0c9c993` — verificado pelo operador.
- Pacote Soup com `AUTOEXEC.BIN` de 221184 bytes e `SE.BIN` de 70732 bytes — verificado pelo operador.

## O que efetivamente falta — uma única ativação de boot, temporária para desenvolvimento

O firmware original da Canon não procura `AUTOEXEC.BIN` por padrão. Para a **primeira prova física** usaremos o instalador histórico **exato** para `5DC.111` e o loader Magic Lantern modificado do Soup, sem instalar um menu Magic Lantern como produto. Isso **ativa uma boot flag persistente** e exige um CF marcado como inicializável. O loader ML é uma dependência **temporária do laboratório**; o objetivo do Rebobina continua ser criar um boot próprio com um instalador de marca própria e créditos GPLv2.

**Riscos reais:** um instalador da versão errada ou defeituoso pode inutilizar a câmera. Com boot flag ativo, um CF inicializável sem `AUTOEXEC.BIN` funcional pode travar o boot. **Remover o CF geralmente permite voltar à inicialização padrão, mas isso NÃO garante a recuperação de toda alteração persistente.** Use bateria carregada e não prossiga se não aceitar possibilidade de dano.

## Roteiro direto — Windows

1. Guarde o **CF A** intacto, com fotos normais. O **CF B** será o de teste; fotos já copiadas. Se tiver mídia importante adicional, copie-a antes de formatar.
2. Baixe o Magic Lantern **para EOS 5D Classic, firmware 1.1.1 (`5DC.111`)**, da [página indicada pelo próprio Soup Engine](https://builds.magiclantern.fm/5DC.html). **Não use arquivo de 5D2/5D3 ou um instalador cuja identificação seja incerta.** Antes de qualquer `.FIR`, verifique que o pacote realmente existe e corresponde a `5DC.111`. O projeto Rebobina **ainda não autenticou externamente o instalador disponibilizado nesse site**.
3. Siga as instruções do pacote específico para preparar o CF B e ativar o modo de boot. **Essa é a etapa de maior risco** porque modifica flags persistentes. Não use o instalador de 2013 encontrado no código-fonte como substituto: sua rotina alterna a flag e seu binário não foi validado neste hardware.
4. **Primeiro controle físico:** depois da ativação, inicie o Magic Lantern original nesse CF e verifique que o corpo e menu normal respondem. Guarde o `AUTOEXEC.BIN` original do Magic Lantern no PC.
5. Baixe o artefato `rebobina-5dc-boot-probe-NAO-HOMOLOGADO` da [execução GitHub Actions da PR #1](https://github.com/neoonzy/rebobina/actions/workflows/5dc-boot-probe.yml). Pegue os arquivos `SE.BIN` e `probe-manifest.json` do ZIP. O `SE.BIN` é **código Rebobina**, não o `SE.BIN` Soup.
6. No PowerShell, a partir do diretório raiz do clone da branch `experimental/5dc-boot-probe`, execute:

   ```powershell
   powershell -ExecutionPolicy Bypass -File camera/5DC.111/boot-probe/tools/stage_windows.ps1 -SoupZip "$env:USERPROFILE\Downloads\Canon-EOS-5D.zip" -RebobinaZip "$env:USERPROFILE\Downloads\rebobina-5dc-boot-probe-NAO-HOMOLOGADO.zip"
   ```

   O script cria `Downloads/Rebobina-5DC-TESTE` e `Downloads/Rebobina-5DC-ROLLBACK`, sem alterar o cartão. Verifica o hash completo do Soup, hooks do `AUTOEXEC.BIN`, a assinatura `FILM` e o SHA-256 do payload Rebobina. Se recusar o pacote, **pare**.
7. Com câmera **desligada**, copie o conteúdo da pasta `Rebobina-5DC-TESTE` para a **raiz do CF B já inicializável**, substituindo o `AUTOEXEC.BIN` e o `SE.BIN`. Mantenha o backup **do ML e do Soup no PC**, não no CF. Ejete com segurança.
8. Ligue sem fotografar e observe se inicia e responde. Desligue, retire o CF B e procure na raiz dele `REBOBINA.TXT`; abra e confira o texto. Não tente novamente em loop se não funcionar.
9. Se houver travamento ou comportamento inesperado, desligue, retire bateria e o CF de teste, recoloque a bateria e teste com o CF A original. Só volte ao CF B após restaurar um `AUTOEXEC.BIN` funcional, e nunca mantenha um CF inicializável sem esse arquivo.

## Como interpretar o resultado

| Observação | Conclusão estrita |
| --- | --- |
| `REBOBINA.TXT` criado com assinatura correta | A rotina de diagnóstico Rebobina foi executada no hardware; **não comprova interface nem estabilidade** |
| Corpo liga mas arquivo não aparece | Loader/rotina de escrita não demonstrados; investigar SEM concluir que o programa executou |
| Câmera trava com CF B | Interromper, retirar bateria/cartão e recuperar o cartão; não continuar rodando experimentos |

## Retorno

Para restaurar **Soup**, substitua apenas `SE.BIN` pelo original guardado. Para retornar ao **Magic Lantern**, restaure seu `AUTOEXEC.BIN` original na raiz do CF e os arquivos exigidos pelo pacote. Para usar a Canon sem extensão, use o CF A sem inicialização experimental. Isso **não apaga automaticamente a boot flag**; removê-la requer método separado, testado para a 5DC.111.

**Créditos:** Magic Lantern Team (GPLv2), Soup Engine de jiijii0 (GPLv2), código da prova Rebobina GPL-2.0-only. As verificações dos scripts não provam a segurança do instalador nem a recuperação de falhas persistentes.
