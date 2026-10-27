# Tasks

## 1. Teste que falha

- [x] 1.1 Em `core/test_kwinish.lua`, chamar `kw.tile("left")` sem opções com uma janela flutuante ativa e conferir o retângulo; verificar que falha hoje com `attempt to index a nil value` (`lua5.4 core/test_kwinish.lua`)
- [x] 1.2 No mesmo teste, chamar de novo e conferir que a janela volta ao retângulo anterior

## 2. Conserto

- [x] 2.1 Normalizar `opts` em `M.tile` (`core/kwinish.lua`); verificar com `for t in core/test_*.lua; do lua5.4 $t; done` (todas passam)

## 3. Registro

- [x] 3.1 Anotar em `docs/STATUS.md` o conserto e o teste do dono: ALT+Esquerda numa janela flutuante encaixa na metade esquerda

## Workflow follow-up

- Arquivar com `/opsx:archive` (ou `$openspec-archive-change` no Codex) depois do teste do dono.
