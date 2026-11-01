#!/usr/bin/env bash
# Structural check of the main OpenSpec specs after archiving enable-titlebar-drag-alpha1 (no openspec CLI here).
# Also proves the checker itself: a broken copy of the specs must be rejected (negative control).
set -u
here=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P) || exit 1
exec python3 -I - "$here" <<'PY'
import glob, os, re, shutil, sys, tempfile

root = sys.argv[1]
ID = "enable-titlebar-drag-alpha1"
REMOVED = ["Drag da decoração indisponível na alpha-0", "Menu sem comandos de grab mudos na alpha-0",
           "Reativação do drag após alpha-0"]
RENAMED = ["Toque na decoração indisponível", "Cliques de mouse pareados"]
OLD_NAMES = ["Toque na decoração indisponível na alpha-0", "Cliques de mouse preservados e pareados na alpha-0"]
ADDED = ["Arrasto pela barra com fim pelo compositor", "Encaixe, prévia e resistência ao arrastar pela barra",
         "Mover e Redimensionar pelo menu na alpha-1"]

def reqs(text):
    """name -> block text, for every '### Requirement:' block."""
    parts = re.split(r"^### Requirement: (.+)$", text, flags=re.M)
    return {parts[i].strip(): re.split(r"^## ", parts[i + 1], flags=re.M)[0] for i in range(1, len(parts), 2)}

def blocks_of(path):
    return reqs(open(path, encoding="utf-8").read())

def check(specs, deltas):
    errs = []
    got = {c: blocks_of(f"{specs}/{c}/spec.md") for c in ("title-bar", "snap-assist")}
    for cap, rs in got.items():
        if not rs: errs.append(f"{cap}: sem requisitos")
        for name, body in rs.items():
            head = re.split(r"^#### Scenario:", body, flags=re.M)
            if not re.search(r"\b(SHALL|MUST)\b", head[0]): errs.append(f"{cap}/{name}: sem SHALL/MUST")
            scen = re.split(r"^#### Scenario:.*$", body, flags=re.M)[1:]
            if not scen: errs.append(f"{cap}/{name}: sem cenário")
            for s in scen:
                if "**WHEN**" not in s or "**THEN**" not in s: errs.append(f"{cap}/{name}: cenário sem WHEN/THEN")
    tb = got["title-bar"]
    for n in REMOVED + OLD_NAMES:
        if n in tb: errs.append(f"title-bar: deveria ter sumido: {n}")
    for n in RENAMED + ADDED:
        if n not in tb: errs.append(f"title-bar: falta: {n}")
    # the delta's texts are copied as they are
    for cap, rs in got.items():
        for n, body in blocks_of(f"{deltas}/{cap}/spec.md").items():
            if n in REMOVED: continue
            if rs.get(n, "").strip() != body.strip(): errs.append(f"{cap}/{n}: diferente do delta")
    return errs

archived = glob.glob(f"{root}/openspec/changes/archive/*-{ID}")
errs = []
if len(archived) != 1: errs.append(f"pasta arquivada de {ID}: {len(archived)} encontrada(s)")
if os.path.exists(f"{root}/openspec/changes/{ID}"): errs.append(f"{ID} ainda está em changes/")
if archived:
    tasks = open(f"{archived[0]}/tasks.md", encoding="utf-8").read()
    if "## Arquivamento" not in tasks: errs.append("tasks.md sem nota de arquivamento")
    errs += check(f"{root}/openspec/specs", f"{archived[0]}/specs")

# negative control: break a copy of the specs; every break must be reported
tmp = tempfile.mkdtemp()
try:
    if archived:
        shutil.copytree(f"{root}/openspec/specs", f"{tmp}/s")
        p = f"{tmp}/s/title-bar/spec.md"
        t = open(p, encoding="utf-8").read()
        t = t.replace("SHALL", "deve", 1).replace("### Requirement: Cliques de mouse pareados",
              "### Requirement: Drag da decoração indisponível na alpha-0\nO sistema SHALL x.\n\n#### Scenario: a\n- **WHEN** a\n- **THEN** b\n\n### Requirement: Cliques de mouse preservados e pareados na alpha-0")
        open(p, "w", encoding="utf-8").write(t)
        bad = check(f"{tmp}/s", f"{archived[0]}/specs")
        for needle in ("sem SHALL/MUST", "deveria ter sumido", "falta: Cliques de mouse pareados"):
            if not any(needle in e for e in bad): errs.append(f"controle negativo não pegou: {needle}")
    else:
        errs.append("controle negativo não rodou (sem pasta arquivada)")
finally:
    shutil.rmtree(tmp)

for e in errs: print("FALHOU -", e)
print("ok - specs principais conferem" if not errs else f"{len(errs)} falha(s)")
sys.exit(1 if errs else 0)
PY
