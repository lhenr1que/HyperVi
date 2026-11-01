# Tasks

## 1. Textos e teste que falha

- [x] 1.1 Em `plugin/test_tooltip.cpp`, casos de `pin` (fixada e não fixada) e `menu` em pt e en, e trocar o caso "pin has no tip" por `float`; verificar que `bash tools/test-plugin-tooltip.sh` falha nesses casos
- [x] 1.2 Em `plugin/tooltip.hpp`, `text` passa a receber o estado do botão (maximizada no maximizar, fixada no fixar) e ganha `pin` e `menu`; verificar com `bash tools/test-plugin-tooltip.sh` (sai 0)

## 2. Plugin

- [x] 2.1 Em `plugin/barDeco.cpp`, o ícone do aplicativo vira alvo da dica (mesmo retângulo do clique que abre o menu, só quando o ícone existe) e o fixar passa o estado `m_pinned`; verificar com `make -C plugin TARGET=<temporário>` sem erro nem aviso novo
- [x] 2.2 Atualizar a linha das dicas em `docs/PLUGIN.md` (fixar, ícone, textos); verificar lendo o trecho

## 3. Integração

- [x] 3.1 `bash tools/check.sh` sai 0
- [x] 3.2 Roteiro ao vivo em `docs/STATUS.md`, "Testes pendentes para o dono": carregar plugin e núcleo do repositório, parar no fixar (antes e depois de fixar) e no ícone, conferir que o clique no ícone ainda abre o menu; e como desfazer

## Workflow follow-up

- Arquivar com `/opsx:archive` (ou `$openspec-archive-change` no Codex) depois do teste do dono.

## Arquivamento (01/11/2026)

Arquivada à mão (CLI `openspec` não instalada) depois do teste ao vivo 10, reescrito para a árvore 0.57 (`70c746a`) e relatado pelo dono como passou em 01/11/2026 (o relato "o teste passou" cobriu os testes 10 e 11 juntos). O dono mandou seguir em 01/11/2026. No spec principal `title-bar`, os 2 requisitos MODIFIED substituíram os anteriores e o ADDED "Dica no botão de fixar e no ícone do aplicativo" foi para o fim, com o texto copiado como está. Conferência estrutural: `tools/test-openspec-specs.sh` passou a conferir também esta mudança; vermelho antes de mover a pasta e depois de mover sem aplicar o delta, verde depois, e o controle negativo pega uma quebra no texto novo.
