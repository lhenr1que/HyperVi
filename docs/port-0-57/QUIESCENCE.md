# Condições de quiescência Q1–Q5 (tarefa 1.3)

Formaliza o design 9.2 com observável, método de verificação e limite. Preparatório; nada instrumentado ainda. Legenda de evidência em [README](README.md). Recursos referenciados: [LEDGER](LEDGER.md).

## 1. Fatos novos que sustentam Q1–Q3 [N]

| Fato | Evidência |
|---|---|
| Existe **uma única** `CRenderContext` em todo `src` do 0.57.0: o membro `IHyprRenderer::m_context`. Nenhuma outra declaração, `make*` ou `new` | `U:render/Renderer.hpp:249`; busca em `*.cpp/*.hpp` do commit `0aa6745f` |
| Sessões não aninham: `beginRender` recusa se `m_context.active()`; `CRenderContext::begin` retorna `false` se ativa | `U:render/Renderer.cpp:1862–1868`; `Context.cpp:14–22` |
| `reset()` faz `m_pass.clear()`, zera `m_currentPass`, `m_backdropCaptures`, `m_active=false` | `U:render/Context.cpp:66–89` |
| Os únicos donos de `CRenderPass` fora de `m_pass` são o `transformedPass` local (`Renderer.cpp:705,714`), movido para `CTransformedWindowPassElement::pass` (`TransformedWindowPassElement.hpp:10`), isto é, **dentro** de `m_pass` | busca por `CRenderPass` em `src` |
| `CRenderContext::active()` e `m_pass` são públicos; `IHyprRenderer::context()` devolve a única instância | `H:render/Context.hpp`; `H:render/Renderer.hpp:134` |
| `g_pHyprRenderer->m_renderPass` e `m_renderData` **não existem** no 0.57: `PLUGIN_EXIT` (`main.cpp:530–531`) e `CBarPassElement.cpp:15,31` não portam; `removeAllOfType` só existe em `CRenderPass` (`H:render/pass/Pass.hpp:24`) | `grep` em `H:render/Renderer.hpp` |
| Nenhum `wl_event_loop_dispatch`/`..._idle` em `src`; o único `std::thread/jthread` é o player de som (`bell/impl/RotatingPlayerThread.cpp:6`) | busca em `U` |
| O unload administrativo roda **síncrono dentro do handler IPC**: `dispatchPlugin` → `unloadPlugin(PLUGIN)` sem adiamento | `U:ipc/s1/Commands.cpp:1885–1920` |
| Há um **segundo** caminho de unload: `reload()` → `handlePluginLoads()` → `updateConfigPlugins` → `unloadPlugin(p,false)` para plugins com `m_loadedWithConfig` ausentes da config. Ocorre **entre** `preReload.emit()` (:610) e `reloaded.emit()` (:824), fora de ambas as emissões | `U:config/lua/ConfigManager.cpp:610,820,824,1093–1102`; `PluginSystem.cpp:205–209` |

Consequência **condicionada** para a tarefa 1.3: `!context().active()` implica ausência de passes do Kwinish sob H-THREAD, desde que todos sejam criados/enfileirados apenas durante a sessão, sem retenção externa. `transformedPass` existe como proprietário local antes de ser movido para `m_pass`; nesse intervalo a sessão deve continuar ativa. `reset()` limpa os passes antes de marcar o contexto inativo. A simplificação dispensa registry/removeAllOfType somente nesse domínio; contador complementar e auditoria do candidato continuam necessários.

**Adendo da revisão P2:** thread única/origem administrativa não provam ausência de reentrância síncrona. Q3 deve abranger também snapshots de decoração e qualquer proprietário transitório de SP, além de emissões/timers/idles/animações. Q1/Q2 não medem esses retentores. Violação registrada não torna o unload seguro: o host prossegue para `dlclose` sem veto. Revisão estática não fecha B1; relatório independente em `evidence/P2-review-contract.txt`.

## 2. Tabela Q1–Q5

| Q | Enunciado operacional | Observável | Como verificar | Critério de aprovação | Controle negativo | Limitação / hipótese |
|---|---|---|---|---|---|---|
| **Q1** | Nenhuma sessão de render ativa; nenhum passe do módulo vivo | `!g_pHyprRenderer->context().active()` lido em `shutdown()`, **antes** de liberar texturas/passes. Complemento: contador de vida de `CBarPassElement`/`CPreviewPassElement` (ctor/dtor), valendo 0 | **Estático:** o conjunto de fatos da tabela 1 (S4/S5). **Harness:** `shutdown()` registra `active()` e o contador; D1 (unload entre frames) lê ambos | `active()==false` e contador==0 em todo unload administrativo; D4: janela destruída entre enqueue e draw ⇒ contador volta a 0 antes do unload | Variante que enfileira de dentro de um listener e dispara `shutdown()` na mesma pilha ⇒ `active()==true` ⇒ **violação registrada**, não "falha" | Passes do tooltip (`CRectPassElement/CTexPassElement`) são classes do host: não há contador de ctor/dtor; cobertos só por `active()==false`. H-THREAD |
| **Q2** | Nenhum callback do módulo na pilha | Contador `entries` (int, thread única) incrementado por guarda RAII em **cada** ponto de entrada da tabela 3; `==0` ao entrar em `shutdown()` (o próprio `PLUGIN_EXIT` não conta) | **Estático (S2):** script que conta `listen(`, `addLuaFunction`, `createFunctionHook`, `setUpdateCallback`, `makeShared<CEventLoopTimer>` e overrides virtuais de `CHyprBar` e exige que cada um conste na tabela 3 e tenha guarda. **Harness:** D2/D3/D7 | `entries==0` em D1–D3; todo ponto de entrada novo sem guarda reprova S2 | Remover uma guarda de um listener ⇒ S2 falha | Mede o que o plugin enxerga. `strongRef()==1` **não** é medida (design 9.2). Nome de contador/exposição ao harness: decisão pendente (log em `shutdown()` ou função de teste) |
| **Q3** | Nenhuma emissão, cópia `CPY` de timers, idle list movida ou tick de animação retém handler do módulo | **Não observável pelo plugin.** Só argumento estrutural + ensaio | **Argumento:** unload vem de (a) handler IPC — um callback de fd do loop, sem emissão do EventBus em curso por construção, pois não há laço aninhado nem threads (fatos da tabela 1) — ou (b) `reload()` entre as duas emissões. Em nenhum há snapshot nosso vivo, **salvo** se outro código chamou o caminho de unload de dentro de uma emissão (ex.: outro plugin com `invokeHyprctlCommand` num listener). **Ensaio opcional B1-NEG:** plugin sonda cujo listener de `config.reloaded` chama `plugin unload` | Registrar como **premissa do harness** e listar na doc de usuário. Não declarar "garantido" | Sonda B1-NEG: resultado só descritivo | H-Q3, H-DTOR. Auditoria dos chamadores de `Config::mgr()->reload()` **feita [N]**: 6 sítios no host — quatro via `doLater` (`PluginSystem.cpp:131,189`, `PluginAPI.cpp:110`, `ConfigActions.cpp:1255`, execução no idle do loop), um callback de promise do diálogo de safe-mode (`Compositor.cpp:959`) e o comando IPC `reload` (`Commands.cpp:1368`); nenhum está dentro de uma emissão do EventBus. Não cobre código de outros plugins |
| **Q4** | Nenhuma interação dependente do módulo ativa, ou encerrada pelo passo 2 | Flags: `m_bDraggingThis`, `m_bDragPending`, `m_bCancelledDown`, `m_bTouchEv`, `m_unsnapAnim`, `m_szPreviewZone` não vazio, `g_pGlobalState->grabActive`, `grabSwallowPress`, `tip.shown`, timer armado, `m_resistPush` | **Harness:** D2 (tooltip armado/visível), D3 (drag + preview + hook). Após o passo 2: todas as flags falsas e `changeMouseBindMode(MBIND_INVALID)` emitido se drag/grab estava ativo (L17); tooltip: timer sem timeout e `tip.bar` vazio; preview: damage do monitor agendado | Estado limpo ao fim do passo 2; nenhuma chamada ao módulo depois do passo 8 | Pular o reset do mouse-bind ⇒ o host permanece em modo move (detectável por `hyprctl` após o unload) | O passo 2 só reage ao estado do plugin; um arrastar iniciado por atalho do usuário (bind do host) não é nosso |
| **Q5** | Unload emitido por origem externa ao Kwinish | Ausência de chamada à rota de unload no código do plugin | **Estático, comando exato:** `grep -nE 'unloadPlugin\|invokeHyprctlCommand\|dlclose\|plugin unload\|"plugin"' plugin/*.cpp plugin/*.hpp` ⇒ **0 ocorrências** (rodar de novo no candidato). `dispatchIfKnown` (`barDeco.cpp:683`) só recebe literais `fullscreen`, `movetoworkspacesilent`, `exec`. `spawn("hyprctl eval …")` (:618) é processo externo | 0 ocorrências + lista de literais inalterada (S2) | Inserir uma chamada ⇒ o grep reprova | Ação de botão `exec:<cmd>` do usuário pode lançar `hyprctl plugin unload`: origem **externa** (processo filho, entrega pela socket), portanto dentro do contrato. Config-managed + reload: caminho (b) acima, também externo à pilha do plugin |

Violação conhecida de Q1–Q5 ⇒ fora do contrato; o ensaio registra contadores e backtrace, sem classificar como sucesso nem falha de B1 (design 9.2 e 9.5).

## 3. Pontos de entrada a proteger (insumo de Q2/S2)

| Grupo | Entradas | Origem |
|---|---|---|
| Listeners globais (10) | `window.open`, `window.updateRules`, `input.keyboard.key`, `window.close`, `input.mouse.button`, `window.active`, `window.floating`, `render.stage`, `config.preReload`, `config.reloaded` | `main.cpp:369,370,457,468,474,490,496,503,510,511` |
| Listeners por barra (6) | `mouse.button`, `touch.down`, `touch.up`, `touch.motion`, `mouse.axis`, `mouse.move` | `barDeco.cpp:57–64` |
| Hook | `hkOnMouseMoved` (+ `m_original`) | `main.cpp:345–350` |
| Virtuais da decoração chamados pelo host | `getPositioningInfo`, `onPositioningReply`, `draw`, `getDecorationType`, `updateWindow`, `damageEntire`, `getDecorationLayer`, `getDecorationFlags`, `getDisplayName` (+ `onInputOnDeco`, `initializeAnimations`, `updateState`, `onWindowMap`, `onWindowFocus` se sobrescritos) | `barDeco.hpp:34–48`; `H:IHyprWindowDecoration.hpp` |
| Passes | `draw`, `needsLiveBlur`, `needsPrecomputeBlur`, `boundingBox`, `opaqueRegion`, `passName`, `type` de `CBarPassElement`/`CPreviewPassElement` | `BarPassElement.cpp`, `PreviewPassElement.*` |
| Timer | lambda do tooltip | `barDeco.cpp:1274–1281` |
| Animação | callback `damageEntire` | `barDeco.cpp:68` |
| Lua | 7 funções | `main.cpp:154–339` |

`stepUnsnap` é conduzido por movimento do ponteiro (`barDeco.cpp:561–586`), sem timer: não há callback diferido que possa sobreviver ao módulo (coerente com o comentário `main.cpp:460–462`).

## 4. Hipóteses das condições

| ID | Hipótese | Fecha com |
|---|---|---|
| H-THREAD | O compositor chama render, EventBus e IPC numa só thread e o unload IPC não é aninhado em sessão/emissão do host. Sustentada por leitura de `src`; threads de bibliotecas externas (Aquamarine) não auditadas | D1/D6; leitura de Aquamarine se o dono quiser |
| H-Q3 | Nenhuma emissão com snapshot do Kwinish está ativa quando o unload vem do IPC ou do `reload()` | argumento estrutural + auditoria dos 6 chamadores de `reload()` (feita) + B1-NEG |
| H-CNT | Contador por guarda RAII é barato e não altera ordem de destruição | revisão do diff |
