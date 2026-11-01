# Projeto

A regra existente mantém a janela flutuante desde o primeiro quadro. Um timer curto, com endereço, passo e workspace capturados e estado revalidado, aplica o retângulo em coordenadas do monitor; a posição final depende de validação ao vivo. A origem da área de trabalho e a suspensão temporária da marca do modo usam etiquetas; ao restaurar em modo não flutuante, o núcleo devolve a janela ao mosaico. O timer de colunas relê a preferência, e zona reutiliza `target`.

O simulador não representa a semântica real de regras do compositor. Recarga de hooks, timers repetitivos e bindings capturados exige solução coordenada; o teste vermelho está preservado, mas nenhuma correção segura foi concluída nesta sessão. A comparação do módulo no timer novo impede que ele aja após reload, sem alegar que os demais timers estão corrigidos.
