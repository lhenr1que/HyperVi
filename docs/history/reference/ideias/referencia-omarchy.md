# Omarchy 4.5 RC como referência

**Estado:** referência (MIT, leitura permitida com crédito)

O Omarchy 4.5 RC já oferece dwindle / scrolling / floating por workspace. A leitura do código dele inspirou o modo flutuante do Kwinish (implementação independente).

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
