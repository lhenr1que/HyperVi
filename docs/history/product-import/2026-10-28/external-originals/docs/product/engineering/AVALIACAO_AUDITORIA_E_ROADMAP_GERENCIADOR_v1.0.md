# Avaliação da auditoria e plano de estabilização — gerenciador Hyprland

**Data de referência:** 28/10/2026  
**Fontes:** relatório de auditoria do Codex fornecido pelo usuário (`Texto colado.txt`), decisões de produto D-01 a D-45 e contrato funcional de Visão Geral/Foco/Restauração v1.4.  
**Status:** avaliação de planejamento. Nenhum código do repositório local foi acessado ou modificado nesta etapa.

> **Nota sobre o nome:** “Kwinish” é o identificador usado pelo repositório auditado; o nome público do projeto e da suíte ainda não foi decidido.

## 1. Parecer executivo

**Recomendação: preservar a arquitetura atual e estabilizar o núcleo antes de integrar o shell oficial.** O projeto já tem uma base relevante (Lua para políticas, plugin C++ para decorações/entrada, Rust/GTK4 para interface), mas ainda não demonstra maturidade suficiente de ciclo de vida e integração para tornar-se imediatamente a fundação de uma suíte distribuível.

O gerenciador é **um complemento independente para Hyprland**, não um compositor nem um motor de tiling autônomo. Utiliza `dwindle` e `scrolling` nativos e políticas próprias para Floating. Isso é compatível com a intenção de desenvolver um programa separado que dependa do Hyprland; não requer reescrita de layouts.

A auditoria informa: 173 commits; branch `master` em `a2506c6`; sem remoto ou tags; quatro worktrees secundárias com alterações locais; `tools/check.sh` aprovado (11 suítes Lua, 4 testes tools, 31 testes Rust e compilação do plugin) e `openspec validate --all --strict` 6/6. **Não houve validação funcional ao vivo nesta auditoria.** Não confundir compilação e testes simulados com funcionalidade validada na sessão Hyprland.

## 2. Achados priorizados

| Prioridade | Referência | Situação | Impacto sobre o produto |
|---|---|---|---|
| P0 | A2 | Perda da etiqueta `kwinish-mode-floated` ao mover janela para workspace especial durante minimização/Mostrar área de trabalho; confirmada pela análise e teste isolado reportado | Corrompe a volta ao Tiling e ameaça decisões D-36, D-43–D-45 |
| P0 | A3 | Origens de `show_desktop` apenas em tabela Lua; podem desaparecer em reload, deixando janelas em `special:kwinish_desktop` | Risco de janelas inacessíveis; cenário integral ainda requer validação ao vivo |
| P0 | M1 | Recarregamento por `eval` pode duplicar hooks/timers e manter atalhos apontando a código antigo | Invalida resultados de teste, cria efeitos repetidos |
| P1 | A1 | Cascata em Floating falha ao vivo, embora o simulador passe e o OpenSpec a exija | Divergência código/spec; qualidade do modo Floating |
| P1 | A4 | Plugin sensível a ABI interna do Hyprland, sem handshake de versão entre as peças | Risco de incompatibilidade em atualizações |
| P1 | M2, M3, M5 | Temporizadores sem revalidação; ação zone:N com menos checagens; ações na barra podem atingir janela ativa em vez da janela proprietária | Comandos obsoletos, janela incorreta e problemas de foco |
| P1 | M8 | Alt+Tab pode tentar focar janelas ocultas por Mostrar área de trabalho sem restaurá-las adequadamente | Mesmo problema de ativação que afetará dock/overview |
| P2 | M7 | Empacotamento, script de reconstrução, ausência de CI | Entrega pouco confiável |
| P2 | Documentação | OpenSpec cobre quatro capacidades; documentação diverge de atalhos, versões e status | Agentes podem planejar sobre premissas falsas |

**Cuidado com evidência:** os rótulos A/P/H pertencem à auditoria. Confirmar uma falha em código não equivale a reproduzir integralmente o sintoma na sessão real. A auditoria não testou uso ao vivo.

## 3. Confronto com decisões de produto

| Decisões | Depende de | Situação hoje |
|---|---|---|
| D-36, D-42 — Mostrar área de trabalho local | Ciclo de vida de janelas e escopo por workspace/monitor | Há implementação, mas A2/A3 tornam a base insegura para expansão |
| D-43 — restaurar ao sair do workspace | Rastreamento confiável de origem e eventos de troca | A desenvolver após corrigir A2/A3/M1 |
| D-44 — restaurar ao mapear nova janela | Eventos de criação, elegibilidade e revalidação de timers | A desenvolver; evitar regras baseadas apenas em intenção de lançamento |
| D-45 — dock ativa janela ocultada | API de restauração/foco e semântica de clique separada do toggle de minimizar | Contrato de comportamento aprovado; depende da implementação futura da dock |
| D-37–D-41 — Visão Geral | Estado consultável de janelas/workspaces, navegação e foco real | Conceito e requisitos definidos; falta implementação específica |
| D-13–D-18 — dock oficial | Estado de janela, minimização estável e API compartilhada | Requer contrato de estados antes do shell |
| D-19–D-35 — launcher/Settings/Design System | APIs estáveis e integração com rice | Manter como roadmap de produto; não introduzir tudo no OpenSpec atual |

## 4. Plano proposto em gates

### Gate 0 — Segurança e baseline (antes de editar)

1. Fazer backup verificável do repositório e, idealmente, criar remoto **privado**. Não publicar, apagar nem limpar worktrees automaticamente.
2. Inventariar e preservar diffs das worktrees `kwinish-apply` e `.alethe/worktrees/job-03/04/05`; decidir seu destino somente após inspeção.
3. Executar e registrar os testes manuais pendentes 10, 11 e 12 no ambiente apropriado, anotando versão do Hyprland, commit e resultado por subcaso.
4. Congelar um relatório de baseline (commit, pacotes instalados, specs ativas, `check.sh`, testes ao vivo). Só arquivar mudanças OpenSpec depois da evidência exigida.
5. Corrigir desatualizações factuais mínimas em README, STATUS, TESTING e PLUGIN, em mudança documental separada.

**Critério de passagem:** status, trabalho pendente e falhas conhecidas são reproduzíveis e recuperáveis; worktrees estão preservadas; nenhuma informação foi descartada.

### Gate 1 — Integridade do ciclo de vida das janelas (prioridade máxima)

Implementar mudanças **pequenas, testáveis e independentes** (ordenar definitivamente após inspecionar acoplamentos):

- **1A. Reload idempotente (M1):** hooks/timers não se duplicam; atalhos invocam o módulo atual; testes tornam-se confiáveis.
- **1B. Proveniência Floating/minimização (A2):** stash/minimize não apagam a marca de modo nem causam restauração para Tiling incorreta; testar alternâncias.
- **1C. Persistência/recuperação de Mostrar área de trabalho (A3):** preservar origens e reconciliar janelas após reload, sem deixá-las inacessíveis; recuperação idempotente e segura.
- **1D. Cascata Floating (A1):** reproduzir no Hyprland aninhado/real e corrigir a divergência ou, se necessário, rever a especificação com decisão explícita do dono.

**Critério de passagem:** cada falha tem reprodução anterior, teste que falha primeiro quando viável, correção, teste automatizado e teste ao vivo indicado; `check.sh` e OpenSpec validados.

### Gate 2 — Foco, segurança de comandos e compatibilidade

- Revalidar destino de ações e timers antes de aplicar mudanças (M2, M3, M5).
- Resolver foco de janelas temporariamente ocultas via Alt+Tab e demais canais (M8).
- Inventariar dependências internas do Hyprland e introduzir protocolo/handshake de versão Lua–plugin–UI (A4).
- Adicionar timeout ao canal síncrono Rust e revisar operações potencialmente bloqueantes na UI/compositor.

### Gate 3 — Expansão controlada de OpenSpec

- Atualizar `openspec/specs/` para representar o **comportamento real e aprovado**, documentando as lacunas e subcasos não verificados.
- Adicionar cobertura para minimizar/restaurar, show-desktop, Scroll/colunas, API `window_action` e Alt+Tab, após conciliar duplicações e changes ativas.
- Integrar D-43 e D-44 ao mesmo núcleo de restauração; D-45 deve ser especificada na fronteira de ação da dock sem antecipar o shell completo.
- Separar documentos de visão futura (suíte, rices, aparência, Rice Store) das specs normativas implementáveis no ciclo atual.

### Gate 4 — APIs para o shell oficial

- Contrato de consulta de estado e comando, idealmente versionado, com tipos para workspace, modo, foco, janela e minimização.
- Publicação de estado em runtime orientada a eventos, sem depender de polling indiscriminado.
- Testes de compatibilidade e integração antes de criar dock, launcher e visão geral reais.

### Gate 5 — Entrega e suíte futura

- CI, empacotamento confiável e matriz de compatibilidade.
- Implementar progressivamente shell oficial, dock, launcher, Settings e Visão Geral.
- Rice Store e metapacotes da suíte apenas depois da estabilidade das APIs e do shell.

## 5. Decisões de gestão do desenvolvimento

- **Não reescrever o núcleo** só porque o design da suíte evoluiu.
- **Não enviar toda a especificação de 45 decisões ao agente como tarefa de implementação única.** Usá-la como visão de produto, selecionando mudanças pequenas para OpenSpec.
- **Não fazer `git worktree prune`, `git clean`, reset, rebase ou descarte de worktrees** sem revisar e aprovar o destino dos dados.
- **Não publicar o código automaticamente**: um remoto privado é opção preferencial até decidir licença, nome e política de distribuição.
- **Não afirmar implementação/validação ao vivo** a partir do relatório de auditoria.

## 6. Próxima solicitação ao Codex

O primeiro pedido após esta avaliação deve ser um **plano de execução do Gate 0 com investigação não destrutiva**, incluindo inventário de worktrees, lista exata dos testes manuais pendentes e critérios de fechamento das changes ativas. Depois, um plano OpenSpec de correção separado para A2/A3/M1 e A1. Veja `PROMPT_CODEX_POS_AUDITORIA_v1.0.md`.

---

**Estado do documento:** proposta de priorização para revisão do dono. Nenhum gate foi executado nem validado por este documento.
