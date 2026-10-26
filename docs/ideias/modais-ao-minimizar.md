# Janelas filhas e modais ao minimizar

**Estado:** ideia, pendência real

Hoje "minimizar" manda só a janela principal para `special:minimized`; os diálogos filhos ("Salvar como") ficam para trás. O KWin trata isso por `transientFor` (só como conceito: é GPL, nunca copiar código).

## Para fazer
- Descobrir se o Hyprland expõe a relação de janela-mãe (campo no objeto de janela ou no `hyprctl clients`).
- Minimizar e restaurar a mãe junto com as filhas, em sincronia.
- Testar com um app que abra diálogo.