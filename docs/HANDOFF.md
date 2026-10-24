# Handoff: estado do Kwinish e como continuar (24/10/2026)

Para quem pega o projeto numa sessão nova (a sessão original ficou grande). Leia este arquivo, depois `README.md`,
`docs/PLUGIN.md` e, só se precisar do histórico de testes, `docs/SPIKE-HYPR-WM-KWIN.md`. Tudo em `~/Downloads/ryoku-work/kwinish/`,
repositório git **local** (nunca houve push, fork ou PR; não faça sem pedido explícito do dono). Commits:
`git -c user.name=lhen -c user.email=lhenriqueds99@gmail.com commit`.

## O que existe e funciona (testado ao vivo no Hyprland 0.56.2 do dono)
- **Plugin `kwinish-bars`** (`plugin/`, C++, fork do hyprbars): barra com ícone do app, fixar, título, minimizar/maximizar/fechar;
  cores ativa/inativa; menu da janela no ícone e no clique direito; duplo clique maximiza; roda sobe/desce a janela; arrasto
  começa no clique; encaixe nas bordas esquerda/direita/topo; **pré-visualização** do encaixe; **Shift ao arrastar** mostra as
  zonas do layout e encaixa na zona; desencaixar ao arrastar devolve o tamanho de antes; `floating_only`; modo de emergência.
- **Núcleo Lua** (`core/`): tile, eixo, empacotar, mostrar área de trabalho, manter acima/abaixo, posicionamento, auto-raise
  (não verificado ao vivo), cantos ativos, Alt+Tab (lógica), binds padrão, **layouts/zonas** (`layouts.lua`), publica as zonas ao plugin.
- **UI Rust** (`ui/`, `kwinish-ui`): overlay do Alt+Tab, **menu de janela estilo KDE** (submenus, marcas, `--at X,Y`), seletor
  de layouts (`kwinish-ui layouts`), barras de reserva (`kwinish-ui bars`).
- **Pacotes** (`packaging/`, rascunhos): `kwinish-git` e `kwinish-bars-plugin-git` (hook do pacman que recompila o plugin).
  Versão instalada pelo dono: `0.0.0.r50.babd9b8-1`. Arquivos em `dist/` (fora do git).

**Decisão (24/10):** snap, zonas, pack e eixo só agem em janelas **flutuantes**; janelas ladrilhadas são ignoradas (`place()` no núcleo) e o menu esconde "Encaixar" nelas. Verificado ao vivo.

## Pendências, em ordem sugerida
1. ~~Editor visual de zonas~~ feito (`ui/src/editor.rs`, lógica em `ui/src/zones.rs`, arquivo `~/.config/kwinish/layouts.json` lido por `core/layouts.lua`). Os divisores arrastam (`zones::edges`/`move_edge`; visto desenhado, arraste não testado com mouse). O editor desenha sobre a área útil do primeiro monitor (descontando reservas e `gaps_out`, não a barra de título do plugin). Falta: Verificado ao vivo (ws 9): layout do usuário no `layouts.json` → `set_layout` → `window_action(addr,"zone:N")` (inclusive bloco flutuante) e o seletor mostra o layout marcado. O dono confirmou (24/10) o ciclo completo: editor → Salvar → seletor → Shift ao arrastar com layout do usuário. O núcleo agora carrega pelo `~/.config/hypr/user.lua` do dono (backup `user.lua.bak-kwinish`).
2. Menu (todas as partes feitas): **regras da janela/app feitas** (`core/rules.lua`, `~/.config/kwinish/rules.json`, submenu "Regras da janela" em Mais ações: lembrar posição/tamanho, sempre na área, acima, abaixo, sem moldura, ocultar de capturas; aplicadas 150 ms após abrir; verificado ao vivo: área e manter acima; posição só vale para janelas flutuantes). **Mover e Redimensionar feitos** (menu raiz, `grab:move|resize` → `hl.plugin.kwinish.grab` no plugin: usa o `changeMouseBindMode` do compositor e termina no próximo clique, que é engolido; verificado ao vivo com `spike/grab.py`: mover +200,+100 e redimensionar pelo canto inferior direito; o dono confirmou com o mouse no menu real e que Esc cancela (comportamento do compositor, não do plugin); não encaixa nas bordas ao soltar). **Atalho da janela feito** (menu raiz "Atalho da janela..." → `kwinish-ui shortcut <addr>` captura a combinação, recusa as sem SUPER/CTRL/ALT e as já usadas por binds do Hyprland, grava `~/.config/kwinish/shortcuts.json` por classe e roda `hyprctl reload`; o núcleo faz `hl.bind` por app ao carregar a config, nunca via eval). Verificado: testes, `Hyprland --verify-config` com o núcleo e um shortcuts.json; **Armadilha achada ao vivo:** o Hyprland roda seus binds antes de o cliente ver a tecla, então apertar SUPER+Q no diálogo FECHOU a janela em vez de capturar. Agora o diálogo entra no submap vazio `kwcapture` (definido pelo núcleo com `hl.define_submap` ao carregar a config), recusa-se a escutar se não conseguir entrar, sai do submap por todos os caminhos e há um vigia de 35 s (`spike/keys.py` reproduz). O atalho foca e sobe a janela flutuante (o ponteiro vai ao centro: é o warp padrão do foco do Hyprland). Não verificado ao vivo com o núcleo novo: captura e conflito.
3. Duplo clique no ícone fecha a janela (KDE faz; não implementado).
4. Vários monitores: menu e editor de zonas agora abrem no monitor sob o ponteiro (`ui/src/mon.rs`, `set_monitor`), verificado ao vivo com HDMI-A-1 ao lado do DP-1; `zone:N` e `tile:` na janela do HDMI deram as posições certas. Não verificado: Shift ao arrastar no 2º monitor, barras de reserva (`bars.rs` já usa origem do monitor), monitores com escala ≠ 1 ou girados.
5. Miniaturas no Alt+Tab; estado do núcleo que sobrevive a reload (hoje em tabelas Lua; usar tags); `rounding > 0` nas zonas e na prévia.
6. Testar o hook do pacman num upgrade real do `hyprland`; remoção do pacote; escolher nome definitivo e publicar (nome provisório "Kwinish", licença MIT).

## Três modos por workspace (adotado pelo dono, 24/10; nada implementado)
Tiling / Scrolling / Floating alternáveis por workspace, com seletor no menu e na barra e a geometria preservada ao trocar.
**O Hyprland 0.56.2 já tem os motores**: `/usr/include/hyprland/src/layout/algorithm/tiled/` traz `dwindle`, `master`, `monocle` e
`scrolling`, e o binário tem `CWorkspaceAlgoMatcher` (`updateWorkspaceLayouts`, `registerFloatingAlgo`), logo existe escolha de algoritmo
por workspace e até algoritmos flutuantes registráveis por plugin. Não é preciso escrever motor de scroll nem adaptar o hyprscroller.
**Confirmado ao vivo (24/10, workspace 9, 3 kitties):** `hyprctl eval "hl.workspace_rule({ workspace = '9', layout = 'scrolling' })"` trocou o workspace de dwindle para scrolling na hora (as janelas viraram colunas em fita, uma fora da tela à direita), e `master`, `monocle` e `dwindle` também; `hyprctl workspaces -j` mostra o motor em `tiledLayout`. `layout = 'floating'` é aceito mas não altera o `tiledLayout`: o modo Floating é o das janelas flutuantes (o que o Kwinish já faz), não um motor. Sem plugin. A regra vale até o próximo reload da config (para persistir, aplicar no núcleo ao carregar). Opções nativas do scroll: `scrolling:column_width`, `focus_fit_method`, `follow_focus`, `direction`, `wrap_focus`. Trabalho do Kwinish: seletor por workspace, modo Floating (o que já existe),
e o estado de geometria que hoje some no reload (pendência 5; guardar em tags ou arquivo). Não obriga nuvem: depende do desktop do dono.

## Referências ("doadores"), avaliação
Pop Shell e KWin: só conceitos (cache de geometria, `transientFor`, z-order); GPL, nunca copiar código para o Kwinish (MIT).
`hyprbars` já é a base do plugin (BSD-3, `plugin/NOTICE`). HyDE (GPL-3): só ideias de detecção de GPU e captura; o Kwinish não toca
em GPU. Ryoku: GPL-3 com terceiros no `NOTICE`; ver `ryoku-work/docs/distro/ESCOPO-DISTRO.md`.

## Comportamentos de referência do Windows e do macOS (24/10; só o que o usuário vê, nada de binário)
Fontes lidas em 24/10: Apple Support "Tile windows on Mac" (guia do macOS 27) e Microsoft Learn/Support sobre Snap layouts. O que vem
só da memória do assistente está marcado (memória) e não foi conferido agora. Implementar sempre do zero.
- **macOS:** arrastar a janela até a borda encaixa; arrastar até o topo abre o Mission Control; segurar Option ao arrastar mostra as áreas
  de encaixe (igual ao nosso Shift ao arrastar); o botão verde abre um menu de layouts para uma ou mais janelas; menu da janela e atalhos de
  teclado; o recurso liga e desliga nas configurações.
- **Windows:** Snap layouts (passar o mouse no botão maximizar abre um menu de layouts; clicar numa zona encaixa a janela); **Snap Assist**
  (depois de encaixar uma janela, sugere as outras abertas para as zonas que sobraram); Snap layouts ao arrastar para o topo da tela
  (liga/desliga); sugestões para encaixar várias janelas de uma vez; Snap groups (as janelas encaixadas voltam juntas). (memória:
  Win+setas, Aero Shake, clique no botão da barra de tarefas minimiza/restaura, Alt+Tab com miniaturas, Task View.)
- **Candidatos para o Kwinish, em ordem de valor:** (1) **Snap Assist** sobre as zonas que já existem: ao encaixar uma janela numa zona,
  oferecer as outras janelas para as zonas vazias; (2) seletor de layout ao passar o mouse no botão maximizar da barra do plugin;
  (3) miniaturas no Alt+Tab (já na pendência 5); (4) abrir o Mission Control-like ao arrastar até o topo (opcional, hoje o topo maximiza);
  (5) liga/desliga do encaixe nas configurações (hoje só pelo config).
- Limite: o assistente não tem um Windows nem um Mac para observar; versões muito novas só valem pelo que as páginas oficiais dizem.

## Omarchy 4.5 RC (releases de omacom/omarchy, lido em 24/10)
Notas oficiais: "Add floating workspaces with themed titlebars and a dock, alongside dwindle and scrolling on `Super + L`" e "Give each
monitor its own workspaces, remembered every time it's plugged back in". Ou seja, o Omarchy já oferece dwindle / scrolling / floating por
workspace: **a "tríade por workspace" não é lacuna única do Kwinish** (um texto colado antes dizia que ninguém oferecia). Como é feito
(plugin, hyprbars, código próprio) **as notas não dizem**; um texto colado afirmou que usa hyprbars e `special:minimized`, sem prova (e com
sintaxe `windowrulev2` de config antiga). O Omarchy é MIT: dá para ler o código dele (somente leitura, com crédito) para ver como fizeram.
O que ainda nos diferencia: barra e menu estilo KWin, zonas de layout com prévia e editor, regras por app, Move/Resize/atalho pelo menu.
Correção do quadro comparativo colado: o Kwinish **não** esconde janelas por flag no `CWindow` (minimiza por `special:minimized`), **ainda não**
trata janelas filhas (`transientFor`) e **ainda não** tem a tríade por workspace (só confirmada como possível).

### Como o Omarchy faz o modo Floating por workspace (leitura do código MIT em 24/10, `omacom/omarchy`, `default/hypr/workspace-modes.lua`)
- **Floating não é um motor**: o Hyprland só reporta `tiledLayout` e aceita nome de layout desconhecido em silêncio (caindo em dwindle), igual ao
  que vimos aqui. O Omarchy aplica o modo **às janelas**: ao ligar o modo, cada janela ladrilhada vira flutuante, é centralizada num tamanho
  confortável (`min(1100, 64% da largura)` × `min(760, 68% da altura)`) e escalonada em cascata de 24 px; ao desligar, só as janelas que o
  modo flutuou voltam a ladrilhar.
- **Etiquetas (tags) nas janelas** guardam isso: `omarchy-mode-floated` (a janela que o modo flutuou), `omarchy-floating-workspace` (qualquer
  janela de um workspace flutuante, para barra de título/sombra) e `omarchy-mode-opened`. A etiqueta morre com a janela, acompanha-a entre
  workspaces e **sobrevive ao reload da config**: é a solução para a nossa pendência 5 (estado do núcleo que some no reload).
  Janelas que já eram flutuantes por conta própria, em tela cheia ou fixadas (pin) ficam fora do modo.
- O modo de cada workspace fica num arquivo (`~/.local/state/omarchy/workspace-layouts/N.lua`, `o.workspace_mode({workspace, mode})`) e é
  reaplicado ao logar; `hyprctl eval` aplica na hora. Janelas novas num workspace flutuante nascem flutuantes por **regra de janela com
  intervalo de workspaces** (`workspace = "r[a-b]"`), no tamanho do modo. `Super + L` cicla dwindle → scrolling → floating.
- Barra de título por **plugin próprio num pacote à parte** (`omarchy-hyprland-titlebars`, `titlebars.so`), não o hyprbars puro.
- API útil que ainda não usávamos: `hl.get_windows({ tag = "..." })` (listar janelas por etiqueta) e `hl.window_rule` com `workspace = "r[a-b]"`
  e `modal = false`.
- Adotar para o Kwinish: seletor por workspace com **modo guardado em arquivo** + **etiquetas para reverter só o que o modo mexeu** + regra de
  abertura por intervalo de workspaces. Crédito e licença MIT do Omarchy se algum trecho for adaptado.

## REA (github.com/morluto/rea, avaliado 24/10)
"Reverse Engineer Anything": MCP (Node 22+) para engenharia reversa de binários, apps e comportamento em execução, apoiado em Hopper, Ghidra ou IDA (ferramentas comerciais à parte); MIT, uso legal e autorizado. Serve pouco aqui: os doadores (Hyprland, hyprbars, KWin, Pop Shell, Ryoku) têm código aberto e se leem direto; para o Hyprland bastou `strings` e os cabeçalhos instalados. Reverter GPL não tira a obrigação GPL do que for derivado. Útil só se um dia houver um binário fechado a estudar.

## Distro baseada em Arch (ideia do dono, 24/10)
Recomendações registradas em `~/Downloads/ryoku-work/docs/distro/ESCOPO-DISTRO.md` (Kwinish independente em pacotes; Ryoku é GPL-3; escopo antes de nome/repositório). Nada decidido nem iniciado. Mapa das pastas: `ryoku-work/LEIA-ME.md`.

## Ideias avaliadas (24/10, texto de outra IA colado pelo dono; só o que se aplica)
- **Minimizar com janelas filhas/modais**: hoje `minimize` manda só a janela para `special:minimized`; diálogos filhos ficam para trás.
  Pendência real (testar com um app que abra diálogo; KWin trata por `transientFor`, só como conceito: o KWin é GPL e este projeto é MIT, nunca copiar código).
- **Página no Ryoku Hub** para layouts, regras e barras (hoje tudo em JSON/menu). Seguir a regra GUI-primeiro do vault.
- **Barras em apps com decoração própria (CSD)**: decidir se há opt-out por app (já existe "sem barra e moldura" e regra por app).
- Descartado: mexer na memória do `CWindow` para "esconder sem mudar de workspace" (frágil a cada versão do Hyprland); manter `special:minimized`.

## Atenção: o núcleo Lua precisa estar carregado de forma persistente
O plugin não faz nada de zona sozinho: sem `require("kwinish")` no config do Hyprland (ex.: `~/.config/hypr/user.lua` com
`package.path = "/usr/share/kwinish/core/?.lua;" .. package.path` e `pcall(require, "kwinish")`, ou `recipes/hyprland-snippet.lua`),
ninguém publica as zonas (`hl.plugin.kwinish.set_zone`) e o encaixe/prévia/Shift param. O `hyprctl eval` dos testes some em
qualquer recarga de config. O plugin agora avisa uma vez (notificação) ao arrastar sem zonas publicadas. Em 24/10 o núcleo do dono
estava só em memória; a edição do `user.lua` do dono depende de autorização dele.

**Bug (24/10, dono):** um clique simples na barra de uma janela maximizada tirava-a do estado maximizado e a jogava para um canto fora da tela (o arrasto de janela flutuante começava já no clique, com o retângulo maximizado como referência). Corrigido no plugin (`onMouseMove`/`handleDownEvent`: janela maximizada só inicia o arrasto após 6 px, sai do maximizado e volta sob o ponteiro no tamanho anterior); compilado, ainda não verificado ao vivo.

## Como testar sem estragar o desktop do dono (aprendido na pele)
- Ambiente em cada comando: `export XDG_RUNTIME_DIR=/run/user/1000 HYPRLAND_INSTANCE_SIGNATURE=$(ls -t /run/user/1000/hypr | head -1) WAYLAND_DISPLAY=wayland-1`
  (a assinatura **muda a cada login/reinício**; uma antiga faz o `hyprctl` falhar em silêncio).
- Teste no **workspace 9** (`hl.dsp.focus({ workspace = 9 })`) com kitties descartáveis (`kitty --class kwa ...`, flutuantes por
  regra no `exec_cmd`); no fim feche-os e volte ao workspace 1. **A janela do Claude no workspace 1 é do dono, nunca mexa**
  (ela está ladrilhada em (80, 58) 1810x992; se algo a mover, `hl.dsp.window.float({ action = "disable", window = "class:com.anthropic.Claude" })`, às vezes duas vezes).
- Ponteiro/teclado virtuais: scripts em `spike/` (`edge-drag2.py`, `hold-drag.py`, `shift-drag.py`, `click-at.py`, `rclick.py`, `menu-click.py`,
  `menu-drive.py`, `dblclick.py`, `wheel.py`, `chord-spike.py`) e `ryoku-work/ryoku-vm/tools/pointer-path.py`. O dispositivo uinput tem que declarar o botão
  esquerdo (0x110). No zsh, argumentos guardados numa variável não se separam: use Python/`bash` para montar listas.
  `grim -g "X,Y WxH"` (com `x` entre largura e altura).
- **Núcleo de teste**: `hyprctl eval "package.path='<repo>/core/?.lua;'..package.path; KWINISH_ZONE_HOOKS=nil; for _,m in ipairs({'kwinish','geometry','layouts'}) do package.loaded[m]=nil end; require('kwinish')"`.
- **`~/.local/bin/kwinish-ui`** (link para `ui/target/debug/kwinish-ui`) sobrepõe `/usr/bin`; crie para testar, **remova depois**.
- **Plugin**: carregar/descarregar (`hyprctl plugin load|unload`) é **bloqueado** para o assistente pelo classificador de segurança; o dono roda no terminal.
  O plugin de teste (`plugin/kwinish-bars.so`) e o instalado (`/usr/lib/hyprland/plugins/`) têm o mesmo nome: descarregue um antes do outro.
  Depois de `make -C plugin abi` o dono precisa recarregar. `sudo` pede senha: o dono instala com `pacman -U dist/*.pkg.tar.zst`.
- **Nunca** guarde nem chame métodos no objeto devolvido por `hl.bind` via `hyprctl eval` (derrubou o Hyprland 0.56.2 na sessão real). `pkill -f <padrão>`
  mata o próprio shell quando o padrão aparece no comando; use `pkill -x kwinish-ui`.
- O `/tmp` é tmpfs e some no reinício (inclusive o scratchpad e as pastas de build dos pacotes).

## Construir
```
make -C plugin abi                  # plugin/kwinish-bars.so e .abi
cargo build --manifest-path ui/Cargo.toml
for t in geometry binds switcher layouts; do lua5.4 core/test_$t.lua; done; cargo test --manifest-path ui/Cargo.toml
# pacotes: copie packaging/PKGBUILD e packaging/plugin/PKGBUILD para uma pasta de trabalho e rode
KWINISH_SRC=$PWD makepkg -f --nodeps   # em cada uma; copie os .pkg.tar.zst para dist/
```

## Decisões do dono
Barras em todas as janelas (`floating_only` opcional); como o KWin (ícone, fixar, menu no ícone); plugin como barra principal e layer-shell
como reserva com aviso de emergência; recompilação automática por hook do pacman, sem pausar upgrade; nome Kwinish por enquanto; MIT; o dono
instala e recarrega o plugin; o dono gosta do KDE em: presets de tiling, prévia do encaixe, menu de janela.
