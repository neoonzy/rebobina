# Inspeção do preview JPEG em CR2 — etapa host

**Data:** 08/11/2026  
**Estado:** protótipo de inspeção *somente leitura* no computador; **sem validação em amostra real da Canon EOS 5D Classic e sem decodificação do RAW**.

## Por que existe

O Soup Engine pode localizar o JPEG de visualização embutido num arquivo CR2 por tags TIFF, mas a localização, isoladamente, não verifica se o intervalo anunciado cabe inteiramente no arquivo. Não devemos confiar num par de offset/tamanho sem conferir o tamanho efetivo do arquivo: especialmente no caso de arquivos truncados, corrompidos ou recebidos de fontes externas.

O wrapper Rebobina em `src/host/cr2_host.c`, `rb_cr2_host_find_preview`, acrescenta verificações de:

- Assinatura básica do CR2 (little endian TIFF + marcação "CR");
- Faixa `offset` / `bytes` completamente dentro do buffer, com subtração para evitar overflow;
- Dimensões positivas e até 12.000 por eixo;
- Confirmação do cabeçalho JPEG pelo parser real do Soup, 3 componentes de cor;
- Concordância das dimensões TIFF com o JPEG embutido.

O wrapper **não** garante que todo o JPEG decodifica sem erro, que o CR2 é autêntico, que o RAW existe ou que o modelo é realmente a EOS 5D. A leitura completa e o tratamento de erros precisam ocorrer antes da publicação de uma saída.

## Como experimentar com um CR2 no futuro

Numa máquina Linux com `git`, `cc`, `make`, Python e rede:

```sh
python3 tools/soup_host.py --fetch
./build/rebobina-cr2-inspect /caminho/para/foto.CR2
```

O primeiro comando baixa **apenas a revisão de código-fonte fixada** do Soup Engine e compila utilitários host. O segundo abre o arquivo somente para leitura, com um limite de **64 MiB**, e imprime as dimensões, o offset e o tamanho do preview validado. Não extrai fotos nem grava arquivos, e não exige acesso à câmera ou ao cartão.

Um resultado positivo significa apenas que a estrutura do *preview* é coerente com a lógica verificada. **Não** significa que o conteúdo CR2/RAW esteja íntegro ou que o código já funcione na câmera.

## Fixture de teste, que NÃO é uma foto Canon

`tests/soup_cr2_preview.c` gera um pequeno JPEG 8×8 com o **codificador real** do Soup e o embute em um envelope sintético com a assinatura e IFD0 TIFF/CR2. **Esse envelope não contém sensor RAW, MakerNotes nem metadados Canon reais**.

O teste usa `se_cr2_find_preview` do upstream através do wrapper e compara duas decodificações do mesmo JPEG: uma recuperada do envelope e outra direta. Também injeta cabeçalho errado, compressão diferente de JPEG, strip truncado, declarações de tamanho maiores que o arquivo, assinaturas falsas e dimensões incoerentes.

`tools/soup_host.py` compila esse teste no CI e verifica que o CLI de inspeção rejeita argumentos ausentes. A última comparação com uma fotografia CR2 real continua **pendente até existir uma amostra autorizada**.

## Próximos passos

1. Validar cabeçalho, offset e tamanho em um **CR2 real da 5D Classic**, lido e preservado sem alterações.
2. Executar `se_jpeg_decode_scaled` no preview completo de uma cópia e testar uma entrada truncada.
3. Conectar a saída JPEG do Soup e os arquivos de filme `.fs` ao adaptador de publicação em duas fases (`rb_output`) num protótipo que trabalhe com fotos reais **somente no computador**, sem sobrescrever originais.
4. Testar perfis e truncamento sob ASan/UBSan, observando o aviso ainda pendente de halation em `engine/look.c`.
5. **Só depois**, adaptar esse caminho para os serviços reais de arquivos Canon/VxWorks, testados na 5D com firmware 1.1.1.

Referências: [Soup Engine host](SOUP-ADAPTACAO.md), [saída em duas fases](SAIDA-SEGURA.md) e [roteiro sem CF](SPRINT-SEM-CF.md).
