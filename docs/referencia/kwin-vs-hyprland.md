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
| Ryoku: Pronto | 22 |
| Ryoku: Parcial | 17 |
| Ryoku: Não | 22 |
| Ryoku: não se aplica (sessão X11) | 1 |
| Hyprland: Nativo (inclui os scripts em Lua) | 20 |
| Hyprland: Parcial | 16 |
| Hyprland: Não | 25 |
| Hyprland: não se aplica | 1 |
| Esforço só P ou M (Lua e Quickshell) | 48 |
| Esforço inclui G (plugin C++ ou protocolo), sem X | 7 |
| Esforço inclui X (inviável ou sem sentido, inclusive "provavelmente X") | 7 |

## A. Janelas e regras

| # | Recurso do KWin | Hyprland | Ryoku hoje | Esforço | Quero? |
|---|---|---|---|---|---|
| 1 | Regras por aplicativo (34 propriedades). O Hyprland cobre cerca de 25: posição, tamanho, flutuar, maximizar, tela cheia, workspace, monitor, opacidade, sem borda, tamanho mínimo e máximo, VRR e tearing, foco, captura de tela, tags | Nativo | Parcial (páginas Window Rules, App Overrides e Layer Rules, uma ação por regra) | P (interface) | [ ] |
| 2 | Regras que o Hyprland não tem: pular barra de tarefas, pager e alternador, camada acima ou abaixo, atividade, atalho por janela, maximizar só na horizontal ou vertical | Não | Não | M (algumas), X (atividade) | [ ] |
| 3 | Manter acima e abaixo (camadas entre janelas flutuantes) | Parcial (`alter_zorder`) | Não | M | [ ] |
| 4 | Maximizar só na horizontal ou só na vertical | Não | Não | M | [ ] |
| 5 | Encaixe rápido: metades e cantos (Quick Tile) | Não | Parcial (nosso `Super + setas`: metades, cantos, maximizar, restaurar) | P (completar topo e base) | [ ] |
| 6 | Zonas de encaixe customizáveis e editor de ladrilhos (Tiles Editor) | Não | Não | G | [ ] |
| 7 | Encaixe ao arrastar para a borda da tela (maximizar ou ladrilhar, "electric borders") | Não | Não | M a G | [ ] |
| 8 | Imã de bordas e janelas (border, window e center snap) | Nativo (`general.snap`) | Pronto | P | [ ] |
| 9 | Políticas de posicionamento: inteligente (menos sobreposição), cascata, centrada, aleatória, canto, sob o mouse | Parcial (`center`, `move`, expressões com o cursor) | Parcial (centrada e cascata no layout flutuante) | M (inteligente) | [ ] |
| 10 | Política de foco: clique, seguir o mouse, prevenção de roubo de foco, elevar ao passar com atraso (auto-raise) | Parcial (sem auto-raise) | Parcial (seguir o mouse no Hub) | P a M | [ ] |
| 11 | Ações do mouse (Alt+arrastar mover e redimensionar, roda na barra de título) | Nativo (`bindm`) | Parcial (atalhos padrão) | P | [ ] |
| 12 | Menu da janela (Alt+Espaço: mover, redimensionar, manter acima, sem borda, criar regra) | Não | Não | M | [ ] |
| 13 | Aviso "o aplicativo não está respondendo" (frozenapp e killprompt) | Não | Não | M | [ ] |
| 14 | Excluir janela da captura de tela e da gravação | Nativo (`no_screen_share`) | Parcial (só por regra) | P | [ ] |
| 15 | Mover, redimensionar e empacotar pelo teclado (Pack left, Grow, Shrink) | Parcial | Parcial (modo resize) | P | [ ] |
| 16 | Decorações de janela (KDecoration, temas Aurorae, botões à esquerda ou direita, duplo clique) | Parcial (plugin hyprbars) | Parcial (nosso hyprbars com 3 botões, sem temas) | M (botões configuráveis), G (temas) | [ ] |
| 17 | Janela maximizada sem borda | Nativo (regra) | Não | P | [ ] |
| 18 | Agrupar janelas em abas | Nativo (groups) | Pronto | P | [ ] |

## B. Áreas de trabalho, atividades e telas

| # | Recurso do KWin | Hyprland | Ryoku hoje | Esforço | Quero? |
|---|---|---|---|---|---|
| 19 | Áreas de trabalho virtuais (grade, linhas, por tela, rolar ao fim, aviso na tela) | Parcial (numeradas, por monitor; sem grade 2D) | Pronto (workspaces, reordenar, roda do mouse) | M (grade 2D) | [ ] |
| 20 | Atividades (Activities do KDE) | Não | Não | G, provavelmente X | [ ] |
| 21 | Várias telas (foco, mover janela entre telas) | Nativo | Pronto | P | [ ] |
| 22 | Layout de telas (resolução, escala, HDR, espelhar, perfis) | Nativo | Pronto (página Displays com perfis) | P | [ ] |
| 23 | Identificar telas (Output Locator) | Não | Não | P | [ ] |
| 24 | Cantos e bordas ativas (ações ao encostar o cursor: visão geral, mostrar área de trabalho, travar) | Não | Não | M (Quickshell com hotspots) | [ ] |
| 25 | Mostrar a área de trabalho (Show Desktop) | Não | Parcial (esconder janela e scratchpad) | P | [ ] |

## C. Troca e visão geral de janelas

| # | Recurso do KWin | Hyprland | Ryoku hoje | Esforço | Quero? |
|---|---|---|---|---|---|
| 26 | Alt+Tab com miniaturas (grade, só o aplicativo atual, modos por área, minimizadas, multitela) | Parcial (alternar, última janela) | Parcial (Alt+Tab só vai à última janela) | M a G (miniaturas por captura de janela) | [ ] |
| 27 | Visão geral e Present Windows e grade de desktops | Não (plugin externo) | Pronto (Overview, "Overview by desktop") | P | [ ] |
| 28 | Thumbnail Aside e Highlight Window (destacar ao passar na barra de tarefas) | Não | Não | M | [ ] |
| 29 | Escurecer janelas inativas, escurecer a tela | Nativo (`dim_inactive`) | Pronto | P | [ ] |

## D. Efeitos visuais

| # | Recurso do KWin | Hyprland | Ryoku hoje | Esforço | Quero? |
|---|---|---|---|---|---|
| 30 | Blur atrás de janelas translúcidas | Nativo | Pronto | P | [ ] |
| 31 | Translucência e opacidade por regra | Nativo | Pronto | P | [ ] |
| 32 | Animações de abrir e fechar (Fade, Scale, Glide, Squash, Sheet) | Parcial (popin, slide, fade) | Pronto (página Animations com presets e curvas) | M (Glide, Squash, Sheet) | [ ] |
| 33 | Minimizar com efeito (Magic Lamp, Window Aperture, Squash) | Não (não há minimizar nativo) | Parcial (minimizar para um workspace especial) | G | [ ] |
| 34 | Janelas "gelatina" (Wobbly Windows) | Parcial | Pronto (ajuste de wobble na página Appearance) | P | [ ] |
| 35 | Fall Apart (janela se desfaz em pedaços) | Não | Não | G, provavelmente X | [ ] |
| 36 | Deslizar entre áreas de trabalho | Nativo | Pronto | P | [ ] |
| 37 | Transições suaves de mudança de tema (Blend Changes, Fade Desktop) | Não | Parcial (capa de recarga do shell) | M | [ ] |
| 38 | Popups e notificações que deslizam | n/a (é do shell) | Pronto (Quickshell) | P | [ ] |
| 39 | Retorno visual ao abrir aplicativo (Startup Feedback) | Não | Não | P | [ ] |

## E. Acessibilidade, entrada e ferramentas

| # | Recurso do KWin | Hyprland | Ryoku hoje | Esforço | Quero? |
|---|---|---|---|---|---|
| 40 | Zoom da tela e lupa (Zoom, Magnifier) | Parcial (fator de zoom do cursor) | Não (nada exposto) | P a M | [ ] |
| 41 | Inverter cores e correção para daltonismo | Parcial (`screen_shader`) | Parcial (infraestrutura de shader existe; os shaders atuais são bone, grain, halftone, onebit e vignette, sem inverter nem daltonismo) | P por shader (GLSL) | [ ] |
| 42 | Localizar o mouse, desenhar na tela (Mouse Mark), animação de clique, pontos de toque | Não | Não | M a G (precisa de eventos globais de entrada) | [ ] |
| 43 | Agitar para aumentar o cursor (Shake Cursor) | Parcial (plugin dynamic-cursors) | Pronto (opção de shake) | P | [ ] |
| 44 | Esconder o cursor ao digitar ou por inatividade | Nativo | Pronto | P | [ ] |
| 45 | Teclas de aderência, lentas, ricochete e teclas do mouse (Sticky, Slow, Bounce, Mouse Keys) | Não | Não | G (filtro de entrada no compositor ou plugin) | [ ] |
| 46 | Sino do sistema e integração com leitor de tela (AT-SPI) | Parcial | Não | M | [ ] |
| 47 | Seletor de cor (Color Picker) | Não (ferramenta externa) | Pronto (`Super + Shift + C`) | P | [ ] |
| 48 | Luz noturna (Night Light) | Parcial (hyprsunset) | Pronto | P | [ ] |
| 49 | Captura de tela e gravação (portal, excluir janelas) | Nativo (portal) | Pronto (screenshot, gravador, Lens, OCR) | P | [ ] |
| 50 | Depuração: mostrar FPS, áreas repintadas, estado de composição | Nativo (overlay de debug) | Não (sem interface) | P | [ ] |
| 51 | Atalhos globais (KGlobalAccel) e atalho por janela | Nativo (binds e protocolo de atalhos globais) | Pronto (página Keybinds com conflitos) | P (atalho por janela: M) | [ ] |
| 52 | Gestos de touchpad e de borda de tela | Nativo (gestos) | Parcial (plugin hyprgrass opcional) | P | [ ] |
| 53 | Teclado virtual, método de entrada, modo tablet | Parcial | Parcial | M | [ ] |
| 54 | Controle de jogo (gamepad navega a interface) | Não | Não | M | [ ] |
| 55 | Detector de inatividade e inibição | Nativo | Pronto | P | [ ] |
| 56 | Rotação automática por sensor (SensorProxy) | Não (daemon externo) | Não | P (daemon externo) | [ ] |

## F. Plataforma e extensibilidade

| # | Recurso do KWin | Hyprland | Ryoku hoje | Esforço | Quero? |
|---|---|---|---|---|---|
| 57 | Sessão X11 além da Wayland | Não (só Wayland, XWayland para aplicativos) | n/a | X | [ ] |
| 58 | HDR, taxa variável (VRR), tearing | Nativo | Pronto (HDR, VRR por tela) | P | [ ] |
| 59 | Multi-GPU e renderizador Vulkan | Parcial | Parcial | X | [ ] |
| 60 | Scripts do KWin (JS e QML, efeitos com script) | Lua com eventos (Nativo) | Pronto (`user.lua`, plugins do shell) | G (camada de compatibilidade com scripts KWin), X (portar tal como são) | [ ] |
| 61 | Os 13 módulos de configuração (KCM): animações, decoração, desktops, efeitos, comportamento das janelas, regras, bordas, scripts, alternador, teclado virtual, XWayland | Não (sem interface) | Parcial (Hub cobre regras, atalhos, animações, gerenciador de janelas) | M por módulo | [ ] |
| 62 | API D-Bus do KWin que apps do KDE usam (Spectacle, KRunner, widgets do Plasma) | Não | Não | G, provavelmente X | [ ] |

## O que não vale a pena (resumo)

- **Sessão X11, API D-Bus do KWin, atividades do KDE e Fall Apart:** o custo é alto e o valor baixo, ou o conceito não existe no Hyprland.
- **Compatibilidade com scripts do KWin:** só faz sentido se existir um script específico que você queira (por exemplo, o Krohnkite do caelestia-kde, que no Hyprland já é substituído pelo ladrilhamento nativo).

## Sugestão de ordem (custo e benefício)

1. **Barato e de alto valor (P):** completar o encaixe rápido (topo e base), maximizar só em uma direção (via Lua), janela maximizada sem borda, zoom da tela, shaders de inverter e de daltonismo, identificar telas.
2. **Médio (M):** menu da janela (Alt+Espaço), aviso de aplicativo travado, cantos ativos de tela, mostrar a área de trabalho, política de posicionamento inteligente, interface para as regras de janela que o Hyprland já suporta.
3. **Grande (G):** Alt+Tab com miniaturas, zonas de encaixe com editor, encaixe por arraste na borda, botões e temas da barra de título, teclas de acessibilidade.

## Limites deste levantamento

- Os esforços são estimativas minhas, não medições.
- Marquei como `Parcial` ou `Não` o que não encontrei no Ryoku por busca no código; pode existir sob outro nome em partes que eu não li (os plugins do shell vindos da loja, por exemplo).
- As colunas do Hyprland vêm da documentação e dos testes na versão 0.56.2; ele muda rápido.
