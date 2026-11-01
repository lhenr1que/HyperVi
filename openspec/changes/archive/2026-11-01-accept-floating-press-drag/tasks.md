# Tasks

Estado em 01/11/2026: proposta **aprovada pelo dono**, commitada em `4abcb1e` e arquivada (o dono mandou prosseguir). Só planejamento: nenhuma mudança de código.

## 1. Proposta

- [x] 1.1 Decisão do dono registrada: opção (a), spec segue o código (01/11/2026).
- [x] 1.2 Delta MODIFIED de "Cliques de mouse pareados" em `specs/title-bar/spec.md`, com o texto atual como base.
- [x] 1.3 Alternativa (b) e gatilho no design.md e no `docs/DEVELOPMENT_STATE.md`.
- [x] 1.4 Conferência estrutural por script fora do repositório, com controle negativo; `bash tools/check.sh`.
- [x] 1.5 Aprovação do dono para a proposta (01/11/2026).
- [x] 1.6 Commit, só com aprovação do dono (aprovado em 01/11/2026).

## 2. Arquivar (etapa própria, com aprovação)

- [x] 2.1 `tools/test-openspec-specs.sh` passa a aceitar o delta mais recente em "Cliques de mouse pareados" (hoje compara com o da alpha-1); vermelho antes de arquivar.
- [x] 2.2 Mover para `openspec/changes/archive/2026-11-01-accept-floating-press-drag/`; copiar o requisito do delta para `openspec/specs/title-bar/spec.md` como está.
- [x] 2.3 Teste verde, controle negativo pegando uma quebra no texto novo; `bash tools/check.sh`.
- [x] 2.4 Nota de arquivamento neste arquivo; `docs/DEVELOPMENT_STATE.md`.

## Arquivamento (01/11/2026)

Arquivada à mão (CLI `openspec` não instalada) depois de o dono mandar prosseguir em 01/11/2026. O requisito MODIFIED "Cliques de mouse pareados" substituiu o do spec principal `title-bar`, com o texto copiado como está; nenhum outro requisito e nenhum código mudaram. `tools/test-openspec-specs.sh` passou a aceitar uma lista de mudanças posteriores à alpha-1 (`guard-titlebar-drag-start`, depois esta), valendo o texto mais recente: vermelho antes de mover a pasta (3 falhas) e depois de mover sem copiar o texto (2 falhas), verde depois; o controle negativo ganhou uma quebra no texto novo e a pega. Nada ao vivo: não há mudança de comportamento.
