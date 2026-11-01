# Proposal

## Why

Achado 6 da revisão independente da alpha-1 (Codex, 01/11/2026; ver `docs/DEVELOPMENT_STATE.md`, revisão da alpha-1): o spec principal `title-bar`, requisito "Cliques de mouse pareados", cenário "Clique simples no título", diz que apertar e soltar o título sem passar do limiar "nenhum drag é iniciado". O código faz outra coisa numa janela flutuante, não maximizada e não encaixada: `CHyprBar::onTitlePress` (`plugin/barDeco.cpp`) chama `handleMovement` já no press, que inicia o drag do compositor (`KwDrag::begin`). Lado a lado, maximizada e encaixada esperam 6 px de movimento.

O motivo é do host: o Hyprland arrasta a janela sob o ponteiro, e no press o ponteiro ainda está sobre a própria barra. É também o comportamento da 0.56.2. Um clique parado inicia e encerra o drag na soltura sem mover a janela; o encaixe ao soltar é barrado por < 10 px.

**Decisão do dono (01/11/2026): opção (a), ajustar o spec ao código.** A opção (b), limiar também na flutuante, fica registrada no design.md como alternativa não escolhida.

## What Changes

- Delta MODIFIED no requisito "Cliques de mouse pareados" do `title-bar`: o texto atual é a base; muda só o necessário.
  - O corpo ganha uma frase: na janela flutuante, não maximizada e não encaixada, o drag do compositor começa no press; nas outras, só depois do limiar.
  - O cenário "Clique simples no título" passa a valer para janela lado a lado, maximizada ou encaixada (texto do THEN igual).
  - Cenário novo, "Clique simples no título de janela flutuante": foco e elevação, drag do press à soltura, sem mover nem encaixar a janela.
- Nenhuma mudança de código, teste ou config.

## Capabilities

### New Capabilities

Nenhuma.

### Modified Capabilities

- `title-bar`: "Cliques de mouse pareados".

## Impact

Só documentação OpenSpec. `plugin/`, `core/`, `ui/`, `docs/API.md` e `docs/CONFIGURAVEL.md` não mudam.

Ao arquivar (etapa própria, com aprovação): `tools/test-openspec-specs.sh` compara "Cliques de mouse pareados" do spec principal com o delta da alpha-1 (`enable-titlebar-drag-alpha1`, que renomeou esse requisito). O teste precisa passar a aceitar o texto do delta mais recente para esse requisito, como foi feito para `guard-titlebar-drag-start`.

Fora de escopo (registrados, não iniciados): achados 4 e 5 da revisão (testes antigos da alpha-1); janela em tiling centrada no cursor (nativo do Hyprland, mantido); contador Q1 só por log; `SWindowRenderPresentation`; 73 logs brutos de P5–T6; `floatWindow` do núcleo Lua; custo C1/C2.
