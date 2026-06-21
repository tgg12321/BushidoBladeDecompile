#!/usr/bin/env python3
"""
decomp.me corpus scraper for BB2-relevant compilers.

Background
----------
decomp.me hosts collaborative matching-decompilation "scratches." Three of its
compilers match the toolchain class Bushido Blade 2 uses (GCC 2.7.2 / PsyQ
3.5), so the corpus of scratches under those compilers is a source of
known-good codegen examples we can mine when stuck on a function here.

Site facts (probed 2026-06-21, see memory/project/decomp-me-corpus.md):

* API root:  ``https://decomp.me/api`` -- public DRF, no auth needed for GET.
* Cloudflare front gates non-browser clients; ``curl_cffi`` impersonating
  ``chrome131`` clears it. Plain ``requests`` / ``urllib`` get 403 + a JS
  challenge page. Polite: 0.5s sleep between requests.
* Pagination is cursor-based (``next`` URL + ``page_size`` up to 100).
* Useful endpoints:
    - ``GET /stats``                              site totals
    - ``GET /scratch-count``                      total scratch count
    - ``GET /compiler``  ``GET /platform``        compiler & platform metadata
    - ``GET /scratch?compiler=<id>&page_size=N``  list (terse fields only)
    - ``GET /scratch/<slug>``                     detail (no target asm field)
    - ``GET /scratch/<slug>/export``              ZIP containing
      ``metadata.json``, ``target.s``, ``target.o``, ``code.c``, ``ctx.c``,
      ``current.o`` -- this is how we get the target assembly text

CLI
---
    python3 tools/decomp_me_scrape.py probe
    python3 tools/decomp_me_scrape.py download --compiler gcc2.7.2-psx \
        --limit 50 --out tmp/decomp_me_corpus/
    python3 tools/decomp_me_scrape.py search --asm-file asm/funcs/AddTbpOfst.s
"""
from __future__ import annotations

import argparse
import io
import json
import os
import re
import sys
import time
import zipfile
from pathlib import Path
from typing import Dict, Iterable, List, Optional

try:
    from curl_cffi import requests as crequests
except ImportError:  # pragma: no cover - runtime dep
    sys.stderr.write(
        "curl_cffi is required (Cloudflare-friendly TLS fingerprint).\n"
        "Install: source .venv/bin/activate && pip install curl_cffi\n"
    )
    raise

API_BASE = "https://decomp.me/api"
SITE_BASE = "https://decomp.me"
DEFAULT_OUT = Path("tmp/decomp_me_corpus")
BB2_COMPILERS = ("gcc2.7.2-psx", "gcc2.7.2-cdk", "psyq3.5")
POLITE_DELAY_S = 0.5
TIMEOUT_S = 60
IMPERSONATE = "chrome131"
CONTACT = "BB2-decomp-corpus-mining/1.0 (tgg12321@gmail.com)"


def make_session() -> "crequests.Session":
    s = crequests.Session(impersonate=IMPERSONATE)
    # The TLS/UA fingerprint is what gets us past Cloudflare; we add a
    # contact header on top so site operators can see who we are.
    s.headers.update({
        "Accept": "application/json",
        "Referer": f"{SITE_BASE}/",
        "X-Contact": CONTACT,
    })
    return s


def get_json(s, path: str, *, extra_headers: Optional[Dict[str, str]] = None) -> dict:
    url = path if path.startswith("http") else f"{API_BASE}{path}"
    r = s.get(url, timeout=TIMEOUT_S, headers=extra_headers or None)
    r.raise_for_status()
    return r.json()


def get_bytes(s, path: str) -> bytes:
    url = path if path.startswith("http") else f"{API_BASE}{path}"
    r = s.get(
        url,
        timeout=TIMEOUT_S,
        headers={"Accept": "*/*", "Referer": f"{SITE_BASE}/"},
    )
    r.raise_for_status()
    return r.content


# --------------------------------------------------------------------------- #
# probe                                                                       #
# --------------------------------------------------------------------------- #

def cmd_probe(_args: argparse.Namespace) -> int:
    s = make_session()
    targets = [
        ("/stats", "site totals"),
        ("/scratch-count", "total scratch count"),
        ("/scratch?compiler=gcc2.7.2-psx&page_size=1", "list (gcc2.7.2-psx)"),
        ("/scratch?compiler=gcc2.7.2-cdk&page_size=1", "list (gcc2.7.2-cdk)"),
        ("/scratch?compiler=psyq3.5&page_size=1", "list (psyq3.5)"),
        ("/scratch?platform=ps1&page_size=1", "list (platform=ps1)"),
    ]
    for path, label in targets:
        print(f"\n== {label}  ({path}) ==")
        try:
            data = get_json(s, path)
        except Exception as e:
            print(f"  ERR: {e}")
            continue
        if isinstance(data, dict):
            keys = list(data.keys())
            print(f"  keys: {keys}")
            if "results" in data and data["results"]:
                print(f"  result[0] keys: {list(data['results'][0].keys())}")
                print(f"  result[0]: {json.dumps(data['results'][0])[:400]}")
            else:
                # Summary endpoint -- show body, capped.
                print(f"  body: {json.dumps(data)[:400]}")
        time.sleep(POLITE_DELAY_S)

    # Detail + export probe on the first gcc2.7.2-psx scratch.
    print("\n== detail + export probe ==")
    try:
        listing = get_json(s, "/scratch?compiler=gcc2.7.2-psx&page_size=1")
        slug = listing["results"][0]["slug"]
        print(f"  picked slug: {slug}")
        time.sleep(POLITE_DELAY_S)

        detail = get_json(s, f"/scratch/{slug}")
        print(f"  detail keys: {list(detail.keys())}")
        time.sleep(POLITE_DELAY_S)

        z = zipfile.ZipFile(io.BytesIO(get_bytes(s, f"/scratch/{slug}/export")))
        print(f"  export ZIP entries: {z.namelist()}")
    except Exception as e:
        print(f"  ERR: {e}")
        return 1
    return 0


# --------------------------------------------------------------------------- #
# download                                                                    #
# --------------------------------------------------------------------------- #

def _scratch_record(detail: dict, export_zip: zipfile.ZipFile) -> dict:
    """Build a single JSON record from detail + export ZIP contents."""
    target_asm = ""
    metadata = {}
    code_c = ""
    ctx_c = ""
    for name in export_zip.namelist():
        try:
            blob = export_zip.read(name)
        except KeyError:
            continue
        if name == "target.s":
            target_asm = blob.decode("utf-8", errors="replace")
        elif name == "metadata.json":
            try:
                metadata = json.loads(blob.decode())
            except Exception:
                metadata = {"_raw": blob.decode(errors="replace")}
        elif name == "code.c":
            code_c = blob.decode("utf-8", errors="replace")
        elif name == "ctx.c":
            ctx_c = blob.decode("utf-8", errors="replace")
    return {
        "slug": detail.get("slug"),
        "name": detail.get("name"),
        "url": f"{SITE_BASE}/scratch/{detail.get('slug')}",
        "compiler": detail.get("compiler"),
        "platform": detail.get("platform"),
        "compiler_flags": detail.get("compiler_flags"),
        "score": detail.get("score"),
        "max_score": detail.get("max_score"),
        "is_matching": detail.get("score") == 0,
        "match_override": detail.get("match_override", False),
        "preset": detail.get("preset"),
        "owner": (detail.get("owner") or {}).get("username"),
        "diff_label": detail.get("diff_label"),
        "creation_time": detail.get("creation_time"),
        "last_updated": detail.get("last_updated"),
        "libraries": detail.get("libraries", []),
        "description": detail.get("description", ""),
        "source_code": detail.get("source_code") or code_c,
        "context": ctx_c if ctx_c else detail.get("context", ""),
        "target_assembly": target_asm,
        "export_metadata": metadata,
    }


def cmd_download(args: argparse.Namespace) -> int:
    out_dir = Path(args.out)
    out_dir.mkdir(parents=True, exist_ok=True)
    s = make_session()

    path = f"/scratch?compiler={args.compiler}&page_size=20"
    fetched = 0
    skipped = 0
    failures = 0
    while path and fetched < args.limit:
        try:
            page = get_json(s, path)
        except Exception as e:
            print(f"list fetch failed: {e}", file=sys.stderr)
            return 2
        for item in page.get("results", []):
            if fetched >= args.limit:
                break
            slug = item["slug"]
            out_path = out_dir / f"{args.compiler}__{slug}.json"
            if out_path.exists():
                skipped += 1
                continue

            time.sleep(POLITE_DELAY_S)
            try:
                detail = get_json(s, f"/scratch/{slug}")
            except Exception as e:
                print(f"  detail {slug} failed: {e}", file=sys.stderr)
                failures += 1
                continue

            time.sleep(POLITE_DELAY_S)
            try:
                zbytes = get_bytes(s, f"/scratch/{slug}/export")
                z = zipfile.ZipFile(io.BytesIO(zbytes))
            except Exception as e:
                print(f"  export {slug} failed: {e}", file=sys.stderr)
                failures += 1
                continue

            record = _scratch_record(detail, z)
            out_path.write_text(
                json.dumps(record, indent=2, ensure_ascii=False) + "\n",
                encoding="utf-8",
            )
            fetched += 1
            match_tag = "MATCH" if record["is_matching"] else f"score={record['score']}"
            print(
                f"  [{fetched}/{args.limit}] {slug}  {record['name']}  "
                f"{match_tag}  ({len(record['target_assembly'])} B asm)"
            )
        path = page.get("next")
        if path:
            time.sleep(POLITE_DELAY_S)

    print(
        f"\nDone. fetched={fetched} skipped(cached)={skipped} failures={failures} "
        f"out={out_dir}"
    )
    return 0 if failures == 0 else 1


# --------------------------------------------------------------------------- #
# search -- asm-token shingles + Jaccard                                      #
# --------------------------------------------------------------------------- #

_MNEM_RE = re.compile(r"^\s*(?:/\*[^*]*\*/\s*)?([a-z][a-z0-9.]*)\b", re.IGNORECASE)

def _opcode_stream(asm_text: str) -> List[str]:
    """Extract the opcode mnemonic from each asm line, ignoring labels/data."""
    ops: List[str] = []
    for line in asm_text.splitlines():
        s = line.strip()
        if not s or s.startswith((".", "#", "glabel", "dlabel")) or s.endswith(":"):
            continue
        # Strip a leading `/* offset hex hex */` comment if present.
        s2 = re.sub(r"^/\*[^*]*\*/\s*", "", s)
        m = _MNEM_RE.match(s2)
        if m:
            mn = m.group(1).lower()
            # Skip assembler directives that slipped through.
            if mn.startswith("."):
                continue
            ops.append(mn)
    return ops


def _shingles(ops: List[str], k: int = 4) -> set:
    if len(ops) < k:
        return {tuple(ops)} if ops else set()
    return {tuple(ops[i : i + k]) for i in range(len(ops) - k + 1)}


def _jaccard(a: set, b: set) -> float:
    if not a or not b:
        return 0.0
    inter = len(a & b)
    union = len(a | b)
    return inter / union if union else 0.0


def cmd_search(args: argparse.Namespace) -> int:
    query_path = Path(args.asm_file)
    if not query_path.exists():
        print(f"asm file not found: {query_path}", file=sys.stderr)
        return 2
    q_ops = _opcode_stream(query_path.read_text(encoding="utf-8", errors="replace"))
    q_sh = _shingles(q_ops, k=args.shingle_k)
    if not q_sh:
        print(f"no opcodes extracted from {query_path}", file=sys.stderr)
        return 2

    corpus_dir = Path(args.corpus)
    files = sorted(corpus_dir.glob("*.json"))
    if not files:
        print(
            f"no corpus files in {corpus_dir} -- run `download` first",
            file=sys.stderr,
        )
        return 2

    scored = []
    for fp in files:
        try:
            data = json.loads(fp.read_text(encoding="utf-8"))
        except Exception:
            continue
        ops = _opcode_stream(data.get("target_assembly", ""))
        sh = _shingles(ops, k=args.shingle_k)
        sim = _jaccard(q_sh, sh)
        if sim > 0:
            scored.append((sim, data))
    scored.sort(key=lambda t: t[0], reverse=True)
    top = scored[: args.top]
    if not top:
        print("(no overlapping shingles found)")
        return 0
    for sim, d in top:
        print(
            f"{d.get('url')}  (similarity {sim:.3f})  "
            f"[{d.get('compiler')}]  {d.get('name')}  "
            f"score={d.get('score')}/{d.get('max_score')}"
        )
    return 0


# --------------------------------------------------------------------------- #
# main                                                                        #
# --------------------------------------------------------------------------- #

def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        description="Scrape decomp.me scratches for the BB2 toolchain corpus.",
    )
    sub = p.add_subparsers(dest="cmd", required=True)

    sp = sub.add_parser("probe", help="probe API endpoints (no downloads)")
    sp.set_defaults(func=cmd_probe)

    sd = sub.add_parser("download", help="download a slice of scratches into a corpus dir")
    sd.add_argument("--compiler", required=True, choices=BB2_COMPILERS + ("any",))
    sd.add_argument("--limit", type=int, default=50)
    sd.add_argument("--out", default=str(DEFAULT_OUT))
    sd.set_defaults(func=cmd_download)

    ss = sub.add_parser("search", help="find closest scratches to a target.s by asm shingles")
    ss.add_argument("--asm-file", required=True)
    ss.add_argument("--corpus", default=str(DEFAULT_OUT))
    ss.add_argument("--top", type=int, default=5)
    ss.add_argument("--shingle-k", type=int, default=4)
    ss.set_defaults(func=cmd_search)

    return p


def main(argv: Optional[List[str]] = None) -> int:
    args = build_parser().parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    raise SystemExit(main())
