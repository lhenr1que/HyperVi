# Especificação exploratória — Área de trabalho e Visão Geral

**Versão:** 0.8  
**Data:** 28/10/2026  
**D-39 aprovada:** clique no cartão abre detalhe; botão separado ativa workspace.
**D-38 aprovada:** Visão Geral abre inicialmente em **Todos os workspaces**. O nível Atual permanece acessível.  
**Produto:** nome ainda em aberto  
**Status vigente:** D-36 a D-42 aprovadas. **D-42:** Mostrar área de trabalho somente no workspace ativo do monitor em foco. As formulações “pendente D-42” preservadas nas seções históricas abaixo foram superadas pela **seção 14**. Regras finas de transição e mecanismo de captura permanecem propostas.

## 1. Objetivo e terminologia

| Recurso | O que é | O que não é |
|---|---|---|
| Mostrar área de trabalho | Revelar temporariamente o wallpaper, removendo janelas da visualização e permitindo restaurá-las | Minimizar em massa como estado definitivo; trocar de workspace; fechar janelas |
| Visão Geral — Atual | Selecionar visualmente uma janela do workspace em foco, inclusive as minimizadas e as fora do viewport Scroll | Launcher de novos aplicativos; Alt+Tab apenas por ordem de foco |
| Visão Geral — Todos | Comparar workspaces, identificar modos e navegar entre eles | Modo VXWM/canvas infinito; dashboard |

## 2. Fluxos de uso recomendados

### Mostrar área de trabalho

1. Acionar ícone dedicado ou atalho configurável.
2. O sistema registra o workspace e estado lógico das janelas, e esconde a apresentação sem alterar os modos individualizados.
3. Exibir estado pressionado/ativado do comando, **se visível**.
4. Repetir ação para restaurar janelas elegíveis. Eventos concorrentes (abertura/fechamento/remoção monitor) precisam ter semântica explícita e registro de erro.
5. **D-42 aprovada:** em vários monitores, afetar somente o workspace ativo do monitor em foco; demais monitores não são alterados.
6. **Dock:** as janelas temporariamente ocultas continuam representadas como abertas no workspace (não como minimizadas). Ao clicar em uma tarefa, proposta é encerrar Mostrar área de trabalho e focar/restaurar apenas a janela escolhida.

### Visão Geral de Todos

1. Abrir por ícone junto aos workspaces ou atalho.
2. Exibir grade de cards: número/nome, modo, monitor e miniatura (com fallback textual), janelas/indicadores.
3. **D-39 (APROVADA):** clique simples no corpo do cartão abre o detalhamento das janelas do workspace, mantendo a Visão Geral aberta.
4. Ação separada `Ir para workspace` ativa-o e fecha a Visão Geral, mesmo se estiver vazio. Este contrato de clique foi **aprovado na D-39**.
5. `Esc`: sair sem alterar disposição das janelas.

### Visão Geral — Atual

1. Selecionar guia `Atual` na visão geral (ou chamada contextual pelo workspace).
2. Exibir simultaneamente as janelas pertencentes ao workspace; **inclui minimizadas** sinalizadas e janelas fora do viewport do layout Scroll.
3. Clique em janela: focar/restaurar janela via gerenciador; não mudar o modo.
4. `Esc`: voltar/sair sem alterar janelas.

## 3. Projeto da barra

| Layout | Visão Geral | Mostrar área de trabalho |
|---|---|---|
| Contínua | Ícone ao lado das pílulas; distinguir do Launcher | Região de sistema à direita, sem confundir com Monitores |
| Modular | Ilha esquerda | Ilha direita, reordenável na ilha |
| Dock desacoplada | Não muda | Não muda |

Separar ícones por finalidade; manter pictogramas lineares uniformes (D-35), fontes do sistema (D-34), opacidade leve (D-33), tema do wallpaper principal (D-30/D-31).

## 4. Casos-limite e invariantes

- Floating: preservar posições, tamanhos, maximização, foco e estados de minimização preexistentes.
- Tiling: não reordenar layout nem criar lacunas permanentes após Show Desktop.
- Scroll: a visão geral de janelas deve representar o conjunto de janelas do workspace, não apenas a faixa visível na tela; pan e offset são restaurados.
- Em qualquer modo: ao fechar uma janela durante a visão geral, retirar sua miniatura sem redirecionar clique a outra janela.
- Minimização especial do projeto: não usar o workspace especial como repositório de Show Desktop sem desambiguar a origem de cada janela.
- Fullscreen real: evitar interceptação acidental de bordas ou popover; ativação explícita por atalho deverá respeitar foco/segurança.
- Multi-monitor: **resolvido na D-40** — overview apresenta todos os workspaces agrupados por monitor; a localização da superfície continua pendente.
- Miniaturas: captura apenas conforme API, desempenho e permissões disponíveis; fallback textual. Evitar exposição de janelas protegidas.
- Rices externos: interface visual e overview próprios podem substituir o UI do rice oficial; ações fundamentais do gerenciador precisam continuar acessíveis via contratos sem sobreposição de shells.

## 5. Viabilidade técnica e referências

- A documentação Hyprland `Uncommon tips & tricks` apresenta script Show Desktop movendo janelas do workspace atual para um workspace especial e restaurando depois. Essa é uma referência, não implementação recomendada sem revisão, pois conflita em potencial com minimizar/restaurar e preservação de origem.
  - https://wiki.hypr.land/0.53.0/Configuring/Uncommon-tips--tricks/
- **Hyprexpo** (plugin externo para Hyprland) disponibiliza visão geral de workspaces, seleção por teclado, configuração de grade, múltiplos monitores e versões recentes citam overview do workspace ativo. É um candidato de estudo, não componente instalado ou aprovado.
  - https://hyprexpo.lol/docs/
  - https://github.com/sandwichfarm/hyprexpo/releases
- Avaliar compatibilidade real com o Hyprland, Lua, plugin C++ existente, ABI, hooks, minimização, Scroll e backend de miniaturas. Não afirmar disponibilidade no código sem auditoria.

## 6. Priorização sugerida

| Fase | Entrega | Critério mínimo |
|---|---|---|
| 1 | Botão Mostrar área de trabalho + toggle reversível | Restaurar estado e geometria em todos os três modos |
| 2 | Visão Geral — Todos | Cartões com identificação e navegação, fallback sem captura |
| 3 | Visão Geral — Atual | Mosaico navegável de janelas, incluindo minimizadas/fora viewport |
| 4, futuro | Mover janelas entre cards, gestos, transições avançadas | Medir performance e estabilidade antes |

Esta priorização **não é um roadmap aprovado** nem modifica os requisitos mínimos atuais da Rice Store.

## 7. Pendências que exigem escolha do usuário

1. **Abertura padrão da Visão Geral — RESOLVIDA (D-38):** sempre iniciar em Todos; alternância manual para Atual. Não recordar último nível por padrão.
2. Mostrar área de trabalho: ícone à direita sempre visível, ou módulo opcional? Proposta: visível por padrão, reordenável.
3. Atalhos específicos e escopo entre vários monitores.
4. Implementação de Show Desktop reversível e backend de overview (Hyprexpo adaptado/inspirado, plugin próprio ou interface externa), após investigação de código.


## 8. Decisão D-38 — comportamento aprovado e consequências

**Aprovado pelo usuário:** ao ativar Visão Geral, o primeiro nível é **Todos os workspaces**. O usuário pode entrar na Visão Geral — Workspace atual pela mesma interface. Nova abertura volta ao nível Todos.

**Fluxo de navegação visual sugerido:** `Barra → Visão Geral (Todos) → Ver janelas do workspace escolhido (Atual) → Selecionar janela → Retornar ao desktop`. A ação para expandir cartão e a semântica exata do clique ainda são propostas.

**Mostrar área de trabalho (D-36):** ícone independente de Visão Geral, próximo aos controles do sistema; toggle temporário com restauração de geometria/layout e minimização anterior. O dock deve continuar representando tarefas como abertas, não artificialmente minimizadas.

### Critérios de aceitação propostos

- `OV-001`: abrir o overview a partir da barra contínua apresenta **Todos**.
- `OV-002`: abrir a partir da ilha de navegação modular apresenta **Todos**.
- `OV-003`: alternar para Atual e fechar; ao abrir novamente, apresenta **Todos**.
- `OV-004`: cards possuem número e ícone de modo, inclusive para workspaces vazios.
- `OV-005`: ao apenas abrir e fechar, nenhum modo de workspace, geometria, ordem Scroll ou estado de minimização muda.
- `SD-001`: Mostrar área de trabalho e restaurar não muda estado lógico das janelas preexistentes.
- `SD-002`: Mostrar área de trabalho e Visão Geral permanecem comandos e símbolos distintos.
- `UX-001`: na barra modular, ambos os ícones continuam disponíveis em ilhas diferentes mesmo quando a dock está desacoplada.

**Pendentes:** atalhos, gesto, execução por monitor, infraestrutura de miniaturas, comportamento durante fullscreen, controle de estados concorrentes e método seguro de ocultação/restauração com Lua/Hyprland/C++.


---

## 9. D-39 — Navegação por cartões, foco e retorno (CONTRATO DE CLIQUE APROVADO)

**Contexto:** D-38 já define a abertura padrão em **Todos os workspaces**. O comportamento principal de clique nos cartões **foi aprovado como D-39**. O contrato principal abaixo prevalece sobre a sugestão exploratória antiga de clique direto para ativação. Detalhes secundários permanecem propostos.

### 9.1 Contrato de interação proposto

| Contexto | Entrada | Resultado | Efeitos proibidos |
|---|---|---|---|
| Visão Geral — Todos | Clicar no corpo do cartão | Abrir detalhe daquele workspace dentro da Visão Geral | Não ativar o workspace; não mudar geometria nem modo |
| Visão Geral — Todos | Acionar `Ir para workspace` no cartão | Ativar workspace escolhido e fechar Visão Geral | Não maximizar nem rearranjar janelas |
| Visão Geral — Todos | Selecionar cartão vazio | Abrir vista vazia, com ação para ativar workspace | Não criar workspace/janelas implicitamente |
| Visão Geral — Detalhe | Clicar numa miniatura de janela | Solicitar foco/restauração daquela janela e fechar Visão Geral | Não mudar layout Tiling/Scroll/Floating |
| Visão Geral — Detalhe | Clicar `Voltar` ou pressionar Esc | Voltar à grade Todos | Não alterar workspace ativo |
| Visão Geral — Todos | Pressionar Esc | Fechar Visão Geral | Não alterar janelas |
| Qualquer nível | Fechar janela enquanto a prévia está aberta | Atualizar conteúdo; invalidar referência da janela fechada | Não focar outra janela por engano |

### 9.2 Layout do cartão proposto

Cada cartão terá: **número do workspace + ícone de modo + identificação do monitor quando necessário + visualização de janelas/placeholder + contagem + ação explícita `Ir para workspace`**. Cartões com muitas janelas podem exibir composição simplificada e contagem, sem depender de captura real. A grade deve ser navegável por teclado e leitor de tela, com rótulos descritivos.

### 9.3 Teclado e acessibilidade propostos

- Setas: mover seleção entre os cartões, respeitando a geometria da grade.
- `Enter` em cartão: abrir detalhe.
- `Tab`: alcançar `Ir para workspace` e os demais controles do cartão.
- `Esc` no detalhe: voltar a Todos. `Esc` em Todos: fechar.
- Tecla para entrar diretamente em workspace **não está definida**; evitar conflitar com atalhos do Hyprland, do rice externo e do gerenciador.
- Foco e seleção devem ser comunicados com contorno e rótulo, nunca somente cor.

### 9.4 Relação com Mostrar área de trabalho — proposta

Se `Mostrar área de trabalho` estiver ativo e o usuário abrir a Visão Geral, recomenda-se desativar a revelação temporária **com restauração lógica segura** antes de montar as prévias, para não expor uma grade de janelas vazia ou imprecisa. A restauração precisa ser transacional. Uma falha deve ser informada, sem prometer estados inexistentes. Essa regra operacional ainda não foi aprovada.

No fluxo `Mostrar área de trabalho → Dock → Focar janela`, a dock não deve registrar as demais janelas como minimizadas; a transição deve terminar o estado temporário antes do foco.

### 9.5 Invariantes de integração

1. `workspace_id` e `window_id` imutáveis ao longo do evento (não usar índice de lista como identidade).
2. Revalidar existência da janela/workspace antes de ativar; evitar clique atrasado após rearranjo.
3. Diferenciar visualização (`preview`) de ação que modifica o foco real (`activate`).
4. A Visão Geral não emite comandos de mutação de geometria ao abrir, alternar guias ou voltar.
5. Preservar origem de janela minimizada inclusive se usar workspaces especiais; em Scroll preservar posição da faixa.
6. Não pressupor suporte de miniaturas no Wayland; aplicar fallback textual e evitar capturar conteúdos protegidos.
7. Em vários monitores, não assumir um escopo sem escolha: documentar monitor de origem e destinos dos cartões, para testar transição de foco.

### 9.6 Cenários adicionais de aceitação (propostos)

- `OV-006`: clique no corpo do cartão abre detalhe sem mudar workspace real.
- `OV-007`: `Ir para workspace` ativa o alvo e fecha overview uma única vez.
- `OV-008`: `Esc` em detalhe volta a Todos; novo `Esc` fecha.
- `OV-009`: cartão vazio permanece navegável, mas não abre programa.
- `OV-010`: selecionar janela minimizada solicita restauração preservando modo e workspace.
- `OV-011`: fechar a janela enquanto aparece no detalhe remove a entrada sem reatribuir o clique.
- `OV-012`: ao fechar e reabrir, overview inicia em Todos (D-38), não no detalhe anterior.
- `SD-003`: integração com Mostrar área de trabalho não falsifica estados de minimização.

### 9.7 Decisões ainda abertas

- **D-39 RESOLVIDA:** clicar no cartão abre detalhe; ação separada ativa workspace.
- Escopo em múltiplos monitores: visão global da sessão ou recorte por monitor.
- Atalhos concretos, gestos, arrastar janelas entre cartões (futuro), tecnologia das miniaturas e responsividade da grade.

**Registro histórico:** após esta versão, a D-39 foi aprovada; ler seção 10. F-01 permanece futuro.


---

## 10. D-39 — Interação aprovada

**Aprovada expressamente pelo usuário:** o clique no corpo do cartão do workspace abre o detalhe das janelas **sem ativar aquele workspace**; o botão **Ir para workspace**, separado, ativa imediatamente o workspace e fecha Visão Geral. A D-38 não muda: toda abertura do comando geral começa em **Todos os workspaces**.

**Manejo de múltiplos monitores:** ainda **não aprovado**. A seção 11 apresenta a proposta para discussão e não cria D-40.

## 11. P-13 — Experiência com vários monitores (PROPOSTA)

### 11.1 Eixos de design independentes

| Eixo | Alternativa 1 | Alternativa 2 |
|---|---|---|
| Escopo de workspaces | Todos os monitores, cards agrupados por monitor | Apenas o monitor em foco, com troca de filtro |
| Superfície da Visão Geral | Uma janela/superfície no monitor de interação | Uma superfície em cada monitor, sincronizadas |

**Recomendação inicial:** uma única superfície no monitor de interação com a grade de **todos os workspaces agrupados por monitor**. Filtro `Todos | Monitor A | Monitor B` opcional. Essa recomendação não foi aprovada.

### 11.2 Layout conceitual de dois monitores

```text
Visão Geral                                     [Todos] [Monitor A] [Monitor B]
┌─────────────────────────────────────────────────────────────────────────┐
│ Monitor A · Principal · 3 workspaces                                    │
│ [Workspace 1 / Tiling] [Workspace 2 / Scroll] [Workspace 3 / Floating] │
│                                                                         │
│ Monitor B · Secundário · 2 workspaces                                   │
│ [Workspace 4 / Floating] [Workspace 5 / Tiling]                          │
└─────────────────────────────────────────────────────────────────────────┘
Clique cartão: detalhe do workspace | Ação Ir para: ativação sem detalhe
```

### 11.3 Contratos propostos

- Abrir overview não altera foco de janela/workspace no compositor; ter foco **na interface** não significa ativar o workspace correspondente.
- Em card de monitor remoto, clique no corpo permanece no overview e abre detalhes; ação separada `Ir para workspace` transfere foco ao workspace **no monitor de origem efetivo** e fecha overview, sem transportar janelas.
- Seleção de janela remota revalida ID da janela e do workspace; se houve movimento de monitor entre render e clique, atualizar os dados e solicitar somente a ação válida.
- `Esc` no detalhe volta à grade; `Esc` na grade fecha. Ao reabrir, prevalece D-38 (nível Todos). Filtro inicial por monitor ainda pendente.
- Sem captura de tela, apresentar cartões com identificadores, títulos de janela e estado, sem alegar miniaturas reais.
- Janela minimizada é representada no workspace **lógico de origem**, nunca como cartão de workspace especial interno sem uma intenção explícita.
- Em clone/espelhamento, agrupar pelas áreas lógicas/workspaces reais, não duplicar cartões para o mesmo workspace.
- Se monitor for removido, não tentar ativar referência obsoleta; reconsultar topologia, reconstruir a interface e explicar que a disposição mudou.
- Escalas mistas, orientação vertical/horizontal e diferentes taxas de atualização demandam layout lógico adaptativo e limitação de custo de captura.
- Mostrar área de trabalho: **escopo por monitor em foco** é sugestão inicial, não decisão. A proteção de restauração deve abranger todas as janelas atingidas e tratar desconexão sem deixar estado oculto.
- Tema de referência permanece **global** pelo wallpaper do monitor principal (D-31); não ativar tema independente por monitor (F-01).

### 11.4 Separação da tecnologia

Hyprland expõe dados de `monitors`, `workspaces` e `clients` via `hyprctl -j`/IPC. Hyprexpo é referência atual de grades multimonitor e overview Scroll, mas o comportamento de clique exigido pela D-39 poderá demandar adaptação ou solução própria. Validar versão exata do Hyprland e ABI de qualquer plugin antes de assumir compatibilidade com o plugin C++ já existente.

### 11.5 Testes sugeridos

- `MULTI-01`: uma superfície mostra workspaces de dois monitores agrupados e rotulados, sem duplicar workspaces.
- `MULTI-02`: clicar em cartão do monitor B abre detalhe sem mudar foco real do monitor A.
- `MULTI-03`: `Ir para workspace` de B realmente ativa B, sem deslocar a janela de A.
- `MULTI-04`: filtro por monitor só altera a interface; ao reabrir, permanece o nível Todos.
- `MULTI-05`: desconectar B durante detalhe impede clique obsoleto e atualiza o overview.
- `MULTI-06`: alterar escala/orientação recompõe layout e hit testing corretamente.
- `MULTI-07`: selecionar janela remota minimizada restaura apenas alvo elegível, sem perder modo de workspace.
- `MULTI-08`: modo Scroll exibe entradas além do viewport, sem reposicionar a faixa ao abrir overview.

### 11.6 Decisão pendente

Escolher **A — Todos os monitores, agrupados por monitor (recomendação)** ou **B — Monitor em foco, com alternância**. A localização padrão da superfície (uma ou múltiplas) pode ser especificada depois, embora a proposta de uma superfície única simplifique a primeira versão.


---

## 11. D-40 — Organização multimonitor APROVADA (28/10/2026)

**Decisão definitiva:** exibir **todos os workspaces agrupados por monitor**, com **filtros opcionais por monitor**. Essa decisão resolve a antiga proposta P-13 sobre o conteúdo exibido, mas **não** escolhe quantas superfícies de Visão Geral devem aparecer simultaneamente.

### Contratos

| Item | Decisão / restrição |
|---|---|
| Nível inicial | **Todos** sempre (D-38) |
| Agrupamento | Títulos dos monitores e cartões de seus workspaces (D-40) |
| Filtro | Todos / Monitor A / Monitor B / ...; filtrar não altera foco ou modo (D-40) |
| Clique no cartão | Detalhamento de janelas, mantendo Visão Geral (D-39) |
| Botão `Ir` | Ativa workspace explicitamente, inclusive remoto (D-39) |
| Identidade | Número + ícone de modo; miniaturas ou fallback sem captura; janelas minimizadas no grupo lógico confiável |

### Alternativas de localização (pendentes)

- **P-14 — Superfície única no monitor em foco (recomendada)**: uma UI mostra todos os grupos em um só display; os demais monitores não recebem overlay. Evita distrair outras telas, simplifica navegação por teclado e lida com resoluções desiguais com grid responsivo/rolagem.
- **P-15 — Superfícies distribuídas**: cada monitor recebe UI local contendo seus cartões, que juntos cobrem todos os workspaces. É mais espacial, mas exige coordenação de foco, teclado, animações e eventos de conexão de monitores. Precisa manter uma forma explícita de acessar filtros e grupos remotos.

**Regra visual comum:** tema global pelo wallpaper principal (D-31); não reintroduzir o dashboard; não confundir `Mostrar área de trabalho` com miniaturas de workspaces; não afirmar que captura/overlay está implementado.

**Detalhes propostos:** grupos ordenados pelo monitor principal e pela posição física; monitores espelhados sem redundância; workspaces sem atribuição em grupo próprio; seleção remota só altera foco ao escolher `Ir` ou uma janela real. Após mudança de topologia, invalidar cartões e recompor a interface de forma segura.

**Teste-chave:** usuário em Monitor A inspeciona workspace no Monitor B, retorna à grade, aplica filtro Monitor B e só muda de foco ao usar ação de ativação. Reabrir a Visão Geral volta a Todos (D-38).

**Mockup:** `MOCKUP_M09_COMPARACAO_DISPOSICAO_MULTIMONITOR_v0.1.png` (propostas P-14 e P-15, ainda não aprovadas).


---

## 12. D-41 — Superfície única da Visão Geral (APROVADA)

- Criar uma única interface de overview no **monitor em foco**, agrupando nela todos os workspaces de todos os monitores (D-40).
- Abertura sempre em `Todos` (D-38); clique no cartão mostra janelas, botão `Ir` ativa workspace explicitamente (D-39).
- **Não mais pendente:** a antiga P-14 é a configuração aprovada; P-15 (uma superfície em cada monitor) não faz parte da primeira versão.
- Filtrar grupo B, abrir seu detalhe ou voltar **não** equivale a ativar monitor B. Seleção explícita revalida o destino real antes da troca de foco.
- O host da UI e o workspace selecionado são propriedades diferentes. A entrada de teclado no overlay não justifica alterar o workspace subjacente.
- A restauração do foco original é uma regra proposta de UX; verificar se o alvo original existe antes de reativá-lo.

## 13. P-16 — Foco e Mostrar área de trabalho multimonitor

**Pendente D-42:** botão de Mostrar área de trabalho deve atuar sobre **A)** apenas o workspace visível do monitor em foco (recomendação) ou **B)** todos os workspaces visíveis simultaneamente. O comportamento local preserva as janelas dos demais monitores e favorece a restauração sem afetar o trabalho paralelo.

O ocultamento é um estado **temporário, reversível e separado da minimização**, incluindo atenção a pinned/special, fullscreen, Alt+Tab, hotplug, novas janelas e crashes. A Visão Geral consulta o inventário lógico de janelas, inclusive ocultas temporariamente. Nenhum backend ou mecanismo de captura é presumido implementado.

**Referência normativa complementar:** `FOCO_SHOW_DESKTOP_MULTIMONITOR_v0.1.md` detalha matriz de foco, transições e testes, distinguindo aprovado de proposto.


---

## 14. D-42 — Mostrar área de trabalho com escopo local (APROVADA)

**Data:** 28/10/2026. O usuário escolheu explicitamente a **opção A**: o botão ou atalho Mostrar área de trabalho atua **somente no workspace ativo do monitor em foco**. Não oculta as janelas do outro monitor nem altera workspaces não ativos.

- Segundo acionamento no mesmo escopo restaura o estado temporariamente oculto; janelas previamente minimizadas continuam minimizadas.
- Visão Geral continua mostrando essas janelas no inventário lógico; não é necessário revelá-las fisicamente atrás do overlay para construir seus cartões.
- Operações de exploração não modificam foco; `Ir`/selecionar janela são ativações explícitas (D-39/D-41).
- Hotplug, mudanças de workspace, novas janelas, pinned/sticky/fullscreen, falhas e reload exigem políticas de restauração seguras e testes.
- **P-17:** recomenda-se restaurar o escopo oculto quando o usuário abandonar o workspace, iniciar nova janela ali ou escolher uma janela oculta, mas essa regra operacional **ainda não foi aprovada como decisão autônoma**.
- A alternativa B (revelar todos os monitores simultaneamente) não será requisito da primeira versão.

**Referência prevalecente de estados, foco e testes:** `CONTRATO_VISAO_GERAL_FOCO_RESTAURACAO_v1.0.md`. **Documento de foco:** `FOCO_SHOW_DESKTOP_MULTIMONITOR_v0.2.md`. As seções 11–13 registram o histórico anterior à escolha D-42; lê-las à luz desta seção 14.

---

## 15. Atualização aprovada: D-43 — restauração automática na saída do workspace

**Substituição pontual de P-17.2:** está aprovada a restauração automática ao **trocar para outro workspace no mesmo monitor** enquanto Mostrar área de trabalho está ativo. Não equivale a mudar apenas o monitor em foco. O contrato principal é `CONTRATO_VISAO_GERAL_FOCO_RESTAURACAO_v1.2.md`.

**Fluxo:** Mostrar área em A2 → A2 tem suas janelas temporariamente ocultas → ativar A3 no mesmo A → restaurar estado lógico anterior de A2, sem mexer em B5 → retornar a A2 com janelas antes visíveis, as previamente minimizadas ainda minimizadas. O botão deixa de indicar revelação no contexto A2.

**Visão Geral:** consultar cartão de A2 ou B5 não representa saída de workspace. O botão `Ir` poderá provocar troca de workspace — a decisão D-43 será avaliada em relação ao **monitor cujo workspace efetivamente mudou**, não ao monitor onde a Visão Geral está desenhada.

**Permanecem em aberto:** novas janelas no workspace revelado, seleção de janela oculta sem troca e recuperação técnica de pinned/fullscreen/hotplug. Código/miniaturas não auditados nesta conversa.


---

## 16. D-44 — Abertura de nova janela com Mostrar área de trabalho ativado (APROVADA)

Se houver criação/mapeamento efetivo de janela de aplicativo elegível no workspace revelado, restaurar **todas as janelas anteriormente visíveis que a revelação ocultou** e mostrar normalmente a janela nova. Não restaurar janelas previamente minimizadas. Não interpretar `spawn` sem janela, notificação ou nova janela em outro workspace como gatilho. Convergir sem duplicação caso D-43 e D-44 ocorram na mesma sequência. Estado de foco em janela já existente sem criação de janela continua como P-17.2c.

Contrato de referência: `CONTRATO_VISAO_GERAL_FOCO_RESTAURACAO_v1.3.md`.

---

## 17. D-45 — Restaurar e focar a janela escolhida na dock

**APROVADA:** estando Mostrar área de trabalho ativo, clique explícito sobre uma **janela existente temporariamente ocultada** na dock restaura todas as janelas antes visíveis e em seguida foca a janela escolhida. Janelas previamente minimizadas continuam minimizadas; não disparar o toggle de minimização do Floating quando a intenção é restaurar o workspace. Apenas passar o cursor sobre a prévia não causa restauração. A ativação de aplicativo favorito aberto só em outro workspace segue seu contrato anterior.

Esta decisão resolve `P-17.2c` para a interação da dock; notas antigas de pendência foram preservadas apenas como histórico. Contrato vigente: `CONTRATO_VISAO_GERAL_FOCO_RESTAURACAO_v1.4.md`.
