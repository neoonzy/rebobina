# Desenvolvimento sem leitor CompactFlash

**Data:** 06/11/2026  
**Contexto:** câmera Canon EOS 5D Classic disponível, mas ainda com firmware **1.1.0**; sem leitor CF para transferir a atualização e testar o payload. O alvo do Rebobina continua sendo **1.1.1**.

## Estratégia

Não interromper o desenvolvimento por ausência de hardware de teste, nem dar por concluídas sprints embarcadas. Dividir atividades entre três trilhas:

| Trilha | O que pode ser feito agora | O que permanece bloqueado |
| --- | --- | --- |
| Interface | Simulador visual, navegação, acessibilidade, estados e feedback de erros | LCD físico, botões reais, interferência na captura |
| Extensões | Contrato C, gerenciador host e testes de ciclo de vida, formatos e ABI futuros | Loader ARM no CF, RAM/relocação e proteção real |
| Processamento | Isolar o motor fotográfico do Soup Engine, usar fotos sintéticas e comparar bytes | Transferência de CR2 real, desempenho na câmera e integridade de fotos |
| Compilação | Objetos freestanding ARM, auditorias e pipelines de CI | Inicialização do VxWorks e recuperação de travamento na 5D |

## Entrega desta etapa

- [x] Construir interface visual responsiva executada no navegador.
- [x] Usar `src/core/ui.c` como origem de estados e navegação.
- [x] Implementar gerenciador de ativação/desativação de módulos **host**, com verificação de contrato.
- [x] Integrar o exemplo Hello ao menu visual **sem acessar hardware**.
- [x] Criar testes HTTP, CLI e C para navegação, rejeição de entradas e falhas.
- [x] Documentar ambiente, limites e interface.
- [ ] Implementar uma HAL simulada de arquivos/tarefas com falhas injetáveis.
- [ ] Isolar/compilar o motor real de processamento Soup Engine no host com testes determinísticos.
- [ ] Fazer simulação de perfis fotográficos e erros de processamento.
- [ ] Validar o firmware 1.1.1 no corpo 5D e confirmar o primeiro boot do Rebobina.
- [ ] Testar menus, desligamento e módulos na câmera.

## Critérios de entrega

O simulador e seu gerenciador são considerados prontos **somente para o ambiente host** quando: C compila; UI responde a eventos; um módulo demonstrativo ativa/desativa; HTTP rejeita entradas fora do contrato; os testes passam em CI; e a documentação não confunde o navegador com emulação de câmera.

As Sprints 3–7 mantêm critérios de aceite físico originais. O simulador adianta **componentes** dessas sprints, mas não as conclui.

## Quando o leitor CF estiver disponível

1. Fazer backup do cartão e confirmar a versão real na câmera.
2. Atualizar o firmware Canon pelo processo oficial **apenas após verificar o arquivo e ter bateria confiável**.
3. Confirmar versão **1.1.1** antes de qualquer teste do Rebobina.
4. Validar o carregador e o binário-base do experimento de boot, com procedimento de recuperação.
5. Executar o teste físico de boot e coletar logs antes de levar a interface para LCD.

**Não copiar este simulador, executáveis do Linux ou objetos ARM relocáveis para o CF.**
