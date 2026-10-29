# Workspaces, Visão Geral, foco e restauração

Consolidação em 29/10/2026; código de referência `a2506c6`. As regras futuras abaixo são produto, não documentação de APIs já existentes. A leitura estática desta consolidação não executou os cenários de aceitação.

## Implementação atual e limites

O [núcleo Lua](../../core/kwinish.lua) já oferece `show_desktop()`, minimizar e ações de janela. `show_desktop()` guarda um único conjunto global `stashed` em memória Lua; um reload perde as origens desse conjunto. Minimizar move o cliente para `special:minimized`; há restauração associada ao foco, mas sem contrato público completo de identidade/proveniência para a suíte. O modo é persistido por workspace; a exceção de modo inicial por monitor de [D-24](DECISIONS.md#d-24) não existe.

A [API atual](../API.md) não fornece o contrato futuro de restauração por escopo/sessão/geração nem o Overview oficial. Alt+Tab textual não equivale à Visão Geral com cartões e miniaturas. Problemas registrados de Floating, procedência e reload permanecem em [STATUS](../STATUS.md) e no [histórico](../history/README.md); não foram corrigidos por esta documentação.

## Decisões de produto e evidência

| Requisito | Grau de aprovação registrado | Implementação atual | Validação |
|---|---|---|---|
| Mostrar área de trabalho temporário e reversível | [D-36](DECISIONS.md#d-36): necessidade solicitada; detalhes não integralmente aprovados | Toggle básico existe, com limitações de memória/procedência | Registros históricos não certificam todas as garantias futuras |
| Visão Geral Todos e detalhe de workspace/janelas | [D-37](DECISIONS.md#d-37): necessidade solicitada e detalhamento em discussão | Ausente | Não executada |
| Abrir sempre em Todos, com acesso ao nível Atual | [D-38](DECISIONS.md#d-38) aprovada | Ausente | Não executada |
| Corpo do cartão inspeciona; botão Ir ativa workspace e fecha | [D-39](DECISIONS.md#d-39) aprovada | Ausente | Não executada |
| Todos os workspaces agrupados por monitor; filtros opcionais | [D-40](DECISIONS.md#d-40) aprovada | Ausente | Não executada |
| Uma superfície no monitor em foco contendo todos os grupos | [D-41](DECISIONS.md#d-41) aprovada | Ausente | Não executada |
| Revelar somente workspace ativo do monitor em foco | [D-42](DECISIONS.md#d-42) aprovada | Há ação no workspace ativo; contrato multimonitor e recuperação ainda incompletos | Conformidade integral não comprovada |
| Troca de workspace ativo no mesmo monitor encerra revelação anterior | [D-43](DECISIONS.md#d-43) aprovada | Ausente | Não executada |
| Nova janela elegível efetivamente mapeada no escopo encerra revelação | [D-44](DECISIONS.md#d-44) aprovada | Ausente | Não executada |
| Dock escolhe janela temporariamente oculta: restaura conjunto e foca alvo | [D-45](DECISIONS.md#d-45) aprovada para esse caminho explícito | Dock oficial e integração ausentes | Não executada |

A fonte externa rotula trechos cumulativos D-36/D-37 como aprovados integralmente em algumas tabelas, enquanto a visão/brief registram necessidades e interação em discussão. Conserva-se aqui o grau limitado das escolhas expressas. D-38–D-45 resolvem pontos específicos sem aprovar automaticamente o restante.

## Visão Geral

O monitor que hospeda o overlay, o workspace explorado na interface, o workspace efetivamente ativo de cada monitor e a janela focada são estados diferentes. O foco de teclado na superfície não autoriza ativar o cartão selecionado.

**Contrato aprovado D-38–D-41:** abrir em Todos; agrupar todos os workspaces por monitor numa única superfície no monitor em foco. Clicar no corpo de B5, enquanto a UI está em A, abre detalhe de B5 sem ativá-lo. `Ir para workspace` ativa o destino real e fecha a Visão Geral, sem transportar o workspace para A. Filtro só altera a apresentação. Abertura seguinte volta a Todos.

**Detalhamento funcional proposto, sem validação:**

- Cartões com número, modo, monitor, contagem e miniatura ou fallback; workspace vazio inspecionável e ativável explicitamente.
- Detalhe inclui janelas minimizadas no workspace lógico de origem e janelas Scroll fora do viewport; selecionar janela solicita restauração/foco pelo gerenciador, sem trocar seu modo.
- `Voltar`/Esc no detalhe retorna a Todos; Esc em Todos fecha. Setas, Enter para detalhe e Tab para ações são propostas de navegação acessível; atalho de ativação direta não definido.
- Revalidar cliente, workspace e monitor antes de executar; janela fechada não redireciona clique para outra entrada. Hotplug recompõe topologia e invalida referências antigas.
- Abertura, filtro e inspeção não alteram geometria, ordem Scroll ou minimização. Captura real não é presumida; texto/ícones mantêm função e respeitam privacidade de conteúdo protegido.
- Restaurar foco anterior ao cancelar, política de fullscreen/pinned/sticky e recuperação detalhada continuam propostas.

P-13 teve o escopo resolvido por D-40; P-14 foi escolhida por D-41. A alternativa P-15 distribuída por monitores fica histórica, fora da primeira configuração aprovada. As apresentações e transições estão relacionadas a [DESIGN](DESIGN.md), sem compromisso de toolkit ou backend.

<a id="mostrar-area-de-trabalho"></a>

## Mostrar área de trabalho

**D-42:** resolver monitor focado e workspace ativo no instante da ação; ocultar temporariamente apenas as janelas elegíveis daquele escopo. Segundo acionamento restaura as janelas afetadas. Outros monitores e workspaces permanecem intactos; as previamente minimizadas continuam minimizadas. Dock e Visão Geral mantêm o inventário lógico e não convertem a ocultação temporária em minimização definitiva.

| Gatilho aprovado | Resultado de produto | Limite de aprovação |
|---|---|---|
| D-43: workspace de A muda A2→A3 | Restaurar as janelas ocultadas de A2; retorno a A2 normal | Só mudar monitor focado A→B, mantendo A2, não é saída |
| D-44: novo toplevel elegível é observado em A2 revelado | Restaurar todas as janelas ocultadas e apresentar a nova normalmente | Spawn sem janela, notificação, overlay, cliente não elegível ou criação em B5 não bastam |
| D-45: clique explícito na dock escolhe cliente de A2 temporariamente ocultado | Restaurar todas as janelas antes visíveis ocultadas; depois focar o escolhido | Hover não aciona. Favorito só remoto segue D-15. Outros caminhos de ativação não são automaticamente aprovados |

D-45 tem precedência sobre o toggle Floating da D-14 nesse estado: a intenção de restaurar/focar não deve minimizar o alvo. Falha de foco não desfaz uma restauração concluída; os resultados precisam ser distinguíveis. D-43/D-44/D-45 concorrentes devem convergir sem toggle adicional. Ao criar uma janela em Tiling/Scroll, a disposição pode mudar naturalmente para acomodá-la; preservar modo e integridade não exige pixels idênticos.

Exemplo: A2 Floating revelado e B5 Tiling normal. Inspecionar B5 no overlay de A não restaura A2 nem ativa B5. `Ir B5`, sem trocar o workspace ativo de A, não constitui por si D-43. Trocar A2→A3 encerra a revelação de A2. Clicar em cliente ocultado de A2 pela dock restaura o conjunto de A2 e foca esse cliente; B5 permanece inalterado.

## Mecanismos técnicos ainda propostos

O contrato externo descreve identidade estável, escopo `(monitor, workspace)`, sessão, geração, proveniência por janela, estados de restauração/recuperação e comandos de restore. Eles **não são nomes nem APIs implementados no núcleo atual**. Idempotência, recuperação de reload e proteção contra IDs reciclados são objetivos de confiabilidade; journal, metadados, persistência e IPC concreto precisam de auditoria e aprovação técnica próprias.

Não depender apenas de tabela Lua ou memória transitória da UI para recuperar janelas. Uma solução futura precisa distinguir: janela anteriormente visível e efetivamente ocultada; previamente minimizada; fechada; movida manualmente; destino removido; ação concorrente e intenção de foco obsoleta. Restauração parcial conserva pendências e erro observável. Nenhuma escolha de armazenamento foi feita nesta consolidação.

A minuta [stabilize-show-desktop-restoration](drafts/stabilize-show-desktop-restoration/proposal.md) é preservada como rascunho. Seu baseline e classificação de capacidade precisam de revisão antes de qualquer proposta real: não existe capacidade-base `show-desktop` no OpenSpec vigente. Nenhum arquivo de `openspec/` foi modificado.

## Pendências funcionais preservadas

- P-17.1: origem por botão em outra barra versus monitor focado; capturar antes do overlay e verificar no compositor. D-42 fixa monitor em foco, sem aprovar implicitamente origem alternativa.
- P-17.2: saída de workspace e nova janela resolvidas respectivamente por D-43/D-44; P-17.2c resolvida pela D-45 **para clique explícito na dock**. Foco de app existente via outros caminhos/single-instance continua em análise.
- P-17.3: Esc sem ativação manter o estado Mostrar área de trabalho é proposta. Notas antigas que sugerem restaurar ao abrir Overview foram substituídas pela orientação de inventário lógico sem revelação física; coordenação técnica permanece pendente.
- Fullscreen, pinned/sticky, scratchpads, classificação de janela elegível, session lock, hotplug e política de falha ainda exigem especificação e teste.

## Matriz de aceitação D-43–D-45 — não executada

As 46 entradas abaixo preservam os IDs e critérios da matriz externa v1.2. São critérios planejados: a consolidação não criou testes, não executou o software e não certifica esses comportamentos. Tipos e resultados da fonte não provam implementação.

| ID | Pré-condição | Evento | Resultado exigido | Tipo | Estado nesta consolidação |
|---|---|---|---|---|---|
| `SD-D43-01` | A2 revelado, B5 normal | A alterna A2→A3 | Restaura somente A2; B5 intacto; retornar A2 normal | Integração | Não executado |
| `SD-D43-02` | A2 revelado | Foco vai para monitor B sem mudar A2 | A2 continua revelado | Integração | Não executado |
| `SD-D43-03` | A2 revelado | Duplo evento A2→A3 | Restore idempotente, sem toggle extra | Unitário/eventos | Não executado |
| `SD-D43-04` | A2 com duas janelas visíveis e uma já minimizada | Revelar, trocar A3 e voltar | Duas visíveis; a terceira segue minimizada | E2E | Não executado |
| `SD-D43-05` | A2 Floating com geometrias distintas | Revelar, alternar A3 e voltar | Posições, empilhamento onde suportado, marcas e dimensões preservadas | E2E | Não executado |
| `SD-D43-06` | A2 Tiling | Revelar, alternar, retornar | Modo e distribuição intactos | E2E | Não executado |
| `SD-D43-07` | A2 Scroll com cliente fora do viewport | Revelar, alternar, retornar | Ordem/posição do Scroll preservadas; todas janelas acessíveis | E2E | Não executado |
| `SD-D43-08` | A2 revelado | Janela escondida fecha | Não ressuscitar nem operar endereço reaproveitado | Unitário/eventos | Não executado |
| `SD-D43-09` | A2 revelado | Janela muda de workspace manualmente | Não sequestrar janela nem ignorar conflito | Integração | Não executado |
| `SD-D43-10` | A2 revelado | Abrir Visão Geral e explorar B5; Esc | Sem ativação de B5 e sem restore por inspeção | E2E | Não executado |
| `SD-D43-11` | A2 revelado | Reload do núcleo Lua | Estado reconciliado; nenhuma janela presa no especial | Resiliência | Não executado |
| `SD-D43-12` | A2 revelado | Reiniciar compositor / sessão nova | Não aplicar snapshot de sessão anterior a janelas novas | Resiliência | Não executado |
| `SD-D43-13` | A2 revelado | Remover monitor A | Recuperar janelas sem operar monitor inexistente | Resiliência | Não executado |
| `SD-D43-14` | A2 revelado | Comando de restauração falha parcialmente | Estado mantém pendências; erro/diagnóstico; novo retry seguro | Falha | Não executado |
| `SD-D43-15` | A2 revelado | Timer/retorno atrasado após restore completo | Nenhuma nova ocultação ou deslocamento | Corrida | Não executado |
| `SD-D43-16` | A2 revelado | Movimento rápido A2→A3→A2 | Sem reentrada em estado revelado nem duplicidade | Corrida | Não executado |
| `SD-D43-17` | A2 revelado | Desconectar/reconectar monitor com ID distinto | Verificação de IDs; sem trocar workspace errado | Multimonitor | Não executado |
| `SD-D43-18` | A2 Floating | Minimizar/restaurar uma janela antes e depois do fluxo | Marca do Floating preservada, regressão do bug conhecido | Regressão | Não executado |
| `SD-D43-19` | Outro workspace/monitor tem apps fullscreen | Revelar/sair de A2 | Não afetar fullscreen remoto; proteção de sessão intacta | Segurança | Não executado |
| `SD-D43-20` | A2 revelado, uma janela aberta na dock | A2→A3 | Dock volta ao estado real após restore, sem falsos minimizados | UI | Não executado |
| `SD-D44-01` | A2 Floating revelado, Firefox e Terminal ocultos | Abrir Arquivos efetivamente em A2 | Firefox/Terminal restaurados, Arquivos visível, posições anteriores preservadas quando aplicável | E2E | Não executado |
| `SD-D44-02` | A2 revelado e uma janela já minimizada antes | Criar nova janela elegível em A2 | Janela previamente minimizada continua minimizada | Regressão | Não executado |
| `SD-D44-03` | A2 revelado, B5 normal | Abrir janela em B5 | A2 permanece revelado; B5 segue independente | Multimonitor | Não executado |
| `SD-D44-04` | A2 revelado | Spawn sem mapeamento / programa falha | A2 não restaura por D-44 | Unitário | Não executado |
| `SD-D44-05` | A2 revelado | Notificação ou popup não elegível | Não dispara restore de A2 | Integração | Não executado |
| `SD-D44-06` | A2 revelado | Nova janela e mudança A2→A3 com eventos concorrentes | Restauração única por geração, sem toggle adicional | Corrida | Não executado |
| `SD-D44-07` | A2 Tiling revelado | Abrir nova janela em A2 | Modo Tiling preservado; novas janelas participam do layout normal | E2E | Não executado |
| `SD-D44-08` | A2 Scroll revelado e janela fora do viewport | Abrir janela elegível em A2 | Todas as janelas acessíveis, ordem/estado do Scroll consistentes | E2E | Não executado |
| `SD-D44-09` | A2 revelado | Janela mapeia e fecha antes da reconciliação | Não atinge identidade reutilizada nem deixa ocultação irrecuperável | Corrida | Não executado |
| `SD-D44-10` | A2 revelado | Aplicativo de instância única reutiliza janela existente | Não confundir com criação; registrar decisão pendente P-17.2c | Limite contratual | Não executado |
| `SD-D44-11` | A2 revelado | Reload durante restore disparado por nova janela | Recuperação rastreável e janelas não ocultas permanentemente | Resiliência | Não executado |
| `SD-D44-12` | A2 revelado | Nova janela numa sessão/monitor identificado de forma obsoleta | Revalidar e impedir restore com destino antigo | Resiliência | Não executado |
| `SD-D45-01` | A2 Floating revelado; Firefox e Terminal ocultos | Clique explícito em Firefox na dock | Restaurar Firefox e Terminal; focar Firefox; NORMAL | E2E | Não executado |
| `SD-D45-02` | A2 revelado; Música já minimizada antes | Clique em Firefox oculto | Música permanece minimizada; demais janelas antes visíveis restauradas | Regressão | Não executado |
| `SD-D45-03` | A2 Floating revelado; Firefox era a última janela focada | Clique em Firefox | Restaurar e focar; **não minimizar** por regra de click-toggle Floating | Regressão | Não executado |
| `SD-D45-04` | A2 revelado, B5 com janelas visíveis | Clique em Terminal de A2 | B5 permanece intacto, foco final em Terminal de A2 | Multimonitor | Não executado |
| `SD-D45-05` | A2 revelado; hover de ícone ou prévia | Hover sem clique | Nenhuma restauração, foco ou minimização | UI | Não executado |
| `SD-D45-06` | A2 revelado; janela oculta fecha antes de clique | Clicar entrada obsoleta | Não focar endereço reutilizado nem outra janela por engano; recuperar entradas restantes | Corrida | Não executado |
| `SD-D45-07` | A2 revelado; Firefox oculto | Clique coincide com saída A2→A3 | Uma única restauração; foco não volta indevidamente a A2 após troca | Corrida | Não executado |
| `SD-D45-08` | A2 revelado; Firefox oculto | Clique coincide com nova janela em A2 | Uma única restauração D-44/D-45; foco conforme intenção vigente e backend | Corrida | Não executado |
| `SD-D45-09` | A2 Tiling revelado | Clique em cliente oculto | Restaurar conjunto sem mudar layout; focar cliente selecionado | E2E | Não executado |
| `SD-D45-10` | A2 Scroll com janela fora do viewport | Clicar sua entrada de dock quando elegível | Restaurar e focar sem perda da janela nem corrupção do Scroll | E2E | Não executado |
| `SD-D45-11` | A2 revelado; aplicativo favorito só aberto em B5 | Clique no favorito | Aplicar política existente de **nova janela em A2**; não restaurar A2 diretamente por D-45 | Regra cruzada | Não executado |
| `SD-D45-12` | A2 revelado; cliente temporariamente oculto | Restore concluído, comando de foco falha | Estado normal preservado, erro de foco informado, sem reocultação | Falha | Não executado |
| `SD-D45-13` | A2 revelado; cliente oculto | Reload ou hotplug entre clique e foco | Revalidar sessão/monitor/workspace/cliente; sem foco em alvo inválido | Resiliência | Não executado |
| `SD-D45-14` | A2 revelado; abrir overview e inspecionar cliente | Inspeção sem selecionar/focar | Não disparar D-45; sem restauração involuntária | Integração | Não executado |

Os oráculos futuros devem observar estado do compositor e procedência, não apenas cache da dock ou comando aceito. Simuladores precisam emitir transições/eventos e injetar duplicidade, atraso, falha, fechamento e reutilização de endereço. Repetir nos três modos e em um/dois monitores; regressões de minimizar, modos e Snap Assist precisam continuar cobertas.

### Identificadores anteriores e colisão histórica

Os cenários `OV-001`–`OV-012`, `MULTI-01`–`MULTI-08`, `SD-001`–`SD-003` e `UX-001` permanecem na [visualização original](../history/product-import/2026-10-28/external-originals/docs/product/features/VISUALIZACAO_WORKSPACES_AREA_DE_TRABALHO_v1.0.md). Os `OV-42-01`–`OV-42-06` e `SD-42-01`–`SD-42-08` permanecem no [contrato v1.4](../history/product-import/2026-10-28/external-originals/docs/product/engineering/CONTRATO_VISAO_GERAL_FOCO_RESTAURACAO_v1.4.md).

A [fonte de foco v0.5](../history/product-import/2026-10-28/external-originals/docs/product/features/FOCO_SHOW_DESKTOP_MULTIMONITOR_v0.5.md) usa também `SD-42-01`–`SD-42-06`, com alguns oráculos diferentes. Até uma revisão técnica aprovada, identificar cada um por **arquivo de origem + ID**; não fundir IDs nem afirmar que um resultado valida ambos. Todos permanecem planejados/não executados no contexto do produto.

Os códigos `D43-R01`–`D43-R12` e `D44-R01`–`D44-R10` do contrato original preservam o detalhamento de invariantes. Sua terminologia técnica não é promovida a API atual. Não usar mockups como oráculos de geometria ou captura real.

## Fontes e precedência

- [Visão v3.13, §§39–49](../history/product-import/2026-10-28/external-originals/docs/product/vision/ESPECIFICACAO_INTERFACE_SUITE_v3.13.md): escolhas e adendos, com grau literal em [DECISIONS](DECISIONS.md).
- [Visualização v1.0](../history/product-import/2026-10-28/external-originals/docs/product/features/VISUALIZACAO_WORKSPACES_AREA_DE_TRABALHO_v1.0.md): histórico das propostas; adendos finais resolvem D-43–D-45.
- [Foco v0.5](../history/product-import/2026-10-28/external-originals/docs/product/features/FOCO_SHOW_DESKTOP_MULTIMONITOR_v0.5.md): nome v0.5 com cabeçalho v0.4, preservado sem correção da fonte.
- [Contrato v1.4, especialmente §§10–12](../history/product-import/2026-10-28/external-originals/docs/product/engineering/CONTRATO_VISAO_GERAL_FOCO_RESTAURACAO_v1.4.md): adendos prevalecem sobre pendências históricas somente nos subcasos resolvidos.
- [Matriz v1.2 integral](../history/product-import/2026-10-28/external-originals/docs/product/engineering/MATRIZ_TESTES_SHOW_DESKTOP_D43_D44_D45_v1.2.md) e [manifesto](../history/product-import/2026-10-28/SOURCES.json).
