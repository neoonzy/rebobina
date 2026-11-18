# REBOBINA 0.2.0-beta — Canon EOS 5D Classic (firmware 1.1.1)

**Primeiro beta interativo para a câmera.** Desenvolvido após os boots reais 0.0.1, 0.0.3, 0.0.4 e os testes 0.1.0–0.1.2, que confirmaram escrita em CF, memória, botões e texto/fundo preenchendo todo o LCD. A versão **0.2.0-beta ainda não foi executada na câmera**, apesar de compilar e passar nos testes host. Programe o uso no **CF de desenvolvimento**, nunca num cartão com fotos sem backup.

> O REBOBINA continua sendo executado por meio do **carregador legado Soup Engine / Magic Lantern** (`AUTOEXEC.BIN` previamente funcional). Este pacote **não instala firmware Canon, não edita bootflags e não inclui carregador independente**. Não substitua o `AUTOEXEC.BIN`.

## O que é funcional na câmera nesta versão (implementado, hardware 0.2.0 ainda pendente)

- **Interface própria permanente:** deixa de encerrar após 40 segundos; desenha a **até 10 vezes por segundo** com `bmp_fill` e `bmp_printf` no LCD já ligado. São 720×480 unidades lógicas do VxWorks/ML (VRAM 360×240 bytes, 4 bits/pixel). O menu não é um framebuffer independente; a Canon/ML ainda pode redesenhar e causar flicker.
- **Navegação:** roda, joystick UP/DOWN e SET; MENU volta uma página ou fecha o menu quando já está no início; LIXEIRA alterna exibição. Quando a interface está ativa, apenas esses controles são consumidos. O disparador, playback e teclas desconhecidas continuam com o tratamento Canon. Para abrir o LCD pela primeira vez, use MENU Canon; ele não é ligado por chamadas diretas de ROM.
- **FILM:** quatro perfis selecionáveis (`SOUP 800`, `PORTRA`, `ACROS`, `CINE` — nomeado como Eterna no fluxo host). O perfil escolhido **é realmente salvo e restaurado**, mas **não altera as fotos na câmera**: a integração da revelação Soup com o runtime embarcado ainda não existe.
- **ROLL:** caderno manual de rolos (`novo rolo`, marcar foto, desfazer, zerar contador, salvar); o contador mede as entradas feitas no menu, **não fotografa nem detecta automaticamente o obturador**.
- **FERRAMENTAS:** leitura de RAM e VxWorks, teste de alocação, gravação/leitura do CompactFlash, salvamento manual, exportação de sessão de diagnóstico.
- **MÓDULOS:** alterna os serviços compilados `Caderno de rolos` e `Diagnósticos`, cada um com consequência real. Mostra `Soup` e `CR2` como **host-only**, sem botão que prometa revelar imagens na câmera. Não é um carregador de módulos binários `.mo`.
- **PREFERÊNCIAS:** salvamento automático, diagnóstico detalhado, abertura inicial, salvar e restaurar valores padrão.
- **Persistência no CF:** configurações em `ML/RB8CFG0.BIN` e `ML/RB8CFG1.BIN` com versão, contador de sequência e checksum FNV-1a. O programa seleciona a cópia válida mais recente, preservando uma cópia anterior em cada gravação. A gravação é somente nos arquivos com prefixo RB8. Caso o hardware não permita recriar um slot, a interface mostra erro e **não** marca alterações como salvas.

### O que ainda **não** é funcional na câmera

Não há desenvolvimento automático de JPEG/CR2 no próprio corpo, monitoramento de disparos, mudanças de ISO/exposição/Picture Style, acesso ao pipeline de captura, instalação de módulos externos, gravação de firmware, nem menu em framebuffer independente da Canon. Tudo isso exige port real e validação separada. A seleção FILM controla **configuração**, não a aparência de imagens capturadas. Na **etapa opcional em Linux**, a seleção pode ser consumida pelo desenvolvedor Soup já existente no repositório para gerar um JPEG em **outra pasta no computador**.

## Instalação de teste (Windows, CF como D:)

1. Extraia o ZIP em uma pasta própria. Faça backup de fotos e do cartão de desenvolvimento. Ligue a câmera somente com bateria carregada.
2. Conecte ao computador o CF em que `AUTOEXEC.BIN` do Soup já é comprovadamente funcional. Na pasta extraída, abra PowerShell:

   ```powershell
   powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\prepare-test.ps1 -CardRoot D:\
   ```

   O script **NÃO altera o CF**. Ele valida o carregador pré-existente, confere o SHA-256 do novo binário e prepara a pasta `Downloads\REBOBINA-020-BETA\`, com `BACKUP\` e `COPIAR-PARA-CF\`. Se a pasta já existir, ele aborta para preservar o backup.
3. Com a câmera **desligada**, copie **somente o conteúdo** de `COPIAR-PARA-CF\` para a raiz do CF. O conteúdo é `SE.BIN` e `ML/RB8BETA.ON`. **Nunca substitua `AUTOEXEC.BIN`.** Marcadores RB5/RB6/RB7 antigos são ignorados pelo novo payload.
4. Ejete o cartão com segurança, coloque na 5D Classic, ligue, abra MENU Canon se o LCD estiver apagado. O REBOBINA deve ocupar a tela. Use a roda/SET para navegar e MENU/LIXEIRA para fechar ou retornar. Não use o cartão de teste em trabalho fotográfico irrepetível.
5. Selecione `FILM` / `PORTRA` e confirme com SET. Vá em `ROLL`, marque duas fotos manualmente, vá em `FERRAMENTAS` e escolha **SALVAR PREFERÊNCIAS** e **EXPORTAR RELATORIO**; os arquivos no CF devem aparecer. Desligue e ligue de novo e verifique se o perfil e rolo persistiram.

Os marcadores da 0.2.0 são independentes dos das versões anteriores. Para desativar a interface **sem trocar o binário**, remova `ML/RB8BETA.ON` com a câmera desligada; o beta ficará em modo headless. Para restaurar a 0.1.2 ou anterior, copie o backup do `SE.BIN` salvo pelo script (renomeie para `SE.BIN` no CF).

Para preparar explicitamente o modo headless (somente diagnóstico), remova primeiro o marcador beta existente e rode:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\prepare-test.ps1 -CardRoot D:\ -Headless
```

## Arquivos gerados / leitura

- `RB8BOOT.TXT`: boot e criação de serviço, teste RAM/CF, marcador da interface.
- `RB8TEST.TXT`: arquivo exclusivo para teste gravar-ler-comparar (a leitura bem-sucedida é reportada via `CF_TEST_HEX=1` em `RB8BOOT.TXT`).
- `RB8SESSION.TXT`: criado quando você escolhe **EXPORTAR RELATORIO** ou SALVAR CADERNO; inclui configurações, eventos, FPS contado e memória.
- `ML/RB8CFG0.BIN` / `ML/RB8CFG1.BIN`: configurações alternadas de 36 bytes, criadas após salvar/auto-save; ambos os arquivos são do REBOBINA.

No PC, leia sem alterar nada:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\read-results.ps1 -CardRoot D:\
```

```bash
python3 tools/read-results.py /media/CANON_CF
python3 tools/config_reader.py /media/CANON_CF
```

**Atenção:** `RB8SESSION.TXT` não é gerado automaticamente após 40 segundos porque o beta agora continua rodando. Use a opção de exportação no menu. Após SALVAR, deixe a câmera ligada por alguns segundos antes de desligar, permitindo escoar as escritas em CF.

## Revelar foto selecionada no Linux (HOST, opcional)

O programa `tools/develop_selected.py` lê o perfil que a 5D salvou e invoca o executável host `rebobina-cr2-develop` já desenvolvido no repositório, com o arquivo `.fs` original do Soup. **Não é processamento no corpo Canon**. Não altera CR2 nem sobrescreve JPEGs. É necessário compilar antes os utilitários host no repositório e ter os `.fs` do Soup Engine em uma pasta local.

```bash
python3 tools/develop_selected.py \
  --card /media/CANON_CF \
  --cr2 /home/usuario/fotos/IMG_1234.CR2 \
  --out /home/usuario/rebobina-output \
  --developer /caminho/rebobina/build/rebobina-cr2-develop \
  --stocks /caminho/soup-engine/Canon-EOS-5D/src/stocks
```

O processamento original no host usa o JPEG embutido no CR2, não o Bayer RAW. A compatibilidade com CR2 reais desta câmera ainda exige validação. Referências: `docs/CR2-DESENVOLVIMENTO-HOST.md` no repositório principal e `tools/soup_host.py`.

## Build e testes (Linux/WSL)

```bash
make clean all test
```

Ferramentas: `clang`, `lld`, `llvm-objcopy`, `readelf`, `make`, Python 3.9+. Gera `dist/SE.BIN` e `dist/manifest.json`. Tests C usam mock de HAL e testam navegação, saída gráfica a 10 passos, reentrada, configuração, slots corrompidos, falhas de CF e ausência de display; testes Python cobrem o formato, checksum, adaptação host, branch do loader e arquivo `.bss`. Não substituem hardware.

O firmware está abaixo do limite de 128 KiB e preserva o contrato `FILM` +4 (boot) +0x44 (GUI) do carregador. É experimental e não é uma atualização de ROM.

## Créditos, licença e responsabilidade

REBOBINA: GPL-2.0-only. **Magic Lantern Team**: interface BMP, hook GUI, pesquisa da plataforma 5DC. **jiijii0 / Soup Engine**: carregador da 5D e pesquisa VxWorks/RAW/JPEG; a seleção de perfis usa nomes do universo Soup, mas não inclui o processamento fotográfico no binário. Créditos upstream: https://github.com/jiijii0/soup-engine e https://github.com/frantony/magic-lantern. Nenhuma associação com a Canon. **Sem garantia**. 
