#!/usr/bin/env python3
"""Inventory asm/data/*.rodata*.s blocks linked by bb2.ld.

Phase 0 of the rodata-cleanup project (see docs/rodata-cleanup-project.md).

For each rodata symbol in a linked asm/data block, identify:
  - symbol address, type (jtbl/string/data), size
  - which functions reference it (asm/funcs/*.s + asmfix.txt + regfix.txt)
  - each owner function's current status (active/authorize/parked/stub/completed)

Writes memory/project/rodata_block_inventory.csv and prints a per-block summary.
"""

from __future__ import annotations

import csv
import json
import re
import sys
from collections import defaultdict
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
ASM_DATA = REPO / "asm" / "data"
ASM_FUNCS = REPO / "asm" / "funcs"
SRC = REPO / "src"
ASMFIX = REPO / "asmfix.txt"
REGFIX = REPO / "regfix.txt"
QUEUE = REPO / "engine" / "queue.json"
OUT_CSV = REPO / "memory" / "project" / "rodata_block_inventory.csv"

# The 12 asm/data/*.rodata*.s files that appear as link inputs in bb2.ld.
# The two BIG files (800.rodata.s, 101C.rodata.s) are splat parents that get
# auto-split into these variants at build time — they are NOT separately linked.
# Confirmed 2026-06-09 by grepping bb2.ld.
LINKED_BLOCKS = [
    "800.rodata_pre.s",
    "800.rodata_post.s",
    "101C.rodata_pre.s",
    "101C.rodata_pre_post.s",
    "101C.rodata_c_pre.s",
    "101C.rodata_post.s",
    "101C.rodata_c2_post.s",
    "101C.rodata_text1a_DB8.s",
    "101C.rodata_text1a_a.s",
    "101C.rodata_text1a_b_pre.s",
    "101C.rodata_text1a_b_post.s",
    "101C.rodata_main_post.s",
]

SYMBOL_REF_RE = re.compile(r"\b(jtbl_[0-9A-Fa-f]{8}|D_[0-9A-Fa-f]{8})\b")
ALIAS_RE = re.compile(r"^([A-Za-z_]\w*)\s*=\s*0x([0-9A-Fa-f]+)\s*;", re.MULTILINE)
NAMED_SYMS = REPO / "named_syms.txt"
SYMBOL_ADDRS = REPO / "symbol_addrs.txt"
DLABEL_RE = re.compile(
    r"^dlabel\s+(\S+)\s*\n(.*?)^enddlabel\s+\1",
    re.MULTILINE | re.DOTALL,
)
ADDR_RE = re.compile(r"/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s")
WORD_LINE_RE = re.compile(r"^\s*(?:/\*[^*]*\*/)?\s*\.word\b", re.MULTILINE)
ASCIZ_RE = re.compile(r'\.asciz\s+"((?:[^"\\]|\\.)*)"')


def asciz_bytes(s: str) -> int:
    """Bytes a single .asciz literal occupies (string + null terminator)."""
    # Decode common C-string escapes; leave others as raw text.
    try:
        decoded = bytes(s, "latin-1").decode("unicode_escape")
    except UnicodeDecodeError:
        decoded = s
    return len(decoded.encode("latin-1", errors="replace")) + 1


def parse_block(path: Path) -> list[dict]:
    """Return [{symbol, address, type, size_bytes}, ...] for one rodata file."""
    text = path.read_text(errors="replace")
    out: list[dict] = []
    for m in DLABEL_RE.finditer(text):
        sym = m.group(1)
        body = m.group(2)

        addr_m = ADDR_RE.search(body)
        address = "0x" + addr_m.group(1).upper() if addr_m else ""

        if sym.startswith("jtbl_"):
            symtype = "jtbl"
        elif ASCIZ_RE.search(body):
            symtype = "string"
        else:
            symtype = "data"

        words = len(WORD_LINE_RE.findall(body))
        ascii_b = sum(asciz_bytes(am.group(1)) for am in ASCIZ_RE.finditer(body))
        size_bytes = words * 4 + ascii_b

        out.append(
            {
                "symbol": sym,
                "address": address,
                "type": symtype,
                "size_bytes": size_bytes,
            }
        )
    return out


def build_alias_map() -> dict[str, set[str]]:
    """canonical_D_or_jtbl_name -> set of human-readable aliases pointing at the
    same address.

    Sources: named_syms.txt, symbol_addrs.txt. Each line `<name> = 0xADDR;`
    declares an alias.
    """
    by_addr: dict[str, set[str]] = defaultdict(set)
    for src in (NAMED_SYMS, SYMBOL_ADDRS):
        if not src.exists():
            continue
        text = src.read_text(errors="replace")
        for m in ALIAS_RE.finditer(text):
            name = m.group(1)
            addr = m.group(2).upper()
            by_addr[addr].add(name)

    # Now: any address that has a `D_<addr>` or `jtbl_<addr>` alias, AND also has
    # other aliases, gets a back-map from the D_/jtbl_ name to those aliases.
    out: dict[str, set[str]] = defaultdict(set)
    for addr, names in by_addr.items():
        canonical_candidates = {f"D_{addr}", f"jtbl_{addr}"} & names
        other_names = names - canonical_candidates
        if not other_names:
            continue
        for canonical in canonical_candidates:
            out[canonical].update(other_names)
        # If no canonical D_/jtbl_ name appears in the named_syms but we still
        # want to map any name at this address to D_<addr>:
        if not canonical_candidates:
            out[f"D_{addr}"].update(other_names)
            out[f"jtbl_{addr}"].update(other_names)
    return out


def collect_owners() -> dict[str, set[str]]:
    """symbol -> set of function names that reference it.

    Sources searched:
      - asm/funcs/*.s      (stub functions whose body is the disassembly)
      - asmfix.txt         (matched-but-bridged functions naming jtbl_/D_ symbols in rules)
      - regfix.txt         (rare, but checked)
      - src/*.c            (matched-pure-C functions referencing D_XXXXXXXX via extern)
    """
    owners: dict[str, set[str]] = defaultdict(set)

    # Pass 1: asm/funcs/*.s — stubs and other functions with remaining disassembly
    for sf in ASM_FUNCS.glob("*.s"):
        func = sf.stem
        text = sf.read_text(errors="replace")
        for sym in SYMBOL_REF_RE.findall(text):
            owners[sym].add(func)

    # Pass 2 + 3: asmfix.txt, regfix.txt — rule lines starting with "funcname:"
    rule_line_re = re.compile(r"^([A-Za-z_]\w+):\s+")
    for src in (ASMFIX, REGFIX):
        if not src.exists():
            continue
        for line in src.read_text(errors="replace").splitlines():
            m = rule_line_re.match(line)
            if not m:
                continue
            func = m.group(1)
            for sym in SYMBOL_REF_RE.findall(line):
                owners[sym].add(func)

    # Pass 4: src/*.c — find every function containing a D_/jtbl_ symbol reference,
    # OR an alias (from named_syms.txt / symbol_addrs.txt) that resolves back to a
    # D_/jtbl_ symbol. We tokenize each .c file into top-level functions
    # (brace-balanced) and assign symbol references to the enclosing function.
    alias_map = build_alias_map()  # canonical_sym -> {alias names}
    # Invert: alias_name -> set of canonical_syms it points at
    alias_to_canon: dict[str, set[str]] = defaultdict(set)
    for canon, aliases in alias_map.items():
        for a in aliases:
            alias_to_canon[a].add(canon)

    # Build a single regex matching any alias (limit to those at our addresses to
    # keep the regex small; but in practice the map is ~hundreds of names which
    # is fine).
    if alias_to_canon:
        alias_re = re.compile(
            r"\b(" + "|".join(re.escape(a) for a in alias_to_canon) + r")\b"
        )
    else:
        alias_re = None

    for sf in SRC.glob("*.c"):
        try:
            text = sf.read_text(errors="replace")
        except Exception:
            continue
        for func, body in iter_c_functions(text):
            for sym in SYMBOL_REF_RE.findall(body):
                owners[sym].add(func)
            if alias_re is not None:
                for alias in alias_re.findall(body):
                    for canon in alias_to_canon[alias]:
                        owners[canon].add(func)

    # Pass 5: transitive ownership via .data tables.
    # A rodata symbol that's referenced from a .data symbol (e.g. a string-pointer
    # table in `asm/data/*.data.s`) inherits the .data symbol's owners. We scan
    # asm/data/*.data.s for dlabel regions that contain a D_/jtbl_ reference, then
    # find owners of the .data symbol itself via the previously-built owner map.
    data_ref_map: dict[str, set[str]] = defaultdict(set)  # rodata_sym -> {data_syms referencing it}
    for df in ASM_DATA.glob("*.data.s"):
        text = df.read_text(errors="replace")
        for m in DLABEL_RE.finditer(text):
            data_sym = m.group(1)
            for rodata_sym in SYMBOL_REF_RE.findall(m.group(2)):
                if rodata_sym != data_sym:
                    data_ref_map[rodata_sym].add(data_sym)

    for rodata_sym, data_syms in data_ref_map.items():
        for ds in data_syms:
            transitive_owners = owners.get(ds, set())
            if transitive_owners:
                owners[rodata_sym].update(transitive_owners)

    return owners


def iter_c_functions(text: str):
    """Yield (func_name, body_text) for each top-level function definition in a .c file.

    Heuristic: a top-level function is a `<func>(...)` signature followed (after
    optional whitespace, comments, and parameter list continuation) by a `{` at
    brace-depth 0. We track brace depth to find the matching `}`.
    """
    # PSX decomp style: `<returnType> <funcName>(<args>) {` on one line, possibly
    # preceded by `static`/`extern`/`inline`. We allow one or more type tokens
    # (alphanumeric, with optional `*`/`&` and whitespace) before the function
    # name, then capture the FINAL identifier before `(`.
    sig_re = re.compile(
        r"^(?:(?:static|extern|inline|const|volatile|unsigned|signed)\s+)*"  # storage/quals
        r"(?:[A-Za-z_]\w*[\s\*]+)+"                                          # return type
        r"([A-Za-z_]\w*)\s*\([^;{}]*?\)\s*\{",                               # func(args) {
        re.MULTILINE | re.DOTALL,
    )
    for m in sig_re.finditer(text):
        func = m.group(1)
        if func in ("if", "for", "while", "switch", "do", "return", "sizeof"):
            continue
        # Find matching close brace from m.end() - 1 (which is the opening {)
        depth = 0
        i = m.end() - 1
        n = len(text)
        body_start = i
        while i < n:
            c = text[i]
            if c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    yield func, text[body_start : i + 1]
                    break
            i += 1


def collect_stubs() -> set[str]:
    """Set of funcs with `replace_with_asmfile` rules in asmfix.txt."""
    stubs: set[str] = set()
    rule_line_re = re.compile(r"^([A-Za-z_]\w+):.*replace_with_asmfile")
    for line in ASMFIX.read_text(errors="replace").splitlines():
        m = rule_line_re.match(line)
        if m:
            stubs.add(m.group(1))
    return stubs


def collect_queue_status() -> dict[str, tuple[str, str]]:
    """func -> (status, file_stem) from engine/queue.json."""
    data = json.loads(QUEUE.read_text())
    out: dict[str, tuple[str, str]] = {}
    for it in data.get("items", []):
        out[it["func"]] = (it.get("status", "?"), it.get("file", "?"))
    return out


def build_src_func_index() -> dict[str, str]:
    """Map function name -> .c file stem by extracting every brace-balanced
    top-level function from each src/*.c file."""
    idx: dict[str, str] = {}
    for sf in SRC.glob("*.c"):
        try:
            text = sf.read_text(errors="replace")
        except Exception:
            continue
        for func, _body in iter_c_functions(text):
            idx.setdefault(func, sf.stem)
    return idx


def classify_status(
    func: str,
    queue_status: dict[str, tuple[str, str]],
    stubs: set[str],
) -> str:
    """Classify with stub > queue-presence > completed.

    A function with a `replace_with_asmfile` rule is fundamentally a stub —
    its rodata comes from the asm/data block, not from cc1. That fact is what
    matters for this project, regardless of whether the function also has a
    queue entry (which it usually does, since stubs are also INCOMPLETE).
    """
    if func in stubs:
        return "stub"
    if func in queue_status:
        return queue_status[func][0]
    return "completed"


def queue_state(
    func: str,
    queue_status: dict[str, tuple[str, str]],
) -> str:
    """Raw queue status, or '-' if not in the queue (i.e. completed)."""
    if func in queue_status:
        return queue_status[func][0]
    return "-"


def resolve_file(
    func: str,
    queue_status: dict[str, tuple[str, str]],
    src_idx: dict[str, str],
) -> str:
    if func in queue_status:
        return queue_status[func][1]
    return src_idx.get(func, "?")


def main() -> int:
    if not QUEUE.exists():
        print(f"ERR: {QUEUE} not found — run `engine queue regen` first?", file=sys.stderr)
        return 2

    queue_status = collect_queue_status()
    stubs = collect_stubs()
    owners_map = collect_owners()
    src_idx = build_src_func_index()

    rows: list[dict] = []
    for block_name in LINKED_BLOCKS:
        block_path = ASM_DATA / block_name
        if not block_path.exists():
            print(f"WARN: {block_path} missing", file=sys.stderr)
            continue
        for sym in parse_block(block_path):
            symbol = sym["symbol"]
            owners = sorted(owners_map.get(symbol, set()))
            if not owners:
                rows.append(
                    {
                        "block_file": block_name,
                        "symbol": symbol,
                        "address": sym["address"],
                        "type": sym["type"],
                        "size_bytes": sym["size_bytes"],
                        "owner_func": "(none)",
                        "owner_file": "",
                        "owner_status": "orphan",
                        "queue_status": "-",
                    }
                )
            else:
                for owner in owners:
                    rows.append(
                        {
                            "block_file": block_name,
                            "symbol": symbol,
                            "address": sym["address"],
                            "type": sym["type"],
                            "size_bytes": sym["size_bytes"],
                            "owner_func": owner,
                            "owner_file": resolve_file(owner, queue_status, src_idx),
                            "owner_status": classify_status(owner, queue_status, stubs),
                            "queue_status": queue_state(owner, queue_status),
                        }
                    )

    OUT_CSV.parent.mkdir(parents=True, exist_ok=True)
    fieldnames = [
        "block_file",
        "symbol",
        "address",
        "type",
        "size_bytes",
        "owner_func",
        "owner_file",
        "owner_status",
        "queue_status",
    ]
    with OUT_CSV.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=fieldnames)
        w.writeheader()
        w.writerows(rows)
    print(f"wrote {len(rows)} rows to {OUT_CSV.relative_to(REPO)}", file=sys.stderr)

    # Per-block summary
    per_block: dict[str, dict] = {}
    seen_sym_in_block: dict[str, set[str]] = defaultdict(set)
    status_counts_field = ("stub", "completed", "active", "authorize", "parked", "orphan")
    for r in rows:
        b = per_block.setdefault(
            r["block_file"],
            {
                "syms": set(),
                "owners": set(),
                "bytes": 0,
                "jtbls": set(),
                "strings": set(),
                "data": set(),
                **{k: 0 for k in status_counts_field},
            },
        )
        sym = r["symbol"]
        if sym not in seen_sym_in_block[r["block_file"]]:
            seen_sym_in_block[r["block_file"]].add(sym)
            b["syms"].add(sym)
            try:
                b["bytes"] += int(r["size_bytes"])
            except (TypeError, ValueError):
                pass
            {"jtbl": b["jtbls"], "string": b["strings"], "data": b["data"]}[r["type"]].add(sym)
        b["owners"].add(r["owner_func"])
        if r["owner_status"] in b:
            b[r["owner_status"]] += 1

    print(
        f"\n{'block':<40} {'syms':>4} {'jtb':>3} {'str':>4} {'dat':>3} {'bytes':>6} "
        f"{'owners':>6}  {'stub':>4} {'comp':>4} {'actv':>4} {'auth':>4} {'park':>4} {'orph':>4}"
    )
    print("-" * 120)
    total = defaultdict(int)
    trivial: list[str] = []
    for name in LINKED_BLOCKS:
        b = per_block.get(name)
        if not b:
            trivial.append(name)
            print(f"{name:<40}   (trivial — no dlabel symbols; pure padding / alignment)")
            continue
        print(
            f"{name:<40} {len(b['syms']):>4} {len(b['jtbls']):>3} {len(b['strings']):>4} "
            f"{len(b['data']):>3} {b['bytes']:>6} {len(b['owners']):>6}  "
            f"{b['stub']:>4} {b['completed']:>4} {b['active']:>4} "
            f"{b['authorize']:>4} {b['parked']:>4} {b['orphan']:>4}"
        )
        total["syms"] += len(b["syms"])
        total["jtbls"] += len(b["jtbls"])
        total["strings"] += len(b["strings"])
        total["data"] += len(b["data"])
        total["bytes"] += b["bytes"]
        for k in status_counts_field:
            total[k] += b[k]
    print("-" * 120)
    print(
        f"{'TOTAL (non-trivial)':<40} {total['syms']:>4} {total['jtbls']:>3} {total['strings']:>4} "
        f"{total['data']:>3} {total['bytes']:>6} {'-':>6}  "
        f"{total['stub']:>4} {total['completed']:>4} {total['active']:>4} "
        f"{total['authorize']:>4} {total['parked']:>4} {total['orphan']:>4}"
    )
    if trivial:
        print(
            f"\n{len(trivial)} trivial block(s) require deletion-only retirement "
            "(no symbols; bb2.ld removal alone):"
        )
        for name in trivial:
            print(f"  - {name}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
