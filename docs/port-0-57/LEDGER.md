# Ledger de recursos do plugin (tarefa 1.2)

Preparatório; **não** é implementação nem validação. Base: `9850593`, plugin pré-port (0.56.2) lido em 30/10/2026. Legenda de evidência em [README](README.md). Linhas `K:` são desta worktree.

## 1. O que o host faz no unload (0.57.0)

`U:plugins/PluginSystem.cpp:139–189` (`unloadPlugin`), ordem real:

| # | Passo do host | Efeito sobre o Kwinish |
|---|---|---|
| 1 | `PLUGIN_EXIT()` só se `!eject` (:143) | Único ponto de código nosso no unload normal. Sem veto. |
| 2 | `removeAllHooksFrom(handle)` (:158) | Remove o hook de movimento. Trampolim em execução não é revogado. |
| 3 | `removeWindowDecoration` para cada ponteiro de `m_registeredDecorations` (:160–162) | Ver N1 abaixo. |
| 4 | dispatchers e comandos hyprctl registrados (:164–172) | Kwinish não registra nenhum. |
| 5 | `Config::mgr()->onPluginUnload(handle)` (:174–175; `U:config/lua/ConfigManager.cpp:1304–1323`) | Remove valores de config e funções Lua por handle. |
| 6 | `erase` do `CPlugin`, `dlclose` (:182–184) | Destrutores estáticos do módulo rodam aqui (H-DTOR). |
| 7 | `doLater(Config::mgr()->reload())` (:187) | Reload **depois** do `dlclose`: emite `config.preReload/reloaded`. |

**Não retirado pelo host:** listeners do EventBus, timers do EventLoop, efeitos de regra, animações, passes, texturas, o `UP<SGlobalState>`.

**N1 [N].** `m_registeredDecorations` é append-only: `U:plugins/PluginAPI.cpp:152` faz `push_back`; `PluginSystem.cpp:160` lê; nada apaga (`grep` em todo `src`: 3 ocorrências). No unload o host compara cada ponteiro **já liberado** com as decorações vivas de todas as janelas por endereço. Sem dereferência (sem UAF), mas um endereço reciclado por outra decoração (grupo, outro plugin) seria removido. Limitação upstream, fora do alcance do plugin; registrar na documentação de usuário, não corrigir.

## 2. Ledger

Colunas: **Aquisição** (baseline) · **Dono / retentores** · **Invalidação** · **Liberação** (quem, quando) · **Rollback de init** · **Q** (condição que protege) · **Estado**.

| ID | Recurso | Aquisição | Dono / retentores | Invalidação | Liberação | Rollback de init | Q | Estado |
|---|---|---|---|---|---|---|---|---|
| L01 | `PHANDLE` | `main.cpp:353`; `globals.hpp:17` | global do módulo | novo `dlopen` = nova geração | morre com o módulo | n/a | — | Reload = geração nova; não reusar (design 9.5). |
| L02 | `g_pGlobalState` (`UP<SGlobalState>`) | `main.cpp:364` | global inline do módulo; guarda config SP, botões, zonas, tip | — | destrutor estático no `dlclose` | **[N]** `~CHyprBar` o usa (`barDeco.cpp:72–74`) e o host destrói as barras **depois** de `PLUGIN_EXIT` (passo 3) ⇒ `shutdown()` **não** pode resetá-lo; só esvazia o que é seu (tip.timer, tip.tex) | Q2 | Ordem obrigatória no protocolo. |
| L03 | 3 efeitos de regra | `main.cpp:365–367` (IDs `globals.hpp:56–58`) | `Desktop::Rule::windowEffects()` | — | `unregisterEffect` em `PLUGIN_EXIT` (`main.cpp:539–541`) | **ausente**: throw após :367 vaza os IDs (eject não chama EXIT) | — | Lacuna real do baseline; exige rollback explícito. |
| L04 | 29 valores de config | `main.cpp:372–433` (29 `makeShared` + 29 `addConfigValueV2`) | host (`m_configValues`) + SP em `g_pGlobalState->config` | — | host, passo 5 (também no eject) | coberto pelo host | — | O SP do módulo morre em L02 depois do host; deleter: S5. |
| L05 | 7 funções Lua (`add_button`, `remember`, `forget`, `set_zone`, `clear_zones`, `set_bar_hidden`, `grab`); keyword legado só se `CONFIG_LEGACY` (não verificado quem o retira; fora do caminho Lua) | `main.cpp:436–445` | host por handle (funções Lua: verificado no passo 5) | closure Lua antiga resolve **por ID** e falha após remoção (`U:ConfigManager.cpp:1241–1250`) | host, passo 5 | coberto pelo host | Q2 (chamada já iniciada) | Chamada em curso é caso à parte. |
| L06 | Hook `CInputManager::onMouseMoved` | `main.cpp:447–453`; `g_pMotionHook` cru | `CHookSystem` por owner | `hook()` falso ⇒ ponteiro anulado, objeto fica no host até o unload | host, passo 2; plugin desregistra antes (passo 5 do protocolo) | coberto pelo host | Q2 | Símbolo único no binário em execução [N: `nm -D` → 1]. `m_original` público [H HookSystem.hpp:25]. |
| L07 | 10 listeners globais `static auto` | `main.cpp:369,370,457,468,474,490,496,503,510,511` (`P,P3,PK,PC,PG,P7,P6,PR,P4,P5`) | `static` local de `PLUGIN_INIT` | — | **só** destrutor estático no `dlclose` (H-DTOR) | estáticos já construídos ficam até o `dlclose` do eject | Q2, Q3 | Candidato a proprietário central + `disconnect` explícito (protocolo, passo 4). `render.stage` (:503) tem assinatura obsoleta (`eRenderStage` vs `SRenderStageEvent` [H EventBus.hpp:34]). |
| L08 | 6 listeners por barra | `barDeco.cpp:57–64` | membros `CHyprSignalListener` da barra | barra destruída | RAII no destrutor da barra | n/a | Q2, Q3 | OK por RAII; lambdas capturam `this` (`[&]`). |
| L09 | `CHyprBar` | `main.cpp:43–46` (`UP`, `emplace_back` em `vector<WP>` — não compila com `SP&&` do 0.57 [H PluginAPI.hpp:267]) | apresentação da janela (SP) + WP em `bars`, `m_self`, `tip.bar` | janela some / `removeDecoration` (`m_decosToRemove`, `U:WindowPresentation.cpp:119–153`) | host (passo 3) ou plugin (protocolo, passo 7) | `addWindowDecoration` pode devolver `false`: baseline ignora | Q2, Q4 | Snapshots de decoração do host podem reter o SP até saírem de escopo. |
| L10 | Animação de cor `m_cRealBarColor` | `barDeco.cpp:66–68` | a barra; callback captura `this` | barra destruída | RAII | n/a | Q3 | Tick em curso: `U:AnimationManager.cpp:150,165` [EXT]. |
| L11 | Timer do tooltip | `barDeco.cpp:1270–1284` | `tip.timer` (SP) **e** `m_timers` do EventLoop | — | `PLUGIN_EXIT`: `tipHide(true)`, `removeTimer`, `reset` (`main.cpp:533–537`) | n/a (criado sob demanda, nunca no init) | Q3, Q4 | Lambda do timer é código do módulo; cópia `CPY` em curso: Q3. |
| L12 | Estado do tooltip | `globals.hpp:61–71` | `g_pGlobalState->tip` | — | `tipHide`, `PLUGIN_EXIT` | n/a | Q4 | Estado de interação: encerrar no passo 2. |
| L13 | Texturas | `m_pTextTex` (`barDeco.cpp:856`), `m_pIconTex` (:802), `button.iconTex` (:919–923, dentro de `g_pGlobalState->buttons`), `tip.tex` (:1297) | `SP<ITexture>` do módulo; objeto e deleter do host (`createTexture` devolve SP do host) | troca de escala/config | **[N]** `~CGLTexture` chama `makeEGLCurrent()` e só então `glDeleteTextures` (`U:render/gl/GLTexture.cpp:16–28`); retorna cedo em shutdown | n/a | Q1 | Liberar fora de sessão ativa (Q1) é seguro por construção; dentro de sessão troca o EGL corrente. Corrige a premissa "sem contexto, adiar". |
| L14 | Passes: `CBarPassElement` (`barDeco.cpp:964`), `CPreviewPassElement` (:519), tooltip `CRectPassElement`×2 + `CTexPassElement` (:1335–1345) | fila da sessão (`UP`) | `CRenderContext::m_pass` | `m_pass.clear()` em `reset()` (`U:render/Context.cpp:66–89`) | host, no fim/abort da sessão | n/a | **Q1** | Vtable/deleter dos dois primeiros estão no módulo. Ver `QUIESCENCE.md` Q1: **[N]** existe uma única `CRenderContext`; fora de sessão não há passe nosso vivo. |
| L15 | Estáticos locais do módulo | `CConfigValue` em `barDeco.cpp:970` e `BarPassElement.cpp:20`; mapa de ícones `icon.cpp:48`; `static const bool PT` (`barDeco.cpp:1243`) | destrutores estáticos | — | `dlclose` | n/a | — | `CConfigValue` guarda referência ao valor do host: confirmar ordem no `dlclose` (S5/D1). |
| L16 | Mapas `zones`, `restores`, `hiddenBars` (chaves = ponteiro de janela como `uintptr_t`) | `main.cpp:261–297,321–339` | `g_pGlobalState` | limpas em `window.close` (`main.cpp:468–472`; `zones` só por `clear_zones`) | L02 | n/a | — | Sem código do módulo retido; ponteiros reutilizados são risco já tratado no close. |
| L17 | Estado do host mutado: modo de mouse-bind (`MBIND_MOVE/RESIZE`) | `main.cpp:314`; `barDeco.cpp:563–584,627` | `g_pKeybindManager` | fim do arrastar | plugin deve repor `MBIND_INVALID` se `grabActive` ou `m_bDraggingThis` | n/a | **Q4** | Unload no meio deixaria o host em modo move. Passo 2 do protocolo. |
| L18 | `reloadConfig`, `addNotification` | `main.cpp:359,455,521`; `barDeco.cpp:147` | lambda/dados do host (`doLater`) | — | host | n/a | — | Sem código do módulo retido [EXT PluginAPI.cpp:109–111]. |
| L19 | Despacho in-process | `barDeco.cpp:683–686` (`m_dispatchers[name](arg)`), `Config::Actions::*`, `executor()->spawn("hyprctl eval …")` (:618) | transitório | — | n/a | n/a | **Q5** | Nomes de dispatcher são literais; `spawn` é processo externo assíncrono. |

## 3. Pontos de init com throw e o que o eject deixa para trás

Ordem em `PLUGIN_INIT` (`main.cpp:352–524`): versão (:358, throw antes de qualquer aquisição) → estado global (:364) → 3 efeitos (:365–367) → 2 listeners (:369–370) → 29 valores (:372–433) → Lua/keyword (:435–445) → hook (:447–455) → 8 listeners (:457–511) → decoração das janelas existentes (:514–519) → `reloadConfig` (:521).

No eject o host executa os passos 2–6 da tabela 1 (sem o 1). Logo, **só estes ficam órfãos**: L03 (efeitos), L07 (até o `dlclose`, que ocorre em seguida) e L02. Rollback mínimo exigido: desregistrar L03 em ordem inversa e, se L07 passar a ter proprietário central, desconectá-lo. Nada mais precisa de rollback explícito; o baseline já está coberto pelo host nos demais itens. `std::exception` só: throw de outro tipo escapa do `catch` do host (`PluginSystem.cpp:117`).

## 4. Hipóteses deste ledger (não demonstradas)

| ID | Hipótese | Como fecha |
|---|---|---|
| H-DTOR | Destrutores estáticos (L07, L15, L02) rodam no `dlclose`: sem `NODELETE`, sem símbolos `UNIQUE` | S5: `readelf -d`/`nm` no `.so` candidato; D1 emitindo `config.reloaded` após unload (passo 7 do host) |
| H-DEL | Deleters de `UP/SP` instanciados no módulo (passes, timer, valores) não são chamados após o `dlclose` | S5 + D1 |
| H-REG (retificada em P2) | `SP&&` não move propriedade por si: no caminho `false` mostrado não há move/reset. `std::move(spNomeado)` continua pertencendo ao chamador; temporário morre no fim da expressão. Não assumir destruição imediata universal | task 3.3: testar SP nomeado/temporário e rollback do chamador |

**Precisão P2:** manter `g_pGlobalState` até a destruição das barras é necessário, mas não suficiente. `removeDecoration` apaga o SP do vetor, não revoga snapshots/SP externos; estes também precisam terminar antes de `dlclose`. A formulação de §3 sobre rollback mínimo é condicionada à propriedade efetivamente transferida, não a uma garantia da API no retorno falso. H-DTOR/H-DEL continuam abertos. Revisão independente: `evidence/P2-review-contract.txt`.
