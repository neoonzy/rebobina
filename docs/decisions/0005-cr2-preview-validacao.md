# ADR 0005 — Validar limite do JPEG embutido antes de usar o preview CR2

**Data:** 08/11/2026  
**Estado:** aceita como regra defensiva no protótipo host; sem garantia de integridade de CR2 real.

## Contexto

A função original `se_cr2_find_preview` do Soup Engine interpreta offsets TIFF e verifica que o JPEG tem marcador inicial SOI, mas não exige, nessa etapa, que todos os bytes anunciados pela tag `StripByteCounts` caibam no arquivo. Confiar cegamente no intervalo poderia criar leitura fora do arquivo ao passar esse slice para processamento.

## Decisão

Implementar `rb_cr2_host_find_preview` como wrapper de teste que:
- exige assinatura TIFF/CR2;
- recusa overflow de offset/comprimento e deslocamento fora de buffer;
- valida tamanho mínimo, dimensões plausíveis, número de componentes e largura/altura declaradas versus cabeçalho JPEG;
- não modifica código do Soup Engine upstream e não aceita um endereço arbitrário de memória proveniente do arquivo.

A versão inicial do teste usa **um envelope CR2 sintético** que contém apenas o preview JPEG, e não um RAW Canon completo. O objetivo é verificar parsing defensivo e limites, não afirmar processamento de fotografias reais.

## Limites

Verificar bounds não é validar integridade criptográfica, corrupção interna de blocos, qualidade de imagem, origem ou autenticidade da foto. Também não há qualquer evidência de segurança física ou desempenho da câmera.

Quando existir uma amostra CR2 real da 5D, registrar dimensões, saída do parser, erros e critérios reprodutíveis, **sem publicar imagens privadas ou metadados pessoais sem autorização**.
