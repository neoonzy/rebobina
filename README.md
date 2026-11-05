# Rebobina

**Um novo software embarcado, livre e modular para expandir as possibilidades de câmeras digitais clássicas.**

O **Rebobina será um programa próprio, executado na câmera**, desenvolvido a partir do conhecimento, dos mecanismos e, quando viável e legalmente compatível, de componentes de código aberto do [Magic Lantern](https://magiclantern.fm). O foco inicial é a **Canon EOS 5D Classic (firmware 1.1.1)**. O objetivo é oferecer uma alternativa modular ao Magic Lantern para esse modelo: interface própria, recursos fotográficos próprios e um sistema que permita adicionar extensões como o [Soup Engine](https://github.com/neoonzy/soup-engine) sem reescrever o executável principal a cada módulo.

**O produto principal não é um instalador do Magic Lantern nem um gerenciador de plugins para o Magic Lantern existente.** O Rebobina pretende executar no lugar da extensão Magic Lantern no cartão, preservando o firmware Canon na ROM. Um eventual instalador será somente uma ferramenta auxiliar. A decisão sobre quais partes do Magic Lantern serão reutilizadas ou adaptadas depende da pesquisa da implementação para 5D Classic.

> [!IMPORTANT]
> **Estado: protótipo de arquitetura no computador.** O Rebobina **ainda não funciona na câmera**, não instala Magic Lantern, não modifica firmware/boot flags e não gera um `AUTOEXEC.BIN` utilizável. Nenhum binário do Soup Engine está integrado ou homologado. Não copie os executáveis deste repositório para o CompactFlash.

## O que queremos construir

- **Programa embarcado completo:** inicialização a partir do CompactFlash, ciclo de vida de tarefas, interface própria e ferramentas fotográficas integradas ao firmware Canon em execução.
- **Núcleo modular:** carregamento, ativação e desativação de recursos por uma API versionada, com gerenciamento de memória e erros.
- **Sistema de extensões:** recursos como Soup Engine instaláveis e atualizáveis sem aplicar patches individuais no executável principal (meta futura, ainda não implementada).
- **Camada por câmera:** abstração de serviços específicos do hardware, começando pela Canon 5D Classic.
- **Ferramentas de desenvolvimento:** simulador, testes e validação automatizada para reduzir os riscos de usar código novo no equipamento.
- **Instalação assistida (secundária):** um utilitário para facilitar a preparação e recuperação do cartão quando existir uma versão embarcada validada.

O Magic Lantern já possui um sistema de módulos em várias versões. O Rebobina vai estudar e reutilizar esse legado quando fizer sentido, mantendo atribuição e GPLv2, mas terá **produto, interface, organização e evolução próprios**. Não é necessário reescrever do zero o que já foi resolvido pela comunidade.

## O que já funciona nesta versão inicial

- Um **protótipo do registro de módulos em C**, executado no computador;
- Contrato inicial de API com verificação de versão, identificador e ciclo de vida;
- Exemplo `hello`, que demonstra inicialização e encerramento de módulo;
- Manifestos JSON validados por ferramenta Python, incluindo um *placeholder* do Soup Engine;
- Testes do registro e dos manifestos, com CI no GitHub Actions.

**Ainda não existe o executável embarcado do Rebobina: não há inicialização própria na câmera, carregamento dinâmico, menu na câmera, suporte a `.mo`, isolamento de módulos, instalação ou integração funcional com o Soup Engine.** Os manifestos descrevem intenções e compatibilidade planejada, não permissões efetivamente impostas.

## Estrutura

```text
include/rebobina/     Contrato inicial de módulos em C
src/core/             Registro e ciclo de vida (protótipo host)
src/host/             Executável demonstrativo no computador
modules/hello/        Módulo de exemplo executável no host
modules/soup-engine/  Manifesto de integração planejada
tools/modules.py      Validação e criação de manifestos
tests/                Testes automatizados
docs/                 Arquitetura, segurança e roteiro
```

## Desenvolvimento atual e próximo marco

O protótipo host foi criado para testar a API de módulos. **Ele não é o produto final**. A próxima prioridade é analisar a inicialização do Magic Lantern na 5D e criar a primeira versão embarcada mínima do Rebobina, primeiro em ambiente de testes e depois em câmera de desenvolvimento. Um instalador desktop não é requisito para iniciar essa etapa.

## Primeiros comandos

Requisitos: compilador C11, `make` e Python 3.9 ou superior. Os comandos **não acessam a câmera**.

```sh
make demo          # compila e executa o protótipo C no computador
make test          # testa o registro C e a ferramenta Python
make check-modules # valida manifestos
python3 tools/modules.py list
python3 tools/modules.py new minha-extensao --name "Minha extensão"
```

O último comando cria um manifesto e um README de rascunho para uma nova extensão; **não cria um binário de câmera**.

## Primeiro dispositivo

| Campo | Alvo inicial |
| --- | --- |
| Câmera | Canon EOS 5D Classic (2005), não Mark II/III/IV |
| Firmware Canon | 1.1.1 |
| Sistema embarcado | VxWorks / DIGIC II |
| Referência de integração | Magic Lantern específico da EOS 5D Classic |
| Primeiro módulo planejado | Soup Engine |

Não há promessa de compatibilidade com outros modelos. Cada adaptação futura precisará de pesquisa, validação e testes próprios.

## Documentação

- [Arquitetura proposta](docs/ARQUITETURA.md)
- [Contrato e criação de módulos](docs/MODULOS.md)
- [Segurança, boot e limites](docs/SEGURANCA.md)
- [Roadmap](docs/ROADMAP.md)
- [Como contribuir](CONTRIBUTING.md)
- [Orientações para agentes e colaboradores](AGENTS.md)

## Créditos e licença

O Rebobina não é afiliado à Canon nem à equipe Magic Lantern. Reconhecemos o trabalho da comunidade do [Magic Lantern](https://github.com/reticulatedpines/magiclantern_simplified) e dos autores do [Soup Engine](https://github.com/jiijii0/soup-engine). **Nenhum código desses projetos foi incorporado nesta primeira estrutura**, exceto a cópia do texto da licença GNU GPL v2. Integrações futuras deverão respeitar direitos autorais, atribuições e licenças dos componentes reutilizados.

Código original do Rebobina licenciado sob **GPL-2.0-only** — veja [LICENSE](LICENSE). Projeto experimental, **sem garantias**.
