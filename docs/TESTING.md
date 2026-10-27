# Testes do Kwinish

## Comando único
```sh
bash tools/check.sh
```
Roda, nesta ordem, e continua depois de uma falha para mostrar todas:
1. cada `core/test_*.lua` com `lua` (Lua 5.5, a que o Hyprland instalado usa: `readelf -d /usr/bin/Hyprland` mostra `liblua.so.5.5`);
2. cada `tools/test-*.sh` (os testes do próprio executor, do isolamento das suítes e do `live-backup.sh`);
3. `cargo test --locked --manifest-path ui/Cargo.toml`;
4. `make -C plugin` com saída num diretório temporário.

Código de saída: **0** tudo passou; **1** alguma falha, ferramenta ausente ou nenhum teste encontrado numa etapa; **2** uso errado. Ferramenta ausente é falha, nunca etapa pulada. Com os caches do cargo prontos, leva uns 20 s (13 s são do plugin).

## Validação parcial
- `bash tools/check.sh --lua-only`: só as suítes Lua; é o que o `check()` do `packaging/PKGBUILD` roda.
- `LUA=lua5.4 bash tools/check.sh`: outro interpretador, para comparar.
- Uma suíte só: `lua core/test_switcher.lua`. Rodar direto é seguro: cada suíte guarda seus arquivos num temporário próprio, e `tools/test-lua-isolation.sh` falha se alguma gravar no HOME/XDG herdado.
- Os testes do executor: `bash tools/test-check.sh`.

## Isolamento
O `check.sh` dá aos testes `HOME` e `XDG_*` temporários, apagados ao sair, e remove `HYPRLAND_INSTANCE_SIGNATURE`, `WAYLAND_DISPLAY`, `DISPLAY` e `DBUS_SESSION_BUS_ADDRESS`: pelo ambiente, nenhum teste acha o compositor, a config do dono ou o `switcher.json` da sessão. `CARGO_HOME` e `RUSTUP_HOME` continuam apontando para o HOME real (registro e toolchains já baixados). O plugin é compilado no temporário: nunca reescreve um `plugin/kwinish-bars.so` que o compositor possa ter carregado, e sempre compila de fato (o `Makefile` não acompanha os `.hpp`).
Não é uma sandbox de segurança: um teste ainda pode ler caminhos absolutos e usar o `/tmp`.

## Pré-requisitos (Arch)
`lua`, `bash`, `coreutils`, `cargo` (pacote `rust`), `gtk4`, `gtk4-layer-shell`, `make`, `gcc`, `pkg-config` e os cabeçalhos do Hyprland (pacote `hyprland`, mesma versão do compositor). Os 18 testes Rust não precisam de tela.

## Testes ao vivo
O `check.sh` não cobre o desktop real: carregar o plugin, mouse real e vários monitores são do dono (`docs/STATUS.md`, "Testes pendentes para o dono"). Todo roteiro começa por `fish tools/dev-reload.fish` (plugin e núcleo do repositório). Se o teste muda arquivo do dono, guarde antes e desfaça depois:
```fish
bash tools/live-backup.sh save ~/.local/state/kwinish-backup/<teste> <caminho absoluto>...
bash tools/live-backup.sh restore ~/.local/state/kwinish-backup/<teste>
```
O `restore` devolve o arquivo (conteúdo e modo), o link (mesmo quebrado) ou a ausência; remove só arquivos ou links que o teste criou; recusa um backup incompleto.

## Acrescentar testes
- Lua: `core/test_<nome>.lua`, descoberto sozinho. Se a suíte toca o disco, redirecione `os.getenv`/`io.open`/`os.execute` para um temporário, como `core/test_kwinish.lua` e `core/test_switcher.lua`.
- Shell: `tools/test-<nome>.sh`, descoberto sozinho; saia com código ≠ 0 em qualquer falha.

## Integração contínua (ainda não configurada)
O job de CI será `bash tools/check.sh` num contêiner Arch com os pré-requisitos acima. O código de saída é o resultado; nada mais precisa ser analisado.
