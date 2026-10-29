# Settings da suíte Arch — Arquitetura funcional consolidada

**Versão:** 1.0 · **Data:** 28/10/2026 · **Status:** design de produto, sem implementação ou testes nesta rodada  
**Produto:** nome público em aberto · **Base:** gerenciador independente para Hyprland + rice oficial + futura Rice Store de rices completos  
**Finalidade:** referência funcional para revisão da arquitetura existente e planejamento via OpenSpec/Codex; não especifica alterações já realizadas no repositório.

## 1. Visão e limites

O **Settings fundamental** acompanha o gerenciador e não depende do rice ativo. A aplicação atual em **Rust/GTK4 será reaproveitada e reorganizada**, preservando os editores e interfaces já existentes, após inventário técnico. O núcleo Lua, plugin C++ e Hyprland continuam responsáveis pelas políticas e pelo estado efetivo de janelas, workspaces, monitores e decorações. Rices externos completos podem possuir seu próprio Settings e não são obrigados a adotar a dock, launcher ou toolkit do rice oficial.

Esta especificação consolida decisões aprovadas **D-23–D-27** e respeita as anteriores: rice completo sem mistura de componentes (D-06), independência do gerenciador (D-08), recursos do rice oficial (D-11–D-22), exclusão do dashboard (D-07) e exclusão do VXWM do escopo inicial (D-10).

## 2. Registro normativo das decisões aprovadas

| ID | Decisão aprovada | Implicação funcional |
|---|---|---|
| **D-23** | Sidebar adaptativa | As páginas de Sistema persistem; Interface mostra controles do rice oficial ou `Rice ativo` para rice externo. |
| **D-24** | Padrão de modo global com exceções por monitor | Novos workspaces usam exceção aplicável ao monitor ou padrão global; existentes preservam modos próprios. |
| **D-25** | Perfis de monitor híbridos | Aplicação manual inicialmente, com automação opcional somente após identificação e reversão confiáveis. |
| **D-26** | Delegar controle de inatividade ao rice quando funcional | Não executar controladores automáticos concorrentes para uma mesma função. |
| **D-27** | Fallback da suíte para idle e bloqueio | Complementar ou assumir funções ausentes no rice; delegar componentes próprios verificados. |

**Não foram aprovados automaticamente:** binários de fallback, schema de manifesto, intervalos de timeout, dimensões visuais, prioridades de roadmap, controles não auditados e política de falha ao vivo.

## 3. Arquitetura da informação

```text
SETTINGS
├── SISTEMA (independente do rice)
│   ├── Janelas                        [essencial]
│   ├── Workspaces                     [essencial]
│   ├── Monitores                      [essencial]
│   ├── Energia e bateria              [essencial; cobertura progressiva]
│   ├── Atalhos                        [essencial]
│   ├── Diagnóstico e recuperação      [essencial]
│   ├── Som e microfone                [expansão proposta]
│   ├── Rede e internet                [expansão proposta]
│   ├── Bluetooth e dispositivos       [expansão proposta]
│   ├── Mouse, teclado e touchpad      [expansão proposta]
│   ├── Aplicativos e inicialização    [expansão proposta]
│   ├── Idioma, região e data          [expansão proposta]
│   └── Privacidade e segurança        [expansão proposta]
└── INTERFACE (sidebar adaptativa)
    ├── Rice oficial ativo: Barra · Dock · Launcher · Aparência
    └── Rice externo ativo: Rice ativo · Configurações próprias (quando disponíveis)
```

A quantidade de páginas pode exigir busca e agrupamentos na sidebar; não converter essa proposta de navegação visual em decisão sem revisão. Não esconder recursos fundamentais quando o rice for trocado. Preservar as preferências do rice oficial enquanto estiver inativo.

## 4. Modelo de responsabilidades (proposta técnica)

| Domínio | Fonte da verdade / executor | Responsabilidade do Settings |
|---|---|---|
| Janelas, workspaces e regras | Núcleo Lua existente + Hyprland IPC | Editar por contrato existente; observar confirmação de estado. |
| Decorações e ações de título | Plugin C++ existente | Expor somente capacidades implementadas e compatíveis. |
| Monitores | Hyprland e mecanismo de perfis | Ler saídas/modos reais; aplicar prévia e reversão segura. |
| Áudio | PipeWire/WirePlumber | Selecionar volume/dispositivos via API existente. |
| Rede | NetworkManager, quando presente | Consultar e editar conexões suportadas. |
| Bluetooth | BlueZ, quando presente | Gerenciar estado e pareamento compatíveis. |
| Bateria | UPower/hardware | Informar presença, carga e dados suportados. |
| Perfil energético | Serviço de perfis disponível (por exemplo, Power Profiles D-Bus) | Consultar valor efetivo e alterar apenas perfis anunciados. |
| Suspensão/hibernação/ações de sessão | systemd-logind e mecanismos autorizados | Solicitar ação; apresentar suporte/inibidores/resultado. |
| Inatividade/DPMS | Idle manager da suíte **ou** do rice | Mostrar proprietário por ação; impedir concorrência; configurar só via contrato real. |
| Bloqueio/autenticação | Locker do rice **ou** fallback compatível | Solicitar bloqueio; observar estado; não implementar senha/autenticação próprios. |
| Preferências do rice externo | Settings do rice | Mostrar link e capacidades; não sobrescrever arquivo de terceiro. |

A interface não deve possuir uma cópia isolada do “estado real”: o Settings observa o serviço, o compositor ou o controlador efetivo. Cache deve ser versionado/invalidado quando necessário.

## 5. Requisitos funcionais por página

### 5.1. Janelas

- Exibir modo efetivo do workspace ativo (Tiling, Scroll, Floating), com possibilidade de selecionar workspace alvo antes de alterar.
- Reutilizar editores existentes de layout, atalhos relacionados, Alt+Tab, Snap Assist e regras, em vez de reimplementá-los.
- Expor minimização/restauração, foco, exceções, encaixe e decorações somente conforme o núcleo/plugin suportarem.
- Tratar diferença entre floating manual e transição automática de modo; preservar proveniência/geométrica onde implementado.
- Estado de aplicação observado, não apenas comando enviado; não incluir VXWM nesta fase.

### 5.2. Workspaces

- Visualizar workspaces reais por monitor, modo efetivo, foco e janelas; não apresentar especiais internos como workspaces comuns.
- **D-24:** para workspace sem modo prévio, aplicar exceção do monitor, se houver, senão padrão global; workspace existente mantém modo atribuído.
- Diferenciar `primeira criação`, `workspace restaurado` e `workspace reassociado após hotplug`.
- Revalidar alvo após eventos externos; acompanhar origem de janelas minimizadas para integração com dock contextual.

### 5.3. Monitores

- Exibir identidade de saída presente/conhecida, resolução, taxa, escala e posição se suportadas.
- Criar/editar/ativar perfis sem presumir conexão simultânea de saídas.
- **D-25:** manual por padrão; automação opt-in futura condicionada à identificação confiável e ao rollback independente da UI.
- Alterações de risco: snapshot íntegro → validação → aplicar temporariamente → confirmação com teclado → persistir ou reverter. Se mecanismo de reversão não existir, bloquear operações potencialmente incapacitantes.

### 5.4. Energia e bateria

- Distinguir perfil energético, bloqueio por inatividade, desligamento da tela, suspensão, hibernação e ações manuais.
- Mostrar controlador responsável **por função** e sua disponibilidade; indicar quando configuração for delegada.
- Tempos/ações editáveis somente se o serviço responsável fornecer integração verificável.
- Hibernação e suspend-then-hibernate somente se capacidades e configuração real permitirem.
- Respeitar inibidores de mídia/jogos/apresentação conforme suportados; diferenciar AC e bateria só em dispositivos aplicáveis.
- Em rice externo, mostrar link `Abrir configurações do rice` apenas quando utilizável.

### 5.5. Atalhos

- Reutilizar editor existente; busca por ação, edição, associação ao gerenciador/rice/usuário e análise de conflito observável.
- Não prometer identificar atalhos privados de rice sem API; `desconhecido` é status legítimo.
- O atalho do launcher `Super+Espaço` continua proposta, não decisão.

### 5.6. Diagnóstico e recuperação

- Hyprland, Lua, plugin C++, Settings, rice ativo e serviços de desktop: exibir `operacional`, `incompatível`, `não carregado`, `falha`, `desconhecido` segundo testes reais.
- Detectar controladores de idle sobrepostos e serviço locker ausente; logs com dados sensíveis omitidos.
- Backup/restauração de preferências somente com prévia dos efeitos; caminho de recuperação mesmo sem shell oficial.

### 5.7. Interface oficial ou Rice ativo

- Barra, Dock, Launcher, Aparência: páginas específicas apenas do rice oficial; obedecer D-11–D-22, inclusive dock contextual/agrupamento/preview híbrida e launcher split/global/posição configurável.
- Rice externo: identificação/compatibilidade/dependências quando conhecidas, link para Settings próprio e retorno seguro ao rice oficial; nenhum editor fictício.

### 5.8. Extensões propostas

Som, redes, Bluetooth, entrada, aplicativos padrão/autostart, idioma/data, privacidade e segurança podem reutilizar serviços do Linux existentes. Priorizar integrar telas e serviços maduros em vez de desenvolver infraestruturas paralelas. Determinar dependências mínimas e experiências de ausência de serviço antes de considerar cada página parte do MVP.

## 6. D-26 e D-27 — modelo de composição de inatividade e bloqueio

**Princípio aprovado:** a suíte garante a *oferta de fallback* compatível e delega ao rice as funções que este fornecer e que estiverem efetivamente verificadas. A disponibilidade de fallback é requisito de produto, **não uma afirmação de que o sistema instalado já está seguro**.

| Idle efetivo do rice | Locker efetivo do rice | Composição recomendada |
|---|---|---|
| Sim | Sim | Usar componentes do rice; fallback da suíte não disputa as funções. |
| Sim | Não | Idle externo + locker da suíte somente após validar ligação entre eles. |
| Não | Sim | Idle da suíte + locker externo somente após validar ligação entre eles. |
| Não | Não | Idle + locker da suíte se disponíveis e verificados. |
| Desconhecido/falhando | Qualquer | Diagnóstico e recuperação controlada; não lançar controlador rival às cegas. |

Separar também DPMS, suspensão automática e bloqueio: um rice pode gerenciar alguns gatilhos, não todos. O contrato é por capacidade/ação, não por pacote ou nome do rice. Não tomar `loginctl lock-session` emitido como prova de que a sessão está bloqueada; bloqueio efetivo exige evidência do locker e do compositor. Quando política exigir lock antes de suspender, confirmar lock antes da sequência de suspensão, respeitando o mecanismo de inibição. Não fingir proteção se o locker falhar ou faltar configuração.

**Alternativas tecnológicas em análise:** hypridle para gestão de inatividade e hyprlock para bloqueio seguro, aproveitando protocolos de sessão e autenticação existentes. Outros serviços compatíveis poderão ser adotados. Não escrever autenticação personalizada.

### 6.1. Máquina de estados funcional (proposta)

```text
DESCONHECIDO
  -> DETECTANDO [inventário do controlador do rice e da suíte]
  -> VALIDANDO [capacidade e ligação idle/locker]
  -> PRONTO:GERIDO_PELA_SUÍTE [fallback verificado]
  -> PRONTO:DELEGADO_AO_RICE [externo verificado]
  -> PRONTO:MISTO [capacidade dividida e testada]
  -> DEGRADADO [uma função deixou de operar]
  -> CONFLITO [automações concorrentes]
  -> INDISPONÍVEL [falta suporte/autorização/serviço]
```

No diagnóstico, a função concreta deve ter sua própria condição; o status agregado nunca deve esconder uma falha de bloqueio. Transições para `pronto` requerem verificações observáveis. Falhas que não possam ser recuperadas automaticamente devem ser explicadas, e não ocultadas.

### 6.2. Handoff entre rices (proposta)

1. Inventariar gestor atual, serviço/locker, inibidores e comandos de bloqueio.
2. Conferir capacidades declaradas do rice alvo e serviços realmente presentes.
3. Preparar configuração de proprietário para cada função, sem sobrescrever arquivos do rice.
4. Ativar em transição controlada, preferencialmente na próxima sessão no primeiro produto; não deixar dois gestores efetivos para a mesma política.
5. Validar estado após ativação, oferecer rollback e manter diagnóstico acessível.
6. Revalidar depois de falhas/reloads/suspensão/hotplug; não presumir fallback funcional sem teste.

## 7. Modelo de estados e persistência

Todo controle relevante deve distinguir:

| Estado | Exemplo de exibição | Requisito |
|---|---|---|
| Suportado e disponível | `Disponível` | Serviço identificado e capaz de executar. |
| Não suportado | `Não suportado neste hardware` | Desativar comando incompatível com explicação. |
| Desconhecido | `Não foi possível verificar` | Não inferir estado positivo. |
| Delegado ao rice | `Gerenciado pelo rice ativo` | Descrever fonte de verdade; link para Settings quando disponível. |
| Salvo, não aplicado | `Requer aplicar` | Persistência não equivale a execução. |
| Aplicando | `Aplicando...` | Operação assíncrona, feedback e timeout. |
| Aplicado | `Confirmado` | Estado efetivo observado. |
| Falha/conflito | `Falha de bloqueio` / `Controladores concorrentes` | Diagnóstico acionável e recuperação segura. |

Recomendações técnicas: configuração atômica e validada, backup, IPC versionado com timeout, leituras assíncronas, identificadores estáveis e atualização por eventos; nenhum `write()` de arquivo ou subprocesso `exit=0` garante aplicação.

## 8. Testes de aceitação propostos (não executados)

| ID | Cenário | Resultado esperado |
|---|---|---|
| ST-01 | Rice oficial ativo | Sistema e Interface oficial acessíveis. |
| ST-02 | Rice externo ativo | Sistema persistente, Interface mostra apenas `Rice ativo`. |
| ST-03 | Trocar rice e retornar | Preferências de barra/dock/launcher preservadas. |
| ST-04 | Criar workspace com exceção de modo por monitor | Novo usa exceção; antigos preservam modo. |
| ST-05 | Conectar/desconectar monitor | Perfil manual e fallback de disposição não quebram o acesso. |
| ST-06 | Aplicar configuração de monitor inválida | Falha controlada ou reversão segura, nunca falsa confirmação. |
| ST-07 | Rice com idle e locker funcionais | Fallback não cria duplicação. |
| ST-08 | Rice sem idle nem locker | Fallback assumido somente se verificado operacional. |
| ST-09 | Rice com idle sem locker | Locker fallback integrado e validado. |
| ST-10 | Rice com locker sem idle | Idle fallback integrado e validado. |
| ST-11 | Bloqueio falha antes da suspensão | Não registrar sessão protegida; executar política de falha explicitamente definida. |
| ST-12 | Rice/crash/desconexão do serviço | Estado degradado comunicado; recuperação sem disputa de controladores. |
| ST-13 | Outros rices alteram perfil energético | Settings reflete estado real compartilhado. |
| ST-14 | Falha de IPC Lua/Hyprland | UI permanece utilizável; erro e diagnóstico acessíveis. |
| ST-15 | Configuração inválida/backup | Sem corrupção; recuperação disponível. |
| ST-16 | Rice externo sem Settings próprio | Não exibir opção de configuração que não funcione. |
| ST-17 | Jogos ou mídia ativam inibidor | Sem suspensão indevida quando o inibidor é respeitado. |
| ST-18 | Segurança de bloqueio em Wayland | Comprovar lock real, não apenas processo ou comando emitido. |

## 9. Fases sugeridas de implementação — não aprovadas como roadmap

**Fase 0 — Inventário técnico:** auditar UI Rust/GTK4 e suas funções existentes (editor de atalhos, layouts, Alt+Tab, Snap Assist), Lua/IPC, plugin C++, reload/autostart e serviços instalados. Classificar `existente`, `parcial`, `ausente` e riscos.

**Fase 1 — Núcleo Settings:** navegação adaptativa, páginas de gerenciador, diagnósticos, observação de capacidades, perfis de monitores manuais, estados `salvo`/`aplicado`.

**Fase 2 — Serviços essenciais de sessão:** inatividade, bloqueio e energia com fallback, delegação por capacidade, resolução de conflitos e testes de proteção de sessão.

**Fase 3 — Rice Store e compatibilidade:** verificação de manifesto e disponibilidade, transição preferencialmente no reinício da sessão, preservar preferências e recuperação do rice oficial.

**Fase 4 — Expansão do sistema:** áudio, rede, Bluetooth, dispositivos, apps padrão, localidade, privacidade e segurança, conforme viabilidade e prioridade.

**Fase 5 — Design system e mockups:** consistência visual da interface oficial/Settings, sem exigir uniformidade de shells externos. O design system pode ser iniciado em paralelo à auditoria funcional, mas não substitui contratos.

## 10. Pendências técnicas e de produto

- Ferramentas e pacotes oficiais do fallback; versão de Hyprland e serviços de sessão suportados.
- Contrato por capacidades do rice e prova de disponibilidade, especialmente componentes externos sem API.
- Regras detalhadas de falha de lock, retomada de suspensão e troca de rice sem dupla automação.
- Hierarquia final de navegação e agrupamento de páginas se a sidebar crescer.
- Classificação de alterações que exigem `Apply`, `Reload` ou nova sessão.
- Design system visual: tokens de cor/tipografia/espaçamento, componentes, animações, acessibilidade.

## 11. Referências de implementação a verificar

- Hypridle (controle de inatividade): https://wiki.hypr.land/Hypr-Ecosystem/hypridle/
- Hyprlock (bloqueio): https://wiki.hypr.land/Hypr-Ecosystem/hyprlock/
- Wayland session lock (semântica de bloqueio): https://wayland.app/protocols/ext-session-lock-v1
- systemd logind (ações de sessão e inibidores): https://www.freedesktop.org/software/systemd/man/latest/logind.conf.html
- WirePlumber: https://pipewire.pages.freedesktop.org/wireplumber/man/wpctl.html

**Status final:** cinco decisões de Settings D-23–D-27 aprovadas; arquitetura funcional consolidada para discussão e preparação técnica. Nenhum código modificado, nenhum serviço instalado e nenhum teste de integração executado.
