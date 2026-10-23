# Spike (fase 0), notas, 23/10/2026

Plano aprovado pelo dono: Lua no núcleo, Rust na UI (layer-shell), repo novo local, nada de push.

## Achados só de leitura (Hyprland 0.56.2, `/usr/share/hypr/stubs/hl.meta.lua`)
- Eventos Lua (`hl.on`): window.{active,open,open_early,close,destroy,fullscreen,move_to_workspace,pin,title,urgent,update_rules}, workspace.*, monitor.*, layer.opened/closed, keybinds.submap, config.reloaded, **input.keyboard.key**.
- **Novo para R1 (Alt+Tab):** `input.keyboard.key` existe. Se o callback receber tecla e estado (press/release) mesmo com a tecla já tratada por bind, a soltura do Alt pode ser vista em Lua sem submap. A confirmar com um listener que só loga.
- **R3 (fim do arrasto) continua aberto:** não há evento de mouse/botão nem de movimento de janela na lista. Opção a medir: `binds.drag_threshold`, e se `bindm` com release em Lua dispara.
- Ferramentas na máquina: cargo/rustc, gtk4-layer-shell 1.3.0 (já instalado), kitty e grim. Não há `foot`; janela descartável de teste será kitty com título fixo. `hyprpm` não existe (confirma: sem plugin).
- Rust: gtk4-layer-shell existe como pacote do sistema; a UI pode usar os bindings `gtk4-layer-shell` do crates.io. A confirmar versão do crate compatível com 1.3.0.

## Próximos passos do spike (precisam mexer no Hyprland ao vivo, esperando ok)
1. Carregar um listener Lua que só loga `input.keyboard.key` (tecla, estado) em arquivo, e apertar Alt+Tab uma vez. Mede se há release e se há repetição.
2. Abrir um kitty descartável, arrastá-lo com `pointer-path.py` e ver quais eventos aparecem (R3).
3. Compilar um "hello" Rust com gtk4-layer-shell, sem abrir sobre as janelas do dono.

## Resultados (23/10/2026)
- **R1 (positivo):** `input.keyboard.key` entrega `(keycode_xkb, tempo_ms, estado)`, estado 1 = aperta, 0 = solta; keycode = evdev + 8 (Alt_L = 64). A soltura do Alt aparece mesmo com a combinação ligada a um bind. Falta medir key-repeat (segurando) e o caso Alt+Tab real.
- **Incidente:** `hyprctl eval` guardando o objeto devolvido por `hl.bind` e chamando `:unbind()` derrubou o Hyprland 0.56.2 do dono (SEGV em `CLuaKeybind::push`; modo seguro). Regra: nada de objetos de bind via eval na sessão real; `hl.on` por eval foi seguro.
- **Teste headless de R3: inviável do jeito tentado.** `AQ_BACKENDS=headless` não foi respeitado: o Hyprland subiu como janela aninhada (monitor `WAYLAND-1`, 891x1020) dentro da sessão do dono, e `hl.dsp.window.drag()` sem ponteiro físico não faz nada.
- **R3 segue em aberto.** Opções: (a) arrastar um kitty descartável na sessão real com uinput e só um listener `hl.on` (sem binds); (b) achar o backend headless certo do aquamarine e outra forma de injetar ponteiro.

## R3, teste na sessão real (23/10/2026, kitty flutuante descartável, uinput, só listener `hl.on`)
- Pressionar o botão sobre a janela, disparar `hl.dsp.window.drag()` por `hyprctl dispatch` e mover o ponteiro **não moveu a janela**: o dispatcher de arrasto só funciona dentro de um bind de mouse (`bindm`), não chamado de fora.
- **Nenhum evento Lua** apareceu durante o press, o movimento e o release (só `window.active/open` no início). Confirma: o Lua do 0.56.2 não expõe início nem fim de arrasto, nem botão do mouse.
- Consequência para o encaixe ao arrastar (item 7): sem evento, só dá para **inferir** o fim (posição da janela estável por alguns ms, via timer `hl.timer`), que é o que o Ryoku já faz e tem o defeito de disparar com o botão ainda pressionado. Alternativas a medir depois: `hyprctl cursorpos` mais estado do botão por fora do Hyprland, ou um `bindm` de usuário com a nossa ação no release. Item 7 fica marcado **experimental** na v1.
- Janela de teste fechada. O listener Lua de log continua carregado até o próximo reload da config (só grava em `spike/events.log`).

## Fase 1, fatia 1 (23/10/2026): `core/kwinish.lua` + `core/geometry.lua`
- `kw.tile(where)` (left right top bottom topleft topright bottomleft bottomright max) e `kw.maximize_axis("h"|"v")`; repetir o mesmo pedido restaura o retângulo original.
- Matemática pura em `geometry.lua`, com `lua5.4 core/test_geometry.lua` (passa). Hyprland-side testado ao vivo com kitty flutuante descartável: metades, canto, eixo horizontal e restaurar deram os retângulos esperados (área útil = reserved + gaps_out; monitor 1920x1080, reserved 60/10/10/10, gaps 18).
- Limites: restaurar guarda o retângulo numa tabela Lua (perde-se no reload; trocar por tags); `tile` em janela tiled a torna flutuante; não testado com vários monitores nem com escala diferente de 1.
