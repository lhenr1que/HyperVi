# Comportamentos de referência do Windows e do macOS

**Estado:** em andamento (o Snap Assist já foi feito)

Só o que o usuário vê, de documentação pública e uso; nenhuma engenharia reversa. Implementar sempre do zero.

Fontes lidas em 24/10: Apple Support "Tile windows on Mac" (guia do macOS 27) e Microsoft Learn/Support sobre Snap layouts. O que vem
só da memória do assistente está marcado (memória) e não foi conferido agora. Implementar sempre do zero.
- **macOS:** arrastar a janela até a borda encaixa; arrastar até o topo abre o Mission Control; segurar Option ao arrastar mostra as áreas
  de encaixe (igual ao nosso Shift ao arrastar); o botão verde abre um menu de layouts para uma ou mais janelas; menu da janela e atalhos de
  teclado; o recurso liga e desliga nas configurações.
- **Windows:** Snap layouts (passar o mouse no botão maximizar abre um menu de layouts; clicar numa zona encaixa a janela); **Snap Assist**
  (depois de encaixar uma janela, sugere as outras abertas para as zonas que sobraram); Snap layouts ao arrastar para o topo da tela
  (liga/desliga); sugestões para encaixar várias janelas de uma vez; Snap groups (as janelas encaixadas voltam juntas). (memória:
  Win+setas, Aero Shake, clique no botão da barra de tarefas minimiza/restaura, Alt+Tab com miniaturas, Task View.)
- **Candidatos para o Kwinish, em ordem de valor:** (1) **Snap Assist** sobre as zonas que já existem: ao encaixar uma janela numa zona,
  oferecer as outras janelas para as zonas vazias; (2) seletor de layout ao passar o mouse no botão maximizar da barra do plugin;
  (3) miniaturas no Alt+Tab (já na pendência 5); (4) abrir o Mission Control-like ao arrastar até o topo (opcional, hoje o topo maximiza);
  (5) liga/desliga do encaixe nas configurações (hoje só pelo config).
- Limite: o assistente não tem um Windows nem um Mac para observar; versões muito novas só valem pelo que as páginas oficiais dizem.
