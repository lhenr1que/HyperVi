# Propostas, alternativas e investigação

**Estado:** planejamento, não autorização de implementação. **Consolidação:** 29/10/2026. Este registro preserva os identificadores das fontes e separa os trechos resolvidos por decisões de produto dos detalhes ainda abertos. Uma proposta aprovada em parte não aprova seu texto inteiro.

O [registro D-01–D-45](DECISIONS.md) mantém aprovação, implementação e validação por decisão. Os documentos [INTERFACE](INTERFACE.md), [SETTINGS](SETTINGS.md), [WORKSPACES](WORKSPACES.md) e [DESIGN](DESIGN.md) apresentam a leitura consolidada por domínio. As fontes completas permanecem no acervo; suas alternativas e exemplos não foram descartados.

## Registro P-* e F-*

**Validade das colunas:** “não implementado” refere-se ao comportamento da futura suíte; recursos menores do gerenciador podem existir, conforme a [arquitetura atual](../ARCHITECTURE.md). “Não executada” significa ausência de ensaio funcional desse requisito nesta consolidação. Conferência documental não certifica UX, integração ou compatibilidade.

### Composição da interface — fonte: visão, §2.2

| ID original | Proposta e limite atual | Aprovação | Implementação / validação |
|---|---|---|---|
| P-01 | Barra inferior inicial; reposicionamento superior | Padrão inferior resolvido por [D-11](DECISIONS.md#d-11); reposicionamento continua proposta | Shell oficial não implementado; validação funcional não executada |
| P-02 | Dock integrada na apresentação inicial; centralização exata | Integração resolvida por D-11; geometria continua proposta | Não implementado; não executada |
| P-03 | Barra/dock em bordas distintas e configurações alternativas | Diretriz geral em [D-02](DECISIONS.md#d-02); combinações e posições detalhadas não aprovadas | Não implementado; não executada |
| P-04 | Evitar título de janela no centro por padrão quando houver dock | Proposta de apresentação | Não implementado; não executada |
| P-05 | Ocultar somente o módulo da dock integrada, mantendo navegação e status acessíveis | Proposta de UX; ocultação inteligente aprovada em [D-03](DECISIONS.md#d-03) não resolve todos os casos | Não implementado; não executada |
| P-06 | Sobreposição/sensor de borda para dock independente, sem resize repetitivo | Proposta de UX e técnica, dependente de validação Wayland | Não implementado; não executada |
| P-07 | Agrupar aplicativos por padrão com chave de configuração | Núcleo resolvido por [D-16](DECISIONS.md#d-16); indicadores e interação de grupos propostos | Não implementado; não executada |
| P-08 | Launcher híbrido dividido; priorizar apps/ações e adicionar outras fontes depois | Formato/layout resolvidos por [D-19](DECISIONS.md#d-19)/[D-20](DECISIONS.md#d-20); fontes, fases e ranking propostos | Não implementado; não executada |
| P-09 | Barra por monitor e launcher no monitor em foco | Proposta; regras de Overview D-40/D-41 não aprovam a política do launcher por extensão | Não implementado; não executada |
| P-10 | Ativação de rice preferencialmente no próximo login, com restauração segura | Proposta de ativação; D-06 não escolhe mecanismo ou momento de troca | Rice Store não implementada; não executada |
| P-11 — barra | Três ilhas e reordenação; editor livre, dimensões e efeitos | Parcialmente resolvida por [D-29](DECISIONS.md#d-29); editor/limites continuam propostos | Não implementado; não executada |

Fonte integral: [visão v3.13, §2.2 e §§30–31][vision]. As fases sugeridas não são um cronograma aprovado.

### Colisão histórica de P-11

A fonte também usa **P-11 na §23.2 para navegação lateral do Settings**. Essa é uma proposta distinta de P-11 — barra. O índice conserva a ambiguidade como dado histórico; referências novas devem informar “P-11, §23.2 — Settings” ou “P-11, §30 — barra”. Não há renumeração retroativa.

| ID com escopo da fonte | Proposta e limite | Aprovação | Implementação / validação |
|---|---|---|---|
| P-11 — Settings, §23.2 | Navegação lateral, Sistema permanente e páginas da interface | Parte da organização foi resolvida por [D-23](DECISIONS.md#d-23); catálogo de controles, fluxos e integrações permanece proposto | Existe Settings GTK4 restrito; sidebar futura não implementada; validação funcional não executada |

### Overview, foco e revelação temporária

| ID original e fonte | Proposta e estado consolidado | Implementação / validação |
|---|---|---|
| P-12 — §41 | Dois níveis Todos/Detalhe; clique para inspecionar e ação separada para ativar resolvidos por [D-38](DECISIONS.md#d-38)/[D-39](DECISIONS.md#d-39). Esc, voltar, seleção de janela, teclado e demais interações continuam propostas | Overview oficial não implementado; ensaios OV-* não executados nesta consolidação |
| P-13 — §42 | Conjunto de workspaces agrupado por monitor resolvido por [D-40](DECISIONS.md#d-40); superfície resolvida por [D-41](DECISIONS.md#d-41). Hotplug, escala, referências inválidas e fallback continuam detalhamento proposto | Não implementado; não executada |
| P-14 — §43.2 | Superfície única no monitor em foco adotada por D-41; mecanismos de identificar monitor de invocação continuam propostos | Não implementado; não executada |
| P-15 — §43.2 | Superfícies distribuídas entre monitores; não integra a primeira versão conforme D-41. Um seletor futuro P-14/P-15 também não foi aprovado | Alternativa histórica, não implementada; não executada |
| P-16 — §44.2 | Escopo local de Mostrar área de trabalho resolvido por [D-42](DECISIONS.md#d-42); operação global continua alternativa sem aprovação. Casos especiais e origem de comando continuam propostos | Rotina atual cobre apenas parte do domínio; contrato completo não implementado; não executada |
| P-17 — §45.3 | Conjunto de propostas de saída/recuperação, decomponível nos itens abaixo; não tratar o grupo como aprovado integralmente | D-43–D-45 ausentes no código atual; não executada |
| P-17.1 — §45.3 | Atalho usa monitor em foco; botão usa monitor da barra acionada após identificar a interação | Origem detalhada ainda proposta; não implementada/validada como contrato da suíte |
| P-17.2 — §45.3 | Restaurar por troca, criação ou ativação. O histórico “pendente” deve ser lido à luz das três decisões posteriores, nos limites abaixo | Não implementado como contrato completo; não executada |
| P-17.2a — §47 | Sair efetivamente do workspace revelado: resolvido por [D-43](DECISIONS.md#d-43) | Não implementado; não executada |
| P-17.2b — §48 | Nova janela elegível efetivamente criada no workspace revelado: resolvido por [D-44](DECISIONS.md#d-44) | Não implementado; não executada |
| P-17.2c — §49 | Escolher pela dock janela existente ocultada: resolvido por [D-45](DECISIONS.md#d-45). Outros caminhos de ativação explícita exigem mapeamento; não ampliar o escopo aprovado | Não implementado; não executada |
| P-17.3 — §§45.3/47.3 | Esc fecha Overview sem restaurar automaticamente quando não há troca de workspace | Proposta de UX; não implementada/validada como interação de Overview |

Fonte integral e evolução de precedência: [visão, §§41–49][vision]. A [especificação consolidada de visualização][visualizacao] e o [contrato de foco/restauração][contrato] contêm critérios planejados; seus textos não comprovam implementação. A [matriz SD-*][matriz] permanece uma fonte de testes candidatos, não um relatório de testes executados.

### F-* — identificadores diferentes que não devem ser normalizados

| ID original | Conteúdo e grau de aprovação | Implementação / validação |
|---|---|---|
| F-01 — visão, §33.2 | Tema independente por monitor, derivado de cada wallpaper. Backlog futuro, fora do MVP; não substitui o tema global aprovado em [D-31](DECISIONS.md#d-31) | Não implementado na suíte; não executada |
| F-1 — visão, §44.1 | Proposta UX de origem do foco: distinguir atalho global e botão da barra, capturando origem antes do overlay | Proposta, diferente de F-01; não implementada/validada como contrato da suíte |

## Detalhamentos não aprovados em bloco

| Domínio e fonte preservada | Alternativas ou mecanismos ainda propostos | Implementação / validação |
|---|---|---|
| [Dock][dock] | Semântica de ocultação integrada, fullscreen, cliques em grupos, interação Tiling/Scroll, acionamento de prévias, menus e acessibilidade | Dock oficial ausente; testes candidatos não executados |
| [Launcher][launcher] | Catálogo de ações, fontes futuras, ranking, dimensões, responsividade, foco/fallback e execução; D-19–D-22 não aprovam todos esses mecanismos | Launcher oficial ausente; testes candidatos não executados |
| [Settings][settings] | Modelo de responsabilidades, contratos de rice, persistência, aplicação/erro, perfis de monitor, identidade, detecção/handshake idle/lock, ferramentas e fases | UI GTK4 parcial existente; integração futura não implementada; testes candidatos não executados |
| [Design system][design] | Biblioteca de ícones, tokens exatos, escala, animações, extração de cores, fonte técnica e limites de transparência | Opções de cor atuais não implementam o sistema completo; mockups não são validação funcional |
| [Foco][foco] e [contrato][contrato] | Identidade de sessão/monitor/workspace/janela, coordenação de estado, geração/journal, revalidação e recuperação | API pública e protocolo persistido não implementados; auditorias históricas não validam a solução proposta |

Não escolher automaticamente schemas de manifesto, nomes como `request_restore`, tecnologias de captura, bibliotecas, processos, ferramentas de idle ou formatos de journal. A exigência observável aprovada de restauração coerente não equivale à aprovação de uma dessas implementações.

## Minuta OpenSpec externa

O diretório [drafts/stabilize-show-desktop-restoration](drafts/stabilize-show-desktop-restoration/) conserva os cinco arquivos originais da minuta fora do OpenSpec real:

- [Metadados](drafts/stabilize-show-desktop-restoration/.openspec.yaml).
- [Proposal](drafts/stabilize-show-desktop-restoration/proposal.md).
- [Design](drafts/stabilize-show-desktop-restoration/design.md).
- [Delta candidato](drafts/stabilize-show-desktop-restoration/specs/show-desktop/spec.md).
- [Tasks](drafts/stabilize-show-desktop-restoration/tasks.md).

**Aprovação:** rascunho técnico pendente de reconciliação. **Implementação:** não incorporado a capacidades/mudanças vigentes. **Validação:** cópia documental e integridade; nenhum comando OpenSpec nem teste funcional executado por esta consolidação.

Divergências a resolver em trabalho técnico autorizado:

1. A proposta classifica `show-desktop` como capacidade modificada, mas não existe spec-base desse domínio no OpenSpec atual. Uma futura proposta terá de reconciliar a classificação como nova capacidade.
2. O baseline citado `60e19df` diverge de `a2506c63dad16384414dc95919b8292aead284df`, base desta consolidação e da auditoria original fornecida.
3. APIs, gerações, persistência e sequenciamento são propostas. Tarefas escritas no draft não são tarefas executadas nem autorização de implementação.
4. A fonte mistura decisões observáveis aprovadas e recomendações técnicas. A promoção de cada trecho exige escopo e critérios verificáveis próprios.

Os prompts [OpenSpec D-43–D-45][prompt-spec], [pós-auditoria][prompt-auditoria] e [Hyprland 0.57][prompt-hypr] permanecem solicitações históricas. Não são instruções operacionais permanentes para agentes.

## Ideias anteriores do repositório

As fontes abaixo permanecem íntegras no histórico. Seu estado original é histórico; a coluna de leitura atual evita reativar tarefas já feitas ou transformar desejos em decisões.

| Fonte histórica | Leitura atual e aprovação | Implementação / validação |
|---|---|---|
| [Seletor em cruz](../history/reference/ideias/seletor-em-cruz.md) | Alternativa de HUD separado, a reconciliar com indicador integrado D-04. Quarto quadrante VXWM não é funcionalidade aprovada para o núcleo inicial | Três modos já existem; HUD da ideia não implementado; ensaio de produto não executado |
| [VXWM/canvas infinito](../history/reference/ideias/vxwm-canvas-infinito.md) | A fonte dizia “roadmap v2.0”; D-10 mantém visão futura, sem compromisso de versão | Não implementado; não executada |
| [Distro Arch](../history/reference/ideias/distro-arch.md) | Origem da visão; distribuição, identidade e dependências não são escolhidas nesta consolidação | Não implementada como suíte/distribuição; não executada |
| [Página no Hub](../history/reference/ideias/pagina-no-hub.md) | Integração opcional, preservando HyperVi independente. Não substituir automaticamente UI atual | Settings GTK4 já existe parcialmente; integração da ideia não validada nesta tarefa |
| [Modais ao minimizar](../history/reference/ideias/modais-ao-minimizar.md) | Relação mãe/filhas e semântica de minimização permanecem investigação técnica | Ação atual move a janela selecionada para workspace especial; comportamento conjunto não comprovado; não executada |
| [Encaixe entre monitores](../history/reference/ideias/encaixe-entre-monitores.md) | O índice antigo dizia bug; fonte posterior registra correção no código e pendência ao vivo. Não recriar tarefa de implementação por causa do título antigo | Estado e evidência preservados em [STATUS](../STATUS.md); não houve novo ensaio ao vivo |
| [Windows/macOS](../history/reference/ideias/comportamentos-windows-macos.md) | Catálogo de comportamentos; separar Snap Assist existente dos demais candidatos, sem copiar código de referências | Implementação parcial descrita tecnicamente; propostas restantes não validadas como produto |
| [Omarchy](../history/reference/ideias/referencia-omarchy.md) e [ferramentas](../history/reference/ideias/referencias-e-ferramentas.md) | Fontes de estudo e atribuições, sem adoção automática de licenças/arquitetura | Reutilização futura depende de inventário por componente; não executada |

Referências locais externas ao pacote, como caminhos `~/Downloads/ryoku-work/hde/`, não são dependências documentais portáveis. Permanecem no original como proveniência; o conteúdo ausente não foi reconstruído ou presumido disponível.

## Estudo de shells e outras decisões técnicas futuras

O estudo arquitetural de Illogical Impulse, Serpantinum, Noctalia e Caelestia é pesquisa, sem escolha de shell aprovada. Fork, integração externa e desenvolvimento próprio com reutilização seletiva continuam alternativas. O estudo não autoriza incorporar componentes, alterar licenças ou trocar a stack atual. Uma futura decisão deve registrar versão/commit, fronteiras com HyperVi, ações diretas ao compositor, dependências transitivas, manutenção e compatibilidade medida.

Uma API com snapshot/eventos, identidades estáveis, capabilities e comandos semânticos é uma proposta arquitetural; não existe como API pública versionada hoje. Inspecionar um workspace na interface e ativá-lo no compositor precisam permanecer operações distinguíveis conforme D-39, mas nomes, transporte e schemas ainda não foram escolhidos.

Os pareceres [da auditoria e roadmap][roadmap] e [da compatibilidade 0.57][hypr] são fontes datadas. Prioridades sugeridas não viram cronograma aprovado; atualização de compositor, ABI, build/reload e correção de comportamento exigem trabalho separado. Esta consolidação não fez upgrade ou teste de compatibilidade.

## Como promover ou encerrar uma proposta

Registrar fonte e escopo, solicitar a decisão pertinente e conservar o estado anterior. Se houver mudança de comportamento/contrato, seguir o OpenSpec real e as regras do [AGENTS](../../AGENTS.md). Não copiar a minuta inteira para specs principais, arquivar mudanças ativas ou marcar critérios planejados como executados. Uma proposta resolvida deve apontar para a decisão que a resolveu, mantendo suas partes ainda abertas.

[vision]: ../history/product-import/2026-10-28/external-originals/docs/product/vision/ESPECIFICACAO_INTERFACE_SUITE_v3.13.md
[dock]: ../history/product-import/2026-10-28/external-originals/docs/product/features/DOCK_ESPECIFICACAO_CONSOLIDADA_v1.2.md
[launcher]: ../history/product-import/2026-10-28/external-originals/docs/product/features/LAUNCHER_HIBRIDO_ESPECIFICACAO_CONSOLIDADA_v1.0.md
[settings]: ../history/product-import/2026-10-28/external-originals/docs/product/features/SETTINGS_ARQUITETURA_FUNCIONAL_CONSOLIDADA_v1.0.md
[visualizacao]: ../history/product-import/2026-10-28/external-originals/docs/product/features/VISUALIZACAO_WORKSPACES_AREA_DE_TRABALHO_v1.0.md
[foco]: ../history/product-import/2026-10-28/external-originals/docs/product/features/FOCO_SHOW_DESKTOP_MULTIMONITOR_v0.5.md
[design]: ../history/product-import/2026-10-28/external-originals/docs/product/design/DESIGN_SYSTEM_INTERFACE_OFICIAL_v1.8.md
[contrato]: ../history/product-import/2026-10-28/external-originals/docs/product/engineering/CONTRATO_VISAO_GERAL_FOCO_RESTAURACAO_v1.4.md
[matriz]: ../history/product-import/2026-10-28/external-originals/docs/product/engineering/MATRIZ_TESTES_SHOW_DESKTOP_D43_D44_D45_v1.2.md
[prompt-spec]: ../history/product-import/2026-10-28/external-originals/docs/product/proposals/PROMPT_CODEX_OPENSPEC_D43_D44_D45.md
[prompt-auditoria]: ../history/product-import/2026-10-28/external-originals/docs/product/engineering/PROMPT_CODEX_POS_AUDITORIA_v1.0.md
[prompt-hypr]: ../history/product-import/2026-10-28/external-originals/docs/product/engineering/PROMPT_CODEX_HYPRLAND_057.md
[roadmap]: ../history/product-import/2026-10-28/external-originals/docs/product/engineering/AVALIACAO_AUDITORIA_E_ROADMAP_GERENCIADOR_v1.0.md
[hypr]: ../history/product-import/2026-10-28/external-originals/docs/product/engineering/HYPRLAND_0_57_COMPATIBILITY.md
