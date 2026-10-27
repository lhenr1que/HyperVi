# Tasks

## 1. Teste que falha

- [x] 1.1 Em `core/test_kwinish.lua`, com o layout `thirds` e duas janelas flutuantes, chamar `window_action(addr, "zone:1@<monitor da janela>")` e conferir que o Assist foi pedido (`exec_cmd` com `kwinish-ui assist`); verificar que falha hoje (`lua5.4 core/test_kwinish.lua`)
- [x] 1.2 No mesmo teste, `zone:1@<outro monitor>` encaixa no outro monitor e não pede o Assist

## 2. Conserto

- [x] 2.1 Ajustar a condição do ramo `zone` em `window_action` (`core/kwinish.lua`); verificar com `for t in core/test_*.lua; do lua5.4 $t; done` (todas passam)

## 3. Registro

- [x] 3.1 Atualizar em `docs/STATUS.md` o teste 1 do dono (Snap Assist com Shift ao arrastar) dizendo que agora deve abrir, e o pendente do HANDOFF ("sem Snap Assist nesse caso")

## Workflow follow-up

- Arquivar com `/opsx:archive` (ou `$openspec-archive-change` no Codex) depois do teste do dono.
