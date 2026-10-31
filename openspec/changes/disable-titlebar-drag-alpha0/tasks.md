# Tasks

Todas as tarefas abaixo estão **não iniciadas**. São o plano T4, dependente de revisão independente da proposta e aprovação/autorização própria do dono. T3 não executa implementação, testes de software ou build. Critérios e matriz detalhados em design.md, Validation Plan.

## 1. Congelar o aceite e demonstrar vermelho (host-free)

- [ ] 1.1 Confirmar base Git e escopo T4 autorizado, incluindo `barDeco.hpp`, `globals.hpp`, UI, testes e documentação; preservar entradas preexistentes. Conferir a revisão de outra família e a aprovação do dono; não usar autorrevisão como independente.
- [ ] 1.2 Criar harness host-free ligado aos corpos reais de input do plugin e `luaGrab`, sem compilar/carregar o plugin inteiro: spies de begin/end/dispatcher, flags, foco, geometria, cancelamento, preview e resistência. Documentar a ligação com produção e evitar modelo desconectado.
- [ ] 1.3 Demonstrar falha por assertiva no baseline para K1/K2, K3/K9, rearm, unsnap, snap-on-release, toque e cancelamentos do grab; cobrir flutuante/tiled/maximizada/encaixada, Shift, fim com foco mudado e captura simulada. Registrar falhas esperadas, não contar falha de compilação como vermelho comportamental.
- [ ] 1.4 Adicionar matriz de pares de mouse: botões, ícone, menu direito, release fora, minimizar com troca de foco, dois dispositivos/botões, barras sobrepostas e clique após grab. Demonstrar vermelho nas inconsistências atuais e exigir pares não consumidos passarem completos.
- [ ] 1.5 Criar teste puro da árvore do menu Rust, sem GTK/display, que falha com os itens `grab:move`/`grab:resize` atuais e preserva as outras ações/ordem, incluindo atalho por app e modos.
- [ ] 1.6 Testar a entrada Lua real com modo/endereço válidos e argumentos inválidos: válido informa indisponibilidade sem foco/drag/swallow; preservar diagnóstico de inválidos. Demonstrar vermelho no comportamento atual.

## 2. Aplicar D1 completo (somente após os vermelhos)

- [ ] 2.1 Excluir em compilação todo o inventário fechado de design.md: K1/K2, K3/K9, rearm e suas chamadas de reinício, unsnap/saída de maximizada por gesto, snap-on-release, ativação de preview/resistência e toda interação por toque da decoração. Não alterar `hostDrag.hpp`, grupo R ou `PLUGIN_EXIT`.
- [ ] 2.2 Excluir armação/consumo das flags de drag/grab e estado auxiliar, ajustando declarações estritamente relacionadas em `barDeco.hpp`/`globals.hpp`. Não instalar hook de resistência na alpha-0; não remover passes/técnica GL por estarem inalcançáveis.
- [ ] 2.3 Retirar cancelamentos exclusivos de drag/grab/toque; manter ou substituir rastreamento de clique por origem/botão para garantir pares de botões/ícone/menu. Preservar hover/tooltip, roda, foco/elevação, duplo clique e a parte geral de tooltip no listener PG.
- [ ] 2.4 Manter nome/argumentos/registro de `luaGrab`, diagnóstico de inválidos e diagnóstico explícito de indisponibilidade para válidos; não focar nem adquirir operação.
- [ ] 2.5 Ocultar Mover/Redimensionar em `ui/src/menu.rs`, ajustar a inserção do atalho por app sem quebrar ordem/ações. Não alterar o fallback `ui/src/bars.rs` ou criar protocolo runtime de capacidades.

## 3. Demonstrar verde e preservar comportamento (host-free)

- [ ] 3.1 Executar matriz de design.md com zero begin/end/dispatcher mouse, zero flags de aquisição/rearm e zero efeitos de gesto excluídos. Complementar com inventário estrutural das rotas compiladas para detectar chamada esquecida.
- [ ] 3.2 Demonstrar pareamento para todos os casos da matriz, ação única, passagem dos pares alheios e ausência de swallow pós-grab. Comparar fluxo de bind nativo simulado com host sem rotas de drag Kwinish; distinguir floating preservado dessa garantia.
- [ ] 3.3 Executar testes puros do menu/entrada Lua e regressões host-free de geometria, tooltip, ações e snap/Assist via menu/teclado (suítes Lua com host simulado, Rust sem display). Registrar comandos e resultados reais por compilador/commit; não chamar isso teste ao vivo.
- [ ] 3.4 Demonstrar mutações: reintroduzir cada classe de início/fim/flag/rearm/toque/cancelamento exclusivo; quebrar release e identidade do par; reinserir item do menu; retornar silenciosamente de grab; apagar ação preservada. Mutante deve compilar antes de morrer por assertiva do oráculo declarado.
- [ ] 3.5 Atualizar oráculos ativos incompatíveis com K1/K2 ligados sem apagar evidências históricas P8-B/Q4. Não exigir equivalência com comportamento deliberadamente retirado, nem usar matriz histórica como prova da implementação nova.

## 4. Documentar e entregar T4 sem ampliar autorização

- [ ] 4.1 Com autorização documental T4, registrar em `docs/API.md` a indisponibilidade temporária de `grab` sem mudança de assinatura, e em `docs/CONFIGURAVEL.md` o menu reduzido e opções de drag preservadas com efeitos inativos. Não reformatar contratos ou apagar dados/opções.
- [ ] 4.2 Registrar resultado e pendências em `docs/DEVELOPMENT_STATE.md`; explicitar débito de `floatWindow` Lua/C++ preservado, grupo R ainda separado e ausência de garantia O1 global.
- [ ] 4.3 Entregar diff/evidências à revisão independente e ao dono, sem commit automático. Build integrado/check completo pertencem ao marco T7; roteiro ao vivo completo em fish, com plugin e núcleo, backup/restauração quando necessário, pertence a T8 com autorização para `docs/STATUS.md`. Não carregar candidato nem declarar alpha integrada validada em T4.
- [ ] 4.4 Manter alpha-1 pendente de Q4/T10 e T9 com regressão H1/H1i antes de reativar drag; não aplicar essas etapas como continuação automática.
