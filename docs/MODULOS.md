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

## Restrições da primeira etapa

- Sem acesso a hardware;
- Sem carregamento dinâmico de binários;
- Sem ABI estável garantida entre versões;
- Sem permissões aplicadas por sistema operacional;
- Sem mecanismo de assinatura ou isolamento de código executável.

Não distribuir um módulo como compatível com a câmera apenas porque o `check` passou.
