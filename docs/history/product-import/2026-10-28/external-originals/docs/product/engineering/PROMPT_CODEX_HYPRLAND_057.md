# Prompt para Codex — auditoria Hyprland 0.57 (somente leitura)

Faça uma auditoria de **compatibilidade e risco de atualização do projeto contra o Hyprland 0.57.0**, sem modificar arquivos, configurações, dependências, pacotes, worktrees nem sessão gráfica.

Contexto: o relatório anterior auditou Hyprland 0.56.2. O projeto tem núcleo Lua, plugin C++ compilado contra headers internos e UI Rust/GTK4. A versão 0.57.0 foi lançada em 28/10/2026; veja `docs/product/engineering/HYPRLAND_0_57_COMPATIBILITY.md` e a auditoria existente em `docs/product/engineering/AUDITORIA_CODEX_ORIGINAL_2026-10-28.txt`.

1. Registre a versão em execução (`hyprctl version`), a instalada (`pacman -Q hyprland ...`), os plugins realmente carregados (`hyprctl plugin list`) e a origem/versão dos headers. Não presuma que instalado e executado coincidam.
2. Compare chamadas, símbolos, event handlers e hooks usados em plugin/main.cpp, plugin/barDeco.cpp e core/ contra as APIs da 0.57.0. Inspecione em especial `created` vs `created_early`, `config.unload`, `reload_config`, `minimize`, `window.update_state`, entrada por mouse, decorators e plug-in ABI.
3. Avalie o impacto nos problemas A1 a A4 e M1 a M3 da auditoria. Não afirme que novas APIs corrigem problemas sem prova.
4. Inspecione os scripts de empacotamento/rebuild e o risco de falso positivo de instalação.
5. Proponha plano mínimo de migração, validação em sessão aninhada, critérios de aceite e rollback. Identifique riscos de indisponibilidade da sessão.
6. Examine a localização de `docs/product` e o estado atual de AGENTS.md, mas não mova nem incorpore arquivos sem autorização.

**Regras:** nada de `sudo`, instalação, desinstalação, `hyprpm update`, reload, plugin load, alteração de código, correções ou commits. Testes exclusivamente de leitura; não carregar plugin na sessão principal. Apresente evidências com caminhos, linhas e comandos executados. Classifique cada achado C/P/H.

**Saída:** relatório em Markdown com matriz de compatibilidade 0.56.2 → 0.57.0, avaliação ABI, impacto em Lua/plugin/UI, condições de teste, plano incremental de adaptação e decisões que ainda exigem consentimento.
