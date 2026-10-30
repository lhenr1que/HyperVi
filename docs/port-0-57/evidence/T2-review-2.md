**Veredito final: approve**, restrito à T2, grupo W.

Não encontrei achados remanescentes classificados como **blocker, major, minor ou nit**. Os três minors da rodada 1 foram resolvidos.

1. **Runner sem o falso positivo identificado na r1.** Em [T2-probe-run.sh:43](docs/port-0-57/evidence/T2-probe-run.sh:43), erro de compilação registra `FAIL` e retorna antes da contabilização. Nas linhas 48–51, relatório de sanitizer ou retorno inesperado também falham; morte exige exit 1 e linha de assertiva `FAIL S`. O teste emite essas linhas em [T2-hostfree.cpp:77](docs/port-0-57/evidence/T2-hostfree.cpp:77).  
   **Reprodução realizada:** leitura do fluxo e contagem dos logs existentes: verde com **35 OK**, **14 mortes comportamentais e 2 estruturais**; vermelho do HEAD com **zero ocorrências de `killed`**, contadores zerados e `FAILURES`.

2. **Contabilidade corrigida.** [T2-probe-run.sh:55](docs/port-0-57/evidence/T2-probe-run.sh:55) distingue morte com destrutor representativo daquela exclusiva da variante sem `erase`. O log confirma **7 mutações comportamentais × 2 compiladores**, mais **MUT1 estrutural × 2**. O `[[maybe_unused]]` em [T2-hostfree.cpp:43](docs/port-0-57/evidence/T2-hostfree.cpp:43) resolve exclusivamente o aviso da variante sem uso.  
   **Reprodução:** MUT1 aparece com `real-dtor:ALL-OK` e `no-erase:FAIL S6` em [T2-probe-run.log:15](docs/port-0-57/evidence/T2-probe-run.log:15).

3. **Relatório restaurado e preciso para o escopo.** [T2-W-findings.md:44](docs/port-0-57/evidence/T2-W-findings.md:44) recupera caminhos, símbolos e implementação. As linhas 17, 24–29 e 51 distinguem predicado corrente, opção A aprovada, `m_self` fraco, publicação em `bars` e limites do host-free. Preserva também a ressalva de reentrância e o build ainda incompleto.  
   **Reprodução:** conferência dessas passagens contra o runner, fake, código e logs no disco.

Não identifiquei problema adicional no diff de produção: [plugin/main.cpp:39](plugin/main.cpp:39) mantém a referência forte local, configura `m_self` antes da chamada e publica somente após sucesso. A inspeção das chamadas do host e callbacks da barra continua sustentando a nova ordem.

Esta rodada realizou **análise estática e contagem dos logs por Python**. Não reexecutei sondas, testes simulados, builds ou testes ao vivo; os resultados desses testes são evidências previamente registradas.

**HANDOFF PARA PRÓXIMA SESSÃO**

- **Resultado:** revisão independente r2 concluída; T2/W aprovada tecnicamente.
- **Git:** branch `port-kwinish-bars-to-hyprland-0-57`, HEAD `dc4f48a312a7be99a2ae1b26a3166c734835c621`; alterações preexistentes em `plugin/main.cpp`, `docs/DEVELOPMENT_STATE.md` e arquivos não rastreados preservadas. Nenhuma escrita ou commit realizado.
- **Decisões:** opção A e diferenças D1/D2 permanecem aprovadas pelo dono. Aprovação técnica desta revisão não autoriza commit.
- **Pendências e riscos:** build completo e validação ao vivo continuam pendentes; D/R permanecem fora do escopo.
- **Próxima ação:** incorporar este parecer em `docs/DEVELOPMENT_STATE.md` quando houver autorização de escrita e obter decisão do dono sobre o fechamento/commit da T2. Esta sessão autoriza somente leitura.