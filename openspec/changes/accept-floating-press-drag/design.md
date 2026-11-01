# Design

## Context

Base conferida em 01/11/2026: branch `port-kwinish-bars-to-hyprland-0-57`, HEAD `9443ea0` (`guard-titlebar-drag-start` arquivada).

Caminho do press no título, lido em `plugin/barDeco.cpp` e `plugin/barInput.hpp`:
- `KwIn::apply` foca a janela e a eleva se for flutuante, e então roda `eAction::FOCUS` → `CHyprBar::onTitlePress`;
- `onTitlePress`: janela flutuante, não maximizada e não encaixada (`!snappedHere`) → `handleMovement()` já no press; senão, `m_bDragPending = true`;
- início adiado (`onMouseMove`): espera `(C - m_pressCursor).size() >= 6`;
- `handleMovement`: porta `startGate`/`passGate` (`guard-titlebar-drag-start`, `5944fe1`), depois `KwDrag::begin(MBIND_MOVE)`; com início, `pairs.dragStarted(BTN_LEFT)` e a soltura vai ao compositor;
- encaixe ao soltar: barrado se o ponteiro andou menos de 10 px desde o press.

Fatos do host, informados pelo dono e não reconferidos aqui: o Hyprland arrasta a janela sob o ponteiro; `binds:drag_threshold` é 0 por padrão e a config do dono não o define; uma tecla apertada com o botão segurado encerra o drag, como qualquer drag do Hyprland.

## Goals / Non-Goals

**Goals:**
- o spec descrever o que o código faz na janela flutuante;
- manter o resto do requisito como está.

**Non-Goals:**
- mudar código ou testes;
- mudar "Arrasto pela barra com fim pelo compositor": ele fala de mover a janela quando o ponteiro passa do limiar, o que continua certo para a flutuante (o início antes do limiar não contradiz o texto);
- o cenário "Press no título vira drag": vale como está para todos os casos em que o drag começa.

## Decisions

### 1. Opção (a): o spec segue o código (escolhida pelo dono em 01/11/2026)

Delta MODIFIED em "Cliques de mouse pareados":
- corpo: frase nova no fim, com o início no press para a flutuante e o limiar para lado a lado, maximizada e encaixada;
- "Clique simples no título": o WHEN passa a nomear janela lado a lado, maximizada ou encaixada; o THEN fica igual;
- cenário novo para a flutuante: clique sem movimento foca e eleva, o drag começa no press e termina na soltura pelo compositor, a janela não muda de posição nem é encaixada.

Custo aceito: como `binds:drag_threshold` é 0, uma tremida de 2–3 px durante o clique move a janela flutuante esses pixels. O spec não promete o contrário: o cenário novo fala de clique sem movimento.

### 2. Alternativa registrada e não escolhida: opção (b), limiar também na flutuante

`onTitlePress` passaria a armar `m_bDragPending` também na flutuante; o drag só começaria depois de 6 px. A tremida some.

Por que não agora:
- o início passaria a depender da porta "ponteiro sobre a própria janela" (`startGate`/`passGate` em `plugin/barInput.hpp`, `5944fe1`): um puxão rápido para fora da barra viraria clique, como já acontece em tiling;
- mexe num caminho que funciona hoje (A1, GS e MX ao vivo passaram) e pediria teste ao vivo.

**Gatilho para trocar:** reclamação de janela flutuante "andando" no clique. A troca exige nova proposta OpenSpec.

## Risks / Trade-offs

- **Tremida move a flutuante** (ver Decisão 1). Custo aceito pelo dono.
- **Tecla com o botão segurado encerra o drag** da flutuante já no press, como qualquer drag do Hyprland. Sem efeito visível num clique; o mesmo vale hoje.
- **Fatos do host não reconferidos nesta proposta** (`drag_threshold`, hit-test do drag): vêm do relato do dono e da revisão. Não mudam o texto, que descreve o observável.

## Migration Plan

1. Aprovação do dono para esta proposta.
2. Commit da proposta, com aprovação.
3. Arquivar (etapa própria, com aprovação): copiar o requisito do delta para o spec principal e ajustar `tools/test-openspec-specs.sh` para aceitar o delta mais recente em "Cliques de mouse pareados" (hoje só aceita o de `guard-titlebar-drag-start`); vermelho antes de arquivar, verde depois, controle negativo.

Voltar é reverter o commit.

## Validation Plan

| O quê | Como |
|---|---|
| Nome do requisito existe no spec principal | conferência estrutural por script fora do repositório (CLI `openspec` ausente) |
| Texto e cenários originais preservados, salvo a mudança pretendida | mesmo script: corpo original é prefixo do corpo novo; cenários iguais, menos o WHEN de "Clique simples no título" |
| SHALL/MUST, WHEN/THEN | mesmo script |
| O script pega erro | controle negativo com cópias quebradas |
| Regressão | `bash tools/check.sh` |

Nada ao vivo: não há mudança de comportamento.

## Open Questions

Nenhuma.
