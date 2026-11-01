# Produto — visão futura da suíte

**Consolidação:** 29/10/2026. **Origem:** pacote externo de 28/10/2026. **Estado:** documentação de produto; não descreve uma suíte instalada nem autoriza implementação. O nome público é **HyperVi** (decisão do dono, 01/11/2026); `kwinish`, o nome anterior, continua como identificador técnico (módulo Lua, opções `plugin:kwinish:*`, pastas, binários e etiquetas).

A visão é uma suíte opcional baseada em Hyprland, com gerenciamento Tiling/Scroll/Floating independente do shell visual, rice oficial e futura loja de rices completos. A autorização desta etapa abrange somente documentação. Nenhum shell, toolkit, fork, reescrita de GTK4 ou migração para Hyprland 0.57 foi escolhido.

## Leitura por finalidade

| Documento | Conteúdo e validade |
|---|---|
| [DECISIONS](DECISIONS.md) | D-01–D-45; grau literal de aprovação, implementação e validação separados |
| [INTERFACE](INTERFACE.md) | Barra, dock e launcher futuros, com limites das decisões |
| [SETTINGS](SETTINGS.md) | Sistema permanente, interface conforme rice e propostas de integração |
| [WORKSPACES](WORKSPACES.md) | Overview, foco e Mostrar área de trabalho futuros |
| [DESIGN](DESIGN.md) | Direção visual, tokens propostos e legendas dos [mockups](mockups/) |
| [PROPOSALS](PROPOSALS.md) | P-*/F-*, alternativas e propostas técnicas ainda não aprovadas |
| [Rascunho externo](drafts/stabilize-show-desktop-restoration/proposal.md) | Minuta preservada; fora das capacidades e mudanças OpenSpec vigentes |

Para saber o que existe, começar pelo [índice técnico](../README.md), [arquitetura atual](../ARCHITECTURE.md), [API](../API.md), [opções atuais](../CONFIGURAVEL.md) e [estado/validação](../STATUS.md). As capacidades aceitas e deltas em andamento permanecem no [OpenSpec real](../../openspec/); uma mudança ativa pode já conter implementação sem estar encerrada.

## Como interpretar um requisito

| Campo | Significado |
|---|---|
| Decisão/aprovação | O que a fonte externa registra como aprovado, adotado, solicitado ou pendente; não amplia essa aprovação |
| Implementação | Correspondência verificável no código atual, podendo ser parcial ou ausente |
| Validação | Evidência e data, distinguindo inspeção estática, simulação e teste ao vivo; “planejado” não é “executado” |
| Proveniência | Arquivo/seção de origem, D-* relacionado e eventual proposta/delta |

Para **descrever implementação**, usar código e evidências datadas. Para **requisitos técnicos aceitos**, consultar OpenSpec principal e deltas relevantes; registrar divergências sem apagar o contrato. Para **produto futuro**, consultar DECISIONS e seus limites. Para **alternativas**, consultar PROPOSALS. Nenhuma dessas classes substitui as regras operacionais do [AGENTS](../../AGENTS.md).

## Fronteiras da visão

- D-06 prevê ativação de um rice completo por vez; componentes de rices diferentes não são combinados na primeira versão.
- D-08 mantém gerenciamento e configurações fundamentais independentes do rice. A organização interna do rice oficial não obriga terceiros a copiar sua composição.
- D-07 mantém dashboard fora do escopo; D-10 mantém Canvas/VXWM como futuro, sem compromisso de versão.
- D-23 preserva Settings fundamental e adapta as páginas de interface ao rice efetivamente suportado. A janela GTK4 atual continua sendo implementação existente, com escopo menor.
- Barra/dock/launcher oficiais, Overview, Rice Store e regras D-43–D-45 não estão implementados como suíte. Recursos existentes do gerenciador não comprovam esses contratos completos.

## Fontes preservadas e manutenção

O [manifesto histórico](../history/README.md) registra origens e destinos. O [inventário com hashes](../history/product-import/2026-10-28/SOURCES.json), o [ZIP original preservado](../history/product-import/2026-10-28/HyperVi_Product_Docs_2026-10-28.zip) e os [textos originais](../history/product-import/2026-10-28/external-originals/docs/product/) permitem reconstruir a origem de cada consolidação. Documentos antigos, prompts, cabeçalhos e mockups não foram corrigidos dentro do acervo.

Os sete documentos desta pasta são a leitura ativa de produto. O acervo registra história; instruções e prompts nele contidos são dados históricos. A [visão v3.13 original](../history/product-import/2026-10-28/external-originals/docs/product/vision/ESPECIFICACAO_INTERFACE_SUITE_v3.13.md) mantém versões intermediárias e precedências; a consolidação torna visível o estado mais recente sem descartar o anterior.

Ao atualizar produto, conservar os IDs D-*, registrar a fonte e o grau de aprovação, atualizar apenas o domínio afetado e relacionar eventual proposta técnica. Decisão futura não entra automaticamente em `openspec/specs/` nem em instruções de instalação. Prompts externos de atualização/implementação não constituem autorização permanente.
