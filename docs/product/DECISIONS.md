# Registro de decisões de produto — D-01 a D-45

Consolidado em **29/10/2026**, a partir do pacote externo de **28/10/2026**. Este registro conserva os IDs e o grau de aprovação informado pela fonte; não produz nova aprovação técnica, implementação ou identidade pública. Não há transcrição integral da conversa externa no pacote.

**Escopo da validação:** nesta consolidação houve conferência documental e comparação estática com a implementação no commit `a2506c63dad16384414dc95919b8292aead284df`. Não foram executados testes de produto, testes simulados, testes ao vivo ou atualização do Hyprland. Resultados históricos do gerenciador continuam datados em [STATUS][status] e no [acervo de auditoria][audit].

As entradas abaixo separam **decisão/aprovação**, **implementação existente** e **validação**. O campo de implementação não transforma o OpenSpec em descrição automática do código: divergências contratuais permanecem registradas no estado técnico. Detalhes operacionais de produto estão em [INTERFACE](INTERFACE.md), [SETTINGS](SETTINGS.md), [WORKSPACES](WORKSPACES.md) e [DESIGN](DESIGN.md); alternativas em [PROPOSALS](PROPOSALS.md).

## Índice

[D-01](#d-01) | [D-02](#d-02) | [D-03](#d-03) | [D-04](#d-04) | [D-05](#d-05) | [D-06](#d-06) | [D-07](#d-07) | [D-08](#d-08) | [D-09](#d-09) | [D-10](#d-10) | [D-11](#d-11) | [D-12](#d-12) | [D-13](#d-13) | [D-14](#d-14) | [D-15](#d-15) | [D-16](#d-16) | [D-17](#d-17) | [D-18](#d-18) | [D-19](#d-19) | [D-20](#d-20) | [D-21](#d-21) | [D-22](#d-22) | [D-23](#d-23) | [D-24](#d-24) | [D-25](#d-25) | [D-26](#d-26) | [D-27](#d-27) | [D-28](#d-28) | [D-29](#d-29) | [D-30](#d-30) | [D-31](#d-31) | [D-32](#d-32) | [D-33](#d-33) | [D-34](#d-34) | [D-35](#d-35) | [D-36](#d-36) | [D-37](#d-37) | [D-38](#d-38) | [D-39](#d-39) | [D-40](#d-40) | [D-41](#d-41) | [D-42](#d-42) | [D-43](#d-43) | [D-44](#d-44) | [D-45](#d-45)

## Decisões

<a id="d-01"></a>

### D-01 — Rice oficial

- **Decisão:** Interface visual concebida como **rice padrão da futura suíte Arch**
- **Aprovação conforme a fonte:** Aprovada
- **Implementação existente:** Não existe rice oficial implementado no Kwinish; existe o gerenciador em Lua/C++/Rust. Ver [arquitetura atual][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.

<a id="d-02"></a>

### D-02 — Separação barra/dock

- **Decisão:** **Barra e dock** são conceitos distintos; a dock pode ficar **integrada à barra ou desacoplada**, com possibilidade de posicionamento separado (inferior, lateral etc.)
- **Aprovação conforme a fonte:** Diretriz adotada; detalhes a definir
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** A diretriz não define todas as posições, geometria ou certificação de rices externos.

<a id="d-03"></a>

### D-03 — Ocultação inteligente

- **Decisão:** **Ocultação inteligente** para a dock
- **Aprovação conforme a fonte:** Aprovada; casos de borda a especificar
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Aprovação da ocultação inteligente não escolhe regras de fullscreen, integração, sensores ou atrasos.

<a id="d-04"></a>

### D-04 — Modo integrado aos workspaces

- **Decisão:** Workspaces apresentam **indicador de modo integrado**, em vez de seletor completamente separado
- **Aprovação conforme a fonte:** Aprovada
- **Implementação existente:** Os três modos por workspace existem em [core/modes.lua][modes]; indicador integrado na barra oficial não existe.
- **Validação:** Inspeção estática e conferência documental; não houve novo teste funcional. Evidências históricas do núcleo não validam a futura suíte.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.

<a id="d-05"></a>

### D-05 — Módulos opcionais

- **Decisão:** **Configurações** e **monitores** devem estar entre os módulos **opcionais** da área de status da barra
- **Aprovação conforme a fonte:** Aprovada
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.

<a id="d-06"></a>

### D-06 — Rices completos

- **Decisão:** Na primeira versão da Rice Store, **selecionar e ativar um rice completo como conjunto**; não misturar componentes de rices diferentes
- **Aprovação conforme a fonte:** Aprovada
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.

<a id="d-07"></a>

### D-07 — Dashboard fora de escopo

- **Decisão:** **Dashboard fora do escopo por enquanto**
- **Aprovação conforme a fonte:** Aprovada
- **Implementação existente:** Exclusão de escopo de produto; dashboard não é componente da implementação documentada.
- **Validação:** Conferência do registro e do limite de escopo; não é resultado de teste funcional.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.

<a id="d-08"></a>

### D-08 — Independência do gerenciador

- **Decisão:** Gerenciador de janelas e configurações fundamentais não devem depender do rice ativo
- **Aprovação conforme a fonte:** Direção arquitetural consolidada na conversa
- **Implementação existente:** O gerenciador atual e suas views existem sem barra/dock oficiais. Uma API pública de rices e a independência operacional completa da futura suíte ainda não foram implementadas; ver [arquitetura][architecture].
- **Validação:** Inspeção estática e conferência documental; não houve novo teste funcional. Evidências históricas do núcleo não validam a futura suíte.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.

<a id="d-09"></a>

### D-09 — Identidade em aberto

- **Decisão:** O nome público do projeto **ainda não está decidido**
- **Aprovação conforme a fonte:** Aprovada
- **Implementação existente:** Registro de decisão de identidade. Esta consolidação conserva o identificador do repositório e não escolhe nome público.
- **Validação:** Conferência do registro e do limite de escopo; não é resultado de teste funcional.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.

<a id="d-10"></a>

### D-10 — Canvas futuro

- **Decisão:** O modo Canvas/VXWM inspirado no driftwm é **visão futura**, não parte do núcleo inicial
- **Aprovação conforme a fonte:** Aprovada
- **Implementação existente:** Canvas/VXWM não é implementado pelo núcleo atual; Tiling, Scroll e Floating estão descritos na [arquitetura][architecture].
- **Validação:** Conferência do registro e do limite de escopo; não é resultado de teste funcional.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.

<a id="d-11"></a>

### D-11 — Composição inicial

- **Decisão:** **Configuração inicial do rice oficial:** barra inferior com dock integrada (opção A)
- **Aprovação conforme a fonte:** Aprovada em 28/10/2026
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.

<a id="d-12"></a>

### D-12 — Escopo do desacoplamento

- **Decisão:** A capacidade de desacoplar a dock **pertence ao rice oficial**; não é exigência dos rices externos. Um rice completo com barra/taskbar integrada fixa é admissível, desde que cumpra os requisitos funcionais do Floating
- **Aprovação conforme a fonte:** Esclarecimento aprovado pela escolha de escopo; detalhes de certificação propostos
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Não exigir que rices externos ofereçam uma superfície Wayland de dock separada ou o editor do rice oficial.

<a id="d-13"></a>

### D-13 — Contexto da dock

- **Decisão:** **Escopo da dock oficial:** mostrar aplicativos com janelas pertencentes ao **workspace ativo**, além dos **favoritos fixados**. A mudança de workspace atualiza a lista contextual.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026**
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.

<a id="d-14"></a>

### D-14 — Clique em janela única no Floating

- **Decisão:** **Clique esquerdo em aplicativo com uma única janela local, no modo Floating:** alternar entre minimizar (quando já está focada) e restaurar (quando minimizada). Se estiver aberta, mas sem foco, primeiro focá-la.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026**; a regra de foco intermediário é esclarecimento operacional proposto
- **Implementação existente:** Há ações de minimizar e restauração no núcleo ([API atual][api]); não há dock oficial que implemente essa interação.
- **Validação:** Inspeção estática e conferência documental; não houve novo teste funcional. Evidências históricas do núcleo não validam a futura suíte.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Conservar como proposta o foco intermediário para janela aberta sem foco; não o promover ao mesmo grau de aprovação da alternância. No estado revelado, aplica-se a precedência limitada da D-45.

<a id="d-15"></a>

### D-15 — Favorito remoto

- **Decisão:** **Favorito cujo aplicativo só está aberto em outro workspace:** tentar abrir **nova janela no workspace atual**, sem navegar automaticamente à janela remota.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026**; limites técnicos e single-instance a validar
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Aplicações single-instance podem recusar uma nova janela; D-15 aprova a tentativa, sem garantir comportamento de terceiros.

<a id="d-16"></a>

### D-16 — Agrupamento configurável

- **Decisão:** **Agrupamento de janelas na dock oficial:** agrupar por aplicativo **por padrão**, oferecendo opção de **desativar o agrupamento no Settings**.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026**; interação com grupos e apresentação sem agrupamento são propostas da seção 16
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Gestos, ações em grupo e seleção de janela permanecem propostas.

<a id="d-17"></a>

### D-17 — Pré-visualização condicional

- **Decisão:** **Pré-visualizações híbridas e configuráveis:** lista textual **ou** miniaturas visuais conforme preferência do usuário, **condicionadas ao suporte técnico**; fallback textual quando miniaturas não estiverem disponíveis.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026**; detalhes de acionamento e escolha do padrão continuam propostos na seção 17
- **Implementação existente:** Alt+Tab atual é textual e há outras prévias do gerenciador; isso não implementa pré-visualizações configuráveis da dock oficial.
- **Validação:** Inspeção estática e conferência documental; não houve novo teste funcional. Evidências históricas do núcleo não validam a futura suíte.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Não garantir captura de miniaturas; acionamento e padrão visual permanecem propostos.

<a id="d-18"></a>

### D-18 — Favoritos e tarefas distintos

- **Decisão:** **Favoritos e janelas separados quando o agrupamento está desativado:** os favoritos permanecem como atalhos persistentes e as janelas locais aparecem como entradas individuais distintas na área de tarefas. Aceita-se repetição visual de ícones, com papéis diferentes.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026** (opção A); detalhes de clique no favorito permanecem sujeitos à D-14 e à proposta da seção 18
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Não confundir favorito persistente com tarefa local; detalhes de clique propostos seguem sujeitos às D-14/D-15/D-45.

<a id="d-19"></a>

### D-19 — Launcher híbrido

- **Decisão:** **Launcher híbrido oficial:** combinar **pesquisa imediata por teclado** e **navegação gráfica por aplicativos** na mesma interface.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026**. Estrutura de categorias, priorização de resultados, ações e atalhos continuam **propostas** da seção 19.
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Categorias, layout e busca global foram depois delimitados por D-20/D-21; fontes adicionais e ações não ficam aprovadas em bloco.

<a id="d-20"></a>

### D-20 — Launcher dividido

- **Decisão:** **Tela inicial do launcher híbrido em layout dividido (split):** coluna de categorias à esquerda e grade de aplicativos à direita, com pesquisa imediata no topo.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026.** Local de abertura, ordenação, busca, ações, tamanhos e comportamento responsivo continuam **propostas** da seção 20.
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Dimensões, ordenação, responsividade e navegação detalhada ainda são propostas.

<a id="d-21"></a>

### D-21 — Pesquisa global

- **Decisão:** **Pesquisa global no launcher:** ao digitar, pesquisar em todos os aplicativos e ações disponíveis, **independentemente da categoria visual selecionada**.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026** (opção A); apresentação e ordenação dos resultados ainda são propostas da seção 21.
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Ranking, apresentação e catálogo exato de ações continuam propostos.

<a id="d-22"></a>

### D-22 — Posição do launcher

- **Decisão:** **Posição de abertura do launcher oficial:** **centralizado por padrão**, com opção de abertura **ancorada na barra** pelo Settings.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026** (opção C); regras de fallback, monitor e foco são recomendações de UX da seção 22.
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Fallback de monitor e foco não decorre automaticamente dessa aprovação.

<a id="d-23"></a>

### D-23 — Sidebar adaptativa

- **Decisão:** **Settings com sidebar adaptativa:** páginas fundamentais do Sistema sempre disponíveis; com rice oficial ativo exibir Barra/Dock/Launcher/Aparência; com rice externo substituí-las por `Rice ativo` e oferecer acesso às configurações próprias quando declarado e disponível.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026** (opção A). Detalhes dos controles e fluxos seguem em especificação funcional, não aprovados automaticamente.
- **Implementação existente:** Existe `kwinish-ui settings` para opções do plugin e Snap Assist. Sidebar adaptativa e integração de rices não existem; ver [opções atuais][config].
- **Validação:** Inspeção estática e conferência documental; não houve novo teste funcional. Evidências históricas do núcleo não validam a futura suíte.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Não aprova todos os controles da especificação funcional nem uma reescrita da UI GTK4.

<a id="d-24"></a>

### D-24 — Modo inicial

- **Decisão:** **Modo inicial de novos workspaces:** padrão global Tiling/Scroll/Floating com **exceção opcional por monitor**. Workspaces que já possuam modo atribuído não mudam automaticamente quando o padrão global ou a exceção de monitor é alterada.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026** (opção C). Semântica de primeira criação, identificação do monitor e restauração são propostas operacionais deste complemento, sujeitas à revisão do código.
- **Implementação existente:** O código mantém modo por workspace; padrão global com exceção por monitor não está implementado ([core/modes.lua][modes]).
- **Validação:** Inspeção estática e conferência documental; não houve novo teste funcional. Evidências históricas do núcleo não validam a futura suíte.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Identidade de monitor, primeira criação e persistência são detalhamento técnico a reconciliar.

<a id="d-25"></a>

### D-25 — Perfis híbridos de monitores

- **Decisão:** **Perfis de monitores híbridos:** ativação **manual por padrão**; reconhecimento e ativação **automáticos opcionais**, disponíveis somente após validação de identidade dos monitores, aplicação segura e recuperação efetiva.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026** (opção C). Critérios técnicos e UI são propostas da seção 26.
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Automação é opcional e condicionada à validação; não constitui ativação automática por padrão.

<a id="d-26"></a>

### D-26 — Delegação de inatividade

- **Decisão:** **Responsabilidade pela política de inatividade quando um rice externo estiver ativo:** se houver controlador próprio de idle **identificado e efetivamente disponível**, delegar a ele o controle de bloqueio por inatividade, desligamento de tela e suspensão automática nas funções que ele assumir. A suíte mantém consulta ao estado e acesso às configurações do responsável; **não inicia controlador concorrente**. Sem controlador externo, usar controlador da suíte somente se estiver configurado e verificado.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026.** Protocolo de detecção, transição, recuperação e UI são propostas de detalhamento da seção 27, não funcionalidades já implementadas.
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Detecção, transição, contrato e recuperação não têm protocolo técnico aprovado.

<a id="d-27"></a>

### D-27 — Fallback de idle/lock

- **Decisão:** **Fallback da suíte para inatividade e bloqueio de sessão:** quando o rice ativo não oferecer componentes compatíveis e verificáveis, a suíte fornece o controlador de inatividade e/ou bloqueador necessários; quando houver soluções próprias funcionais, delega-se a responsabilidade correspondente. Não executar políticas concorrentes nem presumir proteção após falha.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026** (opção A). Escolha das ferramentas, handshake, recuperação e protocolos concretos continuam propostas técnicas da seção 28.
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Ferramentas de idle/lock, handshake e recuperação não foram selecionadas.

<a id="d-28"></a>

### D-28 — Direção V-B

- **Decisão:** **Direção estética V-B — translúcido contido e adaptativo** para a interface oficial: sobriedade, superfícies discretamente translúcidas, blur moderado e fallback opaco; tokens, paleta, intensidade dos efeitos e tema padrão ainda não escolhidos.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026** pela opção B. Não estende a identidade visual a rices de terceiros.
- **Implementação existente:** Há opções de cores do plugin e CSS das views GTK. Não existe o design system completo da interface oficial.
- **Validação:** Inspeção estática e conferência documental; não houve novo teste funcional. Evidências históricas do núcleo não validam a futura suíte.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Direção estética não escolhe branding, biblioteca de UI ou tecnologias.

<a id="d-29"></a>

### D-29 — Barra em três ilhas

- **Decisão:** **Modo modular opcional para a barra do rice oficial:** três ilhas com âncoras lógicas à esquerda, centro e direita, contendo módulos que podem ser **reordenados dentro da respectiva ilha**. A barra contínua continua sendo a apresentação inicial. Não autoriza ilhas arbitrárias ou modularidade entre rices de terceiros.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026** pela opção A. Detalhes de editor, limites de espaço, movimento, tokens e implementação continuam como propostas.
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Três âncoras e reordenação dentro da ilha não autorizam editor livre, ilhas arbitrárias ou composição entre rices.

<a id="d-30"></a>

### D-30 — Tema automático

- **Decisão:** **Tema automático inicial orientado pelo wallpaper:** a variante Claro/Escuro do rice oficial é definida pela imagem selecionada, com possibilidade de substituição manual global.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026**; algoritmo e extração de acento ainda pendentes.
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Algoritmo, limiares e implementação do pipeline permanecem pendentes.

<a id="d-31"></a>

### D-31 — Tema global

- **Decisão:** **Tema global com wallpaper do monitor principal como referência:** mesma variante/paleta em toda a interface oficial, ainda que outros monitores tenham wallpapers diferentes.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026** pela opção A. A alternativa de tema independente por monitor é **F-01 (ideia futura, não aprovada para MVP)**.
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** F-01 (tema independente por monitor) continua futuro. Monitor principal não equivale ao monitor em foco.

<a id="d-32"></a>

### D-32 — Acento automático

- **Decisão:** **Cor de acento gerada automaticamente com base no wallpaper principal**, com seleção manual opcional, independente da escolha claro/escuro.
- **Aprovação conforme a fonte:** **Aprovada** em 28/10/2026; algoritmos, tokens cromáticos e ferramenta ainda pendentes.
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Ferramenta, algoritmo e tokens exatos ainda não foram escolhidos.

<a id="d-33"></a>

### D-33 — Transparência leve

- **Decisão:** **Transparência leve (opção A) como material padrão do rice oficial**, preservando direção V-B; superfícies quase opacas, blur discreto/opcional.
- **Aprovação conforme a fonte:** **Aprovada expressamente** em 28/10/2026. Intervalo de 94–97% de opacidade na barra e dock é referência de prototipagem, não parâmetro final obrigatório.
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** 94–97% de opacidade é referência de prototipagem, sem obrigação numérica final.

<a id="d-34"></a>

### D-34 — Fonte do sistema

- **Decisão:** **Tipografia da interface oficial acompanha a fonte configurada no sistema** (opção C); não impor Inter ou Noto Sans, respeitar fallback/acessibilidade e não alterar preferências de rices externos.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026**. Extração técnica da fonte e escala final de tokens permanecem propostas.
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Fonte específica, extração da preferência e escala final de tokens não foram escolhidas.

<a id="d-35"></a>

### D-35 — Iconografia

- **Decisão:** **Ícones de sistema do rice oficial seguem uma família visual consistente, com pictogramas lineares/simbólicos e linguagem uniforme; os ícones originais dos aplicativos são preservados.** O Settings em GTK4 mantém integração nativa e respeita semântica/acessibilidade.
- **Aprovação conforme a fonte:** **Aprovada expressamente em 28/10/2026**. A biblioteca exata (Lucide, Adwaita simbólico etc.), tamanhos de traço, estados e variantes permanecem **propostas sujeitas a mockups e auditoria técnica**.
- **Implementação existente:** O gerenciador já resolve ícones de aplicativos e tem Settings GTK4. A família de ícones do shell oficial não foi escolhida nem implementada.
- **Validação:** Inspeção estática e conferência documental; não houve novo teste funcional. Evidências históricas do núcleo não validam a futura suíte.
- **Proveniência:** [visão v3.13, §2.1][vision], pacote de 28/10/2026.
- **Limites e relações:** Lucide, Adwaita ou outra biblioteca não foram selecionadas; a regra não altera preferências de rices externos.

<a id="d-36"></a>

### D-36 — Mostrar área de trabalho

- **Decisão:** Incorporar à barra do rice oficial um **ícone para Mostrar área de trabalho**, com alternância reversível de visibilidade das janelas, sem destruir o estado dos modos e layouts.
- **Aprovação conforme a fonte:** **Necessidade solicitada pelo usuário.** Posição exata, ícone, atalho, escopo multimonitor e estratégia técnica não aprovados.
- **Implementação existente:** `show_desktop` existe no [núcleo][core], mas usa conjunto `stashed` global e volátil; não há botão da barra oficial nem contrato completo D-42–D-45.
- **Validação:** Inspeção estática e conferência documental; não houve novo teste funcional. Evidências históricas do núcleo não validam a futura suíte.
- **Proveniência:** [visão v3.13, Complemento de decisões; §§39–44][vision], pacote de 28/10/2026.
- **Limites e relações:** O escopo multimonitor foi delimitado depois por D-42; ícone, posição e atalho exatos seguem em aberto.

<a id="d-37"></a>

### D-37 — Visão Geral

- **Decisão:** Disponibilizar **Visão Geral** para os workspaces e/ou para as janelas de um workspace, facilitando navegação visual.
- **Aprovação conforme a fonte:** **Necessidade solicitada pelo usuário.** Sistema com níveis Todos / Atual; o padrão de entrada em Todos está aprovado pela D-38. Gestos e tecnologia ainda em discussão.
- **Implementação existente:** Não existe Overview oficial com miniaturas; o Alt+Tab atual é um overlay textual ([arquitetura][architecture]).
- **Validação:** Inspeção estática e conferência documental; não houve novo teste funcional. Evidências históricas do núcleo não validam a futura suíte.
- **Proveniência:** [visão v3.13, Complemento de decisões; §§39–44][vision], pacote de 28/10/2026.
- **Limites e relações:** D-38–D-41 delimitam parte da UX; captura, gestos, APIs e backend continuam propostos.

<a id="d-38"></a>

### D-38 — Entrada em Todos

- **Decisão:** Ao abrir **Visão Geral**, apresentar inicialmente **Todos os workspaces**, com acesso secundário ao workspace atual.
- **Aprovação conforme a fonte:** **APROVADA.** Não memorizar o último nível como abertura padrão; detalhes de interação e backend continuam em análise.
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, Complemento de decisões; §§39–44][vision], pacote de 28/10/2026.
- **Limites e relações:** Reabrir sempre em Todos; não memorizar o último nível como padrão de abertura.

<a id="d-39"></a>

### D-39 — Inspeção e ativação

- **Decisão:** Na Visão Geral, **clique no cartão abre o detalhe das janelas**; botão separado **Ir para workspace** ativa o workspace diretamente.
- **Aprovação conforme a fonte:** **APROVADA expressamente em 28/10/2026**.
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, Complemento de decisões; §§39–44][vision], pacote de 28/10/2026.
- **Limites e relações:** Abrir detalhe não muda workspace real; outras ações de teclado/fechamento continuam como propostas de UX.

<a id="d-40"></a>

### D-40 — Agrupamento por monitor

- **Decisão:** Na Visão Geral, **todos os workspaces são exibidos agrupados por monitor**, com **filtros opcionais** para restringir a visualização a um monitor.
- **Aprovação conforme a fonte:** **APROVADA expressamente em 28/10/2026**.
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, Complemento de decisões; §§39–44][vision], pacote de 28/10/2026.
- **Limites e relações:** Agrupamento não estabelece tema por monitor nem uma superfície em cada tela.

<a id="d-41"></a>

### D-41 — Superfície única

- **Decisão:** Na Visão Geral multimonitor, usar **uma única superfície no monitor em foco**, mostrando workspaces de **todos os monitores agrupados por monitor**.
- **Aprovação conforme a fonte:** **APROVADA expressamente em 28/10/2026**. Resolvida a escolha entre P-14 e P-15: P-14 adotada; P-15 não entra na primeira versão.
- **Implementação existente:** Não implementada como funcionalidade da suíte/rice oficial. A base atual é descrita em [ARCHITECTURE][architecture].
- **Validação:** Conferência documental; validação funcional deste requisito não executada.
- **Proveniência:** [visão v3.13, Complemento de decisões; §§39–44][vision], pacote de 28/10/2026.
- **Limites e relações:** Adota P-14 e exclui P-15 da primeira versão; origem de invocação F-1 e preservação técnica de foco seguem propostas.

<a id="d-42"></a>

### D-42 — Escopo local da revelação

- **Decisão:** Mostrar área de trabalho somente no workspace ativo do monitor em foco, preservando os demais monitores e o estado de janelas já minimizadas.
- **Aprovação conforme a fonte:** Aprovada explicitamente em 28/10/2026 (opção A). A origem exata de clique versus atalho e os mecanismos técnicos continuam propostos.
- **Implementação existente:** O núcleo tem `show_desktop` sobre o workspace ativo ([código][core]); isso é correspondência parcial, não atendimento validado do contrato multimonitor/recuperação. D-43–D-45 não estão implementadas.
- **Validação:** Inspeção estática e conferência documental; não houve novo teste funcional. Evidências históricas do núcleo não validam a futura suíte.
- **Proveniência:** [visão v3.13, §45][vision], pacote de 28/10/2026.
- **Limites e relações:** Não transforma a operação local em comando global nem impõe automaticamente a mesma UI a rices externos.

<a id="d-43"></a>

### D-43 — Restauração ao abandonar workspace

- **Decisão:** Ao abandonar efetivamente o workspace revelado no monitor de origem, encerrar a revelação e restaurar as janelas antes visíveis ocultadas por essa operação. Apenas focar outro monitor ou inspecionar cartões não dispara a regra.
- **Aprovação conforme a fonte:** Aprovada expressamente em 28/10/2026. Resolve P-17.2a; não aprova por si só os outros gatilhos. Não promete restauração anterior a qualquer frame da troca.
- **Implementação existente:** Não implementada. A rotina existente não restaura automaticamente por abandono do workspace; ver [limites atuais][architecture] e [estado][status].
- **Validação:** Inspeção estática e conferência documental; não houve novo teste funcional. Evidências históricas do núcleo não validam a futura suíte.
- **Proveniência:** [visão v3.13, §47][vision], pacote de 28/10/2026.
- **Limites e relações:** Preservar janelas previamente minimizadas e não alterar o monitor remoto; eventos atrasados/repetidos não devem produzir restauração no escopo errado.

<a id="d-44"></a>

### D-44 — Restauração por nova janela elegível

- **Decisão:** Quando uma nova janela de aplicativo elegível for efetivamente criada/apresentada no workspace revelado, restaurar todas as janelas antes visíveis ocultadas pela operação e permitir a nova janela sob o modo atual. Um pedido de iniciar aplicativo não é prova de criação; preservar minimizações anteriores e o escopo local.
- **Aprovação conforme a fonte:** Aprovada expressamente em 28/10/2026. Resolve P-17.2b. Foco da nova janela segue o gerenciador; classificação técnica de elegibilidade e recuperação continuam sujeitas à implementação.
- **Implementação existente:** Não implementada. A criação elegível não implementa essa restauração no núcleo atual; ver [limites atuais][architecture].
- **Validação:** Inspeção estática e conferência documental; não houve novo teste funcional. Evidências históricas do núcleo não validam a futura suíte.
- **Proveniência:** [visão v3.13, §48][vision], pacote de 28/10/2026.
- **Limites e relações:** Não reconstruir à força a geometria exata de Tiling/Scroll após a nova janela; não disparar por popup do shell, falha de lançamento ou criação em outro workspace.

<a id="d-45"></a>

### D-45 — Restauração e foco pela dock

- **Decisão:** Ao escolher na dock uma janela já existente temporariamente ocultada por Mostrar área de trabalho, restaurar o conjunto antes visível e depois ativar/focar a janela-alvo válida. Preservar minimizações anteriores; no estado revelado, essa operação prevalece sobre minimizar por clique no Floating. Restauração e foco têm resultados distintos.
- **Aprovação conforme a fonte:** Aprovada expressamente em 28/10/2026. Resolve P-17.2c apenas para esse acionamento pela dock; favorito aberto somente em outro workspace continua sujeito à D-15. Outras formas de ativação não são aprovadas por extensão.
- **Implementação existente:** Não implementada. Não existe dock oficial nem comando semântico versionado que realize esse contrato; ver [API atual][api].
- **Validação:** Inspeção estática e conferência documental; não houve novo teste funcional. Evidências históricas do núcleo não validam a futura suíte.
- **Proveniência:** [visão v3.13, §49][vision], pacote de 28/10/2026.
- **Limites e relações:** Conservar a obrigação observável de restauração coerente/idempotente e validação do alvo. Journal, formato persistido, estruturas de controle de geração e nomes de API são soluções técnicas propostas, não contratos atuais.

## Precedência e alterações futuras

D-11 delimita o padrão das propostas P-01/P-02; D-16, D-19/D-20 e D-29 resolvem apenas partes das respectivas propostas. D-39–D-42 resolvem escolhas de inspeção, agrupamento, superfície e escopo local. D-43/D-44/D-45 resolvem P-17.2a/b/c no escopo explicitamente aprovado. Propostas antigas que ainda digam “pendente” não revogam essas decisões; alternativas técnicas não se tornam aprovadas por aparecerem numa seção posterior.

Uma alteração futura deve identificar quais D-* afeta, o que permanece proposto e qual mudança OpenSpec será considerada. O registro das decisões originais e os bytes das fontes permanecem preservados. A autorização de consolidação documental não autoriza escolher shell, biblioteca visual, nome público ou migrar o compositor.

[vision]: ../history/product-import/2026-10-28/external-originals/docs/product/vision/ESPECIFICACAO_INTERFACE_SUITE_v3.13.md
[architecture]: ../ARCHITECTURE.md
[api]: ../API.md
[config]: ../CONFIGURAVEL.md
[status]: ../STATUS.md
[core]: ../../core/kwinish.lua
[modes]: ../../core/modes.lua
[audit]: ../history/product-import/2026-10-28/external-originals/docs/product/engineering/AUDITORIA_CODEX_ORIGINAL_2026-10-28.txt
