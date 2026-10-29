# KWin versus Hyprland e Ryoku: catálogo de funcionalidades

Levantamento para você avaliar o que vale trazer do KWin para o Hyprland (Ryoku).
Marque a coluna **Quero?** com `[x]`.

## Fontes e método

- **KWin:** clone raso do `KDE/kwin` (commit `b513396`, 17/10/2026), lido em modo leitura. São cerca de 264 mil linhas de C++/QML/Qt, 76 diretórios de plugin (efeitos e serviços), 13 módulos de configuração (KCM), 34 propriedades de regra de janela e uns 90 atalhos de teclado. Os metadados dos plugins, as chaves de `kwin.kcfg` e de `rulesettings.kcfg` e os arquivos de `tabbox/`, `tiles/` e `useractions.cpp` foram lidos diretamente.
- **Hyprland:** a documentação oficial (regras de janela, dispatchers, eventos Lua) e o que testamos na VM (Hyprland 0.56.2).
- **Ryoku:** o repositório (`unstable-dev` 0.103.x): lista de capacidades, módulos do shell, páginas do Hub e atalhos.

## Legenda

- **Hyprland:** `Nativo` (existe no compositor), `Parcial`, `Não`.
- **Ryoku hoje:** `Pronto`, `Parcial`, `Não`.
- **Esforço para trazer:** `P` (horas a poucos dias, Lua/QML), `M` (dias, Quickshell mais Lua), `G` (semanas, plugin C++ ou protocolo novo), `X` (não faz sentido ou não é possível).

## Números

Contagem feita a partir das próprias tabelas abaixo (62 recursos):

| Situação | Quantidade |
|---|---|
| Ryoku: Pronto | 39 |
| Ryoku: Parcial | 12 |
| Ryoku: Não | 10 |
| Ryoku: não se aplica (sessão X11) | 1 |
| Hyprland: Nativo (inclui os scripts em Lua) | 21 |
| Hyprland: Parcial | 16 |
| Hyprland: Não | 24 |
| Hyprland: não se aplica | 1 |
| Esforço só P ou M (Lua e Quickshell) | 48 |
| Esforço inclui G (plugin C++ ou protocolo), sem X | 7 |
| Esforço inclui X (inviável ou sem sentido, inclusive "provavelmente X") | 7 |

## A. Janelas e regras

| # | Recurso do KWin | Hyprland | Ryoku hoje | Esforço | Quero? |
|---|---|---|---|---|---|
| 1 | Regras por aplicativo (34 propriedades). O Hyprland cobre cerca de 25: posição, tamanho, flutuar, maximizar, tela cheia, workspace, monitor, opacidade, sem borda, tamanho mínimo e máximo, VRR e tearing, foco, captura de tela, tags | Nativo | Parcial (páginas Window Rules, App Overrides e Layer Rules, uma ação por regra; botão "De uma janela" cria a regra a partir de uma janela aberta) | P (interface) | [x] |
| 2 | Regras que o Hyprland não tem: pular barra de tarefas, pager e alternador, camada acima ou abaixo, atividade, atalho por janela, maximizar só na horizontal ou vertical | Não | Parcial (regras "Manter acima" e "Ocultar do dock e do alternador" por tag; maximizar num eixo existe no menu da janela; faltam manter abaixo, pager, atalho por janela e atividade) | M (algumas), X (atividade) | [x] |
| 3 | Manter acima e abaixo (camadas entre janelas flutuantes) | Parcial (`alter_zorder`) | Pronto ("Manter acima" no menu da janela: um módulo Lua recoloca a janela no topo a cada troca de foco; não há "manter abaixo") | M | [x] |
| 4 | Maximizar só na horizontal ou só na vertical | Não | Pronto (menu da janela: maximizar na horizontal ou na vertical, para janelas flutuantes; pedir de novo restaura) | M | [x] |
| 5 | Encaixe rápido: metades e cantos (Quick Tile) | Não | Pronto (`Super + setas`: metades, cantos, maximizar, restaurar; `Super + Ctrl + Shift + Cima/Baixo`: metade de cima e de baixo; distância de encaixe no Hub) | P (completar topo e base) | [x] |
| 6 | Zonas de encaixe customizáveis e editor de ladrilhos (Tiles Editor) | Não | Não | G | [sim, porém para depois] |
| 7 | Encaixe ao arrastar para a borda da tela (maximizar ou ladrilhar, "electric borders") | Não | Pronto (soltar a janela na borda esquerda ou direita encaixa na metade, no topo maximiza; vale para arrastar pela barra de título ou com Super, só no layout flutuante) | M a G | [x] |
| 8 | Imã de bordas e janelas (border, window e center snap) | Nativo (`general.snap`) | Pronto | P | [x] |
| 9 | Políticas de posicionamento: inteligente (menos sobreposição), cascata, centrada, aleatória, canto, sob o mouse | Parcial (`center`, `move`, expressões com o cursor) | Pronto (layout flutuante: cascata, inteligente com menor sobreposição, centrada ou sob o ponteiro; falta aleatória e canto) | M (inteligente) | [x] |
| 10 | Política de foco: clique, seguir o mouse, prevenção de roubo de foco, elevar ao passar com atraso (auto-raise) | Parcial (sem auto-raise) | Pronto (Hub > Entrada: clicar, seguir o ponteiro, trazer para frente após um atraso e impedir roubo de foco) | P a M | [x] |
| 11 | Ações do mouse (Alt+arrastar mover e redimensionar, roda na barra de título) | Nativo (`bindm`) | Parcial (Hub > Entrada: Super ou Alt para arrastar e redimensionar; falta a roda do mouse na barra de título, que o hyprbars não oferece) | P | [x] |
| 12 | Menu da janela (Alt+Espaço: mover, redimensionar, manter acima, sem borda, criar regra) | Não | Parcial (`Alt + F3`: minimizar, maximizar, maximizar num eixo, tela cheia, flutuar ou ladrilhar, manter acima, mover para outra área de trabalho, fechar; faltam mover e redimensionar pelo menu e criar regra) | M | [x] |
| 13 | Aviso "o aplicativo não está respondendo" (frozenapp e killprompt) | Nativo (`misc:enable_anr_dialog`, ligado por padrão) | Pronto (validado no PC real em 21/10/2026: janela congelada mostra "O aplicativo não está respondendo" com Encerrar e Esperar; exige kernel e módulo NVIDIA alinhados com as bibliotecas, senão o `hyprland-dialog` aborta no EGL) | M | [x] |
| 14 | Excluir janela da captura de tela e da gravação | Nativo (`no_screen_share`) | Pronto (menu da janela e regra "Ocultar de capturas e gravações": a janela sai preta em capturas, gravações e compartilhamento) | P | [x] |
| 15 | Mover, redimensionar e empacotar pelo teclado (Pack left, Grow, Shrink) | Parcial | Pronto (`Super + Shift + setas` empacota na borda, `Super + Ctrl + setas` cresce e encolhe, `Super + Ctrl + Shift + setas` e `[` `]` crescem até a vizinha, `Alt + F7` move por passos, `Alt + F8` redimensiona) | P | [x] |
| 16 | Decorações de janela (KDecoration, temas Aurorae, botões à esquerda ou direita, duplo clique) | Parcial (plugin hyprbars) | Parcial (hyprbars: botões à direita ou à esquerda, duplo clique, estilo escuro, claro ou translúcido e título à esquerda ou centralizado; sem temas de decoração) | M (botões configuráveis), G (temas) | [x, primeiro botoes, posteriormente pensamos em temas] |
| 17 | Janela maximizada sem borda | Nativo (regra) | Pronto (regra no estado maximizado: sem borda nem cantos) | P | [x] |
| 18 | Agrupar janelas em abas | Nativo (groups) | Pronto | P | [x] |

## B. Áreas de trabalho, atividades e telas

| # | Recurso do KWin | Hyprland | Ryoku hoje | Esforço | Quero? |
|---|---|---|---|---|---|
| 19 | Áreas de trabalho virtuais (grade, linhas, por tela, rolar ao fim, aviso na tela) | Parcial (numeradas, por monitor; sem grade 2D) | Pronto (workspaces, reordenar, roda do mouse) | M (grade 2D) | [x] |
| 20 | Atividades (Activities do KDE) | Não | Não | G, provavelmente X | [Nao] |
| 21 | Várias telas (foco, mover janela entre telas) | Nativo | Pronto | P | [x] |
| 22 | Layout de telas (resolução, escala, HDR, espelhar, perfis) | Nativo | Pronto (página Displays com perfis) | P | [x] |
| 23 | Identificar telas (Output Locator) | Não | Pronto (botão Identificar na página Monitores do Hub) | P | [x] |
| 24 | Cantos e bordas ativas (ações ao encostar o cursor: visão geral, mostrar área de trabalho, travar) | Não | Pronto (cantos e bordas inteiras: visão geral, launcher, configurações rápidas, mostrar área de trabalho e bloquear, no Hub; as bordas esperam mais para não disparar com a barra) | M (Quickshell com hotspots) | [x] |
| 25 | Mostrar a área de trabalho (Show Desktop) | Não | Pronto (`Super + Ctrl + D`, sobrevive a recarregar a configuração) | P | [x] |

## C. Troca e visão geral de janelas

| # | Recurso do KWin | Hyprland | Ryoku hoje | Esforço | Quero? |
|---|---|---|---|---|---|
| 26 | Alt+Tab com miniaturas (grade, só o aplicativo atual, modos por área, minimizadas, multitela) | Parcial (alternar, última janela) | Pronto (no Hyprland: grade de miniaturas por ordem de foco, minimizadas incluídas, soltar o Alt escolhe; falta o modo só do aplicativo atual e por tela) | M a G (miniaturas por captura de janela) | [x] |
| 27 | Visão geral e Present Windows e grade de desktops | Não (plugin externo) | Pronto (Overview, "Overview by desktop") | P | [x] |
| 28 | Thumbnail Aside e Highlight Window (destacar ao passar na barra de tarefas) | Não | Parcial (contorno na janela ao passar o mouse na miniatura do dock do Ryoku e do ii, e ao selecionar no Alt+Tab, e no dock do Caelestia) | M | [x] |
| 29 | Escurecer janelas inativas, escurecer a tela | Nativo (`dim_inactive`) | Pronto | P | [x] |

## D. Efeitos visuais

| # | Recurso do KWin | Hyprland | Ryoku hoje | Esforço | Quero? |
|---|---|---|---|---|---|
| 30 | Blur atrás de janelas translúcidas | Nativo | Pronto | P | [x] |
| 31 | Translucência e opacidade por regra | Nativo | Pronto | P | [x] |
| 32 | Animações de abrir e fechar (Fade, Scale, Glide, Squash, Sheet) | Parcial (popin, slide, fade) | Pronto (página Animations com presets e curvas) | M (Glide, Squash, Sheet) | [x, porem podemos comecar com uma simples e deixar as outras para outro momento] |
| 33 | Minimizar com efeito (Magic Lamp, Window Aperture, Squash) | Não (não há minimizar nativo) | Parcial (minimizar e restaurar de verdade: ações `window.minimize` e `window.restore`, docks restauram com um clique; falta o efeito) | G | [minimizar feito; efeito depois] |
| 34 | Janelas "gelatina" (Wobbly Windows) | Parcial | Pronto (ajuste de wobble na página Appearance) | P | [x] |
| 35 | Fall Apart (janela se desfaz em pedaços) | Não | Não | G, provavelmente X | [Nao] |
| 36 | Deslizar entre áreas de trabalho | Nativo | Pronto | P | [x] |
| 37 | Transições suaves de mudança de tema (Blend Changes, Fade Desktop) | Não | Pronto (o efeito de troca de tema do Ryoku, a capa de recarga do shell, serve; decisão do usuário em 21/10/2026, sem fade gradual extra; medido: ~0,3 s, sem piscar nem quadro vazio) | M | [x] |
| 38 | Popups e notificações que deslizam | n/a (é do shell) | Pronto (Quickshell) | P | [x] |
| 39 | Retorno visual ao abrir aplicativo (Startup Feedback) | Não | Parcial (cartão "Abrindo <app>…" com ícone e indicador até a janela aparecer; os lançamentos por `execute()` do dock ii e de alguns launchers ainda não avisam) | P | [x] |

## E. Acessibilidade, entrada e ferramentas

| # | Recurso do KWin | Hyprland | Ryoku hoje | Esforço | Quero? |
|---|---|---|---|---|---|
| 40 | Zoom da tela e lupa (Zoom, Magnifier) | Parcial (fator de zoom do cursor) | Pronto (`Super + Ctrl + =`, `-` e `Backspace`, até 8x) | P a M | [x] |
| 41 | Inverter cores e correção para daltonismo | Parcial (`screen_shader`) | Pronto (`Super + Ctrl + I` inverte; filtros de protanopia, deuteranopia e tritanopia na lista do shell, ainda sem tela no Hub nem validação com teste de visão de cores) | P por shader (GLSL) | [x, porem para depois] |
| 42 | Localizar o mouse, desenhar na tela (Mouse Mark), animação de clique, pontos de toque | Não | Não | M a G (precisa de eventos globais de entrada) | [Nao, quem sabe no futuro] |
| 43 | Agitar para aumentar o cursor (Shake Cursor) | Parcial (plugin dynamic-cursors) | Pronto (opção de shake) | P | [x] |
| 44 | Esconder o cursor ao digitar ou por inatividade | Nativo | Pronto | P | [x] |
| 45 | Teclas de aderência, lentas, ricochete e teclas do mouse (Sticky, Slow, Bounce, Mouse Keys) | Não | Não | G (filtro de entrada no compositor ou plugin) | [Nao] |
| 46 | Sino do sistema e integração com leitor de tela (AT-SPI) | Parcial | Não | M | [Nao sei como funciona] |
| 47 | Seletor de cor (Color Picker) | Não (ferramenta externa) | Pronto (`Super + Shift + C`) | P | [x] |
| 48 | Luz noturna (Night Light) | Parcial (hyprsunset) | Pronto | P | [x] |
| 49 | Captura de tela e gravação (portal, excluir janelas) | Nativo (portal) | Pronto (screenshot, gravador, Lens, OCR) | P | [x] |
| 50 | Depuração: mostrar FPS, áreas repintadas, estado de composição | Nativo (overlay de debug) | Não (sem interface) | P | [Nao] |
| 51 | Atalhos globais (KGlobalAccel) e atalho por janela | Nativo (binds e protocolo de atalhos globais) | Pronto (página Keybinds com conflitos) | P (atalho por janela: M) | [x] |
| 52 | Gestos de touchpad e de borda de tela | Nativo (gestos) | Parcial (Hub > Entrada: deslizar entre áreas de trabalho, pinça para zoom, deslizar para cima e para baixo com visão geral, launcher ou mostrar área de trabalho; sem teste em touchpad real; gestos de borda de tela não feitos) | P | [x] |
| 53 | Teclado virtual, método de entrada, modo tablet | Parcial | Parcial | M | [x, para depois] |
| 54 | Controle de jogo (gamepad navega a interface) | Não | Não | M | [x, para depois] |
| 55 | Detector de inatividade e inibição | Nativo | Pronto | P | [x] |
| 56 | Rotação automática por sensor (SensorProxy) | Não (daemon externo) | Não | P (daemon externo) | [nao, cogito depois] |

## F. Plataforma e extensibilidade

| # | Recurso do KWin | Hyprland | Ryoku hoje | Esforço | Quero? |
|---|---|---|---|---|---|
| 57 | Sessão X11 além da Wayland | Não (só Wayland, XWayland para aplicativos) | n/a | X | [Nao] |
| 58 | HDR, taxa variável (VRR), tearing | Nativo | Pronto (HDR, VRR por tela) | P | [x] |
| 59 | Multi-GPU e renderizador Vulkan | Parcial | Parcial | X | [nao, repensar depois] |
| 60 | Scripts do KWin (JS e QML, efeitos com script) | Lua com eventos (Nativo) | Pronto (`user.lua`, plugins do shell) | G (camada de compatibilidade com scripts KWin), X (portar tal como são) | [nao sei quais sao esses scripts e efeitos] |
| 61 | Os 13 módulos de configuração (KCM): animações, decoração, desktops, efeitos, comportamento das janelas, regras, bordas, scripts, alternador, teclado virtual, XWayland | Não (sem interface) | Parcial (Hub cobre animações, decoração, bordas, comportamento, regras, alternador, atalhos, gerenciador de janelas e XWayland; faltam scripts do KWin, que não se aplicam) | M por módulo | [x] |
| 62 | API D-Bus do KWin que apps do KDE usam (Spectacle, KRunner, widgets do Plasma) | Não | Não | G, provavelmente X | [Nao] |

## O que não vale a pena (resumo)

- **Sessão X11, API D-Bus do KWin, atividades do KDE e Fall Apart:** o custo é alto e o valor baixo, ou o conceito não existe no Hyprland.
- **Compatibilidade com scripts do KWin:** só faz sentido se existir um script específico que você queira (por exemplo, o Krohnkite do caelestia-kde, que no Hyprland já é substituído pelo ladrilhamento nativo).

## Rodada 1 (feita em 20/10/2026, branch `caelestia-port`)

Itens que passaram para Pronto: 5 (encaixe topo e base, mais a distância de encaixe), 17 (maximizada sem borda), 23 (identificar telas), 25 (mostrar a área de trabalho), 40 (zoom) e 41 (inverter e daltonismo). O 33 ganhou o minimizar e restaurar de verdade; falta só o efeito. Tudo foi testado na VM com um monitor; niri e Wayfire marcam as teclas novas como não suportadas.

Pendências desta rodada: filtros de daltonismo sem tela no Hub e sem validação; zoom e inverter voltam ao padrão quando o Hyprland recarrega; nada testado com mais de um monitor. O item 4 (maximizar numa direção só) ficou para depois.

## Rodadas 2 e 3 (média e grande, feitas em 20/10/2026)

- **Pronto:** 7 (encaixe ao soltar na borda) e 26 (Alt+Tab com miniaturas).
- **Parcial:** 12 (menu da janela no `Alt + F3`, porque `Alt + Espaço` já abre o Rashin) e 24 (cantos ativos).
- **Melhorado, continua Parcial:** 1 (regras de janela ganharam "De uma janela").
- **Não resolvido:** 13 (aviso de app travado). O Hyprland já tem o aviso nativo e ligado, mas ele não apareceu no teste.
- **Para depois, como marcado:** 6 (zonas de encaixe e editor).
- **Pendências:** (resolvido e validado em 21/10/2026: arrastar para fora de uma metade devolve o tamanho original) O encaixe na borda usa um timer Lua de 60 ms, porque o Lua do Hyprland não vê o fim do arrasto pela barra de título. Nada foi testado com mais de um monitor. niri e Wayfire mantêm o Alt+Tab antigo.

## Sugestão de ordem (custo e benefício)

1. **Barato e de alto valor (P):** completar o encaixe rápido (topo e base), maximizar só em uma direção (via Lua), janela maximizada sem borda, zoom da tela, shaders de inverter e de daltonismo, identificar telas.
2. **Médio (M):** menu da janela (Alt+Espaço), aviso de aplicativo travado, cantos ativos de tela, mostrar a área de trabalho, política de posicionamento inteligente, interface para as regras de janela que o Hyprland já suporta.
3. **Grande (G):** Alt+Tab com miniaturas, zonas de encaixe com editor, encaixe por arraste na borda, botões e temas da barra de título, teclas de acessibilidade.

## Limites deste levantamento

- Os esforços são estimativas minhas, não medições.
- Marquei como `Parcial` ou `Não` o que não encontrei no Ryoku por busca no código; pode existir sob outro nome em partes que eu não li (os plugins do shell vindos da loja, por exemplo).
- As colunas do Hyprland vêm da documentação e dos testes na versão 0.56.2; ele muda rápido.
