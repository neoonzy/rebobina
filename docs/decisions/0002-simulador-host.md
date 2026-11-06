# ADR 0002 — Simulador visual usando o núcleo C

- **Data:** 06/11/2026
- **Estado:** Aceita para protótipo host; não determina arquitetura definitiva da câmera.
- **Decisão:** implementar uma interface web local que envia eventos e recebe o estado **calculado no código C**, em vez de reimplementar o menu inteiro em JavaScript.

## Por quê

Sem leitor CompactFlash não é possível validar UI em câmera. A interface existente `src/core/ui.c` já representa seleção e ações de menu, e duplicar a máquina de estados no navegador aumentaria o risco de o protótipo visual se distanciar do futuro port embarcado.

## Contrato atual

- `tools/simulator_server.py` vincula **apenas** `127.0.0.1`, usa `GET /api/state?events=` e serve uma lista fechada de arquivos estáticos;
- `src/host/simulator_bridge.c` executa o menu C e envia um protocolo tabulado para o Python converter em JSON;
- cada chamada C é isolada e reproduz o histórico de eventos desde o início, sem persistência;
- `src/core/manager.c` gerencia individualmente o `hello` de demonstração, com ABI versão 1;
- o frontend só renderiza as linhas retornadas pelo C, com `textContent`, e oferece botões/teclado responsivos;
- os símbolos `t/u/d/s/b` **não são códigos do firmware Canon**; são convenções de simulação.

## Alternativas

- **Duplicar a UI em JS:** rejeitada pelo risco de estado divergente.
- **WASM C diretamente no navegador:** adiada; toolchain e compilação WASM adicionariam complexidade antes de termos a HAL.
- **Aplicativo Qt/Electron:** adiado para não exigir bibliotecas de interface adicionais.
- **Servidor local Python + processo C determinístico:** selecionado pela facilidade de teste em CI e pela ausência de dependências web externas.

## Limites e segurança

O modelo não simula o hardware. Não executa tarefas de câmera, nem acessa memória ou arquivos Canon. Um módulo que falha em `start` deve limpar seus próprios recursos antes de retornar falha; `rb_manager` não pode fazer rollback de efeitos internos desconhecidos.

O gerenciador usa módulos já ligados ao processo ou fornecidos por mecanismos host confiáveis. **Não existe isolamento de memória entre módulos**, nem assinatura, loader de ARM ou suporte a plugin no CF.

Revisitar esta decisão quando uma HAL real, orçamento de RAM e eventos do VxWorks tiverem sido verificados em hardware.
