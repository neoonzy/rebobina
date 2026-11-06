# Criando módulos

## Estado atual

O Rebobina possui duas coisas distintas:

- **Contrato experimental C**, em `include/rebobina/module.h`, demonstrado no host com `modules/hello/hello.c`;
- **Manifestos JSON**, validados por `tools/modules.py`. Eles descrevem extensões, **não instalam nem executam plugins na câmera**.

## Manifesto

Cada módulo vive em `modules/<id>/module.json`. Exemplo:

```json
{
  "id": "exemplo",
  "name": "Exemplo",
  "version": "0.1.0",
  "api_version": 1,
  "status": "planned",
  "platforms": ["5DC.111"],
  "capabilities": ["camera-ui"],
  "description": "Funcionalidade ainda não implementada"
}
```

`status` é um de `planned`, `prototype` ou `camera-tested`. A última opção somente poderá ser usada após teste em hardware documentado; o validador não tem como comprovar essa alegação.

`platforms`: valores admitidos inicialmente: `host` e `5DC.111`. **A presença da plataforma no manifesto não atesta funcionamento.**

`capabilities` é uma lista informativa de capacidades pretendidas; não confere acesso a arquivos, memória ou chamadas de hardware.

## CLI de desenvolvimento

```sh
python3 tools/modules.py list
python3 tools/modules.py check
python3 tools/modules.py new meu-modulo --name "Meu módulo"
```

O comando `new` escreve apenas arquivos de texto em `modules/`; aborta se o diretório já existir ou se o id for inválido. Não gera arquivos para CF.

## Contrato C — rascunho

Um módulo define `rb_module` com `abi_version`, `id`, `name`, `start` e `stop`. O host fornece `rb_host_api` com logger. O registro verifica versão, duplicações e limite; em caso de falha de inicialização, desfaz os módulos já iniciados.

O **primeiro módulo real** será a adaptação do Soup Engine ao contrato escolhido. Isso exige integração de código e revisão de dependências de hardware; não basta renomear `SE.BIN` para `.mo`.

## Prova de módulos independentes no Linux host

Há também um protótipo de carregamento dinâmico de bibliotecas `.so` no **Linux host**, separado da compilação ARM:

```sh
make plugin-demo
make test-plugins
```

O executável `build/rebobina-plugin-host` carrega `build/plugins/hello.so` por `dlopen`, encontra `rebobina_plugin_v1` e executa `start/stop`. Isso permite substituir o plugin **sem recompilar o loader host**, mas não prova que o mesmo mecanismo é viável na Canon. Há testes negativos para versão incompatível e exportação ausente.

**Aviso:** carregar `.so` executa código nativo com as permissões do processo, sem sandbox. Não carregue plugins de origem desconhecida. **`.so` não é o formato de módulo da 5D e `SE.BIN` não é compatível.**

Consulte [o relatório do protótipo](SPRINT5-PROTOTIPO.md).

## Gerenciamento individual no host (novo)

`include/rebobina/manager.h` define `rb_manager`, que reutiliza o validador `rb_registry_add` e mantém um vetor de módulos ativos. Operações: `rb_manager_add`, `rb_manager_enable`, `rb_manager_disable`, `rb_manager_is_active`, `rb_manager_stop_all`.

Os testes verificam ativação individual, rejeição de ativação duplicada, erro de ABI/índice, falha de `start`, bloqueio de registro durante execução e desligamento em ordem reversa. **Módulos que falham em `start` são responsáveis por limpar recursos que tenham adquirido antes de retornar erro.** Não há isolamento de memória ou execução separada.

O [simulador visual](SIMULADOR-VISUAL.md) exercita `rb_manager` com um Hello ligado estaticamente à ponte C: **não** carrega bibliotecas arbitrárias pela interface web, e a tela de Soup Engine informa que o motor não foi integrado.

## Restrições da primeira etapa

- Sem acesso a hardware;
- Sem carregamento dinâmico de binários **na câmera** (há uma prova `.so` somente Linux host);
- Sem ABI estável garantida entre versões;
- Sem permissões aplicadas por sistema operacional;
- Sem mecanismo de assinatura ou isolamento de código executável.

Não distribuir um módulo como compatível com a câmera apenas porque o `check` passou.
