# Tasks

## 1. Experimento no Hyprland aninhado (nada na sessão do dono)

- [x] 1.1 Criar `spike/nested-open/` com uma config mínima do Hyprland 0.56.2 (monitor único, dwindle) e um script que: abre esse Hyprland aninhado pela sessão do dono só com `hl.exec_cmd("Hyprland -c <cfg>", { workspace = "9 silent" })`; abre um kitty dentro dele; amostra `hyprctl -i <instância aninhada> clients -j` a cada ~15 ms por 1,5 s; fecha tudo. Verificar: o log mostra a primeira amostra da janela; `hyprctl clients -j` da sessão do dono não lista nenhum kitty nem Hyprland depois do script.
- [x] 1.2 Responder no aninhado às perguntas (a) e (b) do `design.md`, com um tratador de `window.open_early` na config aninhada que registra os campos da janela e, numa segunda rodada, faz `float enable` + `resize` + `move`. Verificar: log com a ordem dos eventos e a primeira amostra (flutuante ou não, tamanho), e se o compositor aninhado sobreviveu.
- [x] 1.3 Só se (a) ou (b) falharem: responder (c) no aninhado, criando por `hyprctl -i ... eval` uma `hl.window_rule` nomeada com `match.workspace` e depois redeclarando o mesmo nome com `enabled = false`, sem guardar nem chamar métodos no objeto devolvido. Verificar: a próxima janela nasce flutuante com a regra e em mosaico depois de desligada.
- [x] 1.4 Registrar as respostas e o caminho escolhido (2 ou 3 do `design.md`) numa seção "Resultado do experimento" no fim do `design.md`. Se nenhum caminho passar, parar aqui e levar o resultado ao dono.

## 2. Teste que falha

- [x] 2.1 Em `core/test_kwinish.lua`, fazer o simulador `hl` guardar as regras de `hl.window_rule` por nome (redeclarar substitui; `enabled = false` desliga) e aplicá-las ao abrir uma janela, antes do mosaico simulado (`match.workspace`, `match.float = false`; `float`, `size`, `move` relativo ao monitor, `tag` como etiqueta `nome*`), registrando a primeira geometria de cada janela; testar que uma janela aberta num workspace floating tem como primeira geometria um retângulo flutuante igual a `modes.open_rect`, com a etiqueta `kwinish-mode-floated`. Verificar que falha com o código atual (`lua core/test_kwinish.lua` sai ≠ 0).
- [x] 2.2 No mesmo arquivo, cobrir: duas janelas seguidas saem em cascata; diálogo que já abre flutuante fica sem etiqueta e não vai ao mosaico ao trocar para tiling; workspace num monitor fora da origem recebe o `move` relativo a ele; a regra é desligada ao trocar para tiling; workspace tiling continua em mosaico; o temporizador de 150 ms depois do caminho antecipado não move nem redimensiona de novo; trocar para tiling devolve ao mosaico as janelas que nasceram flutuantes. Verificar: os casos novos que dependem do conserto falham antes dele, os outros passam.

## 3. Conserto

- [x] 3.1 Implementar em `core/kwinish.lua` o caminho 3 conforme "Decisão final" do `design.md` (regra por workspace, marca `kwinish-mode-new` convertida em `kwinish-mode-floated`, cascata por redeclaração no `window.open`, `monitor.added`), reutilizando `modes.open_rect`/`work_area`, sem tocar no temporizador de 150 ms (rede de segurança). Verificar: `bash tools/check.sh` sai 0.
- [x] 3.2 Rodar o núcleo do repositório no Hyprland aninhado da tarefa 1.1 e confirmar com a amostragem que a primeira amostra de um kitty num workspace floating já é flutuante, 64% x 68%. Verificar: log no `spike/nested-open/` e resumo no `docs/STATUS.md` (teste no aninhado, não ao vivo).

## 4. Registro

- [x] 4.1 Em `docs/STATUS.md`, "Testes pendentes para o dono": roteiro completo em fish, com `fish ~/Downloads/ryoku-work/kwinish/tools/dev-reload.fish` (plugin e núcleo do repositório), abrir um aplicativo num workspace floating, o que deve acontecer (sem passar pela tela cheia) e como desfazer (`hyprctl reload`); o mesmo texto por inteiro na resposta do chat. Verificar: o roteiro usa só caminhos absolutos.
- [x] 4.2 Atualizar o backlog "janela nova sem piscar" em `docs/STATUS.md` e `docs/HANDOFF.md`, e `docs/CONFIGURAVEL.md` se o comportamento do modo floating estiver descrito lá. Verificar: `grep -n "piscar" docs/*.md` não mostra mais a pendência como aberta.

## Workflow follow-up

- Arquivar com `/opsx:archive` (ou `$openspec-archive-change` no Codex) depois do teste do dono.
