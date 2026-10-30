Veredito: **conditional**. A aprovação do código T1 permanece; o tratamento do minor 1 ainda deixa falsos verdes nas checagens de produção.

Reproduzi em memória, sem alterar arquivos:

1. **Minor — “sete funções incondicionais” não é garantido.**  
   [T1-probe-run.sh:30](/home/lhen/Projects/ryoku-work/kwinish-port-0-57/docs/port-0-57/evidence/T1-probe-run.sh:30) conta linhas e aceita qualquer combinação dos nomes permitidos:
   - inserir `if (false)` antes de `grab`, mantendo a indentação da chamada, conserva os dois contadores em **7**;
   - substituir `remember` por outro `forget`, incluindo o callback, também conserva **7/7**.  
   
   Portanto, a mutação condicional apresentada foi detectada pela forma textual escolhida, mas a propriedade anunciada continua sem garantia.

2. **Minor — expressão invertida ainda passa.**  
   [Linha 27](/home/lhen/Projects/ryoku-work/kwinish-port-0-57/docs/port-0-57/evidence/T1-probe-run.sh:27) procura uma substring. Trocar uma condição por `if (!(g_pInputManager->getModsFromAllKBs() & Input::eKeyboardModifiers::HL_MODIFIER_SHIFT))` mantém **duas formas encontradas e dois usos**. A sonda separada permanece inalterada.

3. **Minor — erro operacional do grep é aceito.**  
   [Linha 29](/home/lhen/Projects/ryoku-work/kwinish-port-0-57/docs/port-0-57/evidence/T1-probe-run.sh:29) usa `! grep`: tanto ausência de correspondência (**1**) quanto erro de leitura/percurso (**2**) viram sucesso. A guarda `-d` resolve diretório ausente, mas não erros dentro de um diretório existente. É necessário aceitar exclusivamente o status **1**.

Demais tratamentos:

- **Minor 2:** melhoria correta. O padrão agora exige a mensagem específica do oráculo; o log no disco registra essa assertiva nos dez mutantes. Continua sendo busca no diagnóstico inteiro, mas resolve o exemplo anterior de `#error static_assert`.
- **Minor 3:** registrar a limitação é adequado. Os arquivos `T1-build-*.log` ainda não estão no diretório consultado; não confirmei independentemente ausência de erros fatais/linker nem os totais. Isso permanece pendente documental.
- **Minor 4:** manter L05 como pendência fora da autorização é correto.

Para aprovar integralmente os tratamentos, falta fechar os três falsos verdes acima e preservar os logs prometidos. Nenhum achado novo no código de produção.

**HANDOFF PARA PRÓXIMA SESSÃO**

Revisão somente leitura concluída. Branch `port-kwinish-bars-to-hyprland-0-57`, HEAD `f76c785`; alterações preexistentes e não rastreados preservados. Nenhuma escrita, compilação ou ação no compositor; runner não executado integralmente. Evidências: análise do script/log e mutações textuais em memória. Próxima ação recomendada: fortalecer as checagens e repetir as mutações, mediante autorização de escrita. Incorporar este relatório ao registro de continuidade nessa etapa; L05 e logs brutos continuam pendentes.