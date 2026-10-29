# Kwinish: estado e evidências

Revisão documental: **29/10/2026**, baseline `a2506c63dad16384414dc95919b8292aead284df` (173 commits). Nenhum teste, build, instalação ou ação no compositor foi executado nesta consolidação.

Arquitetura: [ARCHITECTURE](ARCHITECTURE.md); workflow: [CONTRIBUTING](CONTRIBUTING.md); armadilhas e relatos originais: [HANDOFF histórico](history/handoffs/HANDOFF.md); testes: [TESTING](TESTING.md); regras: [AGENTS](../AGENTS.md).

**Inventário histórico, não consulta atual da máquina:** a [auditoria de 28/10](history/product-import/2026-10-28/external-originals/docs/product/engineering/AUDITORIA_CODEX_ORIGINAL_2026-10-28.txt) registra os dois pacotes em `0.0.0.r172.5333087-1` e Hyprland `0.56.2-4`. Referências a r86/r85 abaixo pertencem aos roteiros e resultados anteriores, incluindo seus meios de desfazer. Não demonstram quais pacotes estão instalados hoje.

## Limites atuais e produto futuro

- A cascata Floating continua com falha ao vivo registrada em 28/10; isso diverge do requisito em [workspace-modes](../openspec/specs/workspace-modes/spec.md). A causa sugerida no relato não foi confirmada por esta revisão.
- Minimizar/ocultar pode perder `kwinish-mode-floated`; `show_desktop` guarda um único `stashed` local que se perde ao recarregar o núcleo. Hooks/timers e internals do plugin continuam riscos técnicos separados, conforme a auditoria de 28/10.
- `harden-user-data-io` e `add-pin-and-icon-tooltips` permanecem mudanças ativas. Tarefas marcadas não equivalem a arquivamento ou cobertura ao vivo completa. Nada foi sincronizado no OpenSpec nesta reorganização.
- Barra/dock/launcher oficiais, Overview e Rice Store não fazem parte da implementação atual. D-43/D-44/D-45 e a API futura de restauração pertencem ao [produto](product/WORKSPACES.md), sem validação executada.
- Hyprland 0.56.2 é a base documentada; 0.57 exige trabalho independente. O [parecer externo](history/product-import/2026-10-28/external-originals/docs/product/engineering/HYPRLAND_0_57_COMPATIBILITY.md) é preliminar, não certificado de compatibilidade.

## O que é
Uma camada de gerenciamento de janelas estilo KDE/KWin para o Hyprland 0.56.2, em três peças:
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
4. **Achados da auditoria do Codex (27/10) ainda não corrigidos** (reproduzidos em isolamento, não vistos no desktop); correções futuras devem passar por mudanças próprias no OpenSpec:
   - ciclo de vida (etapa C): minimizar apaga a procedência do modo floating (a janela pode não voltar ao mosaico); "mostrar área de trabalho" perde as origens depois de um reload; um ajuste de colunas atrasado sobrescreve a escolha mais nova; recarregar o módulo mantém callbacks antigos; `zone` não confere tela cheia como `tile` confere;
   - entrada e comunicação (etapa D): a barra da janela focada aceita o clique destinado à janela da frente; leitura sem tempo-limite no socket das barras; o Assist não revalida workspace, monitor e layout na escolha; cache de símbolos dos botões sem escala;
   - entrega (etapa E): o `Makefile` do plugin ignora os `.hpp`; `kwinish-bars-rebuild` anuncia sucesso sem conferir os `install`; CI.
5. **Funcionalidades propostas, não compromisso de implementação:** duplo clique no ícone fecha a janela; seletor de layout ao passar o mouse no maximizar; miniaturas no Alt+Tab e no Snap Assist, `rounding > 0` nas zonas e na prévia; minimizar com filhas/modais; bind para ciclar o modo do workspace, indicador na barra, modo por monitor; página no Ryoku Hub; opt-out de barra por app com decoração própria.
6. **Bug do shell achado em 27/10 (não é do Kwinish):** maximizar uma janela esvazia o QS Bar e deixa só o espaço reservado. Causa e correção proposta: seção "Relato do dono: maximizar esvazia o QS Bar" abaixo; causa confirmada pelo dono (teste 9, 27/10). A correção é do shell.
7. **Bugs e validações pendentes:** o relato antigo de encaixe entre monitores voltando à origem motivou o sufixo de monitor em `tile:*`/`zone:*`; a correção existe no código e tem simulação registrada, mas o cenário de arrasto real continua pendente no teste 3. Demais achados registrados: atalho não guarda o submap anterior, vários daemons de barras podem coexistir, corrida entre dois diálogos de atalho; busca de ícones com I/O no render; `snap` guarda o retângulo pedido, não o obtido.
8. **Publicação/beta:** ideias históricas em [BETA](history/initial-research/BETA.md). Nome definitivo, publicação, testes de instalação/remoção e upgrade real continuam decisões/trabalhos separados. A consolidação preserva o histórico e não autoriza sua limpeza nem assume dependências externas de cronograma.

## Para a próxima sessão
1. Ler [AGENTS](../AGENTS.md), este arquivo, [arquitetura atual](ARCHITECTURE.md) e [HANDOFF histórico](history/handoffs/HANDOFF.md) (armadilhas: objetos de `hl.bind` por `hyprctl eval` derrubam o Hyprland; `window.open_early` não aceita flutuar/redimensionar; etiqueta posta por regra aparece como `nome*`).
2. `git status` e `git log --oneline -5`: o baseline desta revisão é `a2506c6`. Mudanças documentais não commitadas podem existir; não confundi-las com trabalho de código. Os diretórios não rastreados `.agents/skills/source-command-opsx-*` e as worktrees `kwinish-apply` e `.alethe/worktrees/job-0*` são antigos: não apagar sem o dono.
3. Para trabalho de código, `bash tools/check.sh` é a validação de software prevista no AGENTS. Nesta tarefa exclusivamente documental, foram verificadas referências e preservação, sem executar suítes ou builds.
4. `PATH=$HOME/.local/bin:$PATH openspec list` mostra `add-pin-and-icon-tooltips` (tarefas feitas; arquivar depois do teste 10 do dono) e `harden-user-data-io` (13/13, revisada pelo Codex em 28/10; arquivar depois do teste 11).
5. Carregar o código do repositório no desktop é do dono: `fish ~/Downloads/ryoku-work/kwinish/tools/dev-reload.fish` (plugin e núcleo juntos; vale até o próximo `hyprctl reload`). Mudanças dentro de blocos protegidos por `KWINISH_*` só valem com um bloco novo ou com o pacote reinstalado.
6. Delegação: `codex exec -s read-only` para revisão e `-s workspace-write` numa worktree nova para implementar funcionaram; o dono tem cota do Codex (GPT-6.1). Revisar o diff antes de juntar.
7. A anotação anterior registrava o backup do teste 5 em `~/.local/state/kwinish-backup/teste-5`, sem desfazer executado pelo dono. O comando histórico de restauração é `bash ~/Downloads/ryoku-work/kwinish/tools/live-backup.sh restore ~/.local/state/kwinish-backup/teste-5`. Sua existência e a decisão de restaurar/descartar não foram verificadas nesta revisão; não remover como parte da manutenção documental.

## Testes pendentes para o dono

Roteiros e resultados parciais preservados de 27–28/10. **Não executados nesta consolidação, nem autorizados por ela.** Antes de um teste futuro, conferir versões e caminhos; os comandos de instalação/desfazer abaixo refletem os pacotes e backups da data original. A auditoria de 28/10 já registra r172 instalado, mas não fornece resultado completo do teste 12.
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

**10. Dicas no fixar e no ícone do aplicativo (add-pin-and-icon-tooltips, 27/10)**
Carregue o plugin e o núcleo do repositório (o primeiro `unload` só vale se o plugin instalado estiver carregado; se não estiver, o erro dele pode ser ignorado):
```fish
cd ~/Downloads/ryoku-work/kwinish
make -C plugin
hyprctl plugin unload /usr/lib/hyprland/plugins/kwinish-bars.so
fish tools/dev-reload.fish
```
Teste, numa janela flutuante com a barra do Kwinish (por exemplo um `kitty`):
1. Pare o ponteiro 1 s no botão de fixar (à esquerda): aparece "Em todas as áreas de trabalho".
2. Clique nele (a janela fica fixada; a dica some). Saia do botão e pare de novo: agora diz "Só nesta área de trabalho". Clique de novo para desfixar.
3. Pare o ponteiro 1 s no ícone do aplicativo (canto esquerdo da barra): aparece "Menu da janela", abaixo do ícone e dentro da tela.
4. Clique no ícone: o menu da janela abre como antes e a dica some.
5. Os botões de minimizar, maximizar e fechar continuam com as dicas do teste 8.
Desfazer (volta ao plugin e ao núcleo instalados; nenhum arquivo seu é mudado pelo teste):
```fish
hyprctl plugin unload ~/Downloads/ryoku-work/kwinish/plugin/kwinish-bars.so
hyprctl plugin load /usr/lib/hyprland/plugins/kwinish-bars.so
hyprctl reload
```

**11. Leitura e gravação dos seus arquivos com o núcleo novo (harden-user-data-io, 27/10)**
Antes do teste, os leitores novos já foram rodados sobre os seus arquivos reais, só lendo e fora do compositor: aceitam todos, com as mesmas entradas dos antigos (2 layouts, 4 opções, workspaces 1 a 3 em floating, colunas `2=1`). O `kwinish-ui` do seu PATH é o do repositório (`~/.local/bin/kwinish-ui` aponta para `ui/target/debug/`) e já tem o código novo da interface. Guarde o que o teste pode mudar e carregue o plugin e o núcleo do repositório (o `unload` do instalado só vale se ele estiver carregado; se não estiver, ignore o erro):
```fish
cd ~/Downloads/ryoku-work/kwinish
bash tools/live-backup.sh save ~/.local/state/kwinish-backup/teste-11 ~/.config/kwinish/rules.json ~/.config/kwinish/layout ~/.config/kwinish/settings.json ~/.config/kwinish/workspace-modes.json ~/.config/kwinish/workspace-columns.json
make -C plugin
hyprctl plugin unload /usr/lib/hyprland/plugins/kwinish-bars.so
fish tools/dev-reload.fish
```
Teste:
1. Nada muda ao carregar: as barras mantêm altura e preenchimento das suas configurações e os workspaces 1, 2 e 3 continuam em floating. `kwinish-ui layouts` mostra "teste" e "meu layout".
2. Nenhum aviso do núcleo: `tail -n 100 ~/.local/share/sddm/wayland-session.log | string match '*kwinish:*'` não mostra nada.
3. Gravação de regra: num `kitty` flutuante, clique direito na barra > Mais ações > Regras da janela > "Sempre manter acima das outras". `cat ~/.config/kwinish/rules.json` mostra `{"kitty":{"above":true}}` e `ls -a ~/.config/kwinish/` não mostra nenhum `.tmp`. Abra outro `kitty`: nasce acima. Reabra o menu: a regra aparece marcada; desmarque.
4. Gravação de layout: em `kwinish-ui layouts`, escolha outro layout. `cat ~/.config/kwinish/layout` mostra o nome escolhido. Volte ao de antes.
5. Gravação pela interface: `kwinish-ui settings`, mude a "Altura" da barra (muda na hora) e volte ao valor de antes. `ls -a ~/.config/kwinish/` continua sem `.tmp` nem `.bak`.
Resultado parcial (dono, 28/10): passo 3 gravou o arquivo, o segundo `kitty` nasceu acima e a regra apareceu marcada. O item desmarcado depois foi o "Manter acima das outras" da janela (sem arquivo): `rules.json` continua `{"kitty":{"above":true}}`, e o desfazer abaixo o remove. A janela mantida acima perdia o lugar ao clicar noutra: defeito à parte, corrigido em `5333087` (teste 12). Faltam os passos 1, 2, 4 e 5. Sem terminal, os passos 4 e 5 vão pelo menu da janela (clique direito na barra): Mais ações > Layout de tiling (o escolhido aparece marcado, é o que o arquivo `layout` guarda) e Mais ações > Configurações do Kwinish...
Desfazer (devolve os cinco arquivos como estavam; o `rules.json`, que não existia, deixa de existir) e volta ao plugin e ao núcleo instalados:
```fish
bash ~/Downloads/ryoku-work/kwinish/tools/live-backup.sh restore ~/.local/state/kwinish-backup/teste-11
hyprctl plugin unload ~/Downloads/ryoku-work/kwinish/plugin/kwinish-bars.so
hyprctl plugin load /usr/lib/hyprland/plugins/kwinish-bars.so
hyprctl reload
```

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
