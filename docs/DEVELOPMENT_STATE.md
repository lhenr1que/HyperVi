# Estado operacional do desenvolvimento

Atualizado em **2026-10-30 (UTC)**. Registro curto de continuidade; não é especificação técnica nem documentação de produto. Atualizar somente com autorização de escrita. Confirmar sempre o Git e as evidências reais antes de agir.

## Último marco e estado do trabalho

- **Concluído e formalmente encerrado:** investigação preparatória estática B1–B3 para Hyprland 0.57, com relatórios, evidências, revisão independente e entrega verificada.
- **Não iniciado:** portabilidade C++, implementação do protocolo de unload, build do candidato 0.57 e ensaios gráficos dessa migração. Encerrar a investigação não declara compatibilidade funcional.
- **Concluído (documental):** protocolo de encerramento e continuidade incorporado ao `AGENTS.md` com autorização do dono; este registro criado. Ambos entram no commit documental `docs: establish development handoff protocol`.
- **Concluído (planejamento externo):** Decisão 9 da minuta (unload administrativo quiescente, Q1–Q5, sequência, testes B1–B3, critérios de avanço e de bloqueio) aprovada como base técnica de planejamento. Não é implementação nem validação.

## Referência Git e OpenSpec

- Repositório: `/home/lhen/Projects/ryoku-work/kwinish`.
- Branch: `master`. Base anterior ao protocolo: `b6d02db187da7af7d2e7fa52a97fa2da9d48eea4`; o commit documental que a sucede é o HEAD de `master` (conferir com `git log -1`).
- Mudança documental: `AGENTS.md` (protocolo) e este arquivo, no commit documental.
- Worktree do port: branch `port-kwinish-bars-to-hyprland-0-57` em `/home/lhen/Projects/ryoku-work/kwinish-port-0-57`, criada a partir do commit documental, **sem alterações de produção**. A minuta previa `b6d02db`; a base passou a ser o commit documental, que difere de `b6d02db` apenas por `AGENTS.md` e este arquivo.
- Dirty preexistente, preservado e fora do commit documental: link rastreado `spike/verify/.local/share/kwinish/core`; seis arquivos `SKILL.md` sob `.agents/skills/source-command-opsx-{apply,archive,explore,propose,sync,update}/`; `Kwinish_Product_Docs_2026-10-28.zip` não rastreado. Não limpar nem incluir essas entradas em commit por acidente.
- Worktrees históricas preservadas (além da nova do port): principal; `../kwinish-apply` (`openspec-apply`, HEAD `2598f7d63b791a97973791f47a493dcc26df7910`); `.alethe/worktrees/job-03`, `job-04`, `job-05` (branches `alethe/agent-job-03`, `alethe/agent-job-04`, `alethe/agent-job-05`, HEAD `c6c511d5e996f3507ead2cb5fd3c34e440ae3b29`). Contêm trabalho histórico; não alterar.
- Port proposto: `port-kwinish-bars-to-hyprland-0-57` existe **somente como minuta externa**, em `/home/lhen/Projects/kwinish-plan-hyprland-0.57/minuta/openspec/changes/port-kwinish-bars-to-hyprland-0-57/`. Não foi incorporado ao OpenSpec vigente.
- Diretórios não arquivados já existentes no repositório: `openspec/changes/harden-user-data-io/` e `openspec/changes/add-pin-and-icon-tooltips/`. Suas tarefas têm marcações históricas; isso não comprova funcionamento no alvo 0.57. Não arquivar nem reabrir automaticamente.

## Decisão aprovada nesta sessão

Aprovado pelo dono (formalizado na Decisão 9 da minuta externa e confirmado em 30/10/2026): adotar **unload administrativo quiescente para a primeira versão compatível com Hyprland 0.57**: descarregar de forma controlada quando não houver callbacks/referências temporárias em execução, com desregistro e sincronização demonstrados.

- Unload reentrante amplo fica **fora do escopo suportado da primeira versão**, não uma capacidade implementada ou validada. Não deve bloquear indefinidamente o port nesse escopo delimitado.
- B1 administrativo continua **condicionado** à definição do protocolo e testes específicos; não está resolvido por essa decisão. B2 (estado GL/composição) e B3 (geometria/overlays) também continuam condicionados.
- Esta escolha não autoriza implementação, edição do OpenSpec, instalação ou execução gráfica. A minuta externa já a formaliza; a incorporação ao OpenSpec vigente continua dependendo de autorização específica. Nenhuma compatibilidade funcional com Hyprland 0.57 é declarada antes da validação.
- Os relatórios externos preservam o veredito histórico anterior à decisão de escopo: bloqueado para garantia ampla. Não reescrever essa evidência nem interpretá-la como validação do escopo agora escolhido.

## Verificações e problemas conhecidos

- Investigação anterior: `git fsck` retornou 0; cinco worktrees preservadas; 24 headers selecionados iguais ao upstream fixado; nenhuma carga de plugin ou teste gráfico. Provas no `VERIFICACAO-FINAL.json` abaixo, não uma nova execução nesta sessão.
- Nesta sessão documental: verificações de estrutura, referências, escopo das alterações, Git/índice e preservação; resultado em `/home/lhen/Projects/kwinish-plan-hyprland-0.57/encerramento-protocolo-verificacao.json`.
- **Não executados nesta sessão:** `tools/check.sh`, testes Lua/Rust/C++, build de plugin, validação formal OpenSpec e testes ao vivo. Não reutilizar resultados antigos como se fossem atuais.
- Riscos conhecidos: quiescência e snapshots de callbacks no unload; estado versus conteúdo do stencil e caches GL; matte R versus alpha do conteúdo; bbox lógico versus box física do preview; clipping/damage/input e overlays sob transformações. Ver pareceres, sem declarar crashes reproduzidos ou compatibilidade funcional.

## Evidências e documentos a ler

Diretório da investigação: `/home/lhen/Projects/kwinish-plan-hyprland-0.57/investigacao-b1-b3/`.

1. `RELATORIO-FINAL.md` e `REVISAO-INDEPENDENTE.md`: conclusões e retificações.
2. `B1-LIFECYCLE.md`, `B2-RENDER-OPENGL.md`, `B3-GEOMETRIA.md`: contratos e evidências por arquivo/símbolo/linha.
3. `ADENDO-OPENSPEC.md` e `TESTES-E-GATES.md`: propostas, pendências e ensaios ainda não executados.
4. `VERIFICACAO-FINAL.json`, `EVIDENCIAS.md`, `PACOTE.json` e `evidence/`: preservação e rastreabilidade.
5. `/home/lhen/Projects/kwinish-plan-hyprland-0.57/PROTOCOLO-ENCERRAMENTO-PROPOSTA.md`: seção proposta para AGENTS e mensagem reutilizável; **não aplicada**.

## Próxima ação e limites de autorização

**Recomendada (precisa de autorização):** tarefas 1.2 a 1.4 da minuta na worktree do port: ledger de recursos, condições Q1–Q5 e estratégia de estado GL/geometria, sem editar código de produção. Depois, revisão antes de qualquer sonda experimental.

**Fora de autorização até nova decisão do dono:** código C++/Lua/Rust, build do candidato, carga de plugin, testes gráficos, edição do OpenSpec vigente, mexer nas worktrees históricas, push.

Nova sessão começa **somente leitura**: ler `AGENTS.md`, `docs/README.md`, este registro e a Decisão 9 da minuta; confirmar branch/HEAD/status/worktrees; apresentar a próxima tarefa, critérios e riscos e pedir autorização. Este registro não é autorização permanente.
