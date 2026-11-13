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

## Bootstrap independente — protótipo de ligação offline

O novo diretório [`bootstrap/`](bootstrap/) contém entrada ARM própria, modelo de inicialização freestanding, contrato estrutural e linker de pesquisa, **sem necessitar do Magic Lantern como dependência de build**. O alvo `make offline-bootstrap` gera apenas um `build/arm/5dc-stage0-offline.elf` que **não é carregável pela Canon**, pois não implementa formato de boot, handoff da Canon nem recuperação. Ele termina em um loop deliberado. **Não converter, renomear ou instalar na câmera.** Detalhes e créditos em [`docs/5DC-BOOTSTRAP-OFFLINE.md`](../../docs/5DC-BOOTSTRAP-OFFLINE.md).

## Pesquisa de carregador e recuperação — 13/11/2026

O diretório `bootstrap/` agora contém `boot_chain.[ch]` com a distinção de endereços históricos entre o instalador do Magic Lantern 2013 e a imagem realocada, classificação conservadora das flags e pré-boot com falha por padrão. `boot_handoff_mock.[ch]` exercita em **host** a continuação simulada da Canon após falhas, sem chamar a ROM real. Fonte e riscos estão detalhados em [`docs/5DC-BOOT-CHAIN.md`](../../docs/5DC-BOOT-CHAIN.md). A câmera **não está pronta para um boot experimental**.
