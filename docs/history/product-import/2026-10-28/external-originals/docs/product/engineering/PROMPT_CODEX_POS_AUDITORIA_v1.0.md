# Prompt para Codex — Pós-auditoria: baseline seguro e plano incremental

Recebi sua auditoria técnica e quero **transformá-la em um plano de ação**, sem começar a implementação agora.

## Premissas

- O gerenciador é uma extensão independente para Hyprland: políticas Lua, plugin C++ e UI Rust/GTK4.
- Sua auditoria identificou A1, A2, A3, A4, M1–M10 e divergências de documentação/OpenSpec.
- Existem worktrees com alterações não commitadas e não há remoto Git configurado.
- O futuro shell oficial, Rice Store, visuais e Visão Geral são requisitos de produto separados; NÃO fazem parte desta rodada.
- Os requisitos aprovados para Show Desktop são: D-42 escopo local; D-43 restaurar ao sair do workspace; D-44 restaurar quando uma nova janela elegível for efetivamente mapeada; D-45 restaurar e focar ao selecionar pela dock uma janela temporariamente ocultada. Não implemente D-43–D-45 ainda.

## Missão — somente leitura

1. **Gate 0 — Segurança:** inventarie cada worktree, branch, commit e alteração local. Para cada uma, diga se contém trabalho possivelmente exclusivo. Não execute clean, prune, checkout, reset, rebase, merge ou commit e não crie remoto sem minha autorização.
2. **Baseline:** indique como executar e registrar exatamente os testes manuais pendentes 10, 11 e 12; identifique os subcasos e quais mudanças OpenSpec dependem deles. Não marque como verificado o que eu ainda não testei.
3. **Documentação:** proponha uma correção documental mínima para divergências objetivas (README, STATUS, TESTING, PLUGIN, atalhos e versão), sem editar nesta rodada.
4. **Ordem dos bugs:** analise dependências reais entre M1 (reload/hooks), A2 (perda de proveniência Floating), A3 (show desktop/reload) e A1 (cascata). Recomende a menor sequência de changes independentes possível, com testes de regressão e revisão ao vivo.
5. **OpenSpec:** confronte as duas changes ativas e as quatro specs principais com o código; indique gaps e riscos de duplicação antes de importar a minuta externa `stabilize-show-desktop-restoration`.
6. **Arquitetura:** identifique os pontos existentes a reutilizar, especialmente `window_action`, tags de janela, `kwjson`, testes compartilhados e simulação de eventos. Não proponha reescrita geral.

## Entrega

Produza um relatório Markdown com:
- ordem operacional segura do Gate 0;
- tabela arquivo/worktree/risco/ação proposta;
- roteiro dos testes pendentes e evidências necessárias;
- backlog dos bugs com prioridade, dependências e critérios de aceite;
- proposta de divisão das changes OpenSpec (nomes provisórios);
- riscos de regressão e rollback;
- o **primeiro passo único recomendado**, acompanhado de comando(s) apenas quando não destrutivos.

**Restrições:** não altere arquivos, não faça commits, não instale pacotes, não rode testes no desktop real sem me explicar os efeitos e receber aprovação, não descarte alterações e não comece a integrar o shell. Se uma proposta depender de informações ainda não verificadas, marque explicitamente como pendente.
