# Proposal

## Why

As dicas de minimizar, maximizar e fechar foram verificadas pelo dono (27/10/2026, teste 8) e ele pediu o mesmo no botão de fixar e no ícone do aplicativo, que hoje são os dois únicos alvos clicáveis da barra sem nome visível. O KDE dá dica aos dois ("On all desktops"/"On one desktop" e "Window menu").

## What Changes

- O botão de fixar mostra a dica "Em todas as áreas de trabalho"/"On all workspaces"; numa janela já fixada, "Só nesta área de trabalho"/"Only on this workspace", que é o que o clique faz (como Maximizar/Restaurar).
- O ícone do aplicativo, à esquerda da barra, mostra "Menu da janela"/"Window menu" (o clique nele abre o menu da janela). Um botão do usuário com a ação `menu` ganha a mesma dica.
- Atraso, aparência, posição e regras para sumir são os mesmos das dicas atuais. Botões `exec:` e as outras ações continuam sem dica.

## Capabilities

### New Capabilities

### Modified Capabilities
- `title-bar`: a dica passa a valer também para o fixar e para o ícone do aplicativo, com os textos novos.

## Impact

Só o plugin: `plugin/tooltip.hpp` (textos novos), `plugin/barDeco.cpp` (o ícone como alvo da dica) e o teste `plugin/test_tooltip.cpp`. `docs/PLUGIN.md` atualiza a linha das dicas. Sem opção nova, sem mudança no núcleo, na UI ou em formatos.
