## Abordagem

O press e a roda do mouse passam a chamar `inputIsValid(true)`, como a dica já fazia: só a barra cuja janela está de fato sob o ponteiro (hit-test do host) aceita a entrada; antes, a barra da janela focada aceitava mesmo coberta. Os símbolos dos botões são compartilhados por todas as barras, então guardam uma textura por escala de monitor (`iconTex[scale]`); invalidar a textura única a cada troca de escala a refaria a cada quadro com janelas em monitores de escalas diferentes (revisão do orquestrador, 01/11/2026). O socket tem prazo total e limite de bytes; respostas incompletas ou excessivas são descartadas.

A validação do Assist deve ocorrer no núcleo no momento de aplicar a ação, incluindo contexto capturado na publicação. A entrada atual de publicação em `core/kwinish.lua` não fornece workspace nem layout; sua alteração precisa ser coordenada com o agente que edita esse arquivo. Até haver uma solução demonstrada por teste, este item permanece pendente.
