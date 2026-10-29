# Contrato funcional — Visão Geral, foco e Mostrar área de trabalho

**Versão:** 1.4  
**Data:** 28/10/2026  
**Estado:** consolidação das decisões **D-36 a D-45**. A seção **12** prevalece sobre referências históricas a P-17.2c pendente; a seção **11** consolida D-44. As propostas operacionais estão identificadas como **P-17** e permanecem sujeitas a inspeção e aprovação técnica.  
**Produto:** interface oficial da futura suíte Arch; nome público ainda indefinido.  
**Código:** não foi inspecionado novamente nesta etapa; nenhum item abaixo implica implementação realizada.

## 1. Matriz de decisões aprovadas

| ID | Regra aprovada | Fronteira |
|---|---|---|
| D-36 | Mostrar área de trabalho é ocultação temporária reversível, não minimização definitiva | Não substituir estados do núcleo Lua |
| D-37 | Visão Geral de workspaces e visão das janelas de um workspace | Não é launcher, dashboard nem VXWM |
| D-38 | Visão Geral sempre começa em **Todos os workspaces** | Reabrir não deve recuperar um filtro anterior como tela inicial |
| D-39 | Clique no cartão abre **detalhamento**; botão `Ir` ativa o workspace | Exploração ≠ ativação |
| D-40 | Todos os workspaces agrupados por monitor, com filtros opcionais | Não esconder monitores sem escolha explícita do usuário |
| D-41 | Uma única superfície de Visão Geral no monitor em foco | Outros monitores não recebem overlay duplicado |
| **D-42** | **Mostrar área de trabalho afeta só o workspace ativo do monitor em foco** | Outros monitores/workspaces ficam inalterados |

## 2. Entidades de estado

- `overview_host_monitor`: monitor no qual a Visão Geral foi invocada, capturado antes do overlay.
- `focus_snapshot`: referência validável para monitor/workspace/janela efetivos antes da abertura, **não** cópia imutável de ponteiros internos do Hyprland.
- `explored_workspace`: cartão ou detalhe selecionado visualmente; não altera compositor por si só.
- `activation_target`: workspace ou janela que o usuário escolheu explicitamente para ativar.
- `show_desktop_scope`: par de monitor e **workspace ativo naquele monitor no instante de invocação** (resolver com identidade estável e revalidar em hotplug).
- `temporary_hidden_windows`: conjunto das janelas afetadas, com proveniência e atributos necessários à restauração. Não mudar `minimized` de `false` para `true` para simular a função.

**Invariante I-01:** a seleção na Visão Geral não altera o foco efetivo do gerenciador, nem move janela ou workspace entre monitores.  
**I-02:** só ativação explícita transfere foco ao destino, que precisa ser revalidado.  
**I-03:** Mostrar área de trabalho não modifica definitivamente geometria, layout ou minimização.  
**I-04:** ausência de capturas em tempo real não oculta janelas da lista; mostrar fallback textual quando necessário.  
**I-05:** dados obsoletos não podem iniciar ação sobre monitor/janela inexistente.

## 3. Fluxo aprovado de Visão Geral

```text
CLOSED
  └─ abrir no monitor A ─► ALL(host=A, groups=[A,B,...])
                            ├─ filtro B ─► ALL(filtered=B, host=A)
                            ├─ clique cartão B5 ─► DETAIL(B5, host=A)
                            │                         ├─ Voltar/Esc ─► ALL
                            │                         ├─ selecionar janela X ─► validar X, ativar em B, fechar
                            │                         └─ Ir para B5 ─► validar B5, ativar em B, fechar
                            ├─ Ir para B5 ─► validar B5, ativar em B, fechar
                            └─ Esc ─► CLOSED, recuperar foco anterior se ainda válido
```

**Foco de interface versus foco real:** uma superfície de overview pode receber teclado enquanto o gerenciador mantém a referência à janela anterior. O modelo de UI deve explicitar essa diferença. Ao selecionar uma janela minimizada, a operação de restauração é direcionada **apenas à janela-alvo** no workspace/monitor corretos, e não a todas as janelas do workspace.

**Proposta P-17.1 (origem):** atalho usa o monitor focado no compositor; botão usa a barra de onde veio a interação, se o compositor confirmar essa origem. Capturar origem antes de abrir overlay e validar o resultado em testes Wayland.

## 4. Fluxo aprovado de Mostrar área de trabalho — D-42

```text
NORMAL(scope=A:workspace-2)
  └─ ícone/atalho ─► REVEALED_TEMPORARILY(scope=A:workspace-2)
                         ├─ repetir no mesmo escopo ─► RESTORING ─► NORMAL
                         ├─ abrir overview ─► mostrar inventário lógico (sem revelar tela subjacente)
                         ├─ escolher janela-alvo ─► RESTORING seguro ─► ativar janela
                         ├─ troca de workspace ─► P-17.2: restaurar antes de sair
                         ├─ nova janela elegível mapeada no escopo ─► RESTORING(reason=new_window) [D-44]
                         └─ hotplug / reload / erro ─► reconciliação segura, preferir visibilidade
```

1. Resolver o **workspace ativo do monitor em foco no instante da ação**; não usar simplesmente o workspace global mais recentemente visitado em outro monitor.
2. Ocultar temporariamente apenas janelas elegíveis daquele workspace; preservar as janelas de outros workspaces, monitores e superfícies de shell.
3. Manter registro das janelas afetadas e dos atributos anteriores; janelas previamente minimizadas **continuam minimizadas** ao restaurar.
4. O botão indica que o escopo está temporariamente revelado; ícones da dock continuam indicando aplicativos abertos, não minimizados por causa dessa ação.
5. Ao acionar novamente o comando no mesmo escopo, restaurar a apresentação anterior de forma idempotente e tolerante a janelas fechadas.
6. Em casos críticos de inconsistência, priorizar tornar as janelas recuperáveis e não deixá-las permanentemente invisíveis.

**Exemplo multimonitor aprovado:** Monitor A mostra workspace 2 (Floating) e monitor B mostra workspace 5 (Tiling). Acionar Mostrar área de trabalho no A oculta as janelas elegíveis do workspace A2 e mantém B5 intacto. Clicar novamente no comando do mesmo escopo restaura A2 sem mexer em B5.

## 5. Convivência entre Visão Geral e Mostrar área de trabalho

| Contexto | Resultado esperado | Aprovação |
|---|---|---|
| A2 está revelado e usuário abre overview no A | Cartão A2 mantém janelas no inventário lógico, sem torná-las visíveis por engano | Derivado de D-36/D-37; técnica proposta |
| Dentro do overview, usuário apenas filtra B e examina B5 | A2 permanece revelado; compositor não ativa B5 | D-39/D-40/D-41; coordenação proposta |
| Usuário escolhe janela de A2 | Encerrar ocultamento de A2 antes de restaurar/focar alvo | P-17.2 |
| Usuário escolhe janela de B5 | Ativar B5 no monitor B; política para liberar A2 ao abandonar o contexto proposta em P-17.2 | Ativação explícita aprovada; restauração proposta |
| Usuário fecha overview com Esc | Retornar ao contexto anterior se ainda válido, mantendo o estado local de Mostrar área de trabalho | P-17.3 |
| Troca de monitor sem ativar janela do workspace revelado | Não alterar o outro monitor; manter estado local até ação de saída definida | D-42 + proposta |

**Recomendação P-17.2 — término seguro da revelação:** restauração automática ao **trocar o workspace ativo daquele monitor**, ao abrir uma nova janela nesse workspace ou ao escolher explicitamente uma janela anteriormente oculta. Isso evita deixar tarefas invisíveis sem contexto. Não tratar esta recomendação como decisão aprovada sobre temporização ou comportamento de saída.

**P-17.3 — cancelamento:** `Esc` no detalhe volta para Todos; `Esc` em Todos fecha; se a pessoa estava com Mostrar área de trabalho acionado, **fechar sem escolher janela não obriga a restaurar as janelas ocultas**, pois são funções separadas.

## 6. Multimonitor, foco e eventos de alteração

- **Monitor removido:** cancelar ações pendentes para identificadores antigos e recompor lista de monitores/workspaces. Se houver janelas temporariamente ocultas nele, reconciliar ou restaurar antes da migração, sempre que possível.
- **Workspace movido para outro monitor:** revalidar relação com saída física; não presumir que o ID do workspace implica monitor imutável.
- **Selecionar janela remota:** usar mecanismo de foco que não transfira o workspace ao monitor hospedeiro da Visão Geral. A semântica do dispatcher deve ser verificada na versão local do Hyprland.
- **Escalas diferentes/HiDPI:** calcular tamanho e posições do overlay em unidades lógicas; não adaptar cores por monitor — D-31 permanece tema global.
- **Janela fechada durante a prévia:** eliminar apenas o destino obsoleto, sem selecionar outra janela acidentalmente.
- **Scroll:** representar janelas fora do viewport; abertura da prévia não muda a posição do scroll.
- **Fullscreen, jogos, sticky/pinned e especiais:** definir política e ensaios próprios antes de tentar mover/ocultar clientes. Não interferir com bloqueadores de tela e superfícies de segurança.
- **Crash/reload:** a recuperação não pode depender exclusivamente de uma variável transitória da UI GTK ou do shell; backend deve ter mecanismo de reconciliação/recuperação testado.

## 7. Testes de aceitação

| ID | Cenário | Critério |
|---|---|---|
| `OV-42-01` | Abrir overview no A com B conectado | Somente overlay em A, grupos de A+B, modo Todos |
| `OV-42-02` | Abrir cartão B5 | Detalhe de B5, foco efetivo continua em A |
| `OV-42-03` | Ação Ir B5 | Workspace B5 ativado em B, sem transporte ao A |
| `OV-42-04` | Selecionar janela minimizada de B5 | Janela-alvo restaurada no monitor correspondente |
| `OV-42-05` | Esc: detalhe → todos → fechar | Sem alteração de modo, posição ou workspaces |
| `OV-42-06` | B desconectado durante detalhe | Sem dispatch para destino obsoleto |
| `SD-42-01` | Mostrar área de trabalho com A2 e B5 visíveis | A2 revelado; B5 intocado |
| `SD-42-02` | Duplo acionamento em A2 | Restaura A2 sem alterar estado de B5 |
| `SD-42-03` | A2 tinha janela minimizada antes | Janela continua minimizada após restauração |
| `SD-42-04` | Abrir overview com A2 temporariamente revelado | A2 mantém janelas em prévia/lista; tela subjacente não pisca |
| `SD-42-05` | Nova janela, troca workspace, fechar janela no A2 revelado | Restauração segura sem perder geometria (P-17.2) |
| `SD-42-06` | Hotplug ou falha no backend | Nenhuma janela irrecuperavelmente oculta |
| `SD-42-07` | Teste em Tiling, Scroll e Floating | Modos e disposição persistidos sem alteração indevida |
| `SD-42-08` | Fullscreen e tela de bloqueio | Sem exposição indevida de janelas ou interferência em proteção de sessão |

## 8. Integração com código existente — sem reescrita presumida

**Responsabilidades a auditar:** núcleo Lua — identidade e estados de minimização/restore, proveniência de janela/workspace e política de ocultamento; plugin C++ — capacidades gráficas e interação; UI Rust/GTK4 — Settings, sem exigir que o overlay seja escrito nesse toolkit; Hyprland IPC — estado atual e eventos de foco/hotplug. Não presumir suporte a thumbnails em tempo real ou garantia de persistência de snapshot após crash. O desenho de solução deverá obedecer aos contratos existentes e ao versionamento local.

**Documento de referência visual:** `DESIGN_SYSTEM_INTERFACE_OFICIAL_v1.7.md`, `BRIEF_MOCKUPS_INTERFACE_OFICIAL_v1.0.md`. Arte anterior M-09 é conceitual e pode mostrar rótulos/ícones a revisar; não substitui os contratos acima.

## 9. Pendências explícitas

- **P-17.1:** semântica exata de origem do comando por botão vs atalho.
- **P-17.2:** restauração automática ao sair do workspace revelado, abrir nova janela ou ativar janela oculta.
- **P-17.3:** cancelamento de overview enquanto Show Desktop está ativo.
- Política de fullscreen e sticky/pinned, captura/privacidade de miniaturas, solução de recuperação pós-falha.
- Testes reais com versão instalada do Hyprland, núcleo Lua e plugin C++.

**Estado final desta fase:** D-42 aprovada, requisitos centrais consolidados. Nenhum mockup novo foi tratado como prova de implementação.


---

## Apêndice M-10 — validação visual, não técnica

As pranchas M-10 ilustram estados aprovados **D-36 a D-44**; não executam os comandos. O objetivo é reconhecer um erro de comportamento antes de escrever código.

| Teste visual | Estado inicial | Ação | Esperado |
|---|---|---|---|
| M10-01 | A2 e B5 têm janelas | Revelar A2 | A2 sem suas janelas elegíveis; B5 idêntico |
| M10-02 | A2 revelado | Abrir overview no A | Uma superfície no A; grupos A+B; A2 mantém inventário lógico |
| M10-03 | Overview no A | Clicar cartão B5 | Detalhe em A; foco efetivo não muda para B |
| M10-04 | Detalhe B5 | Clicar `Ir` ou janela | Resolver e ativar destino no B, sem mover workspace entre monitores |
| M10-05 | A2 revelado | Repetir comando em A2 | Restaurar apenas janelas previamente visíveis, conservar previamente minimizadas |
| M10-06 | A2 revelado | Erro/reload/hotplug | Não deixar janelas inacessíveis, validar recuperação |

**P-17 permanece proposta:** a política de restauração automática ao sair de um workspace revelado, ao abrir nova janela e ao ativar uma janela oculta. Não atribuir status de decisão.

**Limites do mockup:** imagens geradas são conceituais e podem divergir em números, títulos, modos ou posições entre quadros. Eles **não** constituem prova de fidelidade pixel a pixel, capturas reais, implementação de miniaturas ou atalhos aprovados. Em qualquer divergência, prevalecem D-36 a D-42 e os invariantes definidos neste documento.

---

## 10. ADENDO NORMATIVO D-43 — Restauração ao trocar de workspace

**Estado:** D-43 aprovada. Esta seção **substitui exclusivamente** a indicação de que a restauração automática por troca de workspace em P-17.2 estava pendente. Outros subcasos P-17 permanecem em aberto.

### 10.1 Definições de domínio

- **Escopo revelado:** par `monitor_id` + `workspace_id` validado no compositor, associado a uma operação de revelação `reveal_generation` e à sessão do Hyprland.
- **Proveniência por janela:** workspace lógico de origem, monitor associado quando identificável, modo de janela relevante (incluindo marca Floating prévia), estado de minimização anterior e **indicação de que a janela foi ou não efetivamente ocultada por esta operação**.
- **Origem preservada:** dados mínimos necessários para reverter somente as operações executadas pelo Mostrar área de trabalho; não confiar somente no endereço da janela, que pode se tornar obsoleto.
- **Saída efetiva:** troca do workspace ativo **do mesmo monitor dono do escopo** de `W` para `W2`. Mudar apenas o monitor com foco, cursor ou teclado não é saída.
- **Recuperação:** reconciliação do registro com o estado real; sucesso confirmado significa que janelas elegíveis estão acessíveis, não apenas que um comando foi enviado.

### 10.2 Máquina de estados — contrato de produto

```text
NORMAL(M,W)
   │ toggle mostrar área (resolvendo M,W atuais)
   ▼
REVEALING(generation)
   ├─ falha sem efeito ─► NORMAL + erro explícito
   └─ confirmação ─► REVEALED(M,W,generation)
                            ├─ segundo toggle ─► RESTORING(reason=toggle)
                            ├─ mudança workspace ativo de M: W → W2
                            │                    └─► RESTORING(reason=exit_workspace) [D-43]
                            ├─ somente mudança de foco para monitor B ─► REVEALED
                            ├─ inspeção de cartão na Visão Geral ─► REVEALED
                            └─ crash/reload/hotplug ─► RECOVERING
RESTORING
   ├─ confirmado ─► NORMAL (descarta registro da geração)
   ├─ janela fechada ─► ignora destino ausente, prossegue com demais
   ├─ falha parcial ─► RECOVERING (retém só entradas pendentes)
   └─ evento duplicado/obsoleto ─► nenhuma ação destrutiva extra
RECOVERING
   ├─ reconciliação concluída ─► NORMAL
   └─ indisponibilidade real ─► erro recuperável / orientação de recuperação
```

A entrega de eventos pelo Hyprland pode ocorrer depois da troca efetiva. O sistema **deve** tratar evento + consulta do estado como verificação conjunta, sem pressupor atomicidade do evento com o frame do compositor. Se o observador iniciar após uma troca perdida, a próxima reconciliação deve detectar `W != activeWorkspace(M)` e restaurar.

### 10.3 Regras de restauração D-43 e invariantes

| Código | Condição | Resultado obrigatório |
|---|---|---|
| `D43-R01` | A2 revelado, A ativa A3 | Restaurar janelas afetadas de A2 sem tocar A3 ou B5 |
| `D43-R02` | Mudou monitor focado A→B, A continua A2 | Preservar A2 revelado; não interpretar como saída |
| `D43-R03` | Compositor entrega 2 eventos de mudança A2→A3 | Restaurar no máximo uma vez por geração; sem inverter janela duas vezes |
| `D43-R04` | Antes de revelar, uma janela de A2 já estava minimizada | Não a reativar ou marcá-la como visível |
| `D43-R05` | Janela fechou durante revelação | Ignorar seu destino no restore; nunca atuar sobre outra janela cujo endereço foi reutilizado |
| `D43-R06` | Uma janela foi movida manualmente para workspace diferente | Não forçá-la de volta sem verificar proveniência e intenção de usuário; sinalizar conflito recuperável |
| `D43-R07` | A2 estava em Scroll, Tiling ou Floating | Preservar modo, ordenação/layout e marcas de Floating antes/depois |
| `D43-R08` | Estado ocultado persiste durante reload | Após reiniciar o núcleo, revalidar sessão e recuperar ou reexpor janelas por mecanismo seguro |
| `D43-R09` | Monitor A removido/reconfigurado | Não executar ações com ID obsoleto; tentar recuperação em monitor válido, sem mover janela erroneamente |
| `D43-R10` | Outro monitor B tem janelas | Nenhuma operação de restauração modifica suas janelas |
| `D43-R11` | Visão Geral apenas inspeciona B5, enquanto A2 revelado | Não restaura nem altera foco efetivo |
| `D43-R12` | Visão Geral ativa B5, sem mudar workspace ativo do monitor A | A2 não é necessariamente encerrado pela D-43; políticas adicionais pendentes |

**Idempotência:** repetir `restore(generation)` não deve produzir novo deslocamento ou refazer minimização. **Reconciliação:** aplicar decisões somente quando identidade, geração, vínculo com a sessão e destino puderem ser revalidados. **Segurança:** em falha, preferir visibilidade recuperável à ocultação permanente, mas sem restaurar janelas previamente minimizadas. **Convivência:** o inventário de janelas da Visão Geral deve incluir janelas reveladas, sem forçá-las a aparecer sob o overlay.

### 10.4 Persistência e `reload` — requisito de confiabilidade

A auditoria antiga identificou falha de proveniência após `reload`. Portanto, o contrato exige um caminho verificável de **recuperação após reconfiguração**, que evite o caso de janelas abandonadas no workspace especial. As estratégias técnicas (journal transacional, consulta/reconstrução a partir de metadados e eventual rollback) dependem de auditoria do código real. Não transferir trabalho de disco bloqueante para callbacks Lua do compositor sem avaliar latência e falha.

**Invariantes adicionais:**

- `I-06` — uma janela oculta temporalmente não passa a contar como voluntariamente minimizada.
- `I-07` — restaurar não modifica workspace do monitor B nem move workspaces entre monitores.
- `I-08` — a conclusão só é reportada depois de confirmação do estado efetivo; gravar um arquivo/emitir comando não é sucesso.
- `I-09` — uma recuperação parcial conserva rastreabilidade e oferece retorno explícito; não marca estado limpo prematuramente.

### 10.5 Casos ainda **não aprovados** por D-43

1. O que fazer com **nova janela** no workspace ainda revelado (P-17.2b).
2. O que fazer com **ativação de uma janela revelada** sem troca de workspace (P-17.2c) — foco e preservação já precisam ser seguros, mas o gatilho de restauração não está decidido.
3. Se `Esc` na Visão Geral deve manter o estado de Mostrar área de trabalho (P-17.3, proposta).
4. Tratamento específico de pinned/sticky, scratchpads e fullscreen, sem perder as garantias de sessão.
5. Solução exata de persistência, intervenção de usuário ou recuperação automática diante de estado inválido.

### 10.6 Critérios de aceite e não escopo

A D-43 estará pronta para ser considerada implementada somente depois de testar: saída normal, reentrada, minimizada prévia, múltiplas janelas, A/B independentes, cancelamento no overview, janela fechada e endereço reaproveitado, callback duplicado, reload, hotplug, Scroll com janelas fora do viewport e regressões do Floating. A proposta OpenSpec contém testes rastreáveis.

**Não escopo do change D-43:** redesenhar dock, reescrever Rust/C++/Lua, construir a Rice Store, criar overview em miniaturas completo, mudar modos do gerenciador ou decidir gatilhos ainda pendentes.


---

## 11. ADENDO NORMATIVO D-44 — Restauração ao criar nova janela

**Status:** **APROVADA**. Atualiza **P-17.2b**, antes pendente. Sempre que houver divergência entre uma menção histórica anterior de "nova janela pendente" e esta seção, **prevalece a D-44**.

### 11.1 Gatilho e escopo

**WHEN** um toplevel novo e elegível for **efetivamente mapeado/observado** no `workspace_id` do `monitor_id` de uma geração ativa `REVEALED`, **THEN** o gerenciador encerra a revelação e restaura integralmente **as janelas que aquela operação havia ocultado**, mantendo previamente minimizadas as não afetadas, e deixa a nova janela normalmente utilizável. A condição é comprovada pelo estado do compositor, não por `spawn`/intenção de executar.

- **Não disparar** por notificação, janela do shell, mudança apenas de foco, diálogo não elegível, solicitação de lançamento sem janela nem mapeamento em outro workspace/monitor.
- **Não duplicar** operações se uma troca de workspace no mesmo monitor também ativar a D-43 na mesma geração.
- **Não aplicar** o mesmo snapshot de retorno a uma nova sessão, ID de monitor obsoleto ou endereço de janela reutilizado.
- **Não forçar foco** para uma janela antiga; o foco da janela nova é determinado pelas políticas existentes, com confirmação de que ela está visível.
- Em layouts automáticos Tiling e Scroll, preservar os modos, as janelas e as relações de ordenação pertinentes, reconhecendo que a nova janela pode alterar naturalmente a distribuição.

### 11.2 Matriz de eventos complementares

| ID | Pré-condição e evento | Resultado obrigatório |
|---|---|---|
| `D44-R01` | A2 revelado; janela nova elegível entra em A2 | Restaurar antigas janelas ocultadas, apresentar a nova, NORMAL ao confirmar |
| `D44-R02` | A2 revelado; janela nova entra em B5 | A2 permanece revelado (se nenhuma troca A2→A3 ocorreu) |
| `D44-R03` | Spawn falha ou nenhum toplevel novo é criado | Não restaurar por D-44 |
| `D44-R04` | A2 tem janela previamente minimizada | Mantê-la minimizada após o evento |
| `D44-R05` | D-43 e D-44 disparam simultaneamente | Restaurar uma vez por geração; ambos não podem inverter o estado |
| `D44-R06` | Janela nova fecha antes da confirmação | Reconciliar; não manipular cliente com endereço/identidade reutilizada |
| `D44-R07` | Nova janela altera a disposição Tiling/Scroll | Layout reflete a nova janela sem corrupção do estado anterior |
| `D44-R08` | Superfície de overlay, notificação, layer-shell | Não disparar se não for toplevel de aplicativo elegível |
| `D44-R09` | Reload durante restauração disparada pela nova janela | Recuperar com proveniência rastreável; janelas não irrecuperavelmente ocultas |
| `D44-R10` | Aplicativo de instância única focaliza janela antiga sem criar nova | Não assumir D-44; caso vinculado a P-17.2c ainda aberto |

### 11.3 Máquina de estados (extensão a D-43)

```text
REVEALED(M,W,g)
  ├─ new_toplevel(candidate) → validar workspace, monitor, sessão, identidade e elegibilidade
  │      ├─ não elegível / fora do escopo → REVEALED (nenhuma ação)
  │      └─ elegível no escopo → RESTORING(g, reason=new_window) [D-44]
  ├─ workspace_exit(M,W) → RESTORING(g, reason=workspace_exit) [D-43]
  └─ focus_only / inspect_overview → REVEALED

RESTORING(g): convergência para UM resultado por geração;
  confirmar visibilidade das janelas antes ocultadas e da nova janela,
  reter apenas pendências de recuperação; informar NORMAL apenas após confirmação.
```

**Ordem:** eventos podem chegar fora de ordem. Não prometer que cada janela restaurará antes do primeiro frame da nova; exigir correção observável e teste de ausência de estados irrecuperáveis.

### 11.4 Testes e limites

Os critérios obrigatórios adicionais estão em `MATRIZ_TESTES_SHOW_DESKTOP_D43_D44_v1.1.md`. A mudança OpenSpec preparada deve **estender a mesma change existente**, não criar uma segunda autoridade de Show Desktop. O software não foi modificado nem testado nesta etapa. Miniaturas de Visão Geral, classificação de toplevel, foco de aplicativos de instância única e detalhes de backend precisam ser verificados no repositório real.

---

## 12. D-45 — Seleção explícita pela dock de janela ocultada (APROVADA)

**Escopo:** workspace `W`/monitor `M` com `show_desktop_state=REVEALED`, contendo um cliente existente `X` que foi temporariamente ocultado por Mostrar área de trabalho. O comando de clique na dock resolve uma identidade atual e verificável para `X`.

**Regra:** restaurar **todas** as janelas anteriormente visíveis ocultadas pelo estado REVEALED, preservar as que já estavam minimizadas e então **focar `X`**. Não limitar a restauração à janela `X`. Não converter ocultação temporária em minimização persistente. Se o foco não puder ser confirmado, manter restauração e sinalizar foco não concluído. A seleção deve ocorrer apenas após comando explícito — hover/preview não executa essa política.

### 12.1 Ordem lógica da operação

1. Receber ação **explícita** `dock_activate(target_window_id)` e consultar estado atual; não confiar em cache visual da dock.
2. Verificar se `X` realmente integra o conjunto de janelas **temporariamente ocultadas** em `W`/`M`, na **sessão e geração atuais**. Se não integrar, aplicar a política normal da dock correspondente, sem fabricar uma revelação.
3. Solicitar `restore(scope=W,M,g,reason=dock_activate)`, pela **mesma máquina de estados** usada em D-43 e D-44; proibir chamada ao toggle de minimização do Floating antes do restore.
4. Conciliar resultados de restauração e revalidar o alvo. Caso `X` ainda exista, pertença ao destino esperado e a intenção do usuário permaneça válida, solicitar foco/restauração de `X`. Em caso de falha de foco, emitir status `restored_but_focus_failed` ou equivalente comprovável.
5. Se D-43/D-44 ocorrer concomitantemente, garantir somente **uma restauração por geração**; evitar que callbacks obsoletos roubem foco após mudança de workspace ou seleção de outro aplicativo.
6. Atualizar a dock e diagnósticos a partir do **estado confirmado**, sem sinalizar foco/restauração antes da resposta do compositor.

**Exceções:** clique em favorito de aplicativo aberto apenas em outro workspace segue a política já aprovada de **solicitar nova janela aqui**, sem focalizar silenciosamente uma janela remota. Não alterar o escopo para toda a sessão. A seleção de janela via Visão Geral já tem fluxo explícito de ativação (D-39); a D-45 especifica em particular o acionamento da **dock**.

### 12.2 Máquina de estados — complemento D-43/D-44

```text
REVEALED(M,W,g)
  ├─ dock_activate(X ∈ temporarily_hidden(M,W,g))
  │     → RESTORING(M,W,g,reason=dock_activate,focus_intent=X) [D-45]
  ├─ new_eligible_toplevel(M,W) → RESTORING(g,reason=new_window) [D-44]
  ├─ workspace_exit(M,W) → RESTORING(g,reason=workspace_exit) [D-43]
  └─ hover / explore_overview → REVEALED (sem alteração)

RESTORING(g): operação idempotente compartilhada
  → restaurar conjunto originalmente visível (não janelas já minimizadas)
  → confirmar restauração / reportar pendências
  → revalidar intenção de foco X e destino
  → FOCUS_REQUEST(X) → FOCUSED(X) | RESTORED_FOCUS_FAILED

Alteração de workspace/monitor em concorrência pode invalidar a intenção de foco;
nunca reativar X se isso significar foco atrasado e inesperado.
```

### 12.3 Critérios de aceitação

- Clique em janela temporariamente oculta de A2 restaura **todas** as janelas da revelação de A2 e foca a selecionada, sem alterar B5.
- Janela previamente minimizada permanece minimizada. Em Floating, o clique **não minimiza** o alvo mesmo que seja considerado anteriormente focado; D-45 tem precedência nesse estado.
- Eventos duplicados D-43/D-44/D-45, janela-alvo fechada, hotplug e reload mantêm proveniência e impedem ações em identidades obsoletas.
- Hover e simples inspeção de cartões não restauram nem focam.
- A falha de foco é registrada separadamente da restauração e não provoca restauração inversa.

**P-17.2c:** fica **resolvida no escopo explícito de acionamento pela dock**; eventuais outros caminhos de ativação devem reutilizar o contrato quando aplicável, mediante auditoria. Esta seção prevalece sobre as menções históricas a P-17.2c como pendente nos itens anteriores. O resultado não foi implementado nem testado no repositório do usuário.
