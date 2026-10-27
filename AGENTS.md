# Kwinish: regras para agentes (Claude Code, Codex)

Gerenciamento de janelas estilo KDE para o Hyprland 0.56.2, em três camadas: `plugin/` (C++), `core/` (Lua), `ui/` (Rust GTK4).

## Ler antes de mexer
1. `docs/STATUS.md`, seção "Para a próxima sessão": estado atual e próximos passos.
2. `docs/HANDOFF.md`: armadilhas e histórico.
3. `docs/API.md` (contratos e formatos) e `docs/CONFIGURAVEL.md` (opções do usuário): atualize-os quando mudar um contrato ou uma opção.

## Mudanças
- Mudança de comportamento ou de contrato: pelo OpenSpec (`openspec/changes/`, `/opsx:propose` no Claude Code, `$openspec-propose` no Codex). Correção pequena e óbvia: direto, com teste.
- Todo conserto começa por um teste que falha.
- Nunca execute dados do usuário como Lua (`load`): os JSON são lidos durante a carga da config e um laço trava o compositor.
- Grave arquivos do usuário por temporário + `rename`.

## Validar
```sh
for t in core/test_*.lua; do lua5.4 $t || echo FALHOU $t; done
cargo test --manifest-path ui/Cargo.toml
make -C plugin
```
Teste simulado não é teste ao vivo: diga qual foi feito.

## Limites
- Não carregue/descarregue o plugin, não use `sudo`, não mexa em `~/.config` do dono sem pedido. Isso é do dono.
- O que só o dono pode verificar vai em `docs/STATUS.md`, "Testes pendentes para o dono", e também por inteiro na resposta do chat: comando (fish), o que deve acontecer, como desfazer.
- Repositório local: sem push, fork ou PR sem pedido explícito. Commits: `git -c user.name=lhen -c user.email=lhenriqueds99@gmail.com commit`.
