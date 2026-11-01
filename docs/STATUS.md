# Kwinish: estado e evidências

Revisão documental: **29/10/2026**, baseline `a2506c63dad16384414dc95919b8292aead284df` (173 commits). Nenhum teste, build, instalação ou ação no compositor foi executado nesta consolidação.

Arquitetura: [ARCHITECTURE](ARCHITECTURE.md); workflow: [CONTRIBUTING](CONTRIBUTING.md); armadilhas e relatos originais: [HANDOFF histórico](history/handoffs/HANDOFF.md); testes: [TESTING](TESTING.md); regras: [AGENTS](../AGENTS.md).

**Inventário histórico, não consulta atual da máquina:** a [auditoria de 28/10](history/product-import/2026-10-28/external-originals/docs/product/engineering/AUDITORIA_CODEX_ORIGINAL_2026-10-28.txt) registra os dois pacotes em `0.0.0.r172.5333087-1` e Hyprland `0.56.2-4`. Referências a r86/r85 abaixo pertencem aos roteiros e resultados anteriores, incluindo seus meios de desfazer. Não demonstram quais pacotes estão instalados hoje.

## Limites atuais e produto futuro

Atualizado em **01/11/2026** (branch do port para o Hyprland 0.57; o estado de cada etapa está em `docs/DEVELOPMENT_STATE.md`).
- **Base: Hyprland 0.57.0.** A linha 0.56.2 fica no histórico do Git. O plugin é compilado para o Hyprland exato em que roda.
- **Consertados, só em simulação, à espera do teste ao vivo do dono** (mudanças OpenSpec ativas, com roteiro no `tasks.md` de cada uma e abaixo, em "Testes pendentes"):
  - `fix-core-lifecycle`: cascata do modo Floating (falha ao vivo de 28/10), procedência do modo ao minimizar e em "mostrar área de trabalho", origens de "mostrar área de trabalho" que se perdiam ao recarregar o núcleo, ajuste de colunas atrasado, ganchos de `setup()` duplicados na recarga, `zone` em tela cheia;
  - `fix-input-ipc`: clique e roda numa barra coberta pela janela da frente, símbolos dos botões por escala de monitor, socket das barras sem prazo, escolha do Snap Assist sem revalidação.
- **Entrega:** `Makefile` com os cabeçalhos, pacote do plugin com todos os fontes, recompilação que confere os `install` e CI (`.github/workflows/check.yml`); instalação, remoção e upgrade reais ainda não foram testados, e a CI ainda não rodou (sem remoto).
- **Riscos aceitos** (tabela em `docs/DEVELOPMENT_STATE.md`): tiling centrado no cursor ao arrastar, `floatWindow` do núcleo encerra drag ativo, C1/C2, opção (b) do achado 6. O toque na barra continua desligado (spec `title-bar`).
- **Ainda não testados ao vivo:** segundo monitor (Shift ao arrastar, modo floating, encaixe ao soltar noutro monitor), escalas diferentes de 1, monitor girado, hook do pacman num upgrade real do `hyprland`.
- Barra/dock/launcher oficiais, Overview e Rice Store não fazem parte da implementação atual. D-43/D-44/D-45 e a API futura de restauração pertencem ao [produto](product/WORKSPACES.md), sem validação executada.

## O que é
Uma camada de gerenciamento de janelas estilo KDE/KWin para o Hyprland 0.57 (a linha 0.56.2 fica no histórico do Git), em três peças:
- **Plugin `kwinish-bars`** (C++, fork do hyprbars): barra de título com ícone, fixar, minimizar/maximizar/fechar, menu, encaixe nas bordas e zonas.
- **Núcleo Lua** (`core/`): a lógica (encaixe, zonas, modos, regras, Alt+Tab, atalhos), carregada pela config do Hyprland. No ambiente dos testes registrados, o Hyprland 0.56.2 rodava a config em **Lua 5.5**.
- **UI em Rust** (`ui/`, `kwinish-ui`): menu da janela, Alt+Tab, seletor e editor de layouts, Snap Assist, janela de configurações, diálogo de atalho, barras de reserva.

## Implementado: resultados ao vivo registrados até 28/10/2026
- Barra de título completa: ícone, fixar, título, três botões, cores ativa/inativa, duplo clique maximiza, roda sobe/desce a janela.
- Encaixe nas bordas esquerda/direita/topo com pré-visualização, volta do topo com animação, Shift no meio do arrasto, fronteira DP-1/HDMI-A-1 sem pulo, resistência de fronteira (`seam_resistance`) sem tremor (dono, 26/10, mouse real e dois monitores).
- Encaixe pelo teclado sem opções e restauração (`fix-snap-entrypoints`, teste 6, 27/10).
- Layouts e zonas, com editor visual, seletor e `layouts.json` do usuário.
- **Snap Assist** pelo menu (25/10) e com Shift ao arrastar no mesmo monitor (`fix-assist-drop-context`, teste 1, 27/10; o relato não detalha soltar noutro monitor).
- Menu da janela estilo KDE: submenus, Mover/Redimensionar, regras por janela/app, atalho por janela, Mais ações > Configurações do Kwinish...
- **Janela de configurações** `kwinish-ui settings` (teste 5, 27/10): cor da barra e Snap Assist mudam na hora, a "Altura" redimensiona a barra inteira (plugin, `868f66a`), os valores voltam depois de `hyprctl reload` + `dev-reload.fish`.
- Três modos por workspace (tiling, scrolling, floating), geometria guardada em etiquetas, colunas por tela lembradas. **Janela nova no modo floating nasce flutuante**, centralizada no retângulo do modo, sem passar pelo mosaico (`fix-floating-mode-open`, teste 7, 27/10; diálogo de tamanho fixo mantém o próprio tamanho, medido ao vivo com `zenity --info`). Só com o núcleo do repositório ou um pacote de 27/10 em diante: o pacote r86 instalado na data daquele relato (25/10) fazia a janela flutuar 150 ms depois de abrir (relato do dono em 28/10, teste 12). **A cascata não anda ao vivo** (28/10): três `kitty` abertos no ws 3 com o núcleo do repositório nasceram flutuantes, sem evento `changefloatingmode`, mas todos em (385, 168) 1100x682, passo 1, embora cada um tenha recebido `kwinish-mode-floated` (então `mode_window_opened` rodou e redeclarou a regra). No simulador a redeclaração troca o retângulo; no Hyprland 0.56.2, chamada de dentro do `window.open`, aparentemente não. Não corrigido.
- Minimizar/restaurar (`special:minimized`), mostrar área de trabalho, empacotar, eixo, cantos ativos.
- **Dicas nos botões** minimizar/maximizar/fechar, estilo KDE, em português (`add-titlebar-button-tooltips`, teste 8, 27/10; o inglês só no teste automático `tools/test-plugin-tooltip.sh`).
- Vários monitores: menu e editor abrem no monitor sob o ponteiro (HDMI-A-1 + DP-1).
- Licenças: cabeçalhos SPDX em `plugin/`; BSD-3 do hyprbars em `plugin/LICENSE.hyprland-plugins` conferido contra o upstream.

## Feito, falta verificar ao vivo
- Correções das revisões no plugin: barras sobrepostas, arrasto preso, limpeza ao fechar janela, Esc no Mover/Redimensionar, maximizada que fugia da tela. O plugin do repositório já foi carregado em 27/10 (testes 5 e 7), mas esses cenários não foram exercidos de propósito: teste 2 abaixo.
- Encaixe depois de arrastar para o 2º monitor: teste 3 abaixo (na data do relato só o DP-1 estava ligado; não consultado nesta revisão).
- **Manter acima/abaixo das outras** sem `kw.setup()` (`5333087`, 28/10): o reerguimento a cada troca de foco só existia dentro de `setup()`, que o `user.lua` do dono não chama, e por isso a janela subia uma vez e perdia o lugar no clique seguinte (relato do dono no teste 11); o auto-elevar da receita também passava por cima dela. Ao vivo: teste 12.
- Snap Assist com geração, `flock` e revalidação do clique; o desfazer das regras ao desmarcar; `force_no_accel`; auto-raise.
- **Dicas no fixar e no ícone do aplicativo** (`add-pin-and-icon-tooltips`, 27/10, pedido do dono depois do teste 8): textos testados (`tools/test-plugin-tooltip.sh`), plugin compilado; ao vivo: teste 10 abaixo.
- Modo floating: workspace no 2º monitor (o `move` relativo ao monitor foi conferido só no Hyprland aninhado e no simulador) e sair da tela cheia dentro do modo (a spec diz que volta flutuante).

## Base de testes (etapa A, concluída em 27/10)

Os resultados deste bloco são históricos; a atualização da documentação não os repetiu. O resultado posterior de 28/10 está na etapa B abaixo (11 suítes Lua, 4 de tools, 31 testes Rust e plugin compilado).
- **Comando único:** `bash tools/check.sh` (Lua 5.5, todas as suítes `core/test_*.lua`, os testes `tools/test-*.sh`, `cargo test --locked`, plugin compilado num temporário; HOME/XDG temporários, sem sockets da sessão). Sai 1 em qualquer falha ou ferramenta ausente. `--lua-only` é o que o `check()` do PKGBUILD roda. Detalhes, limites e uso futuro em CI: `docs/TESTING.md`.
- Último resultado (27/10, com `add-pin-and-icon-tooltips`): código 0; 11 suítes Lua, 4 testes de `tools/` (`test-plugin-tooltip.sh` compila e roda a lógica pura da dica, 28 casos), 23 testes Rust, plugin compilado.
- `tools/live-backup.sh` guarda e restaura o que um teste ao vivo muda (arquivo, link, mesmo quebrado, ou ausência); nunca `rm` em arquivo do dono.
- `spike/nested-open/run.py`: Hyprland aninhado aberto escondido no workspace 9 para testar Lua arriscado do compositor sem tocar na sessão do dono (casos `rules-mode`, `core-mode` e outros; ver o `README.md` dele). Janelas que vêm de outro processo (portal) abrem no workspace ativo do dono: não use para isso.
- Evidências completas da etapa A: commit `67376af` (este arquivo naquela versão).

## Pendências técnicas e propostas (ordem sugerida no relato anterior)
1. **Etapa B, `harden-user-data-io`** (tarefas concluídas em 28/10, falta o teste ao vivo 11, `openspec/changes/harden-user-data-io/tasks.md`; 13 tarefas desde `563ba41`, que acrescentou a 1.4, leitores Rust): leitores JSON sem `load`, validação de layouts igual em Lua e Rust, gravação atômica com backup verificado. **Situação anterior corrigida:** `rules.lua` e `layouts.lua` executavam o arquivo como Lua. Os leitores atuais usam `kwjson`; os resultados e os limites medidos estão abaixo.
   - Progresso (27/10): 1.1, 1.2 e 1.3 feitas (3/13). Decoder `core/kwjson.lua` (nome próprio: um módulo `json` de outra config venceria o `require`), com `core/test_kwjson.lua` e os casos compartilhados `core/testdata/json-cases.txt`, instalado pelo PKGBUILD (conferido em staging). Os seis leitores Lua (rules, layouts, settings, modes, columns, shortcuts) decodificam com ele, e `kwinish.lua` lê todos por `read_cfg` (no máximo 1 MiB + 1 byte, aviso `kwinish: <arquivo> ignored: <motivo>` em stderr uma vez por arquivo e motivo). Regressões em `core/test_userdata.lua`: contra os módulos de `d26c348`, 18 de 27 casos falham, e os laços de rules e layouts só param no `timeout`. `Hyprland --verify-config` numa config temporária (HOME e XDG temporários, sem sessão) com arquivos hostis: o núcleo novo carrega, avisa e ignora; o antigo aplicava o `settings.json` truncado. Contrato em `docs/API.md`, seção 4.
   - Medido em 27/10: o Lua da carga da config do Hyprland 0.56.2 tem um vigia, e um laço é interrompido depois de ~1,5 s de CPU (`[Lua] execution timed out in config reload`). O núcleo antigo, portanto, atrasava a carga em vez de travá-la; a leitura de `rules.json` ao abrir janela roda num timer, onde o vigia não foi medido.
   - 1.4 feita (4/13): `ui/src/zones.rs` tem `json`, `read_text` e `read_json` (no máximo 1 MiB + 1 byte, 32 níveis contados como no Lua); passam por eles os oito leitores de arquivo JSON da interface (layouts, settings, shortcuts, rules, workspace-modes, workspace-columns, switcher, assist). Os mesmos casos de `core/testdata/json-cases.txt` dão o mesmo veredito no serde_json e no `kwjson`. O teste de backup do `write_atomic` usa o mesmo leitor e a raiz certa, então um arquivo que a leitura recusa vai para `.bak` antes de ser substituído. O menu marca só o que o núcleo aplica (regras, modo, classe válida). Antes do conserto: `zones::parse` aceitava 33 níveis e `assist::parse` aceitava mais de 1 MiB (testes vermelhos). Fora: saída do `hyprctl`, o `.abi` do plugin e o arquivo de texto `layout`.
   - Seção 2 feita (6/13): casos compartilhados `core/testdata/layout-cases.txt`, lidos por `core/test_layouts.lua` e `ui/src/zones.rs`. Antes do conserto, o Lua divergia em 7 casos (gap -5, 999999, 1e20, 2.5, 6.0 e "5", e nome de 65 caracteres); o Rust já acertava todos. O Lua agora aceita gap só inteiro, limitado a 0–60, e nome com até 64 caracteres. `layouts.zone_rects` devolve `false` para uma zona sem largura ou altura útil (ou com número não finito); a publicação ao plugin, o `zone:N` e o `assist.plan` a pulam, e as outras mantêm o número (testes em `test_layouts`, `test_assist` e no simulador). Documentado em `docs/CONFIGURAVEL.md`, item 6.
   - 3.1 e 3.2 feitas (8/13): `kwjson.write` grava por `<arquivo>.tmp` + `rename`, cria o diretório com o caminho entre aspas seguras e, com `check`, copia para `.bak` um arquivo que a leitura recusa (sem cópia, não grava). Testado num sistema de arquivos em memória (falha de abertura, escrita, close e rename, primeira gravação, falha de leitura ou cópia do backup) e no simulador: com disco cheio, rules, modos, colunas e `layout` ficam com os bytes de antes (o código antigo os deixava vazios), e um `rules.json` recusado vai para `rules.json.bak`. `save_cfg` em `kwinish.lua` avisa em stderr e devolve false. Contrato em `docs/API.md`, seção 4.
   - 3.3 feita (9/13): switcher e Snap Assist publicam pelo `kwjson.write` (sem backup: runtime é descartável). Antes, uma gravação que falhava deixava `switcher.json` vazio para o `kwinish-ui` (teste novo em `test_switcher.lua`, que continua isolado num runtime temporário). O Assist mantém formato e geração (agora lida como JSON, não por busca no texto), e seus testes de atomicidade passam sem mudança. Todo caminho que vai ao shell passa por `kwjson.quote` (o `mkdir` e o `flock` do Assist).
   - Seção 4 feita (11/13): `write_atomic` lê o arquivo antigo em bytes (um arquivo que não é UTF-8 também vai para `.bak`), para se não conseguir lê-lo (fora "não existe") ou copiá-lo, e grava o backup e o destino por `publish` (temporário + rename, temporário removido em erro). Testes com falhas reais num diretório temporário (o `.bak` ocupado por um diretório, bytes que não são UTF-8, destino que é diretório) e um `rename` simulado: os três primeiros falhavam no código antigo. O editor de zonas e o diálogo de atalho agora avisam a falha (stderr e na própria janela), ficam abertos e não ativam nada; a janela de configurações já não enviava nada. Contrato em `docs/API.md`, seção 4.
   - 5.1 feita (12/13). Tudo simulado ou em isolamento, nada ao vivo. `bash tools/check.sh` saiu 0: 11 suítes Lua com XDG temporário, 4 testes de `tools/`, 29 testes Rust, plugin compilado. `check.sh --lua-only`, o `check()` do PKGBUILD, também saiu 0. Staging das linhas `install` do PKGBUILD: todos os módulos do núcleo, `kwjson` incluído, carregam só do pacote. Amostras `core/testdata/formats/` dos arquivos válidos como os gravadores sempre os fizeram (`zones::render` confere byte a byte): o núcleo novo e o antigo leem as mesmas entradas, então não há migração. `Hyprland --verify-config` com o núcleo do staging e essas amostras (HOME e XDG temporários, sem sessão) carrega, aplica e não grava nada.
   - 5.2 feita (13/13, 28/10). As conferências mecânicas passaram e `openspec validate --strict` também. Revisão independente pelo Codex, somente leitura (`codex exec -s read-only`): 11 defeitos, todos reproduzidos e corrigidos, cada um com teste que falhava antes (`ba2aa84`, `3f0722b`, `c372da3`, `d1d274d`):
     - o Lua lia `-0` como inteiro, e o serde como `-0.0`;
     - o serde recusava `1.7976931348623158e308`, que o Lua lê;
     - uma chave repetida escondia um 33º nível do serde;
     - os escapes `\u` dos casos compartilhados tinham virado caracteres, e um decoder que quebrasse pares surrogate passava em tudo;
     - um arquivo que não abre (permissão) era tratado como ausente e substituído sem cópia;
     - os gravadores liam o arquivo antigo inteiro (agora o início, até o limite, e o backup é copiado em blocos);
     - um texto que o próprio leitor recusaria podia ser publicado;
     - um arquivo só com espaços era substituído sem cópia;
     - o Rust ignorava erros de fechamento (agora `sync_all`);
     - o Alt+Tab focava uma seleção que o overlay não mostrou;
     - uma zona de 0,1 px era posta com 0 px;
     - os casos com `os.exit()` rodavam no processo da suíte, onde saem com código 0 (agora em subprocesso; conferido com um mutante que executa o arquivo).
     A sugestão, que layout e colunas continuam valendo na sessão quando a gravação falha, é o comportamento do design (sem transação) e ficou documentada em `docs/API.md`. `check.sh` sai 0: 11 suítes Lua, 4 de `tools/`, 31 testes Rust. `Hyprland --verify-config` com o núcleo final, sem sessão: hostis ignorados com aviso, válidos aplicados, nada gravado.
   - Próximo: o teste ao vivo 11 do dono e, com ele aprovado, `/opsx:archive harden-user-data-io`.
2. **Testes ao vivo do dono:** itens 2, 3, 10 e 11 abaixo.
3. **Conferir a versão carregada antes de futuros testes** (dono): o relato anterior de pacotes r86/r85 dizia que `hyprctl reload` voltava ao núcleo e plugin antigos de `/usr/share` e `/usr/lib`. A auditoria posterior registra r172.5333087 instalado, sem resultado completo do teste 12. Um reload usa o código da configuração instalada, não garante o HEAD do repositório. Esse era o próximo passo antes da instalação registrada na auditoria de 28/10; não há autorização automática para reinstalar. Procedimento histórico: [HANDOFF](history/handoffs/HANDOFF.md), "Construir".
4. **Achados da auditoria do Codex (27/10)** (reproduzidos em isolamento, não vistos no desktop). **01/11/2026:** as etapas C, D e E abaixo foram consertadas em simulação (`fix-core-lifecycle`, `fix-input-ipc`, commit `91b9cc1` para a entrega), à espera dos testes ao vivo CL e II; o texto original segue para referência:
   - ciclo de vida (etapa C): minimizar apaga a procedência do modo floating (a janela pode não voltar ao mosaico); "mostrar área de trabalho" perde as origens depois de um reload; um ajuste de colunas atrasado sobrescreve a escolha mais nova; recarregar o módulo mantém callbacks antigos; `zone` não confere tela cheia como `tile` confere;
   - entrada e comunicação (etapa D): a barra da janela focada aceita o clique destinado à janela da frente; leitura sem tempo-limite no socket das barras; o Assist não revalida workspace, monitor e layout na escolha; cache de símbolos dos botões sem escala;
   - entrega (etapa E): o `Makefile` do plugin ignora os `.hpp`; `kwinish-bars-rebuild` anuncia sucesso sem conferir os `install`; CI.
5. **Funcionalidades propostas, não compromisso de implementação:** duplo clique no ícone fecha a janela; seletor de layout ao passar o mouse no maximizar; miniaturas no Alt+Tab e no Snap Assist, `rounding > 0` nas zonas e na prévia; minimizar com filhas/modais; bind para ciclar o modo do workspace, indicador na barra, modo por monitor; página no Ryoku Hub; opt-out de barra por app com decoração própria.
6. **Bug do shell achado em 27/10 (não é do Kwinish):** maximizar uma janela esvazia o QS Bar e deixa só o espaço reservado. Causa e correção proposta: seção "Relato do dono: maximizar esvazia o QS Bar" abaixo; causa confirmada pelo dono (teste 9, 27/10). A correção é do shell.
7. **Bugs e validações pendentes:** o relato antigo de encaixe entre monitores voltando à origem motivou o sufixo de monitor em `tile:*`/`zone:*`; a correção existe no código e tem simulação registrada, mas o cenário de arrasto real continua pendente no teste 3. Demais achados registrados: atalho não guarda o submap anterior, vários daemons de barras podem coexistir, corrida entre dois diálogos de atalho; busca de ícones com I/O no render; `snap` guarda o retângulo pedido, não o obtido.
8. **Publicação/beta:** ideias históricas em [BETA](history/initial-research/BETA.md). Nome definitivo, publicação, testes de instalação/remoção e upgrade real continuam decisões/trabalhos separados. A consolidação preserva o histórico e não autoriza sua limpeza nem assume dependências externas de cronograma.

## Para a próxima sessão
1. Ler [AGENTS](../AGENTS.md), este arquivo, [arquitetura atual](ARCHITECTURE.md) e [HANDOFF histórico](history/handoffs/HANDOFF.md) (armadilhas: objetos de `hl.bind` por `hyprctl eval` derrubam o Hyprland; `window.open_early` não aceita flutuar/redimensionar; etiqueta posta por regra aparece como `nome*`).
2. `git status` e `git log --oneline -5`; o último marco e o commit de referência estão em `docs/DEVELOPMENT_STATE.md` (o baseline `a2506c6` citado antes aqui é de 28/10, superado). Mudanças documentais não commitadas podem existir; não confundi-las com trabalho de código. Os diretórios não rastreados `.agents/skills/source-command-opsx-*` e as worktrees `kwinish-apply` e `.alethe/worktrees/job-0*` são antigos: não apagar sem o dono.
3. Para trabalho de código, `bash tools/check.sh` é a validação de software prevista no AGENTS. Nesta tarefa exclusivamente documental, foram verificadas referências e preservação, sem executar suítes ou builds.
4. A CLI `openspec` não está instalada (01/11/2026). Nenhuma mudança ativa em `openspec/changes/`: `add-pin-and-icon-tooltips` e `harden-user-data-io` foram arquivadas em 01/11/2026, depois dos testes 10 e 11 (que passaram), e a segunda criou o spec principal `user-data-io`. A conferência estrutural dos specs principais é `tools/test-openspec-specs.sh`.
5. Carregar o código do repositório no desktop é do dono, pelos comandos do roteiro de cada teste (plugin num `.so` temporário e depois o núcleo por `hyprctl eval`; vale até o próximo `hyprctl reload`). O caminho `~/Downloads/ryoku-work/kwinish` citado nos roteiros antigos não existe mais. Mudanças dentro de blocos protegidos por `KWINISH_*` só valem com um bloco novo ou com o pacote reinstalado.
6. Delegação: `codex exec -s read-only` para revisão e `-s workspace-write` numa worktree nova para implementar funcionaram; o dono tem cota do Codex (GPT-6.1). Revisar o diff antes de juntar.
7. A anotação anterior registrava o backup do teste 5 em `~/.local/state/kwinish-backup/teste-5`, sem desfazer executado pelo dono. O comando histórico de restauração é `bash ~/Downloads/ryoku-work/kwinish/tools/live-backup.sh restore ~/.local/state/kwinish-backup/teste-5`. Conferido em 01/11/2026: `~/.local/state/kwinish-backup/` está vazio, sem `teste-5`; nada a restaurar.

## Testes pendentes para o dono

Roteiros e resultados parciais preservados de 27–28/10. **Não executados nesta consolidação, nem autorizados por ela.** Antes de um teste futuro, conferir versões e caminhos; os comandos de instalação/desfazer abaixo refletem os pacotes e backups da data original. A auditoria de 28/10 já registra r172 instalado, mas não fornece resultado completo do teste 12.
**Regra do dono: todo comando e todo teste deve ser escrito por inteiro na resposta do chat, nunca só apontado para este arquivo.** Cada item: o comando (fish), o que deve acontecer, como desfazer. Todo roteiro carrega o plugin e o núcleo do repositório primeiro.

**CL. Ciclo de vida do núcleo e cascata do modo Floating (OpenSpec `fix-core-lifecycle`; 01/11/2026; não executado)**
Confere a cascata do modo Floating (falha ao vivo de 28/10), a procedência do modo ao minimizar e em "mostrar área de trabalho", as origens de "mostrar área de trabalho" depois de recarregar o núcleo e `zone` em tela cheia. Mexe em `workspace-modes.json` (guardado antes). Use um workspace vazio, por exemplo o 5. Comandos com caminho completo; rode um por vez. Antes de tudo, este comando não deve listar nenhum `kwinish`:
```bash
hyprctl plugin list
```
```bash
bash /home/lhen/Projects/ryoku-work/kwinish-port-0-57/tools/live-backup.sh save /home/lhen/.local/state/kwinish-backup/teste-cl /home/lhen/.config/kwinish/workspace-modes.json /home/lhen/.config/kwinish/workspace-modes.json.bak /home/lhen/.config/kwinish/workspace-columns.json /home/lhen/.config/kwinish/workspace-columns.json.bak
mkdir -p /tmp/kwinish-cl
make -C /home/lhen/Projects/ryoku-work/kwinish-port-0-57/plugin TARGET=/tmp/kwinish-cl/kwinish-bars.so
hyprctl plugin load /tmp/kwinish-cl/kwinish-bars.so
hyprctl eval "package.path='/home/lhen/Projects/ryoku-work/kwinish-port-0-57/core/?.lua;'..package.path; for _,m in ipairs({'kwinish','geometry','layouts','assist','rules','modes','binds','switcher','settings','kwjson'}) do package.loaded[m]=nil end; require('kwinish')"
```
Deve acontecer: `live-backup: 4 caminho(s) guardado(s)`; `make` sem `error`; `plugin load` sem erro. Vá ao workspace 5 (SUPER+5) e ponha-o no modo floating:
```bash
hyprctl eval "require('kwinish').set_workspace_mode(hl.get_active_workspace().id, 'floating')"
```
1. **Cascata:** abra três janelas, uma de cada vez: `kitty --class kwcl &` (três vezes, esperando cada uma aparecer). Todas nascem flutuantes, e cada nova fica deslocada da anterior, em escada, sem passar pelo mosaico.
Os atalhos do Kwinish não estão na sua config (só o núcleo foi carregado), então os passos usam comandos. Para trazer de volta um `kwcl` minimizado ao workspace 5:
```bash
hyprctl eval "require('kwinish').window_action('$(hyprctl clients -j | jq -r 'first(.[] | select(.class == "kwcl" and .workspace.name == "special:minimized")) | .address')', 'workspace:5')"
```
2. **Minimizar:** minimize um `kwcl` pelo botão da barra e traga de volta com o comando acima. Depois ponha o workspace em tiling:
```bash
hyprctl eval "require('kwinish').set_workspace_mode(hl.get_active_workspace().id, 'tiling')"
```
   Os três `kwcl` vão para o mosaico, inclusive o que foi minimizado. Volte ao floating (comando de floating acima).
3. **Minimizado quando o modo acaba:** minimize um `kwcl`, rode o comando de tiling e só então traga a janela de volta com o comando acima: ela volta já no mosaico. Volte ao floating.
4. **"Mostrar área de trabalho" com recarga:** rode `hyprctl eval "require('kwinish').show_desktop()"` (as janelas somem). Recarregue o núcleo com o comando de carga do núcleo acima e rode o mesmo `show_desktop` de novo: as três voltam ao workspace 5.
5. **Zona em tela cheia:** dentro de um `kwcl` (ele fica focado), rode `hyprctl eval "require('kwinish').window_action(hl.get_active_window().address, 'fullscreen')"` para pô-lo em tela cheia e, no mesmo terminal, `hyprctl eval "require('kwinish').window_action(hl.get_active_window().address, 'zone:1')"`: a janela continua em tela cheia, sem mudar de tamanho. Rode o comando de `fullscreen` de novo para sair.
Desfazer (fecha as janelas de teste, devolve os arquivos, tira o plugin e o núcleo):
```bash
kill $(hyprctl clients -j | jq '.[] | select(.class == "kwcl") | .pid') 2>/dev/null
bash /home/lhen/Projects/ryoku-work/kwinish-port-0-57/tools/live-backup.sh restore /home/lhen/.local/state/kwinish-backup/teste-cl
hyprctl plugin unload /tmp/kwinish-cl/kwinish-bars.so
hyprctl reload
```
Se a pasta `/home/lhen/.config/kwinish` sobrar vazia, `rmdir /home/lhen/.config/kwinish` a remove. *Se a sessão travar:* Ctrl+Alt+F3, entre com seu usuário e rode `hyprctl -i 0 plugin unload /tmp/kwinish-cl/kwinish-bars.so`.

**II. Entrada e comunicação: barra coberta, símbolos por escala, Snap Assist (OpenSpec `fix-input-ipc`; 01/11/2026; não executado)**
Só plugin e núcleo. Os passos 2 e 3 gravam o layout ativo em `/home/lhen/.config/kwinish/layout`, guardado antes e devolvido no desfazer. Antes de tudo, este comando não deve listar nenhum `kwinish`:
```bash
hyprctl plugin list
```
```bash
bash /home/lhen/Projects/ryoku-work/kwinish-port-0-57/tools/live-backup.sh save /home/lhen/.local/state/kwinish-backup/teste-ii /home/lhen/.config/kwinish/layout
mkdir -p /tmp/kwinish-ii
make -C /home/lhen/Projects/ryoku-work/kwinish-port-0-57/plugin TARGET=/tmp/kwinish-ii/kwinish-bars.so
hyprctl plugin load /tmp/kwinish-ii/kwinish-bars.so
hyprctl eval "package.path='/home/lhen/Projects/ryoku-work/kwinish-port-0-57/core/?.lua;'..package.path; for _,m in ipairs({'kwinish','geometry','layouts','assist','rules','modes','binds','switcher','settings','kwjson'}) do package.loaded[m]=nil end; require('kwinish')"
kitty --class kwii & kitty --class kwii &
```
Deixe os dois `kwii` flutuantes (SUPER+V em cada um).
1. **Barra coberta:** ponha um `kwii` parcialmente por cima da barra do outro. Clique no de trás (para ele ficar focado) e depois clique na parte da barra dele que está coberta pelo da frente: quem recebe o clique é a janela da frente, e a barra de trás não reage (não foca, não abre menu, não arrasta). Gire a roda do mouse no mesmo ponto: a janela de trás não sobe nem desce.
2. **Snap Assist com layout trocado:** escolha o layout `thirds` com `hyprctl eval "require('kwinish').set_layout('thirds')"`. Rode o comando abaixo e, em até 5 s, arraste um `kwii` pelo título com Shift segurado até a zona da esquerda e solte; o overlay do Assist abre na zona do meio com o outro `kwii`:
```bash
(sleep 5; hyprctl eval "require('kwinish').set_layout('halves')") &
```
   Espere passar os 5 s (o layout mudou por trás) e clique no `kwii` do overlay: ele **não** é encaixado.
3. **Snap Assist normal:** volte ao `thirds` (comando do passo 2), solte de novo um `kwii` na zona da esquerda com Shift e clique no outro no overlay sem mudar nada: ele vai para a zona do meio.
4. **Escala (só se houver dois monitores com escalas diferentes):** leve um `kwii` de um monitor ao outro: os símbolos de minimizar, maximizar e fechar continuam nítidos e no tamanho certo nos dois.
5. **Log:** este comando não deve mostrar `Q1 violated` nem `error`:
```bash
grep -i kwinish "$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/hyprland.log" | tail -n 30
```
Desfazer:
```bash
kill $(hyprctl clients -j | jq '.[] | select(.class == "kwii") | .pid') 2>/dev/null
bash /home/lhen/Projects/ryoku-work/kwinish-port-0-57/tools/live-backup.sh restore /home/lhen/.local/state/kwinish-backup/teste-ii
hyprctl plugin unload /tmp/kwinish-ii/kwinish-bars.so
hyprctl reload
```
Se a pasta `/home/lhen/.config/kwinish` sobrar vazia, `rmdir /home/lhen/.config/kwinish` a remove (só remove pasta vazia). *Se a sessão travar:* Ctrl+Alt+F3, entre com seu usuário e rode `hyprctl -i 0 plugin unload /tmp/kwinish-ii/kwinish-bars.so`.

**RP. A barra desenhada com a foto do quadro (`SWindowRenderPresentation`; 01/11/2026; executado pelo dono: passou)**
Correção de render: a barra passa a usar os deslocamentos de workspace e de flutuação que o compositor entrega em cada quadro, como a borda nativa, em vez de ler o estado vivo da janela. No modo normal os dois são iguais; a diferença aparece em animações e cenas de workspace. Só plugin e núcleo; nenhum arquivo seu muda. Comandos com caminho completo; rode um por vez. Antes de tudo, este comando não deve listar nenhum `kwinish` (se listar, descarregue o caminho mostrado com `hyprctl plugin unload <caminho>`):
```bash
hyprctl plugin list
```
```bash
mkdir -p /tmp/kwinish-rp
make -C /home/lhen/Projects/ryoku-work/kwinish-port-0-57/plugin TARGET=/tmp/kwinish-rp/kwinish-bars.so
hyprctl plugin load /tmp/kwinish-rp/kwinish-bars.so
hyprctl eval "package.path='/home/lhen/Projects/ryoku-work/kwinish-port-0-57/core/?.lua;'..package.path; for _,m in ipairs({'kwinish','geometry','layouts','assist','rules','modes','binds','switcher','settings','kwjson'}) do package.loaded[m]=nil end; require('kwinish')"
kitty --class kwrp &
```
Deve acontecer: `make` sem `error`; `plugin load` sem erro; uma janela `kwrp` com barra. Em todos os passos, a barra tem de acompanhar a janela quadro a quadro, colada nela, sem atraso, salto ou sobra.
1. **Troca de workspace:** com o `kwrp` em tiling no workspace 1, vá ao 2 e volte ao 1 (SUPER+2, SUPER+1). Durante o deslize, a barra anda junto com a janela.
2. **Flutuante:** deixe o `kwrp` flutuante (SUPER+V) e repita o passo 1.
3. **Fixada:** clique no botão de fixar do `kwrp` flutuante e troque de workspace: a janela e a barra ficam paradas na tela enquanto o resto desliza. Clique de novo para desfixar.
4. **Mover para outro workspace:** com o `kwrp` focado, SUPER+SHIFT+2 e depois vá ao 2: a barra chega junto com a janela. Volte com SUPER+SHIFT+1.
5. **Abrir e fechar:** abra outro `kitty --class kwrp &` e feche com o botão de fechar da barra: a barra aparece e some junto com a janela.
6. **Arrasto e encaixe (regressão):** arraste o `kwrp` flutuante pelo título até a borda esquerda e solte: encaixa na metade esquerda, com a barra no lugar.
7. **Log:** este comando não deve mostrar `Q1 violated` nem `error`:
```bash
grep -i kwinish "$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/hyprland.log" | tail -n 30
```
Desfazer (tira o plugin e o núcleo e fecha as janelas de teste):
```bash
kill $(hyprctl clients -j | jq '.[] | select(.class == "kwrp") | .pid') 2>/dev/null
hyprctl plugin unload /tmp/kwinish-rp/kwinish-bars.so
hyprctl reload
```
*Se a sessão travar:* aperte Ctrl+Alt+F3, entre com seu usuário e rode `hyprctl -i 0 plugin unload /tmp/kwinish-rp/kwinish-bars.so`.

**GS. Porta à entrada do arrasto (OpenSpec `guard-titlebar-drag-start`; 01/11/2026; executado pelo dono: passou, com duas observações)**
Corrige três achados da revisão da alpha-1: a saída de encaixe e a de maximizada não mexem mais na janela durante o arrasto de outro componente; o arrasto adiado só começa na janela da própria barra; e todo `grab` que não começa vira erro Lua. Só plugin e núcleo; o `kwinish-ui` não é necessário. Caminho novo, para não sobrescrever um `.so` carregado. Comandos com caminho completo, sem variáveis; rode um por vez. Antes de tudo, este comando não deve listar nenhum `kwinish` (se listar, descarregue o caminho mostrado com `hyprctl plugin unload <caminho>`):
```bash
hyprctl plugin list
```
```bash
mkdir -p /tmp/kwinish-gs
make -C ~/Projects/ryoku-work/kwinish-port-0-57/plugin TARGET=/tmp/kwinish-gs/kwinish-bars.so
hyprctl plugin load /tmp/kwinish-gs/kwinish-bars.so
hyprctl eval "package.path='/home/lhen/Projects/ryoku-work/kwinish-port-0-57/core/?.lua;'..package.path; for _,m in ipairs({'kwinish','geometry','layouts','assist','rules','modes','binds','switcher','settings','kwjson'}) do package.loaded[m]=nil end; require('kwinish')"
kitty --class kwt8 & kitty --class kwt8 &
```
Deve acontecer: `make` sem `error`; `plugin load` sem erro; duas janelas `kwt8` com barra.
1. **Janela vizinha (achado 2):** deixe os dois `kwt8` em tiling, lado a lado ou um sobre o outro. Aperte o título de um e dê um puxão **rápido** para dentro do outro, soltando lá. Esperado: o **outro** nunca é arrastado. Ou a janela que você apertou se move, ou nada acontece (vale como clique). Repita umas cinco vezes, com velocidades diferentes.
2. **Regressão do arrasto:** deixe um `kwt8` flutuante (SUPER+V). Arraste pelo título até a borda esquerda e solte: ele ocupa a metade esquerda. Arraste de novo: ele volta ao tamanho de antes sob o ponteiro (animação de 140 ms) e segue o arrasto. Shift no meio do arrasto mostra as zonas e não larga a janela.
3. **Maximizada:** maximize um `kwt8` (duplo clique no título) e arraste pelo título: ele sai da maximização sob o ponteiro e segue o arrasto.
4. **`grab` que não começa (achado 3):** este comando deve terminar com um erro que contém `grab: no such window`:
```bash
hyprctl eval "hl.plugin.kwinish.grab('move','0x1')"
```
5. **Log:** este comando não deve mostrar `Q1 violated` nem `error`:
```bash
grep -i kwinish "$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/hyprland.log" | tail -n 30
```
*Sem passo próprio:* o achado 1 (outro arrasto começando dentro dos 140 ms da saída de encaixe) é difícil de provocar à mão. Ficou coberto só pelos testes sem compositor e pela ordem no código.
Desfazer (tira o plugin e o núcleo e fecha as janelas de teste):
```bash
kill $(hyprctl clients -j | jq '.[] | select(.class == "kwt8") | .pid') 2>/dev/null
hyprctl plugin unload /tmp/kwinish-gs/kwinish-bars.so
hyprctl reload
```
*Se a sessão travar:* aperte Ctrl+Alt+F3, entre com seu usuário e rode `hyprctl -i 0 plugin unload /tmp/kwinish-gs/kwinish-bars.so`.
*O que me mandar:* ok ou o que viu em cada passo (1–5).
*Resultado (dono, 01/11/2026):* passos 2, 4 e 5 passaram; no 1, a janela vizinha nunca foi arrastada (passou). Duas observações:
- passo 1: ao começar o arrasto de uma janela em tiling, a janela pula para ficar com o centro sob o cursor. É do Hyprland 0.57, não da barra: o próprio host torna a janela flutuante e a centra no cursor (`DragController.cpp:87–158`, opção `binds:drag_center_window`, padrão `true`, não definida na config do dono). Vale também para SUPER+arrastar. Mantido;
- passo 3: ao sair da maximização, o cursor ficou fora da barra. **Bug da barra**, corrigido no roteiro **MX** abaixo.

**MX. Sair da maximização com o cursor na barra (01/11/2026; executado pelo dono: passou)**
A barra lia o tamanho da janela no meio da animação de sair da maximização (`GEOMETRIC_CURRENT`, ainda quase o tamanho maximizado) e a colocava deslocada. Agora lê o tamanho final (`GEOMETRIC_GOAL`). Comandos com caminho completo; rode um por vez. Antes de tudo, este comando não deve listar nenhum `kwinish` (se listar, descarregue o caminho mostrado com `hyprctl plugin unload <caminho>`):
```bash
hyprctl plugin list
```
```bash
mkdir -p /tmp/kwinish-mx
make -C ~/Projects/ryoku-work/kwinish-port-0-57/plugin TARGET=/tmp/kwinish-mx/kwinish-bars.so
hyprctl plugin load /tmp/kwinish-mx/kwinish-bars.so
hyprctl eval "package.path='/home/lhen/Projects/ryoku-work/kwinish-port-0-57/core/?.lua;'..package.path; for _,m in ipairs({'kwinish','geometry','layouts','assist','rules','modes','binds','switcher','settings','kwjson'}) do package.loaded[m]=nil end; require('kwinish')"
kitty --class kwt8 &
```
Deixe o `kwt8` **flutuante** (SUPER+V). Numa janela em tiling, quem posiciona é o Hyprland (centra no cursor, como no GS passo 1).
1. Duplo clique no título para maximizar. Aperte o título perto da **direita** e arraste: a janela volta ao tamanho de antes com o cursor na barra, também perto da direita, e segue o arrasto.
2. Repita apertando perto da **esquerda** e perto do **meio**: o cursor fica na barra, na mesma fração.
3. Log: este comando não deve mostrar `Q1 violated` nem `error`:
```bash
grep -i kwinish "$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/hyprland.log" | tail -n 30
```
Desfazer:
```bash
kill $(hyprctl clients -j | jq '.[] | select(.class == "kwt8") | .pid') 2>/dev/null
hyprctl plugin unload /tmp/kwinish-mx/kwinish-bars.so
hyprctl reload
```
*O que me mandar:* ok ou o que viu em cada passo (1–3).
*Resultado (dono, 01/11/2026):* "o teste passou" (passos 1–3, relato geral; `hyprctl plugin list` antes mostrou nenhum plugin carregado).

**A1. Porta 0.57, alpha-1: arrasto pela barra de volta (01/11/2026; executado pelo dono: passou; C11 feito no A1c)**
OpenSpec `enable-titlebar-drag-alpha1`, política `alt5-rc`: o plugin nunca pede o fim de um arrasto, e o Hyprland o encerra na soltura. Mesmo ambiente da T8 (bash, um só terminal, Hyprland 0.57.0, um monitor DP-1). Os itens marcados *2 monitores* ficam para quando o HDMI-A-1 estiver ligado.

*Fase A: preparar (nada muda no compositor).*
```bash
cd ~/Projects/ryoku-work/kwinish-port-0-57 && git log --oneline -1
R=$PWD; T=/tmp/kwinish-a1; mkdir -p "$T"
make -C plugin TARGET="$T/kwinish-bars.so"
cargo build --locked --manifest-path ui/Cargo.toml
bash tools/live-backup.sh save ~/.local/state/kwinish-backup/a1 ~/.local/bin/kwinish-ui
ln -s "$R/ui/target/debug/kwinish-ui" ~/.local/bin/kwinish-ui
CORE="package.path='$R/core/?.lua;'..package.path; for _,m in ipairs({'kwinish','geometry','layouts','assist','rules','modes','binds','switcher','settings','kwjson'}) do package.loaded[m]=nil end; require('kwinish')"
```
Deve acontecer: o `make` termina sem `error` e cria `/tmp/kwinish-a1/kwinish-bars.so`; o `cargo` termina com `Finished`; o `live-backup` diz `1 caminho(s) guardado(s)`.

*Fase B: Hyprland aninhado (se travar, cai só a janela). Não passe para a C se B falhar.*
```bash
cat > "$T/nested.lua" <<'EOF'
hl.monitor({ output = "", mode = "preferred", position = "auto", scale = "1" })
hl.bind("ALT + mouse:272", hl.dsp.window.drag(), { mouse = true })
hl.bind("ALT + Return", hl.dsp.exec_cmd("kitty --class kwt8"))
EOF
Hyprland -c "$T/nested.lua" > "$T/nested.log" 2>&1 & hp=$!
sleep 3; nest=$(hyprctl instances -j | jq -r --argjson p "$hp" '.[] | select(.pid == $p) | .instance'); echo "instância: $nest"
hyprctl -i "$nest" plugin load "$T/kwinish-bars.so"
hyprctl -i "$nest" eval "$CORE"
hyprctl -i "$nest" dispatch 'hl.dsp.exec_cmd("kitty --class kwt8")'
```
B1. O `kitty` do aninhado tem barra. Arraste-o pelo título: a janela acompanha o ponteiro e para quando você solta.
B2. Descarregar **durante** um arrasto: rode a linha abaixo e, em até 3 s, aperte o título e continue arrastando até depois do unload. Esperado: nada trava, a barra some, e a janela para ao soltar o botão.
```bash
sleep 4; hyprctl -i "$nest" plugin unload "$T/kwinish-bars.so"; sleep 3; hyprctl -i "$nest" plugin load "$T/kwinish-bars.so"
```
B3. `grep -i kwinish "$T/nested.log" | tail -n 30` não mostra `Q1 violated` nem `error`. Encerre: `hyprctl -i "$nest" dispatch 'hl.dsp.exit()'`. Se o aninhado cair, guarde `tail -n 80 "$T/nested.log"` e pare aqui.

*Fase C: sessão real (só se B passou). Carrega o plugin e depois o núcleo do repositório.*
```bash
hyprctl plugin load "$T/kwinish-bars.so"
hyprctl eval "$CORE"
kitty --class kwt8 & kitty --class kwt8 &
```
Deixe os dois `kwt8` flutuantes (SUPER+V).
C1. **Arrastar:** aperte o título e arraste. A janela acompanha e para ao soltar. Um clique simples no título só foca e eleva; o duplo clique maximiza.
C2. **Encaixe na borda:** arraste até a borda esquerda. A prévia aparece; ao soltar, a janela ocupa a metade esquerda. Faça o mesmo na direita, e no topo para maximizar.
C3. **Shift para zona:** arraste segurando Shift. Aparecem as zonas do layout, com a zona sob o ponteiro preenchida; ao soltar, a janela vai para ela, e o Snap Assist pode abrir para a próxima zona vazia. Soltar e apertar Shift no meio do arrasto não pode deixar a janela parada nem largar o arrasto.
C4. **Sair do encaixe:** arraste pelo título a janela encaixada do C2. Ela volta ao tamanho de antes sob o ponteiro (desde o A1b, com a animação de 140 ms antes de o arrasto começar).
C5. **Maximizada e tiling:** maximize um `kwt8` e arraste pelo título: ele sai da maximização sob o ponteiro. Em tiling (SUPER+V), o arrasto só começa depois de uns pixels de movimento.
C6. **Menu Mover/Redimensionar:** clique direito no título, escolha Mover e mova o ponteiro: a janela acompanha, e o próximo clique termina. Leve o ponteiro sobre o título do **outro** `kwt8` antes desse clique: o clique termina o Mover e não abre menu nem inicia arrasto na outra barra. Repita com Redimensionar.
C7. **Arrasto de outro componente:** com SUPER + botão esquerdo, arraste um `kwt8` passando sobre a barra do outro e solte. A outra barra não reage, e o arrasto termina normal.
C8. **Débito aceito (`floatWindow` do Lua):** rode a linha abaixo e, em até 3 s, comece um arrasto com SUPER + botão esquerdo num `kwt8` e continue segurando. Esperado: o arrasto termina sozinho quando o comando roda, e a janela troca entre flutuante e tiling. É o débito registrado, não regressão.
```bash
sleep 4; hyprctl eval "require('kwinish').window_action('$(hyprctl activewindow -j | jq -r .address)','float')"
```
C9. **Unload durante arrasto:** rode a linha abaixo e, em até 3 s, arraste um `kwt8` pelo título e continue segurando. Esperado: nada trava, as barras somem, e a janela para ao soltar. Depois as barras voltam.
```bash
sleep 4; hyprctl plugin unload /tmp/kwinish-a1/kwinish-bars.so; sleep 3; hyprctl plugin load /tmp/kwinish-a1/kwinish-bars.so
```
C10. **Log:** `grep -i kwinish "$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/hyprland.log" | tail -n 30` não mostra `Q1 violated` nem `error`.
C11. *2 monitores:* ver o roteiro **A1c** abaixo.
*Custo aceito, sem passo próprio (C1/C2 do contrato):* se outro programa capturar a entrada bem na hora de soltar, o arrasto continua até a próxima tecla ou clique. Se isso acontecer no uso, anote; é o gatilho para trocar à paridade 0.56.

*Desfazer (tira o plugin e o núcleo, fecha as janelas de teste e o menu e apaga o link do `kwinish-ui`; nenhum outro arquivo seu é tocado).*
```bash
pkill -x kwinish-ui
kill $(hyprctl clients -j | jq '.[] | select(.class == "kwt8") | .pid') 2>/dev/null
hyprctl plugin unload /tmp/kwinish-a1/kwinish-bars.so
hyprctl reload
bash ~/Projects/ryoku-work/kwinish-port-0-57/tools/live-backup.sh restore ~/.local/state/kwinish-backup/a1
```
*Se a sessão real travar:* aperte Ctrl+Alt+F3, entre com seu usuário e rode `hyprctl -i 0 plugin unload /tmp/kwinish-a1/kwinish-bars.so`. Se nem assim responder, encerre a sessão.
*O que me mandar:* para cada item (B1–B3, C1–C11), ok ou o que viu. Se algo falhar, mande também as saídas dos `grep` de log.
*Resultado (dono, 01/11/2026):* B1–B3 e C1–C8, C10 passaram (relato geral). **C9 não chegou a testar o plugin:** rodado num terminal sem a variável `T`, virou `hyprctl plugin unload /kwinish-bars.so` ("plugin not loaded") e o `load` falhou ("cannot open shared object file"); o plugin da alpha-1 continuou carregado. O comando do C9 agora usa o caminho completo; **refeito pelo dono em 01/11: passou** (unload durante arrasto na sessão real). O unload durante arrasto passou no aninhado (B2). **C11 não feito** (um monitor). Desfazer ainda não executado em 01/11 (plugin carregado, link do `kwinish-ui` e backup `a1` presentes, conferido só com consultas).

**A1c. Dois monitores: divisa e soltar no outro monitor (C11 do A1; 01/11/2026; executado pelo dono: passou)**
Pré-requisito: DP-1 e HDMI-A-1 ligados **lado a lado** (a resistência só vale numa divisa vertical). Em 01/11 só o DP-1 estava ligado. Sem `~/.config/kwinish`, o núcleo usa o layout `grid` (2×2). Comandos com caminho completo, sem variáveis; rode um por vez.
```bash
hyprctl monitors -j | jq -r '.[] | "\(.name) x=\(.x) y=\(.y) w=\(.width) h=\(.height) scale=\(.scale)"'
```
Deve mostrar os dois, com o `x` de um igual ao `x + w` do outro (escala 1; com outra escala, compare as larguras lógicas).
```bash
mkdir -p /tmp/kwinish-a1c
make -C ~/Projects/ryoku-work/kwinish-port-0-57/plugin TARGET=/tmp/kwinish-a1c/kwinish-bars.so
cargo build --locked --manifest-path ~/Projects/ryoku-work/kwinish-port-0-57/ui/Cargo.toml
bash ~/Projects/ryoku-work/kwinish-port-0-57/tools/live-backup.sh save ~/.local/state/kwinish-backup/a1c ~/.local/bin/kwinish-ui
ln -s ~/Projects/ryoku-work/kwinish-port-0-57/ui/target/debug/kwinish-ui ~/.local/bin/kwinish-ui
hyprctl plugin load /tmp/kwinish-a1c/kwinish-bars.so
hyprctl eval "package.path='/home/lhen/Projects/ryoku-work/kwinish-port-0-57/core/?.lua;'..package.path; for _,m in ipairs({'kwinish','geometry','layouts','assist','rules','modes','binds','switcher','settings','kwjson'}) do package.loaded[m]=nil end; require('kwinish')"
kitty --class kwt8 & kitty --class kwt8 & kitty --class kwt8 &
```
Deve acontecer: `make` sem `error`; `cargo` com `Finished`; `live-backup` diz `1 caminho(s) guardado(s)`; o `plugin load` sem erro. O `kwinish-ui` no PATH é o que abre o Snap Assist: sem ele, o "Assist não abre" do passo 4 não provaria nada. Deixe os três `kwt8` flutuantes (SUPER+V) no DP-1.
1. **Divisa:** arraste um `kwt8` pelo título em direção ao HDMI-A-1. O ponteiro para na borda e aparece a prévia da borda; continuando a empurrar (cerca de 200 px de movimento), ele atravessa. Afastar-se da divisa e voltar recomeça a resistência.
2. **Borda no outro monitor:** arraste um `kwt8` do DP-1 até a borda esquerda ou direita do HDMI-A-1 e solte, sem Shift. Ele ocupa a metade **no HDMI-A-1**, não volta ao DP-1.
3. **Controle do Assist (mesmo monitor):** com outro `kwt8` flutuante solto no DP-1, arraste um terceiro com Shift e solte numa zona do **DP-1**. Ele ocupa a zona, e o Snap Assist abre sobre a próxima zona vazia. Feche o Assist com Esc.
4. **Shift no outro monitor:** arraste um `kwt8` do DP-1 com Shift e solte numa zona do **HDMI-A-1**. Ele ocupa a zona no HDMI-A-1, e o Snap Assist **não** abre.
5. **Log:** este comando não deve mostrar `Q1 violated` nem `error`:
```bash
grep -i kwinish "$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/hyprland.log" | tail -n 30
```
Desfazer (tira o plugin e o núcleo, fecha as janelas de teste e o Assist e apaga o link do `kwinish-ui`):
```bash
pkill -x kwinish-ui
kill $(hyprctl clients -j | jq '.[] | select(.class == "kwt8") | .pid') 2>/dev/null
hyprctl plugin unload /tmp/kwinish-a1c/kwinish-bars.so
hyprctl reload
bash ~/Projects/ryoku-work/kwinish-port-0-57/tools/live-backup.sh restore ~/.local/state/kwinish-backup/a1c
```
*O que me mandar:* ok ou o que viu em cada passo (1–5).
*Resultado (dono, 01/11/2026):* "teste passou" (passos 1–5, relato geral, com o HDMI-A-1 ligado).

**A1b. Animação ao sair do encaixe (01/11/2026; executado pelo dono: passou)**
Desde o A1, a janela encaixada volta ao tamanho de antes numa animação de 140 ms, **antes** de o arrasto começar (alt5-rc: nada é pausado nem encerrado). Só plugin e núcleo; o `kwinish-ui` não é necessário. Caminho novo, para não sobrescrever um `.so` carregado. Um só terminal.
```bash
cd ~/Projects/ryoku-work/kwinish-port-0-57 && git log --oneline -1
R=$PWD; T=/tmp/kwinish-a1b; mkdir -p "$T"
make -C plugin TARGET="$T/kwinish-bars.so"
CORE="package.path='$R/core/?.lua;'..package.path; for _,m in ipairs({'kwinish','geometry','layouts','assist','rules','modes','binds','switcher','settings','kwjson'}) do package.loaded[m]=nil end; require('kwinish')"
hyprctl plugin load "$T/kwinish-bars.so"
hyprctl eval "$CORE"
kitty --class kwt8 &
```
Deixe o `kwt8` flutuante (SUPER+V).
1. Arraste-o pelo título até a borda esquerda e solte: ele ocupa a metade esquerda.
2. Arraste de novo pelo título, devagar: depois de uns pixels, ele cresce suavemente (cerca de 140 ms) até o tamanho de antes, sob o ponteiro, e então segue o arrasto normalmente.
3. Encaixe de novo e repita com um puxão curto, soltando logo: ele fica no tamanho de antes, onde você soltou, e não continua preso ao ponteiro.
4. Apertar Shift durante o arrasto do passo 2 não deixa a janela parada.
Desfazer:
```bash
kill $(hyprctl clients -j | jq '.[] | select(.class == "kwt8") | .pid') 2>/dev/null
hyprctl plugin unload /tmp/kwinish-a1b/kwinish-bars.so
hyprctl reload
```
*O que me mandar:* ok ou o que viu em cada passo (1–4).
*Resultado (dono, 01/11/2026):* "o teste passou" (passos 1–4, relato geral). O dono carregou plugin e núcleo pelos comandos com caminho completo, sem variáveis.

**T8. Porta 0.57, alpha-0: primeiro teste ao vivo (01/11/2026; executado pelo dono: tudo passou)**
Estado conferido em 01/11, só com consultas: Hyprland 0.57.0 (`0aa6745`) em execução, nenhum plugin carregado, pacotes `kwinish-*` e `/usr/share/kwinish` ausentes, sem `~/.config/kwinish`, `kwinish-ui` fora do PATH. **Fish não está instalado** (shell de login: bash): por decisão do dono em 01/11, este roteiro é em **bash**. Os roteiros 2–12 abaixo citam `~/Downloads/ryoku-work/kwinish`, o plugin em `/usr/lib/hyprland/plugins` e fish: nada disso existe mais. O repositório está em `~/Projects/ryoku-work/`.
Use **um só terminal** do começo ao fim (as variáveis `R`, `T`, `CORE` e `nest` valem só nele). As janelas de teste são `kitty --class kwt8`, para não confundir com o seu terminal.

*Fase A: preparar (nada muda no compositor).*
```bash
cd ~/Projects/ryoku-work/kwinish-port-0-57 && git log --oneline -1
R=$PWD; T=/tmp/kwinish-t8; mkdir -p "$T"
make -C plugin TARGET="$T/kwinish-bars.so"
cargo build --locked --manifest-path ui/Cargo.toml
bash tools/live-backup.sh save ~/.local/state/kwinish-backup/t8 ~/.local/bin/kwinish-ui
ln -s "$R/ui/target/debug/kwinish-ui" ~/.local/bin/kwinish-ui
CORE="package.path='$R/core/?.lua;'..package.path; for _,m in ipairs({'kwinish','geometry','layouts','assist','rules','modes','binds','switcher','settings','kwjson'}) do package.loaded[m]=nil end; require('kwinish')"
```
Deve acontecer: o `git log` mostra o commit atual da branch `port-kwinish-bars-to-hyprland-0-57`; o `make` termina sem `error` e cria `/tmp/kwinish-t8/kwinish-bars.so`; o `cargo` termina com `Finished`; o `live-backup` diz `1 caminho(s) guardado(s)`. O `.so` fica fora do repositório, para nunca sobrescrever um `.so` carregado.

*Fase B: Hyprland aninhado (uma janela; se o plugin derrubar alguma coisa, cai só ela). Não passe para a fase C se B falhar.*
```bash
cat > "$T/nested.lua" <<'EOF'
hl.monitor({ output = "", mode = "preferred", position = "auto", scale = "1" })
hl.bind("ALT + mouse:272", hl.dsp.window.drag(), { mouse = true })
hl.bind("ALT + Return", hl.dsp.exec_cmd("kitty --class kwt8"))
EOF
Hyprland -c "$T/nested.lua" > "$T/nested.log" 2>&1 & hp=$!
sleep 3; nest=$(hyprctl instances -j | jq -r --argjson p "$hp" '.[] | select(.pid == $p) | .instance'); echo "instância: $nest"
hyprctl -i "$nest" plugin load "$T/kwinish-bars.so"
hyprctl -i "$nest" eval "$CORE"
hyprctl -i "$nest" dispatch 'hl.dsp.exec_cmd("kitty --class kwt8")'
```
B1. Abre uma janela com o Hyprland aninhado; `instância:` mostra uma assinatura (não vazia); o `plugin load` não dá erro; o `kitty` lá dentro tem a barra do Kwinish (ícone, título, botões de fixar, minimizar, maximizar e fechar).
B2. Descarregar e recarregar 3 vezes: as barras somem e voltam a cada volta, e a janela aninhada continua viva.
```bash
for i in 1 2 3; do hyprctl -i "$nest" plugin unload "$T/kwinish-bars.so"; sleep 1; hyprctl -i "$nest" plugin load "$T/kwinish-bars.so"; sleep 1; done
```
B3. Descarregar com a dica na tela: rode a linha abaixo e, em até 3 s, pare o ponteiro sobre um botão da barra no aninhado até a dica aparecer. Esperado: barra e dica somem, nada trava; depois recarregue.
```bash
sleep 4; hyprctl -i "$nest" plugin unload "$T/kwinish-bars.so"; sleep 1; hyprctl -i "$nest" plugin load "$T/kwinish-bars.so"
```
B4. Log: `grep -i kwinish "$T/nested.log" | tail -n 30` não mostra `Q1 violated` nem `error`.
B5. Encerrar o aninhado: `hyprctl -i "$nest" dispatch 'hl.dsp.exit()'`. Se ele tiver caído antes, guarde `tail -n 80 "$T/nested.log"` e o relatório em `~/.cache/hyprland/` (se existir) e pare aqui.

*Fase C: sessão real (só se B passou). Carrega o plugin e depois o núcleo do repositório.*
```bash
hyprctl plugin load "$T/kwinish-bars.so"
hyprctl eval "$CORE"
kitty --class kwt8 & kitty --class kwt8 &
```
C1. Desenho (sintomas visíveis de D9–D11; o estado interno de GL não dá para ver a olho). Ponha um `kwt8` flutuante (SUPER+V).
1. Barra com ícone, título e 4 botões; cantos de cima arredondados junto com a janela (`rounding = 10`), sem quadradinhos escuros nas pontas; borda contínua.
2. Fora da barra nada muda: janelas vizinhas, a barra do shell e o papel de parede perto dos cantos não ganham recortes, buracos nem faixas (stencil vazando, D9).
3. Blur (D11): `hyprctl eval "hl.config({plugin={kwinish={bar_blur=true, bar_color='rgba(26262e80)'}}})"`. A barra fica translúcida com o fundo desfocado, sem vazar pelos cantos arredondados. Volte: `hyprctl eval "hl.config({plugin={kwinish={bar_blur=false, bar_color='rgba(26262ef7)'}}})"`.
4. Animação de workspace: com um `kwt8` no workspace 1, SUPER+2 e SUPER+1. A barra desliza junto com a janela, sem ficar parada nem deslocada (risco conhecido da T5: a barra ignora o parâmetro de apresentação novo). Anote o que vir no abrir e no fechar de uma janela (`popin`), porque a barra pode não acompanhar a escala.
5. SUPER+V de volta para tiling: a barra continua desenhada.
C2. Entrada (alpha-0: barra sem arrasto).
1. Clique no título: foca e eleva a janela flutuante. Apertar no título e arrastar: a janela **não** se move.
2. SUPER + botão esquerdo arrastando: move (bind nativo).
3. Duplo clique no título: maximiza; outro duplo clique: restaura.
4. Fixar: clique, depois SUPER+2: a janela continua visível; SUPER+1 e clique de novo para desfixar.
5. Minimizar num `kwt8`: a janela some (vai para `special:minimized`). Fechar no outro `kwt8`: fecha. Depois, clicar em qualquer outra janela funciona normal (o soltar do botão não fica preso).
6. Clique direito no título e clique no ícone: abre o menu da janela (`kwinish-ui`), **sem** Mover nem Redimensionar; Esc fecha.
C3. Dicas (em inglês, porque `LANG=en_US.UTF-8`). Pare 1 s sobre cada parte: fixar "On all workspaces" (fixada: "Only on this workspace"), minimizar "Minimize", maximizar "Maximize" (maximizada: "Restore"), fechar "Close", ícone "Window menu" (abaixo do ícone, dentro da tela). A dica fica por cima das outras janelas e some ao clicar.
C4. Descarregar e recarregar (abra um `kwt8` antes, se não houver nenhum: `kitty --class kwt8 &`).
1. Rode a linha abaixo e, em até 3 s, pare sobre um botão até a dica aparecer: barras e dica somem, nada trava. Depois as barras voltam, nas 3 voltas.
```bash
sleep 4; for i in 1 2 3; do hyprctl plugin unload "$T/kwinish-bars.so"; sleep 1; hyprctl plugin load "$T/kwinish-bars.so"; sleep 1; done
```
2. `grep -i kwinish "$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/hyprland.log" | tail -n 30` não mostra `Q1 violated` nem `error`.

*Desfazer (tira o plugin e o núcleo do repositório, fecha as janelas de teste e o menu e apaga o link do `kwinish-ui`; nenhum outro arquivo seu é tocado).*
```bash
pkill -x kwinish-ui
kill $(hyprctl clients -j | jq '.[] | select(.class == "kwt8") | .pid') 2>/dev/null
hyprctl plugin unload /tmp/kwinish-t8/kwinish-bars.so
hyprctl reload
bash ~/Projects/ryoku-work/kwinish-port-0-57/tools/live-backup.sh restore ~/.local/state/kwinish-backup/t8
```
O `reload` relê a sua config, que não carrega o núcleo: ele sai junto com os ajustes de blur e cor. Se a fase B ainda estiver aberta: `hyprctl -i "$nest" dispatch 'hl.dsp.exit()'`.
*Se a sessão real travar:* Ctrl+Alt+F3, entre com seu usuário e rode `hyprctl -i 0 plugin unload /tmp/kwinish-t8/kwinish-bars.so`. Se nem assim responder, encerre a sessão (as janelas abertas se perdem).
*O que me mandar:* para cada item (B1–B5, C1–C4), ok ou o que viu. Se algo falhar, mande também as saídas dos `grep` de log.
*Resultado (dono, 01/11/2026):* "Rodei os testes e tudo passou" (B1–B5 e C1–C4, relato geral, sem detalhe por item nem trechos de log). Conferido depois, só com consultas: nenhum plugin carregado, só a instância principal, sem `~/.local/bin/kwinish-ui`, backup `t8` restaurado e sem janelas `kwt8`. D9/D10 continuam sem instrumentação de GL (só sintomas visíveis).

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

**10. Dicas no fixar e no ícone do aplicativo (add-pin-and-icon-tooltips; roteiro reescrito para a 0.57 em 01/11/2026; dono relatou "o teste passou" em 01/11/2026)**
O roteiro de 27/10 apontava para `~/Downloads/ryoku-work/kwinish` (que não existe mais) e para o plugin instalado da 0.56; o original está no Git (`181438b`). Só plugin e núcleo; nenhum arquivo seu muda. Caminho novo, para não sobrescrever um `.so` carregado. Comandos com caminho completo; rode um por vez. Antes de tudo, este comando não deve listar nenhum `kwinish` (se listar, descarregue o caminho mostrado com `hyprctl plugin unload <caminho>`):
```bash
hyprctl plugin list
```
```bash
mkdir -p /tmp/kwinish-t10
make -C /home/lhen/Projects/ryoku-work/kwinish-port-0-57/plugin TARGET=/tmp/kwinish-t10/kwinish-bars.so
hyprctl plugin load /tmp/kwinish-t10/kwinish-bars.so
hyprctl eval "package.path='/home/lhen/Projects/ryoku-work/kwinish-port-0-57/core/?.lua;'..package.path; for _,m in ipairs({'kwinish','geometry','layouts','assist','rules','modes','binds','switcher','settings','kwjson'}) do package.loaded[m]=nil end; require('kwinish')"
kitty --class kwt10 &
```
Deve acontecer: `make` sem `error`; `plugin load` sem erro; uma janela `kwt10` com barra. Deixe-a flutuante (SUPER+V). Os textos abaixo são os do locale português; em inglês, entre parênteses.
1. Pare o ponteiro 1 s no botão de fixar (à esquerda, botões padrão `default_buttons`): aparece "Em todas as áreas de trabalho" ("On all workspaces").
2. Clique nele: a janela fica fixada e a dica some. Saia do botão e pare de novo: agora diz "Só nesta área de trabalho" ("Only on this workspace"). Clique de novo para desfixar.
3. Pare o ponteiro 1 s no ícone do aplicativo (canto esquerdo da barra): aparece "Menu da janela" ("Window menu"), abaixo do ícone e dentro da tela.
4. Clique no ícone: a dica some. O menu da janela só abre se o `kwinish-ui` estiver no PATH do Hyprland (hoje não está; o teste 11 o põe); sem ele, só confira que a dica sumiu.
5. Os botões de minimizar, maximizar e fechar continuam com as dicas do teste 8.
Desfazer (tira o plugin e o núcleo e fecha a janela de teste):
```bash
kill $(hyprctl clients -j | jq '.[] | select(.class == "kwt10") | .pid') 2>/dev/null
hyprctl plugin unload /tmp/kwinish-t10/kwinish-bars.so
hyprctl reload
```
Depois do desfazer, este comando não deve mostrar `Q1 violated` (o contador Q1 escreve no log ao descarregar) nem `error`:
```bash
grep -i kwinish "$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/hyprland.log" | tail -n 30
```
*Se a sessão travar:* aperte Ctrl+Alt+F3, entre com seu usuário e rode `hyprctl -i 0 plugin unload /tmp/kwinish-t10/kwinish-bars.so`.

**11. Leitura e gravação dos seus arquivos com o núcleo novo (harden-user-data-io; roteiro reescrito para a 0.57 em 01/11/2026; executado pelo dono em 01/11/2026: passou)**
O roteiro de 27/10 apontava para `~/Downloads/ryoku-work/kwinish`, que não existe mais; o original e o resultado parcial de 28/10 (passo da regra gravado e aplicado) estão no Git (`181438b`). Conferido em 01/11/2026, só lendo: `~/.config/kwinish/` não existe e o `kwinish-ui` não está no PATH. O teste cria os arquivos e põe um link do `kwinish-ui` do repositório em `~/.local/bin`; o desfazer devolve tudo como estava. As saídas de erro do Hyprland vão para o tty1, então os avisos do núcleo são conferidos lendo os arquivos fora do compositor (passo 5). Comandos com caminho completo; rode um por vez. Antes de tudo, este comando não deve listar nenhum `kwinish` (se listar, descarregue o caminho mostrado):
```bash
hyprctl plugin list
```
Backup primeiro (recusa se o diretório de estado já existir):
```bash
bash /home/lhen/Projects/ryoku-work/kwinish-port-0-57/tools/live-backup.sh save /home/lhen/.local/state/kwinish-backup/teste-11-057 /home/lhen/.config/kwinish/rules.json /home/lhen/.config/kwinish/layout /home/lhen/.config/kwinish/layouts.json /home/lhen/.config/kwinish/settings.json /home/lhen/.config/kwinish/workspace-modes.json /home/lhen/.config/kwinish/workspace-columns.json /home/lhen/.local/bin/kwinish-ui
```
```bash
cargo build --release --locked --manifest-path /home/lhen/Projects/ryoku-work/kwinish-port-0-57/ui/Cargo.toml
ln -s /home/lhen/Projects/ryoku-work/kwinish-port-0-57/ui/target/release/kwinish-ui /home/lhen/.local/bin/kwinish-ui
mkdir -p /tmp/kwinish-t11
make -C /home/lhen/Projects/ryoku-work/kwinish-port-0-57/plugin TARGET=/tmp/kwinish-t11/kwinish-bars.so
hyprctl plugin load /tmp/kwinish-t11/kwinish-bars.so
hyprctl eval "package.path='/home/lhen/Projects/ryoku-work/kwinish-port-0-57/core/?.lua;'..package.path; for _,m in ipairs({'kwinish','geometry','layouts','assist','rules','modes','binds','switcher','settings','kwjson'}) do package.loaded[m]=nil end; require('kwinish')"
kitty --class kwt11 &
```
Deve acontecer: `live-backup: 7 caminho(s) guardado(s)`; `cargo` e `make` sem `error`; `plugin load` sem erro; uma janela `kwt11` com barra. Deixe-a flutuante (SUPER+V).
1. **Carga sem arquivos:** as barras aparecem com a altura padrão e nada muda nos workspaces.
2. **Gravação de regra:** clique direito na barra do `kwt11` > Mais ações > Regras da janela > "Sempre manter acima das outras". Se o menu não abrir, o PATH do Hyprland não tem `~/.local/bin`: pare, desfaça e me avise. `cat /home/lhen/.config/kwinish/rules.json` mostra `{"kwt11":{"above":true}}` e `ls -a /home/lhen/.config/kwinish/` não mostra nenhum `.tmp`. Rode `kitty --class kwt11 &` de novo. Sem `workspace-modes.json` o workspace é tiling, e a regra não faz a janela flutuar (só a marca e a sobe na pilha das flutuantes): ela nasce em tiling, o que está certo. O sinal de que a regra pegou é a tag: `hyprctl clients -j | jq -c '.[] | select(.class == "kwt11") | {floating, tags}'` mostra `kwinish-above` nas tags da janela nova. Correção de 01/11/2026: a versão anterior deste passo esperava "nasce acima", herdado do roteiro de 27/10, quando os workspaces 1–3 do dono eram flutuantes.
3. **Gravação de layout:** `kwinish-ui layouts`, escolha um layout. `cat /home/lhen/.config/kwinish/layout` mostra o nome escolhido. Escolha de novo o que estava (ou "off").
4. **Gravação pela interface:** `kwinish-ui settings`, mude a "Altura" da barra (muda na hora) e volte ao valor de antes. `ls -a /home/lhen/.config/kwinish/` continua sem `.tmp` nem `.bak`.
5. **Leitura fora do compositor:** este comando lê os arquivos com os leitores do núcleo e deve imprimir `ok` nas três linhas:
```bash
lua -e "package.path='/home/lhen/Projects/ryoku-work/kwinish-port-0-57/core/?.lua;'..package.path; local j=require('kwjson'); local d=os.getenv('HOME')..'/.config/kwinish/'; for _,p in ipairs({{'rules.json','rules'},{'settings.json','settings'},{'workspace-modes.json','modes'}}) do local _,e=require(p[2]).parse(j.read(d..p[1])); print(p[1], e or 'ok') end"
```
Desfazer (fecha as janelas de teste, devolve os sete caminhos como estavam, inclusive removendo os que não existiam e o link, tira a pasta vazia, o plugin e o núcleo):
```bash
kill $(hyprctl clients -j | jq '.[] | select(.class == "kwt11") | .pid') 2>/dev/null
bash /home/lhen/Projects/ryoku-work/kwinish-port-0-57/tools/live-backup.sh restore /home/lhen/.local/state/kwinish-backup/teste-11-057
rmdir /home/lhen/.config/kwinish
hyprctl plugin unload /tmp/kwinish-t11/kwinish-bars.so
hyprctl reload
```
Resultado (01/11/2026): o dono relatou "o teste passou", sem dizer se do 10, do 11 ou dos dois. Conferido depois só lendo, com o teste 11 ainda não desfeito: `layout` = `thirds` e `settings.json` = `{"bar_height": 28}`, os três leitores do passo 5 dão `ok`, sem `.tmp` nem `.bak`; o log não tem `Q1 violated`. **Não há `rules.json`**: desmarcar a regra grava `{}` e não apaga o arquivo, então a regra do passo 2 não foi gravada (menu não abriu, item não clicado ou gravação recusada; o stderr vai para o tty1). Falta confirmar o passo 2 com o dono e rodar o desfazer. Na repetição (dono, 01/11/2026), a janela nova abriu em tiling, e o `rules.json` continuou ausente. Conferido só lendo: o PATH do Hyprland tem `~/.local/bin`, o link aponta para o binário e o plugin está carregado. Fora do compositor, o mesmo caminho do núcleo (`rules.toggle` → `rules.serialize` → `kwjson.write`, que cria a pasta) grava `{"kwt11":{"above":true}}` num HOME temporário. **Causa e fechamento (01/11/2026):** não era defeito. A ação `rule:above` é um interruptor; um clique no menu ligou a regra depois da conferência, e a chamada manual de diagnóstico desligou de novo (gravou `{}`, janela sem a tag). Repetida a chamada, `rules.json` = `{"kwt11":{"above":true}}` e as duas janelas `kwt11` com `kwinish-above` (conferido só lendo). No Hyprland 0.57 a classe chega como string (`LuaWindow.cpp`, `lua_pushstring`). **Passo 2 passou.** Observação: o arquivo `layout` (gravado no passo 3) sumiu antes do fim; nenhum código do núcleo ou do `kwinish-ui` apaga arquivos de configuração, e o desfazer trata arquivo ausente. Falta o dono rodar o desfazer.
O `rmdir` só remove a pasta se ela estiver vazia; se reclamar, rode `ls -a /home/lhen/.config/kwinish/` e me mostre. *Se a sessão travar:* Ctrl+Alt+F3, entre com seu usuário e rode `hyprctl -i 0 plugin unload /tmp/kwinish-t11/kwinish-bars.so`.

**12. Pacote novo da master: janela nova no modo floating e "Manter acima das outras" (28/10)**
**Contexto original do roteiro:** os pacotes r86/r85 de 25/10 eram anteriores à instalação r172 registrada depois. O pacote daquele relato (`kwinish-git r86.260f15b` e `kwinish-bars-plugin-git r85.738905b`) era anterior à correção da abertura no modo floating; o núcleo dele fazia a janela flutuar 150 ms depois de abrir, por isso ela aparecia primeiro no tamanho do tiling, mais ou menos visível conforme a velocidade do app. Rode depois do desfazer do teste 11. Gerar os pacotes (fora do repositório; o `check()` roda as suítes Lua) e instalar:
```fish
set w (mktemp -d); mkdir $w/core $w/plugin
cp ~/Downloads/ryoku-work/kwinish/packaging/PKGBUILD $w/core/; cp ~/Downloads/ryoku-work/kwinish/packaging/plugin/* $w/plugin/
cd $w/core; KWINISH_SRC=$HOME/Downloads/ryoku-work/kwinish makepkg -f --nodeps
cd $w/plugin; KWINISH_SRC=$HOME/Downloads/ryoku-work/kwinish makepkg -f --nodeps
sudo pacman -U $w/core/kwinish-git-0.0.0.r*-x86_64.pkg.tar.zst $w/plugin/kwinish-bars-plugin-git-0.0.0.r*-x86_64.pkg.tar.zst
hyprctl plugin unload /usr/lib/hyprland/plugins/kwinish-bars.so
hyprctl plugin load /usr/lib/hyprland/plugins/kwinish-bars.so
hyprctl reload
```
(O `*` exclui os pacotes `-debug`.) Teste:
1. No workspace 1 (floating), abra várias vezes o `kitty` e o app em que o problema apareceu: cada janela aparece direto flutuante, no tamanho final, nunca primeiro no tamanho do tiling. Elas ainda nascem no mesmo lugar (cascata parada, achado de 28/10).
2. Num `kitty` flutuante, clique direito na barra > "Manter acima das outras". Clique numa janela que fica por baixo dele: o foco vai para ela (dá para digitar nela), mas o `kitty` continua desenhado por cima. Desmarque no mesmo item.
Desfazer (volta aos pacotes de 25/10, guardados em `dist/`):
```fish
cd ~/Downloads/ryoku-work/kwinish
sudo pacman -U dist/kwinish-git-0.0.0.r86.260f15b-1-x86_64.pkg.tar.zst dist/kwinish-bars-plugin-git-0.0.0.r85.738905b-1-x86_64.pkg.tar.zst
hyprctl plugin unload /usr/lib/hyprland/plugins/kwinish-bars.so
hyprctl plugin load /usr/lib/hyprland/plugins/kwinish-bars.so
hyprctl reload
```

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
- 27/10: testes 6 e 1 ("os testes funcionaram"; roteiros no commit `60e19df`), teste 5 ("tudo funcionou"; roteiro no commit `a431458`), teste 7 ("os testes passaram"; diálogo medido pelo Claude), testes 8 e 9 ("testes passaram"; roteiros no commit `16b20cc`: dicas nos botões e a causa do QS Bar vazio). Mudanças arquivadas em `openspec/changes/archive/2026-10-27-*`; specs principais `window-snapping`, `snap-assist`, `workspace-modes` e `title-bar`.

## Fora de escopo / descartado
Mexer na memória do `CWindow` para esconder janelas; copiar código GPL (KWin, Pop Shell, HyDE); distro própria (ideia registrada, nada iniciado); abrir a janela no "tamanho pedido pelo app" no modo floating (uma regra por workspace não sabe a tempo que a janela vai flutuar; medido em 27/10).
