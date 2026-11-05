# Rebobina — Boot Probe 0.0.1 para Canon EOS 5D Classic

> [!CAUTION]
> **CANDIDATO EXPERIMENTAL, NÃO VALIDADO EM CÂMERA FÍSICA.** Não é firmware para uso cotidiano, não fornece menu e não substitui o Magic Lantern de maneira independente. O uso pode congelar a câmera ou comprometer o cartão. Utilize apenas equipamento/cartão de teste, com backup e recuperação.

## O que foi implementado

- Payload ARM946E-S de código próprio Rebobina, carregável pelo **stub da 5D Classic usado pelo Soup Engine**; arquivo de saída `SE.BIN`.
- Inicialização em `+0x04` e hook de eventos em `+0x44`, com a assinatura `FILM` exigida pelo loader histórico.
- Tenta criar o arquivo **`A:/REBOBINA.TXT`** quando a rotina é chamada, como diagnóstico de execução.
- Todos os botões passam para a GUI Canon/Magic Lantern; não há menu Rebobina, captura ou processamento Soup Engine.
- Não altera a ROM Canon, boot flag ou `AUTOEXEC.BIN`; **depende** do executável do Soup Engine específico da 5D Classic.

## Pré-requisitos

1. **Canon EOS 5D Classic (2005) com firmware exatamente 1.1.1.** Não funciona na 5D Mark II/III/IV.
2. Magic Lantern específico da 5D Classic **já instalado e comprovadamente inicializando** num CompactFlash de teste, sem fotos importantes. Este projeto **não** instala boot flags.
3. Backup completo do cartão, especialmente do `AUTOEXEC.BIN` do Magic Lantern, guardado fora do cartão.
4. Download confiável e extração do pacote específico **Canon-EOS-5D** do [Soup Engine original](https://github.com/jiijii0/soup-engine/releases), com `card/AUTOEXEC.BIN`, `card/SE.BIN` e `card/ML/`.
5. Bateria carregada, leitor CF e plano de recuperação.

**Não use o payload se você ainda não tem o Magic Lantern da 5D funcionando.** Não use a versão de outra câmera.

## Pasta de preparação segura (sem gravar no CompactFlash)

Depois de obter o `SE.BIN` e o manifesto do próprio build (ou do ZIP experimental), rode na pasta deste projeto:

```sh
python3 tools/stage_card.py --soup-card "/caminho/Canon-EOS-5D/card"
```

O script cria **somente** `dist/card-staging/` e `dist/staging-manifest.json` no computador. A pasta de origem permanece intacta; nada é copiado diretamente para o cartão. O script rejeita tamanho, assinatura e hooks incompatíveis do `AUTOEXEC.BIN`, mas **não autentica a procedência histórica do binário por hash confiável** (essa referência ainda não foi localizada).

**Se a verificação recusar o `AUTOEXEC.BIN`, interrompa o teste. Não contorne o bloqueio.**

## Teste físico — apenas quando houver condições de recuperação

1. Confirme novamente **Canon 5D Classic firmware 1.1.1** e um CompactFlash de teste **já inicializável**. Guarde o original `AUTOEXEC.BIN` Magic Lantern e o `SE.BIN` do Soup fora do cartão.
2. Com a câmera desligada, depois de o staging passar, copie o conteúdo de `dist/card-staging/` para o CF de teste. Não apague o `AUTOEXEC.BIN` do cartão inicializável.
3. Ligue e observe. **Se houver travamento ou `Err`, desligue, retire a bateria e o CF e restaure a cópia original.** Não fotografe enquanto testa a inicialização.
4. Desligue normalmente, remova o CF e verifique no computador se existe `REBOBINA.TXT` na raiz. Se contiver `REBOBINA BOOT PROBE 0.0.1`, o código Rebobina executou ao menos a rotina de gravação; isso **não prova estabilidade**.
5. Para restaurar o Magic Lantern, recopie seu `AUTOEXEC.BIN` original; para restaurar o Soup Engine, recopie o `SE.BIN` original do pacote Soup.

> [!WARNING]
> Com o boot flag ativo e o cartão marcado como inicializável, **não remova `AUTOEXEC.BIN` sem restaurar um executável válido**. A câmera pode não iniciar até retirar cartão/bateria.

## Compilar e testar offline

Linux com Clang, LLD, `llvm-objcopy`, `readelf`, Python 3.9+:

```sh
make clean all test
```

O processo gera `dist/SE.BIN` e `dist/probe-manifest.json`. Um build de referência produziu 504 bytes e SHA-256 `f50b41ef6d70be894117277a8f7c1984e571ddc2a10ff471b0371805e81a8834`. Foram aprovados **10 testes Python locais** com arquivos sintéticos. Isso **não** testa o bootloader, VxWorks, chamadas da ROM ou hardware.

Este protótipo é um **marco de integração experimental**, não o Rebobina final. O próximo passo é validar o boot em câmera de desenvolvimento e então implementar interface e módulos com recursos do próprio Rebobina.

## Licença e atribuição

Código original sob [GPL-2.0-only](../../../LICENSE). Protocolo de carga, endereços e ABI estudados no [Soup Engine 5D Classic](https://github.com/jiijii0/soup-engine), de jiijii0 (GPLv2), construído sobre o Magic Lantern. **O código-fonte e os binários de terceiros não são distribuídos por este diretório**. O `AUTOEXEC.BIN` é obtido separadamente no projeto de origem. Sem qualquer garantia.
