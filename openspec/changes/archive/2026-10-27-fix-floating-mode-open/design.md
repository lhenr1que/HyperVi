# Design

## Context

Hoje o modo floating age depois da abertura: `hl.on("window.open")` agenda um temporizador de 150 ms ("a janela precisa de um momento para mapear"), que chama `apply_workspace_mode` → `float_for_mode`. Essa função sai cedo se a janela já flutua, está em tela cheia ou fixada; senão marca `kwinish-mode-floated`, faz `float enable` e põe `modes.open_rect(area, passo)` (ou o retângulo salvo, que só existe em janelas que já passaram pelo modo). Quando o temporizador roda, o Hyprland já pôs a janela no dwindle e já desenhou quadros (medido: mapeada em mosaico aos 105 ms, flutuante aos 267 ms; ver proposal.md).

O Hyprland 0.56.2 oferece dois caminhos para decidir antes do primeiro quadro, ambos sem documentação nos stubs (`/usr/share/hypr/stubs/hl.meta.lua` só declara os nomes):
- o evento `window.open_early`;
- `hl.window_rule({ name, match = { workspace = ... }, ... })`, que o compositor aplica no mapeamento.

Restrição de segurança: em 23/10 guardar e chamar métodos no objeto devolvido por `hl.bind` via `hyprctl eval` derrubou a sessão do dono (memória `hyprland-eval-keybind-crash`). `hl.on` por eval foi seguro. Nenhum experimento desta mudança roda na sessão do dono.

## Goals / Non-Goals

**Goals:**
- A janela nova nasce flutuante e marcada, sem passar pelo mosaico (spec `workspace-modes`).
- Manter o temporizador de 150 ms como rede de segurança: se o caminho antecipado não agir, o comportamento volta a ser o de hoje, nunca pior.

**Non-Goals:**
- Usar o tamanho que o aplicativo pede: o modo continua impondo 64% x 68% (decisão abaixo).
- Mudar a saída do modo, `window.move_to_workspace`, regras por app (`rules.json`), colunas ou os modos tiling/scrolling.
- Mexer no plugin.

## Decisions

1. **Experimento antes de escolher, num Hyprland aninhado.** Um Hyprland 0.56.2 com config mínima (como `spike/hl-test/hyprland.lua`), aberto pela sessão do dono num workspace escondido (`hl.exec_cmd("Hyprland -c <cfg>", { workspace = "9 silent" })`), com um kitty aberto dentro dele e amostragem por `hyprctl -i <instância> clients -j` a cada ~15 ms. Perguntas: (a) `window.open_early` dispara antes de a janela entrar no layout e antes do primeiro quadro, já com endereço, workspace e monitor? (b) `float enable` + `resize`/`move` dentro dele são aceitos sem derrubar o compositor, e a primeira amostra já sai flutuante no retângulo? (c) uma `hl.window_rule` criada por `hyprctl eval` vale para a próxima janela, e redeclarar o mesmo `name` com `enabled = false` a desliga, sem tocar no objeto devolvido? Uma queda no aninhado não afeta a sessão do dono.

2. **Preferido: tratador de `window.open_early` registrado pelo núcleo ao carregar a config**, ao lado dos `hl.on` que já existem, chamando a mesma lógica de `float_for_mode` (marca, flutua, põe o retângulo). Vence se (a) e (b) passarem: não cria estado no compositor, não depende de objetos de regra e não exige nada ao mudar o modo pelo menu. Alternativa rejeitada se (a) ou (b) falharem.

3. **Plano B: uma regra de janela nomeada por workspace floating** (`kwinish-mode-floating-<ws>`: `float`, a etiqueta `kwinish-mode-floated`, tamanho e posição centralizada em pixels calculados para o monitor do workspace), declarada ao carregar a config para cada workspace de `workspace-modes.json` (como o laço de `hl.workspace_rule` que já existe) e em `set_workspace_mode`. Para desligar, redeclarar o mesmo nome com `enabled = false`; **nunca** guardar nem chamar métodos no objeto devolvido. Só se (c) passar. A cascata, que uma regra fixa não faz, fica com o temporizador (deslocamento de 24 px, sem mudar o tamanho).

4. **Rejeitadas:** encurtar o temporizador (ainda sobra pelo menos um quadro em mosaico, e o atraso existe porque a janela pode não estar mapeada); layout flutuante (o Hyprland não tem); gancho no mapeamento dentro do plugin C++ (exige recarregar o plugin e espalha a política de modos, que é do núcleo, por duas camadas). Se nem o caminho 2 nem o 3 passarem, a mudança para e volta ao dono com o resultado do experimento.

5. **Tamanho do modo mantido.** O dono reclamou do tamanho de mosaico, não do retângulo do modo; usar o tamanho pedido pelo aplicativo mudaria a spec e é decisão do dono.

6. **Teste que falha primeiro:** o simulador `hl` de `core/test_kwinish.lua` passa a emitir o evento antecipado (caminho 2) ou a aplicar regras com `match.workspace` (caminho 3) antes de pôr a janela no mosaico simulado, e registra a primeira geometria vista. O teste abre uma janela num workspace floating e exige que a primeira geometria já seja flutuante, com `open_rect`; falha com o código atual. Testes de diálogo flutuante e de tela cheia confirmam que nada muda para eles.

## Risks / Trade-offs

- [`window.open_early` chega sem workspace ou monitor] → o tratador não faz nada e o temporizador age como hoje.
- [O compositor cai ao flutuar dentro do evento] → só se descobre no aninhado; nesse caso, plano B.
- [Tratamento duplo pelo evento antecipado e pelo temporizador] → `float_for_mode` já sai cedo para janela flutuante; um teste cobre o temporizador depois do evento antecipado.
- [Recarregar o núcleo por `hyprctl eval` duplica tratadores] → armadilha conhecida (`docs/HANDOFF.md`); o teste ao vivo usa `fish tools/dev-reload.fish` e termina com `hyprctl reload`.
- [Plano B: regra nomeada não substitui a anterior] → (c) descobre no aninhado; sem isso o plano B não entra.

## Migration Plan

Sem migração: `workspace-modes.json` e as etiquetas não mudam. Desfazer é reverter o commit e recarregar a config.


## Resultado do experimento

Executado em 27/10/2026, somente num Hyprland **aninhado**, v0.56.2,
commit `efb50993780079460b0cbed1363e2166a2de1d9f`. Config e executor:
`spike/nested-open/hyprland.lua` e `spike/nested-open/run.py`; evidências em
`spike/nested-open/results/`. Nenhum plugin foi carregado, nenhum núcleo foi
carregado na sessão do dono e nenhum arquivo de `~/.config` foi alterado.
No dono, cada `eval` continha apenas
`hl.exec_cmd("Hyprland -c <config temporária>", { workspace = "9 silent" })`.
Todas as chamadas experimentais selecionaram a assinatura aninhada por `-i`.

### Respostas

- **(a) Sim, para o kitty comum observado.** Em
  `results/baseline-settled/events.jsonl`, `open_early.before` tem endereço,
  workspace 1 e monitor `WAYLAND-1`, `mapped=true`, `floating=false`,
  `fullscreen=0`, `pinned=false`, posição `[0,0]` e tamanho `[0,0]`.
  O evento seguinte, `open`, já mostra o tamanho de mosaico `[1827,1000]`.
  A primeira amostra em `baseline-settled/baseline-first.json` está em mosaico.
  A saída aninhada é única; o backend Wayland usou 1827×1000, apesar da
  solicitação de modo 1280×800 na config. A amostragem sozinha não comprova
  todos os quadros; a ordem dos eventos mostra o estado anterior ao layout.
- **(b) Não.** Na rodada `early-mutate`, o compositor recebeu **SIGSEGV**
  durante `float enable` + `resize` + `move` em `open_early`. O log tem apenas
  `open_early.before`; não houve `open_early.after` nem `open`, e o socket
  deixou de responder antes da primeira amostra mapeada. O relatório de queda
  aponta `Layout::CSpace::resizeTarget` → `CLayoutManager::resizeTarget` →
  `Config::Actions::resize`. Trecho preservado em
  `results/early-mutate/crash-summary.txt`. Não se deve usar o caminho 2.
- **(c) Sim, para ativar/desativar o float de um kitty comum.** Em
  `results/rules`, ambos os `eval` responderam `ok`. A regra foi criada como
  `hl.window_rule({ name = "kwinish-nested-floating-1", match = { workspace = "1" }, float = true })`
  e desativada com
  `hl.window_rule({ name = "kwinish-nested-floating-1", enabled = false })`.
  Nenhum objeto retornado foi guardado e nenhum método nele foi chamado.
  `enabled-first.json`: primeira amostra aos 105,06 ms, flutuante,
  `[1827,1000]`, sem etiqueta. `disabled-first.json`: aos 105,07 ms,
  em mosaico, `[1827,1000]`. O aninhado sobreviveu às duas janelas.
  Esta rodada comprova a troca da regra, não o tamanho, a etiqueta, a cascata
  nem as exclusões de diálogos/fullscreen/pinned do plano B completo.

### Caminho escolhido e impedimento

O **caminho 3 é o mecanismo candidato sobrevivente**, pois o caminho 2 caiu
e a criação/desativação da regra funcionou. A aplicação fica **pausada antes
da tarefa 2.1**, por conflito entre os artefatos que precisa de decisão do dono:

1. A decisão 3 acima manda aplicar tamanho/posição fixos pela regra e fazer
   a cascata no temporizador. A spec exige que a segunda janela já apareça
   deslocada; a tarefa 2.1 exige primeira geometria igual a `modes.open_rect`
   (com a cascata), e a tarefa 2.2 proíbe novo move/resize pelo timer.
2. `float_for_mode` retorna cedo quando a regra já fez a janela flutuar;
   portanto esse timer, mantido como está, nem realiza a cascata prometida
   pelo desenho. Alterar isso exige reconciliar o desenho e os testes.
3. A regra `float` + etiqueta por workspace precisa distinguir diálogos
   naturalmente flutuantes. O experimento com kitty comum não provou essa
   exclusão; reivindicar todos violaria a spec.

Não foi escrito um teste que aceita uma exceção a esses requisitos nem um
conserto parcial. As tarefas 2–4 permanecem abertas. Também não se declara
que nenhum mecanismo funcionou: (c) passou, mas o plano B descrito ainda não
cumpre o conjunto dos requisitos. O próximo passo depende de revisar o desenho
preservando a cascata desde a abertura, ou de o dono autorizar uma revisão
explícita desse comportamento.

### Isolamento e limpeza

A sessão já tinha dois kitties do dono antes do experimento; eles foram
preservados. A verificação correta é ausência de clientes **adicionados** e
do processo aninhado, em vez de fechar janelas preexistentes para obter uma
lista vazia. `cleanup.json` em `baseline-settled`, `early-mutate` e `rules`
registra `added_owner_clients=[]`, `nested_left=false` e
`existing_kitty_preserved=true`. A sessão do dono manteve sua assinatura.

A primeira rodada (`results/baseline`) registrou um falso negativo de limpeza:
a instância aninhada já tinha encerrado, mas o dono ainda processava o fechamento
assíncrono da superfície `aquamarine`. O executor passou a aguardar a remoção
da superfície desse PID; a repetição `baseline-settled` terminou com código 0.
O executor também encerra apenas processos cujo argumento de config coincide
com o caminho temporário único, inclusive se a inicialização não publicar socket.


### Encerramento e opções para retomar

Em 27/10/2026, após ver o resultado, o dono pediu **encerrar por enquanto com
o experimento registrado** e anotar as alternativas. Nenhuma delas foi
autorizada para implementação nesta sessão:

1. **Revisar o desenho mantendo a cascata desde a abertura.** Preservar a spec
   e os testes propostos; encontrar e comprovar no aninhado um mecanismo que
   aplique o retângulo completo antes do primeiro quadro, sem o `resize` que
   derrubou `open_early`. Também comprovar as exclusões de diálogos,
   fullscreen e pinned antes de qualquer implementação no núcleo.
2. **Aceitar a cascata após 150 ms.** Revisar explicitamente spec, design e
   tarefas para permitir nascer flutuante no tamanho do modo, com posição
   inicialmente fixa, e só depois deslocar pela cascata no timer. A tarefa
   2.2 precisaria permitir esse movimento e os testes da primeira geometria
   precisariam refletir o comportamento aceito. Ainda seria necessário
   resolver a identificação das janelas pertencentes ao modo, preservar
   diálogos sem etiqueta e evitar redimensionar novamente no timer. Esse
   caminho não foi implementado nem validado como solução completa.

Validação ao encerrar: `bash tools/check.sh` terminou com **código 0** no
checkout `kwinish-floating`: 9 suítes Lua (Lua 5.5.1), 3 suítes de ferramentas,
18 testes Rust e compilação do plugin em diretório temporário. É validação
simulada/de compilação da base existente, não prova de conserto do modo
floating. O teste de regressão das tarefas 2.1/2.2 e o conserto continuam
pendentes. `git diff --check` passou. Sem commit, arquivamento ou carga de plugin.

### Complemento (Claude, 27/10): tamanho pedido pelo aplicativo

O dono perguntou se a janela poderia abrir no tamanho que o próprio aplicativo pede. Duas rodadas a mais no mesmo executor, só no aninhado (`results/rules-class`, `results/rules-size`; `cleanup.json` sem janelas acrescentadas na sessão do dono), com o kitty de `--config NONE` e cache vazio, que sozinho escolhe 640x400:

| Regra | Primeira amostra (105 ms) |
|---|---|
| `match = { workspace = "1" }`, `float = true` (rodada `rules`, acima) | flutuante, 1827x1000, o tamanho do mosaico |
| `match = { class = "kwinish-nested-probe" }`, `float = true` | flutuante, 640x400, centralizada |
| `match = { workspace = "1" }`, `float = true`, `size = "819 544"`, `center = true` | flutuante, 819x544, centralizada |

Conclusão: o Hyprland manda à janela o tamanho previsto para o mosaico antes do primeiro quadro, a menos que já saiba que ela vai flutuar. Uma regra por classe sabe disso a tempo; uma regra por workspace só age no mapeamento, e o aplicativo então "pede" o tamanho do mosaico. Abrir no tamanho do próprio aplicativo exigiria uma regra para todas as classes ligada e desligada conforme o workspace ativo for floating: frágil (erra janelas abertas em outro workspace e corre contra a troca de workspace) e sem distinguir diálogos. Já `size` + `center` numa regra por workspace faz a janela nascer flutuante no tamanho dado, sem passar pelo mosaico: o retângulo do modo fica comprovado como mecanismo.

### Decisão final (27/10, dono) e rodadas `rules-mode` e `rules-offset`

O dono escolheu o caminho 3, com a cascata desde a abertura feita por redeclaração da regra. Antes de codificar, a regra que o núcleo vai usar foi conferida no aninhado (`results/rules-mode`, `results/rules-offset`; sessão do dono sem janelas acrescentadas):

- Regra `{ name, match = { workspace = "1", float = false, fullscreen = false }, float = true, size = "819 544", move = "100 100", tag = "+kwinish-mode-floated" }`: o kitty nasce flutuante em (100,100) com 819x544, etiquetado. Redeclarada com `move = "124 124"`, o kitty seguinte nasce em (124,124): a cascata vale desde o primeiro quadro.
- Janela GTK4 de tamanho fixo, que o Hyprland faz flutuar sozinho como a um diálogo: com a regra ligada continua 300x150 e sem etiqueta. `float = false` no `match` exclui as janelas que já nascem flutuantes.
- Kitty com `--start-as=fullscreen`: nasce no retângulo da regra, etiquetado, e vira tela cheia ~100 ms depois. `fullscreen = false` no `match` não o exclui, porque o pedido de tela cheia chega depois do mapeamento. Resultado aceito: a janela fica em tela cheia e, ao sair dela, volta flutuante no retângulo do modo (com o código atual ela cairia no mosaico, mesmo num workspace floating). A spec foi ajustada; o `fullscreen = false` sai da regra.
- `enabled = false` com o mesmo nome desliga a regra: a próxima janela volta ao mosaico.
- Monitor em (500,300): `move = "100 100"` pôs a janela em (600,400). O `move` da regra é relativo ao monitor.
- A etiqueta posta por uma regra aparece como `nome*` (dinâmica), e o núcleo compara nomes exatos. Por isso a regra põe só a marca `kwinish-mode-new`, e o núcleo, ao ver a janela abrir com ela, grava a etiqueta persistente `kwinish-mode-floated`, que já é a que ele gerencia.
- Nas rodadas `kitty2` e `off`, o kitty se maximizou ~200 ms depois de abrir: é o estado que o próprio kitty salva no HOME temporário do aninhado, cuja config não tem o `suppress_event = "maximize"` da config do dono. Não é efeito da regra.

Implementação no núcleo:
- `mode_rule(ws, on)`: regra nomeada `kwinish-mode-floating-<ws>`, `match = { workspace, float = false }`, `float`, `size` e `move` de `modes.open_rect(work_area(mon), mode_cascade)` com o `move` relativo ao monitor, e a marca `kwinish-mode-new`. `mon` é o monitor do workspace (`hl.get_workspace`) ou o ativo, se o workspace ainda não existe. O objeto devolvido nunca é guardado.
- Declarada ao carregar a config para cada workspace floating, de novo em `monitor.added` (na partida o monitor pode ainda não existir) e em `set_workspace_mode`; desligada com `enabled = false` quando o workspace sai de floating.
- No `window.open`, sem esperar: janela com a marca recebe `kwinish-mode-floated`, a cascata avança e a regra do workspace é redeclarada com o próximo retângulo.
- O temporizador de 150 ms fica como está: para uma janela que a regra não pegou, faz o que faz hoje.

### Verificação com o núcleo (27/10, Claude)

- Teste que falha: com o simulador aplicando regras, `born floating at the mode rectangle` falhou com o código anterior e passa com o conserto; `bash tools/check.sh` sai 0 (9 suítes Lua em 5.5, também passam em 5.4).
- Núcleo do repositório no aninhado (`results/core-mode`, monitor de 1827x1000 e workspace 1 floating): a janela A nasce flutuante em (315,112) com 1100x680 (o `open_rect` do passo 0) e a etiqueta `kwinish-mode-floated`; a B, aberta com A ainda aberta, nasce em (339,136); A não se mexe quando a regra é redeclarada; o diálogo de tamanho fixo fica 300x150 sem etiqueta; `set_workspace_mode(1, "tiling")` devolve A e B ao mosaico, guardando os retângulos, e deixa o diálogo flutuante; uma janela aberta depois nasce em mosaico. O Lua vê a etiqueta da regra como `kwinish-mode-new*`, como o simulador modela.
- `results/core-mode-1280x800`: com o modo do aninhado diferente da janela que a sessão dá a ele, o backend redimensiona a saída depois do `monitor.added` sem nenhum evento para o Lua (ordem registrada em `events.jsonl`), e só a primeira janela sai com o retângulo de 1280x800. Num monitor físico o `monitor.added` já traz o modo real; a regra também é recalculada em `monitor.layout_changed` e depois de cada janela aberta.
