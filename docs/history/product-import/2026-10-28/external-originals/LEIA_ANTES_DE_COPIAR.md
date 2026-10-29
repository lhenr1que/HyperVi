# Pacote para copiar à raiz do repositório Kwinish

Extraia este ZIP **na raiz do repositório**, de preferência após criar backup/commit do baseline; ele só contém arquivos novos em `docs/product/` e este leia-me. **Não** execute automaticamente comandos de instalação ou migração.

Exemplo de uso, depois de transferir o zip para o computador Linux:

```bash
cd /caminho/para/kwinish
unzip -l ~/Downloads/Kwinish_Product_Docs_2026-10-28.zip   # confira conteúdo
unzip ~/Downloads/Kwinish_Product_Docs_2026-10-28.zip
# se o zip foi extraído com diretório raiz, mova somente docs/product/ para o repositório
```

A estrutura do ZIP usa caminhos relativos `docs/product/...`, sem alterar `openspec/`, código, `AGENTS.md` ou outros arquivos existentes. Após extrair, leia `docs/product/README.md`.

Para dar descoberta automática aos agentes, o trecho recomendado está em `docs/product/AGENTS_ROOT_SNIPPET.md`; revisá-lo e incorporar manualmente no `AGENTS.md` raiz.

Caso não queira misturar documentação extensa ao código, alternativa: sub-repositório docs e link `docs/product` por Git submodule; não indicado neste estágio de projeto local.
