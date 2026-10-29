# Kwinish: regras para agentes (Claude Code, Codex)

Gerenciamento de janelas estilo KDE para o Hyprland 0.56.2, em três camadas: `plugin/` (C++), `core/` (Lua), `ui/` (Rust GTK4).

## Ler antes de mexer
1. [Mapa documental](docs/README.md), [arquitetura atual](docs/ARCHITECTURE.md) e `docs/STATUS.md`, seção "Para a próxima sessão": implementação, estado e próximos passos.
2. [Contribuição e armadilhas operacionais](docs/CONTRIBUTING.md); [handoff original preservado](docs/history/handoffs/HANDOFF.md) para contexto histórico. `docs/HANDOFF.md` mantém um apontador de compatibilidade.
3. `docs/API.md` (contratos e formatos) e `docs/CONFIGURAVEL.md` (opções do usuário): atualize-os quando mudar um contrato ou uma opção.
4. [Produto](docs/product/README.md) e [decisões D-01 a D-45](docs/product/DECISIONS.md) quando a tarefa tratar da suíte futura. Aprovação de produto, implementação e validação são estados distintos. Preserve o grau literal de aprovação e a fonte; não transforme propostas técnicas, prompts ou mockups em requisitos atuais.

## Documentação e proveniência
- Arquitetura atual deve citar código e evidências datadas; divergências com requisitos vigentes continuam registradas em `docs/STATUS.md`.
- API/CONFIGURAVEL mantêm os contratos e formatos atuais; OpenSpec mantém requisitos observáveis e mudanças incrementais. Produto futuro não substitui nenhum deles.
- [Histórico e manifesto](docs/history/README.md) preservam originais e hashes. Não edite os originais preservados; corrija a leitura consolidada e registre a divergência.
- `docs/product/drafts/` contém minutas externas, fora do OpenSpec vigente. Sua existência não autoriza aplicar tarefas, sincronizar specs ou arquivar mudanças.
- Consolidação documental não autoriza migração do Hyprland, escolha de shell, implementação, publicação ou limpeza de worktrees.

## Mudanças
- Mudança de comportamento ou de contrato: pelo OpenSpec (`openspec/changes/`, `/opsx:propose` no Claude Code, `$openspec-propose` no Codex). Correção pequena e óbvia: direto, com teste.
- Todo conserto começa por um teste que falha.
- Nunca execute dados do usuário como Lua (`load`): os JSON são lidos durante a carga da config e um laço trava o compositor.
- Grave arquivos do usuário por temporário + `rename`.

## Validar
```sh
bash tools/check.sh
```
Roda todas as suítes Lua (Lua 5.5, a do Hyprland instalado), os testes de `tools/`, `cargo test --locked` e compila o plugin num temporário, com HOME/XDG temporários; sai ≠ 0 se qualquer coisa falhar. Validação parcial e detalhes: `docs/TESTING.md`.
Teste simulado não é teste ao vivo: diga qual foi feito.

## Limites
- Não carregue/descarregue o plugin, não use `sudo`, não mexa em `~/.config` do dono sem pedido. Isso é do dono.
- O que só o dono pode verificar vai em `docs/STATUS.md`, "Testes pendentes para o dono", e também por inteiro na resposta do chat: comando (fish), o que deve acontecer, como desfazer.
- Teste ao vivo que muda arquivo do dono: `bash tools/live-backup.sh save` antes e `restore` para desfazer; nunca `rm` (ver `docs/TESTING.md`).
- Todo roteiro de teste ao vivo deve incluir o comando para carregar/recarregar o plugin e depois o núcleo do repositório; nunca presumir que o plugin já está carregado (pedido do dono, 27/10/2026). O dono executa esses comandos.
- Repositório local: sem push, fork ou PR sem pedido explícito. Commits: `git -c user.name=lhen -c user.email=lhenriqueds99@gmail.com commit`.
