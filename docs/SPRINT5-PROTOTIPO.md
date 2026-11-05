# Sprint 5 — Prova de módulos independentes no computador

**Estado:** preparação antecipada da Sprint 5, NÃO implementação embarcada.

O Rebobina agora pode compilar um executável Linux host que carrega um módulo compartilhado **depois** de sua compilação, sem recompilar o executável principal.

```sh
make plugin-demo
make test-plugins
```

O loader usa `dlopen/dlsym` do **Linux** e procura o símbolo `rebobina_plugin_v1`. Valida a versão da API e o contrato antes da inicialização, chama `start`, `stop` e descarrega a biblioteca. Há testes negativos com ABI incompatível e biblioteca sem símbolo.

**Limitações importantes:**

- `.so` é um formato Linux, não um módulo da Canon EOS 5D Classic.
- Uma biblioteca compartilhada executa código nativo com os privilégios do processo, **sem isolamento**. Carregue apenas código produzido localmente e confiável.
- O projeto ainda **não** tem loader ARM, linker/relocations, ABI de câmera, assinatura de módulos ou isolamento de memória.
- O `SE.BIN` do Soup Engine **não é** compatível com esse protótipo.
- O teste no host serve para avaliar o contrato de módulos, não o comportamento em hardware.

A Sprint 5 só poderá ser considerada concluída quando módulos versionados forem carregados e executados na câmera sem recompilar o núcleo, com testes documentados.
