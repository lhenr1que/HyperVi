# Estado operacional do desenvolvimento

Atualizado em **2026-10-30 (UTC)**. Registro curto de continuidade; não é especificação técnica nem documentação de produto. Atualizar somente com autorização de escrita. Confirmar sempre o Git e as evidências reais antes de agir.

## Resultado mais recente — P3 (30/10/2026): Mudança 1 aprovada e fechada

Dono aprovou a Mudança 1 (contrato geométrico independente do Hyprland), H-DOM provisória e `std::optional` nas quatro funções; limites são **contrato interno**, não propriedade do Hyprland. `tools/check.sh`: tudo verde exceto `make -C plugin`, idêntico à falha conhecida de P2 (Hyprland 0.57.0). Matriz de `nullopt`: `docs/port-0-57/NULLOPT-MATRIX.md` (proposta). Commit da Mudança 1: ver `git log` (hash no relatório da sessão). Integração às decorações e portabilidade 0.57 **não iniciadas**; exigem nova autorização.

## Anterior — P2.1 (30/10/2026): bloqueio P3 resolvido

**Concluído (simulado, host-free; sem integração).** Somente `plugin/geometry.hpp`, `plugin/test_geometry.cpp`, `tools/test-plugin-geometry.sh` (não rastreados) e docs em `docs/port-0-57/` + este arquivo. Git: branch `port-kwinish-bars-to-hyprland-0-57`, HEAD `9850593`, sem commit/merge/staging; dirty = este arquivo (M) e os 4 itens não rastreados; nenhum fonte preexistente alterado; nenhuma worktree histórica tocada.

- **Contrato escolhido** (detalhe e fundamento em `GEOMETRY.md` §4): domínio explícito (`kMaxLogical=2^31`, escala `[2^-4, 2^4]`, `kMaxPhysical=2^35`, `kMaxBox=2^40`), guarda `resolved` (`x+w > x`) e invariante de saída; fora do domínio, `std::optional` vazio. O contraexemplo `0x1p53` permanece como teste e é **rejeitado** por todas as quatro funções. Perda de cobertura medida só a partir de 2^50 px; a maior borda aceita é 2^45.
- **Vermelhos:** `P31-red-1.log` (não compila), `P31-red-2.log` (contraexemplo aceito, exit 1). **Verde:** GCC e Clang exit 0, 320 762 verificações, 13 negativos + 46 mutações reais do header, cada uma rejeitada pela assertiva **esperada** (`P31-green-*.log`); UBSan e `-O0/-O3` limpos; fuzz independente `long double` 3 M amostras sem violação.
- **Revisão independente:** `gpt-6.1-sol` via Hermes/`openai-codex` (família diferente; sem Claude/Codex/Gemini CLI no PATH; nada instalado). Rodadas 1 e 2 reprovaram (extensão absorvida → largura −1; união/preview geravam caixa inválida como entrada); corrigidas e reproduzidas; rodada 3 **aprovou** (sem blocker/major; 1 minor de comentário, corrigido, tudo reexecutado). Revisão estática: não executou código. Registro: `RISKS-TESTS.md` §7.
- **Lacuna real achada:** dois alargamentos `nextafter` de `previewBBox` sobreviviam à suíte de P2; agora testados.
- **Recomendação:** Mudança 1 recomendável para aprovação **se o dono aceitar o domínio (H-DOM)** e a API `std::optional`. Não executados: `tools/check.sh`, build do plugin, qualquer teste ao vivo/gráfico. B1/B2/B3, G2/G4/G5, E0/E1, Q1–Q3 seguem abertos.
- **Próxima ação (nova autorização):** o dono decide H-DOM e a aprovação da Mudança 1; só depois, integração do módulo (tarefas 4.2/4.4) ou headers 0.57 (3.1). A autorização desta sessão não se estende à próxima.

## Resultado anterior — P2 (30/10/2026; histórico, superado em P3 por P2.1)

**Parcial e não aprovado.** Build baseline isolado reproduzido; candidato geométrico criado exclusivamente em `plugin/geometry.hpp`, `plugin/test_geometry.cpp`, `tools/test-plugin-geometry.sh`, sem integração. Revisões independentes obtidas em sessões novas de `gpt-6.1-sol`, provedor `openai-codex`, via Hermes, sem ferramentas de escrita. Claude/Codex/Gemini CLI não encontrados; nenhum instalado. A revisão externa de contratos foi obtida **depois da primeira implementação**, usando somente os snapshots P1 anteriores e trechos de fontes, sem mostrar o código candidato. Portanto a sequência original da Etapa 1 não foi cumprida integralmente; não houve edição de código enquanto cada revisão estava em execução.

- Git desta worktree: `/home/lhen/Projects/ryoku-work/kwinish-port-0-57`, branch `port-kwinish-bars-to-hyprland-0-57`, HEAD `9850593b8ea87b231f64b0abe9ec634a33cc4ab7`, sem commit/merge/staging. Dirty inicial: este arquivo modificado e `docs/port-0-57/` não rastreado; preservados e atualizados somente para resultados/retificações autorizados. Nenhum fonte preexistente alterado.
- Build: `make -C /home/lhen/Projects/ryoku-work/kwinish-port-0-57/plugin TARGET=/home/lhen/.hermes/cache/scratch/kwinish-p2/build/kwinish-bars-baseline.so` → **2**. HOME/XDG isolados, sem DISPLAY/WAYLAND_DISPLAY/assinatura. Falhas: `desktop/view/Window.hpp` ausente; overrides antigos dos passes; `m_renderData` removido; `renderRect` exige contexto. Nenhum `.so` gerado/sobrescrito.
- Vermelho inicial, `bash tools/test-plugin-geometry.sh` → **1**: `fatal error: geometry.hpp: No such file or directory`, `FALHOU - compilar plugin/test_geometry.cpp`. P1 implementado → **0**, `OK - P1 numeric oracle`; P2 e P4/P5 tiveram vermelhos de API ausente e verdes próprios.
- Verde intermediário após duas correções da revisão, GCC e Clang → **0**, saída literal: `OK - P1-P5: 3035 verificacoes (variante 0)`. 13 variantes negativas saíram **1 por assertiva**, runner **0**. 13 mutações reais do header em scratch compilaram (**0**) e foram rejeitadas pela suíte positiva (**1**); não contar erro de compilação/crash como detecção.
- Correções reproduzidas vermelho→verde: P5 perdia borda fracionária no roundtrip físico/lógico; P4 perdia borda na união por cancelamento. Ajustes conservadores condicionais com `nextafter`; nenhuma tolerância foi adicionada à assertiva de cobertura.
- **Último parecer independente: reprovado.** P3 falha com `D={0x1p53,0,1,1}`, demais termos zero, escala `1.5`, extensão/AA zero: a borda lógica perde a dimensão antes de escalar; bbox físico largura **0**, desenho largura **2**. Regressão incorporada ao teste. **Estado final GCC e Clang: exit 1**, saída literal: `FALHOU - P3 large finite origin loses logical extent (OPEN review blocker) (variante 0)`. O verde anterior é histórico, não o estado atual.
- Parada após dois ciclos de correção/revisão; não houve terceira tentativa nem restrição silenciosa do domínio numérico. Mudança 1 **não concluída/aceita**. B1/B2/B3 continuam abertos. Sem carga de plugin, gráficos, instalação, configuração global, edição do OpenSpec ou escrita em outras worktrees. `tools/check.sh` não executado.
- Evidências permanentes: `docs/port-0-57/RISKS-TESTS.md` §6 e `docs/port-0-57/evidence/P2-*`; diff entregue separadamente. Documentos P1 receberam retificações de Q1, H-REG, stencil/D10 e definição aritmética.

**Próxima ação recomendada (nova autorização):** resolver primeiro o bloqueio P3, escolhendo explicitamente entre representação/cálculo conservador dos limites e domínio numérico limitado com justificativa. Manter o teste que falha, obter verde e nova revisão independente antes de integrar ou iniciar headers 0.57. G2/G4/G5, E0/E1, Q1/Q2/Q3 e ensaios gráficos continuam pendentes. A autorização desta sessão não se estende à próxima.

## Histórico anterior ao P2 (preservado; próximos passos abaixo foram substituídos pelo resultado acima)

## Último marco e estado do trabalho

- **Concluído e formalmente encerrado:** investigação preparatória estática B1–B3 para Hyprland 0.57, com relatórios, evidências, revisão independente e entrega verificada.
- **Não iniciado:** portabilidade C++, implementação do protocolo de unload, build do candidato 0.57 e ensaios gráficos dessa migração. Encerrar a investigação não declara compatibilidade funcional.
- **Concluído (documental):** protocolo de encerramento e continuidade incorporado ao `AGENTS.md` com autorização do dono; este registro criado. Ambos entram no commit documental `docs: establish development handoff protocol`.
- **Concluído (planejamento externo):** Decisão 9 da minuta (unload administrativo quiescente, Q1–Q5, sequência, testes B1–B3, critérios de avanço e de bloqueio) aprovada como base técnica de planejamento. Não é implementação nem validação.
- **Concluído (preparatório, estático; não commitado):** tarefas 1.2–1.4 da minuta em `docs/port-0-57/` (ledger de recursos, Q1–Q5 verificáveis, estratégia de estado GL, fluxo de geometria, matriz B1–B3 × testes, proposta da primeira alteração C++). Análise estática de fontes e headers: nenhum build, teste, carga de plugin ou execução gráfica. B1, B2 e B3 continuam **condicionados**; as hipóteses não demonstradas estão em `docs/port-0-57/README.md` §3. A minuta externa teve a referência de base corrigida de `b6d02db` para `9850593` (`/home/lhen/Projects/kwinish-plan-hyprland-0.57/CORRECAO-COMMIT-BASE.diff`).

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

**Recomendada (precisa de autorização):** (a) revisão independente de `docs/port-0-57/`; (b) tarefa 2.2 da minuta: reproduzir o vermelho do plugin em build isolado (`TARGET` temporário, sem alterar código); (c) **Mudança 1**, o contrato geométrico host-free descrito em `docs/port-0-57/RISKS-TESTS.md` §4 (3 arquivos novos, nenhum existente alterado, teste vermelho→verde). Decisões do dono pendentes: G2, G4, G5 (`GEOMETRY.md`), exposição dos contadores Q1/Q2. Nenhuma sonda experimental ou ensaio aninhado antes da revisão.

**Fora de autorização até nova decisão do dono:** código C++/Lua/Rust, build do candidato, carga de plugin, testes gráficos, edição do OpenSpec vigente, mexer nas worktrees históricas, push.

Nova sessão começa **somente leitura**: ler `AGENTS.md`, `docs/README.md`, este registro e a Decisão 9 da minuta; confirmar branch/HEAD/status/worktrees; apresentar a próxima tarefa, critérios e riscos e pedir autorização. Este registro não é autorização permanente.
