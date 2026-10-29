# Tasks — Plano de implementação condicionado à auditoria

> **Nenhuma tarefa está concluída.** Execute somente depois de reconciliar o change com o repositório real e obter validação do plano. Não marcar como feito por documentação existente.

## 1. Inspeção / OpenSpec

- [ ] 1.1 Ler `AGENTS.md`, `openspec/config.yaml` e spec/changes relacionados no repositório local.
- [ ] 1.2 Mapear a capability existente de Mostrar área de trabalho; corrigir o caminho provisório `show-desktop` no proposal e specs.
- [ ] 1.3 Verificar o estado atual de `core/kwinish.lua` ou arquivo sucessor, callbacks, testes, persistência e versão Hyprland.
- [ ] 1.4 Reproduzir o bug de reload da auditoria anterior ou documentar por que já está corrigido.
- [ ] 1.5 Validar specs e registrar escolhas técnicas de persistência e identidade de janelas.

## 2. Recuperação e proveniência

- [ ] 2.1 Extrair/reusar uma fonte única de proveniência para janelas minimizadas e temporariamente ocultas.
- [ ] 2.2 Garantir rastreabilidade da operação de revelação por escopo e geração.
- [ ] 2.3 Implementar recuperação segura após reload e falha parcial, sem I/O bloqueante indevido no compositor.
- [ ] 2.4 Proteger contra janelas fechadas, IDs reutilizados e operações de sessão antiga.

## 3. D-43 — Saída de workspace

- [ ] 3.1 Ligar detecção confiável da troca de workspace do **monitor de origem** ao encerramento da revelação.
- [ ] 3.2 Garantir que mudar só o monitor em foco não restaura o workspace antigo.
- [ ] 3.3 Implementar restauração idempotente, sem alterar minimização, modos ou geometria anteriores.
- [ ] 3.4 Confirmar resultado efetivo e publicar estado de erro/parcial para UI/diagnóstico.

## 4. D-44 — Criação efetiva de janela no escopo revelado

- [ ] 4.1 Identificar eventos reais de mapeamento/criação de janelas normais e confirmar escopo (monitor, workspace, sessão, elegibilidade).
- [ ] 4.2 Conectar criação de nova janela elegível à mesma operação idempotente de restauração da D-43; não executar pela intenção de lançamento.
- [ ] 4.3 Restaurar somente as janelas previamente visíveis e permitir que a nova janela participe do modo atual de workspace.
- [ ] 4.4 Proteger contra notificações, outras telas, janelas fechadas durante o evento e eventos concorrentes D-43/D-44.
- [ ] 4.5 Verificar reutilização de janela existente sem criação: encaminhar acionamento explícito pela dock à regra D-45; não aplicar D-44 por engano.

## 5. D-45 — Acionamento explícito na dock

- [ ] 5.1 Mapear a identidade de cliente utilizada pela dock e o método atual de ativação/foco/minimização (incluindo Floating).
- [ ] 5.2 Identificar cliente elegível previamente ocultado pela revelação do workspace atual; diferenciar de favoritos abertos só em outros workspaces.
- [ ] 5.3 Convergir ao mesmo restore idempotente D-43/D-44 e então focar o cliente escolhido, sem passar pelo toggle minimizar Floating.
- [ ] 5.4 Separar confirmação de restore e confirmação de foco; proteger contra destino obsoleto, falha parcial e concorrência.
- [ ] 5.5 Não disparar a regra por hover, prévia ou exploração de cartões.

## 6. Testes

- [ ] 6.1 Criar testes `SD-D43-01` a `SD-D43-04` (básicos, multimonitor e minimização).
- [ ] 6.2 Criar testes `SD-D43-05` a `SD-D43-10` (modos, foco, fechamento, overview).
- [ ] 6.3 Criar testes `SD-D43-11` a `SD-D43-17` (reload, crash, hotplug, corridas).
- [ ] 6.4 Criar testes `SD-D43-18` a `SD-D43-20` (regressão Floating, fullscreen, dock).
- [ ] 6.5 Acrescentar testes `SD-D44-01` a `SD-D44-12` (mapeamento, outros monitores, falha de spawn, corrida D-43/D-44, Tiling, Scroll e reload).
- [ ] 6.6 Atualizar simulador para emitir eventos adequados nas mudanças de workspace/foco.
- [ ] 6.7 Rodar testes Lua/Rust e validar plugin C++/ABI se houver código alterado; testar no Hyprland real.

- [ ] 6.8 Acrescentar e executar testes `SD-D45-01` a `SD-D45-14`, inclusive precedência sobre minimização Floating, falhas de foco e concorrência.

## 7. Conclusão

- [ ] 7.1 Revisar diffs com foco em privacidade, desempenho do compositor e rollback.
- [ ] 7.2 Executar `openspec validate stabilize-show-desktop-restoration` se suportado pela versão local; investigar schema e ajustes necessários.
- [ ] 7.3 Não arquivar nem declarar concluído enquanto persistir janela irrecuperável após reload ou regressão de modo/minimização.
- [ ] 7.4 Registrar quais subcasos de P-17 permanecem fora da mudança (P-17.2c resolvida para clique explícito na dock) e quais exigirão nova decisão de produto.
