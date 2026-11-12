# Canon EOS 5D Classic — protótipo de toolchain

A pasta `camera/5DC.111/` **não contém firmware instalável**. Ela inclui agora uma seleção de símbolos e mapeamento de eventos do Magic Lantern histórico em `ml_compat/` para **testes somente offline**. `probe.c` é um arquivo compilado isoladamente para arquitetura **ARM946E-S / ARMv5TE**. Ele não tem ponto de entrada de boot, chamadas a ROM Canon, linker de câmera, mapeamento validado de memória nem saída `AUTOEXEC.BIN`.

## Testes seguros

```sh
make arm-objects
```

O alvo produz somente `build/arm/*.o` (objetos ELF **relocáveis**, não executáveis). O comando usa Clang com `--target=arm-none-eabi -mcpu=arm946e-s`; precisa de `readelf` para verificar o tipo.

**Não copie esses arquivos para cartões da câmera.** O objetivo é detectar problemas de compatibilidade do núcleo C com compilação freestanding ARM antes de criar um bootloader.

Para a pesquisa de boot e memória, consulte [o levantamento da 5D](../../docs/5DC-PLATAFORMA.md).

## Reuso do Magic Lantern

Em `ml_compat/` temos 10 aliases da ROM e o decodificador de 7 códigos de botões provenientes do port 5DC.111 histórico de 2013, com arquivo de proveniência, créditos e testes. Leia [origens, licença e limites](../../docs/ML-5DC-REUSO.md). **Nenhum artefato gera ou instala boot para a Canon**.
