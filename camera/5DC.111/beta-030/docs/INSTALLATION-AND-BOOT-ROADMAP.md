# Instalação e substituição de Magic Lantern: plano de execução

## Evidência física

O desenvolvedor confirmou que a versão 0.2.0 entrou no LCD da Canon EOS 5D Classic e recebeu comandos. Os relatórios RB8BOOT e RB8TEST enviados comprovam carregamento, teste de memória (RAM_TEST_HEX=1), teste CF (CF_TEST_HEX=1) e interface habilitada (UI_MARKER_HEX=1). O antigo menu Magic Lantern ainda apareceu.

## Por que os arquivos do Magic Lantern ainda são necessários

O carregador utilizado é derivado do Magic Lantern de 2012 e foi adaptado pelo Soup Engine. No mlstub.S do Soup, a rotina se_boot chama ML_INIT_TASK, depois se_loader. O carregamento procura A:/SE.BIN e usa um hook da GUI para oferecer eventos ao Rebobina. As rotinas bmp_fill/bmp_printf da própria imagem Magic Lantern seguem sendo usadas para desenhar o menu.

Portanto **apagar ML/ ou AUTOEXEC.BIN não remove apenas um menu: pode impedir a inicialização** em um CF de boot. Não faça isso. A 0.3 passa a consumir LIXEIRA mesmo quando o LCD ainda não está pronto, para evitar que esse evento abra o menu antigo. Essa alteração não prova que todo o menu ML foi removido.

## Modo compatível: instalador agora disponível

Em um CF que já foi testado com o Soup/REBOBINA, a ferramenta tools/install-rebobina.ps1 faz preflight estrutural do loader (size, assinaturas, hooks), confere SHA-256 do novo SE.BIN, cria staging e backup no PC. Sem -Apply, o cartão não é modificado. Com -Apply, exige digitar INSTALAR. **Só escreve SE.BIN e ML/RB9BETA.ON** e verifica o hash no CF. O modo restaurar usa o backup e exige RESTAURAR. Não altera AUTOEXEC.BIN, bootflags, ROM, FAT nem arquivos de fotografias.

Windows (câmera desligada, CF em D:):

    powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\install-rebobina.ps1 -CardRoot D:\
    powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\install-rebobina.ps1 -CardRoot D:\ -Apply
    powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\restore-rebobina.ps1 -CardRoot D:\ -BackupDir C:\BackupGuardado -Apply

Também há os atalhos INSTALAR-REBOBINA.cmd e RESTAURAR-REBOBINA.cmd. Ambos pedem os caminhos; a atualização exige a confirmação.

Este instalador é uma **atualização compatível** para a câmera de desenvolvimento, não um instalador de firmware em um CF limpo. Falha por padrão se AUTOEXEC.BIN/SE.BIN/Magic Lantern compatível não estiverem presentes.

## Para substituir o Magic Lantern na cadeia de boot

1. Implementar um AUTOEXEC.BIN próprio com entrada ARM, memória/stack/cache e VxWorks seguros para 5DC.111, a partir de origem reproduzível.
2. Provar handoff ao firmware Canon quando o programa estiver ausente, inválido ou falhar. O ELF stage0 existente no projeto ainda é offline e não tem handoff real.
3. Substituir o renderizador BMP e a fonte dependentes da imagem ML, usando framebuffer e verificação de disponibilidade sem interferir na GUI Canon.
4. Comprovar recuperação de cartão/bootflags e proteger usuários contra incompatibilidade de modelo/firmware.
5. Só então permitir instalação em cartão novo, com possibilidade de restaurar o original e distribuição própria.

Há base técnica e testes físicos para o payload REBOBINA, mas **não** para chamar a cadeia de boot de independente. O código do Magic Lantern/Soup é licenciado em GPLv2, com atribuições obrigatórias.

Referências: https://github.com/jiijii0/soup-engine/tree/master/Canon-EOS-5D e pesquisa do projeto em docs/5DC-BOOT-CHAIN.md e docs/5DC-BOOTSTRAP-OFFLINE.md.
