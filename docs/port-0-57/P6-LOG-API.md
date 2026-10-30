# P6 — Log API para Hyprland 0.57 e inventário de keybinds (30/10/2026)

Escopo autorizado: sonda da Log API e alteração **exclusiva** das 5 chamadas `Log::logger->log` de `plugin/barDeco.cpp`; validação; depois **somente leitura** sobre `g_pKeybindManager`. Não tocados: render pass, GL, timers, hooks, lifecycle, ownership, `geometry.hpp`, OpenSpec, worktrees históricas. Nenhum plugin carregado, nenhum teste gráfico, nada instalado, nenhum commit. Base: o working tree da P5 (HEAD `3285517` + alterações P5 não commitadas, preservadas; cópia em scratch). Legenda: `O:` = fonte 0.56.2 (tarball `v0.56.2`, proveniência em `evidence/P5-provenance.json`), `U:` = 0.57.0 fixada `0aa6745f` (= headers instalados), `H:` = header instalado.

**Estado: CONCLUÍDA — aprovada pelo dono em 30/10/2026 (§6), não commitada.** Compilar não prova equivalência funcional; o resultado inclui a diferença medida (§3.3), a revisão independente (reprovada sob a leitura estrita, §5) e a decisão do dono que aceita essa diferença (§6).

## 1. Log API: 0.56.2 × 0.57.0

| Item | 0.56.2 (`O:debug/log/Logger.hpp`) | 0.57.0 (`H:debug/log/Logger.hpp`) |
|---|---|---|
| Template formatado | `log(level, std::format_string<Args...>, Args&&...)` | `log(level, const std::string_view loc, std::format_string<Args...>, Args&&...)` (`:43–56`) |
| Corpo do template | gates `m_logsEnabled`, TRACE; `logMsg = ""; logMsg += std::format(fmt, args...)`; `log(level, logMsg)` | mesmos gates; `logMsg = std::format("[{}] ", loc); logMsg += std::format(fmt, args...)`; `log(level, logMsg)` |
| Não-template | `log(level, const string_view&)` | o mesmo + **novo** `log(level, loc, str)` → `"[loc] str"`, sem formatação (`U:Logger.cpp:27–35`) |
| Macro | — | `LOG(level, fmt, ...)` = template com `loc = Log::logFnName()` (nome da função chamadora) |
| Níveis | `DEBUG, WARN, ERR, CRIT, INFO(=DEBUG), TRACE` | idênticos |

**Forma escolhida:** `Log::logger->log(Log::DEBUG, "kwinish-bars", "<fmt sem o prefixo>", args...)`. O template 0.57 monta `"[kwinish-bars] " + format(fmt, args)`, os mesmos bytes que o literal `"[kwinish-bars] <fmt>"` da 0.56.2. **Rejeitadas:** `LOG(...)` (acrescenta `[runCalls]`/nome da função: muda o conteúdo); pré-formatar com `std::format` e chamar o não-template (formata e aloca **antes** do gate: 1 alocação por chamada com log desligado; referência: 0); `hu()` (contorna `m_logsEnabled`, TRACE e `SRollingLogFollow`).

## 2. Diff aplicado (`evidence/P6-step2.diff`)

5 linhas em `plugin/barDeco.cpp` (380, 611, 654, 677, 807): `"[kwinish-bars] …"` → `"kwinish-bars", "…"`. Argumentos, nível (`Log::DEBUG`) e posição das chamadas inalterados. Nenhuma outra linha mudou; `main.cpp` idêntico ao estado P5.

## 3. Sondas (fontes e hashes em `evidence/P6-probes.txt`)

### 3.1 Sonda E (tipos; flags do Makefile; GCC 16.2.1 e Clang 23.1.4)
`static_assert`: tipo/valor dos níveis (`INFO == DEBUG`: uma troca DEBUG→INFO não é observável); `Log::logger` é `UP<CLogger>`; conceito `NewShape` (forma 0.57) verdadeiro para os tipos das 5 chamadas; `OldShape` (forma 0.56.2) falso; **armadilha** `TrapBinds`: `(level, "literal {}", string_view)` **compila** pelo não-template `(level, loc, str)` e não formata — compilar não é equivalência; `LOG` e `logFnName` existem.
NEG1 (forma de `:380`): falha com `no matching (member) function for call to log`. NEG2 (forma de `:807`): falha com `consteval … basic_format_string … is not a constant expression` (o erro desmascarado na P5). Aceite: **todas** as linhas de erro do log precisam pertencer ao conjunto esperado (a linha consequente do GCC só é aceita se vier de `bits/basic_string.h`).
Mutações (inversão de cada assertiva): **16/16** mortas (8 × 2 compiladores), só por `static_assert` (`P6-probes-mutate.log`).

### 3.2 Sonda R (execução, equivalência real)
`equiv_main.cpp` é compilado duas vezes: com o `Logger.hpp` **0.56.2** (`-I oldinc`; `g++ -H` confirma) e as 5 instruções extraídas **literalmente** do `barDeco.cpp` da P5, e com o header 0.57 instalado e as 5 instruções extraídas do candidato. Só o construtor e os sinks não-inline são definidos na sonda (corpo copiado de `Logger.cpp`); os templates são o código inline real do host. Por instrução, em 3 estados de gate (`log` ligado, desligado, TRACE ligado): bytes + nível emitidos, sequência ordenada de avaliação dos argumentos, número e tamanho das alocações, injeção de `bad_alloc` em cada ponto de alocação e acessor de argumento que lança.
- **Vermelho** (antes da edição, candidato = arquivo P5): referência compila e roda; candidato não compila contra 0.57 (`P6-probes-red.log`).
- **Verde** (`P6-probes-green.log`, exit 0, 30 OK em GCC+Clang): idênticos à 0.56.2 em bytes, nível, gating, ordem de avaliação (por compilador), **contagem** de alocações, falha em cada ponto (`bad_alloc`, nada emitido) e exceção de argumento (propaga, nada emitido, 0 alocações). Log desligado: saída **byte-idêntica, inclusive 0 alocações**.
- **Negativas** (compilam e precisam divergir em algum estado; tamanhos ignorados para não detectar trivialmente): `LOG` macro, `loc` com colchetes, prefixo duplicado, `loc` vazio, nível WARN, nível TRACE, argumento trocado, **pré-formatação** — 8/8 rejeitadas nos dois compiladores.

### 3.3 Diferença medida (não eliminável dentro da API pública 0.57)
Os **tamanhos** das alocações internas do template diferem: ex. `:380` 0.56.2 `40,40` → 0.57 `31,40`; `:807` `113,113` → `98,113`. Origem: o template 0.57 constrói o prefixo com `std::format("[{}] ", loc)` e concatena (host, não plugin). Mesmo número de alocações e mesmos pontos de falha; falha dependente de **tamanho** (ex.: alocador que rejeita 31 e aceita 40 bytes) não foi testada e pode distinguir as versões. O revisor confirmou que não há rota pública 0.57 que preserve bytes + gate + tamanhos (§5).

## 4. Validação

| Item | Resultado |
|---|---|
| Sondas E/R GCC+Clang | exit 0; 30 OK, 0 FAIL; mutações 16/16 |
| `tools/test-plugin-geometry.sh` | GCC exit 0, Clang exit 0 (`OK - P1-P5: 320762 verificacoes`, 13 negativos + mutações rejeitados) |
| `tools/test-plugin-tooltip.sh` | exit 0 |
| `make -C plugin TARGET=<scratch>` | **exit 2** (esperado: outras famílias); 67 linhas de erro GCC (P5: 73); 0 mencionam `Log::`/`CLogger`; nenhum `.so` gerado; `plugin/` sem `.so` |
| `tools/check.sh` | não executado |

**Comparação com a baseline desmascarada da P5** (`-fsyntax-only` por TU, sem limite de erros; baseline = cópia completa de `plugin/` com o `barDeco.cpp` da P5, que reproduziu **exatamente** os logs C da P5):

| Compilador | Baseline P5 | P6 | Eliminados | Persistentes | Novos |
|---|---|---|---|---|---|
| GCC (flags do Makefile) | 73 | 67 | 6 | 67 | 0 |
| Clang (flags da P5, sem `-Wno-c++11-narrowing`) | 96 | 91 | 5 | 91 | 0 |
| Clang (flags do Makefile) | 92 | 87 | 5 | 87 | 0 |

- **Eliminados:** os 5 erros da Log API (`:380, :611, :654, :677` — sem overload; `:807` — `consteval format_string`, mais 1 linha `call to non-constexpr appID()` no GCC).
- **Persistentes (fora desta etapa):** keybinds (`g_pKeybindManager`, `changeMouseBindMode`, `m_dispatchers`: GCC 13, Clang 36), render pass/`m_renderData`/`renderRect`/`renderTexture`/`scissor`, overrides dos passes, `CHyprBar` abstrata, `roundingPower`, `CONFIG_LEGACY`, `HL_MODIFIER_SHIFT`, ownership/`onNewWindow`, `listen` de `render.stage`, `m_isMapped` em `pluginInit`, estreitamentos (Clang).
- **Novos:** nenhum. Os 4 erros a mais do Clang com as flags da P5 são só `-Wc++11-narrowing` (`:88, :1011, :1076×2`).
- Redução numérica não é prova de compatibilidade funcional; a prova da equivalência das chamadas é a sonda R, com a ressalva de §3.3.

Evidência: `P6-build.log`, `P6-error-compare.{txt,json}`, `P6-geometry-*.log`, `P6-tooltip.log`, `P6-runtime-{ref-0562,cand-057}-gcc.txt`.

## 5. Revisão independente

`gpt-6.1-sol` via `hermes chat --safe-mode --provider openai-codex -t web --max-turns 1` (família diferente; Claude/Codex/Gemini CLI ausentes; nada instalado), uma sessão nova por rodada, com o diff real, os headers, as sondas e os logs.

| Rodada | Parecer | Tratamento |
|---|---|---|
| 1 | **reject** — major: equivalência de exceção/efeitos não demonstrada (sequência de alocação do template mudou); minor: o PASS não cobria gates/ordem; NEG aceitava erros em headers | Sonda R reescrita para **medir** alocações (contagem e tamanho), ordem, gates e `bad_alloc`; aceite NEG em todas as linhas |
| 2 | **reject** — major: tamanhos diferem (agora medidos); minor: sem acessor que lança, trace ausente nas falhas; aceite NEG2 amplo | Acessor que lança + trace nas falhas; NEG2 restrito a `basic_string.h`; diferença de tamanho declarada, não filtrada em silêncio |
| 3 | **reject** — único fundamento: a diferença de tamanhos de alocação viola a leitura estrita de "sem efeitos colaterais novos"; confirma que **não há rota pública 0.57** que preserve bytes + gates + tamanhos; minors resolvidos; diff dentro do escopo; comparação de erros consistente | Parado após 3 rodadas. **Dono aprovou e aceitou a diferença** (§6) |

Revisão estática: o revisor não executou código. Pareceres: `P6-review-{1,2,3}.json`; prompts: `P6-review-prompt-{1,2,3}.txt`.

## 6. Decisão do dono (30/10/2026) e fechamento

**Aprovada** a migração das 5 chamadas. O dono aceita como **diferença interna do Hyprland 0.57** a mudança no tamanho das alocações do template de log e **não exige** identidade de tamanho de alocação entre as versões do compositor. A aprovação considera preservados, **nos cenários testados**: conteúdo das mensagens, severidade, gates, ordem de avaliação e número de alocações. As substituições não devem ser desfeitas.

- **Classificação da diferença de §3.3:** diferença interna do host, fora do contrato observável do Kwinish (antes: fundamento da reprovação da revisão sob a leitura estrita).
- **Risco residual registrado:** falhas dependentes especificamente do **tamanho** da alocação (ex.: alocador que recusa 31 bytes e aceita 40) **não foram testadas**; também não foram testados argumentos reais do host (só mocks com os mesmos tipos de expressão).
- **Verificação de fechamento (P7, 30/10/2026):** o diff de `barDeco.cpp` em relação ao estado P5 continua com exatamente as 5 linhas (5 `-`/5 `+`); `main.cpp` idêntico ao estado P5. Reexecução: sondas E/R GCC+Clang exit 0 (30 OK, 0 FAIL), mutações 16/16; geometria GCC/Clang exit 0 (320 762 verificações); tooltip exit 0; `make` isolado exit 2 (esperado, outras famílias), 67 linhas de erro GCC, **0** relacionadas a logging, nenhum `.so`; comparação com a baseline P5: GCC 73→67 e Clang (flags do Makefile) 92→87, **0 novos**. Evidência: `evidence/P7-p6-closure-*.log`.

## 7. Inventário somente leitura: `g_pKeybindManager` (nada implementado)

### 7.1 Utilizações atuais (estado P6)
| Local | Uso | Papel |
|---|---|---|
| `barDeco.cpp:276–277` (`rearmDrag`) | `changeMouseBindMode(INVALID)`; `(MOVE)` | rearmar o arrastar após mover a janela |
| `barDeco.cpp:335–340` (`onTouchMove`) | `m_dispatchers["setfloating"/"resizewindowpixel"/"pin"/"movewindowpixel"]` | arrastar por toque |
| `barDeco.cpp:378–379` (`onMouseMove`, fora da barra) | `m_dispatchers["settiled"]`, `["mouse"]("0movewindow")` | encerrar arrasto por toque / mouse |
| `barDeco.cpp:564, 584–585` (unsnap) | `changeMouseBindMode(INVALID)` e `INVALID`+`MOVE` | pausar/reiniciar o arrastar do host durante a animação |
| `barDeco.cpp:628, 647` (`handleUpEvent`) | `changeMouseBindMode(INVALID)` | fim do arrastar |
| `barDeco.cpp:674` (`handleMovement`) | `changeMouseBindMode(MOVE)` | início do arrastar |
| `barDeco.cpp:684–686` (`dispatchIfKnown`; usado em 705, 707, 709, 713, 817) | `m_dispatchers[name](arg)` para `fullscreen`, `movetoworkspacesilent`, `exec` | botões e menu |
| `main.cpp:314` (`luaGrab`) | `changeMouseBindMode(MOVE|RESIZE)` | Move/Resize do menu |
| `main.cpp:483` (listener `input.mouse.button`) | `changeMouseBindMode(INVALID)` | fim do grab do menu |

### 7.2 Semântica antiga e alternativas 0.57
- `O:KeybindManager.cpp:949–975` `changeMouseBindMode(MODE)`: se `MODE != INVALID`: retorna se já há alvo de drag; `windowAt(cursor, RESERVED|INPUT_EXTENTS|ALLOW_FLOATING)`; sem janela → `passEvent`; se não fullscreen e `MOVE`, `checkInputOnDecos(INPUT_TYPE_DRAG_START)` pode recusar; `g_layoutManager->beginDragTarget(PWINDOW->layoutTarget(), MODE)`. Se `INVALID`: `endDragTarget()` se houver alvo. **Não existe na 0.57** (nem `g_pKeybindManager`; o gerente novo é `Keybinds::mgr()`, sem esse método).
- Primitivas 0.57 (`H:layout/LayoutManager.hpp:78–83,104,111`): `g_layoutManager->beginDragTarget(SP<ITarget>, eMouseBindMode, forcedEdge = nullopt, exclusiveDeviceGrab = false)`, `bool endDragTarget()`, `dragController()->target()/mode()`; `eMouseBindMode` continua em `H:managers/input/InputManager.hpp:38–43`. Substituto mais próximo: reproduzir o corpo antigo com `Desktop::viewState()->hitTest().windowAt(...)`, `Fullscreen::controller()->isFullscreen`, `presentation().checkInputOnDecorations(INPUT_TYPE_DRAG_START, …)` e `beginDragTarget/endDragTarget`. `U:ConfigActions.cpp:1695–1738` (`Actions::mouse`) faz exatamente isso, mas lê `Config::Actions::state()->m_passPressed` (estado global de bind), inadequado para chamadas fora de um bind.
- `m_dispatchers` (0.56.2: mapa preenchido pelo tradutor legado `O:config/legacy/DispatcherTranslator.cpp`; 0.57: **não existe**, nem `config/legacy`). Os dispatchers usados já eram embrulhos de `Config::Actions` na 0.56.2, cujas declarações seguem na 0.57: `setfloating`/`settiled` → `floatWindow(ENABLE/DISABLE, w)`; `pin` → `pinWindow(TOGGLE, w)`; `movewindowpixel`/`resizewindowpixel` → `move/resize(vec, false, w)` (o `exact 50% 50%` era resolvido por `g_pCompositor->parseWindowVectorArgsRelative`: precisa de cálculo próprio); `fullscreen 1/0` → `fullscreenWindow(MAXIMIZED/FULLSCREEN, true)`; `movetoworkspacesilent` → `moveToWorkspace("special:minimized", true, w)` (há sobrecarga `string` na 0.57); `exec` → `Config::Supplementary::executor()->spawn(args)`. `mouse("0movewindow")` → na 0.56.2 o `0` era descartado (`args.substr(1)`) e o efeito dependia de `m_passPressed`; fora de bind isso encerrava o drag → substituto: `endDragTarget()` direto. O plugin já usa `Config::Actions::resize/move/floatWindow` em outros pontos.
- `dispatchIfKnown` hoje ignora silenciosamente nome desconhecido; com chamadas diretas a falha vira `ActionResult` (`std::expected`), a decidir se loga.

### 7.3 Dependências de mouse-bind
- Ambos os hosts encerram **qualquer** drag a cada evento de botão/tecla: 0.56.2 `ensureMouseBindState()` (`O:KeybindManager.cpp:336–346, 389, 486`); 0.57 `endDragTarget()` em `U:keybinds/Manager.cpp:223` (tecla) e `:353` (botão, salvo `captured`). O listener `input.mouse.button` do plugin é emitido antes (`U:InputManager.cpp:722` vs `:732+`); a ordem na 0.56.2 **não foi conferida** (testar).
- `dragBegin` 0.57 mudou: hotspot proporcional (`binds:drag_center_window`), `exclusiveDeviceGrab`, `dragEnd()` devolve `bool`, sinais `m_events.motion/ended` (`U:DragController.cpp`, diff vs O). O unsnap (`stepUnsnap`, `rearmDrag`) reposiciona a janela e reinicia o drag: o reposicionamento pelo hotspot pode alterar a posição após o rearm (hipótese, a medir).
- `luaGrab` foca a janela e confia no hit-test sob o cursor; o substituto deve manter "o host arrasta a janela sob o cursor" ou passar explicitamente `w->layoutTarget()` (mudança semântica a decidir).

### 7.4 Efeitos sobre Q4 e descarregamento
- L17/Q4 (`LEDGER.md:45`, `QUIESCENCE.md:30`): o passo 2 do unload deve encerrar drag/grab iniciado pelo plugin. Hoje `PLUGIN_EXIT` **não** faz isso (`main.cpp:526–542`). Na 0.57 o reset seria `if (g_layoutManager->dragController()->target()) g_layoutManager->endDragTarget();`, condicionado a `m_bDraggingThis`/`grabActive` (não encerrar drag do usuário por bind do host).
- `endDragTarget()` emite `m_events.ended` e pode chamar o layout; chamá-lo dentro do unload é lifecycle → mudança de keybinds precisa ou ficar fora de `PLUGIN_EXIT`, ou entrar junto do protocolo de unload (autorização própria).
- Nenhum hook/registro de keybind é criado pelo plugin (só chamadas); não há desregistro a fazer além do reset do estado de drag.

### 7.5 Testes necessários (antes de implementar)
1. Sonda de tipos: ausência de `g_pKeybindManager`/`m_dispatchers`; assinaturas de `beginDragTarget/endDragTarget/dragController`, `Config::Actions::*` usadas, `executor()->spawn`; NEG com as formas antigas.
2. Tabela de equivalência por chamada (dispatcher antigo → Action 0.57), comparando corpos O×U como na P5; incluir `exact 50% 50%` e `special:minimized`.
3. Teste host-free (se possível) da lógica de seleção de janela do substituto de `changeMouseBindMode` (hit-test vs janela da barra).
4. Aninhado/ao vivo (dono): arrastar pela barra, unsnap com rearm, toque, Move/Resize do menu + Escape, botões fullscreen/minimizar/menu; após cada um, `hyprctl` sem drag ativo.
5. Q4 (B1-D3): unload com drag/grab ativo → nenhum drag ativo após o passo 2; drag iniciado por bind do usuário não é encerrado pelo plugin.
