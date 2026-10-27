# Design

## Context

`window_action` separa o sufixo `@<monitor>` em `mon` (tabela do monitor, ou `nil` se o nome não existir). O ramo `zone` chama `snap_assist` com `and not mon`. Como o plugin sempre manda o sufixo, o Assist nunca abre pelo arrasto.

## Goals / Non-Goals

**Goals:**
- Abrir o Assist no soltar no mesmo monitor.

**Non-Goals:**
- Assist no soltar em outro monitor (exigiria esperar a janela mudar de workspace e monitor antes de calcular as candidatas).
- Mudar o plugin.

## Decisions

- Trocar a condição por "sem monitor, ou monitor igual ao da janela" (comparando pelo nome, como o resto do núcleo). Alternativa: o plugin omitir o sufixo quando o monitor é o mesmo; rejeitada porque exige recompilar e recarregar o plugin e espalha a regra por duas camadas.

## Risks / Trade-offs

- [`w.monitor` ausente] → `snap_assist` já sai cedo sem `w.monitor`; a comparação usa a mesma guarda.
