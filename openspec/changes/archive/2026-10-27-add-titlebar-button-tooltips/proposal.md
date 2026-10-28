# Proposal

## Why

Os botões da barra de título (`kwinish-bars`) são só glifos: quem chega do KDE espera, ao parar o mouse em cima de minimizar, maximizar ou fechar, uma caixinha com o nome da ação, como no Breeze. Pedido do dono em 27/10/2026.

## What Changes

- Parar o ponteiro sobre um botão de minimizar, maximizar ou fechar mostra, depois de um instante, uma dica com o nome da ação logo abaixo do botão, dentro do monitor.
- O texto segue o idioma do ambiente como o resto da interface (português quando o locale começa por `pt`, inglês nos outros casos). No maximizar de uma janela já maximizada a dica diz "Restaurar"/"Restore", que é o que o clique faz.
- A dica some ao tirar o ponteiro do botão, ao clicar, ao começar um arrasto e quando a barra deixa de existir.
- Vale para todo botão com ação `minimize`, `maximize` ou `close`, inclusive os acrescentados pelo usuário com `add_button`. Os outros botões (fixar, `exec:`) não ganham dica nesta mudança.
- Sem opção nova: atraso e aparência fixos, cores tiradas das cores da barra.

## Capabilities

### New Capabilities
- `title-bar`: a barra de título que o plugin põe nas janelas: botões, as ações deles e o que mostram ao passar o mouse.

### Modified Capabilities

## Impact

Só o plugin: `plugin/barDeco.cpp`/`.hpp` (estado de hover e da dica), `plugin/main.cpp` (desenho na etapa de render depois das janelas, limpeza no `PLUGIN_EXIT`), um cabeçalho novo com a lógica pura (texto e posição) e o teste dele em `tools/`. `docs/PLUGIN.md` ganha uma linha. Sem mudança no núcleo Lua, na UI Rust, em formatos de arquivo ou em opções.
