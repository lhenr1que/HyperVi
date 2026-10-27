# Proposal

## Why

O Snap Assist nunca abre quando a janela é encaixada numa zona com Shift ao arrastar. O plugin sempre manda a ação com o monitor do soltar (`zone:N@<monitor>`, `plugin/barDeco.cpp:613`) e o núcleo só chama o Assist quando não vem monitor (`core/kwinish.lua:375`). A intenção era pular só o soltar em **outro** monitor. Achado pela auditoria do Codex (27/10/2026) e conferido no código; explica por que o teste ao vivo do Assist com Shift nunca passou.

## What Changes

- Shift ao arrastar até uma zona do mesmo monitor abre o Snap Assist, como já acontece pelo menu.
- Soltar numa zona de outro monitor continua sem Snap Assist (a janela ainda está no workspace e monitor antigos quando o Assist calcularia as candidatas).

## Capabilities

### New Capabilities
- `snap-assist`: depois de encaixar uma janela numa zona, oferecer as outras janelas flutuantes do mesmo workspace e monitor para a próxima zona vazia.

### Modified Capabilities

## Impact

`core/kwinish.lua` (`window_action`, ramo `zone`), `core/test_kwinish.lua`. Plugin sem mudança (não precisa recarregar).
