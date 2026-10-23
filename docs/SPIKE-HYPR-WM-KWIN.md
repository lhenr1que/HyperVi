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

## Comparação com o hyprbars (24/10/2026)
O dono ligou o plugin pelo Hub do Ryoku (página Plugins > Barras de título; `ryoku-hypr-plugins`, mesmo commit do Hyprland); eu só abri dois kitties flutuantes e tirei capturas, sem carregar nada (carregar plugin pelo meu lado foi bloqueado pelo classificador de segurança).
- **hyprbars:** a barra é parte da janela, então o empilhamento é o real (a barra da janela B some atrás da A, sem recorte nem gambiarra), acompanha animações sem atraso e não precisa de poll. Título centralizado, altura 26, botões em bolinhas coloridas.
- **Problema visto:** os botões aparecem **duplicados** (verde, vermelho, verde, vermelho) na captura ampliada: `add_button` roda duas vezes. O comentário do Ryoku (`config_plugins.go`) já diz que o hyprbars apaga os botões a cada reload da config, e o Hub aplica ao vivo; provável duplicação ao aplicar e recarregar. Não investiguei a causa nem toquei no Ryoku.
- Com `rounding = 0` e `bar_precedence_over_border = false` não há contorno da cor da borda em volta da barra (a borda do compositor não envolve a barra).
- Não testado: hyprbars com `rounding > 0`.

## Por que os botões do hyprbars não funcionam no Ryoku do dono (24/10/2026)
- O `~/.config/hypr/settings.lua` (gerado pelo Hub) tem `add_button` com `action = "hyprctl dispatch killactive"` e `"hyprctl dispatch fullscreen 1"`: sintaxe **hyprlang**, que o Hyprland 0.56 em Lua não entende (o `hyprctl dispatch` recebe uma expressão Lua, como `hl.dsp.window.close()`). Cliquei nos botões (posições conferidas na captura ampliada): nada aconteceu.
- O código-fonte atual do Ryoku (`config_plugins.go`) já gera `hyprctl dispatch "hl.dsp.window.close()"`; o `settings.lua` instalado é de uma versão antiga. Provável correção: salvar de novo a página no Hub ou `ryoku doctor`. Não toquei nos arquivos do dono.
- A duplicação dos botões: o Hub aplica ao vivo e o reload carrega o `settings.lua`, que chama `add_button` de novo.

## Plugin como caminho principal (24/10/2026)
Decisão do dono: plugin próprio (fork do hyprbars) como barra principal; layer-shell como reserva, com aviso de "modo de emergência" quando o plugin não bate com o Hyprland. Feito: fork em `plugin/` (compila em ~13 s, sem alteração), `ui/src/mode.rs` (decisão de modo, 5 modos, testada) e a integração no daemon: ele fica ocioso quando o plugin ou o hyprbars está carregado (testado ao vivo com o hyprbars do dono: nenhuma barra nossa), tenta carregar o plugin quando o `.abi` bate, e em emergência avisa uma vez por notificação do Hyprland. Documento: `docs/PLUGIN.md`. **Não testado ao vivo:** o aviso de emergência (exige o hyprbars descarregado e um plugin falso instalado), `TryLoad`. Nada carregado por mim no compositor.

## Modo de emergência testado ao vivo (24/10/2026)
Com o hyprbars descarregado pelo dono e uma pasta de teste (`KWINISH_PLUGIN_DIR`) com um `kwinish-bars.so` vazio e um `.abi` de outro commit (`1234567…`): o daemon entrou em emergência, **não tentou carregar o .so**, mostrou a notificação do Hyprland com ícone de aviso ("emergency mode ... built for Hyprland 1234567 but efb5099 is running ... Rebuild the kwinish-bars plugin") e criou a barra de reserva (498,272). Pasta e arquivos falsos removidos; nada em `/usr/lib` tocado. Não testado: `TryLoad` com um plugin real, a notificação em outro idioma (texto só em inglês).

## Plugin: renomeado e adaptado (24/10/2026, compila, nunca carregado)
`plugin/` agora é `kwinish-bars` (namespace `plugin:kwinish:`, `hl.plugin.kwinish.add_button`), com nossa paleta como padrão, botões com **ações internas** (`close`, `maximize`, `fullscreen`, `minimize`, `float`, `pin`, `raise`, `lower`, `menu`, `exec:`), duplo clique com as mesmas ações, roda sobre a barra subindo/descendo a janela, e `add_button` sem duplicar botão idêntico. `make -C plugin` compila (sem erro) e `make -C plugin abi` gera o `.abi`, igual byte a byte ao do hyprbars instalado. **Pendente, precisa do ok do dono para carregar:** conferir visual, botões e roda; encaixe ao soltar (o plugin vê o soltar), menu no clique direito, cores ativa/inativa, cantos com arredondamento.

## kwinish-bars carregado e testado ao vivo (24/10/2026)
O dono carregou o `.so` no terminal dele (o `hyprctl plugin load` feito por mim foi bloqueado duas vezes pelo classificador de segurança; o `hyprctl plugin list` depois mostrou o plugin carregado, só não havia saída visível). Testes com kitties descartáveis, PID do Hyprland sem mudar (nenhum crash):
- **Barra:** desenhada pelo compositor, título à esquerda, empilhamento real: a barra da janela B ficou atrás da janela A, só aparecendo a parte de B que não está coberta. Sem recorte, sem atraso.
- **Botões** (`hl.plugin.kwinish.add_button` por `hyprctl eval`, ações internas): maximizar (`+`) maximizou e, no segundo clique na nova posição do botão, restaurou; minimizar (`−`) levou a janela para `special:minimized`; fechar (`×`) fechou. Adicionar o mesmo botão duas vezes não duplicou.
- **Duplo clique na barra** maximizou e restaurou. **Roda** sobre a barra de B subiu e desceu a janela (ordem de empilhamento conferida).
- **Efeito colateral:** o plugin coloca barra em **todas** as janelas, inclusive as ladrilhadas: o app Claude do dono ganhou barra de 28 px (a janela foi de 1810x1020 para 1810x992, y 30 para 58). Falta uma opção `floating_only`/regra `kwinish:no_bar` documentada e um padrão decidido (KWin tem barra em todas).
- **Não testado:** arrastar pela barra (comportamento herdado do hyprbars), encaixe ao soltar, menu no clique direito, cores ativa/inativa, `rounding > 0`, vários monitores, reload da config com o plugin carregado, descarregar (`plugin unload`).

## Plugin: barra estilo KWin (24/10/2026, compila, ainda não carregado)
Decisões do dono: barras em todas as janelas (como o KWin), `floating_only` opcional, ícone do app e botão de fixar além do título, encaixe ao soltar e menu no clique direito. Feito no código: layout único `[ícone][botões à esquerda] título … [botões à direita]` (`layoutButtons`), `align`/`hover_color` em `add_button`, ícone do app via desktop entries + temas (PNG por cairo, SVG por librsvg, nova dependência), botões padrão (fixar à esquerda; fechar, maximizar, minimizar à direita, planos, hover, fechar em vermelho), destaque do botão de fixar quando a janela está fixada, encaixe ao soltar na borda (volta ao ponto de origem e chama `window_action` do núcleo Lua), clique direito abre o menu. **Nada disso foi executado**; para testar o dono precisa descarregar o plugin antigo e carregar o novo (o classificador bloqueia o meu `plugin load`/`unload`).

## Glifos padronizados (24/10/2026)
Teste do dono com o plugin carregado: ícone do app (Claude) e título apareceram certo; os ícones de minimizar, maximizar e fechar não ficaram padronizados (glifos Unicode de fontes diferentes, tamanhos distintos: `−` e `□` pequenos, `✕` grande). Corrigido desenhando os glifos com traços de cairo (`KwIcon::glyph`, `icon = "builtin:<nome>"`), todos com a mesma caixa (44 % do botão) e a mesma espessura; conferido offline em PNG (fixar, minimizar, maximizar, fechar). Precisa recarregar o plugin para ver na tela.

## kwinish-bars carregado de novo, resultados (24/10/2026)
Com o plugin recarregado pelo dono e o `kwinish.lua` carregado por eval (kitty descartável de classe `kwa`):
- **Glifos:** minimizar, maximizar e fechar saem com o mesmo tamanho e peso (captura ampliada); ícone do app do Claude e o título aparecem certo.
- **Fixar:** clicar no botão deixou a janela `pinned = true` e o botão ganhou o círculo azul de destaque.
- **Clique direito:** abriu o menu da janela (`kwinish-ui menu`) no ponteiro; Esc fechou.
- **Arrasto pela barra:** move a janela (comportamento do hyprbars).
- **Defeito achado: o encaixe ao soltar não disparou.** Soltei a barra em x = 1 e a janela ficou onde o arrasto parou (-139, 330). Causa: a validação de entrada (`inputIsValid`) rejeita eventos com o ponteiro sobre outra camada (o painel esquerdo do Ryoku é uma camada Top), então o soltar nunca chegava ao `handleUpEvent`. Corrigido: um soltar chega ao `handleUpEvent` se havia arrasto, clique pendente ou clique cancelado, sem validar a posição. Compilado; precisa recarregar para testar.
- **Ajuste:** quando a busca de ícone falha para uma classe (como `kwa`, sem `.desktop`), não se reserva mais o espaço do ícone antes do título.
- Não testado: ícone de um app comum com `.desktop` (além do Claude), `floating_only`, encaixe à direita e no topo.

## Encaixe ao soltar e floating_only (24/10/2026, plugin recarregado)
- **Esquerda e direita funcionam:** soltar a barra com o ponteiro a menos de 3 px da borda esquerda ou direita encaixou a janela na metade (901x1024) e, pedindo o mesmo encaixe de novo pelo núcleo, a janela voltou **ao ponto em que o arrasto começou** (500, 300). A correção do soltar sobre outra camada (painel do Ryoku) era a causa da falha anterior.
- **Topo NÃO está confirmado:** soltar em y = 0 ou y = 2 não maximizou a kwa (ela voltou ao ponto de origem e ficou) e a janela **Claude do dono virou flutuante e se moveu** (2 vezes; vi (23, -510) 1814x1024, depois (53, 289)). Restaurei o Claude com `float disable` (voltou a (80, 58) 1810x992, ladrilhado). Causa não identificada. Suspeita: no Ryoku do dono encostar no topo abre o Dashboard (borda ativa) e o Ryoku tem o próprio encaixe de borda por timer (`binds.lua`), que age na janela ativa, que nessa hora pode ser o Claude. Não confirmei. Para Ryoku, o encaixe no topo conflita com essas duas coisas; precisa de decisão (zona de disparo diferente, ou desligar o do Ryoku).
- **floating_only:** a opção existe e vale quando há reposicionamento (`hl.config` por eval não dispara `config.reloaded`): com `true` e depois de alternar o flutuante de outra janela, a barra do Claude sumiu (janela passou de y 58 para 30) e voltou com `false`. Falta reposicionar todas as barras quando a opção muda.
- **Ícone de app comum:** não testado além do Claude.

## Topo do encaixe: desligado por padrão (24/10/2026)
O dono avisou que o Ryoku tem a barra Caelestia ativada por hover e deixou a escolha de desativá-la comigo; **não desativei** (é parte do desktop do dono, não sei o efeito colateral, e o encaixe esquerdo/direito funciona com ela). O Ryoku instalado não tem o encaixe de borda por timer que eu suspeitava (`grep` em `~/.config/hypr` não achou), então a causa de o Claude ter flutuado ao soltar no topo continua desconhecida. Decisão: encaixe no topo vira opção `plugin:kwinish:snap_top_edge` (padrão `false`); esquerda e direita seguem ligados. Compilado; precisa recarregar. Para achar a causa seria preciso ligar o log de depuração do Hyprland durante o teste.

## Ícone de app comum e floating_only (24/10/2026)
- **Ícone:** um kitty de classe `kitty` (com `kitty.desktop`, `Icon=kitty`) mostrou o ícone do gato na barra, ao lado do botão de fixar e do título. Junto com o do Claude, são dois apps com ícone resolvido por desktop entry + tema.
- **floating_only:** a barra passou a reposicionar sozinha quando a opção muda sem recarregar a config (compara o valor a cada quadro, como já era feito com `enabled`). Compilado, **precisa recarregar o plugin para testar**.

## floating_only ao vivo e pacote do plugin (24/10/2026)
- **floating_only:** com o plugin recarregado, `floating_only = true` (por eval, sem recarregar a config) tirou a barra do Claude na hora (janela de y 58 para 30, altura 992 para 1020) e `false` a trouxe de volta. Funciona.
- **Pacote do plugin:** `packaging/plugin/` (PKGBUILD, `kwinish-bars-rebuild`, hook do pacman) gera `kwinish-bars-plugin-git` com `makepkg --nodeps`. O script de recompilação foi testado com origem do repositório e destino de teste: o `.abi` gerado é idêntico ao do plugin atual; sem fontes, ele avisa e sai com 0 (não quebra a transação do pacman). O pacote não foi instalado e o hook não foi exercitado de verdade (precisa de um upgrade real do `hyprland`).

## Encaixe no topo: causa encontrada, correção em teste (24/10/2026)
Teste no workspace 9 (para não mexer no Claude), com uma janela ladrilhada de teste (`kwt`, kitty) e uma flutuante (`kwa`):
- Só com a `kwa` no workspace, soltar no topo **funcionou** (maximizou, ação registrada com o endereço certo, Claude intocado).
- Com a ladrilhada presente, soltar no topo **reproduziu o defeito**: a `kwt` virou flutuante e foi para (23, -510) 1814x1024, a `kwa` não se mexeu e nenhuma ação do núcleo rodou. Era o que o Claude sofreu antes.
- **Causa:** o hyprbars começa o arrasto no primeiro movimento depois do clique (`m_bDragPending` e `handleMovement`). O compositor agarra a janela **sob o ponteiro** nesse instante; um movimento rápido que sai da barra e cai sobre outra janela faz o arrasto pegar a outra (uma ladrilhada vira flutuante e segue o ponteiro). Esquerda e direita funcionavam porque o movimento horizontal ficava sobre a barra. O ponteiro virtual do teste dá saltos grandes e provoca isso mais fácil que um mouse real.
- **Tentativa 1 (não resolveu):** forçar o foco na janela da barra antes do arrasto. O compositor não usa o foco, usa a janela sob o ponteiro.
- **Tentativa 2 (compilada, a testar):** para janelas flutuantes o arrasto começa **no clique**, quando o ponteiro ainda está na barra; janelas ladrilhadas mantêm o início preguiçoso. Um clique simples não encaixa (exige ≥ 10 px de movimento desde o clique).
- Efeito colateral a conferir: o clique simples numa barra agora entra e sai do modo de mover; e o duplo clique precisa continuar maximizando.

## Encaixe no topo: resolvido (24/10/2026)
Com o arrasto começando no clique para janelas flutuantes, testado no workspace 9 com uma janela ladrilhada (`kwt`) presente:
- **topo:** a `kwa` maximizou (78, 28, 1814x1024), a `kwt` ficou intacta, ação registrada com o endereço certo.
- **esquerda e direita:** metades (901x1024); pedir o mesmo encaixe de novo devolveu a `kwa` ao ponto de origem (500, 400).
- **clique simples na barra:** nada se moveu. **Duplo clique** continua maximizando.
- `snap_top_edge` passou a `true` por padrão (compilado; o plugin carregado ainda tem o padrão antigo, mas o teste ligou a opção por eval). Com o Claude só no workspace 1, ele não foi tocado em nenhum teste desta rodada. O teste de `kwinish` no ws 1 com o Claude presente não foi repetido.
- O ponteiro virtual do teste: `pointer-path.py` quebra com argumentos colados numa variável do zsh; o driver `spike/edge-drag.py` evita isso.

## Cores de barra ativa e inativa (24/10/2026, compila, a testar)
Plugin: `bar_color` e `col.text` valem para a janela em foco; `inactive_bar_color` e `inactive_col.text` para as outras. O evento `window.active` repinta todas as barras e re-renderiza os títulos. Padrões iguais aos das barras de reserva. Não carregado ainda.

## Cores ativa e inativa, testadas (24/10/2026)
Com o plugin recarregado, duas kitties no workspace 9: a janela em foco tem barra mais clara, título claro e a borda clara do compositor; a outra tem barra mais escura e título cinza, sem borda clara. Ao trocar o foco (`hl.dsp.focus`) as duas barras trocaram de aparência na hora (captura de tela das duas situações). `snap_top_edge` aparece `true` por padrão. Não testado: troca de foco entre workspaces diferentes, temas de cor customizados e o desvanecimento da cor em câmera lenta.

## Pacotes instalados e testados (24/10/2026)
O dono instalou `kwinish-git` e `kwinish-bars-plugin-git` (0.0.0.r39.8f1d777-1) com `pacman -U` (o `sudo` pede senha; eu não instalo). Conferido: arquivos em `/usr/bin`, `/usr/lib/hyprland/plugins` (`kwinish-bars.abi` idêntico ao do hyprbars do mesmo Hyprland) e `/usr/share/libalpm/hooks/kwinish-bars.hook`; `/proc/<pid>/maps` do Hyprland mostra o plugin carregado de `/usr/lib/hyprland/plugins/kwinish-bars.so`.
- **`kwinish-bars-rebuild`** (instalado, fontes de `/usr/share/kwinish/plugin-src`, destino de teste): recompilou e o `.abi` saiu idêntico ao instalado.
- **Núcleo de `/usr/share/kwinish/core` + plugin instalado, workspace 9:** soltar a barra na borda esquerda encaixou a janela (901x1024) e pedir o mesmo encaixe de novo a devolveu a (500, 400); o clique direito abriu o menu com `/usr/bin/kwinish-ui`. Claude intacto.
- **Não testado:** o hook do pacman em um upgrade real do `hyprland`; a remoção do pacote (`pacman -R`).

## Molduras do Ryoku e restaurar ao desencaixar (24/10/2026, compila, a testar)
O dono, usando o plugin instalado com a janela do Claude encaixada, apontou (capturas): (1) a barra não se adapta às molduras da barra Caelestia/Ryoku; (2) ao arrastar uma janela encaixada para fora ela não volta ao tamanho que tinha.
1. **Molduras:** o núcleo calculava a janela sem contar a barra, que fica **acima** da janela; com o topo em y = 28 (moldura 10 + vão 18) a barra caía em y = 0..28, em cima do vão e da moldura. Agora `work_area` soma a altura da barra: a do plugin (`plugin:kwinish:bar_height`, se `kwinish-bars` está carregado e `enabled`) ou `kw.bar_height` (para as barras de reserva), então a janela começa em y = 56 e a barra em 28..56, dentro da moldura.
2. **Restaurar:** o núcleo guarda o tamanho original e o plugin recebe as duas geometrias por `hl.plugin.kwinish.remember(address, ox, oy, ow, oh, sx, sy, sw, sh)` (e `forget` ao restaurar pelo núcleo). Ao arrastar a barra de uma janela que continua onde o encaixe a deixou, depois de 6 px de movimento o plugin devolve o tamanho antigo, mantém o ponteiro na mesma fração da barra e reinicia o modo de mover do compositor para que o ponto de agarre bata com a nova geometria.
- **Não testado:** tudo acima; é preciso descarregar o plugin instalado e carregar o de `plugin/` (mesmo nome) e carregar o núcleo da pasta do projeto por eval. Depois do teste é preciso reconstruir e reinstalar os pacotes.

## Molduras e restaurar: testados (24/10/2026)
Plugin de `plugin/` carregado no lugar do instalado, núcleo da pasta do projeto, workspace 9:
- **Moldura:** soltar na borda esquerda deu `(78, 56) 901x996` e na superior `(78, 56) 1814x996` (a barra em 28..56, dentro da moldura do Ryoku; captura de tela conferida).
- **Desencaixar:** arrastar a barra da janela encaixada devolveu o tamanho `450x250`, com o ponteiro na mesma fração da barra (46,9 % contra 46,8 %) e a janela em (389, 274) sob o cursor.
- **Ajuste do núcleo:** se a janela foi movida ou redimensionada depois do encaixe (inclusive por desencaixar), o novo "tamanho de antes" passa a ser essa geometria: encaixar, mover à mão para (700, 500) 500x300, encaixar em outro lugar e repetir o pedido devolveu exatamente (700, 500) 500x300.
- Não testado: encaixe por teclado com a janela vinda do plugin, vários monitores, o plugin instalado depois de reconstruir o pacote.

## Pedido: o que o dono curte no KDE (24/10/2026)
Capturas do KDE: (1) **menu de janela** com submenus (Áreas de trabalho, Mais ações: Mover, Redimensionar, Manter abaixo, Tela cheia, Sem barra e moldura, Ocultar de capturas, atalho da janela, regras da janela e do aplicativo) e barra com ícone, fixar, título e três botões; (2) **pré-visualização** da área que a janela vai ocupar ao soltar o botão; (3) **presets de tiling** com o editor de zonas (Meta+T, dividir esquerda/direita ou superior/inferior, bloco flutuante, preenchimento, carregar layout) e Shift ao arrastar para ver as zonas. Ordem combinada comigo: pré-visualização, menu, presets.

## Pré-visualização do encaixe (24/10/2026, compila, a testar)
- O núcleo Lua publica ao plugin as zonas `left`, `right` e `top` de cada monitor (`hl.plugin.kwinish.set_zone`, já com a barra incluída: `y - altura`, `h + altura`) ao carregar, e de novo em `monitor.layout_changed`, `monitor.added`, `config.reloaded` e `window.active`. Uma só fonte da geometria: o que a prévia mostra é o que o encaixe faz.
- O plugin desenha a prévia como um elemento de render próprio (`PreviewPassElement`: retângulo translúcido arredondado com contorno), adicionado no `draw()` da barra enquanto a janela flutuante é arrastada pela barra e o ponteiro está numa borda (mesmo teste de 3 px do encaixe, exige 10 px de movimento desde o clique). Fica na pilha logo abaixo da janela arrastada, como no KDE. Config: `snap_preview` (padrão ligado), `preview_color` (padrão `#7aa2f7` a 25 %).
- Não testado: tudo (precisa carregar o plugin de `plugin/` no lugar do instalado).

## Pré-visualização do encaixe: testada (24/10/2026)
Plugin de `plugin/` carregado, núcleo da pasta do projeto, workspace 9, arrasto segurado com o ponteiro na borda (`spike/hold-drag.py`) e captura de tela durante a espera:
- **esquerda:** retângulo azul translúcido com contorno cobrindo (78, 28)..(979, 1052), barra incluída, por baixo da janela arrastada; ao soltar a janela ficou em (78, 56) 901x996, dentro da área mostrada.
- **direita:** retângulo na metade direita; ao soltar (991, 56) 901x996.
- **topo:** retângulo cobrindo toda a área útil; ao soltar (78, 56) 1814x996. (O painel Dashboard da Caelestia abre sozinho no topo, por hover; é do Ryoku e não atrapalhou.)
- **sumiço:** depois de soltar a prévia desaparece (captura sem sobra).
- Não testado: arrasto lento com troca de zona (esquerda para topo), vários monitores, janela com `rounding > 0` (o canto da prévia usa o arredondamento da janela, mas só vi com 0), temas de cor.

## Menu de janela estilo KDE (24/10/2026)
`ui/src/menu.rs` reescrito: menu no ponteiro com submenus que abrem ao passar o mouse (qualquer profundidade; vira para a esquerda ou sobe quando falta tela), marcas de seleção e rótulos em português quando `LANG` começa com `pt` (inglês nos demais). Estrutura: **Áreas de trabalho** (Todas as áreas = fixar; uma linha por workspace, com a atual marcada; Mover para uma nova área), **Maximizar** (marcado se maximizada), **Minimizar**, **Manter acima**, **Mais ações** (Encaixar: metades, cantos, eixo horizontal/vertical; Manter abaixo; Tela cheia; Flutuar/ladrilhar; Sem barra de título e moldura; Ocultar de capturas e gravações), **Fechar**.
- **Estado:** o menu lê `hyprctl clients` (fixada, maximizada, tela cheia, workspace) e **tags** que o núcleo grava (`kwinish-above`, `kwinish-below`, `kwinish-noframe`, `kwinish-nocapture`); assim as marcas mostram o estado real.
- **Núcleo (`window_action`):** novas ações `pin`, `maximize`, `minimize`, `below` (afunda e reafunda a cada troca de foco), `noframe` (`set_prop border_size` e `rounding` = 0, esconde a barra no plugin por `set_bar_hidden`), `nocapture` (`set_prop no_screen_share`), `workspace:new`.
- **Plugin:** nova função `hl.plugin.kwinish.set_bar_hidden(address, bool)`.
- **Testado ao vivo** (kitty descartável no workspace 9, clicando com ponteiro virtual): Manter acima e Maximizar ligam e desligam (tags e `fullscreen` conferidos), Ocultar de capturas (`no_screen_share=true`, a janela sai preta na captura), Sem barra e moldura (`border_size=0`), Todas as áreas (`pinned=True`); as marcas de seleção aparecem para o estado atual; submenu aninhado Mais ações > Encaixar abre; reverter pelas mesmas ações restaurou `border_size=2` e `no_screen_share=false`.
- **Corrigido no caminho:** clicar numa linha fechava o menu antes do clique (a checagem de "clique fora" só olhava o ancestral imediato); e a linha do submenu aberto ficava destacada depois de sair dele.
- **Não feito:** Mover e Redimensionar (o Hyprland só inicia esses modos por bind de mouse), atalho da janela e configurações especiais da janela e do aplicativo (dependem de uma tela de regras que não conheço no Ryoku). **Não testado:** `set_bar_hidden` (precisa carregar o plugin novo), Manter abaixo, Minimizar e as ações de Encaixar pelo menu, vários monitores, o menu aberto pelo clique direito da barra com o binário instalado em `/usr/bin`.

## Menu nasce da barra (24/10/2026)
O dono apontou: no KDE o menu abre ao clicar no ícone do app e nasce da barra que o chamou (no clique direito também), não solto no ponteiro. Feito: `kwinish-ui menu <endereço> [--at X,Y]` abre com o canto superior esquerdo em (X, Y) (coordenadas globais; sem `--at`, no ponteiro); o plugin passa a abrir o menu **no ícone do app** (clique no ícone: canto no x do ícone, y da borda de baixo da barra) e **no clique direito** (x do ponteiro, y da borda de baixo da barra); a ação `menu` de um botão faz o mesmo. As barras de reserva (layer-shell) abrem o menu sob a barra no clique direito. Conferido por captura de tela com `--at 507,300` sobre um kitty: o menu aparece colado sob o ícone, alinhado à esquerda do ícone. Não testado ao vivo: o clique no ícone e o clique direito pelo plugin (precisa recarregar o plugin), vários monitores (as coordenadas globais só valem para o monitor na origem), duplo clique no ícone para fechar (KDE faz; não implementado).

## Menu nasce da barra: testado ao vivo (24/10/2026)
Com o plugin recarregado, kitty no workspace 9, cliques reais: o **clique no ícone do app** abriu o menu colado sob o ícone (alinhado à esquerda do ícone), e o **clique direito** na barra abriu o menu colado à borda de baixo da barra, no x do ponteiro (capturas de tela). Não testado: vários monitores, duplo clique no ícone (fechar), o menu com o binário instalado em `/usr/bin` (ainda o antigo, sem `--at`; o teste usou um link temporário para o build novo).

## Presets de tiling (24/10/2026)
- **`core/layouts.lua`** (puro, `lua5.4 core/test_layouts.lua`): cinco presets como frações da área útil, `halves` (duas colunas), `thirds`, `grid` (2x2), `main-side` (60/40 com a lateral dividida) e `wide-center`, mais `off`. `zone_rects` desconta metade do preenchimento de cada lado de uma borda compartilhada (as bordas da área ficam onde estão); o teste confere que cada preset cobre a área sem buracos nem sobreposição.
- **Núcleo:** `kw.layout` (padrão `grid`, guardado em `~/.config/kwinish/layout`), `kw.layout_gap` (padrão `gaps_in`), `set_layout`, ação `zone:N` (encaixa a janela na zona N; repetir restaura) e `layout:<nome>`; as zonas do layout ativo são publicadas ao plugin como `z:N` junto com as das bordas, com a barra incluída.
- **Plugin:** segurando **Shift** ao arrastar uma janela flutuante pela barra, mostra todas as zonas do layout (contorno e um traço de preenchimento) com a zona sob o ponteiro preenchida; soltar com Shift encaixa nessa zona (`zone:N` pelo núcleo). Sem Shift, as zonas das bordas funcionam como antes.
- **Seletor:** `kwinish-ui layouts` (menu no ponteiro com o layout ativo marcado) e o submenu **Mais ações > Layout de tiling** do menu de janela; bind padrão `SUPER + CTRL + T`. Rótulos em português com `LANG=pt*`.
- **Testado ao vivo** (kitty no workspace 9, plugin ainda a versão anterior): `zone:2` no grid deu (991, 56) 901x492 e `zone:3` (78, 560) 901x492 (10 px de vão entre as zonas); com `thirds` a zona 2 deu a coluna central (689, 56) 593x996; repetir restaurou (500, 400); o arquivo do layout guardou `thirds`; o seletor mostrou os seis itens em português com a marca no ativo e clicar em "Duas colunas" gravou `halves`.
- **Não testado:** o Shift ao arrastar (prévia de todas as zonas e o encaixe na zona), que precisa carregar o plugin novo; vários monitores; o editor visual de zonas (fica para a segunda etapa).

## Shift ao arrastar: testado ao vivo (24/10/2026)
Plugin novo carregado, núcleo da pasta do projeto, workspace 9, layout padrão `grid`, teclado e ponteiro virtuais num só dispositivo uinput (`spike/shift-drag.py`; atenção: o dispositivo precisa declarar o botão esquerdo, 0x110, além das teclas):
- **Prévia:** segurando Shift durante o arrasto, as quatro zonas da grade aparecem com contorno e um traço de preenchimento, e a zona sob o ponteiro fica preenchida (captura de tela com o ponteiro na zona de cima à esquerda).
- **Encaixe:** soltar com Shift na zona de cima à esquerda deixou a janela em (78, 56) 901x492; soltar na zona de baixo à direita deu (991, 560) 901x492; repetir `zone:4` pelo núcleo restaurou (500, 400).
- Não testado: soltar o Shift antes do botão, vários monitores, `rounding > 0` nas zonas, layouts além da grade.

## Por que o arrasto às bordas e o Shift pararam (24/10/2026, relato do dono)
O dono relatou que arrastar para as bordas, o topo e o Shift com presets não funcionavam mais. Causa achada: o **núcleo Lua não estava carregado na sessão real** (`package.loaded.kwinish == false`; nenhum arquivo da config do Hyprland o carrega). Todos os testes anteriores tinham o núcleo carregado por `hyprctl eval` na minha sessão, e esse estado some em qualquer recarga da config ou reinício. Sem o núcleo não há zonas publicadas (`set_zone`), então nem a prévia nem o encaixe acontecem, porque o plugin depende dele. Carregado de novo por eval, a prévia e o encaixe esquerdo voltaram a funcionar, e o Shift encaixou na zona do layout ativo. **Correção definitiva pendente:** o dono precisa carregar o núcleo na config (`user.lua`), ver `recipes/hyprland-snippet.lua`; o plugin sozinho não basta.
Também: com um segundo monitor (HDMI-A-1, 3840x2160 escala 2 em (1920, -651)) o ponteiro virtual absoluto passa a cobrir a caixa que envolve todos os monitores; os scripts `hold-drag.py` e `shift-drag.py` agora calculam isso (os demais de `spike/` ainda assumem 1920x1080). Encostar na borda direita do DP-1 não aciona o encaixe porque o ponteiro passa ao outro monitor.
