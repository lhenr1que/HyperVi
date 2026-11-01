# O que o usuário pode configurar no Kwinish

Levantamento original de 26/10/2026; revisão documental por leitura estática em 29/10/2026, baseline `a2506c6`, sem novos testes. Fontes: [plugin](../plugin/main.cpp), [núcleo](../core/kwinish.lua), [Settings GTK](../ui/src/settings.rs) e [teclas](../core/binds.lua). A ideia de [página no Hub](history/reference/ideias/pagina-no-hub.md) foi preservada no histórico.

**Aplica** diz quando a mudança vale. "Não verificado" significa ausência de teste ao vivo registrado para aquele item. A janela atual configura plugin e Snap Assist; o [Settings da futura suíte](product/SETTINGS.md) não está implementado por ela.

## 1. Opções do plugin (`plugin:kwinish:*`, na config do Hyprland ou por `hyprctl eval "hl.config({plugin={kwinish={...}}})"`)
Aplicam na hora, sem recarregar o plugin (verificado só para `seam_resistance`; as demais não verificado).
**Desde 26/10** quase todas têm interface: `kwinish-ui settings` grava `~/.config/kwinish/settings.json` (validado por `core/settings.lua`, aplicado pelo núcleo a cada carga da config e na hora pela janela). Fora da janela: `inactive_button_color` (o núcleo aceita, a janela não mostra) e `button` (palavra-chave).

| Grupo | Opção (padrão) |
|---|---|
| Barras | `enabled` (true), `floating_only` (false), `bar_height` (28), `bar_part_of_window` (true), `bar_precedence_over_border` (true), `bar_padding` (7) |
| Cores | `bar_color`, `inactive_bar_color`, `col.text`, `inactive_col.text`, `button_hover_color`, `pinned_color`, `preview_color` |
| Título | `bar_title_enabled` (true), `bar_text_size` (12), `bar_text_weight` (400), `bar_text_font` ("Sans"), `bar_text_align` ("left"), `bar_blur` (false) |
| Botões | `default_buttons` (true), `bar_buttons_alignment` ("right"), `bar_button_padding` (5), `icon_on_hover` (false), `show_icon` (true), `on_double_click` ("maximize"), `button` (palavra-chave, um por botão) |
| Encaixe | `snap_top_edge` (true), `snap_preview` (true), `seam_resistance` (200 px, 0 desliga) |

**Alpha-1 do porte 0.57** (OpenSpec `enable-titlebar-drag-alpha1`, código em `port-kwinish-bars-to-hyprland-0-57`, não validado ao vivo): arrastar pela barra voltou, e as três opções de Encaixe voltam a agir, com os mesmos nomes, tipos e padrões. Diferença da 0.56.2: a animação de 140 ms ao sair do encaixe acontece antes de o arrasto começar, e não com ele pausado (mesma aparência); se outro programa capturar a entrada bem na hora de soltar, o arrasto continua até a próxima tecla ou clique (custo aceito). O toque na barra continua desligado.

## 2. Arquivos do usuário (`~/.config/kwinish/`)
| Arquivo | O que guarda | Quem edita hoje | Aplica |
|---|---|---|---|
| `layouts.json` | layouts de zonas e o espaçamento (`gap`) | editor de zonas (`kwinish-ui zones`, Super+Ctrl+E) | ao reabrir o seletor de layouts (relê ao mudar de layout) |
| `settings.json` | opções do plugin e `snap_assist` | `kwinish-ui settings` | na hora, por `apply_settings`, e em cada carga do núcleo |
| `layout` | nome do layout ativo (uma linha; `off` desliga) | seletor (`kwinish-ui layouts`, Super+T) | na hora |
| `rules.json` | regras por app: acima, abaixo, sem moldura, sem captura, workspace, lembrar posição | menu da janela, "Regras da janela" | na próxima janela do app; desmarcar desfaz o efeito na janela já aberta (acima, abaixo, sem moldura, sem captura; posição e workspace não voltam) |
| `shortcuts.json` | uma tecla por app que o foca | `kwinish-ui shortcut` | **só ao recarregar a config** (o Hyprland 0.56.2 pode cair com `hl.bind` por eval) |
| `workspace-modes.json` | modo de cada workspace: tiling, scrolling, floating | menu da janela, "Modo da workspace"; `cycle_workspace_mode` | na hora |
| `workspace-columns.json` | 1 a 3 colunas por tela no modo scrolling | menu da janela | na hora, em workspace no modo scrolling |

## 3. Configuração do núcleo (Lua, com as exceções de UI indicadas)
- `kw.setup({ placement = "smart", autoraise_ms = 400 })`: posição inicial das janelas e auto-elevar ao focar. Nenhum tem interface.
- `kw.snap_assist = false` desliga o Snap Assist; essa opção também existe em `kwinish-ui settings`. `kw.layout_gap` troca o espaçamento entre zonas via Lua.
- `binds.install(kw, { mod, switcher, menu, override })`: teclas padrão (`binds.rows`, 19 linhas) e as que o dono troca em `override`. Nenhuma interface lista nem troca as teclas.
- `kw.hotcorners({ corners, delay_ms, edge, tick_ms })`: cantos ativos. O temporizador só para recarregando a config.
- `switcher.setup()` e as teclas de Alt+Tab (`switcher_keys`).

## 4. Pelo menu da janela (botão direito na barra, ou Alt+F3)
Mais ações > Configurações do Kwinish... abre `kwinish-ui settings`.
Manter acima/abaixo, tela cheia, flutuar/tilar, sem moldura, ocultar das capturas, fixar em todas as workspaces, mover para workspace, encaixar (metades, quartos, eixos), layout de mosaico, modo e colunas da workspace, regras por app, mover/redimensionar, minimizar, maximizar, fechar. Na alpha-1 do porte 0.57, Mover e Redimensionar voltaram ao menu, depois de Minimizar.

## 5. Lacunas e perguntas (o que a revisão achou)
1. ~~Nenhuma opção do plugin tem interface.~~ Resolvido em 26/10: `kwinish-ui settings` (janela própria, não página do Hub; o Hub pode só abri-la). Ainda sem interface: auto-elevar, posição inicial, cantos ativos, teclas padrão (itens 4 e 5).
2. ~~Desmarcar uma regra não desfazia a janela aberta.~~ Resolvido em 26/10 por decisão do dono: desmarcar (ou "esquecer regras") desfaz na hora acima, abaixo, sem moldura e sem captura. Posição e workspace não voltam.
3. **Atalhos por app só valem depois de recarregar.** O menu não diz isso.
4. **As teclas padrão não são visíveis nem editáveis** por interface; só por `override` em Lua.
5. **Auto-elevar, posição inicial e cantos ativos** (seção 3) não têm interface nem sinal de que existem; uma página futura pode expô-los; isso continua planejamento de produto.
6. **Validação:** os arquivos `.json` são lidos como dados JSON, nunca executados; um JSON com erro de sintaxe, acima de 1 MiB ou de 32 níveis, ou com a raiz errada é ignorado inteiro, e uma entrada inválida num documento válido cai sozinha (contrato em [API](API.md), seção 4). O arquivo `layout` é texto, com leitor separado: a primeira linha, sem espaços, deve ser `off` ou um preset conhecido; na ausência ou nome desconhecido usa `grid`. Os limites JSON não descrevem esse leitor. Em `layouts.json`, núcleo e `kwinish-ui` aplicam as mesmas regras (`core/testdata/layout-cases.txt`): nome com até 64 caracteres `[A-Za-z0-9-]`; `gap` inteiro limitado a 0–60 (`-5` vale 0, `999999` vale 60); `gap` fracionário (`2.5`, `6.0`), string, `null` ou ausente usa o espaçamento padrão (`kw.layout_gap` ou o `gaps_in` do Hyprland). Uma zona que, depois do espaçamento, ficaria com menos de 1 px de largura ou altura (menor que o próprio espaçamento) não é publicada ao plugin, não aparece no Shift ao arrastar nem no Snap Assist, e `zone:N` não move a janela para ela; as outras zonas mantêm o número. As opções do plugin dependem da validação do Hyprland (não verificado).
7. **Dois lugares para o mesmo ajuste** (config e eval) e nenhum comando que mostre os valores atuais. Uma página no Hub poderia ler com `hyprctl getoption`.
