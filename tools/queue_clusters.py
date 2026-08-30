#!/usr/bin/env python3
"""Pre-compute warm-start context for the queue: near-duplicate leads,
same-file groups, shared-callee clusters, difficulty proxies.

Reads engine/queue.json + asm/funcs/*.s + memory/grind/<func>/ ledgers and
writes docs/grind/queue-clusters.md (LF). Pure analysis — touches no build
inputs, no engine state. Re-run any time; output is deterministic.

Usage:  python tools/queue_clusters.py  [--json tmp/queue_clusters.json]

Rationale (context-first-before-grinding, token-usage-audit-2026-08-30):
grind sessions re-derive "is there a similar already-known function?" from
scratch each time. This precomputes the answer once so dossiers/workers can
read one table instead of sweeping asm/.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from itertools import combinations
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_DIR = ROOT / "asm" / "funcs"
GRIND_DIR = ROOT / "memory" / "grind"
OUT_MD = ROOT / "docs" / "grind" / "queue-clusters.md"

INSN_RE = re.compile(r"^\s*/\* [0-9A-F]+ [0-9A-F]+ [0-9A-F]{8} \*/\s+(\S+)\s*(.*)$")
JAL_NAME_RE = re.compile(r"^jal\s+([A-Za-z_]\w*)")


def parse_asm(path: Path):
    """Return (mnemonic list, callee set) for one asm/funcs/*.s file."""
    mnems: list[str] = []
    callees: set[str] = set()
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = INSN_RE.match(line)
        if not m:
            continue
        mnem, rest = m.group(1), m.group(2)
        mnems.append(mnem)
        if mnem == "jal":
            name = rest.split()[0] if rest else ""
            if re.match(r"^[A-Za-z_]\w*$", name):
                callees.add(name)
    return mnems, callees


def ngrams(seq: list[str], n: int = 4) -> set[tuple[str, ...]]:
    return {tuple(seq[i : i + n]) for i in range(len(seq) - n + 1)}


def jaccard(a: set, b: set) -> float:
    if not a or not b:
        return 0.0
    return len(a & b) / len(a | b)


def ledger_info(func: str):
    """(floor or None, file count) from memory/grind/<func>/ if present."""
    d = GRIND_DIR / func
    if not d.is_dir():
        return None, 0
    floor = None
    pin = d / "migration_pin.json"
    if pin.is_file():
        try:
            floor = json.loads(pin.read_text(encoding="utf-8")).get("floor")
        except (json.JSONDecodeError, OSError):
            pass
    n_files = sum(1 for p in d.rglob("*") if p.is_file())
    return floor, n_files


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--json", type=Path, default=None, help="also dump raw results as JSON")
    ap.add_argument("--sim-threshold", type=float, default=0.55, help="opcode 4-gram Jaccard cutoff for near-dup pairs")
    ap.add_argument("--top", type=int, default=50, help="max near-dup pairs to report")
    args = ap.parse_args()

    queue = json.loads((ROOT / "engine" / "queue.json").read_text(encoding="utf-8"))
    items = queue["items"] if isinstance(queue, dict) and "items" in queue else queue
    by_func = {it["func"]: it for it in items}
    active = [it for it in items if it.get("status") == "active"]

    # Parse asm for every queue function (any status) + every other asm file
    # (non-queue .s files can still be template donors).
    feats: dict[str, dict] = {}
    for path in sorted(ASM_DIR.glob("*.s")):
        func = path.stem
        mnems, callees = parse_asm(path)
        if not mnems:
            continue
        feats[func] = {
            "n": len(mnems),
            "grams": ngrams(mnems),
            "callees": callees,
            "in_queue": func in by_func,
        }

    missing = [it["func"] for it in items if it["func"] not in feats]

    # --- Near-duplicate pairs: queue member vs any parsed function ---------
    qfuncs = [f for f in feats if feats[f]["in_queue"]]
    others = [f for f in feats if not feats[f]["in_queue"]]
    pairs: list[tuple[float, str, str]] = []
    # queue x queue
    for a, b in combinations(qfuncs, 2):
        fa, fb = feats[a], feats[b]
        if min(fa["n"], fb["n"]) * 2 < max(fa["n"], fb["n"]):
            continue  # length mismatch >2x can't be a near-dup
        s = jaccard(fa["grams"], fb["grams"])
        if s >= args.sim_threshold:
            pairs.append((s, a, b))
    # queue x non-queue donors
    donor_pairs: list[tuple[float, str, str]] = []
    for a in qfuncs:
        fa = feats[a]
        best = (0.0, "")
        for b in others:
            fb = feats[b]
            if min(fa["n"], fb["n"]) * 2 < max(fa["n"], fb["n"]):
                continue
            s = jaccard(fa["grams"], fb["grams"])
            if s > best[0]:
                best = (s, b)
        if best[0] >= args.sim_threshold:
            donor_pairs.append((best[0], a, best[1]))
    pairs.sort(reverse=True)
    donor_pairs.sort(reverse=True)

    # --- Shared-callee affinity (queue x queue) ----------------------------
    callee_pairs: list[tuple[float, int, str, str]] = []
    for a, b in combinations(qfuncs, 2):
        ca, cb = feats[a]["callees"], feats[b]["callees"]
        if len(ca) >= 3 and len(cb) >= 3:
            s = jaccard(ca, cb)
            if s >= 0.5:
                callee_pairs.append((s, len(ca & cb), a, b))
    callee_pairs.sort(reverse=True)

    # --- Same-file groups (active only) ------------------------------------
    groups: dict[str, list[dict]] = {}
    for it in active:
        groups.setdefault(it.get("file", "?"), []).append(it)

    # --- Per-function summary table (active, queue order) ------------------
    rows = []
    for it in active:
        f = it["func"]
        floor, n_ledger = ledger_info(f)
        ft = feats.get(f, {})
        rows.append({
            "func": f,
            "file": it.get("file"),
            "distance": it.get("distance"),
            "verdict": it.get("verdict"),
            "insns": ft.get("n"),
            "ledger_floor": floor,
            "ledger_files": n_ledger,
            "callees": sorted(ft.get("callees", set())),
        })

    # --- Render markdown ----------------------------------------------------
    L: list[str] = []
    L.append("# Queue cluster analysis (auto-generated)")
    L.append("")
    import datetime
    import subprocess
    try:
        head = subprocess.run(["git", "rev-parse", "--short", "HEAD"], cwd=ROOT,
                              capture_output=True, text=True, check=True).stdout.strip()
    except (subprocess.CalledProcessError, OSError):
        head = "unknown"
    L.append("Generated by `tools/queue_clusters.py` — warm-start context per")
    L.append("[[context-first-before-grinding]]. Re-run after queue changes; do not hand-edit.")
    L.append(f"Anchored: {datetime.date.today().isoformat()} @ {head}.")
    L.append(f"Active items: {len(active)} · asm files parsed: {len(feats)}"
             + (f" · queue funcs missing asm: {len(missing)}" if missing else ""))
    L.append("")
    L.append("## Near-duplicate pairs inside the queue")
    L.append("Solving one member gives a proven C template for the other — work them together.")
    L.append("")
    L.append("| sim | func A (insns) | func B (insns) |")
    L.append("|---|---|---|")
    for s, a, b in pairs[: args.top]:
        L.append(f"| {s:.2f} | {a} ({feats[a]['n']}) | {b} ({feats[b]['n']}) |")
    if not pairs:
        L.append("| — | none ≥ threshold | |")
    L.append("")
    L.append("## Best non-queue donor per queue function")
    L.append("Non-queue `asm/funcs/*.s` with highest opcode-shape similarity (template/reference only — donor may be canonical-asm or authorize-bucket).")
    L.append("")
    L.append("| sim | queue func | donor |")
    L.append("|---|---|---|")
    for s, a, b in donor_pairs[: args.top]:
        L.append(f"| {s:.2f} | {a} | {b} |")
    if not donor_pairs:
        L.append("| — | none ≥ threshold | |")
    L.append("")
    L.append("## Shared-callee affinity (≥50% Jaccard, ≥3 callees each)")
    L.append("Same helper cluster ⇒ likely same subsystem/author idiom; context transfers.")
    L.append("")
    L.append("| jaccard | shared | func A | func B |")
    L.append("|---|---|---|---|")
    for s, n, a, b in callee_pairs[:60]:
        L.append(f"| {s:.2f} | {n} | {a} | {b} |")
    if not callee_pairs:
        L.append("| — | — | none | |")
    L.append("")
    L.append("## Same-file groups (active items)")
    L.append("")
    for file, its in sorted(groups.items(), key=lambda kv: -len(kv[1])):
        fl = ", ".join(f"{i['func']}({i.get('distance')})" for i in sorted(its, key=lambda x: x.get("distance") or 0))
        L.append(f"- **{file}** ({len(its)}): {fl}")
    L.append("")
    L.append("## Active queue summary (queue order)")
    L.append("")
    L.append("| func | file | dist | verdict | insns | ledger floor | ledger files |")
    L.append("|---|---|---|---|---|---|---|")
    for r in rows:
        L.append(f"| {r['func']} | {r['file']} | {r['distance']} | {r['verdict']} | "
                 f"{r['insns']} | {r['ledger_floor'] if r['ledger_floor'] is not None else '—'} | {r['ledger_files']} |")
    L.append("")

    OUT_MD.parent.mkdir(parents=True, exist_ok=True)
    with open(OUT_MD, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("\n".join(L) + "\n")
    print(f"wrote {OUT_MD.relative_to(ROOT)}: {len(pairs)} in-queue near-dup pairs, "
          f"{len(donor_pairs)} donor leads, {len(callee_pairs)} callee-affinity pairs, "
          f"{len(groups)} file groups")

    if args.json:
        payload = {
            "near_dup_pairs": [(round(s, 3), a, b) for s, a, b in pairs],
            "donor_pairs": [(round(s, 3), a, b) for s, a, b in donor_pairs],
            "callee_pairs": [(round(s, 3), n, a, b) for s, n, a, b in callee_pairs],
            "rows": rows,
        }
        args.json.parent.mkdir(parents=True, exist_ok=True)
        with open(args.json, "w", encoding="utf-8", newline="\n") as fh:
            json.dump(payload, fh, indent=1)
        print(f"wrote {args.json}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
