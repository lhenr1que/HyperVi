## Implementação e validação

- [x] Testar e corrigir o press na janela superior.
- [x] Testar e limitar leitura do socket das barras.
- [ ] Testar e revalidar o contexto do Assist na escolha (pendente de coordenação em `core/kwinish.lua`).
- [x] Testar e corrigir a escala da textura de símbolo.
- [x] Executar `tools/check.sh` e compilar o plugin com GCC e Clang, registrando as saídas.
- [ ] O dono executa o roteiro ao vivo abaixo.

## Roteiro ao vivo — somente o dono

Antes de começar, `hyprctl plugin list` não deve listar Kwinish. Se listar, descarregue o caminho mostrado. Execute cada comando separadamente:

```bash
hyprctl plugin list
mkdir -p /tmp/kwinish-fix-input-ipc
make -C /home/lhen/Projects/ryoku-work/kwinish-port-0-57/plugin TARGET=/tmp/kwinish-fix-input-ipc/kwinish-bars.so
hyprctl plugin load /tmp/kwinish-fix-input-ipc/kwinish-bars.so
hyprctl eval "package.path='/home/lhen/Projects/ryoku-work/kwinish-port-0-57/core/?.lua;'..package.path; for _,m in ipairs({'kwinish','geometry','layouts','assist','rules','modes','binds','switcher','settings','kwjson'}) do package.loaded[m]=nil end; require('kwinish')"
kitty --class kwt8 & kitty --class kwt8 &
```

As duas janelas devem aparecer com barras. Sobreponha-as, deixando a de trás focada, e aperte onde sua barra está coberta: somente a janela superior deve receber o clique; repita em título, ícone e botões. Em monitores com escalas diferentes, mova as janelas entre eles e confira que os símbolos permanecem nítidos e com o tamanho correto. O Assist ainda não foi corrigido e não deve ser considerado validado. O teste automatizado cobre o socket; o roteiro não simula um compositor travado.

Para desfazer, feche as janelas de teste, descarregue o plugin e recarregue a configuração:

```bash
kill $(hyprctl clients -j | jq '.[] | select(.class == "kwt8") | .pid') 2>/dev/null
hyprctl plugin unload /tmp/kwinish-fix-input-ipc/kwinish-bars.so
hyprctl reload
```
