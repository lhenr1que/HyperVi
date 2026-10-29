# Alerta de compatibilidade — Hyprland 0.57.0 (28/10/2026)

**Estado:** avaliação preliminar, NÃO teste de compatibilidade no repositório ou no desktop.  
**Baseline anterior:** auditoria do Codex em 28/10/2026 contra Hyprland 0.56.2-4 e plugin `kwinish-bars-plugin-git` r172.5333087.  
**Nota:** a atualização do pacote instalado e a versão efetivamente executada na sessão podem ser diferentes. Conferir ambas.

## Fontes primárias

- Release v0.57.0: https://github.com/hyprwm/Hyprland/releases/tag/v0.57.0
- Wiki dos plugins: https://wiki.hypr.land/Plugins/Using-Plugins/

## Alterações notáveis para este projeto

- Novo `config.unload` event, `reload_config()`, `minimize` event, `window.update_state`.
- Semântica de `created` ajustada: `created_early` representa criação antecipada; `created` após setup. Verificar handlers que dependem da ordem de eventos.
- Alterações internas em input, renderização, ciclo de vida e compositor; o plugin C++ usa cabeçalhos internos e hook por símbolo. Compilação/load contra headers 0.56.2 NÃO garante compatibilidade binária com 0.57.
- A existência de eventos de minimização e unloading cria oportunidades para corrigir rastreamento de estado e duplicação de handlers, mas não elimina automaticamente problemas A2, A3 ou M1 da auditoria.

## Antes de recompilar/carregar plugin

Colete, **sem modificar o estado**, quando em sessão Hyprland:

```bash
hyprctl version
pacman -Q hyprland kwinish-git kwinish-bars-plugin-git
hyprctl plugin list
git status --short
git worktree list --porcelain
```

1. Identificar se Hyprland em execução e pacote instalado são ambos 0.57.0.
2. Verificar versão, origem e correspondência dos headers compilados com o compositor.
3. Fazer diff das APIs utilizadas por plugin/main.cpp, plugin/barDeco.cpp, core/, `hyprctl eval` e registro de eventos.
4. Ensaiar compilação e testes em isolamento, sem carregar .so não validado na sessão principal.
5. Com plugin validado, planejar teste em Hyprland aninhado antes de instalar e recarregar na sessão real.
6. Reavaliar `kwinish-bars-rebuild` e o hook de pacote, pois auditoria apontou possível falso sucesso sem verificar a instalação.

**Não executar automaticamente:** `pacman -Syu`, `hyprpm update`, `hyprctl plugin load`, `hyprctl reload`, `sudo make install`, modificações de hooks ou worktrees. Esses comandos podem alterar sessão, sistema ou estado de usuário; exigem planejamento específico.

## Desfecho esperado da auditoria

- Matriz: símbolo/API usada | 0.56.2 | 0.57.0 | risco | ajuste mínimo | evidência.
- Evidência de versão/ABI e plano de rollback seguro.
- Decisão: compatibilidade confirmada / requer adaptação / ainda não testada.
