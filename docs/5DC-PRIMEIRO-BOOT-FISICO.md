# Primeiro boot físico do REBOBINA na Canon EOS 5D Classic

**Data:** 16 de novembro de 2026 (horário de Recife). **Operador/desenvolvedor:** autor do projeto.

## Resultado confirmado em câmera

O **REBOBINA Boot Probe 0.0.1** executou na EOS 5D Classic com firmware Canon 1.1.1 e gravou o arquivo `A:/REBOBINA.TXT` no cartão CompactFlash. A captura do Windows mostrou 147 bytes e o texto:

```text
REBOBINA BOOT PROBE 0.0.1
EOS 5D Classic 1.1.1 / VxWorks
Executado por loader legado Soup Engine.
SEM interface e SEM garantia de compatibilidade.
```

O desenvolvedor confirmou diretamente que o arquivo foi **criado pela câmera**, e não copiado no computador. Esse fato encerra o critério estreito de **primeira execução de código REBOBINA via carregador Soup e gravação observável no CF**.

## Escopo do marco e limites

- `SE.BIN` REBOBINA executado pelo `AUTOEXEC.BIN` legado Soup Engine / Magic Lantern, não por carregador independente.
- Executou e gravou arquivo próprio; ainda não valida interface física, eventos, estabilidade, tarefas adicionais, RAW ou gerenciamento modular.
- Não houve alegação de troca do firmware Canon ou de instalação autônoma do REBOBINA.
- Não há hash do binário 0.0.1 preservado neste relato; não inventar uma referência.
- Versão seguinte de diagnóstico integrado: `camera/5DC.111/diagnostic-003/` — build candidato **ainda sem validação em câmera**.

**Créditos:** Soup Engine (jiijii0) e Magic Lantern Team, ambos reconhecidos como pesquisa GPLv2 usada no carregamento experimental.
