# ADR 0003 — HAL portável e falhas determinísticas

- **Data:** 06/11/2026
- **Estado:** aceita para protótipos host; backend físico ainda não decidido
- **Código:** `include/rebobina/hal.h`, `include/rebobina/hal_sim.h`, `src/host/hal_sim.c`.

## Contexto

A Canon EOS 5D Classic do usuário ainda estava em firmware 1.1.0 e sem leitor CF para testar atualização e inicialização em 1.1.1. Os endereços de ROM descritos pelo Soup Engine **não foram verificados pelo Rebobina em hardware**. Precisamos continuar a engenharia sem confundir simulação com firmware seguro.

## Decisão

Usar **um contrato C de serviços injetáveis**, com callbacks e contexto, sem depender diretamente de `FILE*`, `malloc` ou endereços da ROM Canon. O backend de teste é exclusivamente RAM, de tamanho fixo, com arquivos protegidos contra sobrescrita, handles versionados, retornos explícitos de bytes transferidos, fila cooperativa e falhas programáveis.

Uma falha de I/O pode acontecer **após alguns bytes já terem sido escritos**. O mock preserva esses bytes para que os testes detectem tratamento incorreto. Reinicialização invalida handles e descarta trabalhos pendentes.

## Alternativas descartadas

- Usar `fopen` e `malloc` na API comum: acopla a plataforma ao host.
- Codificar endereços Canon diretamente no motor fotográfico: inseguro sem hardware.
- Emular threads/tempos aleatórios: resultados menos reproduzíveis e potencial falsa confiança.
- Usar arquivos reais como mock: risco desnecessário sobre os dados do usuário.

## Consequências

A HAL mede correção da lógica em host, **não** realismo da câmera. A fila não é o scheduler VxWorks, e não há DMA, cache, eletricidade ou comportamento de CF real. Futuro backend Canon precisará de flush/rename, recursos, memória e tarefas verificados, com recuperação planejada antes de processar imagens.

Ver [HAL-SIMULADA.md](../HAL-SIMULADA.md).
