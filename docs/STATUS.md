# Kwinish: onde estamos (25/10/2026)

Resumo geral para o dono. Detalhes técnicos e armadilhas: `docs/HANDOFF.md`. Repositório git local, 88 commits, nada publicado.

## O que é
Uma camada de gerenciamento de janelas estilo KDE/KWin para o Hyprland 0.56.2, em três peças:
- **Plugin `kwinish-bars`** (C++, fork do hyprbars): barra de título com ícone, fixar, minimizar/maximizar/fechar, menu, encaixe nas bordas e zonas.
- **Núcleo Lua** (`core/`): a lógica (encaixe, zonas, modos, regras, Alt+Tab, atalhos), carregada pela config do Hyprland.
- **UI em Rust** (`ui/`, `kwinish-ui`): menu da janela, Alt+Tab, seletor e editor de layouts, diálogo de atalho, barras de reserva.

## Feito e verificado ao vivo
- Barra de título completa: ícone, fixar, título, três botões, cores ativa/inativa, duplo clique maximiza, roda sobe/desce a janela.
- Encaixe nas bordas esquerda/direita/topo com pré-visualização; Shift ao arrastar mostra as zonas do layout; desencaixar devolve o tamanho anterior.
- Layouts e zonas, com editor visual, seletor, e arquivo `layouts.json` do usuário.
- Menu da janela estilo KDE: submenus, Mover/Redimensionar, regras por janela/app (posição, tamanho, sempre na área, acima, abaixo, sem moldura, ocultar de capturas), atalho por janela.
- Três modos por workspace (tiling, scrolling, floating) com geometria guardada em etiquetas, colunas por tela lembradas.
- Manter acima/abaixo, minimizar/restaurar (`special:minimized`), mostrar área de trabalho, empacotar, eixo, cantos ativos.
- **Snap Assist** (25/10): ao encaixar uma janela numa zona (menu ou Shift ao arrastar), uma sobreposição cobre a próxima zona vazia com as outras janelas flutuantes do workspace; escolher encaixa e passa para a zona seguinte. `core/assist.lua`, `ui/src/assist.rs`. Verificado ao vivo pelo menu com o layout de três colunas; o gesto com Shift ao arrastar não foi testado com mouse real.
- Vários monitores: menu e editor abrem no monitor sob o ponteiro (testado com HDMI-A-1 + DP-1).
- Pacotes de rascunho (`kwinish-git`, `kwinish-bars-plugin-git`) com hook do pacman que recompila o plugin. Instalado: `0.0.0.r50.babd9b8-1`.
- Qualidade: 7 arquivos de teste Lua passando (incluindo o novo simulador de `hl`, `core/test_kwinish.lua`), testes Rust em `ui/`, três rodadas de revisão (Claude no Alethe, Codex em `plugin/`+`core/`, Codex em `ui/`) com correções aplicadas.
- Licenças: cabeçalhos SPDX em `plugin/`; texto BSD-3 do hyprbars em `plugin/LICENSE.hyprland-plugins` **conferido hoje contra o upstream** (`hyprwm/hyprland-plugins`, ramo main): idêntico, copyright "2023, Hypr Development".

## Corrigido no código, ainda não verificado ao vivo
Plugin recompilado mas **não recarregado** pelo dono: correções das revisões (barras sobrepostas, estado de arrasto preso, limpeza ao fechar janela, Esc no Mover/Redimensionar, maximizada que fugia da tela). O núcleo foi testado no workspace 9; o plugin precisa de `hyprctl plugin unload/load` feito por você.

## O que falta
**Depende de você (desktop real):** recarregar o plugin e testar as correções; `sudo pacman -U dist/*.pkg.tar.zst`; teste com mouse real; Shift ao arrastar no 2º monitor; monitores com escala ≠ 1 ou girados; hook do pacman num upgrade real do `hyprland`.

**Funcionalidades:**
1. Duplo clique no ícone fecha a janela (KDE faz).
2. Seletor de layout ao passar o mouse no botão maximizar.
3. Miniaturas no Alt+Tab (e no Snap Assist, hoje só ícone e título); `rounding > 0` nas zonas e na prévia.
4. Minimizar janela com filhas/modais (`transientFor`): diálogos ficam para trás.
5. Bind para ciclar o modo do workspace, indicador na barra, janela nova sem piscar, modo por monitor.
6. Página no Ryoku Hub para layouts, regras e barras (hoje JSON + menu).
7. Opt-out de barra em apps com decoração própria (CSD) por app.
8. Liga/desliga do encaixe e do Snap Assist nas configurações (hoje `kw.snap_assist=false` no `user.lua`).

**Bugs/pendências conhecidos:**
- Encaixe depois de arrastar entre monitores volta ao monitor de origem (plugin escolhe a zona pelo ponteiro, núcleo pelo monitor da janela).
- Clique em área coberta por outra janela aciona a barra da focada (precisa de teste ao vivo).
- Atalho: o submap anterior não é guardado (sempre `reset`); vários daemons de barras podem coexistir; corrida entre dois diálogos de atalho.
- Busca de ícones com I/O no render; `snap` guarda o retângulo pedido, não o obtido; estado do mostrar-área-de-trabalho em tabela Lua.
- Auto-raise nunca verificado ao vivo.

**Publicação:** escolher nome definitivo (hoje "Kwinish", provisório); testar remoção do pacote; não provado que nenhum trecho do Omarchy foi adaptado (a implementação é independente, mas não há comparação mecânica).

## Testes pendentes para o dono
**Regra do dono: todo comando e todo teste deve ser escrito por inteiro na resposta do chat, nunca só apontado para este arquivo.** Cada item: o comando (para o fish), o que deve acontecer, como desfazer. Veja a regra em `docs/HANDOFF.md`.

**1. Snap Assist com Shift ao arrastar (mouse real, plugin carregado)**
```fish
cd ~/Downloads/ryoku-work/kwinish
cargo build --manifest-path ui/Cargo.toml; and ln -sf $PWD/ui/target/debug/kwinish-ui ~/.local/bin/kwinish-ui
hyprctl eval "package.path='$PWD/core/?.lua;'..package.path; KWINISH_ZONE_HOOKS=nil; for _,m in ipairs({'kwinish','geometry','layouts','assist'}) do package.loaded[m]=nil end; require('kwinish').set_layout('thirds')"
```
Abra 3 janelas flutuantes, segure Shift e arraste uma pela barra até a primeira zona. Deve abrir uma sobreposição na zona seguinte com as outras janelas; clique numa para encaixar; repita. Esc cancela.
Desfazer: `rm ~/.local/bin/kwinish-ui; hyprctl reload`, e escolha de novo o seu layout no seletor (o teste troca o layout salvo).

**2. Plugin com as correções das revisões (barras sobrepostas, arrasto preso, Esc no Mover/Redimensionar, maximizada que fugia)**
```fish
cd ~/Downloads/ryoku-work/kwinish
make -C plugin abi
hyprctl plugin unload /usr/lib/hyprland/plugins/kwinish-bars.so
hyprctl plugin load $PWD/plugin/kwinish-bars.so
```
Teste: clique simples na barra de uma janela maximizada (não deve fugir da tela); arraste duas janelas sobrepostas; Mover pelo menu e aperte Esc.
Desfazer: `hyprctl plugin unload $PWD/plugin/kwinish-bars.so; hyprctl plugin load /usr/lib/hyprland/plugins/kwinish-bars.so`.

**3. Encaixe depois de arrastar para o 2º monitor (plugin recompilado em 26/10, precisa recarregar)**
```fish
cd ~/Downloads/ryoku-work/kwinish
make -C plugin abi
hyprctl plugin unload /usr/lib/hyprland/plugins/kwinish-bars.so
hyprctl plugin load $PWD/plugin/kwinish-bars.so
hyprctl eval "package.path='$PWD/core/?.lua;'..package.path; KWINISH_ZONE_HOOKS=nil; for _,m in ipairs({'kwinish','geometry','layouts','assist'}) do package.loaded[m]=nil end; require('kwinish')"
```
Com DP-1 e HDMI-A-1: abra uma janela flutuante no DP-1, arraste pela barra até a borda esquerda ou direita do HDMI-A-1 e solte; depois repita com Shift numa zona. A janela deve encaixar **no HDMI-A-1**, não voltar ao DP-1. Desfazer: como no teste 2 (descarregar o de teste, carregar o instalado) e `hyprctl reload`.

**4. Sem comando pronto ainda:** Shift ao arrastar no 2º monitor; monitores com escala ≠ 1 ou girados; hook do pacman num upgrade real do `hyprland`; auto-raise.

## Fora de escopo / descartado
Mexer na memória do `CWindow` para esconder janelas; copiar código GPL (KWin, Pop Shell, HyDE); distro própria (ideia registrada, nada iniciado).
