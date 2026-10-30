# T2 (grupo W): investigação de equivalência (30/10/2026)

Primeira passada (só leitura) parou no item 1 por instrução ("sem equivalente exato → PARE e apresente opções"). **Decisão do dono (30/10/2026): opção A** (valor ao vivo; D1/D2 abaixo aceitas como diferença registrada). Implementação e resultados: seção "Implementação" no fim.

Fontes:
- **O** = 0.56.2: `/home/lhen/.hermes/cache/scratch/kwinish-p5/old/src0562/src/` (mesma árvore usada por P5).
- **U** = 0.57: `/home/lhen/Projects/kwinish-plan-hyprland-0.57/investigacao-b1-b3/evidence/upstream/Hyprland/src/` (rev. `0aa6745`). `WindowPresentation.hpp` e `WindowBackend.hpp` idênticos (`cmp`) aos instalados em `/usr/include/hyprland/src` (pacote `hyprland 0.57.0-1`).

## Item 1 — `m_X11DoesntWantBorders` (`main.cpp:40`): SEM equivalente exato

| | 0.56.2 | 0.57 |
|---|---|---|
| Predicado | `XWaylandManager.cpp:149–165` `checkBorders`: só X11; átomo em {POPUP_MENU, NOTIFICATION, DROPDOWN_MENU, COMBO, MENU, SPLASH, TOOLTIP} **ou** `isX11OverrideRedirect()` | `X11Backend.cpp:65–69,80,98` `traitsFrom`: mesma lista de 7 átomos **ou** `m_overrideRedirect`; Wayland: `suggestsNoBorder` fica no padrão `false` (`WindowBackend.hpp:70`; `WaylandBackend.cpp:271–275` não o define) |
| Quando é calculado | uma vez por mapeamento, `Window.cpp:2124`, antes de `window.open` (`:2433`) | ao vivo a cada `backend().traits()` (`X11Backend.cpp:292–296`; cache `m_traits` só sem superfície) |
| Pode voltar a `false`? | **não**: os dois únicos escritores (`XWaylandManager.cpp:158,164`) gravam `true`; o `CWindow` X11 sobrevive a unmap/remap (ouvintes `Window.cpp:196–197`, só zerados no destroy `:2794–2796`) | sim (estado atual da superfície) |

**Correção do P5** (`P5-WINDOW-API.md:39`): `suggestsNoBorder` **não** é superconjunto; o 0.56.2 também incluía override-redirect. É a mesma expressão sobre o estado corrente da superfície; difere só pelo histórico pegajoso abaixo.

**Divergência real (pegajosidade):**
- D1: janela X11 que num mapeamento anterior teve átomo sem-borda/override-redirect e é remapeada como janela normal → 0.56.2 sem barra; 0.57 (ao vivo) com barra.
- D2: no laço de `pluginInit` (janelas já mapeadas), janela cujo tipo mudou depois do mapeamento → 0.56.2 usa o valor do mapeamento; 0.57 o atual.
- Fato de contexto: o próprio host 0.57 passou a usar o valor ao vivo para borda/sombra/animação (`Renderer.cpp:660,3419`, `CHyprBorderDecoration.cpp:234`, `Window.cpp:1733`), onde o 0.56.2 usava a flag pegajosa.

**Opções apresentadas ao dono (escolhida: A, 30/10/2026):**
- **A.** `window->backend().traits().suggestsNoBorder` ao vivo. Uma linha; mesma regra do host 0.57 para bordas. Aceita D1/D2 como diferença registrada.
- **B.** A + emular a pegajosidade no plugin (registro de janelas já vistas com `suggestsNoBorder`, limpo no `window.close`/destroy). Mais estado e teardown; ainda não exato (não vê o histórico anterior à carga do plugin nem mapeamentos sem `window.open`).
- Nenhuma opção reproduz o 0.56.2 integralmente; a 0.57 não expõe o histórico.

Independente da escolha (feito na implementação abaixo): `addWindowDecoration(PHANDLE, window, SP&&) → bool` (`plugins/PluginAPI.hpp:267`), `presentation().decorations()` (`WindowPresentation.hpp:43`), não publicar em `bars` se devolver `false` (`m_self` é atribuído antes de propósito: é um auto-ponteiro fraco da barra, não publicação) (H-REG, LEDGER L09/`:61`), e o erro `invalid new-expression of abstract class CHyprBar` (grupo R) deve continuar visível.

## Item 2 — `updateWindowDecos()` (`main.cpp:103`): equivalente no ponto de chamada (confirmado pela revisão Codex r1, com a ressalva de reentrância)

- O `Window.cpp:382–425`: se `!m_isMapped || isHidden()` retorna **sem** processar `m_decosToRemove`; senão processa remoções, `onWindowUpdate`, `updateWindow` em cópia de ponteiros crus.
- U `WindowPresentation.cpp:131–154`: processa `m_decosToRemove` **antes** do teste mapped/hidden; depois `onWindowUpdate` e `updateWindow` em `CDecorationSnapshot` (SP).
- A diferença apontada pelo P5 existe, mas é do **host**: na 0.57 o único escritor de `m_decosToRemove` (`WindowPresentation.cpp:123`, grep em toda a árvore U) chama `updateDecorations()` logo em seguida (`:125`), que esvazia a fila incondicionalmente. Logo, fora de reentrância dentro do próprio flush, a fila está vazia quando `onUpdateWindowRules` roda, e a chamada se comporta como a 0.56.2 com fila vazia (no-op se oculta/não mapeada; senão `onWindowUpdate` + `updateWindow`). O momento em que a 0.57 esvazia remoções de janela oculta mudou no host (em `removeDecoration`, O `:436–443` deixava pendente), e o plugin não o altera.
- Snapshot SP em vez de ponteiros crus: mantém decorações vivas durante o laço (mais seguro), mesmo filtro de pertença (`containsDecoration`).

## Item 3 — `m_isMapped` (`main.cpp:516`) → `mapped()`: equivalente

U `Window.cpp:227–229` devolve o mesmo campo `m_isMapped` (agora privado), escrito em `:1171` (map) e `:1683` (unmap), como O `:2076`/`:2644`. Já usado assim em `luaGrab` (P5).

## Implementação (após a decisão A)

Diff: `T2-step.diff` (só `plugin/main.cpp`, +9/−7).
- `onNewWindow`: `backend().traits().suggestsNoBorder`; `presentation().decorations()`; `makeShared<CHyprBar>`; `m_self` antes do registro; `addWindowDecoration(PHANDLE, window, SP<IHyprWindowDecoration>(bar))`; publicação em `bars` **só depois de `true`**. Em `false` o host não toca o ponteiro (U `PluginAPI.cpp:146–150`) e o SP local morre no fim da função (destrutor `barDeco.cpp:74–78` faz `erase` de `m_self`, no-op).
- Segurança da nova ordem: o código da barra nunca lê `bars` (só `barDeco.cpp:77`); os demais leitores são manipuladores de evento em `main.cpp`, que não rodam dentro de `addWindowDecoration`.
- `onUpdateWindowRules`: `presentation().updateDecorations()`. `pluginInit`: `mapped()`.

Validação (nada ao vivo):
- **Sonda** `T2-probe-run.sh` (GCC 16.2.1 / Clang 23.1.4, headers 0.57.0-1): exit 0, 35 OK. Positivo (tipos e expressões exatas) + NEG1–5 (as 5 formas 0.56.2) rejeitados nos dois compiladores. Vermelho no HEAD: `T2-probe-run-red-HEAD.log` (exit 1; mesmo runner com `T2_PLUGIN` apontando para o `main.cpp` do HEAD).
- **Host-free** (`T2-hostfree.cpp`; corpo de `onNewWindow` extraído verbatim de `main.cpp`; SP/WP reais do hyprutils; `-Werror`, ASan/UBSan): 7 cenários (normal, sem borda, já tem KwinishBar, já tem Hyprbar, decoração alheia, host recusa + sucesso posterior, host remove depois) em duas variantes do destrutor falso (real; sem `erase`, para provar a guarda sem o rollback do destrutor). Cada variante é compilada antes (erro de compilação nunca conta como morte) e só mata com a assertiva `FAIL S<n>` do teste, sem relatório de sanitizer. Mutações: **7 comportamentais** mortas com o destrutor real (MUT2–8) e **1 estrutural** (MUT1, ignorar o `false`), morta só na variante sem `erase`: com o destrutor real ela é observavelmente equivalente (a publicação seria desfeita antes do retorno, sem observador intermediário). O host-free não exercita os efeitos reais do construtor (ouvintes, animação, `m_scheduledRecalc`); numa barra recusada eles são liberados por RAII, exceto `m_scheduledRecalc = true` (`barDeco.cpp:57`): um recálculo extra, sem referência à barra.
- **Build isolado** (`T2-build.sh`): exit 2, sem `.so` (esperado). GCC **57→52**, Clang **65→60**. Saem os 5 erros W e o erro R `abstract class CHyprBar` em `UniquePtr.hpp`; entra o **mesmo** erro R em `SharedPtr.hpp` (instanciado em `main.cpp:44` via `makeShared`): deslocado, não escondido. Nenhum outro erro novo; avisos iguais; sem fatal/linker (`T2-build-compare.txt`).
- **Regressões**: geometria (g++ e clang++), tooltip, ações e sonda T1: exit 0 (`T2-hostfree-*.log`, `T2-regress-T1-probe.log`).
