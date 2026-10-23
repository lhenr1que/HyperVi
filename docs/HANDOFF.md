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
1. ~~Editor visual de zonas~~ feito (`ui/src/editor.rs`, lógica em `ui/src/zones.rs`, arquivo `~/.config/kwinish/layouts.json` lido por `core/layouts.lua`). Os divisores arrastam (`zones::edges`/`move_edge`; visto desenhado, arraste não testado com mouse). O editor desenha sobre a área útil do primeiro monitor (descontando reservas e `gaps_out`, não a barra de título do plugin). Falta: Verificado ao vivo (ws 9): layout do usuário no `layouts.json` → `set_layout` → `window_action(addr,"zone:N")` (inclusive bloco flutuante) e o seletor mostra o layout marcado. Não verificado ao vivo: o clique em Salvar do editor e o Shift ao arrastar com layout do usuário (mesma rota `z:N` dos presets).
2. Menu: Mover e Redimensionar (o Hyprland só inicia por bind de mouse; talvez o plugin consiga), atalho da janela, regras da janela/app.
3. Duplo clique no ícone fecha a janela (KDE faz; não implementado).
4. Vários monitores: menu e editor de zonas agora abrem no monitor sob o ponteiro (`ui/src/mon.rs`, `set_monitor`), verificado ao vivo com HDMI-A-1 ao lado do DP-1; `zone:N` e `tile:` na janela do HDMI deram as posições certas. Não verificado: Shift ao arrastar no 2º monitor, barras de reserva (`bars.rs` já usa origem do monitor), monitores com escala ≠ 1 ou girados.
5. Miniaturas no Alt+Tab; estado do núcleo que sobrevive a reload (hoje em tabelas Lua; usar tags); `rounding > 0` nas zonas e na prévia.
6. Testar o hook do pacman num upgrade real do `hyprland`; remoção do pacote; escolher nome definitivo e publicar (nome provisório "Kwinish", licença MIT).

## Atenção: o núcleo Lua precisa estar carregado de forma persistente
O plugin não faz nada de zona sozinho: sem `require("kwinish")` no config do Hyprland (ex.: `~/.config/hypr/user.lua` com
`package.path = "/usr/share/kwinish/core/?.lua;" .. package.path` e `pcall(require, "kwinish")`, ou `recipes/hyprland-snippet.lua`),
ninguém publica as zonas (`hl.plugin.kwinish.set_zone`) e o encaixe/prévia/Shift param. O `hyprctl eval` dos testes some em
qualquer recarga de config. O plugin agora avisa uma vez (notificação) ao arrastar sem zonas publicadas. Em 24/10 o núcleo do dono
estava só em memória; a edição do `user.lua` do dono depende de autorização dele.

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
