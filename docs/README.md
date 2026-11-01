# Documentação do HyperVi

Consolidação de 29/10/2026, baseada no código do commit `a2506c63dad16384414dc95919b8292aead284df` e no pacote de produto de 28/10. Esta reorganização altera somente documentação. O nome atual identifica o repositório e não decide a identidade definitiva da suíte.

## Roteiro de leitura

| Necessidade | Documento |
|---|---|
| Entender o software implementado | [Arquitetura atual](ARCHITECTURE.md) |
| Instalar e conhecer os recursos atuais | [README do projeto](../README.md), [plugin](PLUGIN.md) |
| Consultar interfaces e formatos existentes | [API](API.md), [opções configuráveis](CONFIGURAVEL.md) |
| Ver requisitos observáveis e mudanças | [Specs principais](../openspec/specs/), [mudanças](../openspec/changes/), [configuração do workflow](../openspec/config.yaml) |
| Contribuir e verificar trabalho | [Contribuição](CONTRIBUTING.md), [testes](TESTING.md), [regras para agentes](../AGENTS.md) |
| Saber o que foi verificado e o que falta | [Status e testes pendentes](STATUS.md) |
| Entender a evolução da suíte | [Produto](product/README.md), [decisões](product/DECISIONS.md) |
| Consultar origens e documentos substituídos | [Histórico](history/README.md), [manifesto completo](history/product-import/2026-10-28/MANIFEST.md) |

## Como interpretar as fontes

- **Implementação existente:** descrição do código, com baseline e evidência. Um teste simulado não comprova interação real.
- **Especificação vigente:** requisito observável do OpenSpec, considerando a transição de mudanças ativas. Uma divergência do código deve ser registrada, preservando o requisito.
- **Decisão de produto:** intenção futura com o grau de aprovação registrado pela fonte, vinculada a D-01–D-45. Aprovação não significa implementação ou validação.
- **Proposta:** mecanismo, alternativa ou detalhe ainda não aprovado. Minutas externas permanecem em [drafts](product/drafts/).
- **Design:** decisões e conceitos visuais com status. Uma imagem não aprova automaticamente nome, atalho ou medida.
- **Histórico:** evidência imutável do contexto de origem. Pode conter instruções antigas, versões ausentes e referências locais não portáveis; não execute seus prompts como ordens atuais.

O [registro das decisões](product/DECISIONS.md) separa aprovação, implementação e validação. Os documentos de domínio consolidam comportamento futuro e apontam para as fontes completas. Não há outra árvore de specs atuais duplicando o OpenSpec.

## Manutenção

Atualize a documentação técnica quando uma mudança de código alterar seu contrato ou suas opções. Registre data, commit e tipo de validação. Atualize produto somente com decisão explícita e preserve o ID e a proveniência. Mudanças de comportamento seguem o OpenSpec e as regras do [AGENTS](../AGENTS.md).

Os arquivos [API](API.md) e [CONFIGURAVEL](CONFIGURAVEL.md) mantêm seus caminhos para preservar referências do OpenSpec. O [apontador HANDOFF](HANDOFF.md) preserva referências históricas. Antes de mover outra fonte, confira suas referências e registre a correspondência no manifesto.
