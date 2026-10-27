# Página do Kwinish no Ryoku Hub

**Estado:** ideia

Hoje as opções do Kwinish estão em arquivos JSON (`~/.config/kwinish/`) e no menu da janela. Uma página no Ryoku Hub permitiria mexer em layouts, regras por app, atalhos e modos de workspace sem tocar em arquivos, seguindo a regra de responder "como faço" primeiro pela interface.

## Pontos em aberto
- O Hub é do Ryoku (GPL-3, fork do dono); o Kwinish é independente (MIT): a página seria um módulo opcional, sem acoplar o núcleo.
- Formato dos arquivos já estável: `layouts.json`, `rules.json`, `shortcuts.json`, `workspace-modes.json`, `workspace-columns.json`.

## Opções do plugin que a página deve expor (26/10)
`seam_resistance` (pixels de empurrão para atravessar a fronteira entre monitores, 0 desliga; padrão 200), `snap_top_edge`, `snap_preview`, `preview_color`, `bar_height`. Hoje se ajustam por `hyprctl eval "hl.config({plugin={kwinish={...}}})"` ou na config do Hyprland.
