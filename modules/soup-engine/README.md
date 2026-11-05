# Soup Engine no Rebobina — integração planejada

Este diretório contém **apenas um manifesto de intenção**. Não existe aqui um módulo executável, nem um pacote que possa ser instalado na câmera.

## Desafio

Na implementação atual da 5D Classic, o Soup Engine utiliza um `AUTOEXEC.BIN` adaptado e `SE.BIN`. Ele **não é compatível automaticamente** com a API experimental do Rebobina nem com módulos `.mo` do Magic Lantern.

## Etapas previstas

1. Mapear serviços usados pelo Soup Engine (arquivos, interface, tarefas, memória e captura).
2. Identificar quais desses serviços podem ser expostos pela camada de compatibilidade da 5D.
3. Propor uma interface compartilhada, sem acoplar o motor fotográfico ao inicializador.
4. Executar o motor em testes host e simulação.
5. Integrar em hardware de teste somente após validar inicialização, recuperação e memória.

Fonte: [neoonzy/soup-engine](https://github.com/neoonzy/soup-engine). Licenças e créditos originais deverão ser preservados em qualquer integração de código.
