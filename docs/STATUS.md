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
- `rules.json` (e outros arquivos lidos com `load`) pode travar o Hyprland ao carregar a config se tiver um laço Lua escrito à mão (achado do Codex em 26/10 no `settings.json`, já corrigido lá com leitura por padrão, sem `load`; mesma correção pendente nos outros).

**Publicação:** escolher nome definitivo (hoje "Kwinish", provisório); testar remoção do pacote; não provado que nenhum trecho do Omarchy foi adaptado (a implementação é independente, mas não há comparação mecânica).

## Etapa A — Base verificável (aprovada pelo dono em 27/10; em andamento)
Origem: auditoria do Codex de 27/10 (roadmap A–F) e plano aprovado pelo dono. Critério: qualquer teste que falhe dá código de saída ≠ 0; nenhuma suíte usa o runtime ou a config pessoal. Sem mudar modos, arquivos pessoais, worktrees ou diretórios não rastreados; sem reload nem carga de plugin. Sem proposta OpenSpec nova: o isolamento do runtime do Alt+Tab também consta na tarefa 3.3 de `harden-user-data-io` (etapa B), que continua aberta.
Achado ao planejar: o Hyprland instalado roda a config em **Lua 5.5** (`readelf` do `/usr/bin/Hyprland`: `liblua.so.5.5`, pacote `lua`); o `AGENTS.md` mandava usar `lua5.4`. As 9 suítes passam nas duas versões; o executor usa `lua` (o do compositor), com `LUA=lua5.4` para comparar.
**Retomar:** base `60e19df`; `git log --oneline 60e19df..` lista o que já entrou; cada commit marca seu item abaixo.
- [x] 1. `core/test_switcher.lua` com runtime temporário próprio (hoje grava `$XDG_RUNTIME_DIR/kwinish/switcher.json` da sessão quando rodado direto) e `tools/test-lua-isolation.sh`, que roda cada suíte com HOME/XDG sentinela e falha se algo for gravado ali.
- [x] 2. `tools/check.sh`, comando único: suítes Lua, testes de `tools/`, `cargo test --locked`, compilação do plugin num diretório temporário; HOME/XDG temporários e sem sockets da sessão; código ≠ 0 em qualquer falha, ferramenta ausente ou nenhuma suíte encontrada. `tools/test-check.sh` testa o executor com ferramentas simuladas.
- [ ] 3. `packaging/PKGBUILD`: `check()` chama `tools/check.sh --lua-only` (sem lista manual de suítes), `checkdepends=('lua')`.
- [ ] 4. Reversão segura: `tools/live-backup.sh` guarda e restaura arquivo, link (mesmo quebrado) ou ausência; o teste 5 deixa de apagar `settings.json` e o link do `kwinish-ui`.
- [ ] 5. Documentação: `AGENTS.md`, `README.md`, novo `docs/TESTING.md`, `docs/HANDOFF.md`.
- [ ] 6. Evidências registradas aqui; testes 6 e 1 movidos para verificados pelo dono.

## Para a próxima sessão (fim de 26/10/2026)
**Próxima mudança planejada (27/10):** `harden-user-data-io` — leitores JSON sem execução, validação de layouts alinhada entre Lua/Rust e publicação atômica com falhas verificadas e backup obrigatório. Proposta, especificação, design e tarefas em `openspec/changes/harden-user-data-io/`; não implementada. Correções de snap integradas em `285c67d`; testes ao vivo 1 e 6 pendentes, mudanças ainda não arquivadas.
**Atualização de 27/10 — fix-assist-drop-context:** implementação revisada e incorporada à master em 27/10; arquivamento aguarda teste ao vivo. Regressão reproduzida antes do conserto (`same-monitor drop must open Snap Assist`); sufixo do mesmo monitor agora permite o Assist, outro monitor continua sem Assist. As 9 suítes Lua passaram no simulador; teste com mouse real pendente no item 1 abaixo.
**Atualização de 27/10 — fix-snap-entrypoints:** implementação revisada e incorporada à master em 27/10; arquivamento aguarda teste ao vivo. `M.tile` normaliza opções ausentes; regressão reproduzida antes do conserto, 9 suítes Lua e 18 testes Rust passaram depois (testes simulados, sem plugin). O teste ao vivo está no item 6 de "Testes pendentes para o dono". `fix-assist-drop-context` também implementada, com teste ao vivo pendente.
**Estado:** o repositório está limpo e tudo commitado. O dono verificou ao vivo com mouse real e dois monitores: encaixe nas bordas e topo, volta do topo com animação, Shift no meio do arrasto, fronteira DP-1/HDMI-A-1 sem pulo, resistência de fronteira (`seam_resistance`, 200 px) sem tremor, e as correções da revisão do Codex no plugin. **Não verificado ao vivo:** Snap Assist com as mudanças novas (assist.json com geração, `flock`, revalidação do clique), o desfazer das regras ao desmarcar, `force_no_accel`.
**Como carregar o código do repositório:** `cd ~/Downloads/ryoku-work/kwinish; and fish tools/dev-reload.fish` (plugin e núcleo juntos; recarregar só o plugin traz de volta o núcleo de `/usr/share/kwinish`, que ignora `@monitor`). Só vale até o próximo `hyprctl reload`; o pacote ainda não foi reinstalado (precisa de sudo).
**Armadilhas aprendidas:** o evento `input.mouse.move` do Hyprland chega depois de o ponteiro mover (não cancela); a resistência usa um gancho em `CInputManager::onMouseMoved` (`plugin/main.cpp`). `CEventLoopTimer` não disparou no plugin: a animação do desencaixe é movida pelos eventos do mouse. `doLater` não deve capturar código do plugin. Focar janelas leva o ponteiro ao centro delas (`cursor:no_warps` durante o ajuste de colunas). O mouse simulado (`ryoku-vm/tools/pointer-path.py 3840 1080 ...`, tela total 3840x1080) é absoluto e não passa pelo gancho de movimento relativo.
**Decisões do dono desta sessão:** desmarcar uma regra desfaz na hora; a resistência de fronteira existe e deve ter espaço nas configurações; revisar o que o usuário configura antes do beta (feito: `docs/CONFIGURAVEL.md`).
**Feito em 26/10 (fim):** janela de configurações `kwinish-ui settings` (`ui/src/settings.rs`, `core/settings.lua`, `~/.config/kwinish/settings.json`): barras, cores, encaixe, resistência de fronteira e Snap Assist; cada mudança grava o arquivo e chama `apply_settings()` no núcleo; o núcleo também aplica o arquivo a cada carga da config (só com o plugin carregado; chaves e valores validados). Vista aberta ao vivo lendo os valores do compositor; **aplicar ao vivo não testado** (o núcleo em memória é o de `/usr/share`, sem `apply_settings`): teste 5 abaixo.
**Relato do dono (26/10, noite):** as mudanças da janela não valiam na hora. Causa: o núcleo em memória era o instalado (`/usr/share`), sem `apply_settings`; o `hyprctl eval` falhava calado. Corrigido: sem `apply_settings`, a janela manda a opção direto ao compositor (`hl.config`). Verificado ao vivo (`bar_padding` = 4 aplicou). Manter os valores depois de `hyprctl reload` ainda exige o núcleo novo (pacote reinstalado ou `dev-reload.fish`).
**OpenSpec adotado (27/10):** `openspec` 1.15.0 instalado em `~/.local` (`npm install -g @fission-ai/openspec@1.15.0`, telemetria desligada), `openspec init --tools claude,codex --language pt-BR`. Regras para agentes em `AGENTS.md` (o `CLAUDE.md` só aponta para ele). Mudanças relevantes passam por `openspec/changes/`; as duas primeiras são os bugs confirmados da auditoria do Codex: `fix-snap-entrypoints` (ALT+setas dava erro) e `fix-assist-drop-context` (Snap Assist nunca abria no Shift+arrastar). Implementadas e revisadas na master em 27/10; testes ao vivo pendentes, ainda não arquivadas. Outros achados da auditoria, a virar mudanças: `rules.json`/`layouts.json` lidos com `load`, gravações sem temporário, Makefile do plugin ignora `.h`, script de rebuild sem checar o `install`, sem CI, docs desatualizados (`PLUGIN.md`).
**Próximos passos, em ordem sugerida:** (1) teste 5 (janela de configurações; abre também pelo menu da janela, Mais ações > Configurações do Kwinish...); (2) teste ao vivo do Snap Assist (teste 1 abaixo); (3) os bloqueadores do beta em `BETA.md` (nome definitivo, licenças, instalação limpa e hook do pacman, separar docs públicas das internas, limpar o histórico antes de publicar); (4) backlog: duplo clique no ícone fecha a janela, miniaturas no Alt+Tab, modais ao minimizar, layouts por monitor, escala diferente de 1 e monitor rotacionado (a borda e a fronteira foram escritas para isso, mas só testadas com escala 1 e 2 lado a lado). O projeto HDE (`../hde/`) espera até o beta.
**Delegação:** `codex exec -s read-only` (revisão) e `-s workspace-write` numa worktree (implementação) funcionaram bem hoje, o dono tem muito limite do Codex. O `agy` está sem cota até ~28/10.

## Verificado pelo dono em 26/10 (mouse real, dois monitores)
Encaixe nas bordas e topo, voltar do topo ao tamanho anterior (com animação de 140 ms), Shift no meio do arrasto, fronteira DP-1/HDMI-A-1 sem o pulo da janela. Correções: escala 2 nas bordas, posição do clique para desencaixar, faixa de 16 px nas fronteiras, monitor sob o ponteiro, `cursor:no_warps` durante o ajuste de colunas. Dev: `fish tools/dev-reload.fish` carrega plugin e núcleo juntos (recarregar só o plugin traz de volta o núcleo de /usr/share). Resistência na fronteira (ideia do dono): não implementada, avaliar se ainda faz falta.

## Testes pendentes para o dono
**Regra do dono: todo comando e todo teste deve ser escrito por inteiro na resposta do chat, nunca só apontado para este arquivo.** Cada item: o comando (para o fish), o que deve acontecer, como desfazer. Veja a regra em `docs/HANDOFF.md`.

**6. Encaixe sem opções e restauração (fix-snap-entrypoints, 27/10)**
Revisado e integrado à master. O teste de regressão falha antes do conserto; as 9 suítes Lua passam depois. Pendente de verificação ao vivo. Para testar sem recarregar o plugin, com o núcleo e plugin já carregados, rode no fish:
```fish
cd ~/Downloads/ryoku-work/kwinish
hyprctl eval "package.path='$PWD/core/?.lua;'..package.path; package.loaded.kwinish=nil; require('kwinish')"
```
Use uma janela flutuante descartável, fora de tela cheia. Rode o comando abaixo e, durante os três segundos, foque essa janela:
```fish
sleep 3; hyprctl eval "require('kwinish').tile('left')"
```
Deve encaixar na metade esquerda. Execute novamente, refocando a mesma janela durante a espera: deve restaurar o retângulo anterior. Este teste verifica a função chamada pelo atalho. Os binds já registrados ainda capturam a função antiga: o teste de SUPER+ALT+Esquerda (padrão), ou do atalho personalizado, exige que a configuração normal carregue o núcleo atualizado e reinstale os binds durante o reload; não registrar `hl.bind` por eval. A instalação/configuração persistente continua pendente.

**1. Snap Assist com Shift ao arrastar (fix-assist-drop-context, 27/10)**
Com o mesmo núcleo temporário do teste 6 e o plugin já carregado, habilite três colunas e o Assist apenas em memória:
```fish
hyprctl eval "local kw=require('kwinish'); kw.layout='thirds'; kw.snap_assist=true; kw.publish_zones()"
```
Abra três janelas flutuantes descartáveis no mesmo workspace e monitor, fora das zonas. Segure Shift e arraste uma pela barra à primeira zona desse monitor. Deve abrir o Assist na próxima zona vazia; escolher outra janela deve encaixá-la; Esc cancela. Se houver dois monitores, soltar uma janela numa zona do outro monitor deve encaixar sem Assist. Requer `kwinish-ui assist` no PATH do compositor (a interface já existente); não é necessário criar/remover links nem recompilar o plugin.

**Desfazer ambos os testes:** restaure a janela de encaixe com a segunda chamada, feche as janelas descartáveis e rode:
```fish
hyprctl reload
```
Isso retorna ao núcleo/configuração instalados e ao layout salvo: os comandos acima não gravam o layout nem alteram dotfiles. As duas mudanças permanecem abertas no OpenSpec até a verificação ao vivo.

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

**5. Janela de configurações (núcleo do repositório carregado)**
```fish
cd ~/Downloads/ryoku-work/kwinish
cargo build --manifest-path ui/Cargo.toml
fish tools/dev-reload.fish
./ui/target/debug/kwinish-ui settings
```
Mude "Altura" para 34: as barras devem engordar na hora. Troque a cor "Barra, janela focada": a barra da janela ativa muda. Desligue "Snap Assist" e encaixe uma janela numa zona pelo menu: a sobreposição não deve abrir. Depois `hyprctl reload` seguido de `fish tools/dev-reload.fish`: os valores devem continuar (vêm do `settings.json`).
Menu: com o binário do repositório no PATH (`ln -sf $PWD/ui/target/debug/kwinish-ui ~/.local/bin/kwinish-ui`; o menu chama o `kwinish-ui` do PATH, e o de `/usr/bin` ainda não conhece `settings`), clique direito na barra de uma janela, Mais ações > Configurações do Kwinish...: a mesma janela abre; abrir de novo só traz a já aberta para a frente.
Desfazer: `rm ~/.config/kwinish/settings.json ~/.local/bin/kwinish-ui; hyprctl reload`.

**4. Sem comando pronto ainda:** Shift ao arrastar no 2º monitor; monitores com escala ≠ 1 ou girados; hook do pacman num upgrade real do `hyprland`; auto-raise.

## Fora de escopo / descartado
Mexer na memória do `CWindow` para esconder janelas; copiar código GPL (KWin, Pop Shell, HyDE); distro própria (ideia registrada, nada iniciado).
