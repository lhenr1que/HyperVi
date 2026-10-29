# Briefing visual — Mockups da interface oficial da suíte Arch

**Versão:** 1.0  
**Data:** 28/10/2026  
**Estado:** briefing para concepção/validação, **não imagens finais nem código**.  
**Fonte de decisões:** `ESPECIFICACAO_INTERFACE_SUITE_v3.9.md` e `DESIGN_SYSTEM_INTERFACE_OFICIAL_v1.7.md`.  
**Nome:** ainda não decidido; não usar “Kwinish” como marca definitiva.

## 1. Objetivo

Criar uma família coerente de mockups que demonstre a experiência do **rice oficial** para Hyprland, sem alterar requisitos funcionais já aprovados. As superfícies do rice oficial podem usar materiais adaptativos; o Settings fundamental é um aplicativo separado e existente em Rust/GTK4, não um componente a reimplementar sem auditoria.

## 2. Restrições já aprovadas

| Referência | Regra |
|---|---|
| D-11 | Layout de instalação com **barra inferior contínua + dock integrada** |
| D-29 | **Barra modular opcional**, três ilhas ancoradas esquerda/centro/direita, reordenação **dentro** das ilhas |
| D-02/D-03 | Dock desacoplável e ocultação inteligente; sem duplicação |
| D-13/D-18 | Dock: favoritos globais + janelas do workspace atual; agrupamento configurável; desagrupado com favoritos e tarefas separados |
| D-14/D-15 | Clique em janela única no Floating minimiza/restaura/foca; favorito ativo só em outro workspace solicita nova janela aqui |
| D-17 | Pré-visualização em lista ou miniaturas quando tecnicamente suportadas |
| D-19–D-22 | Launcher **híbrido Split**, categorias à esquerda, pesquisa global, centralizado por padrão e ancorado opcional |
| D-23 | Settings com sidebar adaptativa para rice oficial versus externo |
| D-28/D-33 | Direção visual **translúcido contido**, com **transparência leve** e leitura prioritária |
| D-30–D-32 | Tema/acento por wallpaper do monitor principal, variante global, escolhas manuais permitidas |
| D-34 | **Fonte configurada no sistema**, sem família única obrigatória |
| D-35 | **Família visual consistente de pictogramas lineares para controles do rice oficial**, preservando ícones originais dos aplicativos |
| F-01 | Paleta independente por monitor: **ideia futura**, não incluir como requisito de primeira entrega |

## 3. Conjunto de telas a produzir

### M-01 — Desktop base / barra contínua

- Wallpaper escuro de exemplo; barra inferior única e discreta.
- À esquerda: botão do launcher + workspaces (número e símbolo do modo).
- Centro: dock com favoritos fixos e aplicativos do workspace ativo.
- À direita: conectividade, áudio, relógio e acessos opcionais a monitores/Settings.
- Demonstrar estados reais de foco e execução, sem inserir um título central obrigatório da janela.

### M-02 — Desktop / barra modular com três ilhas

- Mesmo conteúdo e wallpaper do M-01, alterando **somente o preset visual**.
- Esquerda: navegação. Centro: dock. Direita: status. Blocos não se tocam; sem conflitos de área.
- Segundo frame com dock desacoplada e **ilha central ausente** (não replicar tarefas).

### M-03 — Estados da dock e seletor de janelas

- Favorito fechado; janela local em segundo plano; janela local focada; minimizada; aplicativo com três janelas; marcador de atenção.
- Duas configurações: agrupamento ativo e desativado; no modo desagrupado o favorito é separado das janelas.
- Painel de prévias em **lista** e uma alternativa com **miniaturas** ilustrativas; deixar claro que miniaturas reais dependem de teste Wayland.
- Clique sobre janela focada no Floating minimiza; clique sobre minimizada restaura.

### M-04 — Launcher híbrido Split

- Estado vazio com categorias à esquerda, aplicativos à direita, pesquisa no topo.
- Estado pesquisando ações e aplicativos sem filtro da categoria ativa.
- Versão centralizada e versão ancorada à barra; popovers internos aos limites da tela.

### M-05 — Settings fundamental / sidebar adaptativa

- Página Janelas ou Workspaces; manter estética GTK4 plausível.
- Alternância entre rice oficial (Barra/Dock/Launcher/Aparência) e rice externo (`Rice ativo`).
- Mostrar feedback real de Aplicado / Salvo / Falha, sem controles falsamente funcionais.

### M-06 — Tema claro adaptado ao wallpaper

- Mesma estrutura do M-01 e M-04 em tema claro e acento diferente.
- Demonstrar fonte do sistema com métricas distintas e transparência leve.

## 4. Componentes e estados que deverão ter zooms de detalhe

**Componentes:** pílula de workspace, ícone de dock, indicador de execução/foco, marcador de minimização, grupo e contagem, popover de janelas, campo de busca, item de categoria, item do Settings, toggle e mensagem de diagnóstico.

**Estados:** normal, hover, focado por teclado, pressionado, ativo, minimizado, múltiplas janelas, atenção, indisponível, erro; não basear a diferenciação exclusivamente na cor.

## 5. Especificações de apresentação para protótipo

- Começar com quadro **1920×1080**, trabalhando em **pixels lógicos** nos componentes; testar 2560×1440 e 3840×2160 com escala, sem assumir que 1 px do mockup equivale a 1 pixel físico.
- Barra: 44–48 px lógicos como referência **não aprovada**; dock integrada: 22–26 px de arte do ícone; altura clicável maior.
- Superfícies de barra/dock com **94–97% de opacidade como direção inicial**, a calibrar; launcher, menus e Settings mais opacos.
- Fonte **do sistema** e fallback sans-serif; por ora, usar uma família temporária apenas como representação, rotulada como tal.
- Modos Tiling/Scroll/Floating com ícones distintos ao lado dos números.
- Paletas de wallpaper ilustrativas; todas as combinações de texto/estado precisam ser verificadas quanto a contraste.

## 6. Movimento para protótipos interativos

- Hover 90–140 ms; menu 140–200 ms; launcher 180–240 ms; smart-hide 160–220 ms; troca contínua/modular 180–260 ms.
- Tempos **não aprovados como especificação final**; usar para comparar sensações de UI.
- Comparar com versão de movimento reduzido e sem blur. Nunca simular revelação por hover durante fullscreen de jogo.

## 7. Critérios de aceite visual

1. As telas preservam as escolhas D-01 a D-34, sem criar novos módulos de rice de terceiros nem dashboard.
2. O usuário reconhece sem explicação longa a distinção entre Tiling, Scroll, Floating, janela ativa, minimizada e grupo.
3. Barra modular e barra contínua partilham **os mesmos dados e as mesmas interações**.
4. As ilhas não colidem; dock desacoplada não aparece duplicada na barra.
5. UI permanece legível com wallpaper saturado, fonte larga e escala elevada.
6. Animações e sinais visuais jamais apresentam sucesso antes da confirmação de estado.
7. O visual final ainda será aprovado pelo usuário após mockups; não usar os diagramas anteriores como identidade congelada.

## 8. Próximas decisões pendentes

- **Biblioteca concreta dos pictogramas de sistema:** a família **visual outline consistente** foi aprovada (D-35), mas Lucide/GTK simbólico etc. ainda serão avaliadas.
- Forma final dos indicadores (ponto, traço, contagem, atenção) após comparação lado a lado.
- Curvas e durações finais do movimento.
- Tamanhos, espaçamento, raio e material por superfície após prototipagem real.
- Formato dos mockups finais: desktop, comparação claro/escuro, painéis de detalhe e futuras telas responsivas.


## 9. Primeira prancha gerada — 28/10/2026

**Arquivo:** `MOCKUPS_COMPARACAO_BARRAS_v0.1.png`. Esta imagem é um **estudo inicial**, não um mockup aceito para implementação.

| Cena | Entregue no estudo | Refinamento seguinte |
|---|---|---|
| M-01 | Barra contínua inferior em wallpaper roxo noturno | Janela Floating visível e workspace com cada pictograma de modo |
| M-02 | Três ilhas ancoradas sobre o mesmo wallpaper | Tamanhos consistentes, indicadores legíveis, janelas visíveis |
| M-02b | Exemplo reduzido com dock lateral | Refinar distâncias e ausência explícita da ilha central |
| Estados | Referências de aberto/focado/minimizado/grupo/atenção | Validar linguagem final, tamanho real e acessibilidade |
| Temas | Pequena comparação claro/escuro ilustrativa | Versão completa com wallpaper claro e cores derivadas |

A prancha não adiciona dashboard, não introduz busca permanente em tela e não obriga a mesma aparência para rices externos. A organização das três ilhas respeita D-29; a família visual de controles respeita D-35.


## 10. Segunda prancha — mockups refinados v0.2

A nova prancha apresenta **desktop com janelas visíveis** e comparativo de barra contínua vs. três ilhas sobre cenário equivalente, mais estudos exploratórios de dock, launcher e Settings. Ver `MOCKUPS_REFINADOS_DESKTOP_v0.2.md`.

**Correções de fidelidade obrigatórias antes de aprovar a UI:**
- Mostrar `número + modo` em cada pílula e o **modo Floating do workspace ativo** enquanto as janelas estiverem sobrepostas.
- Tornar explícita a diferença entre status focado, aberto e minimizado; os estados vêm do compositor, não do último clique.
- Na versão dock desacoplada, retirar ilha central. Nas versões Tiling/Scroll, documentar a política de ocultação inteligente sem esconder obrigatoriamente toda a barra.
- No launcher Split, **pesquisa de arquivos e itens recentes são apenas ilustrações**, não recursos ratificados para o MVP.
- Validar a aparência do Settings contra o aplicativo Rust/GTK4 existente.

**Próxima sequência proposta:** (1) corrigir as pílulas em 1080p; (2) gerar cenários Tiling e Scroll com mesmas janelas; (3) consolidar dock separada; (4) detalhar o launcher e Settings sem extrapolar requisitos aprovados.


## Adendo visual M-06/M-07/M-08 — dois comandos novos de navegação

- **M-06 — Visão Geral de Todos:** cards em grade de workspaces com número, modo Tiling/Scroll/Floating, monitor e janelas quando representáveis; workspace ativo em destaque. Mesmo wallpaper e tokens atuais.
- **M-07 — Visão Geral do workspace Atual:** visão simultânea das janelas locais, com títulos, estado de minimização e opção de retornar; Scroll inclui janelas fora da área de rolagem.
- **M-08 — Mostrar área de trabalho:** comparar antes/depois no mesmo workspace Floating, evidenciando que todas as janelas foram temporariamente ocultas e retornam ao estado anterior.
- Em M-01/M-02: adicionar *Visão Geral* próximo às pílulas de workspaces e *Mostrar área de trabalho* na região de sistema, sem confundir esses comandos com launcher ou Monitores.
- Não desenhar dashboard; não criar tela de busca permanente; não apresentar interfaces como implementadas.


## 12. Prancha v0.1 — inclusão de Visão Geral e Mostrar área de trabalho

**Decisão aprovada:** D-38 — a Visão Geral abre inicialmente em **Todos os workspaces**, com alternativa de navegar para o workspace atual. D-36/D-37 permanecem demandas do produto.

**Cenas:**

1. Desktop Floating em barra contínua e em três ilhas modulares, com **janelas comparáveis** e ícones Visão Geral (navegação) e Mostrar área de trabalho (sistema).
2. Visão Geral de todos os workspaces em grade (exemplo: Tiling, Scroll, Floating, Vazio); cada cartão com identificação, modo e indicações de janelas.
3. Nível Workspace atual com mosaico de janelas e estados distintos, sem alterar o layout ativo.
4. Mostrar área de trabalho: wallpaper visível enquanto as janelas estão temporariamente ocultas; barra e dock mantêm o estado correto dos apps.
5. Close-ups de estados dos botões e pílulas com número + modo de cada workspace.

**Não inferir a partir da arte:** atalhos exibidos, render de miniaturas, agrupamento numérico, bibliotecas usadas, ícones em pixels definitivos, capacidades de captura, posição exata, implementação de minimização ou fluxo de jogos fullscreen.

**Arquivo principal:** `MOCKUP_VISUALIZACAO_E_AREA_DE_TRABALHO_v0.1.png`. Prancha suplementar: `MOCKUP_ESTUDOS_VISUALIZACAO_DETALHES_v0.1.png`.


---

## Refinamento P-12 — cenas de navegação da Visão Geral (proposta)

Produzir próximos estudos separando os estados: (1) Todos com workspace 2 em hover sem ativar; (2) detalhe de workspace 2 com botão Voltar e miniaturas; (3) clique explícito em Ir para workspace; (4) Esc de detalhe para Todos; (5) seleção de janela minimizada no Floating; (6) cartão vazio com fallback sem captura.

Obrigatório: **mesmo wallpaper, mesmas janelas e mesmo modo em todas as cenas comparáveis**; workspaces exibem número + pictograma; distinguir visualmente Visão Geral de Show Desktop e do launcher; usar superfície quase opaca, tipografia do sistema e acento derivado do wallpaper. O design não deve alegar que o backend já funciona.

**Status:** D-38 aprovada; P-12 aguardando validação.


---

## M-09 — Visão Geral multimonitor (brief v0.7)

**Objetivo:** complementar M-06/M-07 com dois monitores, sem modificar D-38/D-39.

**Cena comparativa proposta:** dois monitores A/B, com 3 workspaces no A e 2 no B; uma superfície de Visão Geral mostra grupos A/B; cada cartão expõe número, ícone do modo, estado e botão separado `Ir para workspace`. A primeira abertura é em Todos. Um segundo frame aplica filtro somente no monitor B; clique num cartão de B mostra detalhe sem trocar monitor; ativação explícita faz o foco mudar ao monitor/workspace real.

**Cenas de exceção:** B desconectado durante overview, escala mista, workspace vazio, janela minimizada de B, captura indisponível, layout Scroll fora do viewport.

**Invariantes:** wallpaper e tema global iguais nas cenas comparadas; usar superfície clara de contraste acessível sobre wallpaper escuro, sem modificar geometria ou dispositivos reais; não desenhar busca permanente ou dashboard; não atribuir recursos do VXWM ao MVP; não sugerir que arrastar janelas entre monitores já está aprovado.

**Decisão pendente:** exibir todos os workspaces agrupados ou iniciar filtrado pelo monitor em foco; não rotular hipótese como D-40.


---

## M-09 v0.1 — D-40 e localização da Visão Geral (comparação)

**Decisão aprovada:** D-40, organização global com grupos de workspaces por monitor e filtros opcionais. **Não está aprovado** se a UI será exibida em uma superfície no monitor em foco ou distribuída por múltiplos monitores.

**Cena A (P-14):** dois displays físicos lado a lado; Visão Geral aparece somente no monitor em foco, contendo seções A (workspaces 1–3) e B (4–5); display secundário continua normal. Inclui filtros Todos/A/B, botão `Ir` separado do cartão e cabeçalhos por monitor.

**Cena B (P-15):** os mesmos displays com Visão Geral local em ambos; monitor A mostra 1–3 e B mostra 4–5. A UI distribuída precisa manter acesso visível ao conjunto e aos filtros, sem duplicar automaticamente o controle de teclado.

**Detalhes:** cards exibem número+ícone de modo+miniaturas conceituais, grupo de workspaces B inclui um vazio, barra/launcher mantêm decisões anteriores. Não inventar pesquisa persistente, dashboard, tema por monitor ou funcionalidade de drag/drop aprovada.

**Entrega:** `MOCKUP_M09_COMPARACAO_DISPOSICAO_MULTIMONITOR_v0.1.png` + recortes A e B. **Arte exploratória, não especificação final de pixels, tempo de captura, UX de teclado nem requisitos de tecnologia.**


---

## M-10 — D-41 e transições de foco (brief de próxima rodada)

Quadros a criar quando solicitados: (1) Visão Geral em A com grupos A+B e B sem overlay; (2) filtro B sem troca de foco real; (3) detalhe B5 dentro do overlay de A; (4) clique explícito em `Ir` transferindo foco para B5; (5) `Esc` sem ativação devolvendo foco anterior; (6) Mostrar área de trabalho local de A com B inalterado versus operação global, apenas como alternativas para D-42.

**Sinalização visual obrigatória:** usar tratamentos diferentes para `monitor hospedeiro`, `workspace explorado`, `workspace efetivamente ativo`, `janela focada`, `janelas temporariamente ocultas`. Sem dashboard, sem mover janelas automaticamente, sem substituir os ícones de apps. Utilizar o mockup M-09 A como referência de composição, mas seguir a D-41 em caso de divergência artística.


---

## M-10 — D-42 aprovada: foco, revelação local e restauração (brief consolidado)

**Objetivo da próxima rodada de mockups:** demonstrar, sem inventar funções novas, que a Visão Geral e Mostrar área de trabalho coexistem com múltiplos monitores e com a organização de janelas Tiling/Scroll/Floating.

**Quadros obrigatórios (mesmo wallpaper, configuração e aplicativos em todas as comparações):**

1. **Estado inicial:** A2 (Floating) e B5 (Tiling), janelas abertas em ambos; barra com botões de Visão Geral e Mostrar área de trabalho.
2. **D-42 acionada em A:** A2 temporariamente revelado; B5 idêntico ao quadro anterior. Botão local indica estado ativo. Dock não trata janelas ocultas como minimizadas.
3. **Visão Geral em A:** superfície única mostrando cartões agrupados A+B; cartões de A2 continuam listando janelas temporariamente ocultas.
4. **Inspeção de B5:** dentro do overlay de A, detalhamento de B5, com marcador de exploração distinto do foco real.
5. **Ir B5:** foco transferido explicitamente a B5 no monitor B, sem deslocar janelas entre telas.
6. **Restauração em A2:** segundo acionamento local devolve janelas anteriores à apresentação correta, preservando janela já minimizada (não deve reaparecer).
7. **Estados extremos:** monitor B desconectado, captura de miniatura indisponível, janela fechada no meio do fluxo, Scroll com janelas fora do viewport, fullscreen real.

**UI:** manter barra contínua inicial, modo modular de três ilhas opcional, transparência leve, tema global pelo wallpaper principal, fonte configurada no sistema, ícones lineares para comandos e originais para apps. Sem dashboard, VXWM, busca de arquivos no MVP ou pretensa interface distribuída entre telas.

**Aviso de rastreabilidade:** item de saída automática do escopo Show Desktop é **P-17.2 proposta**, não decisão. Animações, tipos de miniatura e atalhos indicados na arte não são necessariamente definitivos.


---

## Entrega M-10 — Fluxo de quatro estados e extensões

**Produzido em 28/10/2026.** Duas pranchas geradas como estudos: `MOCKUP_M10_FLUXO_E_RESTAURACAO_v0.1.png` (seis cenas, inclui restaurar) e `MOCKUP_M10_COMPARACAO_4_QUADROS_v0.1.png` (quatro cenas de interação, incluindo detalhe remoto).

### Revisões obrigatórias antes de homologar arte

1. Conservar a mesma identidade de janelas, geometria e ordem Z de A2 e B5 em todas as cenas comparáveis; o modelo gerativo pode ter variado esses elementos.
2. Garantir que a janela **previamente minimizada** em A2 permaneça minimizada após restaurar o desktop; não mostrar sua reaparição.
3. Identificar de forma visível o **monitor hospedeiro do overlay** e o **monitor cujo workspace é explorado** sem transferir foco.
4. Diferenciar `workspace selecionado na UI`, `workspace realmente ativo` e `janela efetivamente focada`.
5. Na barra modular, preservar as três ilhas e o botão de Mostrar área de trabalho na ilha direita.
6. Evitar qualquer aparência de dashboard, widgets persistentes de busca, VXWM em produção ou miniaturas reais garantidas.

P-17.2 não é decisão; nenhuma das transições pode ser tomada como regra nova sem validação.
