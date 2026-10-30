# P5 — Compatibilidade estrutural com Hyprland 0.57: header e acessores da janela (30/10/2026)

Escopo autorizado: sondas de compilação temporárias e uma alteração incremental **limitada ao include da janela e aos acessores cuja equivalência foi demonstrada**. Fora do escopo e **não tocados**: interfaces de render pass, chamadas/estado GL, hooks, timers, lifecycle/teardown, ownership/registro de decorações, integração de `geometry.hpp`, Log API. Nenhum plugin carregado, nenhum teste gráfico, nada instalado, nenhum commit. Legenda de evidência em [README](README.md) §2; aqui `O:` = fonte 0.56.2 (tarball da tag `v0.56.2`, só em scratch, proveniência em `evidence/P5-provenance.json`) e `U:` = fonte 0.57.0 fixada (`0aa6745f`, igual aos headers instalados).

Compilar não prova equivalência funcional: a coluna "Limitações" é parte do resultado.

## 1. Decisões de projeto para a futura integração (dono, 30/10/2026)

Registradas como decisões; **não autorizam implementação nesta tarefa**.

| ID | Decisão |
|---|---|
| G2 | Snapshot geométrico **coerente por quadro**, copiado por valor; nenhuma referência emprestada ao contexto de renderização (`ctx`, `presentation`) sobrevive à chamada. |
| Input | Consumir o **último snapshot publicado válido**, com controle de geração e invalidação. |
| Snap | Cancelar o candidato de snap cujo preview obrigatório não possa ser representado (`previewBBox` → `nullopt`). |
| Damage | Se a união segura falhar, solicitar **redraw completo do monitor afetado**, se a API suportar. |
| `nullopt` | Diagnóstico em log com **limitação de frequência**; sem notificação gráfica por padrão. |
| G4, G5 | **Pendentes** até a migração das APIs correspondentes (enqueue do preview; captura/cena do tooltip). |

## 2. Matriz de equivalência

Estado: **A** = aplicado nesta etapa; **D** = adiado (motivo na coluna). Uso = linhas do baseline `3285517` (em `barDeco.cpp`, +1 após a linha 10 no candidato).

| API antiga (0.56.2) | Candidata 0.57.0 | Evidência no header/fonte | Sonda | Limitações semânticas | Estado |
|---|---|---|---|---|---|
| `#include <…/desktop/view/Window.hpp>` | `<…/desktop/view/window/Window.hpp>` (+ `window/WindowPresentation.hpp` em `barDeco.cpp`, porque `Window.hpp` só declara `CWindowPresentation`) | H: `desktop/view/window/Window.hpp` (classe `Desktop::View::CWindow`; inclui `WindowMetadata.hpp` ao fim) | A | Mesmo tipo (`PHLWINDOW::get()` → `CWindow*`, base `IGeometric`); o header novo não puxa `WindowPresentation.hpp` | A — main.cpp:9, barDeco.cpp:10 |
| `m_title` | `metadata().title()` → `const std::string&` | H: `WindowMetadata.hpp`; O: `Window.cpp:1446–1452` × U: `Window.cpp:903–908` (mesmo gatilho `onUpdateMeta`, mesmos eventos) | B | Referência viva: as atribuições copiam (`m_szLastTitle`), nenhuma referência é guardada | A — 1061, 1062 |
| `m_class` | `metadata().appID()` | O: `Window.cpp:1466–1470` × U: `Window.cpp:921–926`; U: `X11Backend.cpp:35` normaliza `appid` X11 | B | Para X11 o valor vem de `normalizedX11String(...)`; a normalização da 0.56.2 (`fetchClass`) não foi comparada linha a linha | A — 721, 724, 791, 794, 800, 806 |
| `m_isMapped` (público) | `mapped()` | U: `Window.cpp:227–229` devolve o mesmo `m_isMapped` (agora privado); escritores O:2076/2644 × U:1171/1683 | C | Nenhuma | A — main.cpp:309. **D** — main.cpp:515 (está em `pluginInit`, lifecycle) |
| `m_isFloating` | `isFloating()` | O: `WindowTarget.cpp:281–292` (`floating()` lia `m_isFloating`) × U: `Window.cpp:1949–1951` → `m_target->floating()` → `m_floating` (`WindowTarget.cpp:282–300`); `m_target` criado em `initialize()` (`Window.cpp:126`), chamado por `create()`, único factory (construtor privado); sem outro escritor | C | Mesma fonte de verdade (alvo do layout), armazenamento movido. Lê o alvo **da janela**, não o do grupo (`layoutTarget()`), como antes | A — 391, 418, 593, 769 |
| `m_pinned` | `static_cast<bool>(m_state & WINDOW_STATE_PINNED)` | H: `EXPOSE_ENUM_AS_MASK` (`macros/Enums.hpp`, `operator bool` **explícito**); escritores O (8) × U (10) correspondem (ConfigActions, XDGShell ×2, Fullscreen ×2, WindowTarget, regra estática, "tiled pinned") | C | Máscara só converte explicitamente: todas as ocorrências usam `static_cast<bool>` (necessário no ternário de `updateTip`) | A — 866, 1004, 1159, 1246 |
| `rounding()` | `presentation().rounding()` | O: `Window.cpp:1101–1109` × U: `WindowPresentation.cpp:285–293`, corpo idêntico | C | Nenhuma além da mudança de dono | A — 515, 1006 |
| `roundingPower()` | `presentation().roundingPower()` | O: `:1111–1115` × U: `:295–299`, idêntico | C | — | **D** — 1048/1056/1058 estão dentro de `renderRect(...)`, chamada GL proibida; migram com a família de render (exige `CRenderContext&`) |
| `getRealBorderSize()` | `presentation().borderSize()` → `CHyprBorderDecoration::borderSize()` | O: `Window.cpp:1138–1153` × U: `CHyprBorderDecoration.cpp:207–227`, mesmo cálculo e cache; `m_borderDecoration` criado em `WindowPresentation::initialize()` (`:64`), sem outro escritor; invalidação O (`updateWindowData`, `updateDecorationValues`, 19 chamadores) × U (`updateWindowData`, `refreshValues`→`updateState`, 17 chamadores) | C | **Diferença real, delimitada:** os handlers de fullscreen da 0.56.2 chamavam `updateDecorationValues()` diretamente; na 0.57 dependem de `propertiesChanged`, que retorna cedo se a janela está **não mapeada ou oculta**. O cache pode ficar velho nesse caso até o próximo `refreshValues/updateWindowData`. A borda do host lê o mesmo cache: a barra fica coerente com o host 0.57, não idêntica à 0.56.2 nessa borda | A — 1006, 1092 (com a limitação) |
| `m_floatingOffset` | `presentation().floatingOffset()` → `const Vector2D&` | O: `Window.cpp:1203–1238` × U: `WindowPresentation.cpp:423–455`, mesma fórmula; escritores O (5) × U (7: inclui as variantes set/clear) correspondem; U adiciona a origem `WORKSPACE/WINDOW` | C | É o **valor vivo bruto**, equivalente ao campo antigo. **Não** é o `SWindowRenderPresentation::floatingOffset` do quadro, que a 0.57 zera em cenas não-MONITOR quando a origem é `WORKSPACE` (`U:WindowRenderPresentation.cpp:12`). A troca pelo snapshot é da mudança de render/G2, não desta | A — 1019, 1028, 1029 |
| `m_windowDecorations` (`vector<UP<…>>`) | `presentation().decorations()` → `span<const SP<IHyprWindowDecoration>>` | H: `WindowPresentation.hpp` | D | Ownership UP→SP | **D** — `onNewWindow` (main.cpp:40), ownership |
| `addWindowDecoration(…, UP<…>)` | `addWindowDecoration(…, SP<IHyprWindowDecoration>&&)` | H: `PluginAPI.hpp:267` × O: `PluginAPI.hpp:265` | D | `UP<Derived>` não liga mais; `SP<Derived>` liga. Afeta `g_pGlobalState->bars` (`WP`), `m_self`, rollback (H-REG) | **D** — lifecycle/ownership |
| `updateWindowDecos()` | `presentation().updateDecorations()` | O: `Window.cpp:383–401` × U: `WindowPresentation.cpp:131–154` | D | **Não equivalente:** a 0.57 processa remoções pendentes **antes** do teste mapped/hidden; a 0.56.2 não processava em janela oculta/não mapeada. Toca teardown | **D** — main.cpp:102 |
| `m_X11DoesntWantBorders` | `backend().traits().suggestsNoBorder` | U: `X11Backend.cpp:98` = `NO_BORDER_ATOM \|\| m_overrideRedirect`; O: `XWaylandManager.cpp:158,164` | D | Candidata é superconjunto (inclui override-redirect) — não demonstrada equivalente | **D** — `onNewWindow` |
| `IHyprWindowDecoration::draw(PHLMONITOR, float const&)` | `draw(CRenderContext&, PHLMONITOR, float const&, const SWindowRenderPresentation&)` | H: `IHyprWindowDecoration.hpp:50` | D | Família de render | **D** — fora do escopo (torna `CHyprBar` abstrata) |

Já compilavam sem mudança e não foram tocados: `position/size(GEOMETRIC_CURRENT)`, `m_workspace`, `m_monitor`, `m_ruleApplicator`, `isHidden()`, `wlSurface()`, `layoutTarget()`.

## 3. Sondas (scratch; fontes em `evidence/P5-probes.txt`)

Quatro TUs independentes, compiladas com os flags do Makefile (`-std=c++2b`, `pkg-config` do plugin), HOME/XDG temporários, sem DISPLAY/WAYLAND. Modo positivo: `static_assert` sobre tipos/assinaturas 0.57 e conceitos `requires` provando que a API antiga sumiu ou ficou privada. Modo `-DPROBE_NEG`: usa a API 0.56.2 e **precisa falhar** com o diagnóstico esperado. Mutações: 9 cópias com uma assertiva trocada; cada uma precisa falhar **somente** por `static assertion failed`.

| Sonda | Família | GCC 16.2.1 | Clang 23.1.4 | NEG (ambos) |
|---|---|---|---|---|
| A `probe_a_include` | header da janela | exit 0 | exit 0 | exit 1, `…/desktop/view/Window.hpp: No such file` / `file not found` |
| B `probe_b_metadata` | metadata | exit 0 | exit 0 | exit 1, sem `m_title`, `m_class` |
| C `probe_c_presentation` | apresentação/geometria/estado | exit 0 | exit 0 | exit 1, sem `m_isFloating`, `m_pinned`, `rounding`, `getRealBorderSize`, `m_floatingOffset`; `m_isMapped` privado |
| D `probe_d_decorations` | tipos/ownership de decorações | exit 0 | exit 0 | exit 1, sem `updateWindowDecos`, `m_X11DoesntWantBorders`, `m_windowDecorations` |

Runner `run.sh`: 16/16 OK, **exit 0** (`evidence/P5-probes-run.log`). Mutações `mutate.sh` (GCC): **9/9 rejeitadas** só por `static_assert`, **exit 0** (`P5-probes-mutate.log`). Correções durante o trabalho, todas reexecutadas: A usava `__has_include` fora de diretiva (a primeira execução falhou e o NEG do Clang passou vazio; corrigido com `#if/#error` e expectativa mais estrita); revisão 1 apontou conceito conjuntivo em C e falta de `UP<Derived>` em D (corrigidos); revisão 2 apontou que a mutação m9 era trivialmente contraditória (trocada por inversão real); duas mutações tinham `\&` literal (erro de sintaxe contado como morte) — corrigidas, e o critério passou a exigir que **todos** os erros sejam `static_assert`.

Limites: as sondas checam tipos e formas de expressão; não executam criação de janela, invalidação de cache, mudança de metadata nem renderização.

## 4. Comparação dos erros de compilação

Três medidas, todas com `TARGET`/saída em scratch (nenhum `.so` gerado; nenhum artefato existente sobrescrito; `plugin/` não tem `.so`):

- **B0** = `make` do HEAD `3285517`: **exit 2**, 14 linhas de erro, **idênticas** a `evidence/P2-build.log` (comparação normalizada). Dois `fatal error` de include escondem todos os erros restantes de `main.cpp`/`barDeco.cpp`.
- **B1** = HEAD com **somente** o caminho do include corrigido (cópia scratch), `-fsyntax-only` por TU, sem limite de erros: referência "desmascarada".
- **C** = candidato desta etapa, mesmos comandos; `make` do candidato: **exit 2**, 73 linhas de erro (GCC).

| Compilador | B1 | C | Eliminados | Persistentes | Novos |
|---|---|---|---|---|---|
| GCC | 98 | 73 | 27 | 71 | 2 |
| Clang | 119 | 96 | 24 | 95 | 1 |

- **Eliminados:** todos os erros dos 9 acessores aplicados (m_class ×6, m_isFloating ×4, m_pinned ×4, m_floatingOffset ×3, rounding ×2, getRealBorderSize ×2, m_title ×2, m_isMapped ×1) e, no GCC, 3 erros em cascata (`could not convert {<expression error>…}`).
- **Novos:** só `barDeco.cpp:806` (GCC 2 linhas, Clang 1): a chamada `Log::logger->log(Log::DEBUG, "…{}: {}", appID, e.what())` agora é analisada até o fim e bate na **Log API 0.57** `log(level, loc, fmt, args…)` (`H:debug/log/Logger.hpp:40–45`): o literal vira `loc` e `appID()` cai na posição do format string. É erro **desmascarado** da mesma família dos 4 `Log::CLogger::log` persistentes, não regressão do acessor (confirmado pela revisão).
- **Persistentes (dependências registradas, fora desta etapa):** `g_pKeybindManager`/`changeMouseBindMode`/`m_dispatchers` (keybinds); `m_renderPass`, `m_renderData`; `renderRect/renderTexture/scissor` sem `CRenderContext&`; overrides de `CBarPassElement`/`CPreviewPassElement`; `CHyprBar` abstrata (`draw`); `roundingPower` ×3 (linhas GL); `m_X11DoesntWantBorders`, `m_windowDecorations`, `UP→SP`, `updateWindowDecos`, `m_isMapped` em `pluginInit`; `listen` de `render.stage`; `CONFIG_LEGACY`; `HL_MODIFIER_SHIFT`; Log API; estreitamentos `-Wc++11-narrowing` (Clang).

Evidência: `P5-B0-build.log`, `P5-C-build.log`, `P5-error-compare.txt/.json`.

## 5. Regressões geométricas

`tools/test-plugin-geometry.sh` após a alteração: GCC **exit 0** e Clang **exit 0**, `OK - P1-P5: 320762 verificacoes`, 13 negativos e 46 mutações rejeitados pela assertiva esperada (`P5-geometry-*.log`). `tools/test-plugin-tooltip.sh`: exit 0. `geometry.hpp`/`test_geometry.cpp` não foram alterados (o teste não inclui os arquivos editados; serve de não-regressão do contrato).

## 6. Revisão independente

`gpt-6.1-sol` via `hermes chat --safe-mode --provider openai-codex` (família diferente da do implementador; Claude/Codex/Gemini CLI ausentes; nada instalado), uma rodada por sessão nova, sem ferramentas, com o diff real, as sondas, trechos de fontes e a comparação de erros.

| Rodada | Parecer | Tratamento |
|---|---|---|
| 1 (`P5-review-1.json`) | **reject**: 2 major (validade de `m_target` em `isFloating()` não demonstrada; equivalência do cache de borda sem os chamadores de invalidação) + 2 minor de sonda | Evidência adicional: factory único e escritor único de `m_target`/`m_borderDecoration`; comparação de chamadores de invalidação, que revelou a diferença real em fullscreen de janela oculta/não mapeada, agora documentada como limitação; sondas corrigidas e reexecutadas |
| 2 (`P5-review-2.json`) | **approve**; 1 minor (m9 contraditória) | m9 corrigida, mutações reexecutadas (9/9). Ressalva do revisor mantida: `borderSize()` não é "equivalência incondicional" |

O revisor também confirmou: nenhuma edição toca interface de render pass, chamada/estado GL, hooks, timers, lifecycle ou ownership; os adiamentos são coerentes; a classificação de `:806` está correta. Revisão estática: o revisor não executou código.

## 7. Próxima mudança recomendada (precisa de autorização)

**Log API, sem render:** migrar as 5 chamadas `Log::logger->log` de `barDeco.cpp` para a forma 0.57 (`LOG(level, fmt, …)` ou `log(level, loc, fmt, …)`), com sonda própria antes (inclui o erro desmascarado de `:806`). Família independente de render/GL/lifecycle. Depois, **somente sonda/inventário** do substituto de `g_pKeybindManager->changeMouseBindMode/m_dispatchers` (13 GCC / 36 Clang erros): o modo de mouse-bind participa de Q4 (restauração no unload), então a migração pode tocar lifecycle e precisa ser delimitada antes. Em mudança separada: ownership de decorações (`onNewWindow`, UP→SP, `bars`, `m_self`, `m_isMapped` de `pluginInit`, `updateDecorations`), que depende do protocolo de unload (B1/H-REG). Render pass/GL (`draw`, `renderRect(ctx…)`, `roundingPower`, snapshot G2) por último.
