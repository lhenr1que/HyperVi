# Design

## Context

Base conferida em 30/10/2026: branch `port-kwinish-bars-to-hyprland-0-57`, HEAD `bf2049a`; 105 entradas preexistentes no status. T1/M e T2/W estão commitadas. Fontes: `docs/DEVELOPMENT_STATE.md` (T2/T1/Q4-A1/Q4-A/P8-B), `docs/port-0-57/P8-DRAG.md`, `docs/port-0-57/evidence/Q4A1-alt5-contract.md`, e plano externo `/home/lhen/Projects/kwinish-plan-hyprland-0.57/continuidade-2026-10-30/{PLANO.md,codex-direction-review.md}`. O plano escolhe D1; o parecer recomenda D1 completo e distingue o débito floating. Contagens editoriais 9/3 do contrato Q4-A1 divergem da matriz 10/4; esta proposta não depende dessas contagens nem corrige o original.

Q4-A1 mostra por modelo que remover apenas os fins (`naive5`) prende drags; `alt5-rc` continua falhando liveness sob captura. Não é aprovação de implementação nem prova ao vivo. Nenhuma garantia baseada em identidade de janela/modo, A3′ ou D1 do estudo Q4 é aceita aqui (essa premissa D1 não é a alternativa D1 de desligamento).

## Goals / Non-Goals

**Goals:** delimitar alpha-0 sem drag da decoração e do menu, mantendo botões e menu utilizáveis e entrada pareada; permitir futura T4 com testes vermelhos primeiro, sem fim não comprovado.

**Non-Goals:** fazer o plugin inteiro compilar nesta etapa, portar R, mudar unload/lifecycle, portar ou corrigir `hostDrag.hpp`, conter floating do núcleo, habilitar fallback GTK como substituto do drag, mudar formatos ou configurações pessoais. T3 é somente proposta pendente de revisão independente de outra família e aprovação do dono.

## Decisions

### 1. Exclusão em compilação e inventário fechado

T4 removerá as rotas ou as excluirá por condição de compilação fixa da alpha-0; um gate somente runtime não serve, pois as referências incompatíveis continuariam compiladas. Não adicionar opção runtime ou ativação automática. Remover declarações/estado estritamente associados é parte de D1; não usar a tarefa para limpeza geral.

As linhas abaixo foram conferidas em `bf2049a`, são âncoras históricas e devem ser localizadas por símbolo ao implementar:

| Parte excluída | Pontos da base / alcance fechado |
|---|---|
| K1 | `barDeco.cpp:664–681`, `handleMovement`, chamada `KwDrag::begin` :676; inícios no press e movimento pendente de `handleDownEvent`/`onMouseMove` |
| K2 | `main.cpp:304–322`, `luaGrab`, chamada `KwDrag::begin` :317 (P8-B citava :315 antes de T2); foco/seleção e aquisição do grab indisponíveis |
| K3 | `barDeco.cpp:278,566,586,630,649`; `main.cpp:484`; nenhum substituto `endDragTarget` |
| K9 | `barDeco.cpp:381`, dispatcher `mouse` com `0movewindow`, junto do ramo touch/drag externo ao retângulo |
| Rearm | pares :278–279 e :586–587; `m_rearmPending` em movimento e listener Shift `main.cpp:455–466` |
| Unsnap | `maybeUnsnap`, `stepUnsnap`, restauração/resize/posição/tempo e saída de maximizada por limiar em `onMouseMove`; não mexer na restauração explícita pelo núcleo |
| Soltura/snap | `snapOnRelease`, chamada :654, escolha de edge/zone/monitor, restauração de posição para snap e dispatch ao núcleo originados pelo drag |
| Preview e resistência | ativação de `drawSnapPreview` pelo drag, `holdAtSeam` e encaminhamento por `hkOnMouseMoved`; não instalar hook de resistência na alpha-0; tipos/passes/render GL continuam no grupo R, sem portá-los ou apagá-los para obter verde |
| Toque | `onTouchDown/Up/Move` :199–212/:327–345 e ramos touch dos handlers: retirar a interação por toque da decoração nesta alpha, inclusive conversão de coordenadas, float/resize/pin/move/tile e cancelamentos próprios; não converter em clique sintético |
| Flags/estado | impedir qualquer aquisição ou armação de `m_bDraggingThis`, `m_bDragPending`, `m_bTouchEv`, `m_touchId`, `m_rearmPending`, `grabActive`, `grabSwallowPress`; excluir estado auxiliar de drag/unsnap/resistência/preview e seus consumidores |
| Cancelamentos de drag | retirar os do título usados para preparar drag, os de toque, as guardas grab no início de `onMouseButton` e o swallow de press/release do listener PG (`main.cpp:481–488`); preservar em PG a parte geral de tooltip (:477–480) |

`m_bCancelledDown` mistura drag e cliques de ações: não pode ser apagada indiscriminadamente sem substituir o rastreamento de cliques consumidos. Estado de pareamento de botão não é flag de aquisição. Depois da exclusão, press simples no título vazio não arma operação nem consome par para drag; foco/elevação e duplo clique configurado continuam. Movimento sem gesto continua atualizando hover/tooltip.

### 2. Entrada pareada

Para mouse, toda interação de botão da barra/ícone/menu consumida pelo Kwinish deve consumir também a soltura correspondente, inclusive se foco ou ponteiro mudarem. Eventos não consumidos devem passar em par. Associar o consumo à origem e ao botão, sem roubar release de outro dispositivo/botão/barra; não usar uma flag global de drag como substituto. Ação ocorre uma única vez por clique. Botão direito abre menu e mantém seu par coerente (hoje a soltura direita é ignorada). Botões meio/laterais continuam passando. Não cancelar clique que servia somente para terminar grab, pois não há grab.

Não se promete que cliques nas ações preservadas nunca afetem drag nativo: floating ainda pode encerrá-lo indiretamente. A garantia de ausência de begin/end cobre rotas de drag da decoração/menu, não o interior de ações do host preservadas.

### 3. Menu e entrada Lua

Ocultar, na composição dos itens de `ui/src/menu.rs` (`build`, linhas :261–262), `grab:move` e `grab:resize` na alpha-0; não apenas tornar a chamada inerte em `core/kwinish.lua`. Conferir o índice de inserção do atalho por app (`root.insert(5, ...)`), pois a remoção de duas linhas altera a ordem. Preservar fechar, minimizar, maximizar, workspaces e demais ações.

Manter registro e assinatura de `hl.plugin.kwinish.grab(mode,address)` para não remover a superfície registrada em T1: argumentos inválidos continuam diagnosticados; argumentos válidos recebem diagnóstico explícito de indisponibilidade pela via de erro Lua já existente, antes de foco/início/flags. O núcleo pode continuar encaminhando chamadas explícitas a essa entrada; não precisa mudar para esconder itens que a UI constrói. Não instalar timer/hook novo para simular movimento ou dar fallback silencioso.

### 4. O que permanece e saída

Título, ícone, botões, tooltips, foco/elevação, roda, duplo clique configurado, abertura do menu e ações não relacionadas ao grab continuam. Encaixe explícito pelo teclado/menu, zonas, restauração e Snap Assist via menu continuam; não apagar `remember`/`forget`/zonas compartilhados só por terem consumidores de drag. Os payloads `tile:*@monitor`/`zone:*@monitor` mantêm interpretação; apenas não são emitidos por soltura da barra na alpha-0. Specs atuais de monitor do soltar seguem válidas para ações explícitas. A exceção ao cenário Shift+drag de Snap Assist fica no delta.

Os nomes, tipos, padrões e dados de `snap_top_edge`, `snap_preview`, `seam_resistance` e `preview_color` permanecem; seus efeitos ligados ao drag ficam inativos nesta alpha. Não alterar Settings para criar capacidade dinâmica. Não mudar barras fallback GTK; não atribuir a elas o aceite D1 do plugin. “Só bind nativo” refere-se a mover interativamente a janela na alpha-0 com plugin; não proíbe ações discretas de encaixe/workspace.

Reativação somente na alpha-1, após decisão explícita Q4/T10, recheck de `target()` depois de `DRAG_START` em T9 (regressão H1/H1i), novo contrato/revisão e validação de entrada. T9 sozinho não demonstra posse nem desbloqueia fim de drag. Não apagar o helper nesta proposta. `floatWindow` Lua (e ações C++ preservadas que usam floating) continua débito conhecido; alpha-0 não recebe garantia O1 global. `PLUGIN_EXIT`/Q4-L17 permanece fora da tarefa.

## Risks / Trade-offs

- Perda deliberada de drag/resize pelo menu e de toque na decoração → ocultação e comunicação explícita; movimento interativo pelo bind nativo já configurado, sem prescrever nova configuração.
- Remover fins sem inícios/cancelamentos cria falso D1 → harness com spies e mutações das rotas reais, mais inspeção estrutural.
- Mexer no handler inteiro pode quebrar botões/tooltip ou deixar release órfão → matriz de pares e casos de foco/dispositivo/barras sobrepostas.
- Fins indiretos de floating persistem → débito expresso, sem afirmar isolamento de todas as ações.
- Preview inalcançável ainda precisa compilar → R permanece independente; T4 não garante `.so`.

## Migration Plan

Aguardar revisão independente e aprovação do dono para proposta e eventual commit documental. Depois pedir autorização própria para T4, incluindo os arquivos de teste e atualizações de API/CONFIGURAVEL. T4 começa pelos testes abaixo vermelhos no baseline, implementa D1 e demonstra verde sem retocar R. Build e teste ao vivo não pertencem a T3; validação integrada/build ficam no plano T7 e roteiro do dono em T8. Não produzir roteiro de carga para candidato que ainda não compila. Não sincronizar nem arquivar esta mudança automaticamente. Reativar drag não é rollback automático: exige os gates alpha-1.

## Validation Plan

Todos os testes abaixo são futuros e host-free; nenhum foi executado em T3. Exercitar código real extraído/compartilhado com os handlers, não somente reescrever uma política em modelo. Spies de begin/end/dispatcher, cancelamento, foco, movimento, snap e estado deverão registrar a sequência completa por evento. Checagem textual isolada é complemento, não prova comportamental.

| Requisito | Vermelho primeiro em T4 | Aceite e não-vacuidade |
|---|---|---|
| D1 completo | press/motion/release em flutuante, tiled, maximizada, snap prévio; Shift, bordas, dois monitores simulados e grab move/resize | zero begin/end/dispatcher mouse, zero flags de aquisição/rearm, zero unsnap/snap/preview/resistência; reintroduzir cada início, fim, flag e cancelamento exclusivo deve falhar por assertiva |
| Toque excluído | down/move/up com IDs distintos sobre título/botão | zero ação/cancelamento/float/resize/pin/move oriundos de toque; mutação reativa rota e falha |
| Pareamento | ações dos botões, ícone e menu direito; release fora, foco mudado por minimizar, dois ponteiros/botões, barras sobrepostas e clique após grab recusado | mesma identidade de par consumida, zero press/release órfãos por intervenção do plugin, ação uma vez; casos negativos deixam ambos passar; mutações de release e consumo indevido falham |
| Menu explícito | teste puro da árvore de itens de menu Rust, sem GTK/display, incluindo classe com atalho e modos | ausentes grab:move/resize; presentes e ordenadas demais ações; mutação reinserindo item falha |
| Entrada Lua | chamar corpo real de luaGrab com Lua/host falso, válidos e inválidos | válido informa indisponibilidade, inválido mantém diagnóstico; nenhum foco, begin/end, flag ou swallow; teste não se satisfaz com retorno silencioso |
| Bind nativo preservado | simular sequência nativa com barra presente, sem ação preservada de floating, e comparar com host sem rotas de drag Kwinish | plugin não muda delta, não adquire/encerra nem engole eventos pelo antigo grab/drag; não prova bind real do dono |
| Funções preservadas | regressões geometria/tooltip/ações e casos existentes de snap/Assist pelo menu/teclado | nenhum novo gatilho por Shift+drag; ações explícitas continuam com mesmo payload/monitor; mutação que apaga todas as ações falha |

As suites antigas de equivalência P8-B esperam K1/K2 ativos: não usá-las como aceite D1 nem apagar sua evidência histórica. Registrar a mudança intencional de oráculo nos testes ativos. Quando houver compilação de harness, erro de compilação não conta como mutante morto; registrar separadamente falha por assertiva. Captura simulada não prova captura real nem GL/lifecycle.

## Open Questions

Premissas para o revisor independente conferir: inventário fechado cobre todas as entradas e efeitos de drag; exclusão de toque integral é a redução pretendida; ocultação fixa na UI é adequada à alpha do plugin sem ampliar para fallback; via de diagnóstico Lua é suficiente e não introduz nova assinatura; pareamento por origem/botão é exequível nos eventos disponíveis; deltas reconciliam o gatilho legado Shift+drag sem reduzir snap explícito; opções preservadas e débito floating estão descritos sem prometer O1; T4 requer autorização dos arquivos de estado/testes/documentação listados. Nenhuma dessas conferências é declarada revisão independente nesta proposta.
