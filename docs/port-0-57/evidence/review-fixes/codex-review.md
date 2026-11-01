A revisão identificou falhas no contexto do Snap Assist e na recarga do núcleo, além de lacunas nos testes de entrega. O veredito é **approve with fixes**.

**Achados confirmados**

1. **Major — [core/kwinish.lua:429](/home/lhen/Projects/ryoku-work/kwinish-port-0-57/core/kwinish.lua:429): escolha do Assist continua válida após trocar o workspace exibido.** A comparação verifica o workspace da janela candidata, mas não o workspace ativo no monitor onde o Assist foi apresentado. Se o usuário mudar de workspace enquanto o overlay está aberto e então escolher uma janela, ela ainda pode ser encaixada no workspace anterior, contrariando o cenário previsto na spec. **Correção:** verificar que o workspace ativo do monitor publicado ainda é `st.workspace`; adicionar teste que troca o workspace exibido sem mover a candidata.

2. **Major — [core/kwinish.lua:361](/home/lhen/Projects/ryoku-work/kwinish-port-0-57/core/kwinish.lua:361): o hook de posicionamento conserva o módulo antigo após reload.** O guard impede registrar outro hook, mas a closure original continua usando `work_area`, que consulta `M.bar_height` do primeiro módulo. Após recarregar `kwinish` e alterar `bar_height` no módulo novo, o posicionamento de novas janelas sem o plugin pode usar a altura antiga e sobrepor a barra fallback. **Correção:** fazer o hook encaminhar para uma função do `package.loaded.kwinish` atual e testar uma mudança de `bar_height` entre reloads.

3. **Minor — [tools/test-packaging.sh:25](/home/lhen/Projects/ryoku-work/kwinish-port-0-57/tools/test-packaging.sh:25): o teste de dependências aceita erro do Make como sucesso.** `make -q` retorna 1 quando o alvo está desatualizado, mas também pode retornar 2 por erro. O teste registra falha somente para retorno 0; assim, um Makefile quebrado pode passar por todos os testes de headers. **Correção:** exigir explicitamente retorno 1 para cada header e tratar 2 como falha.

4. **Minor — [tools/test-packaging.sh:69](/home/lhen/Projects/ryoku-work/kwinish-port-0-57/tools/test-packaging.sh:69): o teste não comprova que a falha de instalação é informada.** Ele só exige que a mensagem de sucesso esteja ausente. Um script que silenciosamente não instale nada e saia com zero também passa, embora o objetivo seja informar a falha. **Correção:** exigir a mensagem específica de falha e testar também falha na segunda instalação, da ABI.

**Suspeita**

- **Minor — [core/kwinish.lua:764](/home/lhen/Projects/ryoku-work/kwinish-port-0-57/core/kwinish.lua:764): o temporizador da cascata não verifica fullscreen.** Se uma janela reivindicada pelo modo entrar em fullscreen nos 30 ms entre a abertura e o callback, o código ainda solicita resize e move. Não confirmei o efeito desses dispatches no compositor ao vivo. Convém verificar fullscreen no callback e testar essa transição.

**Validação e estado:** executei o diff solicitado e fiz análise estática do código e de trechos do upstream Hyprland 0.57. Tentei `bash tools/check.sh`, mas ele parou antes dos testes: `mktemp` recebeu erro de sistema de arquivos somente leitura. Nenhum teste simulado, build ou teste ao vivo foi concluído nesta revisão. Não alterei arquivos. A branch é `port-kwinish-bars-to-hyprland-0-57`, HEAD `3d28f18ece9cc14d9af067089dd886b058e3a8be`; já havia alterações locais em `AGENTS.md`, `README.md` e quatro arquivos de `docs/`.

**HANDOFF PARA PRÓXIMA SESSÃO**

- **Objetivo e resultado:** revisão independente do diff concluída; quatro achados confirmados e uma suspeita, com veredito **approve with fixes**.
- **Decisões e pendências:** corrigir e testar os achados; verificar ao vivo ou por teste representativo a suspeita de fullscreen. Esta revisão não aprova implementação nem validação como concluídas.
- **Evidências:** referências de código e cenários acima; `tools/check.sh` bloqueado pelo `mktemp` em filesystem somente leitura. Nenhum arquivo de continuidade foi editado nesta investigação.
- **Próxima ação recomendada:** reproduzir os casos em testes, corrigir as falhas e executar `bash tools/check.sh` em ambiente que permita temporários.
- **Limites de autorização:** esta sessão autorizou somente leitura e a tentativa da validação indicada; correções e testes ao vivo exigem escopo autorizado na próxima etapa.