# Contribuindo com o Rebobina

Obrigado pelo interesse! O projeto ainda é experimental e utiliza a **Canon EOS 5D Classic, firmware 1.1.1**, como referência inicial.

## Desenvolvimento local

Você precisa de um compilador C11, `make` e Python >= 3.9.

```sh
make demo
make hal-demo     # HAL mock, sem arquivos reais nem cartão
make test
make check-modules
make audit-platform
make arm-objects   # compilação de objetos, NÃO firmware
make sim-ui        # simulador de interface no terminal
make web-sim       # interface visual em navegador local
python3 tools/soup_host.py --fetch  # teste upstream Soup e compila CLI Linux (requer Internet)
# Leia docs/CR2-DESENVOLVIMENTO-HOST.md para processar CR2 em diretorio exclusivo
```

Antes de enviar alterações, documente o propósito, os arquivos alterados, os comandos realmente executados e suas saídas. Não descreva testes não executados como aprovados.

## Pull requests

- Separe mudanças pequenas e rastreáveis;
- Adicione testes para lógica nova;
- Atualize a documentação para mudanças de API;
- Não versione ROMs, firmware Canon, imagens privadas ou binários de origem incerta;
- Trate dependências do Magic Lantern e Soup Engine como projetos de terceiros, preservando licença e autoria;
- Nenhuma alteração de inicialização em câmera deve ser tratada como segura sem evidência de teste específica.

## Projetando um novo módulo

Leia [docs/MODULOS.md](docs/MODULOS.md). O primeiro passo é um manifesto e testes host. Um manifesto válido **não prova compatibilidade** com hardware.

## Licença

As contribuições originais deverão ser compatíveis com GPL-2.0-only. Se você reutilizar código, identifique sua origem e mantenha seus avisos de copyright.
