# Q4-A — Política segura de encerramento de drag (estudo host-free, 30/10/2026)

**Escopo:** investigação, modelo e testes host-free. **Nenhum código de produção alterado**; K3/K9/rearm, `PLUGIN_EXIT`, listeners do plugin, render/GL intocados; nada carregado; sem commit. Base `f76c785`. Fontes: `U:` = Hyprland 0.57.0 `0aa6745f` (`DragController.cpp` sha256 `6a7c339a…0a5f`, igual ao dos relatórios anteriores); trechos e greps em `Q4A-policy-host-sources.txt`. Código: `Q4A-policy-tests/` (`host.hpp`, `policies.hpp`, `policytest.cpp`, `probe.cpp`, `run.py`, `expected-matrix.txt`). Saídas: `Q4A-policy-run.log` (runner), `Q4A-policy-detail.txt` (por cenário/política). Revisão: `Q4A-policy-review-*.json`.

## 1. Modelo de eventos (Etapa 1)

`host.hpp` replica `dragBegin` (U:159–251) e `dragEnd` (U:252–334): `m_target` (WP), `m_dragMode`, `m_wasDraggingWindow`, emissão de `ended` por *scope guard* na saída (:258), com o **sinal real** do hyprutils 0.14.3 (snapshots, aninhamento). Ganchos síncronos: decorações `DRAG_END` com continuação ou retorno antecipado (:287–292), group add (:299), `changeFloatingMode` → `window.floating` (:314; LayoutManager.cpp:52), `setTargetGeom` (:321), `fullWindowFocus` → `window.active` (:323; FocusState.cpp:220), foco dentro de `dragBegin` (:245), decorações `DRAG_START` (group bar inicia drag próprio, `CHyprGroupBarDecoration.cpp:417`, foco em :420; ou decoração de terceiro que inicia e devolve `false`). Entradas do Kwinish: início (semântica de `KwDrag::begin`, P8-B), fim (K3/K9), rearm, unload passo 2, fila *idle* (`doLater`). Recusa por tela cheia modelada dentro de `dragBegin` (caso `drag_threshold <= 0`; com limiar > 0 ela ocorre no `mouseMove` do host, fora do início).

As políticas só recebem a camada observável:

| Observável pelo plugin (API pública 0.57; tipos confirmados por `probe.cpp` nos headers instalados) | Só o compositor (ou o oráculo) sabe |
|---|---|
| `target()` (WP travado), `mode()`, `wasDraggingWindow()`, `dragThresholdReached()`, `draggingTiled()`, `exclusiveDeviceGrab()` | identidade da sessão; quem chamou `dragBegin`/`dragEnd` |
| `m_events.ended` **sem payload** (`CSignalT<>`); `m_events.motion` | qual sessão um `ended` encerra; se há `dragEnd` ou emissão na pilha; posição de cada ouvinte no snapshot |
| eventos do barramento que o plugin assina (`window.active`, `window.floating`, `mouse.*`) | código de outros plugins/Lua dentro dos callbacks; ordem dos ouvintes de terceiros |
| resultado do helper (`KwDrag::begin`: `STARTED` = `beginDragTarget` foi chamado, não que o drag existe nem que é do plugin) | se o host recusou dentro de `dragBegin`; se uma decoração iniciou drag próprio |

Fatos verificados nesta sessão por grep em todo `src/`: `m_wasDraggingWindow` só é escrito em :275 (`true`), :288 e :325 (`false`); `m_dragMode` só em :161, :267, :289, :329; **nenhum ouvinte do host** em `DragController::m_events.ended` (só a emissão :258); chamadores de `dragBegin` checam `target()` antes (Window.cpp:1861/1871, InputManager.cpp:876, ConfigActions.cpp:1704). Binário instalado exporta `wasDraggingWindow`, `dragBegin`, `dragEnd` (`nm -DC /usr/sbin/Hyprland`).

## 2. Janelas da transição de fim

| Janela | Linhas | Estado típico (topo da pilha) | Código estranho síncrono? | Conclusão |
|---|---|---|---|---|
| W1 | :274→:275 | alvo nulo, flag herdada | nenhum | excluída estruturalmente |
| W2 | :275→:288 / :325 | alvo nulo ou de terceiro reentrante, **flag true** | sim: decorações (inclui group bar → `fullWindowFocus`), group add, `window.floating`, layout, `window.active` (Lua do usuário, outros plugins, Kwinish) | **não** excluível: `mouse.move` pode ser emitido sincronamente via `simulateMouseMovement` por dispatchers (ConfigActions.cpp:92,129,777,1041,1237) e layouts; rearm/unsnap do Kwinish ficam em `mouse.move`. Coberta por **observação** (`wasDraggingWindow`) |
| W3 | :325→emissão | flag false | só `setClientResizingState` (:52–57) → XDG agenda `doLaterLock` (XDGShell.cpp:444–498, 668–676); X11 vazio | excluída estruturalmente para o caminho lido (0.57 fixada) |
| W4 | não mapeado :262→emissão | **não escreve a flag** (herda a do quadro externo); alvo **mantido** se não há janela (:263–266) | nenhum após o reset (`unsetOverride` vem antes) | excluída estruturalmente; o `ended` ainda invalida a posse |
| W5 | emissão de `ended` | alvo nulo/terceiro, flag false | ouvintes de `ended` anteriores ao do Kwinish | mundo fechado: o host não tem ouvintes → excluída. **Mundo aberto: não excluível (D1)** |

Aninhamento: um `dragEnd` mapeado aninhado em W2 zera a flag antes do fim externo, mas emite `ended` antes disso e invalida a posse (`R4.nested-clear`). A flag fica presa em `true` só se uma exceção atravessar W2: efeito apenas de liveness.

## 3. Política candidata `veto` (Etapa 2)

Posse só é usada se **todas** valerem no instante do fim (`policies.hpp`, `PolVeto`):

- **(a) Aquisição verificável:** o `beginDragTarget` do **próprio** plugin rodou (resultado `STARTED`, não `REFUSED_BY_DECO`), nenhum `ended` chegou durante o início e o alvo depois é o pedido. Guarda RAII `inBegin`.
- **(b) Invalidação:** ouvinte por aquisição com **token** capturado; `ended` com token atual zera a posse e solta o ouvinte; callbacks antigos (snapshots aninhados) são ignorados.
- **(c)** alvo adquirido vivo e igual a `target()`.
- **(d) Veto de transição ambígua:** `wasDraggingWindow()` verdadeiro ⇒ não encerra **e** não inicia.
- **(e) Transação não reentrante:** sem fim/início do plugin em andamento; posse **consumida antes** de `endDragTarget()`.
- **Rearm** = fim bem-sucedido + novo início com nova aquisição. **Unload passo 2** = `unloading`, fim se (a)–(e), solta o ouvinte sempre.
- **Pré-requisito do helper (revisão 1):** `KwDrag::begin` precisa **rechecar `target()` depois do despacho `DRAG_START`** e imediatamente antes de `beginDragTarget` (variante `veto-rc`). Sem isso, uma decoração de terceiro que inicia drag e devolve `false` faz o próprio helper substituir a sessão dela (`H1`). Exige mudança no helper P8-B: **não autorizada aqui**.

Nenhum item usa alvo, modo, geração ou flag local isoladamente como prova: alvo só vale junto de (a)+(b); geração/token só invalidam; `wasDraggingWindow` só **veta**. Flags do plugin (`m_bDraggingThis`, `grabActive`) não entram na posse.

**Argumento de segurança (mundo fechado, `veto-rc`).** Suponha um fim do Kwinish sobre sessão S de terceiro com (a)–(e) válidos. Por A3′, `target()` era nulo quando S começou, logo a sessão do plugin terminou após a aquisição por um `dragEnd` (alvo vivo ⇒ `ended` na saída) ou o alvo expirou (viola (c)). Se aquele `dragEnd` já retornou, o ouvinte (registrado antes dele) recebeu `ended` ⇒ (b) falso. Logo o fim do Kwinish ocorreu **dentro** desse `dragEnd`: W1/W3/W4 não têm código estranho; W2 é vetada por (d); resta W5 antes do ouvinte do Kwinish, o que exige ouvinte estranho de `ended` (inexistente no host). Se o fim originou-se no próprio Kwinish, (e) bloqueia também W5.

**Premissas:** A1 thread única; **A3′ (reforçada pela revisão 1):** nenhuma sessão começa entre a guarda `target()` de um chamador e seu `dragBegin` — vale para o helper do Kwinish só com o recheck; para o host, depende de decorações `DRAG_START` (ConfigActions.cpp:1704→1733→1736) e do `GROUP->remove` da group bar antes de seu `dragBegin` sem guarda (Group.cpp:234–299, layout/visibilidade síncronos; **não auditado exaustivamente**); A4 `ended` síncrono e único por fim com alvo vivo; A5 unload fora de emissões; **D1 (nova, não aceita):** nenhum ouvinte de `ended` de terceiros, anterior ao do Kwinish, reinicia drag e alcança sincronamente uma entrada de fim do Kwinish; **D2:** classificação W1–W5 e escritores da flag valem só para `0aa6745f`. Exclusões adicionais (não consequências de D1): falha de `listen` (C6), exceções do host, *wrap* do token.

## 4. Testes host-free (Etapa 3)

`python3 docs/port-0-57/evidence/Q4A-policy-tests/run.py` — GCC 16.2.1 e Clang 23.1.4, `-Werror`, ASan/UBSan; **42 cenários × 7 políticas**; sondas de tipo nos headers instalados (positivo compila; `-DNEG` falha só por `sessionId` ausente). Oráculos sobre a camada de verdade, inacessível às políticas:

- **O1** (evento a evento) nenhum `dragEnd` chamado pelo Kwinish sobre sessão iniciada por outro, nem início do Kwinish que substitua sessão ativa;
- **O2** pedido de fim em contexto *comprovadamente seguro* (sem `dragEnd`/emissão na pilha, sessão viva iniciada pelo Kwinish, nenhum `ended` desde o início) foi atendido (inclusive no *idle*);
- **O3** verificação **diferencial**, não causal: estado final comparado com a execução em que o Kwinish nunca encerra (`none`); sessão estrangeira final deve ser idêntica (alvo, ordinal, modo). Diferenças presentes nas duas execuções são anotadas como do compositor. Pode acusar sessão estrangeira legítima habilitada por um fim próprio correto, e não vê fim indevido quando nenhuma execução retém a sessão: O1 cobre esse caso;
- `s` = contrato do cenário (sem início após unload; `STARTED` só para sessão própria limpa); `o` = sessão do Kwinish viva sem posse (informativo).

Resultado real (`Q4A-policy-run.log`): build 0, execução 0 e sem saída de sanitizer nos dois compiladores; saídas idênticas; sondas OK; matriz igual à esperada; `RESULT: PASS (… this is not an approval)`. Contagem de cenários com falha:

| Política | O1 | O2 | O3 | Leitura |
|---|---|---|---|---|
| `none` (nunca encerra; helper atual) | 1 (`H1`, substituição pelo helper) | **13** | — | “não encerrar nada” não passa (`s`: 6 — helper reporta `STARTED` em recusas; sem unload) |
| `target-eq` | 22 | 1 | 17 | inseguro (`s`: 6) |
| `C3-prev` (contrato anterior) | 16 | — | 11 | inseguro (todas as rotas R4, G1, G2, G4, R4.emit, H1) |
| `veto` (helper atual) | 3: `H1`, `R4.emit.host`, `L1.A3` | — | 2 | depende do recheck do helper |
| **`veto-rc` (candidata)** | **2: `R4.emit.host` (D1), `L1.A3` (A3)** | — | 2 | segura no mundo fechado; falha D1 |
| `defer` (fim no idle; helper atual) | 2: `H1`, `L1.A3` | — | 1 | fecha W5; exige trabalho diferido (+ recheck) |
| `hook` (ganchos em begin/end; helper atual) | 1: `H1` | 1 (`H1`) | — | identidade real; exige ganchos (+ recheck) |

Cobertura pedida: R1–R4 (R4 em 9 rotas, aninhado e emissão), drag próprio normal (`N1`), externo na mesma janela (`E1`), callbacks aninhados (`R1`, `R2`, `R4.nested-clear`), fim durante emissão (`R4.emit.*`, `M2`, `M3`), reinício (`N2` rearm, `N3`), saídas sem `ended` (`X1` morte silenciosa; `X2` `ended` sem reset; `X3` recusas; `X4` recusa com callbacks), group bar (`G1` início tomado pela barra, mesmo/outro membro; `G2` soltar no grupo; `G3` aba arrastada pelo usuário; `G4` início pela barra dentro do fim; `G5` início do Kwinish no foco da barra), decoração de terceiro (`H1`), início dentro do fim (`B1`), unload (`U1`–`U3`), limite A3 (`L1`).

**Mutações** da candidata (cópia em scratch; compila, roda sem sanitizer, célula declarada muda): **11/16 detectadas** — 10 por O1/O2/`s`; `begin-ambiguity-veto` só pela marca informativa `o` (sessão órfã), não por oráculo de segurança. Detecção = célula alterada, não prova de que cada salvaguarda é necessária; 5 redundâncias explicadas: consumir antes do fim e guarda `inEnd` cobrem-se mutuamente (o par removido é detectado por `R4.emit.own`); `++acquisition` no desarme e checagem ouvinte/geração em `owns()` são redundantes com o desarme por aquisição; a rechecagem de token no idle é redundante com `owns()`.

## 5. R4

Preservado: `bash docs/port-0-57/evidence/Q4A-followup-tests/run.sh` **continua exit 1** (2 falhas R4 por assertiva no GCC; o runner para no primeiro compilador) — `Q4A-policy-old-runner-R4.log`. Nenhum arquivo antigo alterado. No estudo novo, `C3-prev` falha todas as rotas R4; `veto-rc` passa as rotas W2 e `R4.emit.own`, mas **falha `R4.emit.host`** (variante W5); `defer` e `hook` passam as rotas R4.

## 6. Limitações

Réplica manual, não o host: sem hit-test, ordem real de decorações, Lua real, `updateDragWindow` completo, cancelamento de eventos de botão, nem laço real de eventos. Classificação das janelas é leitura estática de uma versão. A exclusão de despacho recursivo do laço (`wl_display_run` só em EventLoopManager.cpp:137; nenhum `wl_event_loop_dispatch` em `src/`) é **hipótese** para bibliotecas externas. `PolDefer` não verifica o cancelamento real no unload (o callback enfileirado ainda captura o objeto). Não modelados: C6, exceções do host, *wrap* do token, rollback das flags dos chamadores quando o início é vetado (`handleMovement` marca `m_bDraggingThis` sem olhar o resultado) e o cancelamento da soltura pelo Kwinish (`handleUpEvent`): com a candidata, um drag da **group bar** iniciado durante o início do Kwinish (`G1`) não é encerrado pelo plugin e o host só o encerra num botão **não cancelado** — falha de liveness a decidir antes de produção.

## 7. Veredito

**Q4-A continua BLOQUEADO para implementação.** Com as APIs 0.57 existentes, sem trabalho diferido nem ganchos, `veto-rc` é a política mais forte encontrada; sua garantia de não encerrar drag alheio depende de A3′ e D1 (terceiros), que o plugin não pode verificar, e exige mudar o helper P8-B. Não é garantia de liveness.

## 8. Alternativas (decisão do dono)

1. **Aceitar A3′ + D1** (mesma classe da A3 anterior) e usar `veto-rc` síncrona; documentar o risco com plugins que ouvem `DragController::ended` ou decorações `DRAG_START` que iniciam drag.
2. **Fim diferido** (`doLaterLock` + cancelamento real em `PLUGIN_EXIT`): fecha W5; muda a ordem dos efeitos (o fim ocorre após o restante do handler, ex. `snapOnRelease`); exige autorização de lifecycle e ainda o recheck.
3. **Ganchos de função** em `CDragStateController::dragBegin/dragEnd`: identidade real (também cobre A3); exige autorização de hook/lifecycle, verificação de *inlining* e ainda o recheck do helper.
4. **Upstream:** validação da sessão **atual** ou fim condicional (ex.: id de sessão devolvido pelo início e `endDragTarget(id)` que só encerra se for a atual). Payload em `ended` **não** basta, nem com id: chega tarde para um ouvinte anterior em W5; payload de alvo também não (R4 reusa o alvo). Mover a emissão antes do reset exigiria protocolo de reentrada; guarda em `dragBegin`.
5. **Não encerrar e não cancelar a soltura sem posse** (delegar ao host): liveness pelo comportamento nativo; muda a entrega do botão ao cliente.

## 9. Revisão independente

Sessões novas Hermes / `openai-codex` / `gpt-6.1-sol` (família diferente da desta sessão, Claude), `--safe-mode`, `--max-turns 1`, sem ferramentas de escrita; estática (não executou código). Prompts preservados em `Q4A-policy-review-prompt-*.txt`.

- **Rodada 1 — `conditional`** (`Q4A-policy-review-1.json`): 1 major + 4 minor. **Major reproduzido e aceito:** decoração `DRAG_START` que inicia drag e devolve `false` entre a guarda e o `beginDragTarget` do helper ⇒ substituição pelo Kwinish (`H1`; O1 em todas as políticas com o helper atual). Tratamento: A3 reforçada para A3′; variante `veto-rc` com recheck; mutante `helper-target-recheck` detectado. Minors aceitos: W4 herda a flag e pode manter o alvo; W3 depende do caminho completo de `setClientResizingState` (trecho :52–57 acrescentado); O3 é diferencial; payload de alvo não resolve; `PolDefer` não prova cancelamento; recursão do laço é hipótese. Avaliação do revisor: O1/O2 independentes, a suíte não premia “nunca encerrar”; veredito bloqueado considerado honesto; nenhuma sequência adicional encontrada além de W5 dentro das premissas.
- **Rodada 2 — `approve_as_planning_basis`** (`Q4A-policy-review-2.json`): só minors. Major da rodada 1 aceito como **pré-condição documentada**, não correção implementada; nenhum contraexemplo novo dentro de A1, A3′, A4, A5, D2 além da família D1 (sequência `R4.emit.host` reproduzida pelo revisor); D1 é o resíduo da janela ambígua, não o único pré-requisito de produção. Minors tratados depois: comentário do código passou a dizer “dois oráculos independentes + verificação diferencial”; mutação detectada só pela marca `o` explicitada; alternativa upstream corrigida (id em `ended` não fecha W5). Essas correções **não foram re-revisadas**. Aprovação = base de planejamento host-free; não autoriza produção, mudança do helper, listeners, `PLUGIN_EXIT`, K3/K9/rearm, trabalho diferido ou ganchos.

Prompts enviados conferidos byte a byte com `~/.hermes/state.db` (sessões `local-d531bf37…` e `local-2f39dac6…`).
