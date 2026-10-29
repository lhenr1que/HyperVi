# Design: direção aprovada, propostas e mockups

Consolidação documental em 29/10/2026, a partir das fontes de 28/10/2026. Este documento não define nome, marca definitiva, toolkit ou shell. Os mockups são conceitos gerados, sem prova de implementação, captura real ou fidelidade entre quadros. Nenhuma validação de contraste, desempenho ou UI foi executada nesta consolidação.

## Decisões e estado real

| Requisito | Aprovação de produto | Implementação existente e validação |
|---|---|---|
| V-B: translúcido contido e adaptativo | [D-28](DECISIONS.md#d-28) aprovada | Design system da suíte ausente; não validado em runtime |
| Contínua inicial; modular opcional com três ilhas e reordenação interna | [D-11](DECISIONS.md#d-11) e [D-29](DECISIONS.md#d-29) aprovadas | Barra oficial ausente; imagens conceituais |
| Tema inicial Automático orientado pelo wallpaper, com Claro/Escuro manuais | [D-30](DECISIONS.md#d-30) aprovada | Pipeline oficial ausente; algoritmo não aprovado/testado |
| Uma variante/paleta global pelo wallpaper do monitor principal | [D-31](DECISIONS.md#d-31) aprovada | Ausente; tema independente por saída é futuro F-01 |
| Acento do wallpaper principal, com escolha manual independente do modo Claro/Escuro | [D-32](DECISIONS.md#d-32) registrada como aprovada | Ausente; algoritmo, ferramenta e tokens não definidos |
| Transparência leve, superfícies quase opacas | [D-33](DECISIONS.md#d-33) aprovada | Ausente; 94–97% de opacidade é referência de prototipagem, não parâmetro final |
| Fonte efetiva configurada no sistema | [D-34](DECISIONS.md#d-34) aprovada | Integração do shell ausente; GTK4 atual mantém seu comportamento próprio |
| Família visual consistente de controles lineares/simbólicos; ícones originais dos apps | [D-35](DECISIONS.md#d-35) aprovada | Biblioteca/desenhos não escolhidos; ícones existentes não certificam o sistema futuro |

Opções de cor do plugin e CSS das views atuais não equivalem ao design system completo. A [UI Rust/GTK4](../../ui/src/main.rs) e os controles existentes devem ser inventariados e preservados; a direção de reaproveitamento não autoriza reescrita. Estado atual em [ARCHITECTURE](../ARCHITECTURE.md) e [CONFIGURAVEL](../CONFIGURAVEL.md).

D-32 foi registrada pela fonte como aprovação após orientação para prosseguir; conservar essa proveniência no [registro](DECISIONS.md#d-32). Não ampliar essa aprovação aos algoritmos ou à sincronização global de aplicações.

## Composição das superfícies

As regras funcionais completas ficam em [INTERFACE](INTERFACE.md), [SETTINGS](SETTINGS.md) e [WORKSPACES](WORKSPACES.md). Esta tabela consolida sua tradução visual; geometrias são propostas, não decisões adicionais.

| Superfície | Direção e limites |
|---|---|
| Barra contínua | Inferior, navegação, dock integrada e status; cada workspace tem número e pictograma de modo. Sem título central obrigatório |
| Barra modular | Âncoras esquerda/navegação, centro/dock, direita/sistema. Reordenação só dentro da ilha; sem ilhas arbitrárias ou editor livre aprovado |
| Dock desacoplada | Remove representação central; uma dock funcional. Orientação, borda, margens e área reservada precisam de validação |
| Launcher | Split com busca no topo, categorias à esquerda e grade à direita; centralizado e ancorado compartilham comportamento |
| Settings | Sidebar adaptativa, formulários legíveis, estados Salvo/Aplicado/Falha distintos; preservar widgets e semântica GTK existentes |
| Visão Geral | Número + modo por cartão, grupos por monitor e uma superfície na tela focada; exploração visual distinta de workspace ativo |
| Mostrar área de trabalho | Comando distinto de Overview/Monitores; indicar estado local sem falsificar minimização das tarefas |

No preset modular com dock desacoplada, duas ilhas podem ficar visíveis: três âncoras lógicas não exigem três superfícies vazias. Reordenação não autoriza mover qualquer módulo entre ilhas. Responsividade, overflow, âncora central exata, área exclusiva e recolhimento somente da dock continuam detalhamentos propostos.

## Tema, acento e tipografia

**Política aprovada:** a imagem do monitor principal define a variante e o acento globais. Alterar só o wallpaper secundário não muda a referência global. Escolha manual Claro/Escuro e escolha manual de acento são independentes e preservadas como substituições. Rices externos não recebem temas ou preferências sobrescritos.

**Pipeline técnico proposto, não implementado:** identificar referência válida; analisar luminosidade/distribuição; escolher variante com estabilidade; extrair cores candidatas; filtrar por contraste; gerar tokens; atualizar superfícies coordenadamente. Matugen e análise própria são candidatos, sem seleção de ferramenta. Gerar paleta e decidir Claro/Escuro a partir da imagem são responsabilidades diferentes.

Fallback para última paleta válida ou tema neutro, cache, debounce, histerese, hotplug do monitor principal, atualização sem flicker e adaptadores GTK/Qt são propostas. Não pressupor que exista uma fonte global universal ou que uma preferência GTK aplique tema a todos os programas.

A fonte oficial deve acompanhar preferência de interface obtida por integração suportada; sem preferência, usar sans-serif resolvida pelo sistema e fallback para português/símbolos/escrita internacional. Família, escala, mudança em runtime e resolução de preferências divergentes precisam de implementação verificada. A escolha do usuário não autoriza redistribuir fontes.

## Tokens e componentes propostos

IDs preservados da elaboração: T-01 (tipografia), G-01 (geometria), I-01/I-02 (iconografia/estados), M-01/M-02 (movimento), A11Y-01 (acessibilidade) e DS-API-01 (tokens). São propostas; as escolhas posteriores D-34/D-35 resolvem apenas fonte do sistema e família visual, mantendo valores e biblioteca em aberto.

| Família | Conteúdo conceitual |
|---|---|
| Cor/superfície | Base, painel, overlay, popover, Settings; texto principal/secundário/desabilitado; acento e variantes; foco e bordas; sucesso/alerta/erro |
| Geometria | Espaçamentos, raios, traços, alvos interativos, densidade e unidades lógicas |
| Tipografia | Papéis caption/body/title, família efetiva, fallback e escala |
| Movimento | Hover, pressionado, menu, launcher, recolhimento, troca de preset e paleta, com alternativa reduzida |
| Estado | Aberto, focado, minimizado, múltiplo, atenção, desabilitado; seleção de UI diferente de ativação real |

Os nomes YAML/JSON e taxonomias das fontes são exemplos de design, sem relação automática com formatos atuais. Uma API de tokens e suas migrações não foram aprovadas. Widgets visuais, controles GTK e componentes de outro toolkit podem consumir subconjuntos coerentes sem impor aparência a rices externos.

Valores ilustrativos preservados para protótipo, **não requisitos aprovados**:

- Barra 44–48 px lógicos; ícones integrados 22–26 px e desacoplados 24–32 px; launcher 600–720 px adaptativos.
- Espaçamento entre ilhas 8–16 px, alvos de interação maiores que o desenho; raios e escala ainda abertos.
- Referência mais recente de opacidade para barra/dock: 94–97%. Intervalos anteriores diferentes continuam históricos nas fontes.
- Hover 90–140 ms; menu 140–200 ms; launcher 180–240 ms; smart-hide 160–220 ms; troca de preset 180–260 ms. Curvas/durações dependem de protótipo.

Bibliotecas candidatas como Lucide e símbolos GTK/Adwaita exigem revisão de licença, legibilidade e integração antes da escolha. Logotipos de apps não devem virar outline por conveniência estética.

## Acessibilidade e confiabilidade propostas

Foco visível, rótulos acessíveis, navegação por teclado/IME e leitor de tela onde suportado; estados distinguíveis por forma/texto além de cor. Testar contraste na superfície composta com wallpaper e alpha; referências de 4,5:1 para texto comum e 3:1 para texto grande/elementos pertinentes são metas da proposta, não medições realizadas.

Prever ausência de blur/captura, opacidade total, movimento reduzido e fontes com métricas diferentes. Evitar clipping, alvos críticos truncados e loops de animação permanentes. Indicador de estado deve refletir confirmação do serviço/compositor; animação de solicitação pode mostrar progresso, mas não sucesso fictício. Capturas e notificações precisam de tratamento de privacidade.

## Acervo de mockups

Os seis PNG foram preservados com nomes e bytes originais em `mockups/`; os hashes e fontes estão no [manifesto](../history/product-import/2026-10-28/SOURCES.json). Nenhuma imagem foi editada nesta consolidação.

| Arquivo | Leitura e limite |
|---|---|
| [M-01 contínua v0.2](mockups/MOCKUP_M01_BARRA_CONTINUA_v0.2.png) | Estudo da barra contínua e janelas Floating; número+modo por workspace precisa de revisão |
| [M-02 modular v0.2](mockups/MOCKUP_M02_BARRA_MODULAR_v0.2.png) | Três ilhas; validar consistência de dados e remoção da ilha central quando desacoplada |
| [M-09 superfície única v0.1](mockups/MOCKUP_M09_A_SUPERFICIE_UNICA_v0.1.png) | A imagem mantém selo “Proposta”; a escolha posterior D-41 aprova superfície única, sem aprovar cada detalhe da arte |
| [M-10 quatro quadros v0.1](mockups/MOCKUP_M10_COMPARACAO_4_QUADROS_v0.1.png) | Comparação conceitual de foco/inspeção; nomes e atalhos ilustrativos não são decisões |
| [M-10 fluxo/restauração v0.1](mockups/MOCKUP_M10_FLUXO_E_RESTAURACAO_v0.1.png) | Fluxo ilustrativo; preservar minimização anterior e identidade das janelas exige validação futura |
| [Visão Geral Todos v0.1](mockups/MOCKUP_VISAO_GERAL_TODOS_v0.1.png) | Grade conceitual; não demonstra captura, navegação ou backend implementados |

“Arch Suite” na arte não define nome público. Atalhos, cores exatas, pictogramas, aplicativos ilustrados e dimensões não são ratificados pelo mockup. O ZIP contém uma seleção de seis imagens; pranchas e fontes editáveis referenciadas mas ausentes não foram recriadas.

Débitos visuais conhecidos: número+modo pouco claro em cada pílula; cenas Floating que podem aparentar workspace Tiling; diferenças gerativas de geometria/ordem Z/identidade entre quadros; busca de arquivos e recentes desenhados sem aprovação de MVP. Não inferir novos módulos ou funcionalidades a partir dessas ilustrações.

## Revisão visual futura — não executada

- Mesmos dados/janelas e wallpaper nas comparações; mudar apenas o preset desejado.
- Claro/escuro e wallpapers extremos; fonte larga, 1080p/4K, HiDPI e escala fracionária/mista.
- Contínua/modular, dock integrada/desacoplada, agrupamento ligado/desligado, overflow e popovers internos à tela.
- Estados de execução, foco, minimização, grupo e atenção; número+modo em todas as entradas de workspace.
- Launcher central/ancorado e busca global; Settings real preservado; motion reduzido e sem blur.
- Dois monitores: host, seleção e foco efetivo distintos; Show Desktop local sem mudar o outro monitor; minimização prévia preservada.

Os requisitos funcionais aprovados prevalecem sobre uma cena artística divergente. Não há novo roadmap, escolha de shell ou identidade definitiva nesta documentação.

## Fontes preservadas

- [Design System v1.8](../history/product-import/2026-10-28/external-originals/docs/product/design/DESIGN_SYSTEM_INTERFACE_OFICIAL_v1.8.md): cabeçalho v1.7 e numeração cumulativa preservados; adendos D prevalecem nos pontos específicos que resolvem.
- [Brief de mockups v1.1](../history/product-import/2026-10-28/external-originals/docs/product/design/BRIEF_MOCKUPS_INTERFACE_OFICIAL_v1.1.md): cabeçalho v1.0; mantém etapas antigas que não representam pendência atual.
- [Visão v3.13, §§29–49](../history/product-import/2026-10-28/external-originals/docs/product/vision/ESPECIFICACAO_INTERFACE_SUITE_v3.13.md).
- [DECISIONS](DECISIONS.md) e [PROPOSALS](PROPOSALS.md).
