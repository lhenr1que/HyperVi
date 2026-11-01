# Interface oficial: barra, dock e launcher

Consolidação documental em 29/10/2026, baseada nas fontes de produto de 28/10/2026 e na implementação `a2506c6`. Este documento descreve **produto futuro**. A interface oficial, sua barra, dock e launcher não estão implementados no HyperVi. A revisão documental não executou testes de integração dessas superfícies.

O [registro D-01–D-45](DECISIONS.md) conserva o grau literal de aprovação. A aprovação do comportamento não aprova automaticamente algoritmo, toolkit, esquema de configuração ou implementação. A [arquitetura atual](../ARCHITECTURE.md), a [API atual](../API.md) e o [OpenSpec vigente](../../openspec/specs/) continuam separados deste documento.

## Escopo e responsabilidades

| Requisito | Aprovação de produto | Implementação e validação |
|---|---|---|
| Interface oficial como rice da futura suíte | [D-01](DECISIONS.md#d-01) aprovada; nome em aberto em [D-09](DECISIONS.md#d-09) | Não implementada; sem validação funcional da suíte |
| Gerenciador e configurações fundamentais disponíveis com outro rice | [D-08](DECISIONS.md#d-08): direção arquitetural consolidada | Lua/C++/GTK4 já têm responsabilidades próprias; a integração de rices não existe nem foi testada |
| Rice completo ativo como conjunto | [D-06](DECISIONS.md#d-06) aprovada | Rice Store ausente; manifesto, ativação, conflitos e rollback são propostas |
| Dashboard excluído por enquanto; Canvas/VXWM futuro | [D-07](DECISIONS.md#d-07) e [D-10](DECISIONS.md#d-10) | Não são requisitos da interface inicial; nenhuma implementação iniciada |

Os controles visuais devem apresentar o estado observado e encaminhar ações ao responsável pelo domínio. A proposta de integração não autoriza o shell a duplicar políticas Lua nem a escrever configurações fundamentais ao ativar um rice. Um rice externo pode fornecer barra/taskbar inseparável ou outra composição; o desacoplamento de dock é recurso do rice oficial ([D-12](DECISIONS.md#d-12)). Critérios de conformidade e certificação de terceiros continuam propostos.

## Barra

| Requisito | Estado de aprovação | Implementação e validação |
|---|---|---|
| Barra inferior contínua com dock integrada na instalação padrão | [D-11](DECISIONS.md#d-11) aprovada | Ausente no HyperVi; mockups são conceitos |
| Barra e dock separáveis | [D-02](DECISIONS.md#d-02): diretriz adotada, detalhes a definir; D-12 delimita o oficial | Ausente; nenhuma validação Wayland |
| Número e indicador de Tiling/Scroll/Floating integrados a cada workspace | [D-04](DECISIONS.md#d-04) aprovada | Modos existem no núcleo; representação oficial não existe |
| Settings e Monitores como módulos opcionais de status | [D-05](DECISIONS.md#d-05) aprovada | Módulos oficiais ausentes; Settings GTK atual tem escopo menor |
| Preset modular opcional: três âncoras esquerda/centro/direita; reordenação dentro de cada ilha | [D-29](DECISIONS.md#d-29) aprovada | Ausente; editor, geometria e persistência não definidos |

Composição funcional recomendada: navegação à esquerda (launcher, workspaces e acesso à Visão Geral), dock integrada na região central, sistema à direita (indicadores, relógio e acessos opcionais). Título de janela central obrigatório não foi aprovado. Reposicionar a barra para cima, zonas de clique separadas para número/modo, prioridades de overflow e arraste são detalhamentos propostos.

No preset modular, a dock central continua integrada ao layout lógico. Ao desacoplá-la, a ilha central fica ausente; não deve haver duas representações funcionais da mesma dock. Desativar a dock preserva caminhos de acesso a aplicativos e configurações. A apresentação contínua/modular não modifica janelas nem modos.

Mostrar área de trabalho e Visão Geral são comandos distintos: navegação/ilha esquerda e sistema/ilha direita são posições de design propostas. Seus comportamentos estão em [WORKSPACES](WORKSPACES.md); tokens e mockups, em [DESIGN](DESIGN.md).

## Dock contextual

| Requisito | Estado de aprovação | Implementação e validação |
|---|---|---|
| Favoritos globais e tarefas do workspace ativo | [D-13](DECISIONS.md#d-13) aprovada | Dock oficial ausente; atribuição lógica de minimizadas ainda incompleta na API |
| Agrupamento por aplicativo ligado por padrão, desligável no Settings | [D-16](DECISIONS.md#d-16) aprovada | Ausente; sem testes de agrupamento |
| Desagrupado: favorito persistente separado de cada tarefa local | [D-18](DECISIONS.md#d-18) aprovada | Ausente; repetição visual de ícones é intencional |
| Uma janela local no Floating: clique minimiza quando focada e restaura quando minimizada | Núcleo da [D-14](DECISIONS.md#d-14) aprovado; focar primeiro uma janela em segundo plano é esclarecimento operacional proposto | Núcleo tem minimizar/foco; integração da dock não existe nem foi validada |
| Favorito aberto apenas remotamente solicita nova janela aqui | [D-15](DECISIONS.md#d-15) aprovada; single-instance e erros exigem validação | Ausente; solicitação aceita não comprova nova janela |
| Prévia textual ou miniaturas, configurável e condicionada ao suporte; fallback textual | [D-17](DECISIONS.md#d-17) aprovada condicionalmente | Captura/preview oficial ausentes; padrão Automático, hover e acionamento são propostas |
| Ocultação inteligente | [D-03](DECISIONS.md#d-03) aprovada; gatilhos em aberto | Ausente; fullscreen, interseção e reserva de área não validados |

Exemplo aprovado de papéis: navegador favorito com três janelas locais resulta em um grupo no modo agrupado; no desagrupado, um favorito e três tarefas. Agrupar/desagrupar não cria, fecha ou move janelas. Janelas de outros workspaces não se tornam tarefas locais; favoritas continuam disponíveis.

A interpretação de uma dock por monitor acompanhando o workspace ativo daquela saída continua detalhamento multimonitor a validar. Janelas minimizadas precisam ser associadas ao workspace lógico de origem; o atual `special:minimized` não fornece sozinho esse contrato completo. O [estado atual da minimização](../API.md) não equivale à API futura da dock.

### Interações que continuam propostas

- Grupo com várias janelas abre seletor local; não minimiza nem fecha o grupo inteiro. Favorito com uma janela continua relacionado à D-14.
- Clique central solicita nova janela quando suportado; clique direito abre menu contextual; hover identifica sem focar ou executar.
- Menus: abrir aqui/nova janela; escolher janela; focar/restaurar/minimizar conforme alvo; fixar/desafixar; fechar somente uma janela inequivocamente selecionada. Acesso remoto deve ser explícito. Excluir `Fechar todas` do menu principal é recomendação.
- Tiling/Scroll: focar/restaurar sem toggle automático de minimização é proposta, não ampliação da D-14.
- Ordenação, arraste, MRU, tempos de hover, overflow, sensor de borda e comportamento de popovers seguem em avaliação.
- Dock integrada pode recolher só aplicativos e preservar navegação/status; dock desacoplada pode recolher por interseção. Não provocar resize contínuo. Fullscreen real não deve revelar superfícies acidentalmente. A semântica fina ainda depende de aprovação e teste.

**Precedência da D-45:** se o alvo da dock foi temporariamente ocultado por Mostrar área de trabalho, restaurar o conjunto antes visível e focar o escolhido; não executar primeiro o toggle Floating. Essa decisão não está implementada. Ver [restauração](WORKSPACES.md#mostrar-area-de-trabalho).

## Launcher híbrido

| Requisito | Estado de aprovação | Implementação e validação |
|---|---|---|
| Pesquisa imediata e navegação gráfica numa interface | [D-19](DECISIONS.md#d-19) aprovada | Launcher oficial ausente; não testado |
| Tela inicial Split: categorias à esquerda, apps em grade à direita e busca no topo | [D-20](DECISIONS.md#d-20) aprovada | Ausente; dimensões e responsividade não aprovadas |
| Busca global de apps e ações, independente da categoria selecionada | [D-21](DECISIONS.md#d-21) aprovada | Ausente; fontes, ranking e apresentação detalhados como baseline recomendada |
| Centralizado por padrão, opção ancorada à barra no Settings | [D-22](DECISIONS.md#d-22) aprovada | Ausente; monitor, foco e fallback são propostas |

A fonte consolidada do launcher prevalece sobre suas propostas antigas na visão. As regras abaixo são **baseline recomendada**, sem aprovação implícita pela D-19–D-22 e sem validação executada:

| Domínio | Baseline e limites propostos |
|---|---|
| Estados | Fechado; grade em Todos; categoria; pesquisa global; Ações; confirmação; pedido em curso; sem resultados. Ao apagar a consulta, retornar à categoria visual; reabrir com Todos e consulta vazia |
| Fontes | Apps `.desktop` aplicáveis à sessão e ações explicitamente registradas; respeitar itens ocultos. Consulta textual nunca executa shell. Arquivos, web, clipboard, plugins e recentes são expansão futura |
| Ranking e dados | Normalização, relevância previsível, cache/indexação assíncrona quando necessário; sem histórico/telemetria/frequência por padrão. Algoritmo não definido |
| Posicionamento | Atalho: monitor focado; botão: monitor da barra de origem; ancoragem indisponível/atalho sem botão inequívoco: fallback central. Revalidar escala, área útil e hotplug |
| Ações | Três modos do workspace alvo, Settings, monitores e bloqueio conforme capacidades reais. Energia sensível exige confirmação. Catálogo/MVP e atalhos não aprovados |
| Navegação | Busca focada ao abrir; setas navegam, Tab percorre regiões; Enter ativa item identificado; Esc fecha ou cancela só a confirmação. Hover não ativa |
| Lançamento | Capturar destino na ativação; usar aplicação/`.desktop` identificado. Não mover silenciosamente janela single-instance remota. Diferenciar pedido aceito, janela observada e erro |
| Fechamento | Fechar após pedido aceito é baseline; preservar busca em falha imediata e prevenir duplicidade sem bloqueio permanente |
| Sessão | Uma superfície lógica do launcher oficial; ao trocar rice, não disputar foco com launcher externo |

`Super+Espaço` é sugestão sujeita à auditoria de conflitos; `Super` isolada não foi decidido. Launcher não aplica o toggle de minimizar da dock. Feedback de modo, foco ou execução precisa de observação real, não apenas de animação.

## Validação planejada

Nenhum teste abaixo foi executado nesta consolidação:

- Barra: alternar contínua/modular, reordenar módulos na mesma ilha, desacoplar sem duplicar dock; validar escala, overflow, teclado e fullscreen.
- Dock: favorito + três janelas nos dois modos de agrupamento; minimizar só a janela focada no Floating; favorito remoto solicita janela local; fallback sem miniaturas; alvo fechado não redireciona clique.
- Launcher: escolher Internet e buscar Terminal globalmente; teclado completo/IME; ancoragem sem espaço; hotplug; single-instance; falha de lançamento; confirmação sensível; ativação não minimiza janela existente.
- Rice externo: configurações fundamentais continuam acessíveis; superfície oficial não concorre com a externa.

Os detalhes e cenários originais permanecem nas fontes. As pendências são reunidas em [PROPOSALS](PROPOSALS.md).

## Fontes preservadas

- [Visão v3.13, §§2–22 e 31](../history/product-import/2026-10-28/external-originals/docs/product/vision/ESPECIFICACAO_INTERFACE_SUITE_v3.13.md).
- [Dock consolidada v1.2](../history/product-import/2026-10-28/external-originals/docs/product/features/DOCK_ESPECIFICACAO_CONSOLIDADA_v1.2.md).
- [Launcher consolidado v1.0](../history/product-import/2026-10-28/external-originals/docs/product/features/LAUNCHER_HIBRIDO_ESPECIFICACAO_CONSOLIDADA_v1.0.md).
- [Manifesto de origem e hashes](../history/product-import/2026-10-28/SOURCES.json).

A fonte da dock menciona uma vez “favoritos por workspace” na priorização, em contraste com seus favoritos globais e D-13. Esta consolidação segue o escopo global registrado na decisão e conserva a formulação original no acervo.
