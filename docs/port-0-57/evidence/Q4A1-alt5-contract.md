# Q4-A1 — Alternativa 5: não pedir fim de drag sem posse demonstrável (estudo host-free, 30/10/2026)

**Escopo autorizado:** testes host-free, documentação e revisão. **Nenhum código de produção alterado**; K3/K9/rearm, `PLUGIN_EXIT`, helper `KwDrag::begin`, render/GL intocados; nada carregado; sem commit. Base `f76c785`.

**Decisão do dono (registrada, não implementada):**
- a alternativa 5 é a candidata prioritária;
- A3′ e D1 **não** são aceitas como garantia;
- `veto-rc` fica só como referência experimental;
- upstream (id de sessão + fim condicional) fica como solução futura.

**Fontes:** `U:` Hyprland 0.57.0 `0aa6745f`; trechos numerados do host e das rotas atuais do plugin em `Q4A1-sources.txt`.

**Código:**
- `Q4A1-tests/q4a1test.cpp` (reusa sem alterar o controlador de `Q4A-policy-tests/`);
- `Q4A1-tests/run.py`;
- `Q4A1-tests/expected-matrix.txt`.

**Saídas:** `Q4A1-run.log`, `Q4A1-detail.txt`, `Q4A1-detail-verbose.txt`. **Revisão:** `Q4A1-review-*.json`.

## 1. Fatos do host (0.57)

- `CInputManager::onMouseButton` emite `mouse.button` no barramento **antes** de tudo; se um ouvinte cancela, o host não processa o evento (U:InputManager.cpp:720–724).
- **Captura de entrada** (InputManager.cpp:736–741) é testada antes do ramo de soltura sem press: chama `onMouseEvent(…, captured=true)` e retorna. Com captura, `endDragTarget()` **não** roda (Manager.cpp:353).
- Sem captura, todo evento de botão chega a `onMouseEvent`, que chama `endDragTarget()` sem condição (Manager.cpp:353). O evento não é repassado ao cliente se havia drag (:369/388/393/400/416).
- Soltura cujo press não foi registrado no host (press cancelado pelo Kwinish) segue InputManager.cpp:748–751: encerra o drag e retorna **sem enviar ao cliente**.
- Todo evento de tecla encerra o drag em Manager.cpp:223, **inclusive com captura** (só `handleInternalKeybinds` depende dela, :214). Exceções que retornam antes do fim (revisão 2): sessão inativa (:190–195), teclado com `m_allowBinds=false` (:197–198), falha de tradução de estado (:203), keybind interno consumido (:215) e teclado virtual com `DISALLOWACTION` (InputManager.cpp:1718–1722 nem chama `onKeyEvent`). O plugin não observa qual ocorreu.
- **Inventário fechado dos pontos que encerram drag** (grep em todo `src/`, `Q4A1-sources.txt`):
  - `Window.cpp:1673` (unmap da própria janela arrastada);
  - `ConfigActions.cpp:253` (`floatWindow`, se o estado muda: **qualquer** drag);
  - `ConfigActions.cpp:1700` (`Actions::mouse` na soltura: qualquer drag);
  - `Manager.cpp:223` e `:353`;
  - internos do controlador (`DragController.cpp:125,176,182,345`).

## 2. Mapa de eventos e consumidores (Etapa 1)

| # | Fluxo | Entrada do host | Rota atual do Kwinish | Cancela? | Pede fim? | Sob a alternativa 5 (desenho modelado) |
|---|---|---|---|---|---|---|
| 1 | press na barra | bus `mouse.button` | `onMouseButton`→`handleDownEvent` (barDeco.cpp:154–197, 347–427); início K1 :676 (flutuante no press; ladrilhada/maximizada no 1º movimento) | **sim** (:397) | não | igual, mas **cede** se já há drag ativo (não cancela, não marca flags) |
| 2 | soltura após press na barra | bus | `handleUpEvent` (:625–662) | **sim** (:639–640) | **sim**, K3 :649 (fim incondicional) | não cancelar, não pedir: host encerra (:748–751 → :353), salvo captura |
| 3 | soltura com foco mudado | bus | :626–636 | não | sim, K3 :630 | não pedir |
| 4 | press fora da barra durante drag | bus | :376–389 | não | sim, K9 :381 (dispatcher `mouse`) | não pedir (o mesmo press encerra no host) |
| 5 | botão direito / roda na barra | bus | press direito na barra cancelado e abre o menu (:184–193; soltura ignorada); roda :215–226; meio/laterais ignorados (:160–161) | sim | não | press direito **cede** se há drag ativo (sem menu); roda igual |
| 6 | movimento | bus `mouse.move` (InputManager.cpp:292) | rearm :273–280 (fim+início), unsnap :566 e :586–587, início pendente :323–324 | não | **sim** (rearm/unsnap) | rearm só início se `target()` nulo e sem unsnap pendente; unsnap decidido e animado **antes** do início, abortado se houver drag ativo |
| 7 | teclado | bus `keyboard.key` (:1712); depois Manager.cpp:223 | `PK` (main.cpp:458–467): Escape limpa `grabActive`; Shift arma rearm em **todas** as barras | não | não | limpa `grabActive` em qualquer tecla (o host normalmente encerra o grab nela; exceções no §1); Shift arma só barra em drag; soltura limpa o rearm |
| 8 | grab do menu | `luaGrab` (main.cpp:302–320, K2); `PG` (:475–490) | grab armado **mesmo se o início falhar**; press cancelado :485, soltura engolida :488 | sim/sim | sim, main.cpp:484 | grab só armado com `STARTED`; se o drag já sumiu, o clique passa; senão press cancelado e **soltura passa** |
| 9 | toque | bus `touch.*` | `onTouchDown/Up/Move` (:199–212, 327–345): ações, sem controlador de drag | sim (down) | **indireto**: `floatWindow` :337, :380, :652 | guarda `target()` |
| 10 | botão “float” / duplo clique | `runAction("float")` :696 | `floatWindow` encerra **qualquer** drag (ConfigActions.cpp:249–253) | — | **indireto, fora de K3/K9** | guarda `target()` |
| 11 | **núcleo Lua** | `hl.dsp.window.float` → `LuaBindingsDispatchers.cpp:494–495` → `CA::floatWindow` | `core/kwinish.lua:88` (`place`), `:391` (`window_action` float), `:590` (`float_for_mode`), `:639` (`set_workspace_mode`) | — | **indireto** (qualquer drag) | **não modelado**; o Lua não tem `target()`: exige guarda/decisão própria |
| 12 | fechar janela | `runAction("close")` | unmap → Window.cpp:1672–1673 encerra só o drag **daquela** janela | — | consequência nativa do fechamento | igual |
| 13 | group bar | aba: bind → `Actions::mouse` → `DRAG_START` → CHyprGroupBarDecoration.cpp:392–423; ou via `KwDrag::begin` | — | — | cur: K3 encerra o drag da barra | host encerra na soltura |
| 14 | fim natural pelo host | tecla, botão, `floatWindow`, `Actions::mouse`, unmap, internos (§1) | — | — | — | igual |
| 15 | decorações | `INPUT_TYPE_BUTTON` (InputManager.cpp:863), `DRAG_START`, `DRAG_END` (DragController.cpp:287) | a barra não implementa `onInputOnDeco` | — | — | igual |
| 16 | unload | `PLUGIN_EXIT` | hoje não encerra | — | não | não encerra; a soltura seguinte chega ao host |
| 17 | hook de movimento | `hkOnMouseMoved` (main.cpp:440–456) | altera delta | — | não | igual |

Não auditados transitivamente (revisão 1): implementações completas de `fullscreenWindow`, `closeWindow`, `alterZOrder`, ações configuráveis de duplo clique e `window_action` além de `float`. Pelo inventário fechado do §1, só alcançariam um fim via `floatWindow`, `Actions::mouse`, unmap da própria janela ou internos do controlador. Isso **não foi provado** para cada ação.

## 3. Modelo e oráculos (Etapa 2)

Camada de entrada replica o §1:
- barramento primeiro;
- captura;
- soltura sem press;
- `onMouseEvent`;
- `pass_mouse_when_bound`;
- teclas com e sem `m_allowBinds`;
- botões identificados por **(ponteiro, botão)**.

Rotas do plugin copiadas das linhas citadas.

| Variante | Significado |
|---|---|
| `absent` | sem plugin (base nativa) |
| `cur-0.56` | **encerramento solicitado pelo Kwinish**: rotas atuais |
| `veto-rc` | referência experimental (posse + veto), rotas atuais |
| `naive5` | só remove os pedidos de fim, mantém cancelamentos |
| `alt5` | **sem pedido de fim + encerramento natural pelo host**: rotas da alternativa 5, helper atual |
| `alt5-rc` | `alt5` + recheck de `target()` no helper |

**Oráculos** (camada de verdade, inacessível ao plugin):
- **O1:** nenhum drag de terceiro encerrado ou substituído por chamada do Kwinish, inclusive indireta via `floatWindow`.
- **O2:** nenhum drag do Kwinish vivo em **qualquer** ponto em que a interação acabou (botões soltos, sem grab, flags limpas), além do estado final.
- **O3:**
  - por evento: o Kwinish cancelou um evento que encerraria um drag alheio **já ativo antes do evento**;
  - no fim: botão preso no host;
  - par press/soltura inconsistente no cliente por causa do plugin;
  - drag sobrevivendo a evento cancelado.
  - Peculiaridades nativas (press engolido com drag ativo e soltura repassada, e o inverso) são anotadas, não atribuídas ao plugin.
- **O4:** flags do host e do plugin coerentes no fim, incluindo contador do unsnap e flag de rearm (revisão 2).
- **`h` (informativo):** drag vivo com modo `INVALID` deixado pelo `dragEnd` envolvente (R3, causado pelo host).
- **`s`:** retomada após rearm; unsnap realizado e não animado durante drag alheio.
- **Mutação conta só se o oráculo declarado passa a falhar** naquela célula.

## 4. Resultados (`python3 docs/port-0-57/evidence/Q4A1-tests/run.py`, exit 0)

GCC 16.2.1 e Clang 23.1.4, `-Werror`, ASan/UBSan: build 0, execução 0, sem saída de sanitizer, saídas idênticas. Matriz de **39 cenários × 6 variantes** igual à esperada. Cenários com falha:

| Variante | O1 | O2 | O3 | O4 | s |
|---|---|---|---|---|---|
| `absent` | — | — | C1, C2 (captura nativa: cliente fica com o press) | — | — |
| `cur-0.56` | **10**: F1 G1 H1i L1i M4 M5 RA2 T1 US3 Y1 | K2 K3 | B5 R5 Y1 | M4 RA3 | US3 |
| `veto-rc` | F1 L1i | K2 K3 | **10**: B5 G1 H1i M4 M5 R5 RA2 T1 US3 Y1 | M4 RA3 | RA1 US3 |
| `naive5` | F1 H1i | **18** | **24** | M4 RA3 | 4 |
| `alt5` | H1i | C1 C2 K2 K3 | — | — | — |
| **`alt5-rc`** | **—** | **C1 C2 K2 K3** | **—** | **—** | **—** |

**Leitura dos resultados:**
- Remover só os pedidos de fim (`naive5`) prende drags. A alternativa 5 **exige mudar as rotas de entrada**.
- `cur-0.56` encerra drags de terceiros em 9 cenários.
- `veto-rc` não encerra indevidamente pelas rotas K3/K9, mas prende entrada e não cobre `floatWindow`.
- `alt5-rc` passa O1, O3 e O4 em todos os cenários. **Falha O2 em três:**
  - **C1/C2 (captura de entrada na soltura):** o host não encerra o drag do Kwinish. Ele fica vivo até a próxima tecla (C2) ou clique não capturado. `cur-0.56` encerra, então é **regressão de liveness** da alternativa 5.
  - **K2/K3 (tecla que não chega a Manager.cpp:223 — binds desativados, `DISALLOWACTION`, keybind interno — durante o grab, sem clique depois):** falha em **todas** as variantes do Kwinish; preexistente. Com clique depois (K1/K4) termina.
- O4 não acusa nada em nenhuma variante; a não-vacuidade vem só da mutação `alt5-keeps-bar-flag`.

**Mutações** de `alt5-rc`: **15/15 detectadas pelo oráculo declarado**:
- voltar a cancelar a soltura (O2);
- cancelar a soltura do grab (O2);
- manter pedidos de fim (O1);
- `floatWindow` sem guarda (O1);
- rearm encerrando antes (O1);
- sem redesenho do unsnap (s);
- unsnap sem abortar (s);
- rearm durante unsnap pendente (s);
- barra sem ceder a drag ativo (O3);
- flag de grab mantida após tecla (O3);
- flag da barra não limpa (O4);
- botão direito sem ceder (O3, R5);
- grab armado sem `STARTED` (O3, M5);
- flag de rearm mantida após a soltura (O4, RA3);
- sem recheck do helper (O1).

**Preservação:**
- O runner reexecuta `Q4A-policy-tests/run.py`: exit 0, matriz 42 × 7 inalterada (R1–R4, H1, L1), 11/16 mutações como antes.
- `Q4A-followup-tests/run.sh` continua **exit 1** com as 2 falhas R4 (`Q4A1-old-runner-R4.log`).

## 5. Recheck de `target()` em `KwDrag::begin` (Etapa 3)

- **Isolado:** `H1i` (e `H1` do estudo anterior). Uma decoração de terceiro inicia drag no `DRAG_START` e devolve `false`.
  - Com o helper atual, o `beginDragTarget` do Kwinish substitui a sessão dela (O1 em `cur`, `naive5`, `alt5`).
  - Com o recheck (`alt5-rc`, `veto-rc`), devolve `ALREADY`.
  - A mutação `helper-recheck` é detectada nas duas suítes.
- **Group bar nativa:** devolve `true` e já cai em `REFUSED_BY_DECO`. O recheck só importa para decorações de terceiros e não altera o caso sem drag iniciado.
- **Conclusão:** **necessária** para O1, independente da política de fim, **não aplicada**. Não garante posse nem o alvo certo: o helper escolhe a janela por hit-test do ponteiro.

## 6. Viabilidade da alternativa 5 (Etapa 4)

| Pergunta | Resposta |
|---|---|
| Implementável com as APIs atuais? | **Para as rotas do plugin C++, sim no modelo**: sem novo ouvinte, trabalho diferido ou gancho. Usa o fim nativo e `target()` como guarda. |
| Preserva arrastar/redimensionar? | **Parcialmente.** Sem captura: soltura, rearm, group bar, grab e `pass_mouse_when_bound` funcionam no modelo. Com captura ativa na soltura, o drag sobrevive até a próxima tecla/clique não capturado (C1/C2): regressão frente ao atual. As diferenças do §7 dependem de teste ao vivo. |
| Evita encerrar operações de terceiros? | **Por pedido do plugin C++, sim no modelo** (O1 limpo), com as guardas de `floatWindow`. **Não coberto:** os `floatWindow` do núcleo Lua (#11) e ações não auditadas transitivamente. O host continua encerrando drags alheios em todo evento não cancelado: regra nativa. |
| Exige mudanças nas rotas de input/decorações? | **Sim**, todas em produção e não autorizadas (lista abaixo). Decorações do host não mudam. |
| Depende de Q4-B ou de lifecycle? | Não depende de novo mecanismo de lifecycle (U1). “Q4-B” não está definido nos docs; tratado como a metade de unload de Q4. Exige **reconciliar** o critério legado Q4/L17 (“emitir `MBIND_INVALID` no unload”): decisão documental do dono. |

Mudanças que a alternativa 5 exige em produção (não autorizadas):
- remover K3 (:630, :649, main.cpp:484) e K9 (:381);
- não cancelar soltura (:639–640, :488);
- barra ceder a drag ativo no press esquerdo **e direito** (sem menu durante drag alheio);
- grab só armado quando o início retorna `STARTED`;
- rearm armado só na barra em drag e limpo na soltura;
- `grabActive` limpo em qualquer tecla e clique de grab sem drag repassado;
- rearm só início, sem unsnap pendente e conferindo que o ponteiro está sobre a janela da barra;
- unsnap antes do início;
- guarda nos `floatWindow` C++;
- decisão para os `floatWindow` do Lua;
- recheck no helper.

## 7. Limitações remanescentes

- **Teclas que não chegam ao fim nativo (K2/K3):** o grab pode ficar vivo até o próximo clique. Preexistente; a alternativa 5 não piora nem resolve.
- **Fronteira do O3:** só conta drags já ativos antes do evento. Um ouvinte do barramento anterior ao Kwinish que inicie drag no mesmo press e tenha o press cancelado pelo Kwinish não é contado (revisão 2, minor).
- **Captura de entrada (C1/C2):** sem prova de posse, o plugin não pode encerrar o próprio drag quando o host ignora a soltura. `PROTO::inputCapture->isCaptured()` é visível nos headers instalados, mas só diria que o host **não** encerrará. Liveness sob captura fica pendente de decisão.
- **Núcleo Lua:** `floatWindow` via `hl.dsp.window.float` encerra qualquer drag ativo e não há `target()` no Lua. Fora do modelo.
- **Ordem:** `snapOnRelease` passa a rodar antes do fim pelo host. O efeito real de `Actions::move` sobre alvo arrastado (DefaultFloatingAlgorithm.cpp:147–149, 235–244; `setTargetGeom` no `dragEnd`) não é decidível host-free.
- **Grab** termina na soltura do clique, não no press. Clique depois de o host já ter encerrado o grab passa ao aplicativo (antes era engolido).
- **Hit-test:** rearm e unsnap redesenhado chamam o helper, que escolhe a janela sob o ponteiro. Depois que o host para o move (Shift), o ponteiro pode estar sobre outra janela. Preexistente; exige conferir a janela antes do início. Não modelado.
- **Peculiaridades nativas:** press engolido com drag ativo e soltura repassada (B5, M3); com captura, o cliente fica com o press (C1/C2 em `absent`).
- **Defeito atual achado:** com dois ponteiros, `cur`/`veto-rc`/`naive5` cancelam a soltura do segundo por causa de `m_bCancelledDown` do primeiro, deixando botão preso (B5).
- **Fidelidade do modelo:** réplica manual. Não modela hotkeys, binds reais além de um bind de group bar, toque com controlador, `updateDragWindow` completo, hit-test, nem as ações não auditadas. R4 sobre fim do Kwinish não se aplica à alternativa 5 (não há entradas de fim). Drags de terceiro reiniciados dentro do fim do host continuam sendo comportamento do host (R4i, `h`).

## 8. Veredito e recomendação

A alternativa 5 resolve, **no modelo e para o plugin C++**, a segurança (O1) e o bloqueio de entrada (O3), sem lifecycle novo. **Não** satisfaz liveness em todos os casos: sob captura de entrada o drag do próprio Kwinish sobrevive à soltura (regressão). O núcleo Lua ainda pede fins indiretos. Pela regra do dono, **Q4-A continua BLOQUEADA**.

Recomendação: **redesenho delimitado antes de qualquer implementação**. Decisões necessárias:
1. política sob captura: aceitar o atraso até a próxima tecla/clique não capturado, ou outra solução sem fim não comprovado;
2. guarda/decisão para os `floatWindow` do Lua;
3. aceitar as diferenças do §7 (grab na soltura, ordem do snap, clique pós-grab);
4. conferência de janela no rearm.

Com isso decidido, a próxima etapa seria uma **implementação limitada** autorizada à parte, com testes do repositório vermelho-primeiro e roteiro ao vivo do dono.

## 9. Revisão independente

Hermes / `openai-codex` / `gpt-6.1-sol`, sessões novas, `--safe-mode`, `--max-turns 1`, sem ferramentas de escrita; estática.

- **Rodada 1 — `conditional`** (`Q4A1-review-1.json`): 6 major + 1 minor, todos aceitos.
  - **Reproduzidos e corrigidos no modelo:**
    - captura de entrada (C1/C2; virou limitação de liveness);
    - closure `expectKw` pendente (UB real, agora função estática);
    - O3 só terminal (agora por evento, sobre drag já ativo);
    - botões sem identidade de ponteiro;
    - O2/O4 só terminais (O2 em todo fim de interação; O4 com contador do unsnap; `h` para modo `INVALID` do host);
    - conflito rearm × unsnap pendente (US4, corrigido no desenho);
    - mutação passa a exigir o oráculo declarado.
  - **Também aceitos:** inventário de fins indiretos incompleto (§1 inventário fechado, núcleo Lua #11, ações não auditadas) e recomendação exagerada (§6/§8 rebaixados).
  - **Minor:** recheck não garante janela certa (hit-test), registrado no §5/§7.
  - **Ajustes de desenho surgidos ao corrigir:** barra cede a drag ativo (Y1); grab limpo em qualquer tecla (M4).
- **Rodada 2 — `conditional`** (`Q4A1-review-2.json`): confirma o tratamento da rodada 1 e julga **honesto** manter Q4-A bloqueada com redesenho delimitado. Encontrou 3 major + 2 minor, todos aceitos:
  - **Major — botão direito** (rota do menu cancelava evento que encerraria drag alheio): reproduzido (R5, O3 em `cur`/`veto-rc`/`naive5`); desenho corrigido (cede); mutação.
  - **Major — grab armado com início falho:** reproduzido (M5); desenho corrigido (só com `STARTED`); mutação.
  - **Major — teclas que não encerram** (`DISALLOWACTION`, :203, :215): reproduzido (K3/K4); limitação de liveness preexistente, documentada no §1/§7.
  - **Minor — fronteira do O3:** registrado no §7, não modelado.
  - **Minor — `rearmPending` fora do O4:** reproduzido (RA3, M4: defeito preexistente em todas as variantes do plugin); O4 ampliado; desenho limpa a flag; mutação.
- Uma 3ª rodada não foi feita; as correções da rodada 2 não foram revisadas externamente.
