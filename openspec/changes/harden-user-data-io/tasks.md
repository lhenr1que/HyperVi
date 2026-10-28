# Tasks

## 1. Leitor JSON e limites

- [x] 1.1 Criar `core/test_userdata.lua` com regressões dos leitores Lua: laço em rules e layouts (subprocesso com `timeout`, nunca dentro do Hyprland), JSON truncado e raiz errada em rules, layouts, settings, modes, columns e shortcuts, e settings com campo aninhado. Mostrar a falha no código anterior (módulos extraídos do Git para um temporário, sem worktree).
- [x] 1.2 Criar o decoder JSON Lua compartilhado (`core/kwjson.lua`; nome próprio para não colidir com um módulo `json` já carregado pela config); verificar com nova suíte pura (`core/test_kwjson.lua`) para null/arrays/objetos vazios, strings escapadas/Unicode (inclusive surrogates), UTF-8 inválido, números (inclusive não finitos), trailing garbage, profundidade 32/33 e tamanho 1 MiB/1 MiB+1. Guardar os casos de aceitar/recusar numa lista de texto (`core/testdata/json-cases.txt`) que a suíte Rust da 1.4 também lê. Incluir o helper na lista de instalação do PKGBUILD (a suíte entra pelo glob do check.sh); conferir instalação em staging temporário.
- [x] 1.3 Migrar os leitores Lua de rules, layouts, settings, modes, columns (`workspace-columns.json`) e shortcuts, lendo no máximo limite+1 bytes num só helper, com diagnóstico em stderr uma vez por arquivo e erro. Rodar regressões e todas as suítes Lua com runtime temporário; carregar o núcleo com `Hyprland --verify-config` numa config temporária, sem sessão; documentar sintaxe, limites e política de erros em docs/API.md.
- [ ] 1.4 Leitores Rust da `ui/` (serde_json não executa código; faltam limites e forma): criar em `ui/src/zones.rs` um leitor único que lê no máximo 1 MiB + 1 byte e recusa mais de 32 níveis (mesma contagem do Lua); testes que falham antes: 1 MiB/1 MiB+1, 32/33 níveis, UTF-8 inválido, ausente, e os casos de `core/testdata/json-cases.txt` com o mesmo resultado do Lua. Migrar todo leitor de arquivo JSON da interface: `zones::load` (layouts.json), `settings::load`, `shortcut::load`, `menu.rs` (rules.json, workspace-modes.json, workspace-columns.json), `main.rs` (switcher.json) e `assist.rs` (assist.json, duas leituras). O `parses` passado a `write_atomic` usa o mesmo leitor e a mesma raiz, para que um arquivo que a leitura recusa vá para `.bak` antes de ser substituído. As marcas do menu seguem os tipos do núcleo (regra booleana só `true`, workspace inteiro > 0, rect de 4 inteiros com w,h > 0; modo só tiling/scrolling/floating). Fora, por não serem arquivos do usuário: a saída do `hyprctl` e o arquivo de texto `layout`. Verificar com `cargo test --locked --manifest-path ui/Cargo.toml`.

## 2. Paridade de layouts

- [ ] 2.1 Adicionar fixtures equivalentes Lua/Rust para gap negativo, gigante, fracionário, string, ausente e zonas pequenas; mostrar os casos divergentes antes do conserto.
- [ ] 2.2 Alinhar os validadores e impedir geometria final não positiva/não finita na publicação; verificar fixtures e cargo test --manifest-path ui/Cargo.toml. Atualizar docs/CONFIGURAVEL.md com a normalização e o descarte de geometrias inválidas.

## 3. Gravação confiável no núcleo

- [ ] 3.1 Criar testes de preservação do destino em falha de serialização, abertura, escrita, close e rename, incluindo primeira gravação; usar I/O injetado e /tmp. Mostrar os testes que falham com a gravação direta atual.
- [ ] 3.2 Implementar helper atômico Lua com propagação de erro e backup obrigatório de configuração inválida. Migrar rules, modos, colunas e layout ativo; verificar bytes anteriores intactos em cada falha e registrar o contrato em docs/API.md.
- [ ] 3.3 Usar o helper em switcher e Assist sem alterar formato/geração; verificar leitor vendo JSON completo, regressões do Assist e isolamento de test_switcher.lua em runtime temporário. Documentar que runtime é descartável e não exige backup.

## 4. Gravação confiável na interface

- [ ] 4.1 Adicionar testes Rust para falhas de leitura, backup e rename; mostrar que o helper atual ignora falha de backup, sem depender de permissões reais de ~/.config.
- [ ] 4.2 Reforçar write_atomic e chamadores em settings/shortcut/editor; verificar arquivo anterior preservado, temporários removidos quando possível e ausência de dispatch de aplicação quando o save falha. Documentar diagnóstico de gravação no contrato existente.

## 5. Integração e revisão

- [ ] 5.1 Executar todas as suítes Lua com XDG_RUNTIME_DIR temporário, cargo test --locked e testes de staging do PKGBUILD; conferir formatos válidos antigos com fixtures e registrar resultados em docs/STATUS.md sem apresentar teste simulado como ao vivo.
- [ ] 5.2 Revisar independentemente parser, limites e caminhos de falha de I/O; verificar que não restou load sobre dados de usuário, nem leitor de arquivo JSON em `ui/src` fora do helper com limites, nem publicação direta dos arquivos abrangidos, e validar com openspec validate harden-user-data-io --strict.

## Workflow follow-up

- Implementar com /opsx:apply harden-user-data-io ou $openspec-apply-change harden-user-data-io depois da revisão dos artefatos.
- Arquivar somente após revisão e verificações previstas; testes de instalação real, quando necessários, recebem comandos completos no STATUS e no chat.
