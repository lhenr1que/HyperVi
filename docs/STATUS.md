# Kwinish: onde estamos (27/10/2026)

Resumo geral para o dono. Detalhes técnicos e armadilhas: `docs/HANDOFF.md`; testes: `docs/TESTING.md`; regras para agentes: `AGENTS.md`. Repositório git local, 140 commits, nada publicado. Pacotes instalados (antigos, sem o trabalho de 26–27/10): `kwinish-git 0.0.0.r86.260f15b-1`, `kwinish-bars-plugin-git 0.0.0.r85.738905b-1`.

## O que é
Uma camada de gerenciamento de janelas estilo KDE/KWin para o Hyprland 0.56.2, em três peças:
- **Plugin `kwinish-bars`** (C++, fork do hyprbars): barra de título com ícone, fixar, minimizar/maximizar/fechar, menu, encaixe nas bordas e zonas.
- **Núcleo Lua** (`core/`): a lógica (encaixe, zonas, modos, regras, Alt+Tab, atalhos), carregada pela config do Hyprland. O Hyprland instalado roda a config em **Lua 5.5**.
- **UI em Rust** (`ui/`, `kwinish-ui`): menu da janela, Alt+Tab, seletor e editor de layouts, Snap Assist, janela de configurações, diálogo de atalho, barras de reserva.

## Feito e verificado ao vivo
- Barra de título completa: ícone, fixar, título, três botões, cores ativa/inativa, duplo clique maximiza, roda sobe/desce a janela.
- Encaixe nas bordas esquerda/direita/topo com pré-visualização, volta do topo com animação, Shift no meio do arrasto, fronteira DP-1/HDMI-A-1 sem pulo, resistência de fronteira (`seam_resistance`) sem tremor (dono, 26/10, mouse real e dois monitores).
- Encaixe pelo teclado sem opções e restauração (`fix-snap-entrypoints`, teste 6, 27/10).
- Layouts e zonas, com editor visual, seletor e `layouts.json` do usuário.
- **Snap Assist** pelo menu (25/10) e com Shift ao arrastar no mesmo monitor (`fix-assist-drop-context`, teste 1, 27/10; o relato não detalha soltar noutro monitor).
- Menu da janela estilo KDE: submenus, Mover/Redimensionar, regras por janela/app, atalho por janela, Mais ações > Configurações do Kwinish...
- **Janela de configurações** `kwinish-ui settings` (teste 5, 27/10): cor da barra e Snap Assist mudam na hora, a "Altura" redimensiona a barra inteira (plugin, `868f66a`), os valores voltam depois de `hyprctl reload` + `dev-reload.fish`.
- Três modos por workspace (tiling, scrolling, floating), geometria guardada em etiquetas, colunas por tela lembradas. **Janela nova no modo floating nasce flutuante**, centralizada no retângulo do modo e em cascata, sem passar pelo mosaico (`fix-floating-mode-open`, teste 7, 27/10; diálogo de tamanho fixo mantém o próprio tamanho, medido ao vivo com `zenity --info`).
- Manter acima/abaixo, minimizar/restaurar (`special:minimized`), mostrar área de trabalho, empacotar, eixo, cantos ativos.
- Vários monitores: menu e editor abrem no monitor sob o ponteiro (HDMI-A-1 + DP-1).
- Licenças: cabeçalhos SPDX em `plugin/`; BSD-3 do hyprbars em `plugin/LICENSE.hyprland-plugins` conferido contra o upstream.

## Feito, falta verificar ao vivo
- Correções das revisões no plugin: barras sobrepostas, arrasto preso, limpeza ao fechar janela, Esc no Mover/Redimensionar, maximizada que fugia da tela. O plugin do repositório já foi carregado em 27/10 (testes 5 e 7), mas esses cenários não foram exercidos de propósito: teste 2 abaixo.
- Encaixe depois de arrastar para o 2º monitor: teste 3 abaixo (hoje só o DP-1 está ligado).
- Snap Assist com geração, `flock` e revalidação do clique; o desfazer das regras ao desmarcar; `force_no_accel`; auto-raise.
- **Dicas nos botões** minimizar/maximizar/fechar (`add-titlebar-button-tooltips`, 27/10): texto e posição testados (`tools/test-plugin-tooltip.sh`), plugin compilado; desenho, atraso e descarregar com a dica na tela: teste 8 abaixo.
- Modo floating: workspace no 2º monitor (o `move` relativo ao monitor foi conferido só no Hyprland aninhado e no simulador) e sair da tela cheia dentro do modo (a spec diz que volta flutuante).

## Base de testes (etapa A, concluída em 27/10)
- **Comando único:** `bash tools/check.sh` (Lua 5.5, todas as suítes `core/test_*.lua`, os testes `tools/test-*.sh`, `cargo test --locked`, plugin compilado num temporário; HOME/XDG temporários, sem sockets da sessão). Sai 1 em qualquer falha ou ferramenta ausente. `--lua-only` é o que o `check()` do PKGBUILD roda. Detalhes, limites e uso futuro em CI: `docs/TESTING.md`.
- Último resultado (27/10, com `add-titlebar-button-tooltips`): código 0; 9 suítes Lua, 4 testes de `tools/` (o novo `test-plugin-tooltip.sh` compila e roda a lógica pura da dica, 22 casos), 18 testes Rust, plugin compilado.
- `tools/live-backup.sh` guarda e restaura o que um teste ao vivo muda (arquivo, link, mesmo quebrado, ou ausência); nunca `rm` em arquivo do dono.
- `spike/nested-open/run.py`: Hyprland aninhado aberto escondido no workspace 9 para testar Lua arriscado do compositor sem tocar na sessão do dono (casos `rules-mode`, `core-mode` e outros; ver o `README.md` dele). Janelas que vêm de outro processo (portal) abrem no workspace ativo do dono: não use para isso.
- Evidências completas da etapa A: commit `67376af` (este arquivo naquela versão).

## Pendências reais, em ordem sugerida
1. **Etapa B, `harden-user-data-io`** (em andamento, `openspec/changes/harden-user-data-io/tasks.md`; 13 tarefas desde `563ba41`, que acrescentou a 1.4, leitores Rust): leitores JSON sem `load` (hoje `rules.lua` e `layouts.lua` executam o arquivo como Lua: um laço trava o compositor), validação de layouts igual em Lua e Rust, gravação atômica com backup verificado.
   - Progresso (27/10): 1.1, 1.2 e 1.3 feitas (3/13). Decoder `core/kwjson.lua` (nome próprio: um módulo `json` de outra config venceria o `require`), com `core/test_kwjson.lua` e os casos compartilhados `core/testdata/json-cases.txt`, instalado pelo PKGBUILD (conferido em staging). Os seis leitores Lua (rules, layouts, settings, modes, columns, shortcuts) decodificam com ele, e `kwinish.lua` lê todos por `read_cfg` (no máximo 1 MiB + 1 byte, aviso `kwinish: <arquivo> ignored: <motivo>` em stderr uma vez por arquivo e motivo). Regressões em `core/test_userdata.lua`: contra os módulos de `d26c348`, 18 de 27 casos falham, e os laços de rules e layouts só param no `timeout`. `Hyprland --verify-config` numa config temporária (HOME e XDG temporários, sem sessão) com arquivos hostis: o núcleo novo carrega, avisa e ignora; o antigo aplicava o `settings.json` truncado. Contrato em `docs/API.md`, seção 4.
   - Medido em 27/10: o Lua da carga da config do Hyprland 0.56.2 tem um vigia, e um laço é interrompido depois de ~1,5 s de CPU (`[Lua] execution timed out in config reload`). O núcleo antigo, portanto, atrasava a carga em vez de travá-la; a leitura de `rules.json` ao abrir janela roda num timer, onde o vigia não foi medido.
   - Próximo: 1.4 (leitores Rust da `ui/`), depois a seção 2 (paridade de layouts).
2. **Testes ao vivo do dono:** itens 2, 3, 8 e 9 abaixo.
3. **Reinstalar os pacotes** (dono, `sudo`): sem isso, `hyprctl reload` volta ao núcleo e plugin antigos de `/usr/share` e `/usr/lib`, e tudo de 26–27/10 só vale depois de `fish tools/dev-reload.fish`. Procedimento: `docs/HANDOFF.md`, "Construir".
4. **Achados da auditoria do Codex (27/10) ainda não corrigidos** (reproduzidos em isolamento, não vistos no desktop); cada um vira mudança no OpenSpec:
   - ciclo de vida (etapa C): minimizar apaga a procedência do modo floating (a janela pode não voltar ao mosaico); "mostrar área de trabalho" perde as origens depois de um reload; um ajuste de colunas atrasado sobrescreve a escolha mais nova; recarregar o módulo mantém callbacks antigos; `zone` não confere tela cheia como `tile` confere;
   - entrada e comunicação (etapa D): a barra da janela focada aceita o clique destinado à janela da frente; leitura sem tempo-limite no socket das barras; o Assist não revalida workspace, monitor e layout na escolha; cache de símbolos dos botões sem escala;
   - entrega (etapa E): o `Makefile` do plugin ignora os `.hpp`; `kwinish-bars-rebuild` anuncia sucesso sem conferir os `install`; CI.
5. **Funcionalidades:** duplo clique no ícone fecha a janela; seletor de layout ao passar o mouse no maximizar; miniaturas no Alt+Tab e no Snap Assist, `rounding > 0` nas zonas e na prévia; minimizar com filhas/modais; bind para ciclar o modo do workspace, indicador na barra, modo por monitor; página no Ryoku Hub; opt-out de barra por app com decoração própria.
6. **Bug do shell achado em 27/10 (não é do Kwinish):** maximizar uma janela esvazia o QS Bar e deixa só o espaço reservado. Causa e correção proposta: seção "Relato do dono: maximizar esvazia o QS Bar" abaixo; confirmação ao vivo: teste 9.
7. **Bugs conhecidos:** encaixe depois de arrastar entre monitores volta ao de origem (teste 3); atalho: o submap anterior não é guardado, vários daemons de barras podem coexistir, corrida entre dois diálogos de atalho; busca de ícones com I/O no render; `snap` guarda o retângulo pedido, não o obtido.
8. **Publicação/beta** (`docs/BETA.md`): nome definitivo (hoje "Kwinish"), instalação limpa e remoção do pacote, hook do pacman num upgrade real do `hyprland`, separar docs públicas das internas, limpar o histórico. O projeto HDE (`../hde/`) espera o beta.

## Para a próxima sessão
1. Ler `AGENTS.md`, este arquivo e `docs/HANDOFF.md` (armadilhas: objetos de `hl.bind` por `hyprctl eval` derrubam o Hyprland; `window.open_early` não aceita flutuar/redimensionar; etiqueta posta por regra aparece como `nome*`).
2. `git status` e `git log --oneline -5`: a `master` deve estar limpa em `f6090d6` ou depois. Os diretórios não rastreados `.agents/skills/source-command-opsx-*` e as worktrees `kwinish-apply` e `.alethe/worktrees/job-0*` são antigos: não apagar sem o dono.
3. `bash tools/check.sh` deve sair 0 antes de começar.
4. `PATH=$HOME/.local/bin:$PATH openspec list` mostra `harden-user-data-io` e `add-titlebar-button-tooltips` (tarefas feitas; arquivar depois do teste 8 do dono); aplicar a `harden-user-data-io` com `/opsx:apply` (Claude) ou `$openspec-apply-change` (Codex), começando por um teste que falha.
5. Carregar o código do repositório no desktop é do dono: `fish ~/Downloads/ryoku-work/kwinish/tools/dev-reload.fish` (plugin e núcleo juntos; vale até o próximo `hyprctl reload`). Mudanças dentro de blocos protegidos por `KWINISH_*` só valem com um bloco novo ou com o pacote reinstalado.
6. Delegação: `codex exec -s read-only` para revisão e `-s workspace-write` numa worktree nova para implementar funcionaram; o dono tem cota do Codex (GPT-6.1). Revisar o diff antes de juntar.
7. O backup do teste 5 continua em `~/.local/state/kwinish-backup/teste-5` (o dono não rodou o desfazer). Para voltar ao `settings.json` de antes: `bash ~/Downloads/ryoku-work/kwinish/tools/live-backup.sh restore ~/.local/state/kwinish-backup/teste-5`; para manter as configurações atuais, o dono pode apagar essa pasta.

## Testes pendentes para o dono
**Regra do dono: todo comando e todo teste deve ser escrito por inteiro na resposta do chat, nunca só apontado para este arquivo.** Cada item: o comando (fish), o que deve acontecer, como desfazer. Todo roteiro carrega o plugin e o núcleo do repositório primeiro.

**2. Plugin com as correções das revisões (barras sobrepostas, arrasto preso, Esc no Mover/Redimensionar, maximizada que fugia)**
```fish
cd ~/Downloads/ryoku-work/kwinish
make -C plugin
fish tools/dev-reload.fish
```
Teste: clique simples na barra de uma janela maximizada (não deve fugir da tela); arraste duas janelas sobrepostas pela barra da de cima (só ela se move); Mover pelo menu da janela e aperte Esc (cancela).
Desfazer (volta ao plugin e núcleo instalados):
```fish
hyprctl plugin unload ~/Downloads/ryoku-work/kwinish/plugin/kwinish-bars.so
hyprctl plugin load /usr/lib/hyprland/plugins/kwinish-bars.so
hyprctl reload
```

**3. Encaixe depois de arrastar para o 2º monitor (precisa do DP-1 e do HDMI-A-1 ligados)**
```fish
cd ~/Downloads/ryoku-work/kwinish
make -C plugin
fish tools/dev-reload.fish
```
Abra uma janela flutuante no DP-1, arraste pela barra até a borda esquerda ou direita do HDMI-A-1 e solte; depois repita com Shift numa zona. A janela deve encaixar **no HDMI-A-1**, não voltar ao DP-1. Desfazer: os mesmos três comandos do teste 2.

**8. Dicas nos botões da barra (add-titlebar-button-tooltips, 27/10)**
Carregue o plugin e o núcleo do repositório (o primeiro `unload` só vale se o plugin instalado estiver carregado; se não estiver, o erro dele pode ser ignorado):
```fish
cd ~/Downloads/ryoku-work/kwinish
make -C plugin
hyprctl plugin unload /usr/lib/hyprland/plugins/kwinish-bars.so
fish tools/dev-reload.fish
```
Teste, numa janela com a barra do Kwinish (por exemplo um `kitty` flutuante):
1. Pare o ponteiro sobre o botão de minimizar e não mexa por 1 s: aparece logo abaixo do botão, por cima do conteúdo da janela, uma caixinha "Minimizar". Faça o mesmo no maximizar ("Maximizar") e no fechar ("Fechar").
2. Passe o ponteiro rápido pelos três botões sem parar: nenhuma dica.
3. Pare no maximizar até a dica aparecer e clique: a janela maximiza, a dica some e não volta enquanto o ponteiro não sair do botão. Saia e pare de novo no maximizar: a dica diz "Restaurar". Clique para restaurar.
4. Numa janela encostada na borda direita da tela, pare no fechar: a dica fica inteira dentro da tela.
5. Pare no botão de fixar (à esquerda): nenhuma dica.
6. Descarregar com a dica na tela: rode o comando abaixo, leve o ponteiro a um botão de minimizar de outra janela em menos de 5 s e espere. A dica aparece, depois o plugin é recarregado: a dica some, o compositor continua funcionando e as barras voltam.
```fish
sleep 5; fish ~/Downloads/ryoku-work/kwinish/tools/dev-reload.fish
```
O inglês ("Minimize", "Maximize", "Restore", "Close") vale quando o locale do Hyprland não começa por `pt`; não precisa testar ao vivo (exigiria reiniciar a sessão com outro `LANG`; está no teste automático).
Desfazer (volta ao plugin e ao núcleo instalados; nenhum arquivo seu é mudado pelo teste):
```fish
hyprctl plugin unload ~/Downloads/ryoku-work/kwinish/plugin/kwinish-bars.so
hyprctl plugin load /usr/lib/hyprland/plugins/kwinish-bars.so
hyprctl reload
```

**9. Confirmar a causa do QS Bar vazio ao maximizar (só leitura; vale com o plugin instalado ou o do repositório)**
Abra um terminal com a barra do Kwinish, maximize-o pelo botão de maximizar da barra (o QS Bar esvazia) e rode nele:
```fish
hyprctl activeworkspace -j | string match -r '"hasfullscreen": \w+'
hyprctl activewindow -j | string match -r '"fullscreen": \d'
```
Deve aparecer `"hasfullscreen": true` e `"fullscreen": 1` (1 = maximizada, 2 = tela cheia de verdade). Restaure pelo mesmo botão. Para ver que não depende do Kwinish, maximize o terminal pelo próprio Hyprland, sem a barra:
```fish
hyprctl eval 'hl.dispatch(hl.dsp.window.fullscreen({ mode = "maximized" }))'
```
O QS Bar deve esvaziar do mesmo jeito. Desfazer: rode o mesmo `hyprctl eval` de novo (ele alterna) ou clique no maximizar da barra.

**4. Sem comando pronto ainda:** Shift ao arrastar no 2º monitor; workspace floating no 2º monitor; monitores com escala ≠ 1 ou girados; hook do pacman num upgrade real do `hyprland`; auto-raise.

## Relato do dono: maximizar esvazia o QS Bar (27/10; causa no shell, não corrigido aqui)
Sintoma: clicar no maximizar da barra deixa a barra lateral do Ryoku (QS Bar) vazia, só com o espaço reservado. Cadeia, conferida no código e medida:
1. O botão manda `fullscreen 1` (`plugin/barDeco.cpp`, `runAction("maximize")`; o menu e o núcleo usam `hl.dsp.window.fullscreen({ mode = "maximized" })`): é o maximizar nativo do Hyprland, que respeita as áreas reservadas.
2. No Hyprland 0.56.2 o `hasfullscreen` da workspace vale para qualquer modo, maximizado incluído (`CFullscreenController::hasFullscreen(PHLWORKSPACE)` só olha o modo interno; só a consulta por monitor exige tela cheia de verdade). Medido no Hyprland aninhado (`spike/nested-open/run.py maximize`): maximizada → `hasfullscreen: true`, `fullscreen: 1`; tela cheia → `true`, `2`.
3. O provedor do shell `ryoku-wm-hyprland watch ... workspaces` (pacote `ryoku-desktop-hyprland`) copia esse `hasfullscreen` para `workspace.fullscreen` (`ryoku/wm/hyprland/watch.go`, `readWorkspaces`, no fork `ryoku-work/ryoku-fork`).
4. `Wm.outputHasFullscreen` (`/usr/lib/qt6/qml/Ryoku/Ui/Singletons/Wm.qml`) lê esse campo, e o `Frame.qml` do QS Bar (`~/.config/quickshell/shell/modules/bar/Frame.qml`, `monFullscreen`) esconde todos os elementos e tira a entrada; a reserva vem de outra superfície (`FrameEdge.qml`), que não olha tela cheia e continua lá. Numa tela cheia de verdade a janela cobre essa faixa; maximizada, não cobre, e sobra o espaço vazio.
Correção proposta ao dono (no shell, que este repositório não altera): no `readWorkspaces`, contar a workspace como tela cheia só quando um cliente dela tem `"fullscreen": 2` em `hyprctl clients -j` (o provedor já decodifica `clients -j` em `decodeWindows`). Assim o QS Bar, a dock, o modo jogo e a pausa do papel de parede deixam de tratar janela maximizada como tela cheia, o mesmo critério que o Hyprland usa por monitor. Contornar no Kwinish (maximizar sem o modo do Hyprland, flutuando e redimensionando) perderia o estado de maximizada (restaurar, arrastar para sair, a marca no menu) e não resolveria o pedido de maximizar dos próprios aplicativos, que cai no mesmo modo.

## Verificado pelo dono (histórico curto)
- 26/10, mouse real e dois monitores: encaixe nas bordas e topo, volta do topo (140 ms), Shift no meio do arrasto, fronteira sem pulo; correções de escala 2 nas bordas, posição do clique para desencaixar, faixa de 16 px nas fronteiras, monitor sob o ponteiro, `cursor:no_warps` no ajuste de colunas.
- 27/10: testes 6 e 1 ("os testes funcionaram"; roteiros no commit `60e19df`), teste 5 ("tudo funcionou"; roteiro no commit `a431458`), teste 7 ("os testes passaram"; diálogo medido pelo Claude). Mudanças arquivadas em `openspec/changes/archive/2026-10-27-*`; specs principais `window-snapping`, `snap-assist` e `workspace-modes`.

## Fora de escopo / descartado
Mexer na memória do `CWindow` para esconder janelas; copiar código GPL (KWin, Pop Shell, HyDE); distro própria (ideia registrada, nada iniciado); abrir a janela no "tamanho pedido pelo app" no modo floating (uma regra por workspace não sabe a tempo que a janela vai flutuar; medido em 27/10).
