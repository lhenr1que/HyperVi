# Contribuição e operação

Guia consolidado em 29/10/2026. As regras operacionais estão no [AGENTS](../AGENTS.md); os relatos originais permanecem no [handoff](history/handoffs/HANDOFF.md) e no [rascunho beta](history/initial-research/BETA.md). Esta consolidação não autoriza publicação, escolha de nome, shell, atualização do sistema ou execução de testes ao vivo.

## Fluxo de trabalho

1. Leia [arquitetura](ARCHITECTURE.md), [status](STATUS.md), [API](API.md) e [opções](CONFIGURAVEL.md).
2. Confira o estado Git; preserve alterações e worktrees de outras sessões. Não limpe arquivos não rastreados incidentalmente.
3. Mudança de comportamento ou contrato passa pelo OpenSpec conforme AGENTS. Correções pequenas seguem suas regras de teste; todo conserto começa por um teste que falha.
4. Atualize contratos e opções quando mudarem. Registre divergências; não torne um bug consistente removendo um requisito.
5. Execute a validação adequada descrita em [TESTING](TESTING.md), identifique testes simulados e ao vivo e revise o diff. Em documentação pura, verifique integridade, referências e escopo; não apresente isso como teste do software.
6. Commit, push, fork e PR seguem autorização do dono. A presente consolidação não inclui commit.

## Cuidados técnicos vigentes

- Dados do usuário são dados: nunca `load` de JSON ou Lua derivado de arquivo do usuário. Use a leitura e validação existentes; preserve gravação por temporário e `rename`.
- O plugin C++ é vinculado ao build do Hyprland. Compilar não comprova carregamento, foco ou arrasto reais.
- Criar objetos de bind por `hyprctl eval` e duplicar hooks/timers no reload são armadilhas registradas. Consulte o status e o handoff antes de alterar o ciclo de carga.
- Snap, zonas e ações afins dependem do contexto de janela/monitor e das políticas Floating. Modos por workspace não devem ser inferidos apenas do layout visual.
- Estado reconstruído por tags e estado transitório Lua têm durações diferentes. `stashed` de mostrar desktop continua volátil; D-43–D-45 são comportamento futuro.
- A UI layer-shell pode reter foco durante ações. Preserve temporizações/revalidações atuais; não transfira políticas para uma futura dock ou Overview sem contrato explícito.

## Testes que dependem do dono

Não carregue/descarregue plugin, use `sudo` ou altere `~/.config` sem pedido. Registre o que só o dono pode verificar em [STATUS](STATUS.md), seção "Testes pendentes para o dono", com roteiro completo: comandos fish, comportamento esperado e como desfazer. Entregue o mesmo roteiro integral no chat quando solicitar sua execução.

Um roteiro ao vivo inclui carregar/recarregar o plugin e depois o núcleo do repositório, sem presumir plugin já carregado. Antes de alterar arquivo do dono, use o mecanismo `tools/live-backup.sh save`; desfazer usa `restore`, nunca remoção. Os comandos permanecem documentados em [TESTING](TESTING.md) e não foram executados nesta consolidação.

## Prontidão de distribuição

O beta original é um rascunho histórico. Continuam pertinentes como critérios a avaliar: instalação limpa independente de Ryoku/HDE, remoção dos pacotes, hook de rebuild em upgrade real, testes de interação e documentação pública. Nenhum desses itens recebe aprovação ou conclusão nesta tarefa. Nome definitivo, publicação e tratamento de dados pessoais no histórico exigem decisão separada; não autorizam reescrita ou limpeza automática.

## Licenças e atribuições

Conserve [LICENSE](../LICENSE), [NOTICE do plugin](../plugin/NOTICE), [licença de origem hyprbars](../plugin/LICENSE.hyprland-plugins) e cabeçalhos SPDX. O acervo mantém referências e triagens históricas. Antes de reutilizar componente externo, registre origem, commit, arquivos transitivos, licença, autoria e alterações. A pesquisa de shells não autoriza incorporação de código nem escolhe uma base.

## Manutenção documental

O [índice](README.md) orienta a leitura. [Produto](product/README.md) e [decisões](product/DECISIONS.md) conservam status distintos da implementação. Preserve originais antes de substituir texto; registre origem/destino no [manifesto](history/product-import/2026-10-28/MANIFEST.md). Corrija documentos consolidados, mantendo intactos os originais preservados e o OpenSpec sem autorização específica.
