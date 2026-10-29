# Referências de projetos e ferramentas avaliadas

**Estado:** referência

Avaliações de fontes de inspiração (Pop Shell, KWin, hyprbars, HyDE, Ryoku, REA) e de ideias de outras IAs que o dono colou.

## Doadores de ideias
Pop Shell e KWin: só conceitos (cache de geometria, `transientFor`, z-order); GPL, nunca copiar código para o Kwinish (MIT).
`hyprbars` já é a base do plugin (BSD-3, `plugin/NOTICE`). HyDE (GPL-3): só ideias de detecção de GPU e captura; o Kwinish não toca
em GPU. Ryoku: GPL-3 com terceiros no `NOTICE`; ver `ryoku-work/docs/distro/ESCOPO-DISTRO.md`.

## REA
"Reverse Engineer Anything": MCP (Node 22+) para engenharia reversa de binários, apps e comportamento em execução, apoiado em Hopper, Ghidra ou IDA (ferramentas comerciais à parte); MIT, uso legal e autorizado. Serve pouco aqui: os doadores (Hyprland, hyprbars, KWin, Pop Shell, Ryoku) têm código aberto e se leem direto; para o Hyprland bastou `strings` e os cabeçalhos instalados. Reverter GPL não tira a obrigação GPL do que for derivado. Útil só se um dia houver um binário fechado a estudar.

## Ideias avaliadas
- **Minimizar com janelas filhas/modais**: hoje `minimize` manda só a janela para `special:minimized`; diálogos filhos ficam para trás.
  Pendência real (testar com um app que abra diálogo; KWin trata por `transientFor`, só como conceito: o KWin é GPL e este projeto é MIT, nunca copiar código).
- **Página no Ryoku Hub** para layouts, regras e barras (hoje tudo em JSON/menu). Seguir a regra GUI-primeiro do vault.
- **Barras em apps com decoração própria (CSD)**: decidir se há opt-out por app (já existe "sem barra e moldura" e regra por app).
- Descartado: mexer na memória do `CWindow` para "esconder sem mudar de workspace" (frágil a cada versão do Hyprland); manter `special:minimized`.
