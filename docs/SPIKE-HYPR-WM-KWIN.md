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

## Teste com binds reais na sessão do dono (24/10/2026)
Feito com autorização do dono ("Teste"): bloco temporário no `~/.config/hypr/user.lua` (backup, `pcall(dofile, live-test.lua)`), `hyprctl reload`, teclas por uinput; ao final o `user.lua` foi restaurado do backup (0 linhas com "kwinish"), recarregado, symlink e processos removidos.
- **ALT + F3 (menu):** abriu `kwinish-ui menu` para a janela ativa; Esc fechou. OK.
- **Alt+Tab real:** com o bind `ALT + code:49` + Alt segurado, o overlay abriu (cartões kwc, Claude, kwa, kwb, o segundo destacado) e soltar o Alt focou a janela destacada e fechou o overlay. **Um Alt+Tab = um passo** (log do `step`), o problema do T7 não aparece. OK.
- **SUPER+CTRL+ALT+Q (tile topleft):** colocou o kitty em 901x506 no canto e repetir restaurou 350x250 em (900,700). OK.
- **Achado:** em `hl.bind`, `code:N` usa o keycode **xkb (evdev + 8)**: `code:41` (grave) não dispara, `code:49` sim. `ALT + grave` pelo keysym também não foi confirmado (o layout do dono é `us`; não investiguei por quê, o primeiro teste pode ter sido anterior ao bind carregado).
- **Efeito colateral a corrigir:** a lista do Alt+Tab inclui todas as janelas mapeadas, inclusive a do próprio app do dono; é o esperado, mas falta filtrar janelas "fantasma" (popups, janelas sem título, workspaces especiais) antes de dar como pronto.
- Não testado: key-repeat segurando a tecla do passo, SHIFT (passo para trás) com bind real, mostrar a área de trabalho e manter acima por bind real.

## Cantos ativos e filtro do Alt+Tab (24/10/2026)
- **Alt+Tab:** a lista ignora janelas não mapeadas e sem classe e título (popups, tooltips); teste puro cobre isso. Janelas em workspace especial (minimizadas) continuam na lista de propósito.
- **Cantos ativos:** `kw.hotcorners{corners={...}, delay_ms, edge, tick_ms}`. O Lua não vê movimento do ponteiro, então um timer `repeat` de 100 ms lê `hl.get_cursor_pos()`; a detecção de canto (`geometry.corner_at`, vários monitores por retângulo) e o dwell (`geometry.dwell`, dispara uma vez e rearma ao sair) são funções puras, com testes. Ações: `"show_desktop"`, `"menu"`, `{exec=cmd}` ou função.
- **Testado ao vivo** (ponteiro virtual, canto inferior esquerdo, ação `touch`): parado 2 s no canto disparou uma vez; passar pelo canto em 0,1 s não disparou. Custo: o CPU do Hyprland ficou em ~18,8 % com o timer e ~19,0 % sem ele (a linha de base do compositor já é alta; a diferença está no ruído).
- **Limites:** o timer não pode ser parado sem recarregar a config (não seguro o objeto timer, por causa do crash de objetos userdata); a posição do ponteiro vem em coordenadas globais, vários monitores só testados na lógica pura; não conferi se os cantos do Hub do Ryoku disparam junto no mesmo canto; sem bind/config real, só `hyprctl eval`.

## Spike da barra de título (24/10/2026), `ui/src/bar.rs` (`kwinish-ui bar <endereço>`)
Licença: MIT (arquivo `LICENSE`, titular "lhen" como no git; **o dono não escolheu, foi recomendação minha, trocar é só mudar o arquivo e o PKGBUILD**).
- **Soltar o botão chega à barra (R3 resolvido para janelas com a nossa barra):** o log mostrou `RELEASE` com a posição final; o compositor nunca informa isso ao Lua, mas uma superfície layer-shell nossa recebe o clique e o soltar normalmente.
- **IPC não é gargalo:** `dispatch hl.dsp.window.move(...)` direto no `.socket.sock` custa ~0,07 ms (≈13 mil movimentos/s); via processo `hyprctl`, ~4,5 ms. O spike usa o socket.
- **Arrasto exato:** drag denso de 60 passos (~100 Hz, ponteiro virtual) terminou com a janela exatamente no deslocamento do ponteiro; 60 movimentos aplicados, soltar registrado.
- **Dois erros que o spike pegou:** (1) margens de uma superfície ancorada contam a partir da área útil (descontando barras com zona exclusiva, como a do Ryoku); é preciso `set_exclusive_zone(-1)` para usar a origem do monitor. (2) calcular a posição global como origem da barra + coordenada local **acumula erro** (a coordenada local chega antes de a superfície se mexer): o deslocamento deu 169 px para um movimento de 100. A correção é ler a posição global do ponteiro no compositor (`j/cursorpos`, barato) a cada movimento.
- **Não medido:** suavidade visual com mouse real (só ponteiro virtual), quadros perdidos, custo de CPU do overlay; a barra aqui cobre uma faixa acima da janela, sem seguir foco, tela cheia, workspace, vários monitores, XWayland, tiled, nem fechar quando a janela fecha; sem botões, sem duplo clique, sem roda do mouse (itens 11 e 16 do catálogo); coordenada local do clique veio 12 px à esquerda do esperado (provável padding do CSS), não investigado.

## Barras de título, daemon (24/10/2026), `ui/src/bars.rs` (`kwinish-ui bars [--only prefixo] [--skip a,b]`)
- Um processo faz poll de 80 ms em `j/clients`, `j/monitors` e `j/activewindow` pelo socket (0,05 ms cada) e cria, move e destrói uma barra layer-shell acima de cada janela flutuante, visível, não fullscreen e fora de workspace especial. Sem eventos do `.socket2` (poll basta; trocar se o custo importar).
- Barra: título, botões maximizar e fechar, estilo ativo/inativo; arrastar move a janela pelo socket e **soltar perto da borda** (3 px) encaixa (esquerda, direita ou topo = maximizar) via `window_action` do núcleo Lua; antes de encaixar a janela volta ao ponto onde o arrasto começou, então "restaurar" volta para lá (KWin faz igual); duplo clique maximiza e repetir restaura; clique direito abre o menu da janela; roda do mouse sobre a barra sobe (`alter_zorder top`) ou desce (`bottom`) a janela (item 11 do catálogo).
- **Achado:** a lista de `j/clients` está em ordem de empilhamento (primeiro = embaixo; `alter_zorder` reordena). Usada para esconder a barra de uma janela quando outra, mais acima, cobre a faixa da barra.
- **Limitação estrutural:** uma superfície layer-shell fica acima de todas as janelas, não no nível da janela dona. Por isso a barra some inteira quando qualquer parte dela é coberta (não é recortada), e uma janela tiled nunca cobre barras (a barra só existe para flutuantes). Barras acima de uma janela no topo do monitor ficam grudadas em y = 0 e cobrem as primeiras linhas da janela.
- **Testado ao vivo** (2 kitties descartáveis, `--only kw`): barras criadas nos lugares certos com estilo ativo; arrastar exato; soltar em x = 1 encaixou à esquerda (901x1024) e, depois de maximizar por duplo clique e repetir, restaurou ao ponto pré-arrasto; botão ✕ fechou a janela e removeu a barra; roda para baixo afundou a janela B sob a A (captura de tela) e a barra de B passou a ficar escondida. Teste Rust: `cargo test` (`Rect::hits`).
- **Não testado:** botão de maximizar isolado, clique direito (menu), roda para cima, borda direita e superior ao soltar, vários monitores, XWayland, janelas em fullscreen, mudança de workspace, mouse real (suavidade visual), uso de CPU contínuo do poll, mais de ~5 janelas; sem tema, sem botão de minimizar, ícone do app ou menu de sistema; se o app já desenha a própria barra (CSD) haverá duas.

## Barras: recorte e bordas (24/10/2026)
- **Recorte:** a barra deixou de ser escondida inteira. `widest_gap` (função pura, `cargo test`) calcula o maior trecho da faixa da barra que as janelas mais acima não cobrem; a barra é recortada a esse trecho (mínimo 48 px) e só some se não sobrar um trecho largo. A superfície passou a ancorar esquerda+direita+topo, e a largura vem das margens (o compositor decide), em vez de pedir tamanho ao GTK.
- **Bordas e cantos vêm do Hyprland:** `general:border_size`, `decoration:rounding`, `general:col.active_border` e `col.inactive_border` (gradiente "aarrggbb 0deg", usa a primeira cor) são relidos a cada ~1 s; o CSS é regerado se mudarem. A barra cobre `largura da janela + 2 × borda`, fica logo acima da borda da janela (a borda de cima da janela vira o divisor) e desenha a própria borda (cima, esquerda, direita) na mesma cor, ativa ou inativa. No Ryoku do dono: borda 2 px, arredondamento 0, ativa `c8c3dc`, inativa `141318`.
- **Testado ao vivo** (2 kitties): barra da A = 498..952 (450 + 2×2), 28 px acima da borda; barra da B recortada para 952..1152 quando a A a cobre pela esquerda; com a B por cima as duas barras ficam inteiras; a B cobrindo a faixa da A inteira esconde a barra da A. Conferido por captura de tela e ampliação: bordas laterais alinhadas.
- **Não testado:** mudança de borda ou arredondamento em tempo de execução; `rounding` maior que 0 (raio herdado, mas sem a curva do `rounding_power`, e a janela some nos cantos de baixo da barra); gradiente com mais de uma cor; vários monitores.
- **Imperfeições conhecidas:** no ponto de corte aparece uma borda vertical na extremidade recortada; o botão de maximizar e o título somem se o trecho visível não os inclui.

## Barras: sem contorno no corte (24/10/2026)
Pedido do dono: o contorno da barra é só do topo e dos lados da própria janela; onde outra janela corta a barra não deve nascer um contorno novo. A barra recortada agora recebe as classes `cut-left` e `cut-right` (CSS `border-left/right: none`) no lado cortado. Conferido em captura ampliada (corte à esquerda): sem linha vertical no corte; sobra só a borda de cima e a lateral externa. O corte à direita usa a mesma lógica, não foi fotografado. A linha logo abaixo da barra é a borda real de cima da janela, não da barra.

## Barras: divisor removido (24/10/2026)
A barra agora termina na borda de cima da janela (margem superior `y - 28`, antes `y - borda - 28`), então o fundo da barra cobre a borda de cima da janela e as laterais se emendam sem linha divisória. Conferido em captura ampliada. A borda de cima volta a aparecer sozinha quando a barra está escondida. Não testado: `rounding` maior que 0.

## Barras com arredondamento (24/10/2026)
Teste ao vivo com `decoration:rounding = 12` (via `hyprctl eval hl.config`, revertido com `hyprctl reload`; o valor volta a 0):
- **Funciona:** a barra releu o arredondamento sozinha em ~1 s e passou a ter cantos de cima com raio 12 (mudança em tempo de execução OK).
- **Defeito:** a barra termina reta sobre a borda de cima da janela, mas a janela tem os cantos de cima curvos; sob os dois cantos da barra aparece um entalhe transparente onde a curva da janela se afasta (visível na captura ampliada). Com rounding 0 (config do dono) não ocorre.
- **Caminhos:** (a) preencher o entalhe: estender a barra R px para baixo nas laterais com um preenchimento côncavo (região fora do arco do canto da janela, sem cobrir o anel da borda que o compositor desenha), via `DrawingArea`/cairo; (b) regra `rounding 0` só para janelas com barra (perde os cantos de baixo); (c) a barra cobrir os R px de cima do conteúdo da janela (perde linhas do app). Recomendo (a).

## Barras: preenchimento côncavo (24/10/2026)
- **Feito:** com `decoration:rounding` R > 0 a superfície da barra ganha R px de altura. A parte de baixo é um `DrawingArea` (cairo) que, em cada lado: (1) continua a borda lateral reta por R px na cor da borda; (2) preenche com a cor da barra a região entre a lateral e o canto curvo do conteúdo da janela, que é uma superelipse `|dx/R|^p + |dy/R|^p = 1` com `p = decoration:rounding_power` (4,0 na config do dono). Barra e janela viram uma forma só. O lado cortado por outra janela (`cut-left`/`cut-right`) não desenha seu pedaço.
- **Clique atravessa:** a região de entrada da superfície é só a linha da barra (`set_input_region`); clicar na faixa transparente logo abaixo da barra (dentro do preenchimento) focou a janela de baixo (teste ao vivo).
- **Dinâmico:** com `rounding = 12` (via `hyprctl eval`) as barras passaram a 40 px de altura e os cantos apareceram; depois de `hyprctl reload` (rounding 0) voltaram a 28 px sem reiniciar.
- **Corrigido no caminho:** a primeira versão pintava uma faixa escura sob a barra (fundo padrão da janela GTK); a classe `kwwin` torna o fundo transparente.
- **Testes:** `cargo test` (4): trecho livre, retângulos, curva do canto (circular passa por `R(1-cos 45°)`), parse de cor.
- **Não testado:** `rounding_power` diferente de 4 na tela (a curva é a mesma fórmula, mas só olhei visualmente o 4); corte lateral com arredondamento; raio grande (> altura da barra); vários monitores. A região de entrada fixa em 32767 px de largura.
- Nome do projeto: **Kwinish** mantido por enquanto (decisão do dono, sem conflito no AUR nem no GitHub).
