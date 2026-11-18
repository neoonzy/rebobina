# REBOBINA 0.3.0-beta — experiência REBOBINA e migração assistida

**Canon EOS 5D Classic, firmware 1.1.1.** O usuário confirmou que a versão 0.2 já executa no hardware, com LCD e navegação. A nova versão prioriza o REBOBINA: a LIXEIRA não é mais repassada ao menu antigo quando a tela ainda está apagada. A versão 0.3.0 ainda precisa de teste físico.

## Funcional agora (firmware no CF previamente habilitado)

- Interface de 720x480 coordenadas VxWorks, até 10 quadros/s, botão LIXEIRA para mostrar/ocultar, roda e SET para navegação, MENU para voltar.
- 13 perfis de filme no menu em lista rolável; seleção salva para uso no revelador no computador. **Nenhum filtro muda fotos na câmera nesta versão.**
- Caderno de rolos manual, diagnósticos de RAM e CF, módulos internos simples e preferências com salvamento em dois slots `ML/RB9CFG0.BIN` e `ML/RB9CFG1.BIN`; na primeira execução importa, sem alterar, os antigos arquivos `ML/RB8CFG0.BIN` e `ML/RB8CFG1.BIN`.

## Instalação (Windows) no cartão que já funcionava

Com câmera desligada, bateria carregada, backups de fotos e CF em D:\, abra PowerShell na pasta extraída:

```powershell
# Conferir, preparar e criar backup NO COMPUTADOR; não altera o CF
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\install-rebobina.ps1 -CardRoot D:\

# Após conferir o backup, instalação assistida (pede digitar INSTALAR)
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\install-rebobina.ps1 -CardRoot D:\ -Apply
```

O assistente exige `AUTOEXEC.BIN` já funcionando e com estrutura patchada do Soup, verifica manifesto e SHA-256, cria backup, e **somente** instala `SE.BIN` mais `ML/RB9BETA.ON`. Ele NÃO mexe em bootflags, firmware, `AUTOEXEC.BIN`, fotos ou arquivos genéricos do ML. O carregador e os arquivos necessários do ML devem permanecer. Se o cartão não tiver a base compatível, o instalador **recusa** a instalação: ainda não temos bootloader independente seguro ou instalador para cartão virgem.

Para restaurar versão anterior com a câmera desligada:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\restore-rebobina.ps1 -CardRoot D:\ -BackupDir "C:\caminho\BACKUP" -Apply
```

### Estado do projeto

REBOBINA já tem runtime, UI, configurações e serviços próprios, mas **a cadeia de boot ainda executa o ML/Soup**. Não remova `AUTOEXEC.BIN` nem a pasta `ML`. A etapa seguinte para substituição integral requer um `AUTOEXEC.BIN` independente com inicialização VxWorks e handoff real para o Canon, além de procedimento verificado de recuperação. O protótipo próprio em `camera/5DC.111/bootstrap` é offline e não instalável.

Os filtros CR2 (13 perfis) podem ser executados **no computador** com `tools/develop_selected.py` e a ferramenta host do repositório, preservando originais. Não funcionam na Canon ainda.

Consulte `docs/INSTALLATION-AND-BOOT-ROADMAP.md`. Créditos GPLv2 ao Magic Lantern Team e jiijii0/Soup Engine. Software experimental e sem garantia.

## Migracao de configuracoes sem perder rollback

O firmware 0.3 lê os slots RB9 em primeiro lugar. Se não existirem válidos, importa os slots RB8 da 0.2, mas salva sempre em RB9. Assim, ao restaurar o SE.BIN da 0.2, as preferências anteriores continuam disponíveis. As escolhas novas da 0.3 não se propagam para a 0.2. O instalador guarda cópias de ambos os pares no computador.
