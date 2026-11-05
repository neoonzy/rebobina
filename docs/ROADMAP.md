# Roadmap — Rebobina embarcado

**Produto principal:** um novo programa na Canon EOS 5D Classic, inspirado no Magic Lantern e com interface e sistema de módulos próprios. **Não** um instalador de Magic Lantern.

Este roadmap apresenta as etapas amplas. A divisão operacional em ciclos, tarefas e critérios de aceite está em **[SPRINTS.md](SPRINTS.md)**.

As etapas diferenciam protótipo host, simulação e firmware experimental. Nada neste repositório está pronto para instalação em câmera.

## Etapa 0 — Fundação

- [x] Definir o escopo da 5D Classic (1.1.1) e o propósito modular;
- [x] Documentar arquitetura, segurança e diretrizes de contribuição;
- [x] Criar protótipo C de registro de módulos no host;
- [x] Criar manifestos JSON e ferramenta de validação;
- [x] Adicionar testes e CI host (primeira execução aprovada);
- [ ] Definir política de versões e changelog.

## Etapa 1 — Estudar a plataforma de base

- [ ] Identificar a versão exata do Magic Lantern para 5DC.111 e sua procedência;
- [ ] Mapear os mecanismos de inicialização, hooks, tarefas e endereços da 5D Classic;
- [ ] Estudar código GPLv2 reutilizável e obrigações de atribuição;
- [ ] Mapear memória disponível, execução de interface, captura e escrita em CF;
- [ ] Identificar limitações do sistema de módulos já existente no Magic Lantern;
- [ ] Analisar a integração atual do Soup Engine e as dependências que precisaremos abstrair.

**Entrega:** mapa técnico confiável do hardware e decisão fundamentada de reaproveitamento. Sem distribuir binários à câmera.

## Etapa 2 — Primeiro Rebobina embarcado

- [ ] Criar build ARM específico da 5D Classic;
- [ ] Integrar com o boot e com o ambiente Canon sem alterar a ROM;
- [ ] Inicializar um serviço mínimo e reportar diagnóstico no equipamento;
- [ ] Implementar HAL inicial com interface, tarefas, arquivos e memória;
- [ ] Testar em simulador e depois numa câmera de desenvolvimento, com recuperação documentada.

**Entrega:** primeiro executável experimental Rebobina que inicializa e encerra de forma controlada. Não é um release para usuários finais.

## Etapa 3 — Interface e sistema de módulos

- [ ] Implementar interface Rebobina na tela da câmera;
- [ ] Definir ABI embarcada e carregador versionado de módulos;
- [ ] Resolver dependências, compatibilidade e ativação/desativação;
- [ ] Garantir limites de memória, logs e tratamento de falhas conforme a plataforma;
- [ ] Criar um primeiro módulo simples que funcione na câmera, com testes documentados.

**Entrega:** núcleo próprio extensível, independente do menu habitual do Magic Lantern.

## Etapa 4 — Portar o Soup Engine

- [ ] Separar motor fotográfico das funções específicas de inicialização;
- [ ] Adaptar arquivos, interface e tarefas à API Rebobina;
- [ ] Executar primeiro no simulador e validar resultados e recursos;
- [ ] Testar processamento JPEG/RAW e interrupções na câmera;
- [ ] Documentar compatibilidade, instalação, desempenho e limitações.

**Entrega:** primeiro módulo fotográfico real do ecossistema Rebobina.

## Etapa 5 — Distribuição e expansão

- [ ] Criar instalador desktop opcional com validação de hashes, backup e reversão;
- [ ] Preparar pacotes reproduzíveis por câmera/firmware;
- [ ] Melhorar interface, acessibilidade e experiência de gerenciamento de módulos;
- [ ] Documentar como construir novas extensões e portar outros modelos;
- [ ] Implementar novos módulos (intervalômetro, ferramentas de foco e exposição etc.).

## Critério para marcar algo como pronto

Código versionado, testes reproduzíveis e documentação distinguindo claramente host, simulação e câmera real. Nenhuma melhoria de instalação substitui a validação do programa embarcado.
