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
