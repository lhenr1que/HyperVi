# Tasks

## 1. Lógica pura e teste que falha

- [x] 1.1 Escrever `plugin/test_tooltip.cpp` (texto por ação, locale e estado maximizado; posição presa no monitor, embaixo ou em cima da barra) e `tools/test-plugin-tooltip.sh`, que o compila com `g++` e roda; verificar que falha antes do cabeçalho existir (`bash tools/test-plugin-tooltip.sh` sai ≠ 0)
- [x] 1.2 Criar `plugin/tooltip.hpp` (sem Hyprland: `KwTip::portuguese`, `KwTip::text`, `KwTip::place`); verificar com `bash tools/test-plugin-tooltip.sh` (sai 0)

## 2. Dica no plugin

- [x] 2.1 Estado `tip` em `globals.hpp`; posse da dica em `damageOnButtonHover` (só a barra com a janela sob o ponteiro), temporizador de 700 ms, dano do monitor ao mostrar e esconder; verificar com `make -C plugin TARGET=<temporário>` sem erro
- [x] 2.2 Desenho em `RENDER_POST_WINDOWS` (`main.cpp`) com `CRectPassElement`/`CTexPassElement`; esconder em qualquer aperto de botão do mouse, no destrutor da barra e no `PLUGIN_EXIT` (removendo o temporizador do `g_pEventLoopManager`); verificar compilando de novo
- [x] 2.3 Uma linha em `docs/PLUGIN.md` sobre as dicas (quais botões, idioma, sem opção); verificar lendo o trecho

## 3. Integração

- [x] 3.1 `bash tools/check.sh` sai 0 (Lua, `tools/` com o teste novo, Rust, plugin)
- [x] 3.2 Roteiro ao vivo em `docs/STATUS.md`, "Testes pendentes para o dono": carregar o plugin e o núcleo do repositório, parar o mouse em cada botão (pt e en), clicar, sair, descarregar com a dica na tela; e como desfazer

## Workflow follow-up

- Arquivar com `/opsx:archive` (ou `$openspec-archive-change` no Codex) depois do teste do dono.
