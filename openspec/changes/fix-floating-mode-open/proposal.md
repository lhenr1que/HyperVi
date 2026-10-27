# Proposal

## Why

Num workspace no modo floating, uma janela nova aparece primeiro **em mosaico**, ocupando toda a área útil (como a única janela do dwindle), e só depois vira flutuante no retângulo do modo. Relatado pelo dono em 27/10/2026 (workspaces 1 e 2 em floating) e medido ao vivo no mesmo dia, com um kitty descartável aberto em `workspace 3 silent`: aos 105 ms a janela já estava mapeada e em mosaico com 1827x1000; aos 267 ms, flutuante com 1100x682, centralizada. A causa está no código: o núcleo só faz a janela flutuar num temporizador de 150 ms disparado por `hl.on("window.open")` (`core/kwinish.lua`, fim do arquivo → `apply_workspace_mode` → `float_for_mode`), depois de o Hyprland já ter posto a janela no layout de mosaico. Com a animação de abertura, o salto de tela cheia para o retângulo flutuante fica bem visível. É o item "janela nova sem piscar" do backlog do `docs/STATUS.md`.

## What Changes

- Num workspace no modo floating, a janela nova nasce flutuante: o primeiro quadro em que aparece já é flutuante, e ela nunca passa pelo tamanho de mosaico.
- O retângulo final continua o de hoje: o retângulo salvo na etiqueta `kwinish-rect-*` quando existe; senão 64% x 68% da área útil (no máximo 1100 x 760), centralizado, com o passo de cascata (`modes.open_rect`). A etiqueta `kwinish-mode-floated` continua marcando as janelas que o modo fez flutuar, para que sair do modo volte só essas ao mosaico.
- Sem mudança para janelas que já nascem flutuantes (diálogos), em tela cheia ou fixadas: continuam sem a etiqueta, como hoje.
- Sem mudança para janela levada de outro workspace (`window.move_to_workspace`) nem para os modos tiling e scrolling.
- O mecanismo (evento `window.open_early` do Hyprland 0.56 ou regra de janela por workspace) é escolhido por um experimento num Hyprland aninhado, nunca na sessão do dono: ver `design.md`.

## Capabilities

### New Capabilities
- `workspace-modes`: modo de cada workspace (tiling, scrolling, floating), escolhido pelo usuário e lembrado entre sessões, e como as janelas entram e saem de cada modo.

### Modified Capabilities

## Impact

`core/kwinish.lua` (abertura de janela no modo floating: `float_for_mode`, `apply_workspace_mode`, tratadores de `window.open`/`window.open_early`; `set_workspace_mode` se o mecanismo for por regra), `core/test_kwinish.lua` (simulador `hl` com o evento antecipado ou com regras). Formato de `workspace-modes.json` e menu sem mudança; `docs/CONFIGURAVEL.md` só se o comportamento visível do modo for descrito lá. Plugin e UI sem mudança. Verificação ao vivo exige carregar o núcleo do repositório (`fish tools/dev-reload.fish`), feita pelo dono.
