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

## Fase 1, fatia 2 (23/10/2026)
- `kw.pack(left|right|up|down)`, `kw.show_desktop({filter=})`, `kw.keep_above()` + `kw.setup()` (registra os listeners; chamar uma vez).
- Testado ao vivo com dois kitties descartáveis (vermelho e azul): empacotar, esconder e restaurar pela workspace especial `special:kwinish_desktop`, e manter acima (o vermelho continuou sobre o azul focado, conferido por captura de tela).
- **Achados:** `hl.dsp.window.bring_to_top` não eleva a janela de forma visível; o que funciona é `alter_zorder({mode="top"})`, e só depois do evento `window.active` (timer de 30 ms, porque o evento dispara antes de a nova janela focada ser elevada). Cuidado ao recarregar o módulo: `package.loaded` zera o estado (a tabela de restaurar).
- Limites: estado em tabelas Lua (perde-se no reload); `show_desktop` guarda a workspace de origem, não o foco; "manter abaixo" não feito; grow/shrink fica com o dispatcher nativo `resize relative` (sem código novo).

## Fase 1, fatia 3 (23/10/2026)
- `kw.setup{placement="smart"|"cascade"|"center"|"cursor", autoraise_ms=N, filter=fn}`. Colocação roda em `window.open` (timer de 30 ms, relê a janela por endereço para não segurar objeto destruído); auto-raise reeleva a janela flutuante focada depois de N ms se ela continuar focada.
- Testes puros (`lua5.4 core/test_geometry.lua`) cobrem center, cursor, cascade e smart. **Ao vivo:** colocação inteligente testada com dois kitties (o segundo foi para a direita, sem sobrepor o primeiro).
- **Não verificado ao vivo: auto-raise.** Com `follow_mouse=1` o ponteiro virtual do teste não trocou o foco (a janela ativa continuou kwb com o cursor sobre a kwa), então o caminho de reelevação não foi exercitado. Testar com mouse de verdade ou revisar o motivo (possível `float_switch_override_focus`).
- Os listeners de `setup` ficam registrados até o próximo reload da config; no teste foram limitados a janelas de classe `kw*` pelo `filter`.

## Alt+Tab (24/10/2026)
- **Decisão de arquitetura:** o overlay não pega o teclado. O Lua (`core/switcher.lua`) é dono de teclas e foco (bind `ALT + Tab` chama `step`, a soltura do Alt chega por `input.keyboard.key`), escreve `$XDG_RUNTIME_DIR/kwinish/switcher.json`, e o `kwinish-ui` (Rust, gtk4 0.11 + gtk4-layer-shell 0.8, `KeyboardMode::None`) só desenha o arquivo. Isso evita o problema do T7 (reenvio de teclas ao ganhar foco e soltura dentro de submap) por construção: um Alt+Tab é um `step`, sem foco de teclado.
- **Testado ao vivo** (3 kitties descartáveis, `step` por eval, soltura e Esc por uinput): o overlay apareceu centralizado com a segunda janela destacada; soltar o Alt focou a janela escolhida e fechou o overlay; Esc cancelou sem mudar o foco; dois passos selecionaram o terceiro. Segurança: timeout de 15 s fecha sem focar.
- **Não testado:** o bind `ALT + Tab` real (não registrado na sessão por causa do crash do `hl.bind`), key-repeat segurando Tab, miniaturas (hoje cartões com classe e título), janelas em outros workspaces/monitores, ordem MRU depois de reload (parte vazia, completa com a ordem do compositor).
- Testes puros: `lua5.4 core/test_switcher.lua`.

## Menu da janela (24/10/2026)
- `kwinish-ui menu <endereço>` (processo de uma vez só, `ui/src/menu.rs`): camada transparente em tela cheia pega clique fora e Esc, menu no ponteiro, cada item roda `hyprctl eval "require('kwinish').window_action(addr, ação)"`. Ações: maximizar, metades, eixo h/v, manter acima, flutuar/ladrilhar, tela cheia, mover de workspace, fechar. Segurança: foco de teclado exclusivo, mas sai sozinho em 20 s.
- **Testado ao vivo** (kitty descartável): o menu abriu no ponteiro, clicar em "Tile left" ladrilhou a janela (902x1024) e o processo saiu; Esc fechou; clique fora fechou (checado pela ausência do processo depois).
- **Não testado:** bind `ALT + F3` real, vários monitores (posição usa `hyprctl cursorpos` em coordenadas globais, sem deslocar pelo monitor), submenus, "criar regra" (item 12 do catálogo), mover/redimensionar pelo menu, teclado (setas) dentro do menu, tema claro. Rótulos alinhados à esquerda no último build, sem nova captura.

## Empacotamento e colisões (24/10/2026)
- `packaging/PKGBUILD` (rascunho local, `url` é placeholder, licença declarada MIT **sem arquivo LICENSE: o dono ainda não escolheu**): `makepkg --nodeps` gerou `kwinish-git` com `/usr/bin/kwinish-ui`, `/usr/share/kwinish/{core,recipes}`, README. Os testes Lua rodam no `check()`. Não instalei o pacote.
- **Colisão com o Ryoku do dono (conferido com `hyprctl binds`):** com `mod = "SUPER"`, 10 dos 17 binds padrão já existem (SUPER+ALT+setas, SUPER+ALT+H, SUPER+CTRL+ALT+setas, SUPER+D) e `ALT + Tab` também. Com `SUPER + CTRL` colidem só as 4 setas. Para quem usa Ryoku, a instalação precisa trocar `mod`/`override`; para um Hyprland comum (exemplo oficial) não deve haver colisão, mas não conferi a lista oficial.
- `spike/verify/live-test.lua` é um arquivo para o dono testar os binds reais sem colisão (`SUPER + CTRL + …`, `ALT + grave` para o switcher, `ALT + F3` para o menu). `--verify-config` aceita.
