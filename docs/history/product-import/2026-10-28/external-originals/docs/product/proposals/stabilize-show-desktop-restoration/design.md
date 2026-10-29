# Design — Robustez da restauração de Mostrar área de trabalho

## Context

Na auditoria anterior do código local (commit histórico `60e19df`), o núcleo Lua já disponibilizava Mostrar área de trabalho, porém podia perder a origem das janelas transferidas para um workspace especial após reload. A procedência Floating também tinha falha de minimização/restauração. A implementação real e a versão local do Hyprland precisam ser reinspecionadas antes de aprovar solução concreta.

## Goals / Non-Goals

**Goals:** cumprir D-43, D-44 e D-45, tornar restauração idempotente, recuperar proveniência após reload, preservar estado de minimização/modos/geometria e evitar manipular o monitor errado.

**Non-Goals:** reescrever núcleo em Rust; criar nova solução paralela de minimização; construir Visão Geral gráfica; definir outras vias de ativação além da dock e política de pinned/fullscreen sem pesquisa; adicionar Rice Store/VXWM/dashboard.

## Decisions / propostas para validação após auditoria

1. **Uma única autoridade de estado de janela.** Reutilizar o núcleo Lua para registro do estado de revelação, com a mesma proveniência que informa minimização e modo Floating. Evitar que UI e plugin mantenham cópias conflitantes.
2. **Escopo e geração.** Endereçar operações por `monitor_id`, `workspace_id`, `session_id`, `reveal_generation`, com revalidação da identidade da janela. Um endereço Hyprland por si só não identifica uma janela de forma duradoura.
3. **Estado distinto de minimização.** Registrar quais janelas estavam visíveis e quais foram ocultadas pela operação; a janela previamente minimizada nunca entra no conjunto a restaurar.
4. **Saída por workspace efetivo.** Detectar evento de mudança de workspace do monitor originário e confirmar o estado por consulta; não confundir `focusedmon` com mudança do workspace ativo. Considerar eventos perdidos, duplicados, delayed callbacks e mudanças via dispatch.
5. **Nova janela no escopo revelado (D-44).** Escutar eventos de mapeamento/criação de toplevel de aplicativo elegível, confirmando que o cliente realmente pertence ao workspace revelado. Solicitação de lançamento, notificação, janelas fora do escopo ou simples reutilização de instância anterior não bastam. Eventos D-43 e D-44 devem convergir na mesma restauração por geração.
6. **Restaurar como operação idempotente.** Restaurar apenas entradas confirmadas como ainda pertencentes à operação; registrar sucesso individual, conservar pendências até reconciliação; não alternar toggle em resposta duplicada.
7. **Seleção pela dock (D-45).** Identificar a intenção explícita sobre uma janela do conjunto temporariamente oculto; não executar o toggle Floating de minimizar antes de restaurar. Reusar `request_restore` por geração; após convergência, validar destino e aplicar foco, registrando separadamente falha de foco. Concorrência D-43/D-44/D-45 não deve provocar foco tardio em outro workspace.
8. **Proveniência e reload.** Investigar journal validado/atômico **ou** reconstrução confiável por metadados do compositor; não assumir que um arquivo basta. Persistir/reconstituir antes de permitir ocultação irrecuperável. Não introduzir I/O bloqueante em callback de compositor sem avaliação.
9. **Foco.** Inspeção da Visão Geral é seleção lógica. Em troca de workspace, o sistema não deve reativar janelas do workspace anterior nem transferir foco a outro monitor durante restore.
10. **Falhas.** Hotplug, janelas fechadas, reuso de identidade, sessão nova, reload, e concorrência com troca de modo devem provocar reconciliação segura. Expor diagnóstico de restauração pendente.

## Event contract — pseudofluxo

```text
on_workspaces_reconciled(snapshot):
  for each active_reveal in reveals:
    actual = snapshot.active_workspace_for(reveal.monitor_id)
    if actual != reveal.workspace_id:
      request_restore(reveal.generation, reason="workspace_exit")

on_window_mapped(candidate, snapshot):
  if new_app_toplevel_is_eligible(candidate) and
     candidate.workspace == revealed_workspace_in_same_monitor(snapshot):
    request_restore(reveal.generation, reason="new_window")  # D-44

on_dock_window_activate(target, snapshot):
  if target in temporarily_hidden_clients_for_current_reveal(snapshot):
    request_restore(generation, reason="dock_activate", focus_intent=target)  # D-45
  else:
    dispatch_existing_dock_behavior(target)

request_restore(generation, reason, focus_intent=None):
  if generation already completed: return no_op
  validate session, monitor, workspace, clients and provenance
  restore only eligible / still hidden clients, one by one
  record confirmed successes; retain failures as pending
  reconcile and publish result: applied | partial | failed
  if focus_intent and intent_still_valid_after_restore:
    request_focus(focus_intent); confirm_focus_or_report_separately
```

**Importante:** pseudofluxo ilustrativo, sem garantia de que os métodos ou IPC nomeados já existam. O IPC real, métodos Lua e política de journal são decisões técnicas a tomar após abrir o repositório.

## Risks / Trade-offs

- Eventos podem notificar depois da troca de workspace; promessa de ordem estrita de frames seria incorreta.
- Alguns mecanismos de ocultação usam workspace especial e podem conflitar com minimizar; não copiar o padrão sem verificar proveniência atual.
- Metadados persistidos com IDs obsoletos podem manipular janela nova; persistência deve ser limitada à sessão e revalidada.
- Recomposição de layout nos modos Scroll/Tiling exige regressão no backend e observação em compositor real.
- Hotplug pode tornar o workspace original inválido; recuperar acesso vale mais que preservar coordenadas impossíveis.

## Migration / audit gate

1. Abrir `AGENTS.md`, OpenSpec atual, `core/kwinish.lua` (se ainda existir), regras Lua, plugin e UI.
2. Confirmar se `show-desktop` já possui spec com outro caminho; atualizar a delta sem sobrescrever requisitos existentes.
3. Reproduzir bug após reload e medir sequência de eventos reais versus simulados.
4. Propor menor alteração compatível com API, sem violar estabilidade do compositor.
5. Implementar e testar primeiro recuperação, depois D-43, D-44 e D-45 na mesma máquina de estados, e por fim casos de falha e regressão.
