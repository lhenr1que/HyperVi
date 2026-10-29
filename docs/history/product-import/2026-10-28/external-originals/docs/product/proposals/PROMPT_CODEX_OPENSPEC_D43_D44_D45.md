# Prompt para Codex — Auditoria OpenSpec D-43/D-44/D-45 (somente leitura)

Contexto: estou evoluindo um **gerenciador de janelas independente para Hyprland**, com núcleo Lua, plugin C++ e Settings Rust/GTK4 existentes. O nome público do projeto e da futura suíte Arch **não está definido**. Não confundir com o projeto CLEA/Nycti descontinuado.

A auditoria histórica do commit local `60e19df` apontou risco de perda de proveniência de janelas após reload e problemas de minimização/Floating. **Esse estado não foi revalidado.**

## Decisões aprovadas

- **D-43:** mudar para outro workspace **no mesmo monitor** encerra Mostrar área de trabalho, restaurando apenas as janelas anteriormente visíveis que foram temporariamente ocultadas.
- **D-44:** mapear efetivamente **nova janela elegível** no workspace revelado encerra a ocultação, restaura as janelas antes visíveis e apresenta a nova janela. Tentativa de spawn ou nova janela em outro workspace não basta.
- **D-45:** clicar **na dock em janela já existente temporariamente ocultada** encerra a ocultação, restaura **todo o conjunto anteriormente visível** e em seguida **foca a janela clicada**. Janelas previamente minimizadas permanecem minimizadas. No Floating, essa intenção de restaurar/focar **prevalece** sobre o clique normal que minimiza uma janela focada. Hover, prévia ou explorar cartão do overview não acionam D-45. Favorito aberto apenas em outro workspace segue a política anterior (solicitar nova janela no workspace atual).
- **D-42:** escopo local do workspace ativo do monitor em foco. Nenhuma destas decisões altera outros monitores/workspaces por consequência não autorizada.

## Instruções (não implementar nesta rodada)

1. Inspecione `AGENTS.md`, Git, OpenSpec specs/changes atuais, código de Mostrar área de trabalho/minimização/foco da dock e integrações Lua/C++/Rust. **Não descarte alterações locais.**
2. Compare `PACOTE_OPENSPEC_D43_D44_D45/` com o repositório. A capability `show-desktop` é **provisória**; não sobrescreva specs existentes nem crie autoridade de restauração paralela.
3. Identifique os eventos reais de mudança de workspace, mapeamento de toplevel e **ativação explícita de janela pela dock**, diferenciando hover/preview, clique em favorito e clique em tarefa.
4. Verifique a precedência sobre o toggle de minimização no Floating. Projete uma operação idempotente com token de geração, identidade e escopo por sessão, revalidando cliente antes de focar.
5. Separe resultados: **restauração confirmada**, **foco confirmado**, parcial/falha; evite roubo de foco tardio em corridas com troca de workspace e D-44.
6. Reproduza ou descarte tecnicamente os problemas de reload e proveniência do histórico. Não faça I/O bloqueante inseguro em callback do compositor.
7. Revise proposta, delta spec, design, tasks e matriz `MATRIZ_TESTES_SHOW_DESKTOP_D43_D44_D45_v1.2.md` conforme o formato e a versão local do OpenSpec.
8. **Não altere código ainda.** Entregue relatório de auditoria, matriz requisito→implementação real, riscos, revisão do change e plano incremental. Aguarde aprovação para implementação.

**Limites:** não mexer em Rice Store, dashboard, VXWM, tema, API de captura ou interface visual de Visão Geral. Não afirmar que testes passaram ou que o change foi validado sem executá-los.
