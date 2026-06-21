---
name: decomp-me-corpus
description: tools/decomp_me_scrape.py — scrape decomp.me scratches for the three BB2-toolchain compilers (gcc2.7.2-psx / gcc2.7.2-cdk / psyq3.5) into tmp/decomp_me_corpus/ as a corpus of known-good GCC 2.7.2 PSX codegen examples; includes a shingle-Jaccard search to find scratches whose target asm overlaps a BB2 asm file.
metadata:
  type: project
---

# `tools/decomp_me_scrape.py` — decomp.me corpus scraper

decomp.me hosts a large public corpus of matching-decomp "scratches" under
the exact toolchain class BB2 uses: GCC 2.7.2 / PsyQ 3.5. This tool mines
that corpus so it can be searched for prior-art codegen when we get stuck on
a function.

## Endpoints confirmed (probed 2026-06-21)

API root: `https://decomp.me/api`. Public DRF; **no auth required for GET**.
Cursor-based pagination (`results` + `next` URL; `page_size` up to 100).

| Endpoint | What we use it for |
|---|---|
| `GET /stats` | site totals (asm_count, scratch_count, github_user_count) |
| `GET /scratch-count` | exact total scratch count |
| `GET /compiler`, `GET /platform` | toolchain registry (ps1 platform; the three compilers) |
| `GET /scratch?compiler=<id>&page_size=N` | terse list (slug, name, score, owner, compiler, etc.) |
| `GET /scratch/<slug>` | full detail (source_code, context, compiler_flags, ...) — but **no** target asm field |
| `GET /scratch/<slug>/export` | ZIP: `metadata.json`, `target.s`, `target.o`, `code.c`, `ctx.c`, `current.o` — this is how we get the target assembly text |

## BB2-relevant counts (probed 2026-06-21)

Site totals: ~245,000 scratches total, ~167,000 asm objects, ~3,500 GitHub
users. The PS1 platform (`platform=ps1`) covers the three BB2 compilers.
Listing returns multi-page results for each — the corpus per compiler is
large enough to be useful (hundreds of scratches at minimum; download
incrementally via `--limit`).

## Cloudflare caveat

decomp.me is fronted by Cloudflare. **Plain `requests` / `urllib` get HTTP
403 + a JS-challenge page** from this network's IP reputation. The scraper
uses `curl_cffi` with `impersonate="chrome131"` to present a real browser
TLS fingerprint, which clears the challenge. Other impersonations tried
(chrome120/124, firefox120, safari17, edge101) were all blocked. If a future
Chrome bumps the wire fingerprint, update `IMPERSONATE` in the script.

The session also sends `Referer: https://decomp.me/` and an
`X-Contact: BB2-decomp-corpus-mining/1.0 (tgg12321@gmail.com)` header so
site operators can identify the scraper. Polite: 0.5 s sleep between
requests.

## What a corpus file contains

`tmp/decomp_me_corpus/<compiler>__<slug>.json`, one per scratch:

| Field | Source |
|---|---|
| `slug`, `name`, `url`, `compiler`, `platform`, `compiler_flags`, `preset`, `owner`, `creation_time`, `last_updated`, `libraries`, `description` | detail |
| `score`, `max_score`, `match_override`, `is_matching` (= `score == 0`) | detail (score=0 == byte-match) |
| `source_code` | detail (the user's best C attempt) |
| `context` | export `ctx.c` (preferred) or detail `context` |
| `target_assembly` | export `target.s` (raw MIPS text) |
| `export_metadata` | export `metadata.json` (compiler version, flags as seen by the export pipeline) |

`is_matching == True` means the user's C body is byte-identical to target —
those are the gold examples.

## Invocation

The scraper runs under the project venv (curl_cffi is the only added dep):

```
source .venv/bin/activate
pip install curl_cffi   # one-time

# 1) verify the API path + sample shapes (no downloads)
python3 tools/decomp_me_scrape.py probe

# 2) pull a slice into the corpus (idempotent; skips cached slugs on re-run)
python3 tools/decomp_me_scrape.py download --compiler gcc2.7.2-psx --limit 50 \
    --out tmp/decomp_me_corpus/
python3 tools/decomp_me_scrape.py download --compiler gcc2.7.2-cdk --limit 50 \
    --out tmp/decomp_me_corpus/
python3 tools/decomp_me_scrape.py download --compiler psyq3.5      --limit 50 \
    --out tmp/decomp_me_corpus/

# 3) find the top-5 closest scratches to a BB2 target.s by asm shingles
python3 tools/decomp_me_scrape.py search --asm-file asm/funcs/AddTbpOfst.s
```

## How `search` works

Reads the query `.s` file, extracts the opcode stream (mnemonic per asm
line, ignoring labels/directives/comments), forms 4-shingles (overlapping
4-grams of mnemonics), Jaccard-similarity against each cached corpus
scratch's `target_assembly` shingles, prints the top N. Cheap, no embedding
model, no per-query API call. Tunable via `--shingle-k` (k=4 is strict, k=2
is loose). Useful as a coarse pre-filter; manual inspection of the matches
still needs to verify they're actually structurally analogous.

## Caveats

* All cached files live under `tmp/decomp_me_corpus/` (gitignored per
  root-cleanliness).
* The corpus snapshot is point-in-time; re-running `download` with the same
  `--limit` will fetch newer scratches as they're posted (cached slugs are
  skipped, so the list page must yield enough new slugs to reach `--limit`).
* The detail endpoint exposes `context_text` but is excluded by the
  serializer's `exclude = ["target_assembly", "context_fk"]`; the export ZIP
  is the only way to get the raw target asm text without auth.
* No rate-limit headers observed in responses, but be conservative: the
  built-in 0.5 s delay is the standing politeness contract.
