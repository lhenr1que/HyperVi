# Proposta

## Por quê
A auditoria de 28/10 identificou perda de estado e ações atrasadas no ciclo de vida; a cascata também falhou ao vivo. A causa da regra redeclarada é hipótese, não evidência de comportamento do Hyprland 0.57.

## Mudanças
Preservar origem e proveniência por etiquetas, reler colunas no timer, conferir elegibilidade de zona, posicionar explicitamente a cascata após abertura e impedir callbacks antigos. A janela nasce flutuante pela regra; a posição em cascata pode ser ajustada depois da abertura.

## Capacidades
- `workspace-modes`: cascata e proveniência ao ocultar.
- `core-lifecycle`: restauração, colunas e recarga.
- `window-snapping`: elegibilidade de zona.

## Impacto
Núcleo e testes Lua; nenhum ajuste de opção, plugin ou UI. A recarga deixou de registrar de novo os ganchos de `setup()`.
