# Proposal

## Why

Os atalhos de encaixe pelo teclado (ALT+setas, ALT+Q/W/A/S, ALT+Enter, de `core/binds.lua`) não encaixam: `M.tile(where)` é chamado sem a tabela de opções e lê `opts.monitor`, o que gera erro de Lua em `nil`. Achado pela auditoria do Codex (27/10/2026) e conferido no código (`core/kwinish.lua:130`).

## What Changes

- Encaixar pelo teclado passa a funcionar na janela ativa, no monitor dela.
- O encaixe pelo menu e pelo arrasto continua como está, inclusive o monitor do soltar (`tile:left@DP-1`).
- Um teste do núcleo cobre a chamada sem opções, que o teste de binds não pegava (ele substitui as ações).

## Capabilities

### New Capabilities
- `window-snapping`: encaixe de janelas flutuantes em metades, quartos e tela inteira, pelo teclado, pelo menu e pelo arrasto da barra.

### Modified Capabilities

## Impact

`core/kwinish.lua` (`M.tile`), `core/test_kwinish.lua`. Sem mudança de formato de arquivo, de opção ou do plugin.
