# Tarefas

- [x] Reproduzir a regra de cascata obsoleta no simulador e posicionar a janela do modo após abrir.
- [x] Preservar a marca do modo em minimização e ocultação e restituir mosaico ao retornar para modo não flutuante.
- [x] Persistir as origens de mostrar área de trabalho em etiquetas.
- [x] Relê a escolha de colunas ao executar a atualização adiada.
- [x] Impedir que os ganchos de `setup()` sejam registrados de novo depois de recarregar o módulo: trava global `KWINISH_SETUP_HOOKS`, como os outros ganchos `KWINISH_*`, e opções em `KWINISH_SETUP_OPTS`, lidas na hora (orquestrador, 01/11/2026). Vermelho em `red-reload.log`, verde em `green-reload.log`. Os binds vêm da carga da config e os outros ganchos do núcleo já tinham trava global; o `switcher.setup` não tem trava, mas os ganchos antigos só atuam sobre o estado antigo, já fechado (limite registrado).
- [x] Aplicar a mesma elegibilidade de `tile` a `zone`.
- [x] Executar `bash tools/check.sh`: saída 0, registrada em `docs/port-0-57/evidence/fix-core-lifecycle-tests/check.log` (Lua e ferramentas simulados/isolados, 32 testes Rust, build do plugin).
- [ ] Dono: executar o roteiro ao vivo abaixo e registrar o resultado. Nenhum teste ao vivo foi executado nesta sessão.

## Roteiro ao vivo para o dono (bash)

Leia tudo antes de executar. Use um workspace de teste sem outras janelas importantes. Os comandos de `hyprctl` e abertura de janelas são somente para o dono. O roteiro modifica configurações do Kwinish; salve primeiro e restaure ao terminar. Os caminhos de estado abaixo pressupõem `XDG_CONFIG_HOME=/home/lhen/.config`; se a sessão usar outro diretório, substitua os quatro caminhos pelos correspondentes antes de executar. Confirme que nenhum plugin Kwinish está carregado; se houver, descarregue manualmente o caminho mostrado antes de continuar, como no roteiro GS.

```bash
hyprctl plugin list
bash /home/lhen/Projects/ryoku-work/kwinish-port-0-57/tools/live-backup.sh save /tmp/kwinish-lifecycle-backup /home/lhen/.config/kwinish/workspace-modes.json /home/lhen/.config/kwinish/workspace-modes.json.bak /home/lhen/.config/kwinish/workspace-columns.json /home/lhen/.config/kwinish/workspace-columns.json.bak
mkdir -p /tmp/kwinish-lifecycle
make -C /home/lhen/Projects/ryoku-work/kwinish-port-0-57/plugin TARGET=/tmp/kwinish-lifecycle/kwinish-bars.so
hyprctl plugin load /tmp/kwinish-lifecycle/kwinish-bars.so
hyprctl eval "package.path='/home/lhen/Projects/ryoku-work/kwinish-port-0-57/core/?.lua;'..package.path; for _,m in ipairs({'kwinish','geometry','layouts','assist','rules','modes','binds','switcher','settings','kwjson'}) do package.loaded[m]=nil end; require('kwinish')"
```

O build e o carregamento devem terminar sem erro. No workspace de teste, ative floating e abra duas janelas em sequência:

```bash
hyprctl eval "local w=hl.get_active_workspace(); require('kwinish').set_workspace_mode(w.id,'floating')"
kitty --class kw-lifecycle-1 &
kitty --class kw-lifecycle-2 &
```

Ambas devem nascer flutuantes, sem aparecer em mosaico; depois da abertura a segunda deve ficar deslocada em relação à primeira. Minimize uma pelo botão, restaure-a e mude para tiling com o comando abaixo: a janela que o modo fez flutuar deve voltar ao mosaico. Repita minimizando antes de mudar o modo e restaurando depois; também deve voltar ao mosaico.

```bash
hyprctl eval "local w=hl.get_active_workspace(); require('kwinish').set_workspace_mode(w.id,'tiling')"
```

Para a área de trabalho, ative floating novamente, invoque `show_desktop`, recarregue o módulo e invoque novamente. As janelas devem retornar ao workspace de origem; depois, ao mudar para tiling, as janelas reivindicadas pelo modo devem voltar ao mosaico.

```bash
hyprctl eval "local w=hl.get_active_workspace(); require('kwinish').set_workspace_mode(w.id,'floating'); require('kwinish').show_desktop()"
hyprctl eval "package.loaded.kwinish=nil; require('kwinish')"
hyprctl eval "require('kwinish').show_desktop()"
hyprctl eval "local w=hl.get_active_workspace(); require('kwinish').set_workspace_mode(w.id,'tiling')"
```

Para colunas, selecione scrolling e escolha duas colunas; abra ou feche uma janela e escolha rapidamente três colunas antes de 250 ms. A largura final deve ser um terço; desmarcar a escolha durante a espera não deve reaplicar a largura anterior. Para zona, ponha uma janela em tela cheia, peça uma zona pelo menu e confirme que sua geometria não muda. Para a recarga: rode duas vezes o comando de carga do núcleo acima e mude o foco entre duas janelas flutuantes; com `autoraise_ms` configurado, a janela sobe uma vez só (sem ganchos duplicados).

Para desfazer, feche as janelas de teste, restaure o backup e descarregue o plugin de teste; recarregue então a configuração normal:

```bash
bash /home/lhen/Projects/ryoku-work/kwinish-port-0-57/tools/live-backup.sh restore /tmp/kwinish-lifecycle-backup
hyprctl plugin unload /tmp/kwinish-lifecycle/kwinish-bars.so
hyprctl reload
```

Se a sessão travar, entre em outro TTY e use `hyprctl -i 0 plugin unload /tmp/kwinish-lifecycle/kwinish-bars.so`; restaure o backup quando possível. Não apague manualmente arquivos do dono.
