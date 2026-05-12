#!/usr/bin/env python3
"""audit_kengo_renames.py -- glance-level suspicion ranking of every
Kengo-derived BB2 rename.

Computes a suspicion score per rename and sorts highest-first so the
likely false positives surface at the top. Signals:

  - size_delta:  |bb2_size - kengo_size| / max(bb2_size, kengo_size)
                 0.0 = identical, >0.20 = >20% diff = suspect
  - group_size:  count of other BB2 funcs claiming the same Kengo name
                 (TU-static clusters of N>=5 with no strong-tier anchor
                 are statistically dubious)
  - confidence:  weight by tier; seq-similarity opseq_ratio matters most
  - opseq_low:   bool flag if seq-similarity ratio < 0.40

Run with:
  python3 tools/audit_kengo_renames.py
  python3 tools/audit_kengo_renames.py --top 50         # only worst N
  python3 tools/audit_kengo_renames.py --suspect-only   # only suspect score >=2
  python3 tools/audit_kengo_renames.py --decided-only   # reviewed action queue
  python3 tools/audit_kengo_renames.py --include-suppressed
"""
from __future__ import annotations

import argparse
import csv
import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_FUNCS = ROOT / "asm" / "funcs"
CSV_PATH = ROOT / "kengo_matches.csv"
# Snapshot of the CSV before the matcher re-run (commit fa35f5f). The
# live CSV got rekeyed by suffixed BB2 names and reports `name-unique`
# for everything because the suffix-stripper finds the bare Kengo name.
# We want the ORIGINAL confidence that drove each rename — that's in
# the pre-iteration snapshot.
CSV_PRE_PATH = ROOT / "tmp" / "kengo_matches_pre.csv"
DECISIONS_PATH = ROOT / "kengo_name_decisions.csv"
SUPPRESSED_DECISIONS = {"keep", "byte-sensitive"}


def addr_re(line: str) -> int | None:
    m = re.search(r"\b80([0-9A-Fa-f]{6})\b", line)
    return int(f"80{m.group(1)}", 16) if m else None


def load_renames() -> list[dict]:
    """For each renamed asm/funcs/<name>.s where name != 'func_*', pull
    canonical address from the second line and match against CSV."""
    addr_to_renamed: dict[int, str] = {}
    for p in sorted(ASM_FUNCS.glob("*.s")):
        name = p.stem
        if name.startswith("func_") or name.startswith("D_"):
            continue
        lines = p.read_text(encoding="utf-8", errors="replace").splitlines()
        if len(lines) < 2:
            continue
        a = addr_re(lines[1])
        if a:
            addr_to_renamed[a] = name

    csv_rows: dict[int, dict] = {}
    src_csv = CSV_PRE_PATH if CSV_PRE_PATH.exists() else CSV_PATH
    for r in csv.DictReader(src_csv.open()):
        m = re.match(r"^func_([0-9A-Fa-f]{8})$", r["bb2_func"])
        if m and r["kengo_name"]:
            csv_rows[int(m.group(1), 16)] = r

    out: list[dict] = []
    for addr, name in addr_to_renamed.items():
        r = csv_rows.get(addr)
        if r:
            r = dict(r)
            r["renamed_to"] = name
            r["addr"] = addr
            out.append(r)
    return out


def load_decisions(path: Path) -> dict[str, dict]:
    """Load reviewed naming decisions keyed by current renamed symbol."""
    if not path.exists():
        return {}

    decisions: dict[str, dict] = {}
    with path.open(newline="", encoding="utf-8") as f:
        for row in csv.DictReader(f):
            name = (row.get("renamed_to") or "").strip()
            if not name:
                continue
            decisions[name] = {
                "decision": (row.get("decision") or "").strip(),
                "confidence": (row.get("confidence") or "").strip(),
                "reason": (row.get("reason") or "").strip(),
                "next_action": (row.get("next_action") or "").strip(),
            }
    return decisions


# Suspicion scoring weights (higher = more suspect).
TIER_WEIGHT = {
    "name-unique":        0,
    "name-callgraph":     0,
    "callgraph":          0,
    "caller-unique":      1,
    "caller-callgraph":   1,
    "affinity-unique":    2,
    "affinity-callgraph": 2,
    "seq-similarity":     2,
    "size-only-unique":   3,
    "size-only-ambiguous":4,
}


def score_one(r: dict, group_count: int) -> tuple[float, list[str]]:
    score = 0.0
    reasons: list[str] = []

    # Size delta
    try:
        bs = int(r["bb2_insns"])
        ks = int(r["kengo_insns"])
        if max(bs, ks) > 0:
            delta = abs(bs - ks) / max(bs, ks)
            if delta >= 0.30:
                score += 3
                reasons.append(f"size_delta={delta:.2f}")
            elif delta >= 0.20:
                score += 2
                reasons.append(f"size_delta={delta:.2f}")
            elif delta >= 0.10:
                score += 1
                reasons.append(f"size_delta={delta:.2f}")
    except (KeyError, ValueError):
        pass

    # Confidence tier
    conf = r.get("confidence", "")
    tier_w = TIER_WEIGHT.get(conf, 2)
    if tier_w > 0:
        score += tier_w
        reasons.append(f"tier={conf}")

    # seq-similarity ratio
    if conf == "seq-similarity":
        try:
            ratio = float(r.get("opseq_ratio") or 0)
            if ratio < 0.40:
                score += 2
                reasons.append(f"opseq={ratio:.2f}")
            elif ratio < 0.50:
                score += 1
                reasons.append(f"opseq={ratio:.2f}")
        except ValueError:
            pass

    # Group size (number of BB2 funcs sharing the bare Kengo name)
    if group_count >= 10:
        score += 3
        reasons.append(f"group={group_count}")
    elif group_count >= 5:
        score += 2
        reasons.append(f"group={group_count}")
    elif group_count >= 3:
        score += 1
        reasons.append(f"group={group_count}")

    # Tiny functions (3-5 insns) are statistically degenerate
    try:
        bs = int(r["bb2_insns"])
        if bs <= 4:
            score += 1
            reasons.append(f"tiny={bs}")
    except (KeyError, ValueError):
        pass

    return score, reasons


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--top", type=int, default=0,
                    help="Only show top N most suspect (default: all)")
    ap.add_argument("--suspect-only", action="store_true",
                    help="Only show entries with suspicion score >= 2")
    ap.add_argument("--csv", action="store_true",
                    help="CSV output for spreadsheet review")
    ap.add_argument("--decisions", type=Path, default=DECISIONS_PATH,
                    help=f"Decision CSV path (default: {DECISIONS_PATH.relative_to(ROOT)})")
    ap.add_argument("--include-suppressed", action="store_true",
                    help="Include decision=keep/byte-sensitive rows in output")
    ap.add_argument("--decided-only", action="store_true",
                    help="Only show rows that have a loaded decision")
    args = ap.parse_args()

    rows = load_renames()
    if not rows:
        print("No renames detected — is asm/funcs/ regenerated?", file=sys.stderr)
        return 1
    decisions = load_decisions(args.decisions)

    # Group by bare Kengo name (strip _<ADDR> suffix).
    SUFFIX = re.compile(r"_[0-9A-Fa-f]{8}$")
    group_count: dict[str, int] = defaultdict(int)
    for r in rows:
        bare = SUFFIX.sub("", r["renamed_to"])
        group_count[bare] += 1

    scored: list[dict] = []
    for r in rows:
        bare = SUFFIX.sub("", r["renamed_to"])
        n = group_count[bare]
        s, reasons = score_one(r, n)
        decision = decisions.get(r["renamed_to"], {})
        r["_score"] = s
        r["_reasons"] = ", ".join(reasons)
        r["_group_n"] = n
        r["_decision"] = decision.get("decision", "")
        r["_decision_confidence"] = decision.get("confidence", "")
        r["_decision_reason"] = decision.get("reason", "")
        r["_next_action"] = decision.get("next_action", "")
        scored.append(r)

    scored.sort(key=lambda r: -r["_score"])
    all_scored = list(scored)
    if not args.include_suppressed:
        scored = [r for r in scored if r["_decision"] not in SUPPRESSED_DECISIONS]
    if args.decided_only:
        scored = [r for r in scored if r["_decision"]]
    if args.suspect_only:
        scored = [r for r in scored if r["_score"] >= 2]
    if args.top > 0:
        scored = scored[:args.top]

    if args.csv:
        w = csv.writer(sys.stdout)
        w.writerow(["score", "renamed_to", "bb2_addr", "bb2_insns",
                    "kengo_insns", "confidence", "opseq_ratio",
                    "group", "decision", "decision_confidence",
                    "reasons", "decision_reason", "next_action"])
        for r in scored:
            w.writerow([
                f"{r['_score']:.0f}", r["renamed_to"],
                f"0x{r['addr']:08X}", r["bb2_insns"], r["kengo_insns"],
                r["confidence"], r.get("opseq_ratio", ""), r["_group_n"],
                r["_decision"], r["_decision_confidence"], r["_reasons"],
                r["_decision_reason"], r["_next_action"],
            ])
        return 0

    print(f"{'sc':>3s}  {'renamed_to':<48s}  {'BB2':>4s}/{'Kg':<4s}  "
          f"{'group':>5s}  {'conf':<22s}  {'decision':<12s}  reasons")
    print("-" * 146)
    for r in scored:
        bb_k = f"{r['bb2_insns']}/{r['kengo_insns']}"
        print(f"{r['_score']:>3.0f}  {r['renamed_to']:<48s}  {bb_k:<9s}  "
              f"{r['_group_n']:>5d}  {r['confidence']:<22s}  "
              f"{r['_decision']:<12s}  {r['_reasons']}")

    print()
    suppressed = sum(1 for r in all_scored if r["_decision"] in SUPPRESSED_DECISIONS)
    print(f"Total renames audited: {len(all_scored)}")
    print(f"  decisions loaded: {len(decisions)}")
    if not args.include_suppressed:
        print(f"  suppressed reviewed keep/byte-sensitive rows: {suppressed}")
    print(f"  rows shown: {len(scored)}")
    print(f"  shown suspect-score >=4:  "
          f"{sum(1 for r in scored if r['_score']>=4)}")
    print(f"  shown suspect-score 2-3: "
          f"{sum(1 for r in scored if 2 <= r['_score'] < 4)}")
    print(f"  shown suspect-score 0-1: "
          f"{sum(1 for r in scored if r['_score'] < 2)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
