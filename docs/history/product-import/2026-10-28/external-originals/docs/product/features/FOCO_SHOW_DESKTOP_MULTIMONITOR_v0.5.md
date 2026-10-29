# D-41 e D-42 — Foco, Visão Geral e Mostrar área de trabalho em múltiplos monitores

**Versão:** 0.4  
**Data:** 28/10/2026  
**Natureza:** regras funcionais aprovadas e propostas explícitas; não descreve código implementado.  
**Documento normativo consolidado:** `CONTRATO_VISAO_GERAL_FOCO_RESTAURACAO_v1.0.md`.

## Decisões vigentes

| Decisão | Status | Regra |
|---|---|---|
| D-38 | Aprovada | Visão Geral abre em Todos |
| D-39 | Aprovada | Clique em cartão detalha; botão Ir ativa |
| D-40 | Aprovada | Todos os workspaces agrupados por monitor e filtros opcionais |
| D-41 | Aprovada | Superfície única no monitor em foco |
| **D-42** | **Aprovada — opção A** | **Mostrar área de trabalho afeta somente o workspace ativo do monitor em foco; demais monitores não são afetados** |

## Resolução de foco

Distinguir **monitor hospedeiro do overlay**, **seleção de exploração** e **foco efetivo do compositor**. Inspecionar workspaces de outro monitor não transfere foco. `Ir` ou escolher janela é a operação explícita; validar que o destino ainda existe e ativá-lo em seu monitor real, sem mover o workspace.

## Mostrar área de trabalho — comportamento de produto

1. Capturar o workspace ativo do monitor em foco **na invocação**.
2. Ocultar temporariamente suas janelas elegíveis; não tocar em outros monitores, workspaces ou janelas originalmente minimizadas.
3. Acionar o comando novamente nesse escopo restaura o estado anterior, sem alterar layout ou modo.
4. Manter aplicativos abertos na dock; não mudar indicadores para minimizados só por causa da revelação temporária.
5. Com Visão Geral aberta, exibir inventário lógico de janelas ocultas **sem restaurar visualmente as janelas atrás do overlay**.
6. Em erro/hotplug, preferir recuperação de janelas visíveis a ocultação irrecuperável.

## Situações pendentes (P-17, não novas decisões)

- Origem por clique em barra vs atalho global.
- Ao alternar workspace, abrir nova janela ou selecionar janela oculta: recomendada restauração automática do escopo anterior, sujeita à avaliação da política Lua.
- Ao fechar a Visão Geral com Esc, recomendar que Mostrar área de trabalho não seja desativado implicitamente.
- Políticas para fullscreen, pinned/sticky, bloqueio seguro e sobrevivência a reload/crash.

## Matriz de testes mínima

| ID | Entrada | Resultado esperado |
|---|---|---|
| SD-42-01 | A2 revelado, B5 visível | B5 permanece inalterado |
| SD-42-02 | Clique repetido no ícone em A2 | A2 restaurado, minimizações prévias preservadas |
| SD-42-03 | Overview de A com A2 revelado | A2 não parece vazio |
| SD-42-04 | Inspecionar B5 a partir de A | Não transfere foco a B |
| SD-42-05 | `Ir` para B5 | Ativa B5 em B sem movê-lo |
| SD-42-06 | Hotplug/crash com A2 revelado | Reconciliação e recuperação sem janelas irrecuperáveis |

**Para a máquina de estados, cenários completos e responsabilidades de implementação:** consultar `CONTRATO_VISAO_GERAL_FOCO_RESTAURACAO_v1.0.md`. O histórico exploratório permanece em `FOCO_SHOW_DESKTOP_MULTIMONITOR_v0.1.md`.

---

## Adendo D-43 — Encerramento automático de revelação quando o workspace é abandonado

**Aprovado:** trocar o workspace ativo no **mesmo monitor** termina o estado Mostrar área de trabalho do workspace anterior, com restauração exclusiva das janelas afetadas. Não se confunde com apenas alterar o monitor em foco, tampouco com explorar cartões da Visão Geral. Ao retornar, a apresentação anterior estará recuperada, sem reativar minimizações prévias.

**Ainda pendente:** criação de janela no workspace revelado, restauração seletiva ao escolher uma janela, fullscreen e recuperação de crash/hotplug em nível de implementação. Ver `CONTRATO_VISAO_GERAL_FOCO_RESTAURACAO_v1.2.md` e minuta OpenSpec.


---

## D-44 — Nova janela restaura o workspace revelado

Aprovado: criação/mapeamento efetivos de uma nova janela elegível no workspace atualmente revelado **encerra Mostrar área de trabalho**, restaurando todas as janelas que estavam visíveis antes da operação e permitindo a exibição normal da janela nova. Janelas previamente minimizadas permanecem minimizadas. Uma tentativa de lançar aplicativo, uma notificação ou uma janela criada em outro workspace **não** dispara D-44 por si só. Se o compositor sinalizar simultaneamente saída de workspace (D-43), a restauração deve ser idempotente por geração.

**Pendente:** foco numa janela existente sem criação de outra (`P-17.2c`), casos especiais e mecanismos concretos. Contrato vigente: `CONTRATO_VISAO_GERAL_FOCO_RESTAURACAO_v1.3.md`.

---

## D-45 — Clique na dock para selecionar janela temporariamente ocultada

**APROVADA em 28/10/2026.** Ao clicar explicitamente na dock sobre uma janela já aberta e temporariamente ocultada por Mostrar área de trabalho, a suíte **encerra o estado de revelação**, restaura **todas** as janelas que eram visíveis e foram ocultadas e **foca a janela selecionada**. As que estavam minimizadas antes continuam minimizadas. A D-45 tem precedência sobre a regra do Floating de minimizar por clique na janela focada.

Na presença de dois monitores, o escopo é somente `(monitor,workspace)` da revelação. As janelas do outro monitor continuam sem alteração; não transportar workspaces para o monitor que recebeu a ação. Se o alvo fechar, o foco não deve incidir em cliente obsoleto ou reciclado. Hover e inspeção da Visão Geral não são gatilhos.

A restauração usa a mesma operação idempotente das D-43/D-44. O foco é um segundo resultado confirmado; sua falha não reverte a restauração. A especificação completa está em `CONTRATO_VISAO_GERAL_FOCO_RESTAURACAO_v1.4.md`. P-17.2c fica resolvida para o clique explícito na dock; outras vias exigem mapeamento do código real, sem duplicar o controlador.
