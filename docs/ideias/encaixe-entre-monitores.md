# Encaixe depois de arrastar entre monitores

**Estado:** corrigido no código em 26/10 (plugin envia `@monitor`, núcleo usa; teste simulado em `core/test_kwinish.lua`); **falta verificar com mouse real e dois monitores** (teste 3 em `docs/STATUS.md`). Sem Snap Assist nesse caso.

O plugin escolhe a zona pelo monitor do ponteiro, mas move a janela de volta a `m_dragStart` e o núcleo calcula o retângulo pelo monitor da janela: ao soltar no segundo monitor, o encaixe pode acontecer no monitor de origem.

## Para fazer
- Passar o monitor escolhido do plugin ao núcleo (`window_action(addr,"zone:N")` com o monitor) e usá-lo em `layout_rects`.
- Testar com mouse real e dois monitores (o dono tem DP-1 e HDMI-A-1).