# Soup Engine: UBSan, ASan e correção experimental de shift

**Data:** 08/11/2026  
**Estado:** achado concreto de UB no upstream + teste de equivalência no computador. **Não homologado para a EOS 5D.**

## Descoberta

Ao compilar a revisão fixada `jiijii0/soup-engine@6422c86b4530a5bdb4c5619f5287044ae7c661e3` com Clang `-fsanitize=address,undefined -fno-sanitize-recover=all`, o teste Soup 800 acusou:

```text
engine/jpeg_enc.c:353:32: runtime error: left shift of negative value -639
```

No `fdct` do encoder JPEG (versão IJG integer slow), as atribuições originais

```c
p[0] = (tmp10 + tmp11) << PASS1_BITS;
p[4] = (tmp10 - tmp11) << PASS1_BITS;
```

podem deslocar **valores negativos** à esquerda, operação com comportamento indefinido no padrão C. Esse achado é distinto do aviso GCC `-Wstringop-overflow` em `engine/look.c`, que ainda precisa de investigação mais profunda.

## Abordagem adotada

A função `sanitizer_encoder_copy` em `tools/soup_host.py` lê o código original, confere exatamente os dois trechos esperados e gera **apenas no diretório temporário `build/soup-sanitizer-compat/`** uma cópia para o build sanitizado:

```c
p[0] = (tmp10 + tmp11) * (1 << PASS1_BITS);
p[4] = (tmp10 - tmp11) * (1 << PASS1_BITS);
```

Para os intervalos do fixture, essa substituição elimina o shift negativo e mantém o valor numérico. **O motor upstream permanece intacto**, inclusive na compilação de referência sem sanitizador e na fonte remota. A mudança não é uma revisão geral de overflow aritmético, nem foi integrada ao firmware Canon.

## Testes e evidências

A pipeline compila duas variantes: referência upstream com GCC (sem patch) e cópia local com o encoder ajustado sob **Clang ASan + UBSan**. Ambas recebem os mesmos 8×8 pixels sintéticos, e a saída JPEG é escrita **somente em arquivos temporários exclusivos**, com comparação byte a byte por Python.

| Perfil original | Tamanho JPEG | Comparação |
| --- | ---: | --- |
| `soup800.fs` | 1006 bytes | Idêntico |
| `portra.fs` | 1010 bytes | Idêntico |
| `acros.fs` | 1000 bytes | Idêntico |

[Execução CI verificada](https://github.com/neoonzy/rebobina/actions/runs/42550242368) — sanitizadores passaram nesses três casos, saídas JPEG completas idênticas e teste de CR2 sintético aprovado.

**A equivalência demonstrada é limitada aos fixtures e compiladores testados.** O build original usa uma expressão de comportamento indefinido, portanto resultados iguais nesses três casos não autorizam assumir equivalência geral. Não foram testadas imagens reais, todos os tamanhos, níveis de compressão ou ambientes embarcados.

O aviso de `engine/look.c` sobre possível tamanho inválido em `memcpy` **continua visível no build GCC**, não foi descartado nem corrigido neste trabalho. É possível que o compilador não conheça os limites internos do run de halation, mas **não podemos afirmar que seja falso positivo sem revisão de invariantes e testes maiores**.

## Reproduzir

```sh
python3 tools/soup_host.py --fetch
```

O comando baixa a revisão de origem fixada, compila os componentes, executa os 13 selftests, os testes sintéticos da HAL e as variantes normal/sanitizada. Requer Linux, Git, Python, Make, CC e Clang. Não acessa nem modifica a câmera.

## Pendências

- [x] Reproduzir o UB de deslocamento negativo no codificador original.
- [x] Gerar ajuste **somente para testes** numa cópia temporária.
- [x] Comparar arquivos JPEG completos de três perfis e rodar ASan/UBSan.
- [ ] Revisar o aviso da halation, os limites reais dos buffers e outros pontos de UB.
- [ ] Fazer testes com fotografias reais autorizadas e dados CR2 truncados.
- [ ] Decidir como incorporar correção ao módulo Rebobina, mantendo licença GPLv2 e atribuições.
- [ ] Validar com processador ARM946E-S e câmera real antes de distribuir qualquer firmware.
