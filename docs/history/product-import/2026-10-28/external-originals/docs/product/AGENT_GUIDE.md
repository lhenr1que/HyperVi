# Regras para agentes de IA que trabalham neste repositório

**Aviso importante:** esta pasta é documentação de VISÃO DE PRODUTO; a maior parte das ideias não está implementada. Não trate texto de mockups como API ou critério de aceite automaticamente.

1. Antes de implementar, leia `AGENTS.md` na raiz, código relevante, testes e `openspec/specs/` existentes. Reconcile divergências.
2. Ao receber uma funcionalidade, procure a decisão aprovada em `vision/ESPECIFICACAO_INTERFACE_SUITE_v3.13.md`; confira seu status. Regras propostas / P-* e ideias futuras / F-* não são decisões aprovadas.
3. Não copiar `proposals/stabilize-show-desktop-restoration` para `openspec/changes` sem inspeção do código e autorização do usuário.
4. Não executar reescrita ampla, upgrade do Hyprland, rebuild/instalação ou limpeza de worktrees sem autorização específica.
5. Hyprland 0.56.2 foi baseline da auditoria em 28/10/2026; 0.57.0 lançou-se nesse dia. Antes de compilar/carregar plugin C++, comparar versão do compositor em execução, headers e ABI, verificar APIs alteradas.
6. Validar alterações no Lua, C++, Rust/GTK4 de forma integrada. Rodar tests e separar simulação de teste funcional ao vivo.
7. Preservar o comportamento do gerenciador independente do rice; não implementar Shell/Store por reflexo de design.
8. Relatar cada resultado com arquivo, símbolo, teste e limitações. Não inventar implementações nem resultados.
9. Evitar tocar worktrees secundárias sujas; identificar, preservar e solicitar decisão de quem mantém o projeto.
10. Para o trabalho imediato, comece pelo parecer de compatibilidade 0.57 e pelo baseline, não pela UI futura.
