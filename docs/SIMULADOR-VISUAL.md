# Simulador visual do Rebobina — primeira versão

**Escopo:** computador; interface e ciclo de vida de um módulo de exemplo. **Não é um emulador da EOS 5D.**

Este simulador permite trabalhar na experiência de uso do Rebobina sem o leitor CompactFlash. A interface no navegador representa artisticamente a parte traseira da Canon 5D Classic, mas **as ações são processadas pelo código C do projeto**, por um servidor Python local. Não se conecta à câmera, não altera cartões e não cria firmware.

## Executar

Requisitos: computador com Linux, compilador C11, Python 3.9+ e `make`. Para o simulador web, **não precisa** de Clang ARM, Magic Lantern, firmware Canon ou leitor CF.

```sh
git clone https://github.com/neoonzy/rebobina.git
cd rebobina
make web-sim
```

Abra **http://127.0.0.1:8765/** em um navegador no **mesmo computador**. Interrompa com `Ctrl+C`. O servidor escuta apenas o endereço local `127.0.0.1`: não é um site público nem é acessível automaticamente pelo telefone na rede.

Se precisar usar uma porta diferente:

```sh
make build/rebobina-sim-bridge
python3 tools/simulator_server.py --port 8899
```

Para executar a suíte: `make test`. A interface de terminal anterior permanece acessível por `make sim-ui`.

## Controles

| Tecla / botão visual | Evento C | Função |
| --- | --- | --- |
| `Delete` / lixeira | `t` | Abre e fecha o menu |
| `↑` / botão superior | `u` | Move a seleção para cima |
| `↓` / botão inferior | `d` | Move a seleção para baixo |
| `Enter` / `SET` | `s` | Entra na página selecionada; alterna Hello na página Módulos |
| `Escape` / voltar | `b` | Retorna ao menu ou o fecha |
| Botão Reiniciar | reinicia eventos | Restabelece o estado inicial |

O simulador usa o menu de quatro itens definido em `src/core/ui.c`: Início, Ferramentas, Módulos e Sobre. A entrada de teclado é apenas uma conveniência do laboratório, não o mapeamento real de eventos da 5D.

### Fluxo para experimentar um módulo

1. Aperte **lixeira** para abrir o menu.
2. Pressione **baixo** duas vezes e **SET** para entrar em Módulos.
3. Na tela Módulos, pressione **SET** para ativar o **Hello (simulação)**.
4. Pressione novamente **SET** para desativá-lo; **Voltar** retorna ao menu.

Isso chama `rb_manager_enable` e `rb_manager_disable` do projeto C com o exemplo `hello`, **que não é o binário do Soup Engine**.

## Arquitetura e fonte da verdade

```text
Navegador (simulator/web)
    botões -> GET /api/state?events=...
        |
        v
tools/simulator_server.py       Python HTTP somente localhost
        |
        +--> build/rebobina-sim-bridge (um processo por requisição)
                  |
                  +--> src/core/ui.c       menu/seleção
                  +--> src/core/manager.c  Hello ativo/inativo
                  +--> src/core/registry.c validação do contrato
        |
        v
JSON { state, rows, source:"rebobina-core-c", hardware:false }
        |
        v
Navegador desenha linhas usando textContent
```

O servidor recebe uma sequência de até **128 eventos** (`t`, `u`, `d`, `s`, `b`); a cada requisição, o processo C **reexecuta essa sequência do zero** e retorna o estado. Esse formato é intencional para ser simples, determinístico e testável, mas não é um sistema de sessão real e os módulos não mantêm dados entre execuções. Um módulo marcado como 'habilitado' é **habilitado no estado simulado**; o processo temporário é encerrado após responder, então não existe plugin executando continuamente em segundo plano. Nenhuma ação do navegador cria arquivos na câmera nem persiste configurações.

O servidor expõe **somente GET**, arquivos web explicitamente listados e a API de simulação; rejeita outros caminhos, POST, eventos desconhecidos e sequências longas. Não aceita caminhos de arquivo nem argumentos de shell do usuário. Isso reduz a superfície de erro, mas o software permanece um **protótipo local**.

## Testes

```sh
make test
make check-modules
make audit-platform
```

- C: início/parada individual de módulos, erro de ABI, duplicação, estado inválido, rollback entre módulos e limpeza;
- HTTP: abre menu, muda seleção, entra em Módulos, ativa e desativa Hello, retorna;
- Entrada inválida: eventos proibidos, tamanho máximo e tentativa de acesso a arquivos fora da pasta web;
- HTTP: servidor somente leitura, cabeçalhos de segurança e resposta estruturada;
- ARM: `make arm-objects` compila o manager como objeto **relocável** para ARM946E-S; não é um executável de câmera.

A CI Linux executa os testes de host com e sem UBSan. Isso não substitui teste de câmera.

## Limitações

- **Não** emula DIGIC II, VxWorks, LCD físico, cartão CompactFlash, USB, sensores, firmware Canon nem o bootloader.
- A carcaça visual não é representação técnica dimensional da 5D; serve para prototipação de UX.
- **Não** processa JPEG/CR2, não ativa funções fotográficas reais e não executa Soup Engine.
- O gerenciador em C só opera sobre módulos **já presentes e confiáveis no processo**; não carrega plugins ARM de um cartão.
- Dependências, memória, interrupções de energia, concorrência de câmera e restauração de estado não são simuladas.
- A ABI permanece experimental. Um `.so` Linux e um `SE.BIN` legado não são módulos embarcados Rebobina intercambiáveis.

## Critérios para etapa seguinte

- Introduzir testes de navegação e acessibilidade com usuários e diferentes tamanhos de tela;
- Projetar a HAL e o modelo de eventos com **interfaces simuladas** de arquivo/tarefas sem incorporar endereços Canon nas telas;
- Separar o motor do Soup Engine de `cam.h`, com testes de saída reproduzível a partir de **imagens sintéticas**;
- Somente após obter leitor CF, atualizar o firmware da 5D Classic **1.1.0 para 1.1.1**, confirmar a procedência dos binários e testar o boot de forma controlada.

## Segurança e situação física

A câmera do usuário **ainda estava em firmware 1.1.0** quando esta etapa foi iniciada e não havia leitor CF. **Não tente usar binários para 1.1.1 em 1.1.0.** A PR experimental de boot é separada desta branch do simulador.

Consulte [o roadmap de trabalho sem CF](SPRINT-SEM-CF.md) e [a arquitetura](ARQUITETURA.md).
