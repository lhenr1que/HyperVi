# Design

## Context

Ver proposal.md para os defeitos reproduzidos. Os parsers puros de rules/layouts executam `load`; settings/modes/shortcuts fazem busca textual. A UI usa serde_json e já tem write_atomic, mas ignora falha de backup, lê arquivos inteiros sem limite de tamanho e aceita a profundidade padrão do serde_json (128); o menu marca valores que o núcleo descarta. O núcleo grava rules, modo, colunas, layout ativo e switcher diretamente; assist já usa rename. Contratos em docs/API.md; não duplicar sua tabela de formatos.

## Goals / Non-Goals

**Goals:** um leitor JSON Lua compartilhado, validadores de domínio existentes, publicação atômica por arquivo e testes de falhas sem compositor.

**Non-Goals:** transações entre arquivos e mudanças de estado no compositor; resolução de edições concorrentes; fsync/garantia contra perda de energia; instalação real ou mudança no plugin.

## Decisions

- Helper JSON em Lua puro, sem load e sem pacote externo obrigatório, para funcionar no simulador e na configuração Hyprland. Descida recursiva com posição crescente, limite de entrada de 1 MiB e 32 níveis. Representar null, array e objeto distintamente (incluindo coleções vazias); tratar escapes, Unicode e números finitos. Preferir isso a regex (não respeita estrutura) ou biblioteca obrigatória não presente no pacote. Cada chamada lê no máximo limite+1 bytes para detectar excesso antes de alocar arquivo inteiro.
- Migrar rules, layouts, settings, modes e shortcuts para decodificação + validação de domínio. Erro sintático/raiz errada dá coleção vazia e diagnóstico na camada de I/O; campo inválido num documento válido não perde os vizinhos. Membros duplicados seguem último valor, como serde_json; campos desconhecidos continuam ignorados. Não mudar os nomes/formas de arquivos.
- Normalizar gap como a UI existente: inteiro limitado a 0–60; demais tipos usam nil/padrão. Guardar a validação da geometria final em zone_rects e publicação, pois até uma zona fracionária válida pode ficar menor que seu padding. Definir testes Lua/Rust com as mesmas entradas e resultados.
- Helper de escrita Lua compartilhado: serializar antes de abrir temporário, temporário no diretório do destino, checar write/close/rename e propagar erro. Migrar rules, modos, colunas, layout (texto simples) e switcher; preservar geração e fluxo do Assist ao reutilizar o helper. Caminhos não entram em shell sem quoting seguro. Não chamar isso de transação de modo.
- Leitores Rust: serde_json já trata o conteúdo como dados; falta aplicar os mesmos limites. Um helper em `zones.rs` (onde já está write_atomic) lê no máximo limite+1 bytes e recusa mais de 32 níveis, contados como no Lua; serve a todos os arquivos JSON lidos pela interface (configuração e runtime). A validação de backup de write_atomic usa esse leitor e a raiz esperada, para que um arquivo recusado nunca seja sobrescrito sem cópia. O menu marca só o que o núcleo aplicaria. A saída do `hyprctl` não é arquivo do usuário e fica fora.
- Reforçar write_atomic Rust: erro de leitura diferente de NotFound aborta; backup inválido precisa concluir antes da substituição; remover temporários em falhas. Chamadores preservam diagnóstico e não despacham aplicação de uma alteração cuja gravação falhou. Configuração válida gerada hoje não exige migração; inválida é preservada em backup antes de qualquer substituição explícita.
- Testes com fixtures compartilhadas e injeção de falhas (abertura, escrita, close, rename, backup), sempre em /tmp ou memória. Laços maliciosos são testados por processo com timeout, nunca no compositor. Atualizar o PKGBUILD para instalar o helper novo e executar suas suítes.

## Risks / Trade-offs

- [Novo parser é código sensível] → testes de gramática, Unicode, limites, raízes vazias/null e comparação de fixtures com serde_json; revisão independente antes de integrar.
- [JSON malformado antes parcialmente aceito deixa de valer] → documentar a mudança e preservar o arquivo original; não tentar recuperar executando fragmentos.
- [Falha de disco pode ocorrer depois de aplicar parte do modo] → propagar erros, documentar que reconciliação da transição é outra mudança; não prometer rollback do compositor.
- [Rename evita conteúdo parcial, mas não evita última gravação vencer] → concorrência entre múltiplos editores fica explicitamente fora do escopo.

## Migration Plan

Implementar com testes que falham, atualizar leitores e escritores em passos separados e conferir instalação do helper em staging. Arquivos válidos continuam no mesmo formato. Não tocar nos arquivos pessoais durante o desenvolvimento. Reversão do código pelo Git e dados inválidos recuperáveis no backup; nenhuma migração destrutiva automática.
