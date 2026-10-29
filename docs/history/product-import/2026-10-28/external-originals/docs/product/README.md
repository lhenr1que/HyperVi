# Planejamento de produto — futura suíte Arch / gerenciador Hyprland

**Última consolidação:** 28/10/2026 · **Estado:** documentação de produto, NÃO implementação em vigor.  
**Nome:** ainda não definido; `kwinish` é apenas o identificador técnico atual.  
**Codex:** leia `AGENT_GUIDE.md` antes de usar documentos desta pasta.

## Hierarquia de autoridade

1. **Código e comportamento efetivamente testado:** determinam o que existe hoje. Não inferir implementação a partir desta pasta.
2. **`openspec/specs/` do repositório:** documenta o contrato técnico atualmente aceito, sujeito à auditoria de divergências.
3. **`vision/ESPECIFICACAO_INTERFACE_SUITE_v3.13.md`:** registro da visão de produto e decisões D-01 a D-45; distinguir status aprovado de proposta.
4. **`features/`, `design/`, `engineering/`:** detalhamento, propostas e contratos a reconciliar com o código.
5. **`proposals/`:** material de planejamento, NÃO instalar no OpenSpec automaticamente.

## Navegação

- Visão e decisões: `vision/ESPECIFICACAO_INTERFACE_SUITE_v3.13.md`
- Dock: `features/DOCK_ESPECIFICACAO_CONSOLIDADA_v1.2.md`
- Launcher: `features/LAUNCHER_HIBRIDO_ESPECIFICACAO_CONSOLIDADA_v1.0.md`
- Settings: `features/SETTINGS_ARQUITETURA_FUNCIONAL_CONSOLIDADA_v1.0.md`
- Visão Geral e Mostrar área de trabalho: `features/VISUALIZACAO_WORKSPACES_AREA_DE_TRABALHO_v1.0.md`
- Design System: `design/DESIGN_SYSTEM_INTERFACE_OFICIAL_v1.8.md`
- Contrato de restauração: `engineering/CONTRATO_VISAO_GERAL_FOCO_RESTAURACAO_v1.4.md`
- Auditoria real de 28/10/2026 (0.56.2): `engineering/AUDITORIA_CODEX_ORIGINAL_2026-10-28.txt`
- Riscos da transição para Hyprland 0.57: `engineering/HYPRLAND_0_57_COMPATIBILITY.md`
- Próximo pedido ao Codex: `engineering/PROMPT_CODEX_HYPRLAND_057.md`

## Regra de integração

Nenhum agente deve reinterpretar decisões de produto como autorização para implementar ou reescrever o sistema. Verificar estágio real, especificar delta pequeno, obter aprovação e testar em sessão aninhada quando envolver plugin/ABI.

## História

Este pacote contém as **versões consolidadas** das especificações e um subconjunto selecionado dos mockups. As versões anteriores não foram copiadas para evitar duplicações na indexação dos agentes. O histórico aprovado D-01 a D-45 permanece no documento de visão. Preserve cópia do pacote original completo fora do repositório se quiser histórico de cada arquivo.
