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
