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
- **Snap Assist** (25/10): ao encaixar uma janela numa zona (menu ou Shift ao arrastar), uma sobreposição cobre a próxima zona vazia com as outras janelas flutuantes do workspace; escolher encaixa e passa para a zona seguinte. `core/assist.lua`, `ui/src/assist.rs`. Verificado ao vivo pelo menu com o layout de três colunas; o gesto com Shift ao arrastar foi relatado como funcionando pelo dono em 27/10 (teste 1).
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
- Modo floating: a janela nova nasce em mosaico (área útil inteira) e só flutua ~160 ms depois (relato do dono e medição ao vivo com kitty descartável no ws 3, 27/10: mapeada em mosaico 1827x1000 aos 105 ms, flutuante 1100x682 aos 267 ms). Planejado em `openspec/changes/fix-floating-mode-open/`. Experimento no Hyprland aninhado feito (27/10, Codex + Claude, `spike/nested-open/`): flutuar dentro de `window.open_early` derruba o compositor; uma regra de janela nomeada por workspace com `float` + `size` + `center` faz a janela nascer flutuante no tamanho do modo, sem passar pelo mosaico; "tamanho pedido pelo app" não sai de uma regra por workspace (o app recebe o tamanho do mosaico antes do primeiro quadro). Falta o dono decidir a cascata; tarefas 2–4 abertas, nada implementado no núcleo.
- Encaixe depois de arrastar entre monitores volta ao monitor de origem (plugin escolhe a zona pelo ponteiro, núcleo pelo monitor da janela).
- Clique em área coberta por outra janela aciona a barra da focada (precisa de teste ao vivo).
- Atalho: o submap anterior não é guardado (sempre `reset`); vários daemons de barras podem coexistir; corrida entre dois diálogos de atalho.
- Busca de ícones com I/O no render; `snap` guarda o retângulo pedido, não o obtido; estado do mostrar-área-de-trabalho em tabela Lua.
- Auto-raise nunca verificado ao vivo.
- `rules.json` (e outros arquivos lidos com `load`) pode travar o Hyprland ao carregar a config se tiver um laço Lua escrito à mão (achado do Codex em 26/10 no `settings.json`, já corrigido lá com leitura por padrão, sem `load`; mesma correção pendente nos outros).

**Publicação:** escolher nome definitivo (hoje "Kwinish", provisório); testar remoção do pacote; não provado que nenhum trecho do Omarchy foi adaptado (a implementação é independente, mas não há comparação mecânica).

## Etapa A — Base verificável (aprovada pelo dono em 27/10; concluída em 27/10)
Origem: auditoria do Codex de 27/10 (roadmap A–F) e plano aprovado pelo dono. Critério: qualquer teste que falhe dá código de saída ≠ 0; nenhuma suíte usa o runtime ou a config pessoal. Sem mudar modos, arquivos pessoais, worktrees ou diretórios não rastreados; sem reload nem carga de plugin. Sem proposta OpenSpec nova: o isolamento do runtime do Alt+Tab também consta na tarefa 3.3 de `harden-user-data-io` (etapa B), que continua aberta.
Achado ao planejar: o Hyprland instalado roda a config em **Lua 5.5** (`readelf` do `/usr/bin/Hyprland`: `liblua.so.5.5`, pacote `lua`); o `AGENTS.md` mandava usar `lua5.4`. As 9 suítes passam nas duas versões; o executor usa `lua` (o do compositor), com `LUA=lua5.4` para comparar.
**Retomar:** base `60e19df`; `git log --oneline 60e19df..` lista o que já entrou; cada commit marca seu item abaixo.
- [x] 1. `core/test_switcher.lua` com runtime temporário próprio (gravava `$XDG_RUNTIME_DIR/kwinish/switcher.json` da sessão quando rodado direto) e `tools/test-lua-isolation.sh`, que roda cada suíte com HOME/XDG sentinela e falha se algo for gravado ali.
- [x] 2. `tools/check.sh`, comando único: suítes Lua, testes de `tools/`, `cargo test --locked`, compilação do plugin num diretório temporário; HOME/XDG temporários e sem sockets da sessão; código ≠ 0 em qualquer falha, ferramenta ausente ou nenhuma suíte encontrada. `tools/test-check.sh` testa o executor com ferramentas simuladas.
- [x] 3. `packaging/PKGBUILD`: `check()` chama `tools/check.sh --lua-only` (sem lista manual de suítes), `checkdepends=('lua')`.
- [x] 4. Reversão segura: `tools/live-backup.sh` guarda e restaura arquivo, link (mesmo quebrado) ou ausência; o teste 5 deixa de apagar `settings.json` e o link do `kwinish-ui`.
- [x] 5. Documentação: `AGENTS.md`, `README.md`, novo `docs/TESTING.md`, `docs/HANDOFF.md`.
- [x] 6. Evidências registradas aqui; testes 6 e 1 movidos para verificados pelo dono.

**Evidências (27/10, todas simuladas ou em diretórios temporários; nada ao vivo, sem reload nem plugin):**
- `bash tools/check.sh` no `4eccbb0`: código 0 em 19 s; Lua 5.5 com 9 suítes, 3 testes de `tools/` (47 casos), 18 testes Rust, plugin compilado no temporário. `plugin/kwinish-bars.so` do repositório intocado (26/10 21:52); nenhum temporário sobrou. Retrato de `~/.config/kwinish`, `~/.config/hypr`, runtime do kwinish, `~/.local/bin` e `~/.local/state` igual antes e depois, salvo o `~/.local/state/ryoku/screentime.json`, que o Ryoku atualiza sozinho.
- O laço antigo do `AGENTS.md` com uma suíte falhando (cópia temporária do repositório): imprime FALHOU e sai com **0**; `check.sh --lua-only` sai com **1**.
- `tools/test-lua-isolation.sh` falhou antes do conserto (`core/test_switcher.lua gravou ... runtime/kwinish`) e passa depois, em Lua 5.5 e 5.4.
- `tools/test-check.sh`: 30 casos (sucesso; falha em cada etapa; `lua`, `cargo` e `make` ausentes; nenhuma suíte; `--lua-only`; uso errado; HOME/XDG e sockets isolados; `CARGO_HOME` mantido). Cinco sabotagens do `check.sh` (pular as suítes se `lua -v` falhar, sair 0 com falhas, não mudar HOME, não remover os sockets, quebrar o isolamento) foram todas pegas.
- `tools/test-live-backup.sh`: 17 casos; três sabotagens (copiar seguindo o link, ignorar a marca de backup completo, apagar diretório no lugar de um ausente) pegas. O roteiro novo do teste 5 rodou no fish com HOME falso: `settings.json` e link do `kwinish-ui` voltaram idênticos (md5 e alvo), e no cenário vazio nada sobrou.
- `makepkg -f --check --noarchive` com BUILDDIR/SRCDEST/PKGDEST temporários: código 0, `check()` rodou `check.sh --lua-only` no clone do commit; com uma suíte falhando só na cópia de build, `check()` falhou (código 4). Nada instalado, sem `sudo`.

**Limites conhecidos:** sem tempo-limite por suíte (um laço infinito num teste trava o `check.sh`; a etapa B prevê subprocessos com tempo-limite para os leitores); o `Makefile` do plugin ainda ignora `.hpp` para `make -C plugin` normal (etapa E; o `check.sh` contorna compilando sempre); isolamento não é sandbox; CI não configurada.
**Próximo:** etapa B, `harden-user-data-io` em lotes (leitores primeiro). `fix-snap-entrypoints` e `fix-assist-drop-context` arquivadas em 27/10 (`openspec/changes/archive/2026-10-27-*`), com specs principais `window-snapping` e `snap-assist`.

## Para a próxima sessão (fim de 26/10/2026)
**Próxima mudança planejada (27/10):** `harden-user-data-io` — leitores JSON sem execução, validação de layouts alinhada entre Lua/Rust e publicação atômica com falhas verificadas e backup obrigatório. Proposta, especificação, design e tarefas em `openspec/changes/harden-user-data-io/`; não implementada. Correções de snap integradas em `285c67d`; testes ao vivo 1 e 6 relatados como funcionando pelo dono em 27/10; mudanças arquivadas em 27/10.
**Atualização de 27/10 — fix-assist-drop-context:** implementação revisada e incorporada à master em 27/10; arquivada em 27/10 (spec principal `openspec/specs/snap-assist`). Regressão reproduzida antes do conserto (`same-monitor drop must open Snap Assist`); sufixo do mesmo monitor agora permite o Assist, outro monitor continua sem Assist. As 9 suítes Lua passaram no simulador; o dono relatou o teste com mouse real (item 1) como funcionando em 27/10; arquivada em 27/10.
**Atualização de 27/10 — fix-snap-entrypoints:** implementação revisada e incorporada à master em 27/10; arquivada em 27/10 (spec principal `openspec/specs/window-snapping`). `M.tile` normaliza opções ausentes; regressão reproduzida antes do conserto, 9 suítes Lua e 18 testes Rust passaram depois (testes simulados, sem plugin). O dono relatou o teste ao vivo (item 6) como funcionando em 27/10; arquivada em 27/10. `fix-assist-drop-context` também implementada, com teste ao vivo pendente.
**Estado:** o repositório está limpo e tudo commitado. O dono verificou ao vivo com mouse real e dois monitores: encaixe nas bordas e topo, volta do topo com animação, Shift no meio do arrasto, fronteira DP-1/HDMI-A-1 sem pulo, resistência de fronteira (`seam_resistance`, 200 px) sem tremor, e as correções da revisão do Codex no plugin. **Não verificado ao vivo:** Snap Assist com as mudanças novas (assist.json com geração, `flock`, revalidação do clique), o desfazer das regras ao desmarcar, `force_no_accel`.
**Como carregar o código do repositório:** `cd ~/Downloads/ryoku-work/kwinish; and fish tools/dev-reload.fish` (plugin e núcleo juntos; recarregar só o plugin traz de volta o núcleo de `/usr/share/kwinish`, que ignora `@monitor`). Só vale até o próximo `hyprctl reload`; o pacote ainda não foi reinstalado (precisa de sudo).
**Armadilhas aprendidas:** o evento `input.mouse.move` do Hyprland chega depois de o ponteiro mover (não cancela); a resistência usa um gancho em `CInputManager::onMouseMoved` (`plugin/main.cpp`). `CEventLoopTimer` não disparou no plugin: a animação do desencaixe é movida pelos eventos do mouse. `doLater` não deve capturar código do plugin. Focar janelas leva o ponteiro ao centro delas (`cursor:no_warps` durante o ajuste de colunas). O mouse simulado (`ryoku-vm/tools/pointer-path.py 3840 1080 ...`, tela total 3840x1080) é absoluto e não passa pelo gancho de movimento relativo.
**Decisões do dono desta sessão:** desmarcar uma regra desfaz na hora; a resistência de fronteira existe e deve ter espaço nas configurações; revisar o que o usuário configura antes do beta (feito: `docs/CONFIGURAVEL.md`).
**Feito em 26/10 (fim):** janela de configurações `kwinish-ui settings` (`ui/src/settings.rs`, `core/settings.lua`, `~/.config/kwinish/settings.json`): barras, cores, encaixe, resistência de fronteira e Snap Assist; cada mudança grava o arquivo e chama `apply_settings()` no núcleo; o núcleo também aplica o arquivo a cada carga da config (só com o plugin carregado; chaves e valores validados). Vista aberta ao vivo lendo os valores do compositor; **aplicar ao vivo não testado** (o núcleo em memória é o de `/usr/share`, sem `apply_settings`): teste 5 abaixo.
**Relato do dono (26/10, noite):** as mudanças da janela não valiam na hora. Causa: o núcleo em memória era o instalado (`/usr/share`), sem `apply_settings`; o `hyprctl eval` falhava calado. Corrigido: sem `apply_settings`, a janela manda a opção direto ao compositor (`hl.config`). Verificado ao vivo (`bar_padding` = 4 aplicou). Manter os valores depois de `hyprctl reload` ainda exige o núcleo novo (pacote reinstalado ou `dev-reload.fish`).
**OpenSpec adotado (27/10):** `openspec` 1.15.0 instalado em `~/.local` (`npm install -g @fission-ai/openspec@1.15.0`, telemetria desligada), `openspec init --tools claude,codex --language pt-BR`. Regras para agentes em `AGENTS.md` (o `CLAUDE.md` só aponta para ele). Mudanças relevantes passam por `openspec/changes/`; as duas primeiras são os bugs confirmados da auditoria do Codex: `fix-snap-entrypoints` (ALT+setas dava erro) e `fix-assist-drop-context` (Snap Assist nunca abria no Shift+arrastar). Implementadas e revisadas na master em 27/10; testes ao vivo relatados como funcionando pelo dono em 27/10, arquivadas em 27/10. Outros achados da auditoria, a virar mudanças: `rules.json`/`layouts.json` lidos com `load`, gravações sem temporário, Makefile do plugin ignora `.h`, script de rebuild sem checar o `install`, sem CI, docs desatualizados (`PLUGIN.md`).
**Próximos passos, em ordem sugerida:** (1) ~~teste 5~~ verificado em 27/10; (2) etapa B (`harden-user-data-io`, leitores primeiro); a janela nova no modo floating (`fix-floating-mode-open`) pode correr em paralelo, porque mexe em outra parte do núcleo; (3) os bloqueadores do beta em `BETA.md` (nome definitivo, licenças, instalação limpa e hook do pacman, separar docs públicas das internas, limpar o histórico antes de publicar); (4) backlog: duplo clique no ícone fecha a janela, miniaturas no Alt+Tab, modais ao minimizar, layouts por monitor, escala diferente de 1 e monitor rotacionado (a borda e a fronteira foram escritas para isso, mas só testadas com escala 1 e 2 lado a lado). O projeto HDE (`../hde/`) espera até o beta.
**Delegação:** `codex exec -s read-only` (revisão) e `-s workspace-write` numa worktree (implementação) funcionaram bem hoje, o dono tem muito limite do Codex. O `agy` está sem cota até ~28/10.

## Verificado pelo dono em 26/10 (mouse real, dois monitores)
Encaixe nas bordas e topo, voltar do topo ao tamanho anterior (com animação de 140 ms), Shift no meio do arrasto, fronteira DP-1/HDMI-A-1 sem o pulo da janela. Correções: escala 2 nas bordas, posição do clique para desencaixar, faixa de 16 px nas fronteiras, monitor sob o ponteiro, `cursor:no_warps` durante o ajuste de colunas. Dev: `fish tools/dev-reload.fish` carrega plugin e núcleo juntos (recarregar só o plugin traz de volta o núcleo de /usr/share). Resistência na fronteira (ideia do dono): não implementada, avaliar se ainda faz falta.

## Verificado pelo dono em 27/10
Teste 5, janela de configurações (relato do dono, 27/10: "tudo funcionou"): a cor "Barra, janela focada" muda na hora; desligar o Snap Assist impede a sobreposição ao encaixar pelo menu; a "Altura" redimensiona a barra inteira (conserto do plugin em `868f66a`); os valores continuam depois de `hyprctl reload` + `dev-reload.fish`; e Mais ações > Configurações do Kwinish... abre a janela. O roteiro está no `docs/STATUS.md` do commit `a431458`; se o "desfazer" não foi rodado, o backup continua em `~/.local/state/kwinish-backup/teste-5`.
Testes 6 (encaixe sem opções e restauração, `fix-snap-entrypoints`) e 1 (Snap Assist com Shift ao arrastar, `fix-assist-drop-context`): depois dos roteiros, o dono respondeu "os testes funcionaram" (27/10, 17:55, no chat do Codex). O relato não detalha os subcasos (por exemplo, soltar noutro monitor sem Assist). Os roteiros estão no `docs/STATUS.md` do commit `60e19df`.

## Testes pendentes para o dono
**Regra do dono: todo comando e todo teste deve ser escrito por inteiro na resposta do chat, nunca só apontado para este arquivo.** Cada item: o comando (para o fish), o que deve acontecer, como desfazer. Veja a regra em `docs/HANDOFF.md`.

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
