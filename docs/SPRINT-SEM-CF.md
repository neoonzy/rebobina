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
- [x] Implementar uma HAL simulada de arquivos/memória/tarefas com falhas injetáveis; [contrato e limites](HAL-SIMULADA.md).
- [x] Compilar e executar **os selftests do motor original Soup** no host (checkout GPLv2 fixado, sem copiar para o Rebobina): [CI](https://github.com/neoonzy/rebobina/actions/runs/42228272935).
- [x] Compilar e executar um **teste de integração HAL Rebobina + `se_arena`/`se_rd` reais** com dados sintéticos.
- [x] Gerar um **JPEG sintético 8×8 com `jpeg_enc.c` do Soup**, validar seu cabeçalho e testar escrita interrompida sem sobrescrever origem fictícia ([CI](https://github.com/neoonzy/rebobina/actions/runs/42229454141)).
- [ ] Integrar o motor completo no processo Rebobina, com processamento de CR2/JPEG **reais** e testes comparativos de bytes.
- [x] Aplicar um perfil fotográfico real **Soup 800** a RGB sintético 8×8, salvar JPEG com publicação em duas fases e testar falhas da saída no host ([guia](SAIDA-SEGURA.md)).
- [x] Inspecionar o preview de envelope CR2 **sintético**; extrair, decodificar e desenvolver o JPEG pelo `previewcheck`, preservando entrada ([guia](CR2-INSPECAO-HOST.md)).
- [x] Rodar Soup 800, Portra e Acros com ASan/UBSan, validar saída JPEG idêntica à referência e registrar UB do encoder ([guia](SOUP-SANITIZERS.md)).
- [x] Implementar CLI Linux do preview CR2 → perfil Soup → JPEG com backend POSIX e publicação exclusiva; referência idêntica byte a byte em **CR2 sintético** ([guia](CR2-DESENVOLVIMENTO-HOST.md)).
- [ ] Validar o processamento de uma fotografia **CR2 real da 5D** e integrar os demais perfis sem perder originais.
- [ ] Validar o firmware 1.1.1 no corpo 5D e confirmar o primeiro boot do Rebobina.
- [ ] Testar menus, desligamento e módulos na câmera.

## Executar os testes de host

```sh
make hal-demo
make test
make arm-objects
python3 tools/soup_host.py --fetch
```

O último comando requer acesso à internet e baixa apenas código-fonte Soup Engine de revisão fixada para `build/`. Um adaptador de compatibilidade no arquivo `host/main.c` desse checkout temporário permite compilar o utilitário desktop macOS no Linux; **`engine/` permanece inalterado**. Ao repetir o comando, remova o checkout temporário com `make clean`. Os selftests executam demosaico, LUT, JPEG e parsing corrompido, mas não provam a adaptação na Canon.

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
