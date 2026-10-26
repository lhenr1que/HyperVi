# Página do Kwinish no Ryoku Hub

**Estado:** ideia

Hoje as opções do Kwinish estão em arquivos JSON (`~/.config/kwinish/`) e no menu da janela. Uma página no Ryoku Hub permitiria mexer em layouts, regras por app, atalhos e modos de workspace sem tocar em arquivos, seguindo a regra de responder "como faço" primeiro pela interface.

## Pontos em aberto
- O Hub é do Ryoku (GPL-3, fork do dono); o Kwinish é independente (MIT): a página seria um módulo opcional, sem acoplar o núcleo.
- Formato dos arquivos já estável: `layouts.json`, `rules.json`, `shortcuts.json`, `workspace-modes.json`, `workspace-columns.json`.