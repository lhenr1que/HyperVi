# Brief: gerenciador de janelas estilo KWin para qualquer Hyprland

## Objetivo
Planejar (e, só depois de aprovado, construir) um programa de gerenciamento de janelas que traz comportamentos do KWin
para o Hyprland. O diferencial: ele precisa ser instalável em **qualquer** Hyprland, não só no Ryoku.

## Leia primeiro (em `ryoku-work/`)
- `kwin-vs-hyprland-marcado.md`: catálogo de 62 recursos do KWin comparados com o Hyprland e o Ryoku, com a coluna
  **Quero?** marcada pelo dono. É a lista de escopo.
- `kwin-vs-hyprland.md` e `REVISAO-GEMINI-KWIN.md`: o levantamento original e uma revisão dele.
- `ORQUESTRACAO.md`: o registro das tarefas já feitas no Ryoku (T1 no_warps, T4 mostrar a área de trabalho, T5 snap em
  todos os layouts, T9 menu da janela, e T7 Alt+Tab, que NÃO foi resolvido; o registro explica as duas tentativas e o
  problema das solturas de tecla dentro de submaps no Hyprland 0.56).
- Essas tarefas estão em `ryoku-fork` (branch `caelestia-port`): Lua em `ryoku/hyprland/` e a camada neutra de compositor
  em `ryoku/wm/`. Leia como foram feitas, mas o programa novo não pode depender do Ryoku.

## Primeira entrega: o plano, por escrito, aqui em `ryoku-work/`
1. Quais recursos marcados entram numa primeira versão e quais ficam para depois.
2. A arquitetura: o que roda dentro do Hyprland (config Lua, dispatchers, eventos), o que precisa de um processo à parte
   (daemon lendo o socket de eventos do Hyprland) e o que precisa de UI (Quickshell ou outra), e por que cada peça.
3. Como se instala num Hyprland qualquer (pacote, script, plugin `hyprpm` ou só config), sem pressupor Ryoku, Quickshell
   ou uma versão específica além da mínima que você justificar.
4. Riscos: Alt+Tab com soltura de tecla, diferenças entre versões do Hyprland, e outros que encontrar.

Pare aí e espere o ok do dono antes de escrever código. Nada de push, fork ou PR sem aprovação explícita.
Pronto quando: o plano estiver aprovado; depois, a primeira versão funcionando no desktop do dono.

## Contexto e restrições
- Máquina: Ryoku Linux (Arch + Hyprland 0.56 com config em Lua), 1920x1080, usuário `ryo`.
- O agy (Gemini via Antigravity) está sem cota desde 23/10; volta por volta de 28/10.
- Há um desktop só, usado pelo dono. Os testes na tela são em fila, com `grim` para captura e
  `ryoku-vm/tools/pointer-path.py` para cursor e cliques (`c` clica, `d`/`u` apertam e soltam para arrastar).
  Nunca mova as janelas do dono; teste com uma janela descartável.
- Dispatchers do Hyprland em Lua: `hyprctl dispatch 'hl.dsp.focus({ workspace = 2 })'`.
- Commits: `git -c user.name=lhen -c user.email=lhenriqueds99@gmail.com commit`.
