# Revisão do Gemini sobre as adaptações do KWin (triada pelo Claude)

Relatório bruto: `/tmp/claude-1000/-home-ryo-Downloads/91cb420f-af62-47de-ba0e-705271e5d715/scratchpad/relatorio-limpo.md` (14 achados).
Revisor: Antigravity `gemini-4-argon-high`, só leitura, numa cópia isolada. Triagem: cada achado foi conferido no código.

| # | Achado do Gemini | Triagem | Ação sugerida |
|---|---|---|---|
| 1 | `no_warps` só em `genConfig`, falta em `fullConfigLua` (aplicação ao vivo) e fora do layout flutuante | **Confirmado** (config_settings.go:417 e :1026) | Corrigir: emitir nos dois lugares |
| 2 | Restaurar leva a janela ao workspace ativo, não ao de origem; clicar no ícone da janela ativa não minimiza | **Parcial**: o restaurar é assim por escolha minha anterior; falta a minimização por clique | Decidir com o dono |
| 3 | Encaixe na borda dispara com o botão ainda pressionado (timer 60 ms); sem cantos; ignora a barra; dois estados de "maximizado" | **Confirmado** o código (binds.lua:196-217; `ryoku_snap_grab` aborta com fullscreen; "full" deixa 6 px) | Corrigir: unificar maximizado, cantos, `m.reserved` |
| 4 | `ryoku_show_desktop` perde o mapa de origem no reload; abrir app nova com o desktop à mostra restaura as antigas | **Confirmado** (`came_from` é local, binds.lua:67) | Corrigir: guardar a origem como tag |
| 5 | `Maximize horizontally/vertically` e `Grow` no menu da janela não fazem nada fora do layout flutuante | **Confirmado** (`floatSnapLua` só em `genFloatAll`) | Corrigir: emitir sempre |
| 6 | Cascata anda para a esquerda; janelas abrem em 62×72% | **Rejeitado**: a cascata para a esquerda foi decisão do dono (HANDOFF). O tamanho é por desenho | Nenhuma |
| 7 | Alt+Tab sem teclado (Esc, setas), solta só no `Alt_L`, miniaturas estáticas, sem Alt+crase | **Confirmado** (keyboardFocus None; só `Alt_L`) | Corrigir: Esc/setas e `Alt_R` |
| 8 | Quick tile e Grow colidem; `Super+Baixo` minimiza | **Rejeitado em parte**: minimizar com `Super+Baixo` é o comportamento do KDE e do Windows | Nenhuma |
| 9 | Menu da janela: Maximizar ignora o alvo; sem Mover/Redimensionar/Pin | **Confirmado** (WindowMenu.qml:31 usa a janela em foco) | Corrigir o alvo; Mover/Redimensionar depois |
| 10 | Zoom reseta no reload; inverter apaga o filtro de daltonismo | **Não verifiquei** | Verificar antes |
| 11 | Regras de janela: 1 propriedade por regra, `size/move` só inteiros | **Não verifiquei** | Verificar antes |
| 12 | Barra Caelestia: cliques inertes, `skipTaskbar` ignorado, dock sem menu | **Parcial**: cliques já tratados nesta sessão (áudio abre); o resto é verdade | Tratar na barra |
| 13 | Prévias com captura de 1 quadro | **Conhecido**: foi uma correção de crash, não um defeito | Nenhuma |
| 14 | Ordem de foco do Alt+Tab começa vazia | **Não verifiquei** | Verificar antes |

Os números de linha estavam corretos nos achados que conferi. O Gemini não inventou comportamentos do KDE que eu possa comprovar, mas errou ao contrariar duas decisões do dono (6 e 8).
