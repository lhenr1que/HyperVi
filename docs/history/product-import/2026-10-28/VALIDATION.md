# Validação da consolidação documental

Concluída em 29/10/2026. Baseline: `a2506c63dad16384414dc95919b8292aead284df`. Escopo autorizado: documentação, proveniência, índices e revisão. Não houve commit, mudança de shell, implementação, atualização do sistema ou migração do Hyprland.

## Resultado

| Verificação executada | Resultado |
|---|---|
| Origens do manifesto | 60: 28 documentos do repositório, ZIP original e 31 entradas externas |
| Entradas preservadas | 28/28 no arquivo do repositório; 31/31 no pacote e na extração |
| SHA-256 de arquivos preservados | 65/65 conferem; ZIP da raiz, cópia e conteúdo original íntegros |
| Destinos e inventário de saída | Todos existem; 94 arquivos documentais atuais inventariados |
| Documentos consolidados com hash | Todos os hashes registrados no manifesto conferem |
| Markdown e referências | 99 arquivos físicos + 28 contextos do arquivo original; 659 links relativos conferidos |
| Erros de links na documentação ativa | 0 |
| Referência histórica ausente | 1, descrita abaixo e preservada sem alteração |
| Decisões D-01–D-45 | 45 IDs/âncoras únicos; campos de decisão, aprovação, implementação, validação e origem completos |
| Cenários futuros | 46 SD-D43/D44/D45 e 18 ST-* conservam o conjunto de IDs das fontes; todos planejados/não executados |
| Regras operacionais AGENTS | Trecho desde `## Mudanças` idêntico byte a byte ao original |
| CLAUDE | Importação `@AGENTS.md` preservada |
| Arquivos rastreados protegidos | 342/342 inalterados por SHA-256 |
| OpenSpec vigente, ativo e arquivado | 36/36 arquivos inalterados por SHA-256 |
| Preexistentes não rastreados | 7/7 preservados por SHA-256 |
| Worktrees | 1403 entradas conservam caminhos, modo, tamanho, mtime e alvo de symlink |
| HEAD e index | Inalterados; nada staged, nenhum commit |
| Escopo | Somente documentos originais alterados/movidos e arquivos novos em `docs/` |
| `git diff --check` | Passou, saída vazia |

Dados detalhados: [VALIDATION.json](VALIDATION.json), [SOURCES.json](SOURCES.json), [MANIFEST.json](MANIFEST.json), [SHA256SUMS](SHA256SUMS) e [PROTECTED.json](PROTECTED.json). [Comandos de integridade](README.md#verificar-integridade).

## Revisão semântica

Uma revisão independente somente leitura comparou D-01–D-41 literalmente com as tabelas da visão, normalizando apenas Markdown/espaços: 41/41 correspondem. D-42–D-45 foram conferidas contra os adendos finais. Nuances D-14/D-17/D-24/D-29/D-33/D-36/D-37 foram preservadas; não houve ampliação identificada.

A revisão encontrou a generalização incorreta de que todos os arquivos eram JSON. API e CONFIGURAVEL agora distinguem explicitamente o arquivo `layout`, texto de uma linha conforme `load_layout()`/`set_layout()` em `core/kwinish.lua`. Também foram reconciliados baseline, atalhos, `settings`, tags de manter acima/abaixo e relatos de instalação/testes, sem alterar código.

## Divergências e limitações preservadas

1. O [Design System original](external-originals/docs/product/design/DESIGN_SYSTEM_INTERFACE_OFICIAL_v1.8.md), linha 922, referencia `MOCKUPS_COMPARACAO_BARRAS_v0.1.png`, imagem ausente do pacote. O link original foi conservado; DESIGN explica a seleção incompleta de imagens.
2. Versões em nomes/cabeçalhos e referências a versões ausentes continuam no acervo. Não há fontes editáveis de mockups nem transcrição integral da conversa externa.
3. P-11 designa propostas distintas; F-01/F-1 e cenários SD-42 têm significados/oráculos diferentes conforme arquivo/seção. A leitura consolidada usa proveniência para desambiguar, sem renumerar originais.
4. Uma frase da dock sobre favoritos por workspace diverge da leitura consolidada D-13/favoritos globais. INTERFACE registra a precedência, preservando a frase de origem.
5. A minuta externa usa baseline `60e19df` e classifica show-desktop como capacidade modificada sem existir spec-base. Permanece rascunho fora de `openspec/`; não foi corrigida nem aplicada.
6. Cascata Floating, procedência ao minimizar/ocultar, perda de `stashed`, hooks/timers, revalidação de contexto e testes ao vivo pendentes continuam como riscos técnicos; a reorganização não os resolve.
7. Compatibilidade Hyprland 0.57 não foi validada. Pareceres e prompts de atualização continuam históricos; não houve upgrade, rebuild ou reload.

Originais mantêm seus contextos e referências privadas/locais como dados de origem. A validação de links considera Markdown inline, imagens, referências definidas e âncoras, ignorando blocos e trechos de código. Inclui os 28 contextos dentro do arquivo original e lista separadamente problemas das fontes congeladas. Não testou disponibilidade HTTP nem transforma caminhos exemplificativos/runtime em dependências do repositório.

## Git e diffs

Estado completo: [GIT-STATUS.txt](GIT-STATUS.txt). Resumo: [DIFF-SUMMARY.txt](DIFF-SUMMARY.txt).

- 9 arquivos rastreados modificados, exclusivamente documentação.
- 17 caminhos antigos removidos da árvore ativa; todos possuem destino histórico byte a byte no manifesto.
- 92 não rastreados: 7 preexistentes e 85 arquivos documentais novos.
- 18 documentos históricos foram movidos sem alterar bytes. HANDOFF conserva um apontador no caminho antigo para referências de OpenSpec; os outros 17 aparecem como `D` até eventual staging.
- O diff rastreado isolado não inclui os arquivos novos. Eles incluem 61 Markdown, 8768 linhas, além de acervo, imagens, ZIPs e metadados; parte corresponde aos originais movidos.
- Nada foi staged ou commitado. Worktrees e arquivos não rastreados preexistentes não foram limpos.

## Árvore de leitura final

```text
README.md / AGENTS.md / CLAUDE.md
docs/
  README.md / ARCHITECTURE.md / CONTRIBUTING.md
  API.md / CONFIGURAVEL.md / PLUGIN.md / TESTING.md / STATUS.md
  HANDOFF.md                     # apontador de compatibilidade
  product/
    README.md / DECISIONS.md / INTERFACE.md / SETTINGS.md
    WORKSPACES.md / DESIGN.md / PROPOSALS.md
    mockups/                     # seis PNG originais
    drafts/README.md
    drafts/stabilize-show-desktop-restoration/  # cinco arquivos intactos
  history/
    README.md
    initial-research/            # BETA, BRIEF, PLANO, SPIKE originais
    handoffs/HANDOFF.md
    reference/ideias/ e reference/referencia/
    audits/2026-10-28/README.md
    product-import/2026-10-28/   # fontes, manifesto, hashes e esta revisão
openspec/                        # 36 arquivos intactos
```

O [manifesto legível](MANIFEST.md) e seu JSON relacionam individualmente cada origem e destino. Os [originais do repositório](repository-originals.zip), o [pacote preservado](Kwinish_Product_Docs_2026-10-28.zip) e suas [entradas](external-originals/) permanecem recuperáveis.

## Limite da validação

Foram executadas verificações documentais e revisão semântica. Não foram executados testes de software, builds, instaladores, comandos do compositor, ferramentas de atualização ou testes ao vivo. Resultados de testes de 24–28/10 foram preservados como evidência histórica. Nenhum shell, toolkit, fork ou identidade definitiva foi escolhido.
