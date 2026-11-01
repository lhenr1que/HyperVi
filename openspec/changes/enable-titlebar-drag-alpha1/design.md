# Design

## Context

Base conferida em 01/11/2026: branch `port-kwinish-bars-to-hyprland-0-57`, HEAD `dde3f03`.
- A alpha-0 (D1, T4 `2395356`) compila, passa em `tools/check.sh` e no teste ao vivo do dono (T8, `c9575b5`).
- A T9 (`510d204`) fez o recheck de `target()` em `KwDrag::begin` depois de `DRAG_START`.
- A decisão T10 está em `docs/DEVELOPMENT_STATE.md` (`dde3f03`).

Fontes de projeto:
- `docs/port-0-57/evidence/Q4A1-alt5-contract.md`: §3 variantes, §5 resultados, §6 viabilidade e lista de mudanças, §7 limitações;
- `docs/port-0-57/P8-DRAG.md`;
- `openspec/changes/disable-titlebar-drag-alpha0/design.md`: o inventário fechado da D1 é o que esta mudança religa, menos o toque.

O estudo Q4-A1 é um **modelo** host-free (39 cenários × 6 variantes, réplica manual da camada de entrada do 0.57). Ele não é teste ao vivo e não modela toque com controlador, hit-test nem as ações não auditadas (§7).

## Goals / Non-Goals

**Goals:**
- religar arrasto, encaixe, prévia, resistência, unsnap, rearm e Mover/Redimensionar do menu;
- nenhuma rota do plugin pede fim de drag;
- manter o pareamento de cliques da alpha-0 fora do drag;
- trocar para a paridade 0.56 sem redesenho, se o dono pedir.

**Non-Goals:**
- toque na barra;
- ganchos no Hyprland ou proposta upstream (opcional e separada);
- Q2;
- mudar `hostDrag.hpp` (feito na T9), o grupo R, o `PLUGIN_EXIT` ou formatos persistentes;
- O1 global (ver Open Questions).

## Decisions

### 1. Política de fim: `alt5-rc`

Decisão do dono em 01/11/2026. As rotas do plugin **nunca** pedem fim de drag. Não voltam:
- K3, os 6 `changeMouseBindMode(MBIND_INVALID)` de `bf2049a`, hoje sem equivalente;
- K9, o `mouse` com `0movewindow`;
- nenhum `endDragTarget()` novo.

O fim é sempre o nativo do compositor: a soltura do botão chega ao host sem cancelamento (contrato §6: "não cancelar soltura").

Mudanças exigidas pelo contrato §6, com âncoras históricas de `bf2049a` a localizar por símbolo:

| Parte | Comportamento na alpha-1 |
|---|---|
| K1 (`handleMovement` → `KwDrag::begin`) | Início pelo ajudante. O unsnap acontece **antes** do início. A posse local da barra (`m_bDraggingThis`) só é armada quando `begin` devolve `BEGIN_STARTED` |
| K2 (`luaGrab` → `KwDrag::begin`) | `grabActive` só é armado com `BEGIN_STARTED`. É limpo em qualquer tecla; um clique de grab sem drag ativo é repassado, não engolido |
| Press na barra durante drag ativo | A barra **cede**: com `dragController()->target()` ativo, nem o botão esquerdo nem o direito são tomados (sem menu durante drag alheio) |
| Soltura | Repassada ao host, que encerra. Limpa as flags de drag da barra e o rearm |
| Rearm por Shift (listener) | Armado só na barra em drag e limpo na soltura. O reinício é **só início** (sem fim antes), sem unsnap pendente, e confere que o ponteiro está sobre a janela da barra |
| Encaixe ao soltar, Shift para zona, prévia (`drawSnapPreview`), resistência (`holdAtSeam`/`hkOnMouseMoved`) | Voltam como em `bf2049a`, alimentados pelo estado da barra em drag. O encaixe ao soltar despacha ao núcleo depois de o host receber a soltura |
| `floatWindow` do plugin C++ (`barDeco.cpp`, ação do botão) | Guarda: não chama com `dragController()->target()` ativo |
| Toque | Continua fora (inventário D1, linha "Toque") |

### 2. Pareamento de cliques com o drag

O pareamento da alpha-0 (`KwIn::SPairs`, por botão) continua para botões, ícone e menu. O press no título que **inicia** drag é a exceção: a soltura dele não é tomada pela barra, porque precisa chegar ao host para o fim nativo.
- `KwIn::SClicks::onPress` ganha um resultado de drag, decidido pelo limiar de movimento como em `bf2049a`.
- Esse resultado não arma `pairs.held` para a soltura.
- Um press que não vira drag (clique simples, duplo clique) segue o par atual.

O limite aceito em 31/10/2026 continua: o par é por botão, sem identidade do ponteiro.

### 3. Custos aceitos e limites (contrato §5–§7)

- **C1/C2:** com captura de entrada na soltura, o host não encerra o drag da barra. Ele fica vivo até a próxima tecla (C2) ou o próximo clique não capturado. É regressão frente ao 0.56.2, aceita pelo dono.
- **K2/K3:** teclas que não chegam ao fim nativo durante o grab mantêm o grab até o próximo clique. Já acontecia antes, em todas as variantes.
- O host continua encerrando drags alheios em todo evento não cancelado. É regra nativa, não do plugin.
- Unload com drag da barra ativo: o plugin não encerra o drag. O hook de resistência sai com o HANDLE; o listener de Shift (rearm) é do plugin e precisa ser desconectado no `PLUGIN_EXIT` antes do `dlclose`. Isso fica verificado no roteiro ao vivo.

### 4. Alternativa anotada: paridade 0.56 (`cur-0.56`)

A pedido do dono, a troca precisa ser possível.
- Na paridade, o Kwinish volta a pedir o fim nas rotas K3/K9/rearm de `bf2049a` (com a API 0.57, `endDragTarget()`), e a soltura volta a ser cancelada.
- Ganho: o drag nunca fica preso.
- Custo: no modelo, encerra drag de terceiro em 9–10 cenários (F1, G1, H1i, L1i, M4, M5, RA2, T1, US3, Y1).
- **Gatilho de troca:** drag preso relatado no uso e mais incômodo que o risco de cortar drag alheio.
- Para que a troca toque poucos pontos, os pedidos de fim **ausentes** ficam concentrados num único ponto do código: a soltura da barra e o rearm.
- A troca é mudança de política: exige nova proposta OpenSpec.

## Risks / Trade-offs

- Drag preso sob captura (C1/C2): custo aceito, verificado ao vivo e documentado no STATUS; a alternativa 0.56 está anotada.
- O `floatWindow` do núcleo Lua encerra qualquer drag: ver Open Questions.
- Religar o drag pode quebrar o pareamento da alpha-0. Mitigação: a matriz de pares é reexecutada, e o caso do drag fica separado.
- O modelo não é o binário: o roteiro ao vivo cobre os cenários centrais (drag, Shift, encaixe, prévia, divisa de monitores, menu Mover/Redimensionar, drag alheio por SUPER+arrastar, captura C1/C2, unload durante drag).
- A prévia usa um passe próprio: o contador Q1 (`510d204`) cobre `CPreviewPassElement`.

## Migration Plan

1. Arquivar a D1 (`disable-titlebar-drag-alpha0`) antes desta mudança, porque os deltas aqui removem e renomeiam requisitos que ela adiciona.
2. Pedir aprovação do dono para esta proposta e uma revisão independente, se ele quiser.
3. Implementação com autorização própria, começando por testes vermelhos.
4. Build e check isolados.
5. Roteiro ao vivo do dono.

Voltar para a alpha-0 é reverter o commit da implementação. Ir para a paridade 0.56 exige nova proposta.

## Validation Plan

Testes futuros, host-free salvo indicação:

| Requisito | Vermelho primeiro | Aceite e não-vacuidade |
|---|---|---|
| Sem pedido de fim | inventário estrutural nas fontes compiladas: `MBIND_INVALID`, `endDragTarget`, `0movewindow` | zero ocorrências; mutação que reintroduz qualquer uma falha |
| Início só com `STARTED` | decisões de press/grab com `eBeginResult` simulado | posse/`grabActive` armados só em `BEGIN_STARTED`; mutação que arma em recusa falha |
| Barra cede a drag ativo | press esquerdo e direito com `target()` ativo | nada tomado, sem menu; mutação falha |
| Soltura do drag passa | sequência press-limiar-soltura | soltura não cancelada, flags e rearm limpos; mutação que cancela falha |
| Pareamento fora do drag | matriz da alpha-0 (`tools/test-plugin-input.sh`) | continua verde |
| Rearm | Shift com/sem drag da barra, com unsnap pendente, ponteiro fora | só início, só nas condições do contrato |
| Guarda `floatWindow` C++ | ação do botão com `target()` ativo | não chama; mutação falha |
| Encaixe/Assist | suítes Lua existentes (zona, monitor do soltar, Assist) | verdes; Shift+drag volta a disparar o Assist |
| Menu | teste Rust da árvore | Mover/Redimensionar presentes; mutação que remove falha |
| Toque | inventário estrutural | rotas de toque seguem ausentes |
| Ao vivo (dono) | — | roteiro bash no STATUS com os cenários de Risks, incluindo C1/C2 e unload durante drag |

Mutantes só contam como mortos por assertiva depois de compilar.

## Open Questions

1. ~~`floatWindow` do núcleo Lua~~ **Decidido pelo dono em 01/11/2026: débito registrado (opção a).**
   - Pontos: `core/kwinish.lua` `place`, `window_action` float, `float_for_mode`, `set_workspace_mode` (contrato #11). O Lua não tem `target()`, e essas ações podem encerrar qualquer drag ativo.
   - A alpha-1 não muda o núcleo nem cria API para isso. Motivo: são ações pedidas pelo usuário, e o host já encerra drags em qualquer evento do usuário (PLANO §4).
   - Consequência: a garantia O1 da alpha-1 cobre só as rotas do plugin C++. O débito é documentado em `docs/DEVELOPMENT_STATE.md` e no roteiro ao vivo.
   - Opção descartada por ora: (b), `hl.plugin.kwinish.dragging()` com guarda no núcleo. Pode voltar se o débito aparecer no uso.
2. Revisão independente desta proposta: o dono decide se quer antes de aprovar.
