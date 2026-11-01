## Implementação e validação

- [x] Testar e corrigir o press na janela superior.
- [x] Testar e limitar leitura do socket das barras.
- [x] Testar e revalidar o contexto do Assist na escolha (orquestrador, depois de juntar `fix-core-lifecycle`): `assist.json` passa a trazer `workspace` e `layout`; o overlay manda `assist:<geração>`; o núcleo só põe a janela se geração, janela, workspace, monitor, layout e retângulo da zona ainda conferirem. Vermelho em `red-assist.log`, verde em `green-assist.log`; teste Rust `pick_is_revalidated_by_the_core`.
- [x] Testar e corrigir a escala da textura de símbolo.
- [x] Executar `tools/check.sh` e compilar o plugin com GCC e Clang, registrando as saídas.
- [x] O dono executa o roteiro ao vivo abaixo.

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

As duas janelas devem aparecer com barras. Sobreponha-as, deixando a de trás focada, e aperte onde sua barra está coberta: somente a janela superior deve receber o clique; repita em título, ícone e botões. Em monitores com escalas diferentes, mova as janelas entre eles e confira que os símbolos permanecem nítidos e com o tamanho correto. Assist: com o layout `thirds` (SUPER+T), encaixe um `kwt8` flutuante na zona 1 arrastando-o pelo título até a borda esquerda com Shift; com o overlay do Assist aberto, troque de layout (SUPER+T, escolha outro) e só então clique no outro `kwt8` no overlay: ele não deve ser encaixado. Repita sem trocar nada: ele vai para a zona mostrada. O teste automatizado cobre o socket; o roteiro não simula um compositor travado.

Para desfazer, feche as janelas de teste, descarregue o plugin e recarregue a configuração:

```bash
kill $(hyprctl clients -j | jq '.[] | select(.class == "kwt8") | .pid') 2>/dev/null
hyprctl plugin unload /tmp/kwinish-fix-input-ipc/kwinish-bars.so
hyprctl reload
```

## Arquivamento (01/11/2026)

Arquivada à mão (CLI `openspec` não instalada) depois de o dono relatar, em 01/11/2026, que os testes ao vivo CL e II do `docs/STATUS.md` passaram. Os deltas foram aplicados aos specs principais com o texto copiado como está (MODIFIED substituídos, ADDED no fim, capabilities novas com Purpose escrito no arquivamento). Conferência estrutural: `tools/test-openspec-specs.sh`, que passou a conferir também esta mudança.
