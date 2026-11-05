# Arquitetura proposta

> Documento de projeto. O núcleo embarcado, o carregador e a camada de hardware ainda **não existem** no código atual.

## Objetivo

Conseguir adicionar funcionalidades sem alterar o `AUTOEXEC.BIN` toda vez. A instalação, o carregamento e a compatibilidade com a câmera devem ser responsabilidades separadas.

```text
Computador (futuro)                  Canon EOS 5D Classic (futuro)
┌─────────────────────────────┐      ┌─────────────────────────────────┐
│ instalador / preparador CF  │      │ bootloader Canon (existente)    │
│  backup • hash • compat.    │      │        │                        │
└──────────────┬──────────────┘      │        ▼                        │
               │ cartão verificado  │ loader mínimo / inicialização  │
               └────────────────────►│        │                        │
                                     │        ▼                        │
                                     │ serviços Rebobina              │
                                     │   ├─ HAL 5DC.111               │
                                     │   ├─ gestão de memória          │
                                     │   ├─ API de módulos            │
                                     │   └─ interface de usuário      │
                                     │        │                        │
                                     │    módulos opcionais           │
                                     │       └─ Soup Engine           │
                                     └─────────────────────────────────┘
```

O componente realmente implementado hoje é um **protótipo de registro de módulos em C no computador** (`src/core/registry.c`). O restante é arquitetura-alvo.

## Separação de responsabilidades

1. **Ferramentas host:** validar manifestos, preparar pacotes e, futuramente, orientar a instalação. Nunca ativar boot flags silenciosamente.
2. **Bootstrap/loader:** componente específico por câmera. A decisão de usá-lo dependerá da análise do Magic Lantern 5D. A validação de integridade precisa ocorrer antes de carregar módulos, mas não substitui a checagem do bootloader Canon.
3. **HAL (Hardware Abstraction Layer):** APIs Canon/Magic Lantern encapsuladas por modelo e firmware, sem endereços de memória hard-coded dentro dos módulos.
4. **Core:** registro, resolução de dependências (futuro), inicialização ordenada, encerramento e recursos compartilhados.
5. **Módulos:** recursos isolados por interface lógica, sem garantia de isolamento de memória. Na câmera, qualquer falha de módulo pode afetar o sistema inteiro.

## Módulos: decisões iniciais

- **API versionada:** `RB_MODULE_ABI_VERSION`; incompatibilidades rejeitadas.
- **Identificador único:** cada módulo tem `id` estável; conflitos são rejeitados.
- **Ciclo de vida:** `start` e `stop`; se uma inicialização falhar, os anteriores são encerrados em ordem reversa.
- **Manifesto por módulo:** `module.json` com versão, plataformas e capacidades declaradas.
- **Compatibilidade explícita:** `5DC.111` não significa 5D Mark II nem homologação.
- **Capacidades no manifesto não são isolamento nem permissões aplicadas.** Precisaremos de validação e controle no runtime antes de confiar nelas.
- **Sem carregador dinâmico agora:** o exemplo `hello` é ligado estaticamente ao protótipo host, não funciona na câmera.

## Relação com o Magic Lantern

O Magic Lantern já oferece módulos `.mo` em suas plataformas suportadas. Não é necessário reinventar todo seu trabalho. A primeira pesquisa é identificar o que pode ser reutilizado com segurança no port da 5D Classic, cuja integração atual do Soup Engine usa patch específico do binário.

A reutilização de trechos do Magic Lantern deverá respeitar sua licença GPLv2 e manter atribuições. A interface experimental Rebobina **não é** a API de módulos ML `.mo`.

## O que precisamos medir em hardware antes de prosseguir

- Variante exata do executável Magic Lantern e SHA-256 de referência;
- Layout de memória, budgets do payload, stacks e tarefas da 5D Classic;
- Inicialização e retorno seguro ao firmware original;
- Latência de interface, gravação simultânea do CR2 e falhas de energia;
- Consumo de RAM e memória livre por módulo;
- Recuperação quando um módulo está corrompido ou ausente.

Nenhuma das etapas envolve prometer recuperação após um `AUTOEXEC.BIN` inválido: nesses casos, nosso código pode sequer chegar a executar.
