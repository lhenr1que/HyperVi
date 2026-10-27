# Tasks

## 1. Leitor JSON e limites

- [ ] 1.1 Criar regressões para laços em rules/layouts, JSON truncado, raiz errada e settings com campo aninhado; mostrar falha no código anterior com subprocesso e timeout para os laços, nunca dentro do Hyprland.
- [ ] 1.2 Criar o decoder JSON Lua compartilhado; verificar com nova suíte pura para null/arrays/objetos vazios, strings escapadas/Unicode, números, trailing garbage, profundidade 32/33 e tamanho 1 MiB/1 MiB+1. Incluir o helper e a suíte nas listas do PKGBUILD; conferir instalação em staging temporário.
- [ ] 1.3 Migrar leitores rules/layouts/settings/modes/shortcuts, limitando também a leitura do arquivo a limite+1 bytes. Rodar regressões e todas as suítes Lua com runtime temporário; documentar sintaxe, limites e política de erros em docs/API.md.

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
- [ ] 5.2 Revisar independentemente parser, limites e caminhos de falha de I/O; verificar que não restou load sobre dados de usuário, nem publicação direta dos arquivos abrangidos, e validar com openspec validate harden-user-data-io --strict.

## Workflow follow-up

- Implementar com /opsx:apply harden-user-data-io ou $openspec-apply-change harden-user-data-io depois da revisão dos artefatos.
- Arquivar somente após revisão e verificações previstas; testes de instalação real, quando necessários, recebem comandos completos no STATUS e no chat.
