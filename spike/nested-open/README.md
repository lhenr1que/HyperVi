# Experimento de abertura no Hyprland aninhado

Config mínima e executor da tarefa 1 de `fix-floating-mode-open`.
O único `eval` na sessão do dono abre `Hyprland -c <config temporária>` com
`workspace = "9 silent"`. Eventos ficam na config aninhada; regras e demais
chamadas usam explicitamente `hyprctl -i <assinatura aninhada>`.
Não carrega plugin nem altera `~/.config`.

Comandos (fish), cada saída deve ser uma pasta nova:

```fish
cd /home/ryo/Downloads/ryoku-work/kwinish-floating
python3 /home/ryo/Downloads/ryoku-work/kwinish-floating/spike/nested-open/run.py baseline --out /tmp/kwinish-baseline-nova
python3 /home/ryo/Downloads/ryoku-work/kwinish-floating/spike/nested-open/run.py early-mutate --out /tmp/kwinish-early-nova
python3 /home/ryo/Downloads/ryoku-work/kwinish-floating/spike/nested-open/run.py rules --out /tmp/kwinish-rules-nova
python3 /home/ryo/Downloads/ryoku-work/kwinish-floating/spike/nested-open/run.py rules-class --out /tmp/kwinish-rules-class-nova
python3 /home/ryo/Downloads/ryoku-work/kwinish-floating/spike/nested-open/run.py rules-size --out /tmp/kwinish-rules-size-nova
```

**Atenção: `early-mutate` derrubou o compositor aninhado em 0.56.2.**
Não use esse mecanismo na sessão principal. O executor limpa seus processos
no `finally`. Não guarda nem chama métodos de objetos retornados por regras/binds.

Resultados preservados em `results/`; resumo e limites da evidência no
`design.md` da mudança, seção “Resultado do experimento”.
O backend Wayland adotou o tamanho da superfície externa (1827×1000), não o
modo solicitado de 1280×800. Cada rodada amostra `clients -j` a cada ~15 ms
por 1,5 s. A primeira amostra não substitui a observação da ordem dos eventos.
Snapshots da sessão do dono guardam só endereço, classe, PID e workspace;
os kitties que já estavam abertos são preservados.
