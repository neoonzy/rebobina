# Segurança e instalação — princípios inegociáveis

**Nenhum binário deste repositório deve ser instalado numa câmera neste estágio.**

## O que o Rebobina não faz hoje

O Rebobina não altera firmware Canon, boot flags, partições, formatação de cartões ou arquivos `AUTOEXEC.BIN`. O utilitário `tools/modules.py` serve exclusivamente para metadados de módulos no diretório de desenvolvimento.

## Limites do boot

O Magic Lantern costuma executar a partir do cartão, mas a ativação do boot flag na câmera é uma mudança persistente e específica da plataforma. Mesmo que nosso carregador futuro faça verificações, ele **não pode recuperar sozinho** situações em que o bootloader Canon não consiga iniciar o executável.

Não confiar num nome de arquivo, assinatura textual ou tamanho isolado para identificar um binário de origem. Quando houver patch de executável, vincular o procedimento a hashes e variantes explicitamente aprovados.

## Requisitos antes de qualquer distribuição embarcada

1. Testes host, verificações automatizadas e simulação passando;
2. Hashes SHA-256 conhecidos do executável-base e documentação da cadeia de build;
3. Arquivos específicos da câmera e versão de firmware, sem misturas;
4. Plano de recuperação e procedimento com backup;
5. Verificação de limites de memória, stack e escrita no cartão;
6. Teste físico documentado em câmera de desenvolvimento;
7. Avisos de risco, licença e atribuição;
8. Não alterar boot flag, firmware ou cartão sem consentimento explícito de quem executa a ferramenta.

## Política de contribuição

- Evitar adicionar firmware Canon, dumps de ROM, CR2 pessoais, chaves, dados sensíveis ou binários proprietários ao repositório;
- Prefira fixtures sintéticas e scripts que indiquem como fornecer dados externos;
- Não publicar imagens de atualização `.FIR`, patches de ROM ou `AUTOEXEC.BIN` modificados sem revisão e validação;
- Toda integração com Magic Lantern e Soup Engine deve manter as obrigações de licenciamento e créditos.

Segurança em nível de sistema não decorre apenas de metadados declarativos: no hardware alvo não há isolamento de processos semelhante ao de um desktop.
