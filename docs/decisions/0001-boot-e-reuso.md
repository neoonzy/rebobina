# ADR 0001 — Programa independente, boot e reaproveitamento

- **Data:** 05/11/2026
- **Status:** Proposta de arquitetura condicionada à pesquisa da 5D Classic
- **Alvo:** Canon EOS 5D Classic, firmware 1.1.1

## Contexto

O Rebobina deve ser um **software próprio que roda na câmera**, e não um instalador de Magic Lantern. O Magic Lantern oferece aprendizados e código sob GPLv2, mas o port moderno `magiclantern_simplified/dev` inspecionado não possui um diretório `5DC.111/`. O Soup Engine para a primeira 5D oferece uma referência concreta de hooks em uma build Magic Lantern de 2012, mas essa base não foi identificada por SHA-256.

A inicialização e o display da 5D Classic são sensíveis. Copiar patches com endereços absolutos sem identificar exatamente o executável-base não é um caminho aceitável.

## Decisão proposta

1. **Produto:** entregar um executável embarcado e interface **Rebobina**, carregados por cartão, coexistindo com o firmware Canon original e sem regravar a ROM.
2. **Reutilização:** aproveitar código GPLv2 do Magic Lantern ou do Soup Engine apenas onde a origem, licença e compatibilidade forem verificadas. Documentar créditos e trechos adaptados. Não importar firmware Canon nem binários-base de procedência incerta.
3. **Separação:** manter serviços de câmera em HAL `5DC.111`, núcleo independente de hardware e módulos desacoplados de endereços absolutos.
4. **Carregamento:** projetar sistema próprio de módulos, mas não assumir compatibilidade binária automática com `.mo` do ML nem `SE.BIN` do Soup Engine.
5. **Boot:** **nenhuma decisão final de patch/bootstrap** até obter binário-base e fontes da versão exata da 5D Classic; experimentar primeiro em host/simulador, depois em câmera de desenvolvimento com recuperação.
6. **Instalador:** somente depois de existir um executável Rebobina comprovadamente inicializável. Ele é auxiliar, não produto principal.

## Alternativas avaliadas

| Alternativa | Decisão | Motivo |
| --- | --- | --- |
| Criar instalador do ML atual | Rejeitada como produto principal | Não atende à ideia de novo software na câmera |
| Alterar Soup/ML 2012 via patches diretamente | Adiada | Requer SHA, proveniência e testes físicos |
| Reescrever todo o ML do zero | Rejeitada como premissa | Repetiria pesquisa de hardware e trabalho comunitário já feito |
| Núcleo próprio com HAL e reuso seletivo GPLv2 | **Escolha arquitetural preliminar** | Preserva a identidade do Rebobina e reduz duplicação, com etapas de validação |

## Consequências e bloqueios

- O protótipo de módulos em C atual **não comprova** funcionamento embarcado.
- A evolução deve aceitar limites de memória, pilhas separadas, GUI concorrente e bloqueios de I/O.
- Testes estáticos verificam **consistência documental**, não validade de endereços na ROM.
- Sprints de bootstrap dependem da identificação real do `AUTOEXEC.BIN` de referência e de uma câmera de desenvolvimento.
- A licença GPL-2.0-only do Rebobina é compatível com a reutilização que respeite a origem e os avisos exigidos pela GPLv2.

## Evidências

Consultar [levantamento técnico](../5DC-PLATAFORMA.md) e [inventário](../data/5dc.111-evidence.json). Reavaliar esta decisão ao obter fonte e binário correspondentes ao mesmo build histórico.
