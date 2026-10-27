# Proposal

## Why

A auditoria de 27/10 reproduziu laços Lua executados por `rules.parse` e `layouts.load_user`, capazes de bloquear a configuração do compositor. Também demonstrou `gap` que gera largura negativa e opção aninhada confundida com opção de topo; várias gravações ainda truncam o destino antes de concluir.

## What Changes

- Ler JSON como dados, sem `load`, com forma de raiz e limites validados, mantendo os formatos públicos existentes.
- Unificar os critérios de layouts Lua/Rust (incluindo gap) e respeitar a estrutura dos objetos de configurações.
- Publicar arquivos de configuração e estado runtime por temporário no mesmo diretório + rename; verificar erros de escrita, fechamento e publicação.
- Preservar o arquivo anterior em falhas e recusar sobrescrever arquivo ilegível sem backup bem-sucedido.
- **BREAKING** apenas para arquivos fora do contrato: expressões Lua em JSON e tokens extraídos de JSON malformado/aninhado deixam de ser aceitos. Não há migração de formato para arquivos válidos gerados pela UI.

## Capabilities

### New Capabilities
- `user-data-io`: leitura segura dos dados locais e publicação sem conteúdo parcial, compartilhada entre núcleo e interface.

### Modified Capabilities

Nenhuma especificação principal existente; os contratos atuais estão em `docs/API.md`, seções 4 e 7, e `docs/CONFIGURAVEL.md`.

## Impact

`core/rules.lua`, `layouts.lua`, `settings.lua`, `modes.lua` e seus testes; leitores/escritores em `core/kwinish.lua`, `switcher.lua`, `assist.lua`; helper compartilhado novo em Lua; `ui/src/zones.rs` e chamadores (`settings`, `shortcut`, editor); `packaging/PKGBUILD` para instalar/testar o helper; `docs/API.md` e `CONFIGURAVEL.md`.

Sem mudança no plugin, sem dependência de Ryoku e sem pacote externo obrigatório novo. Fora do escopo: recuperação dos modos, sessão, bloqueios entre múltiplos editores, CI e loja de rices.
