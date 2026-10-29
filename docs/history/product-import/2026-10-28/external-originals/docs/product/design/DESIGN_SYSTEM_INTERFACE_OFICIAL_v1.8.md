# Design System — Rice oficial e Settings da futura suíte Arch

**Versão:** 1.7  
**Data:** 28/10/2026  
**Estado:** **V-B aprovada (D-28)**; **três ilhas modulares aprovadas (D-29)**; **tema automático baseado no wallpaper aprovado (D-30)**; **tema global pelo wallpaper do monitor principal aprovado (D-31)**. A alternativa de temas independentes por monitor foi mantida no backlog futuro (F-01). **Acento derivado automaticamente do wallpaper principal com override manual aprovado (D-32).** **Transparência leve selecionada (D-33): materiais predominantemente opacos; intervalo de referência 94–97% de opacidade na barra e dock, a calibrar em mockups.** **Fonte do sistema escolhida (D-34)**, com fallback e hierarquia responsiva; a família **visual** de ícones lineares foi aprovada (D-35); biblioteca concreta, desenhos e durações de animação permanecem propostas. Algoritmos, valores de cores, materiais e implementação ainda exigem definição/validação.  
**Produto:** nome público ainda em aberto.  
**Documento principal:** `ESPECIFICACAO_INTERFACE_SUITE_v3.9.md` (registro completo D-01 a D-35).

> **Leitura vigente:** as decisões D-28 a D-33 e os requisitos detalhados nas seções 18–25 prevalecem sobre hipóteses históricas preservadas nas seções anteriores. Valores de tipografia, geometria, ícones e movimento nestas seções são **propostas**, não decisões aprovadas.

## 1. Objetivo

Definir linguagem visual coesa para **barra inferior com dock integrada por padrão**, dock separável, launcher híbrido Split (abertura centralizada configurável) e **Settings fundamental**. A proposta deve ser independente de marcas, nomes e ferramentas de desenho, e respeitar os estados efetivos do Hyprland/gerenciador.

**Não confundir:** personalização dos componentes do rice oficial não habilita misturar componentes de rices de terceiros. A Rice Store ativa rices completos (D-06). O Settings fundamental permanece disponível mesmo com rice externo (D-08 e D-23).

## 2. Decisões funcionais que restringem o visual

| Decisões | Consequência para o design |
|---|---|
| D-11, D-02, D-03 | Barra inferior + dock integrada inicialmente; dock desacoplável e ocultação inteligente. |
| D-04, D-05 | Workspace indica número+modo; status pode incluir Configurações e Monitores. |
| D-13 a D-18 | Dock contextual por workspace, favoritos globais, agrupamento configurável, prévias híbridas, desagrupado com favoritos/tarefas separados. |
| D-19 a D-22 | Launcher híbrido, Split, pesquisa global, centralizado com opção ancorada. |
| D-23 a D-27 | Sidebar do Settings adaptativa, controles fundamentais, estados confiáveis, energia e coordenação de serviços. |

## 3. Direção estética aprovada e histórico de alternativas

### V-A — Sólido e minimalista

Superfícies opacas em grafite/marinho, acento pontual, bordas suaves, baixa elevação. Boa legibilidade e menor custo gráfico; requer cuidado para não parecer aplicação genérica.

**Exemplo ilustrativo (não aprovado):** `base #151A22`, `surface #252D38`, `text #F1F5FA`, `accent #79A6DA`.

### V-B — Translúcido contido e adaptativo (recomendação)

Superfícies parcialmente translúcidas apenas onde fazem sentido (barra, launcher, popovers), blur limitado e fallback para superfícies sólidas. Acento coerente com tema/wallpaper, controlado para preservar hierarquia. O Settings poderá ter superfícies opacas e compartilhar os mesmos tokens sem usar blur.

**Exemplo ilustrativo (não aprovado):** `base #141A25`, `surface #26344A`, `text #EAF1FA`, `accent #80B6E7`.

### V-C — Vidro expressivo

Translucidez, blur e camadas visíveis, brilho e destaque de superfície mais perceptíveis, motion mais evidente. Exige regras rigorosas para contraste, consumo de recursos e configurações de acessibilidade.

**Exemplo ilustrativo (não aprovado):** `base #132033`, `surface #314C6D`, `text #F0F5FF`, `accent #78D7F2`.

**Decisão D-28 aprovada:** direção V-B. **Decisão D-30 aprovada:** tema inicial automático com aparência derivada do wallpaper, e substituição manual Claro/Escuro. Os códigos de cor são ilustrativos; extração de acento, fallback detalhado, múltiplos monitores, opacidade, blur, geometria, tipografia e movimento ainda exigem especificação.

## 4. Princípios de design (propostos)

1. **Legibilidade e estrutura** acima de efeitos visuais.
2. **Estados observáveis** (aplicado, salvo, indisponível, falha) não confundidos.
3. **Uma linguagem e várias superfícies:** barra/dock, popovers, launcher e Settings compartilham tokens, ainda que sejam escritos em toolkits diferentes.
4. **Composição orientada a contexto:** docks integradas e flutuantes, vertical/horizontal, launcher centralizado/ancorado, workspaces por monitor.
5. **Degradação graciosa:** sem blur, sem captura de miniaturas ou sem extração de acento, interface permanece funcional.
6. **Controle de movimento e custo:** respeitar redução de movimento; evitar animações permanentes e capturas repetidas desnecessárias.
7. **Não imposição a rices externos:** a aparência oficial é um rice, não um requisito da plataforma de rices completos.

## 5. Taxonomia de tokens sugerida

```text
color.surface.base       color.surface.raised     color.surface.overlay
color.text.primary       color.text.secondary    color.text.disabled
color.accent             color.focus             color.border
color.success            color.warning           color.danger
space.1 4px              space.2 8px             space.3 12px
space.4 16px             space.6 24px            space.8 32px
radius.sm                radius.md               radius.lg
stroke.subtle            stroke.strong
motion.fast              motion.normal           motion.reduced
opacity.panel            opacity.popover         opacity.disabled
```

Valores, escalas e contrastes são exemplos a validar com visualização real. Não atrelar nomes de tokens ao toolkit GTK4 ou a Quickshell. Usar a unidade de pixel lógico e testar diferentes fatores de escala.

## 6. Aplicação por superfície

### 6.1. Barra e dock

- Barra inferior com módulos de launcher, workspaces, dock integrada e status. Indicadores de modo incorporados aos workspaces.
- Quando a dock integrada se recolher, evitar apagar o restante da barra por consequência automática.
- Dock separada respeita orientação, bordas e área útil; popovers surgem para o interior do monitor.
- Favoritos e tarefas separados somente no modo desagrupado; agrupamento e pré-visualizações mantêm indicadores visuais perceptíveis também sem cor.
- Indicadores diferenciados: aplicação aberta, janela focada, mais de uma janela, atenção, minimizada; especificar tamanhos após escolha visual.

### 6.2. Launcher Split

- Busca acima; categorias à esquerda; grade/resultados à direita.
- A pesquisa global independe da seleção de categoria.
- Visual responsivo e rolagem clara; navegação completa por teclado e foco explícito.
- Modo ancorado usa mesmos componentes e contraste do centralizado.

### 6.3. Settings

- Sidebar de Sistema + Interface adaptativa, com barra de pesquisa de configurações proposta.
- Controles do rice oficial substituídos por `Rice ativo` quando um rice externo é ativado.
- Cabeçalhos, controles, avisos e diálogos compartilham tokens, mas nenhum controle sem integração real deve aparentar funcionar.
- Priorizar as telas já implementadas em Rust/GTK4: auditá-las antes de aplicar CSS ou substituir widgets.

## 7. Acessibilidade, desempenho e testes

- Contraste de texto e foco verificável em temas claro/escuro e sobre papel de parede complexo.
- Alvos de toque/clique adequados à escala; operação por teclado e leitor de tela onde houver API suportada.
- Semântica não apenas cromática: ícones, forma, texto e posição também distinguem estados.
- Preferência reduzir movimento; efeitos discretos ou desligáveis; fallback opaco para transparência.
- Testar monitor único/múltiplos, HiDPI/múltiplas escalas, diferentes frequências e painel lateral.
- Verificar isolamento da interface visual versus políticas fundamentais e performance do compositor.

## 8. Itens pendentes

| Item | Estado |
|---|---|
| Direção visual V-A / V-B / V-C | **Concluída — V-B aprovada (D-28)** |
| Tema inicial e origem da escolha automática | **Aprovado — D-30: wallpaper, não preferência global do ambiente** |
| Política multimonitor do tema | **Aprovada — D-31: tema global com wallpaper do monitor principal** |
| Cor de acento derivada da imagem versus escolha manual | **Proposta a detalhar**, sem paleta/algoritmo definitivo |
| Grau de transparência/blur e fallback opaco | Pendente |
| Tipografia, ícones e escala de densidade | Pendente |
| Geometria e tamanhos finais (barra/dock/launcher) | Pendente |
| Animações e estados de hover/foco | Pendente |
| Mockups completos e validação de acessibilidade | Pendente |

## 9. Sequência de trabalho sugerida

**Direção V-B aprovada → detalhar barra contínua/modular → temas/acento → tokens e hierarquia → componentes → mockups desktop/multimonitor → guia de implementação + revisão do código atual.**

**Nota de escopo:** design de produto, não evidência de funcionalidade implementada. Nenhum nome de produto foi aprovado e nenhum componente externo deve ser obrigado a seguir estes tokens.


## 10. Histórico: proposta P-11 do layout modular (refinada e aprovada parcialmente por D-29)

### 10.1 Leitura da referência

Imagem fornecida pelo usuário na conversa (`1793228915156.png`, 1536 × 864): três blocos compactos separados no rodapé (launcher/atalhos à esquerda, aplicativos ao centro, status à direita); há também um painel de aplicativos sobre a área esquerda e uma pesquisa isolada no topo. A intenção considerada aqui é **modularidade visual da barra**, não adoção integral da organização da imagem nem mudança dos requisitos funcionais já aprovados.

### 10.2 Presets de superfície

| Preset | Composição | Status |
|---|---|---|
| **Contínua** | Uma superfície da barra inferior: launcher + workspaces / dock / status | **Padrão já aprovado D-11** |
| **Modular / ilhas** | Três superfícies: esquerda launcher + workspaces; centro dock; direita status e relógio | **Aprovado como alternativa opcional (D-29)**; regras de responsividade ainda em proposta |

A *dock visualmente isolada no preset modular* permanece **integrada ao layout lógico da barra**, ao contrário da **dock desacoplada**, que tem âncora independente e orientação própria. A troca de presets não modifica as janelas; o shell muda somente a apresentação.

### 10.3 Tokens adicionais (a validar)

```text
bar.layout.mode          # continuous | modular
bar.layout.edge          # bottom | top, quando houver suporte
bar.island.anchor        # left | center | right
bar.island.gap           # pixels lógicos, proposto
bar.island.outerMargin   # pixels lógicos, proposto
bar.island.radius        # geometria própria para ilhas
bar.island.material      # V-B + fallback opaco
bar.module.order         # ordem dos módulos em sua ilha, proposta
bar.module.visible       # visibilidade, respeitando módulos essenciais
```

Os nomes são taxonomia de design conceitual, **não contrato existente ou API implementada**.

### 10.4 Interações e robustez

- **Ocultação inteligente:** se a dock integrada se recolher, preservar navegação/status; quando desacoplada, não renderizar outra dock no bloco central.
- **Overflow:** priorizar elementos essenciais (launcher, workspace ativo, status-chave), compactar a dock antes de causar colisões; evitar que ilhas central e laterais se sobreponham em monitores menores.
- **Popovers:** menus e pré-visualizações aparecem para dentro da tela e não são recortados pelo contêiner da ilha.
- **Multi-monitor / HiDPI:** usar âncoras lógicas, escala e identidade da saída, não coordenadas absolutas fixas.
- **Persistência:** mudanças de aparência não mexem em regras de janelas; incluir recuperação para preset contínuo se a configuração de ilhas estiver inválida.

### 10.5 Resolução da escolha

**D-29 aprovada:** alternativa **A**, com três ilhas ancoradas e possibilidade de reordenar módulos **dentro de cada ilha**. Não foi aprovado posicionamento livre arbitrário, criação ilimitada de ilhas nem movimentação irrestrita de qualquer módulo entre ilhas. A barra contínua permanece o padrão da instalação (D-11).

**Atenção:** isto não contradiz a D-06: não será possível misturar barra de um rice e dock de outro na Rice Store inicial.

## 11. D-29 — Layout modular opcional com três ilhas ancoradas (APROVADO)

**Decisão expressa, 28/10/2026:** o usuário escolheu a alternativa A: **três ilhas ancoradas e módulos reordenáveis em cada ilha**. Esta é uma **decisão de produto/interface**, não comprovação de implementação existente, nem aprovação de drag-and-drop ou edição livre de layout.

### 11.1 Matriz de presets e acoplamento

| Layout da barra | Estado da dock | Superfícies visíveis / função |
|---|---|---|
| Contínuo (**padrão aprovado**) | Integrada (**padrão aprovado**) | Uma faixa com navegação, dock e status |
| Modular (**opcional aprovado**) | Integrada | Ilhas esquerda, centro e direita, com dock como ilha central |
| Contínuo | Desacoplada | Barra com navegação e status; dock independente sem duplicação |
| Modular | Desacoplada | Âncoras esquerda e direita permanecem; **a ilha central da dock deixa de existir**; dock independente |
| Ambos | Desativada | Sem dock; launcher/workspaces/status preservados conforme opções de visibilidade |

A existência das **três âncoras lógicas** não obriga renderizar três superfícies quando uma ilha não tem conteúdo. A preferência de agrupamento, escopo de workspace e minimização/restauração da dock não se altera ao trocar de layout.

### 11.2 Contrato visual das ilhas

- `left` **Navegação**: por padrão launcher e pílulas de workspace, com indicador de Tiling/Scroll/Floating incorporado. A ordem interna dos módulos permitidos pode ser alterada.
- `center` **Dock**: favoritos e janelas do workspace ativo, com as políticas já aprovadas. Os elementos internos continuam sob as regras próprias da dock (ordem de favoritos, agrupamento). Não pressupor que todas as tarefas sejam módulos da barra.
- `right` **Sistema**: relógio, rede, áudio, Bluetooth, notificações, bateria quando houver e acessos opcionais a Settings/monitores. Ordem e visibilidade de módulos opcionais configuráveis.
- **Reordenação dentro da ilha:** a decisão aprovada é mudar a ordem de módulos admissíveis **dentro de sua âncora**. Transferir o launcher para o status, criar quarta ilha, deslocar ilhas livremente, ou arrastar regiões para outra borda **não** foi aprovado.
- **Âncora central verdadeira:** procurar manter a dock centrada no monitor e não apenas centrada no espaço residual entre ilhas laterais; em conflitos de largura, aplicar política responsiva explícita.
- **Posições relativas:** preservar alinhamento esquerda/centro/direita ao trocar resolução e escala, com margens lógicas uniformes.

### 11.3 Estratégia responsiva — PROPOSTA, não decisão aprovada

Ordem candidata de degradação quando as ilhas não couberem no monitor:

1. Remover rótulos secundários e espaçamento opcional; preservar ações essenciais.
2. Compactar o status por prioridades configuráveis e limitar comprimento de relógio/data.
3. Restringir largura da dock, usando overflow navegável ou compactação de tarefas; manter acessibilidade às janelas.
4. Manter âncora central sem sobreposição; se a largura for insuficiente, apresentar estado compacto previsível ou oferecer mudança sugerida para barra contínua.
5. **Nunca** ocultar silenciosamente o único caminho para abrir Settings, desbloquear a sessão ou acessar aplicativos.

**Separar superfície de área reservada:** em vez de assumir que uma camada de tela recorta o wallpaper, estudar como reservar a área necessária para evitar colisão com janelas, popovers e áreas exclusivas do Wayland/Hyprland. Não considerar `exclusive zone`, blur ou hit-testing resolvidos até validar no compositor real.

### 11.4 Ocultação inteligente e interação

- A dock integrada ao layout modular pode recolher **apenas a ilha central**, preservando as duas ilhas laterais; esse comportamento específico permanece proposta de UX até sua validação.
- Desacoplar não cria cópia da dock: a ilha central torna-se ausente e a dock passa a usar âncora e orientação próprias.
- Popovers (seletor de janelas, relógio, rede, menu de modo) devem abrir para dentro do monitor e não podem ser cortados pela superfície/contêiner da ilha.
- Ao entrar em fullscreen real, a política de visibilidade de todas as superfícies precisa ser respeitada, sem revelação acidental por sensores de borda; o comportamento detalhado permanece sujeito a testes.
- Ao usar múltiplos monitores, calcular posicionamento em pixels lógicos da saída correspondente, e restaurar a configuração com segurança se uma saída deixar de existir.
- Reordenar módulos não pode ativar aplicações acidentalmente. A UI deve oferecer um modo de edição inequívoco, com cancelar/restaurar padrão, mas o mecanismo (botões, arrastar, lista ordenável) continua aberto.

### 11.5 Settings proposto

Caminho: `Settings → Interface → Barra → Organização`.

| Controle | Valor / política | Estado |
|---|---|---|
| Layout | `Contínuo` ou `Modular` | **Disponibilidade aprovada D-29; contínuo como padrão D-11** |
| Reordenação | Ordem de módulos permitidos em cada ilha | **Aprovada D-29** |
| Visibilidade de módulos não essenciais | Alternar e escolher prioridades | Proposta |
| Espaçamento entre ilhas | Densidade/intervalo coerente por monitor | Proposta |
| Margem externa | Respeitar limites e escala da tela | Proposta |
| Altura/densidade | Compacta, padrão, confortável | Proposta |
| Material | opacidade e blur configuráveis, com fallback opaco | Direção V-B aprovada; valores propostos |
| Restaurar preset | Recuperar layout válido | Proposta de segurança |
| Por monitor | Preferências distintas por saída | Fora da baseline aprovada; avaliar posteriormente |

**Importante:** a configuração da dock (acoplar/desacoplar, agrupamento, previews, smart-hide) permanece em sua página específica e não deve ser duplicada na página Barra.

### 11.6 Esquema de estado conceitual (não é API existente)

```yaml
panel:
  layout: continuous  # continuous | modular
  islands:
    left: [launcher, workspaces]
    center: [dock]
    right: [network, audio, clock]
dock:
  attachment: integrated  # integrated | detached | disabled
```

Restrições: IDs de módulos estáveis, validação de entradas, migração de esquema e persistência atômica. Não salvar em formatos conflitantes com o núcleo Lua sem auditoria. Rices externos não importam automaticamente essa configuração.

### 11.7 Cenários de aceitação candidatos

1. Trocar `continuous ↔ modular` não modifica os modos do workspace nem a geometria de janelas.
2. Reordenar rede/relógio na ilha direita persiste após reiniciar o shell; não afeta workspaces.
3. Desacoplar a dock remove a ilha central (sem duplicar favoritos/tarefas).
4. Ocultar a dock não oculta launcher nem controles fundamentais, exceto fullscreen conforme política aplicável.
5. Com monitores de escala diferente e uma saída removida, nenhuma ilha fica fora de tela ou cobre outra.
6. Com muitas janelas e workspaces, o sistema mostra overflow acessível sem recortar itens críticos.
7. O rice externo não recebe nem sobrescreve a configuração dos módulos oficiais.
8. Falha de configuração permite recuperação para preset contínuo sem sobrescrever decisões fundamentais da suíte.

## 12. Refinamento dos fundamentos visuais V-B — propostas para próxima aprovação

**Nenhum valor numérico ou paleta abaixo foi aprovado ainda.** Eles são parâmetros para prototipagem visual e comparação, não requisitos de código.

### 12.1 Temas e cor de acento

- **Candidato de instalação:** tema escuro neutro (grafite com leve azul), com versão clara e opção de seguir preferência do ambiente **quando ela for detectável**.
- **Acento:** cor base calma e moderadamente saturada; possível opção `Automático pelo wallpaper`, sempre com fallback estável e controle manual.
- **Regra visual:** não usar cor derivada do wallpaper diretamente em texto ou controles críticos sem verificar contraste. Não usar valor de cor único para distinguir ativo, erro e atenção.
- **Não assumir** que a escolha da direção visual `adaptativo` implique sincronizar a cor em tempo real com o wallpaper: essa funcionalidade ainda é apenas proposta.

**Paleta de referência ilustrativa, tema escuro** (para protótipos, não final):

| Papel | Cor candidata | Observação |
|---|---|---|
| Fundo neutro de contexto | `#151B29` | Referência do preview, não força wallpaper |
| Superfície de painel | `#273449` | Material-base para composição |
| Superfície elevada/popover | `#313F56` | Separação de menus sem excesso de brilho |
| Texto primário | `#F1F5FB` | Validar contraste na superfície final |
| Texto secundário | `#BBC9DC` | Verificar fundos claros e fotos |
| Acento suave | `#8BB9E8` | Apenas ilustração; pode ser customizado |
| Borda sutil | `#5B708A` | Evitar substituir foco de teclado por borda fraca |

### 12.2 Materiais e transparência

| Superfície | Regra proposta | Motivação |
|---|---|---|
| Ilhas / barra | Translucidez moderada com fundo controlado | Preservar wallpaper sem perda de legibilidade |
| Popovers de janelas/status | Mais opacos que barra | Leitura clara de textos e prévias |
| Launcher | Fundo mais estável e opaco | Pesquisa e grade legíveis |
| Settings | Predominantemente opaco | Campos, avisos e diagnóstico prioritários |
| Sem blur/capacidade indisponível | Superfície sólida de contraste equivalente | Degradação graciosa |

**Parâmetros de exploração visual, sem aprovação:** opacidade em torno de `0,88–0,94` para barra/ilhas e `0,94–0,98` para popovers/launcher; blur leve se suportado. Esses valores não representam comandos do Hyprland nem teste de legibilidade. O contraste deve ser avaliado na composição final com wallpaper claro, escuro e de alta frequência visual.

### 12.3 Geometria e hierarquia

- Escala de espaçamento proposta: **4, 8, 12, 16, 24, 32** pixels lógicos.
- Raios propostos: pequenos **8**, médios **12**, grandes **16** pixels lógicos, com maior arredondamento em ilhas; sem misturar estilos incompatíveis.
- Ilha com altura percebida semelhante à barra contínua; diferença de material, não de tamanho funcional.
- Ícones com mesma linguagem geométrica; uso preferencial de ícones de aplicativos reais para apps, com traços consistentes nos controles do sistema.
- Diferenciar `aberto`, `ativo`, `minimizado`, `múltiplas janelas`, `atenção` por forma/posição/contagem além da cor.
- Alvos interativos com escala e acessibilidade adequadas; considerar referência de 36–44 px lógicos, a validar em monitores e densidades distintos.
- Hierarquia de texto prevista: título da página > grupo > rótulo do controle > descrição contextual, evitando aplicar transparência decorativa aos textos.

### 12.4 Movimento e desempenho

- Mudança `continuous ↔ modular` com animação curta e sem movimentar janelas do usuário; tolerar modo de movimento reduzido.
- Sem animação contínua de status e sem renderização onerosa quando a dock está oculta.
- Blur, miniaturas e papel de parede dinâmico não podem ser pré-requisitos para interagir com Settings ou recuperar a sessão.
- Medir custo em monitores com diferentes escalas/taxas de atualização; não presumir desempenho perfeito por se tratar de GPU potente.

### 12.5 Acessibilidade e contraste — metas propostas

- Mirar contraste mínimo de **4,5:1** para texto normal e **3:1** para componentes de interface e indicadores essenciais, com verificação sobre os materiais compostos finais.
- Foco de teclado não deve depender apenas de cor; usar contorno perceptível e ordem de Tab coerente.
- Evitar indicar estado apenas com uma marca muito pequena, especialmente nos workspace pills e na dock.
- Fornecer alternativas sem transparência e com movimento reduzido sem perder funcionalidades.

## 13. Próxima decisão de design

Definir o **comportamento inicial do tema**: (A) escuro de fábrica com claro e automático disponíveis, (B) claro de fábrica com escuro/automático disponíveis, ou (C) seguir a preferência do ambiente quando detectável, com fallback definido. A decisão de sincronizar acento com wallpaper é **separada** e posterior.

## 14. Status e rastreabilidade

| Tema | Estado |
|---|---|
| Direção visual V-B | **Aprovada — D-28** |
| Layout modular com três ilhas e reordenação interna | **Aprovado — D-29** |
| Barra contínua como preset de instalação | **Aprovada anteriormente — D-11** |
| Editor livre, número arbitrário de ilhas, arraste entre ilhas | **Não aprovado** |
| Tema inicial e alternância automática | **Aprovados — D-30** |
| Escopo multimonitor e imagem de referência | **Aprovados — D-31; opção independente arquivada como F-01 (futuro)** |
| Paleta/acento e extração do wallpaper | **Propostos, não aprovados** |
| Opacidade, blur, raios e dimensões | **Candidatos, não aprovados** |
| Toolkit, API de configuração e testes reais | **Pendentes de auditoria do código existente** |


## 15. D-30 — Tema automático baseado no wallpaper (aprovada em 28/10/2026)

### 15.1. Decisão aprovada

O usuário escolheu a alternativa de **tema automático inicial**, esclarecendo que a mudança deve ocorrer **em função do papel de parede selecionado**, e **não meramente pela preferência claro/escuro exposta pelo ambiente**. Isso complementa a direção visual **V-B — Translúcido contido e adaptativo**.

- **Modo inicial da interface oficial:** `Automático (wallpaper)`.
- **Modos substitutos disponíveis no Settings:** `Claro` e `Escuro`, acionados manualmente; em modo manual, trocar wallpaper não altera claro/escuro automaticamente.
- **Evento:** seleção/troca de wallpaper reconhecida pela interface oficial pode recalcular a aparência; operações devem ser aplicadas de maneira coordenada, sem flashes ou alterações parciais persistentes.
- **Limite de responsabilidade:** o sistema de tema da interface oficial **não reescreve configurações de rices externos**, que continuam gerenciando sua própria aparência. O Settings fundamental mantém funcionalidade independente.
- **Preservar decisões anteriores:** barra contínua é padrão; modular em três ilhas é opcional; dock, launcher e Settings mantêm os comportamentos definidos.

### 15.2. Pipeline proposto para a primeira implementação (não aprovado em detalhe)

```text
Usuário seleciona wallpaper
          │
          ▼
Validar imagem e obter representação reduzida (cache opcional)
          │
          ├──► Análise de luminosidade perceptiva e sua distribuição
          │               └──► Sugestão de variante CLARA ou ESCURA
          │
          └──► Extração de cores relevantes da imagem
                          └──► Proposta de acento / paleta harmônica
          │
          ▼
Gerar tokens semânticos para a variante escolhida
          │
          ▼
Validar contrastes, substituir cores inadequadas e manter acessibilidade
          │
          ▼
Aplicar atomicamente nos componentes do rice oficial
          │
          ▼
Disponibilizar preferências e diagnóstico no Settings
```

**Separar duas operações:** (i) detectar a aparência clara/escura a partir do wallpaper e (ii) gerar cores de acento a partir dele. A decisão D-30 **aprova (i)**. A geração dinâmica de acento é a direção natural da proposta estética, mas **ainda não foi especificada/ratificada como obrigação**; deve ter substituição manual.

**Não usar apenas luminosidade média bruta:** paredes de cor clara com detalhes escuros, wallpapers de alto contraste ou imagens com áreas brilhantes devem ser analisados pela distribuição da luminosidade e pelo impacto nas regiões atrás dos elementos da interface. Evitar troca de modo por pequenas mudanças no valor calculado (histerese/limiares propostos).

### 15.3. Comportamentos e salvaguardas propostos

| Cenário | Comportamento recomendado | Estado |
|---|---|---|
| Wallpaper predominantemente claro | Selecionar variante clara com superfícies suficientemente opacas e texto escuro | Interpretação da D-30; algoritmo a validar |
| Wallpaper predominantemente escuro | Selecionar variante escura com superfícies e texto adequados | Interpretação da D-30; algoritmo a validar |
| Wallpaper de iluminação mista | Decisão por análise robusta, com possibilidade de ajuste manual | Pendente de calibração |
| Wallpaper muda no modo `Automático` | Atualizar tokens da interface oficial, mantendo consistência entre superfícies | Direção aprovada; implementação a validar |
| Wallpaper muda no modo `Claro`/`Escuro` manual | Preservar a variante escolhida | Regra de override manual recomendada |
| Wallpaper indisponível/erro de leitura | Conservar último tema válido; caso não exista, usar tema neutro com contraste verificado | Fallback **proposto** |
| Wallpaper diferente por monitor | Evitar temas globais conflitantes; definir imagem de referência ou tema individual por monitor | **Pendente — escolha futura** |
| Blur indisponível ou contraste insuficiente | Aumentar opacidade/trocar tokens, manter legibilidade | Regra de acessibilidade recomendada |
| Rice externo ativo | Não modificar o tema ou o wallpaper do rice terceiro | Limite de escopo já aprovado |

### 15.4. Interação com a implementação GTK4

O app Settings existente em Rust/GTK4 deve receber tokens compatíveis e respeitar as preferências do usuário, **sem ser reescrito**. Pode usar o `Adw.StyleManager` quando aplicável ou um adaptador próprio, mas nunca assumir que mudar a preferência GTK equivale automaticamente a aplicar tema em todo software instalado. Aplicações terceiras podem ter mecanismos próprios; sincronização global é um recurso separado, opt-in e sujeito a compatibilidade.

### 15.5. Ferramentas candidatas — não aprovadas

- **Matugen:** gera paletas claras/escuras a partir de imagens e exporta tokens de cores. Pode atender à extração da paleta, mas **a política automática de escolher claro/escuro a partir da imagem não deve ser presumida como fornecida pelo Matugen**. O gerenciador precisa definir e testar essa decisão.
- **Analisador próprio de luminosidade:** componente pequeno e testável para inferir modo quando necessário, preservando overrides e fallback.
- **GTK4/libadwaita:** oferece APIs para forçar tema claro/escuro por aplicação; não garante automaticamente coerência com todas as apps e rices.

Referências técnicas consultadas em 28/10/2026:
- https://github.com/InioX/Matugen/blob/main/README.md
- https://github.com/InioX/matugen/wiki/Usage
- https://gnome.pages.gitlab.gnome.org/libadwaita/doc/1-latest/styles-and-appearance.html

### 15.6. Critérios de aceitação propostos

1. Ao selecionar wallpapers claros/escuros de teste, a interface oficial adota uma variante legível consistente com sua classificação, quando `Automático` está ativo.
2. Alterar o wallpaper não modifica a variante escolhida manualmente pelo usuário.
3. Falha no processamento da imagem não deixa componentes com cores ilegíveis nem valores incompletos.
4. Acentos escolhidos não são usados isoladamente para transmitir estados de foco, erro, atenção ou seleção.
5. Dock integrada/desacoplada, barra contínua/modular, launcher e Settings permanecem consistentes após a transição.
6. Nenhuma configuração de rice externo é sobrescrita.
7. A troca de wallpaper não exige reiniciar o gerenciador de janelas para atualizar a aparência do rice oficial, se a API do shell suportar recarga de tokens; caso contrário, registrar limitação explícita.
8. Em configurações multimonitor, não aplicar cores inconsistentes sem uma política de wallpaper de referência acordada.

### 15.7. Pendências após D-30

- **P1:** fonte do wallpaper de referência em múltiplos monitores: primário ou tema independente por monitor.
- **P2:** acentos automaticamente extraídos do wallpaper como padrão e opções de substituição manual.
- **P3:** estratégia exata de classificação claro/escuro, inclusive wallpapers mistos.
- **P4:** ordem do fallback sem imagem válida e resposta a troca automática de wallpaper.
- **P5:** tipografia, grau de translucidez, cores finais, dinâmica de blur e animações.

**Histórico:** as menções anteriores a “seguir preferência do ambiente” como alternativa futura estão superadas pela D-30; o **wallpaper é a fonte primária da escolha automática**.


## 16. D-31 — Tema global derivado do wallpaper principal (APROVADO em 28/10/2026)

### 16.1. Escolha confirmada

O usuário escolheu expressamente a alternativa **A — tema global baseado no wallpaper do monitor principal**. A interface visual oficial usa uma **única variante de tema e uma única paleta global** em todas as saídas, mantendo a coerência entre barra (contínua ou modular), dock, launcher e componentes oficiais de interface.

Esta decisão complementa a D-30: a seleção automática entre tema claro/escuro continua baseada no **wallpaper escolhido**, com o **monitor principal** como referência em configurações de múltiplos monitores. Se o usuário selecionar manualmente Claro ou Escuro, essa substituição permanece válida para a interface inteira e prevalece sobre a análise automática.

**Escopo aprovado:** política de referência e aplicação globais. **Não equivale a** ter aprovado algoritmo de análise, geração dinâmica de acentos, sincronização de temas de aplicativos de terceiros, biblioteca, persistência técnica ou troca de tema por monitor.

### 16.2. Matriz funcional de referência

| Evento / contexto | Comportamento estabelecido ou consequência direta | Condição |
|---|---|---|
| Imagens diferentes no monitor principal e no secundário | Usar o wallpaper do **monitor principal** para o tema global | D-31 aprovada |
| Wallpaper do monitor principal alterado em modo Automático | Reavaliar a variante global da interface oficial | D-30 + D-31; algoritmo a validar |
| Wallpaper apenas do monitor secundário alterado | Não mudar a paleta global por essa alteração | D-31 aprovada |
| Usuário escolhe Claro ou Escuro manualmente | Manter a escolha manual global | D-30 aprovada |
| Launcher aberto em monitor secundário | Usar a mesma paleta global, sem adaptação local | D-31 aprovada |
| Monitor principal desconectado/trocado | Reavaliar a referência **apenas após definir com segurança o novo primário**, sem tema inválido ou oscilação | Salvaguarda proposta, não regra técnica fechada |
| Fonte de wallpaper indisponível | Conservar último tema válido ou recorrer a paleta neutra validada | Fallback proposto |

### 16.3. Backlog futuro — F-01 (NÃO APROVADO PARA MVP)

**Tema independente por monitor.** Cada monitor poderia derivar sua variante claro/escuro e paleta do wallpaper local, permitindo que a barra/dock acompanhe o fundo daquela saída. É uma ideia explicitamente solicitada para **o futuro**, não uma funcionalidade prometida ou aprovada para a primeira entrega.

Questões para investigação futura:

1. Como propagar tokens distintos para cada superfície e monitor sem gerar inconsistência visual.
2. Qual tema o launcher centralizado/ancorado usa ao abrir ou migrar de saída.
3. Como tratar o Settings GTK4 (janela móvel entre monitores), popovers e persistência de preferências.
4. Como evitar recomputações, oscilações de tema, inconsistências e regressões de acessibilidade.
5. Como identificar wallpapers por saída quando o rice utiliza mecanismos diferentes de wallpaper.

**Critério para retomar F-01:** existência de um contrato de tema por saída e testes de multimonitor, desempenho, acessibilidade e integração. Até lá, D-31 prevalece.

### 16.4. Próxima decisão — acento cromático

A política **claro/escuro** está definida pelas D-30 e D-31. Permanecem independentes as escolhas de **cor de acento**: (A) derivação automática do wallpaper principal, com substituição manual; (B) paleta de acento fixa; (C) derivação automática com escolha configurável de estratégia. A escolha futura não altera a política global aprovada.

**Rastreabilidade:** seções antigas deste documento mencionam multimonitor e modo inicial como *pendentes* por preservarem o histórico das versões anteriores. Elas estão superadas pelas D-30 e D-31; esta seção e a tabela de decisões atualizada prevalecem.


## 17. D-32 — Cor de acento adaptativa com opção manual (aprovada em 28/10/2026)

### 17.1 Decisão aprovada e interpretação

Em resposta à proposta explícita de **derivar automaticamente o acento do wallpaper principal com possibilidade de escolha manual**, o usuário orientou prosseguir. Adota-se a política como decisão **D-32**; a seleção do algoritmo, os valores e a intensidade da cor **ainda não estão aprovados**.

- **Padrão:** `accent.source = wallpaper` — derivar cor de acento do wallpaper que determina o tema global (D-30/D-31).
- **Alternativa:** `accent.source = manual` — o usuário escolhe a cor no Settings; essa preferência persiste quando o wallpaper muda.
- **Independência:** o controle `appearance.mode = automatic | light | dark` é **separado** de `accent.source = wallpaper | manual`. É possível escolher *tema escuro com acento automático*, ou *tema automático com acento manual*.
- **Alcance:** os tokens pertencem ao rice oficial. Não modificar rices externos, estilos arbitrários de programas GTK/Qt nem configurações da Rice Store.
- **Segurança visual:** contraste mínimo, hierarquia, distinção entre foco/seleção/estado e fallback prevalecem sobre fidelidade literal à cor da imagem.

### 17.2 Pipeline proposto de paleta (não aprovado em detalhe)

```text
Wallpaper ativo do monitor principal (D-31)
       ↓ leitura/validação da imagem
Amostragem e avaliação de luminosidade (D-30)
       ↓
Seleção clara/escura com estabilidade anti-oscilação
       ↓
Cores candidatas → filtragem por contraste/legibilidade
       ↓                                ↖ acento manual opcional
Tokens semânticos: surfaces/text/accent/focus/status
       ↓ validação e atualização atômica
Barra contínua/modular + dock + launcher + interface oficial
       ↓
Fallback seguro se a operação falhar
```

**Proposta de implementação:** preferir paleta tonal, e não copiar a cor predominante de cada pixel. Rejeitar acentos demasiadamente próximos ao fundo ou que tornem ícones ilegíveis; converter um tom da imagem em várias tonalidades semânticas (ex.: `accent`, `on_accent`, `accent_container`, `focus_ring`). As cores de sucesso/alerta/erro não devem depender totalmente do wallpaper, nem atuar como único indicador de estado.

**Controle de atualização:** calcular apenas na troca efetiva de wallpaper / monitor principal, com cache por imagem e debounce; evitar reanálise por quadro e troca repetida claro/escuro em wallpapers ambíguos. Se mudar a saída principal, consultar a nova imagem apenas depois de confirmar a nova seleção de monitor.

### 17.3 Parâmetros que o Settings deverá expor (proposta)

| Controle | Opções | Situação |
|---|---|---|
| Modo do tema | Automático (wallpaper) / Claro / Escuro | Aprovado em D-30 |
| Referência da imagem | Monitor principal | Aprovado em D-31 |
| Cor de acento | Do wallpaper / Manual | **Aprovado em D-32** |
| Cor manual | Seletor e presets opcionais | Proposta |
| Intensidade do acento | Suave / Normal / Vibrante (ou controle contínuo) | Proposta, não aprovada |
| Transparência | Regulação global com opções por superfície | Proposta |
| Reduzir transparência | Alternativa opaca para legibilidade | Proposta |

Ao trocar manualmente a cor, exibir visualização prévia e opção **Restaurar automático**. Não sobrescrever a preferência manual por conta de atualizações do wallpaper. Se o wallpaper não estiver legível, conservar última paleta **validada** ou fallback neutro testado.

### 17.4 Tokens de cor propostos (estrutura, valores ilustrativos)

| Token | Função |
|---|---|
| `surface.base`, `surface.panel`, `surface.popover`, `surface.settings` | Fundos neutros adaptados à variante, com alpha individual |
| `text.primary`, `text.secondary`, `text.disabled` | Texto calculado para superfície correspondente |
| `accent.base`, `accent.hover`, `accent.pressed`, `accent.container`, `accent.on_base` | Acento e variações com contraste |
| `focus.ring`, `outline.subtle`, `outline.strong` | Foco por teclado, contornos e separadores |
| `status.success`, `status.warning`, `status.danger` | Semântica relativamente estável entre wallpapers |

**Exemplos cromáticos somente para exploração:** wallpaper púrpura → acento lilás frio; wallpaper areia/creme → acento âmbar suave; wallpaper verde/floresta → acento esmeralda moderado. Não se aprovou código hexadecimal nem paleta obrigatória. A imagem de referência fornecida pelo usuário inspira a discussão, mas não deve ser tratada como wallpaper definitivo da suíte.

### 17.5 Materiais V-B por componente (propostas, não decisões)

| Superfície | Tratamento sugerido | Motivo |
|---|---|---|
| Barra contínua ou três ilhas | Opacidade de **88–94%** como intervalo exploratório; blur moderado | Conter ruído visual e manter legibilidade |
| Dock desacoplada | Mesmo material da barra, sem forçar mesma geometria | Unidade visual sem acoplamento funcional |
| Preview e menus | **94–98%** opacidade exploratória; blur limitado | Legibilidade e previsibilidade em popovers |
| Launcher Split | **92–97%** opacidade exploratória; blur opcional | Prioridade para busca e seleção por teclado |
| Settings Rust/GTK4 | Predominantemente opaco | Conforto de leitura em formulários |

Os percentuais são **hipóteses para mockups/testes**, não valores de implementação aprovados. O compositor poderá aplicar blur em superfícies de camada identificadas por namespace, sujeito à versão e ao ambiente real; não definir comandos nem modificar regras antes de auditoria. Sem blur, usar fundo mais opaco com contraste estável. Estados (foco, ativo, minimizado, múltiplas janelas) também precisam ser distinguíveis sem cor.

### 17.6 Requisitos de validação propostos

- Medir contraste de texto de corpo segundo WCAG AA (referência: 4,5:1 para texto comum e 3:1 para texto grande), e avaliar controles/indicadores não textuais que precisam ser percebidos, inclusive sobre wallpaper claro, escuro e de alto detalhe.
- Testar 100% e 150–200% de escala lógica, múltiplos monitores, barra modular, dock lateral e popovers sem clipping.
- Garantir navegação por teclado com foco claro mesmo quando o acento extraído tiver pouco contraste.
- Evitar piscadas durante atualização de tokens. Preservar as preferências se análise ou renderização falhar.
- Testar opção sem blur, opacidade máxima e movimento reduzido; impactos de desempenho em jogos e fullscreen.
- Não tentar recolorir todos os aplicativos de terceiros; apenas superfícies oficiais sob controle da suíte.

### 17.7 Referências técnicas para verificação posterior

- Matugen — geração de paletas Material You/base16 por imagem ou cor, exportação e templates: https://github.com/InioX/matugen e https://github.com/InioX/matugen/wiki/Usage
- Hyprland — regras de camada com `blur`, `blur_popups`, `ignore_alpha`: https://wiki.hypr.land/configuring/core/rules/layer-rules/

**Não aprovado:** escolha de Matugen como dependência definitiva, algoritmo próprio de luminância, valores de opacidade e raio, cores finais, integração específica com o toolkit ou arquivos gerados.

### 17.8 Próxima escolha de design

Definir **o nível inicial de translucidez** da direção V-B: leve (mais opaco), equilibrado (blur e opacidade moderados), ou expressivo (mais vidro), preservando controle de reduzir efeitos. Em seguida, fechar tipografia, geometria de componentes e motion. Manter no backlog a possibilidade F-01 de paletas por monitor.


---

## 18. D-33 — Transparência leve (APROVADA em 28/10/2026)

### 18.1 Escolha expressa

O usuário escolheu **A — transparência leve** para os materiais do rice oficial, como refinamento da direção estética **V-B (D-28)**. Essa escolha **substitui as propostas históricas de opacidade equilibrada (88–94%) ou vidro acentuado (78–86%) como comportamento padrão**. Não elimina a possibilidade de o usuário regular efeitos nas configurações.

- **Intenção aprovada:** superfícies pouco transparentes, legíveis e discretas; wallpaper influencia sobretudo o tema e o acento, sem atravessar visualmente textos e controles.
- **Referência de prototipagem associada à opção A:** aproximadamente **94–97% de opacidade** para barra contínua, ilhas modulares e dock. Não foi aprovado um número exato por componente.
- **Blur:** opcional e discreto; nenhum requisito funcional deve depender de blur.
- **Menus, seletores, launcher e Settings:** podem ser mais opacos, com valores específicos a validar; o Settings existente em GTK4 deve permanecer predominantemente sólido.
- **Acessibilidade:** manter alternativas opacas e sem efeitos, inclusive quando wallpapers forem complexos ou coloridos.
- **Escopo:** apenas rice oficial; nenhum rice externo precisa adotar esta aparência.

### 18.2 Material por componente — hipóteses de calibração (NÃO APROVADAS em detalhe)

| Superfície | Opacidade-alvo para os primeiros mockups | Observações |
|---|---:|---|
| Barra contínua | 94–97% | Alpha final validado sobre wallpapers diferentes; não confundir cor RGBA com opacidade efetiva após composição. |
| Ilhas modulares | 94–97% | Três âncoras D-29; preservar alinhamento, separações e legibilidade. |
| Dock desacoplada | 94–97% | Material alinhado ao da barra; regras de exposição e posição independentes. |
| Launcher Split | 97–100% | Não sacrificar contraste ou leitura da pesquisa. |
| Menus e pré-visualizações | 97–100% | Favorecer identificação de janelas e controles. |
| Settings GTK4 | Preferencialmente 100% | Pode compartilhar cores/tokens sem simular blur. |

**Nota de precedência:** seções antigas que recomendavam 88–94% ainda existem como histórico. O preset vigente é a opção A. Valores acima são guias de protótipo, não contratos técnicos nem requisitos imutáveis.

## 19. Tipografia — proposta T-01 (PENDENTE)

**Princípio:** sans-serif neutra, legível e consistente em textos curtos, nomes de janelas, configurações longas e monitores HiDPI. Não usar fonte decorativa futurista como base de interação.

### 19.1 Famílias a avaliar

| Candidata | Vantagem de UX | Atenção de integração | Status |
|---|---|---|---|
| **Inter** | Aparência moderna, boa hierarquia em UIs | Disponibilidade, empacotamento, fallback e compatibilidade com GTK4 | Proposta preferencial |
| **Noto Sans** | Cobertura linguística ampla e consistência | Ocupação visual um pouco diferente; avaliar métricas | Alternativa |
| **Fonte do ambiente** | Respeita preferências do usuário e padrões do toolkit | É preciso lidar com métricas diferentes entre barra e Settings | Alternativa |

**Nenhuma família está aprovada.** Propor fonte principal com fallback configurável; preservar escalonamento/acessibilidade. Não redistribuir fontes sem verificação de licença e empacotamento quando chegar à implementação.

### 19.2 Escala tipográfica candidata (em px lógicos, não pt físicos)

| Token | Tamanho de referência | Peso de referência | Exemplos |
|---|---:|---:|---|
| `type.caption` | 12 | 400/500 | metadados, subtítulos auxiliares |
| `type.label.small` | 13 | 500 | identificadores, elementos compactos |
| `type.body` | 14 | 400 | labels, menus e configurações |
| `type.body.prominent` | 16 | 500/600 | seções de cartões e cabeçalhos menores |
| `type.heading` | 20 | 600 | títulos de página / seção |
| `type.title` | 24 | 600/700 | títulos principais de Settings |

Valores são propostas iniciais. Ao aumentar escala de texto no sistema, o layout deverá crescer ou quebrar linhas; **não cortar** rótulos importantes para preservar medidas rígidas. Para relógios, contadores e frequências, avaliar numerais tabulares. Evitar letras de largura fixa fora de caminhos/trechos técnicos.

## 20. Geometria e espaçamento — proposta G-01 (PENDENTE)

**Unidades:** px **lógicos** e tokens, nunca medidas fixas em pixels físicos. Compatibilizar com escalas fracionárias, rotações e superfícies de tamanhos diversos.

### 20.1 Escala base e raios candidatos

```yaml
space:
  1: 4
  2: 8
  3: 12
  4: 16
  6: 24
  8: 32
radius:
  xs: 6
  sm: 8
  md: 12
  lg: 16
  xl: 20
stroke:
  subtle: 1
  focus: 2
```

### 20.2 Dimensões exploratórias

| Peça | Dimensão sugerida | Comportamento ao variar largura |
|---|---|---|
| Barra contínua | altura 44–48 px lógicos | largura adaptada ao monitor; respeitar área reservada |
| Ilhas modulares | altura alinhada à barra, padding horizontal 8–12 | âncora esquerda, centro ou direita, nunca sobreposição; compactar antes de reduzir itens críticos |
| Espaço entre ilhas | 8–16 px | manter separação visual mínima; se não couber, aplicar política explícita de compactação |
| Dock integrada | ícones de aplicativos 22–26 | limitar tarefas visíveis com overflow acessível, sem ocultar janelas silenciosamente |
| Dock desacoplada | ícones 24–32 | orientação horizontal/vertical, conteúdo e popover adaptados |
| Workspaces | pílulas com alvo de interação >=36 px | áreas distintas para abrir workspace e escolher modo, sem perder foco por teclado |
| Botões compactos | alvo efetivo >=36–40 px quando couber | não reduzir hit area apenas porque o desenho do ícone é pequeno |
| Launcher Split | largura-alvo 600–720, adaptativa | em telas estreitas reduzir largura e controlar rolagem; o split deve seguir reconhecível |
| Popovers | largura ligada ao conteúdo, máximo útil | sempre abrir dentro dos limites do monitor, sem sobrepor controles críticos |
| Settings | áreas de formulários flexíveis | navegação lateral pode reduzir largura ou recolher em telas estreitas (proposta) |

**Atenção:** 3 ilhas visuais não implicam automaticamente 3 zonas exclusivas no compositor; contrato de camada/área útil ainda precisa de auditoria. A dock pode se desacoplar sem duplicar tarefas (D-02/D-29).

### 20.3 Modos de densidade (candidatos)

- **Confortável (recomendado):** espaçamentos e alvos acima.
- **Compacto (opcional futuro):** reduz espaços visuais, mas preserva áreas interativas utilizáveis.
- Não aprovar, neste momento, editor livre de ilhas nem escala independente arbitrária dos módulos.

## 21. Iconografia e estados visuais — proposta I-01 (PENDENTE)

**Direção:** família coerente de ícones **outline/simbólicos** para ações do sistema, com peso de traço estável. Usar ícones próprios dos aplicativos anunciados pelo `.desktop` sempre que possível; não substituir todos por pictogramas do design system.

| Categoria | Regra visual candidata |
|---|---|
| Navegação do sistema, Settings, status | pictogramas simbólicos da mesma família; 16–20 px |
| Ícones de aplicativos na dock | identidade original do app, 22–26 px integrada; 24–32 px desacoplada |
| Modos Tiling/Scroll/Floating | símbolos diferentes e reconhecíveis em ~14–16 px, junto ao número do workspace |
| Indicador de execução e foco | forma/posição e contraste além de cor |
| Janela minimizada | marcador secundário ou estilo distinto sem sumir com a tarefa |
| Múltiplas janelas | contagem ou marcador múltiplo, com alternativa de tooltip/acessibilidade |
| Atenção / erro | indicador semântico acessível; não depender de animação piscante permanente |
| Desabilitado | conservar legibilidade e explicar limitação, não usar exclusivamente baixa opacidade |

**Candidatos de família:** Lucide, ícones simbólicos do ambiente/GTK, ou conjunto próprio mínimo. Não misturar arbitrariamente vários estilos. A escolha deve considerar licenças e disponibilidade em Lua/Quickshell/GTK4 conforme a implementação da interface oficial, sem pressupor toolkit do shell. Preferir rótulos acessíveis para ícones que possam ser ambíguos, especialmente o modo Scroll e ações sobre vários workspaces.

## 22. Animações e microinterações — proposta M-01 (PENDENTE)

**Direção candidata:** movimento curto, previsível e funcional. Não acoplar a taxa de atualização do monitor à taxa de animação lógica; evitar loops contínuos e efeitos que roubem foco.

| Interação | Duração-alvo exploratória | Curva/condição |
|---|---:|---|
| Hover e realce | 90–140 ms | rápida e discreta |
| Clique/pressionado | 80–120 ms | feedback imediato |
| Menu/popover | 140–200 ms | fade/leve translação; posicionamento estável |
| Launcher abrir/fechar | 180–240 ms | fade + deslocamento/escala mínimos; foco de teclado pronto sem esperar animação |
| Barra contínua ↔ modular | 180–260 ms | manter pontos de ancoragem; sem duplicar barra/dock em transição |
| Dock smart hide | 160–220 ms | sensor de borda confiável, sem revelação indevida em fullscreen |
| Mudança de paleta | 160–280 ms | transição curta e coerente; evitar flicker em wallpaper trocando rapidamente |

**Regras de redução de movimento:** movimento reduzido desativa translações/escala e reduz animações não essenciais a mudanças imediatas ou fades mínimos; feedback de estado permanece presente. Animações da interface não podem interferir em fullscreen real ou capturar input durante jogos. Valores de tempo são **candidatos a teste**, não decisão aprovada.

## 23. Acessibilidade e estados de interação — proposta A11Y-01

1. **Foco visível** independente do acento do wallpaper e de hover; suporte consistente a teclado no launcher, dock, barra e Settings.
2. **Contraste sobre a superfície composta real**, incluindo transparência e wallpaper: metas de referência 4,5:1 para texto comum e 3:1 para componentes essenciais, a testar com temas claros, escuros e extremos.
3. **Diferenciação não cromática:** ativo, minimizado, múltiplas janelas, alerta, selecionado, aplicado/pendente/falha não podem depender só de cor.
4. **Áreas de clique e precisão:** ícones pequenos podem ocupar hit areas maiores, com tratamento de touchpad, mouse e escalas fracionárias.
5. **Tipografia/escala de sistema:** respeitar ampliação do texto, sem truncamento silencioso de opções críticas.
6. **Reduzir transparência/blur/movimento:** opções não devem quebrar o layout nem ocultar controles.
7. **Feedback de ação real:** confirmação de aplicar/salvar/erro no Settings não pode ser confundida com mera animação visual.

## 24. Tokens e contratos entre componentes — proposta DS-API-01

Separar **tokens semânticos** (cor/superfície/estado) de **tokens geométricos** (raios, medidas, espaçamento) e **preferências** (tema, origem do acento, blur, movimento). Exemplo de especificação para futura implementação, sem selecionar stack:

```yaml
# Esquema conceitual — NÃO é um arquivo de configuração do software atual.
appearance:
  mode: wallpaper # wallpaper | light | dark — D-30
  accent: wallpaper # wallpaper | manual — D-32
  panel_material: subtle # D-33: intenção; parâmetros por superfície pendentes
  panel_layout: continuous # continuous | modular — D-29
  reduce_motion: false # preferência opcional, sujeita ao usuário/sistema

tokens:
  surface: {panel: null, popover: null, settings: null}
  on_surface: {primary: null, secondary: null}
  accent: {base: null, on_base: null, focus: null}
  radius: {small: 8, medium: 12, large: 16}
  motion: {fast_ms: 120, normal_ms: 200}
```

A integração GTK4 deve aproveitar o aplicativo existente e, quando necessário, consumir um subconjunto de tokens exportados. A definição de uma API de tokens e seu formato ainda exige auditoria do repositório. Não sobrescrever configurações de rice externo.

## 25. Estado das decisões e critérios de validação

| Identificador | Conteúdo | Estado |
|---|---|---|
| D-28 | Direção V-B, translúcido contido/adaptativo | **Aprovada** |
| D-29 | Barra modular opcional: 3 ilhas, módulos reordenáveis | **Aprovada** |
| D-30 | Tema automático determinado pelo wallpaper | **Aprovada** |
| D-31 | Paleta global pelo wallpaper do monitor principal | **Aprovada** |
| D-32 | Acento extraído do wallpaper com opção manual | **Aprovada** |
| **D-33** | **Transparência leve como preset inicial** | **Aprovada** |
| F-01 | Tema independente por monitor | Backlog futuro, sem compromisso |
| T-01 / G-01 / I-01 / M-01 | Família tipográfica, valores geométricos, iconografia e animações | **Propostas a decidir/testar** |

**Casos de validação necessários antes de fixar tokens:**

- Barra contínua e três ilhas, dock integrada/desacoplada nas bordas possíveis; sem sobreposição no modo modular.
- Wallpapers brancos, pretos, saturados, com alto detalhamento e luminosidade mista; textos/ícones legíveis sobre superfície com alpha.
- Zoom de interface e escalas fracionárias, trabalho em dois monitores e transição de saída principal.
- Estados ativo, minimizado, múltiplas janelas, atenção, erro e selecionado distinguíveis sem cor.
- Launcher acessível exclusivamente por teclado; popovers sem perda de foco e sem clipping.
- Modo de movimento reduzido, sem blur, opacidade total e fullscreen de jogo.
- A interface Settings existente permanece funcional e não é reescrita apenas para obter o visual novo.

**Próxima decisão sugerida:** escolher família tipográfica entre Inter, Noto Sans ou herdar fonte do sistema; depois validar um pequeno conjunto de componentes reais (workspace pill, botão da dock, campo de pesquisa, toggle e popover) antes de fechar ícones e animações.


---

## 26. D-34 — Usar a fonte configurada no sistema (APROVADA em 28/10/2026)

**Decisão expressa:** o usuário escolheu **C — fonte configurada no sistema** para a interface oficial. Não será imposta Inter, Noto Sans ou outra fonte específica como identidade visual obrigatória. Essa decisão refere-se ao **rice oficial** e não pretende substituir as preferências de rices de terceiros.

### 26.1. Contrato de comportamento

1. A família tipográfica da barra, dock, launcher, menus e popovers oficiais acompanha a **preferência efetiva de fonte de interface do ambiente**, desde que possa ser obtida por uma integração suportada. O Settings existente em Rust/GTK4 continua respeitando sua configuração nativa de fonte; não se exige sincronização por sobreposição de CSS arbitrário.
2. Se diferentes componentes expuserem preferências divergentes, o produto deve escolher explicitamente um mecanismo de resolução e informar qual fonte está efetiva; **não presumir que exista uma única fonte global garantida em todas as distribuições**.
3. Se a preferência não estiver disponível, recorrer a uma **fonte sans-serif genérica resolvida pelo sistema**, seguida por fallback que cubra acentos em português, símbolos e escrita internacional. A família concreta de fallback não fica fixada pela D-34.
4. Alterações de fonte deverão atualizar a interface quando o sistema fornecer evento confiável; caso contrário, pode ser necessária atualização da sessão de UI. Essa questão é **implementação a verificar**, não comportamento já presente.
5. O ajuste da fonte não pode forçar alterações de configuração, pacotes de fontes ou temas nos rices externos. A preferência do usuário **não autoriza redistribuir fontes**.
6. A hierarquia tipográfica continua definida por **papéis semânticos** (caption, body, title), não pela métrica de uma família específica. Quando a família ou escala mudar, permitir expansão, truncamento informado ou responsividade, sem cortar ações críticas.

### 26.2. Tokens e tela de preferências — PROPOSTA

```yaml
# Taxonomia conceitual de design; NÃO É API EXISTENTE.
typography:
  family: system
  fallback: sans-serif
  source_status: detected  # detected | fallback | unavailable; proposta
  roles:
    caption: {size_lpx: 12, weight: regular}
    label: {size_lpx: 13, weight: medium}
    body: {size_lpx: 14, weight: regular}
    prominent: {size_lpx: 16, weight: medium}
    heading: {size_lpx: 20, weight: semibold}
    title: {size_lpx: 24, weight: semibold}
```

Tamanhos em **px lógicos ilustrativos**, não aprobados como valores finais. Recomendação de Settings → Interface → Aparência → Tipografia: exibir **“Fonte: Seguir sistema”** com nome efetivamente resolvido apenas quando puder ser verificado. Um override específico do rice poderá ser estudado, mas **não foi aprovado**. Em GTK4, reutilizar os widgets/estilos existentes antes de criar uma camada visual paralela.

### 26.3. Casos de teste

- Alterar a fonte de interface e verificar barra, dock, launcher, popovers, menus e Settings; relatar diferenças de integração honestamente.
- Testar famílias estreitas e largas, fonte com caracteres faltantes, UI com 125%/150% de escala e texto ampliado.
- Confirmar que pílulas de workspace e relógio não são recortados; atalhos essenciais conservam área clicável.
- Verificar tema claro, escuro, acento manual e wallpaper sem reformatar indevidamente o conteúdo.

**Precedência documental:** a seção 19 continua como **histórico de opções**. A decisão atual é **D-34: fonte do sistema**, não Inter.

## 27. Proposta I-02 — Sistema de ícones e indicadores visuais (A VALIDAR)

### 27.1. Linguagem proposta

| Superfície | Proposta | Racional |
|---|---|---|
| Ações do sistema / launcher / Settings | Um conjunto consistente de ícones **outline/simbólicos**, traço regular, alinhado à grade | Identidade contínua sem desenho excessivo |
| Aplicativos da dock e do launcher | **Ícones reais do aplicativo**, resolvidos pelos metadados disponíveis; fallback neutro quando ausente | Preserva reconhecimento e evita ícones falsos |
| Modos dos workspaces | **Três pictogramas inequívocos**: Tiling (divisões em grade), Scroll (colunas/faixa horizontal), Floating (janelas sobrepostas) | D-04; VXWM não integra a primeira entrega |
| Notificações, rede, energia, bloqueio | Símbolos próprios das ações/estados; legenda e tooltip onde necessário | Evita ambiguidades do sistema |
| Temas de ícones alternativos | Podem ser investigados como preferência do rice, **sem exigir** a troca de ícones de terceiros | D-06: sem modularidade entre rices |

**Bibliotecas candidatas, não aprovadas:** Lucide para o rice oficial e ícones simbólicos do tema do ambiente para superfícies GTK4. É necessário verificar correspondência visual, disponibilidade, licenças e consistência de espessura antes de escolhê-las.

**Dimensões exploratórias:** controles/estado 16–20 px lógicos; símbolos de modo 14–16; aplicativos na dock integrada 22–26 e desacoplada 24–32. Usar área de interação maior do que a arte do ícone. Nenhum valor está definitivamente aprovado.

### 27.2. Vocabulário de estados — proposta determinística

A representação precisa indicar o **estado observado pelo gerenciador**, jamais supor que uma ação foi aplicada porque o ícone animou. Estados como execução, foco, minimização, contagem e atenção devem ser combináveis, sem depender apenas da cor de acento.

| Elemento | Estado | Indicador proposto | Interação relevante |
|---|---|---|---|
| Favorito fixado sem janela no workspace ativo | Atalho disponível | Ícone comum **sem** marcador local | Abre no workspace atual; se existir só em outro workspace, solicitar nova janela (D-15) |
| Aplicativo em execução, uma janela local sem foco | Aberto | **Ponto preenchido** sob o ícone | Clique traz para frente |
| Janela local focada | Ativa | **Traço curto sólido** sob o ícone e/ou superfície elevada | No Floating, clique na janela única focada minimiza (D-14) |
| Janela única minimizada | Minimizada | **Ponto/traço vazado**, não invisível | Clique restaura e foca no Floating (D-14) |
| Grupo com N janelas locais | Múltiplas janelas | Marcador de contagem **N** quando couber; no compacto, indicador múltiplo e tooltip | Clique abre seletor; prévias híbridas D-17 |
| Grupo com alguma janela em foco | Grupo ativo | Indicador ativo combinado à contagem | Seleção focalizada destacada no seletor |
| Aplicativo solicitando atenção | Atenção | Pequeno símbolo **!** ou marcador de estado com descrição; **sem piscar indefinidamente** | Ativação normal, sem roubar foco automático |
| Fav + tarefas com agrupamento desativado | Separados | Favorito mantém posição própria; cada janela local usa indicador individual | D-18; ícones repetidos são intencionais |
| Janela só em outro workspace | Fora do escopo local | Não criar tarefa local; favorito pode continuar visível sem indicador de janela local | D-13 e D-15; possível informação contextual explicitamente rotulada |
| Item indisponível | Desabilitado | Pictograma e rótulo explicativos, não só opacidade | Não executar ação inexistente |

**Observação:** a aplicação real de indicador minimizado depende da preservação correta do workspace de origem, ponto já identificado como sensível na auditoria anterior. A resolução técnica não é presumida.

### 27.3. Pílulas de workspace e foco

- Mostrar **número + pictograma do modo** (Tiling/Scroll/Floating) sempre, conforme D-04.
- Área para ativar workspace e área para escolher modo devem ser distinguíveis por hit-testing e navegação por teclado, sem dividir o alvo em regiões pequenas demais.
- **Workspace ativo:** superfície realçada e borda/contorno; **inativo:** superfície neutra. Ocupado, vazio ou solicitando atenção podem ser comunicados por ícones/indicadores complementares, não somente cor.
- **Foco por teclado:** contorno externo perceptível que não se confunde com indicador de workspace ativo; mesma semântica no launcher e Settings.
- **Modo de edição de ilha:** identificar o estado de reordenação para evitar iniciar aplicativos ao tentar mover um módulo (D-29); gesto exato permanece em aberto.

### 27.4. Semântica e acessibilidade

- Labels acessíveis derivados do estado real, como “Navegador, três janelas neste workspace, uma minimizada”.
- Se o texto não couber, tooltip e UI de seleção detalhada; nunca perder contagem ou estado em função exclusivamente de cor.
- Desenhar e testar os símbolos em 100%, 125%, 150%, HiDPI e tema claro/escuro. Usar formas legíveis em 16 px lógicos.
- Conteúdo de notificação ou miniaturas deve respeitar privacidade; hover não altera foco ou exposição de janela automaticamente.

## 28. Proposta M-02 — Sistema de movimento e animações (A VALIDAR)

Objetivo: comunicar mudança de **estado**, não simular uma alteração que não foi confirmada. Movimento discreto, responsivo e sem interferência com os efeitos do Hyprland.

| Evento | Transição proposta | Duração inicial sugerida | Fallback/restrição |
|---|---|---:|---|
| Hover/foco de ícone | Transição de superfície e contorno | 90–140 ms | Foco por teclado sempre visível |
| Clique/pressionado | Realce imediato, sem bounce | 80–120 ms | Não afirmar sucesso antecipadamente |
| Seletor de janelas e menu contextual | Fade + deslocamento mínimo para a área útil | 140–200 ms | Não cortar popover nas ilhas |
| Launcher centralizado/ancorado | Fade + transformação mínima | 180–240 ms | Campo disponível para digitação imediatamente |
| Alternar barra contínua/modular | Reorganização coordenada das superfícies | 180–260 ms | Sem dock duplicada ou perda de hit area |
| Dock smart-hide | Recolher/revelar a superfície elegível | 160–220 ms | Nunca revelar indevidamente sobre fullscreen real |
| Troca de tema/acento | Interpolação de tokens de cor, quando suportada | 160–280 ms | Aplicação atômica ou fallback sem flicker |
| Mudança de modo de workspace | Atualização do pictograma após confirmação | 100–180 ms | Não animar falsas mudanças de layout |

Todos os **tempos são hipóteses para prototipagem**, não valores aprovados. A implementação real pode exigir limites de desempenho, evitar animação simultânea em muitas superfícies e adaptar-se a monitores de diferentes taxas de atualização.

**Modo reduzir movimento (proposta):** desativar translação e escala não essenciais, não piscar, manter foco/estado por mudanças instantâneas ou fades breves. No fullscreen, respeitar o controle do compositor e não ativar sensores de borda involuntariamente. No Settings, desabilitar efeitos com alto custo e não duplicar animações de geometria que o compositor já esteja executando.

**Regra de fail-safe:** primeiro confirmar resultado do gerenciador ou serviço; depois mostrar indicador de estado. Animação de solicitação pode indicar processamento, mas nunca ser tratada como sucesso.

## 29. Critérios de validação visual — antes dos mockups finais

| Verificação | O que deve ser demonstrado |
|---|---|
| Tema | Claro e escuro automáticos pelo wallpaper principal, com um acento legível e override manual |
| Fonte | Pelo menos duas fontes de sistema com métricas diferentes, sem cortes críticos |
| Barra | Contínua padrão e modular opcional (três ilhas), com igual semântica e nenhuma ilha sobreposta |
| Dock | Integrada e desacoplada, agrupamento ligado/desligado, favoritos vs tarefas e minimização no Floating |
| Estados | Aberto, focado, minimizado, múltiplo, atenção, item desabilitado, foco pelo teclado |
| Modos | Pílula com número + Tiling / Scroll / Floating claramente diferenciados |
| Launcher | Split centralizado e ancorado, pesquisa global e prévias/menus por dentro do monitor |
| Settings | Sidebar adaptativa com rice externo; controles GTK4 nativos preservados |
| Movimento | Transições não intrusivas, motion reduzido e fullscreen sem interfaces inesperadas |
| Resolução | Monitor estreito, 1080p, 4K, HiDPI e escalas fracionárias |

**Status ao final desta etapa:** D-34 aprovada. I-02 e M-02 são **propostas** prontas para comparação visual, não decisões de biblioteca/duração exata. Dimensões de G-01 continuam em avaliação.


---

## 30. D-35 — Família visual consistente para ícones do rice oficial (APROVADA)

**Data:** 28/10/2026. O usuário selecionou a alternativa **A — família consistente de ícones lineares para o sistema**, preservando ícones originais dos aplicativos.

### 30.1 Contrato do design

| Área | Política | Estado |
|---|---|---|
| Controles de sistema do rice oficial | Ícones lineares/simbólicos com gramática e peso coerentes | **Aprovado (D-35)** |
| Aplicativos | Manter o ícone instalado/original; não converter logotipos em outline | **Aprovado (D-35)** |
| Workspaces | Número e pictograma do modo em **cada entrada individual** | **Aprovado anteriormente (D-04)** |
| Modos | Pictogramas diferentes para Tiling, Scroll e Floating | **Obrigação funcional**; desenhos específicos a validar |
| Estados da dock | Sinais distintos para aberto, ativo, minimizado, grupo e atenção | **Proposta visual** I-02; validar no mockup |
| Settings GTK4 | Aproveitar símbolos e widgets sem sacrificar semântica ou acessibilidade | Diretriz de implementação; auditar código |
| Rice externo | Não sobrescrever seus ícones ou temas | Regra de integração existente |

**Famílias candidatas, não aprovadas:** Lucide e ícones simbólicos da plataforma GTK/Adwaita. Evitar impor dependência até verificar compatibilidade, licenciamento, qualidade de desenhos a 16–24 px lógicos e integração real.

### 30.2 Prancha inicial de comparação M-01/M-02

![Conceito da barra contínua e modular](MOCKUPS_COMPARACAO_BARRAS_v0.1.png)

**Estado:** primeiro estudo gerado para comparação, não mockup final aceito. A prancha tem duas vistas sobre o mesmo wallpaper: (A) superfície inferior contínua; (B) três ilhas separadas, ancoradas. O material quase opaco e a paleta violeta seguem V-B/D-33/D-30–D-32, mas cores exatas ainda são ilustrativas.

**Notas de revisão visual obrigatórias:**

1. A prancha **simplifica indevidamente** a relação número+ícone de modo em cada workspace; corrigir no mockup refinado.
2. A primeira comparação foca painéis sobre wallpaper; o próximo estudo terá janelas abertas em Floating/Tiling para avaliar hierarquia espacial e sobreposição.
3. O conjunto de ícones de aplicativo visível é apenas ilustrativo, não uma lista obrigatória de aplicativos da suíte.
4. Pictogramas, agrupamento, badges e microinterações mostrados ainda não são especificações finais.
5. A busca permanente no topo do wallpaper de referência externo **não** foi adotada; o launcher continua sob demanda, Split, centralizado por padrão (D-19–D-22).

### 30.3 Critérios de aceite após iteração

- [ ] Cada pílula indica número + Tiling/Scroll/Floating sem ambiguidade.
- [ ] Barra contínua e três ilhas modular utilizam **os mesmos aplicativos, indicadores e workspaces**.
- [ ] Dock desacoplada não duplica a ilha central.
- [ ] Janelas em Floating não ocultam status indevidamente.
- [ ] Tipografia permanece substituível pela fonte do sistema.
- [ ] As cores mantêm contraste em um wallpaper escuro e outro claro.
- [ ] Os estados não dependem somente de cor.


---

## 31. Mockups refinados M-01/M-02 (v0.2) — validação visual

**Arquivos:** `MOCKUPS_REFINADOS_DESKTOP_v0.2.png`, `MOCKUPS_REFINADOS_DESKTOP_v0.2.md` e recortes M-01/M-02.  
**Status:** material para discussão, **não nova decisão**.

O layout **contínuo inferior** e o layout **modular de três ilhas** foram comparados com wallpaper e janelas equivalentes em Floating. A direção V-B, a transparência leve, a fonte configurada no sistema e os ícones lineares de controles permanecem decisões vigentes.

**Débito visual detectado:** o pequeno conjunto de workspaces na barra gerada ainda não apresenta satisfatoriamente `número + pictograma de modo` para cada item; também há risco de a cena de janelas flutuantes aparentar ter um workspace Tiling ativo. Os mockups são **ilustrações**, não prova de cumprimento técnico. O desenho final deve corrigir isso, evidenciar foco real e separar hover de estado aplicado.

Os quadros suplementares de launcher e Settings **não ampliam automaticamente o escopo do MVP**: indexação de arquivos e telas inéditas não foram aprovadas; Settings deve preservar Rust/GTK4 existente. As miniaturas e demais detalhes permanecem propostas a validar. Não foram alteradas D-01–D-35.


---

## 32. Dois novos elementos de navegação — Show Desktop e Visão Geral (D-36/D-37)

**Estado:** inclusão funcional solicitada; iconografia/posição exata ainda propostas.

### Barra contínua

- Lado esquerdo: launcher, pílulas de workspaces e **ícone Visão Geral** candidato próximo às pílulas; evitar que ambos usem o mesmo pictograma de grade.
- Meio: dock integrada (inalterada).
- Direita: indicadores/relógio, atalho Monitores (opcional) e botão **Mostrar área de trabalho** (candidato: extremidade direita).

### Barra modular (três ilhas)

- Ilha esquerda: launcher, pílulas e **Visão Geral**.
- Ilha central: dock (sem alterações de escopo e agrupamento).
- Ilha direita: conectividade/status/relógio + **Mostrar área de trabalho** (reordenável e opcional na composição da ilha).
- Desacoplar a dock não desloca automaticamente Visão Geral nem Mostrar área de trabalho.

### Diretrizes visuais

- Dois ícones com significados distintos: Visão Geral = organização espacial de workspaces/janelas; Mostrar área de trabalho = revelar fundo, **não um monitor para ajustar resoluções**.
- Rótulos acessíveis obrigatórios, indicadores de ação quando Show Desktop está ativo, estado de foco por teclado e ausência de piscadas.
- Modalidade Visão Geral Todos/Atual deve compartilhar o design system V-B: quase opaco, adaptativo por wallpaper principal, tipografia do sistema e estados claros.
- Miniaturas são conteúdo auxiliar, com tratamento de ausência de captura. Exibir número + modo em cada workspace inclusive na miniatura.

### Próxima prancha

Revisar M-01/M-02 para acrescentar ambos os controles; criar M-06 (Visão geral — Todos), M-07 (Visão geral — Atual) e M-08 (Área de trabalho revelada). Não desenhar dashboard. Não duplicar lançador/pesquisa no topo. Arte é referência, não prova de integração ao Hyprland.


## 28. D-38 — Visão Geral: entrada em Todos os workspaces (APROVADA)

- Comando distinto do **Launcher Split**: ícone linear para **Visão Geral** na ilha de navegação (barra contínua: junto a workspaces); **Mostrar área de trabalho** na região de status/ilha direita.
- O nível inicial aprovado é **Todos os workspaces**, com identificadores **número + modo** claros em cada cartão. O nível **Workspace atual** permanece acessível por alternância.
- Tema automático e acento derivados do wallpaper principal, transparência leve, pictogramas lineares, tipografia do sistema e opções de barra contínua/modular permanecem os mesmos.
- Estados ilustrativos dos botões: normal, hover, foco de teclado e acionado. Um ícone deve comunicar a Visão Geral, e outro a revelação temporária da área de trabalho.
- **Cautela:** rótulos e atalhos nos mockups são ilustrativos; gráficos de janelas e miniaturas não são capturas reais. Não aprovar por inferência tempos de animação, posição em pixels nem tecnologia de implementação.
- **Janelas:** em Mostrar área de trabalho, não transformar as janelas ocultas em tarefas permanentemente minimizadas na dock. A Visão Geral não deve modificar layouts ao navegar.

**Mockups:** `MOCKUP_VISUALIZACAO_E_AREA_DE_TRABALHO_v0.1.png` e `MOCKUP_ESTUDOS_VISUALIZACAO_DETALHES_v0.1.png`. Essas pranchas são CONCEITOS e não representam uma instalação funcional.


---

## 29. P-12 — Padrão visual de cartão da Visão Geral (proposta)

Cartão: identificador de workspace, ícone Tiling/Scroll/Floating, indicador de ativo, monitor quando necessário, composição de janelas quando possível, contagem e botão secundário **Ir para workspace**. O corpo do cartão é a área de **inspeção**, não de ativação imediata.

No detalhe: ação **Voltar a todos**, miniaturas/listas selecionáveis com estado de janela (ativa, minimizada, fora do viewport no Scroll), comandos com foco visível. Em resolução menor, usar grade responsiva; em ausência de miniaturas, usar cartões de texto e ícones.

Estados de UI: normal, hover, foco, selecionado, carregando miniatura, indisponível, vazio, falha. Não equiparar hover a seleção nem a ativação de workspace. O modo visual V-B, transparência leve, fonte do sistema e ícones lineares permanecem D-28/D-33/D-34/D-35.

**Pendente de aprovação:** comportamento do clique do cartão (P-12). **Sem novas decisões D.**


---

## 27. D-39 / P-13 — Visual da Visão Geral em múltiplos monitores

**D-39 aprovada:** o cartão é uma superfície de inspeção; a ação secundária `Ir para workspace` é uma chamada explícita de ativação. Exigir estados de foco visíveis (borda/realce), não confundir hover com troca de workspace.

**P-13 proposta:** cabeçalhos de seção com identificação do monitor (principal/secundário ou nome real quando disponível), contagem de workspaces e cartões de número + modo + prévia/fallback. Opção de filtro compacto por monitor; uma única superfície centrada no monitor de interação como hipótese inicial. Não impor que o tema mude por monitor; D-31 é global.

**Geometria/responsividade:** cartões de mesma gramática visual entre monitores, grade adaptável ao espaço lógico disponível, respeitando monitores verticais e HiDPI. Sem exclusão silenciosa de workspaces; lista rolável quando não houver espaço. Foco pelo teclado distinto do workspace realmente ativo.

**Estados:** monitor desconectado -> atualização da topologia e remoção/aviso, não seleção inválida; janela fechada -> invalidação da prévia sem troca de alvo; minimizada -> selo de estado sem falsificar thumbnail; workspace vazio -> entrada navegável com `Ir para workspace`.

**Ainda em aberto:** escopo multimonitor inicial, número de superfícies, gestos, tamanhos, animações, capturas e suporte de runtime; nenhum mockup implica implementação concluída.


---

## D-40 — Refinamento visual da Visão Geral multimonitor

**Aprovado:** workspaces agrupados sob cabeçalhos dos monitores, com filtros opcionais. O agrupamento é informação funcional, não uma mudança na paleta.

**Tokens/composição propostos:** cabeçalho do grupo com ícone de display, rótulo e contagem; cartões com número, indicador Tiling/Scroll/Floating, miniatura de janela ou fallback de privacidade, botão secundário `Ir`; contorno discreto de workspace ativo, contraste independente da cor de acento. O card inteiro abre detalhes (D-39). Evitar que os botões secundários pareçam o único destino clicável.

**P-14 (preferência de projeto não aprovada):** overlay único no monitor em foco, com largura adaptativa e agrupamento vertical. Em displays menores, reduzir colunas ou permitir rolagem interna, sem truncar botões nem capturas. O monitor B não ganha uma segunda camada enquanto a UI está em A.

**P-15 (proposta alternativa):** painéis locais em monitores distintos, preservando o mesmo vocabulário visual. Precisa resolver foco de teclado, filtros e leitura global da organização sem presumir que a presença física de telas coincide com a numeração de workspaces.

Os dois exemplos devem utilizar a mesma lista de workspaces A: 1–3, B: 4–5; wallpaper roxo em ambos; tema global D-31 e transparência leve D-33. **Arte de conceito não demonstra capturas em tempo real**. Não incluir funcionalidades futuras de arrastar janelas entre monitores como se estivessem aprovadas.


---

## D-41 — Apresentação multimonitor da Visão Geral

**Aprovado:** uma superfície de Visão Geral somente no monitor em foco, apresentando os grupos de workspaces dos demais monitores na própria interface. Conservar tema global D-31, materiais discretos D-33, ícones lineares D-35 e número+modo por cartão D-04. Ao filtrar outra tela ou selecionar cartão, não representar visualmente um deslocamento de foco real antes da ação `Ir`/seleção de janela. O monitor do cartão pode ser marcado por legenda discreta, não por mudança de paleta por monitor (F-01 segue futura).

**Proposta de estados:** foco de teclado no overlay (anel de foco visível), cartão selecionado (realce diferente do workspace ativo), monitor de origem (título) e destino de navegação (destaque de seleção). Reduzir movimento conforme preferência. Em Mostrar área de trabalho, o ícone acionado deve indicar `temporariamente revelado`, sem mostrar ícones de aplicativos como minimizados. Escopo multimonitor D-42 pendente.


---

## D-42 — Estados visuais de Mostrar área de trabalho em múltiplos monitores

**Aprovado:** somente o workspace ativo no monitor em foco é revelado temporariamente. Na barra contínua, o botão permanece na seção de sistema; na barra modular, permanece na ilha direita. O indicador só deve acender no escopo efetivamente ativo; os demais monitores não mostram falsos estados de minimização.

**Estados recomendados:** Normal, Hover, Revelado temporariamente, Restaurando, Falha/indisponível. O botão deve receber indicação acessível de estado; não depende exclusivamente da cor de acento. Para Visão Geral: Monitor hospedeiro, Workspace explorado (seleção), Workspace efetivamente ativo e Janela focada devem possuir indicações diferenciadas.

**No mockup M-10:** comparar monitores A e B com a mesma configuração antes da ação; após ativar Mostrar área de trabalho em A, B fica **visualmente idêntico**, e as janelas de A continuam no inventário da Visão Geral. Um quadro de restauração deve mostrar que uma janela que já estava minimizada permanece minimizada.

**Referência de fluxos:** `CONTRATO_VISAO_GERAL_FOCO_RESTAURACAO_v1.0.md`. Duração de animações, desenho final de ícones e implementação técnica seguem propostas.


---

## M-10 — Semântica visual do foco e da revelação (sem nova decisão)

Os estados e tokens cromáticos de D-28/D-30/D-31/D-33/D-34/D-35 continuam válidos: wallpaper principal define tema global, transparência leve e ícones lineares do sistema.

- **A2 revelado:** botão Mostrar área de trabalho aparece pressionado **na barra do monitor A**, não na do B; a dock não marca todos os apps como minimizados.
- **Visão Geral no A:** apenas A exibe overlay; workspaces B estão dentro da grade do overlay; B mantém as janelas visíveis ao fundo.
- **Cartão B5 explorado:** realce de seleção de UI, sem confundir com foco efetivo.
- **Foco transferido explicitamente:** sinalizar destino somente após ativação confirmada; não deslocar workspace entre monitores.
- **Restaurar A2:** recolocar a disposição anterior e manter as janelas já minimizadas assim.

Referências exploratórias: `MOCKUP_M10_FLUXO_E_RESTAURACAO_v0.1.png` e `MOCKUP_M10_COMPARACAO_4_QUADROS_v0.1.png`. Elas não demonstram transições implementadas nem perfeição visual entre estados.
