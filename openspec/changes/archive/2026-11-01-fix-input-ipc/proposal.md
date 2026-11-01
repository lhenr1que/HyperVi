## Por quê

A entrada da barra, a comunicação do daemon e a escolha no Assist podem usar informação incorreta ou bloquear a interface. A textura de símbolos ignora a escala do monitor.

## O que muda

- Restringir presses à barra da janela sob o ponteiro.
- Limitar duração e tamanho das respostas ao daemon de barras.
- Recusar a escolha do Assist quando workspace, monitor ou layout mudar.
- Invalidar símbolos em escala diferente.

## Capacidades

- `title-bar`: entrada e escala dos símbolos.
- `snap-assist`: validade da escolha.
- `bars-ipc`: limites da leitura de respostas.
