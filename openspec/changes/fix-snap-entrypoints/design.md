# Design

## Context

`M.tile(where, opts)` já trata `opts` ausente em `target(opts)` (usa a janela ativa), mas a linha seguinte lê `opts.monitor` direto. `window_action` sempre passa `opts`; os binds não.

## Goals / Non-Goals

**Goals:**
- Corrigir só o ponto de entrada sem opções.

**Non-Goals:**
- Mudar binds, plugin ou as outras funções de encaixe (`maximize_axis`, `pack` não leem `opts.*` sem guarda; conferido).

## Decisions

- Normalizar `opts = opts or {}` no início de `M.tile`, como `M.setup` já faz. Alternativa: os binds passarem `{}`; rejeitada porque deixaria a armadilha para quem chamar `kw.tile("left")` na própria config (a receita documenta esse uso).

## Risks / Trade-offs

- [O teste de binds substitui as ações e não pegou o erro] → o teste novo chama `kw.tile` de verdade no simulador de `hl`.
