# Roadmap

O avanço depende de evidências: **não confundir protótipo host com câmera suportada**.

## Etapa 0 — Fundação de projeto

- [x] Definir escopo e câmera inicial;
- [x] Documentar arquitetura modular e limites;
- [x] Criar registro C de módulos no host;
- [x] Criar manifestos e ferramenta de validação;
- [x] Adicionar testes e workflow de CI (execução ainda deve ser verificada);
- [ ] Publicar convenções de versionamento e changelog.

## Etapa 1 — Pesquisa da Canon 5D Classic

- [ ] Catalogar build exato do Magic Lantern para 5DC.111;
- [ ] Documentar inicialização VxWorks, chamadas Canon e hooks usados;
- [ ] Mapear memória, threads/tasks, recuperação e armazenamento;
- [ ] Comparar carga e funcionamento do Soup Engine atual;
- [ ] Confirmar compatibilidade de licenças e origem dos arquivos.

**Saída:** documentação técnica com fontes e riscos. **Sem binários novos para câmera.**

## Etapa 2 — Ferramenta desktop segura

- [ ] Inspecionar cartão CF e mostrar diagnóstico sem escrita;
- [ ] Validar câmera/firmware/binários mediante hashes conhecidos;
- [ ] Gerar backup verificável e plano de reversão;
- [ ] Preparar uma cópia de saída (staging), nunca sobrescrever automaticamente o cartão;
- [ ] Testar casos de erro e operações interrompidas.

## Etapa 3 — Núcleo embarcado mínimo

- [ ] Decidir com base em pesquisa se aproveitamos ML e seus hooks ou precisamos de integração nova;
- [ ] Implementar HAL mínima 5DC.111 e logs;
- [ ] Testar inicialização e desligamento primeiro em simulação, depois em hardware de desenvolvimento;
- [ ] Estabelecer budgets de memória e stack;
- [ ] Projetar estratégia de falha de módulo e recuperação.

## Etapa 4 — Extensões

- [ ] Definir ABI para câmera e manifesto definitivo;
- [ ] Resolver dependências, versões e carregamento de módulos;
- [ ] Implementar menu de módulos e estados (ativado/desativado);
- [ ] Portar Soup Engine **a partir do código-fonte**, sem reaproveitar binário incompatível;
- [ ] Validar RAW, escrita simultânea, desempenho e interrupções.

## Etapa 5 — Experiência e expansão

- [ ] Interface na câmera com acessibilidade e diagnóstico;
- [ ] Instalador desktop com UX clara de riscos e reversão;
- [ ] Documentar processo para portar outros modelos;
- [ ] Construir releases reproduzíveis e separados por firmware;
- [ ] Avaliar novas extensões: intervalômetro, sobreposição de informações, controle de exposição.

## Regra para marcar uma etapa como pronta

Teste documentado, instruções reproduzíveis e distinção entre desenvolvimento host, simulação e câmera física. Somente um teste executado comprova seu resultado.
