# Preserved grind evidence

## Why this exists

Committed files in this repo — `src/*.c` comments, `docs/grind/*.md`, and
`inline_asm_canonical.txt` authorization rows — cite evidence paths under
`tmp/`. `tmp/` is **fully gitignored scratch** (AGENTS.md, "File-edit
conventions"), so those citations were durable only for as long as nobody
cleaned the directory.

The 2026-09-24 hygiene run measured the exposure: **751 distinct `tmp/` paths
were cited from committed files, and 700 of them still resolved** — i.e. the
project had accumulated a large body of load-bearing evidence that lived
exclusively in disposable scratch, including the evidence backing
canonical-asm authorizations.

`tmp/grind/` had reached 16 GB / 336k files at that point, so "just never clean
it" was not a workable answer.

## What was preserved

| File | Contents |
|---|---|
| `evidence-2026-09-24.tar.gz` | Every cited `tmp/` file that still existed — 521 files, 36.4 MB raw, 4.2 MB compressed |
| `evidence-2026-09-24.manifest.txt` | `sha1  bytes  path` for all 521, so a citation can be checked without extracting |

Paths are stored **verbatim**, including the leading `tmp/`. A citation of
`tmp/grind/CD_ready/s58/splice.py` extracts to exactly that path:

```bash
tar -xzf docs/grind/evidence-2026-09-24.tar.gz tmp/grind/CD_ready/s58/splice.py
grep 'CD_ready/s58/splice.py' docs/grind/evidence-2026-09-24.manifest.txt
```

Three binary artifacts were deliberately **not** preserved — they are rebuildable
compiler/link outputs, not evidence: `tmp/ab/cc1.mixed`, `tmp/gccdbg/cc1`, and
`tmp/grind/func_80076D74/s2/handoff/fullbuild/bb2.exe`.

Citations in committed files were **not rewritten**. Rewriting 530 comment lines
across `src/` would be large, oracle-irrelevant churn for no gain; the mapping
rule above resolves them, and the manifest makes a stale citation detectable.

## Standing rule going forward

**A committed file must not cite `tmp/` as the sole home of load-bearing
evidence.** `tmp/` is scratch and gets purged. When a construct, an
authorization, or a killed hypothesis needs a durable citation, put the evidence
in a tracked location:

- per-function reasoning → `memory/grind/<func>/` (tracked)
- narrative and rulings → `docs/grind/journal.md`, `docs/grind/decisions.md`
- bulk artifacts worth keeping → a dated bundle alongside this one

Citing `tmp/` for something genuinely ephemeral (a one-off reproduction command,
a scratch log referenced in the same session) remains fine.
