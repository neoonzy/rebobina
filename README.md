# Rebobina

**Uma plataforma aberta e modular para dar novas possibilidades a câmeras digitais clássicas.**

O Rebobina é um projeto experimental de software embarcado inspirado no [Magic Lantern](https://magiclantern.fm), com foco inicial na **Canon EOS 5D Classic (firmware 1.1.1)**. Queremos facilitar a instalação e a integração de extensões, começando pelo [Soup Engine](https://github.com/neoonzy/soup-engine), sem exigir que cada extensão substitua o executável principal da câmera.

> [!IMPORTANT]
> **Estado: protótipo de arquitetura no computador.** O Rebobina **ainda não funciona na câmera**, não instala Magic Lantern, não modifica firmware/boot flags e não gera um `AUTOEXEC.BIN` utilizável. Nenhum binário do Soup Engine está integrado ou homologado. Não copie os executáveis deste repositório para o CompactFlash.

## O que queremos construir

- **Núcleo pequeno:** inicialização controlada, gestão de módulos, registro de eventos e serviços compartilhados.
- **Módulos independentes:** contrato de API, compatibilidade por câmera/firmware, inicialização e desligamento separados, sem patches individuais no executável principal.
- **Integração por câmera:** uma camada específica para recursos do hardware, começando na Canon 5D Classic.
- **Instalação assistida:** futuramente, uma ferramenta de computador para validar arquivos, criar backups, preparar cartões e orientar a recuperação.
- **Desenvolvimento testável:** grande parte da lógica executável e verificável no computador antes de testes físicos.

O **Magic Lantern já possui uma arquitetura modular** em parte de seus modelos. O Rebobina não pretende atribuir a si essa ideia, mas estudá-la e aperfeiçoar sobretudo o fluxo de integração e distribuição para a 5D Classic.

## O que já funciona nesta versão inicial

- Um **protótipo do registro de módulos em C**, executado no computador;
- Contrato inicial de API com verificação de versão, identificador e ciclo de vida;
- Exemplo `hello`, que demonstra inicialização e encerramento de módulo;
- Manifestos JSON validados por ferramenta Python, incluindo um *placeholder* do Soup Engine;
- Testes do registro e dos manifestos, com CI no GitHub Actions.

**Ainda não há carregamento dinâmico, menu na câmera, suporte a `.mo`, isolamento de módulos, instalação ou integração funcional com o Soup Engine.** Os manifestos descrevem intenções e compatibilidade planejada, não permissões efetivamente impostas.

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
