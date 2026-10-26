# Seletor em cruz dos modos de janela (HUD)

**Estado:** ideia, nada implementado

Menu circular de quatro quadrantes para trocar o modo do workspace: Cima = Tiling, Direita = Floating, Esquerda = Scrolling, Baixo = VXWM (apagado, "em breve"). Solta a tecla sobre o quadrante e a troca acontece com fade-out do HUD. A troca em si já existe (`kwinish.set_workspace_mode`, menu "Modo da área de trabalho").

## Avaliação do plano HDE colado pelo dono (26/10)
Visão: "Modular Hybrid Desktop Environment" com quatro modos (Tiling, Scrolling, Floating, e VXWM = canvas 2D infinito, v2.0), transições animadas entre
eles e um seletor em cruz (HUD) em Quickshell (Cima = Tiling, Direita = Floating, Esquerda = Scrolling, Baixo = VXWM "em breve").
- **Já existe**: os três modos por workspace (`core/modes.lua`, menu "Modo da área de trabalho"), com a geometria de cada janela guardada em etiquetas e
  restaurada ao voltar. O Hyprland já anima o movimento e o redimensionamento das janelas ao trocar de modo; **não é preciso um motor de interpolação
  próprio em C++**: basta ajustar as curvas/animações (`windowsMove`) na config.
- **Divergências com o código real**: o Kwinish hoje é núcleo Lua + UI Rust (GTK4 layer-shell) + plugin C++ só para barras e prévias, não um motor C++ de modos;
  o HUD poderia ser Rust/GTK (como o menu) ou QML se for parte do shell do Ryoku; o monorepo `hde-suite/` e os nomes (HDE, PKGBUILD-hde-desktop) não existem.
- **Conflito de atalhos**: o plano sugere `Super + Ctrl + setas`, mas no Ryoku elas já **redimensionam a janela** (`binds.lua` do dono) e `Super + Ctrl + Alt + setas` mandam a
  workspace para outro monitor. Escolher outra combinação.
- **VXWM**: precisa de uma câmera/zoom que o Hyprland não oferece nativamente para janelas (só zoom de cursor); é um projeto à parte.
- **Próximo passo sugerido**: o seletor em cruz (HUD) com três quadrantes ativos e o de baixo apagado, chamando `kwinish.set_workspace_mode`; atalho a decidir.
