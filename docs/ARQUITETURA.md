# Arquitetura proposta

> Documento de projeto. O núcleo embarcado, o carregador e a camada de hardware ainda **não existem** no código atual.

## Objetivo

**Construir uma alternativa embarcada ao Magic Lantern, e não um instalador para ele.** O Rebobina deve ter código de inicialização, núcleo, interface e módulos próprios, operando junto ao firmware Canon original em execução. Ele poderá reutilizar componentes GPLv2 do Magic Lantern, após revisão de compatibilidade, mas não será apenas uma camada desktop sobre uma instalação já pronta.

A capacidade de adicionar funcionalidades sem alterar o `AUTOEXEC.BIN` a cada extensão é uma meta fundamental. A instalação, o carregamento e a compatibilidade com a câmera são responsabilidades diferentes; a instalação assistida será um recurso auxiliar e posterior.

```text
Canon EOS 5D Classic (arquitetura pretendida)
  Firmware Canon original (permanece na ROM)
      |
      | bootloader lê cartão CF preparado
      v
  Rebobina — executável embarcado próprio
      |
      +-- bootstrap / integração com VxWorks e DIGIC II
      +-- HAL e serviços compartilhados
      +-- interface de usuário Rebobina
      +-- sistema de módulos
             +-- funcionalidades fotográficas
             +-- Soup Engine (adaptação futura)
             +-- outros módulos (futuros)

Computador: testes, simulador e ferramenta auxiliar de instalação
```

Componentes implementados no computador: **registro de módulos** (`src/core/registry.c`), **gerenciador individual de módulos** (`src/core/manager.c`), **modelo de interface** (`src/core/ui.c`), simulador terminal (`src/host/menu_sim.c`), [simulador visual web](SIMULADOR-VISUAL.md) via ponte C + servidor localhost e carregador **Linux host** de bibliotecas `.so` (`src/host/plugin_loader.c`). A HAL em RAM (`include/rebobina/hal.h`, `src/host/hal_sim.c`) simula serviços de arquivo/memória/tarefas e permite falhas injetáveis sem tocar em mídia. O motor original Soup Engine é compilado externamente no host para selftests, **não como extensão Rebobina**. O protocolo de saída `src/core/output.c` publica JPEG no mock em duas fases, sem sobrescrever originais. A semântica real de `sync` e `rename_noreplace` **não foi validada no CF** ([SAIDA-SEGURA.md](SAIDA-SEGURA.md)). O motor Soup também executa um stock real sobre pixels sintéticos no host. O inspector host CR2 (`src/host/cr2_host.c`, `src/host/cr2_inspect.c`) usa o parser Soup para validar o intervalo do preview JPEG, sem tocar no cartão ou no RAW. O teste fim a fim com um envelope CR2 **sintético** não substitui testes com amostras de câmera ([CR2-INSPECAO-HOST.md](CR2-INSPECAO-HOST.md)). A compilação ARM produz objetos relocáveis, não um executável. O **bootstrap, HAL Canon, carregador embarcado e tela física** permanecem arquitetura-alvo.

## Separação de responsabilidades

1. **Rebobina embarcado:** é o programa principal. Deve inicializar, cooperar com o sistema Canon, oferecer interface própria e gerenciar a execução das extensões.
2. **Bootstrap/loader:** entrada específica para cada câmera; a implementação dependerá do estudo do Magic Lantern para 5D Classic. Validar módulos não substitui a checagem do bootloader Canon.
3. **HAL (Hardware Abstraction Layer):** APIs Canon/VxWorks e recursos aproveitados do Magic Lantern encapsulados por firmware, evitando endereços hard-coded dentro dos módulos.
4. **Core:** gestão de memória, eventos, registro de serviços, módulos, dependências, inicialização e desligamento.
5. **Módulos:** funcionalidades extensíveis com interface lógica, sem isolamento de memória garantido na câmera.
6. **Ferramentas host (auxiliares):** simulador, compilação, testes, preparação de pacotes e, futuramente, um instalador. Nunca ativar boot flags silenciosamente.

## Módulos: decisões iniciais

- **API versionada:** `RB_MODULE_ABI_VERSION`; incompatibilidades rejeitadas.
- **Identificador único:** cada módulo tem `id` estável; conflitos são rejeitados.
- **Ciclo de vida:** `start` e `stop`; se uma inicialização falhar, os anteriores são encerrados em ordem reversa.
- **Manifesto por módulo:** `module.json` com versão, plataformas e capacidades declaradas.
- **Compatibilidade explícita:** `5DC.111` não significa 5D Mark II nem homologação.
- **Capacidades no manifesto não são isolamento nem permissões aplicadas.** Precisaremos de validação e controle no runtime antes de confiar nelas.
- **Sem carregador dinâmico agora:** o exemplo `hello` é ligado estaticamente ao protótipo host, não funciona na câmera.

## Relação com o Magic Lantern

O Rebobina pretende **substituir a extensão Magic Lantern no cartão por um programa seu**, preservando o firmware original Canon na câmera. Isso não implica criar um firmware Canon novo nem gravá-lo na ROM. O Magic Lantern já oferece módulos `.mo` em plataformas suportadas; devemos estudar seus contratos e sua integração com a 5D Classic antes de escolher entre reaproveitar código e desenvolver componentes novos. O Soup Engine atual usa patch específico do executável da 5D e exigirá adaptação real para ser um módulo do Rebobina.

A reutilização de trechos do Magic Lantern deverá respeitar sua licença GPLv2 e manter atribuições. A interface experimental Rebobina **não é** a API de módulos ML `.mo`.

## O que precisamos medir em hardware antes de prosseguir

- Variante exata do executável Magic Lantern e SHA-256 de referência;
- Layout de memória, budgets do payload, stacks e tarefas da 5D Classic;
- Inicialização e retorno seguro ao firmware original;
- Latência de interface, gravação simultânea do CR2 e falhas de energia;
- Consumo de RAM e memória livre por módulo;
- Recuperação quando um módulo está corrompido ou ausente.

Nenhuma das etapas envolve prometer recuperação após um `AUTOEXEC.BIN` inválido: nesses casos, nosso código pode sequer chegar a executar.
