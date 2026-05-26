#!/usr/bin/env python3
"""scan_canonical_asm.py -- unified canonical-asm classifier.

For each function in asm/funcs/ plus inline-asm functions found in src/*.c,
classifies into one of four tiers and cross-checks the result against the
two manual registries (inline_asm_canonical.txt, known_blocked.txt) to
report drift in both directions.

Tiers
-----

  CERTAIN_ASM  (Tier 1, 100% confidence)
      Contains an instruction GCC 2.7.2 physically cannot emit:
        cop0_op                 mfc0/mtc0/ctc0/cfc0/rfe/tlb*
        syscall                 raw syscall instruction
        break_instruction       break with non-div-guard code (not 6, not 7)
        handwritten_overflow_op add / addi / sub (overflow-trapping forms)
        sp_manipulation         $sp swap or context save/restore
        reserved_register       $k0 / $k1 (kernel-only; never produced by GCC)
        no_return_fragment      no jr/j/b* -- orphan code / data-as-code

  GTE_MIGRATE_CANDIDATE
      Function uses COP2/GTE instructions but has NO other Tier 1 or Tier 2
      signal. Almost always C+gte.h macros in the original source --
      splat dumps the macro expansion as inline asm, so the byte-level
      output looks identical to hand-coded asm, but the structural shape
      (matrix load + per-vertex transform loop, mvmva opcodes, stmac
      extraction sequences) is textbook PsyQ SDK macro usage. Do NOT add
      to inline_asm_canonical.txt; route to `dc.sh gte-migrate <func>`
      which converts the body to C+macros (same bytes, authentic source).

  LIKELY_ASM  (Tier 2, very high confidence)
      GCC won't naturally emit, but a sufficiently determined C author
      could in theory; in practice, all instances we have observed are
      hand-coded asm:
        bios_trampoline         addiu $tN,$zero,0xA0|B0|C0 ; jr $tN ;
                                addiu $t1,$zero,FUNCNO  (function-ID
                                pinned into the indirect-call delay slot)
        absolute_jal            jal to a numeric absolute address rather
                                than a symbol (not produced by the
                                PsyQ linker / GCC for ordinary calls)

  PURE_C_CANDIDATE
      No Tier 1 or Tier 2 signal. Normal decomp workflow applies.
      Heuristic ("hand-coded but no proof") signals are reported separately
      by `tools/scan_hand_coded.py` -- this tool does not promote on them.

  ALREADY_MATCHED
      Function body exists as pure C in src/ (find_in_src reports
      kind=c_function). Out of scope for further classification.

Registry cross-check
--------------------

  MISSING -- function is Tier 1 or Tier 2 but is on NEITHER
             inline_asm_canonical.txt nor known_blocked.txt. Candidate
             for auto-promotion to the appropriate registry.

  UNEVIDENCED -- function is in one of the registries but Tier 1/2
                 detection found no qualifying signal. Two non-error
                 reasons this can happen:
                   * The body is hand-coded but the *individual
                     instructions* are all GCC-emittable (Tier 3 only --
                     the entries authorized via feedback_hand_coded_*).
                   * Detector blind-spot worth investigating.

Outputs
-------

Default
  Summary to stdout; full CSV to tmp/canonical_asm_scan.csv.

--diff
  Also writes tmp/canonical_asm_diff.txt with proposed appends to
  inline_asm_canonical.txt and known_blocked.txt. Never overwrites
  the registries -- review and merge by hand.

--json
  Emit the full per-function classification dict to stdout.

--single FUNC
  Classify exactly one function (skips the cross-check report).

Exit codes
  0  scan completed; report consistent
  1  scan completed but MISSING or UNEVIDENCED entries present
  2  internal error (parse failure, missing tools)
"""
from __future__ import annotations

import argparse
import csv
import json
import re
import sys
from collections import Counter
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
sys.path.insert(0, str(TOOLS))
import classify_func as cf  # type: ignore

ROOT = TOOLS.parent
ASM_FUNCS = ROOT / "asm" / "funcs"
SRC_DIR = ROOT / "src"
TMP_DIR = ROOT / "tmp"

CANONICAL_TXT = ROOT / "inline_asm_canonical.txt"
KNOWN_BLOCKED_TXT = ROOT / "known_blocked.txt"

# --- Tier 1 detectors not already covered by classify_func -----------------

RESERVED_REGS = ("$k0", "$26", "$k1", "$27")
RESERVED_REG_RE = re.compile(
    r"(?<![\w$])" + "|".join(re.escape(r) for r in RESERVED_REGS) + r"(?![\w])"
)

# Coprocessor mnemonics whose register operands are coprocessor regs, NOT
# GPRs. Splat prints both as "$N", so $26/$27 on a cop2 insn means cop2
# register 26/27, not GPR $k0/$k1. Skip these when looking for kernel-reg
# usage.
COP_MOVE_OPS = {
    "mfc0", "mtc0", "cfc0", "ctc0",
    "mfc2", "mtc2", "cfc2", "ctc2",
    "lwc2", "swc2",
} | cf.GTE_OPS


def detect_reserved_register(insns) -> str | None:
    """Return a short evidence string if any instruction operand mentions
    a kernel-reserved GPR ($k0/$k1). $at is *not* flagged here because
    pseudo-instruction expansions legitimately use it. Coprocessor moves
    are skipped because their operand registers are coprocessor regs, not
    GPRs (e.g. `mtc2 $a3, $26` references cop2 reg 26, not GPR $k0)."""
    for _, mnem, ops in insns:
        if mnem in COP_MOVE_OPS:
            continue
        if RESERVED_REG_RE.search(ops):
            return f"{mnem} {ops}".strip()
    return None


def detect_cop2(insns) -> str | None:
    """Return the first COP2/GTE mnemonic in the function, if any."""
    for _, mnem, _ in insns:
        if mnem in cf.GTE_OPS:
            return mnem
    return None


# --- Tier 2 detectors -------------------------------------------------------

ABS_JAL_RE = re.compile(r"^\s*0x[0-9A-Fa-f]+\s*$")


def detect_absolute_jal(insns) -> str | None:
    for _, mnem, ops in insns:
        if mnem != "jal":
            continue
        if ABS_JAL_RE.match(ops):
            return ops.strip()
    return None


# BIOS-jumptable handler installer (broader than is_bios_trampoline, which
# only scans the first 6 insns). Pattern anywhere in the body:
#   addiu $tN, $zero, 0xA0|0xB0|0xC0    # BIOS dispatcher address
#   jalr $tN  (or jr $tN)               # call dispatcher
#   addiu $tM, $zero, <small>           # function-ID pinned in delay slot
# GCC 2.7.2 has no syntax to demand a specific register-load in an indirect
# call's delay slot. Even one match is decisive.
BIOS_TABLE_ADDRS = (0xA0, 0xB0, 0xC0)
_JUMP_TARGET_REG_RE = re.compile(r"^(t\d|k[01])$")
_DELAY_REG_RE = re.compile(r"^(t\d|k[01]|a[0-3]|v[01])$")
_REG_IMM_RE = re.compile(r"^\$(\w+)\s*,\s*(?:\$zero\s*,\s*)?(.+)$")


def _parse_mips_imm(s: str) -> int | None:
    s = s.split("#")[0].split("(")[0].split("/*")[0].strip().rstrip(",")
    if not s:
        return None
    try:
        return int(s, 0)
    except ValueError:
        return None


def detect_bios_jumptable(insns) -> str | None:
    """Anywhere-in-body BIOS dispatcher pattern (see comment above)."""
    for i, (_, mnem, ops) in enumerate(insns):
        if mnem not in ("jalr", "jr"):
            continue
        regs = re.findall(r"\$(\w+)", ops)
        if not regs:
            continue
        jump_reg = regs[-1]
        if not _JUMP_TARGET_REG_RE.match(jump_reg):
            continue
        # Walk back up to 8 insns looking for the load of $jump_reg with
        # one of the BIOS table addresses.
        loaded: int | None = None
        for j in range(i - 1, max(-1, i - 9), -1):
            _, pm, pops = insns[j]
            if pm not in ("addiu", "li", "ori"):
                continue
            m = _REG_IMM_RE.match(pops)
            if not m or m.group(1) != jump_reg:
                continue
            val = _parse_mips_imm(m.group(2))
            if val is None:
                break
            if val in BIOS_TABLE_ADDRS:
                loaded = val
            break
        if loaded is None:
            continue
        if i + 1 >= len(insns):
            continue
        _, dm, dops = insns[i + 1]
        if dm not in ("addiu", "li"):
            continue
        m = _REG_IMM_RE.match(dops)
        if not m:
            continue
        delay_reg = m.group(1)
        if delay_reg == jump_reg or not _DELAY_REG_RE.match(delay_reg):
            continue
        delay_val = _parse_mips_imm(m.group(2))
        if delay_val is None or not (0 <= delay_val <= 0xFF):
            continue
        kind = "jalr" if mnem == "jalr" else "jr"
        return f"bios_0x{loaded:X}({delay_val:#x}) via {kind} ${jump_reg}"
    return None


# --- Classification ---------------------------------------------------------

TIER_CERTAIN = "CERTAIN_ASM"
TIER_GTE_MIGRATE = "GTE_MIGRATE_CANDIDATE"
TIER_LIKELY = "LIKELY_ASM"
TIER_PURE_C = "PURE_C_CANDIDATE"
TIER_MATCHED = "ALREADY_MATCHED"

# Permanent-canonical Tier 1 signals (in headline priority order). A
# function with any of these is CERTAIN_ASM no matter what else is found.
TIER1_PERMANENT_KEYS = (
    "cop0_op",
    "syscall",
    "syscall_instruction",
    "break_instruction",
    "handwritten_overflow_op",
    "sp_manipulation",
    "reserved_register",
    "no_return_fragment",
)

# cop2_function is sub-categorized: ONLY-cop2 functions are GTE-migrate
# candidates (originally C+gte.h, retire via dc.sh gte-migrate). cop2 +
# any permanent Tier 1 or Tier 2 signal -> stays CERTAIN_ASM /
# LIKELY_ASM as gated by that other signal.
TIER2_KEYS = ("bios_trampoline", "bios_jumptable", "absolute_jal")


def classify_function(func: str) -> dict:
    out: dict = {
        "func": func,
        "tier": None,
        "headline_reason": None,
        "signals": [],
        "in_canonical": False,
        "in_blocked": False,
        "src_kind": None,
        "src_file": None,
        "asm_lines": 0,
    }

    src = cf.find_in_src(func)
    out["src_kind"] = src.get("kind")
    out["src_file"] = src.get("file")

    if src.get("kind") == "c_function":
        out["tier"] = TIER_MATCHED
        out["headline_reason"] = "c_function_in_src"
        return out

    insns = cf.load_asm_lines(func)
    if not insns:
        out["tier"] = TIER_PURE_C
        out["headline_reason"] = "no_asm_found"
        return out
    out["asm_lines"] = len(insns)

    signals: list[dict] = []

    perm = cf.detect_permanent_blockers(insns)
    for kind in perm:
        if kind == "no_return_fragment":
            signals.append({"kind": kind, "evidence": f"{len(insns)}-insn fragment, no jr/j/b*"})
            continue
        evidence = _evidence_for_blocker(kind, insns)
        signals.append({"kind": kind, "evidence": evidence})

    rr = detect_reserved_register(insns)
    if rr:
        signals.append({"kind": "reserved_register", "evidence": rr})

    gte = detect_cop2(insns)
    if gte:
        signals.append({"kind": "cop2_function", "evidence": f"{gte} present"})

    is_bios, bios_kind = cf.is_bios_trampoline(insns)
    if is_bios:
        signals.append({"kind": "bios_trampoline", "evidence": bios_kind or "bios_trampoline"})
    else:
        # Broader pattern: BIOS-jumptable call anywhere in body, with the
        # function-ID register pinned in the indirect-call's delay slot.
        # GCC has no syntax to demand this; even one match is decisive.
        bjt = detect_bios_jumptable(insns)
        if bjt:
            signals.append({"kind": "bios_jumptable", "evidence": bjt})

    abs_jal = detect_absolute_jal(insns)
    if abs_jal:
        signals.append({"kind": "absolute_jal", "evidence": f"jal {abs_jal}"})

    out["signals"] = signals

    tier1_permanent = [s for s in signals if s["kind"] in TIER1_PERMANENT_KEYS]
    has_cop2 = any(s["kind"] == "cop2_function" for s in signals)
    tier2 = [s for s in signals if s["kind"] in TIER2_KEYS]

    if tier1_permanent:
        out["tier"] = TIER_CERTAIN
        for k in TIER1_PERMANENT_KEYS:
            if any(s["kind"] == k for s in tier1_permanent):
                out["headline_reason"] = k
                break
    elif tier2:
        # Tier 2 gates over cop2 -- a BIOS trampoline that happens to also
        # touch the GTE is canonical-asm, not gte-migrate-eligible.
        out["tier"] = TIER_LIKELY
        out["headline_reason"] = tier2[0]["kind"]
    elif has_cop2:
        out["tier"] = TIER_GTE_MIGRATE
        out["headline_reason"] = "cop2_function"
    else:
        out["tier"] = TIER_PURE_C
        out["headline_reason"] = None

    return out


def _evidence_for_blocker(kind: str, insns) -> str:
    if kind == "cop0_op":
        for _, m, ops in insns:
            if m in cf.COP0_OPS:
                return f"{m} {ops}".strip()
    if kind == "break_instruction":
        for _, m, ops in insns:
            if m == "break" and cf.break_code(ops) not in cf.GCC_DIV_BREAK_CODES:
                return f"break {ops}".strip()
    if kind in ("syscall", "syscall_instruction"):
        for _, m, ops in insns:
            if m == "syscall":
                return "syscall"
            if m == ".word":
                k = cf.raw_syscall_break_kind(ops)
                if k:
                    return f".word {ops}".strip()
    if kind == "handwritten_overflow_op":
        for _, m, ops in insns:
            if m in ("add", "addi", "sub"):
                return f"{m} {ops}".strip()
    if kind == "sp_manipulation":
        for _, m, ops in insns:
            if m in ("addu", "or", "move", "subu", "lw", "lwl", "lwr") and "$sp" in ops:
                return f"{m} {ops}".strip()
    return ""


# --- Registry parsing -------------------------------------------------------

def load_canonical() -> set[str]:
    out: set[str] = set()
    if not CANONICAL_TXT.exists():
        return out
    for raw in CANONICAL_TXT.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        name = line.split("#", 1)[0].strip()
        if name:
            out.add(name)
    return out


def load_known_blocked() -> set[str]:
    return set(cf.load_known_blocked().keys())


# --- Function enumeration ---------------------------------------------------

INLINE_ASM_GLABEL_RE = re.compile(r'glabel\s+(\w+)\b')


def enumerate_functions() -> list[str]:
    """Real functions only: files in asm/funcs/ whose body begins with
    `glabel` (not `dlabel` -- splat dumps data fragments to .s files too),
    plus any inline-asm glabel blocks in src/*.c."""
    names: set[str] = set()
    for p in ASM_FUNCS.glob("*.s"):
        try:
            head = p.read_text(encoding="utf-8", errors="ignore")[:512]
        except OSError:
            continue
        if re.search(r"^\s*glabel\s+\w+", head, re.MULTILINE):
            names.add(p.stem)
    for src in SRC_DIR.glob("*.c"):
        try:
            text = src.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue
        for blk in re.finditer(r"__asm__\s*\((.*?)\)\s*;", text, re.DOTALL):
            for m in INLINE_ASM_GLABEL_RE.finditer(blk.group(1)):
                names.add(m.group(1))
    return sorted(names)


# --- Reporting --------------------------------------------------------------

def write_csv(rows: list[dict], path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as fh:
        w = csv.writer(fh)
        w.writerow([
            "func", "tier", "headline_reason", "signals",
            "in_canonical", "in_blocked",
            "src_kind", "src_file", "asm_lines",
        ])
        for r in rows:
            signals_str = "; ".join(f"{s['kind']}={s['evidence']}" for s in r["signals"])
            w.writerow([
                r["func"], r["tier"], r["headline_reason"] or "", signals_str,
                "1" if r["in_canonical"] else "",
                "1" if r["in_blocked"] else "",
                r["src_kind"] or "", r["src_file"] or "", r["asm_lines"],
            ])


def _summary_counts(rows: list[dict]) -> dict:
    c = Counter(r["tier"] for r in rows)
    headlines = Counter()
    for r in rows:
        if r["tier"] == TIER_CERTAIN and r["headline_reason"]:
            headlines[r["headline_reason"]] += 1
    return {"tiers": dict(c), "tier1_headlines": dict(headlines)}


def _registry_audit(rows: list[dict]) -> dict:
    missing_canonical: list[dict] = []
    missing_blocked: list[dict] = []
    missing_gte_migrate: list[dict] = []
    unevidenced_canonical: list[str] = []
    unevidenced_blocked: list[str] = []
    miscategorized_gte_on_canonical: list[str] = []

    for r in rows:
        if r["tier"] == TIER_MATCHED:
            continue

        if r["tier"] == TIER_GTE_MIGRATE:
            # GTE-only functions: should be routed to `dc.sh gte-migrate`,
            # NOT added to either registry. If they're already on
            # canonical-list, the user has made a deliberate call -- the
            # canonical-list comment may document a non-instruction-level
            # reason (custom ABI, hand-scheduled pacing) that the scanner
            # can't see. Flag those for review separately from the clean
            # gte-migrate work list.
            entry = {
                "func": r["func"],
                "tier": r["tier"],
                "headline": r["headline_reason"],
                "signals": r["signals"],
            }
            if r["in_canonical"]:
                miscategorized_gte_on_canonical.append(r["func"])
            else:
                missing_gte_migrate.append(entry)
            continue

        if r["tier"] in (TIER_CERTAIN, TIER_LIKELY) and not (r["in_canonical"] or r["in_blocked"]):
            entry = {
                "func": r["func"],
                "tier": r["tier"],
                "headline": r["headline_reason"],
                "signals": r["signals"],
            }
            if r["headline_reason"] in (
                "cop0_op", "syscall", "syscall_instruction",
                "break_instruction", "handwritten_overflow_op",
                "sp_manipulation", "no_return_fragment",
            ):
                missing_blocked.append(entry)
            else:
                missing_canonical.append(entry)

        if r["in_canonical"] and r["tier"] == TIER_PURE_C:
            unevidenced_canonical.append(r["func"])
        if r["in_blocked"] and r["tier"] == TIER_PURE_C:
            unevidenced_blocked.append(r["func"])

    return {
        "missing_canonical": missing_canonical,
        "missing_blocked": missing_blocked,
        "missing_gte_migrate": missing_gte_migrate,
        "unevidenced_canonical": unevidenced_canonical,
        "unevidenced_blocked": unevidenced_blocked,
        "miscategorized_gte_on_canonical": miscategorized_gte_on_canonical,
    }


def print_summary(rows: list[dict], audit: dict) -> None:
    summary = _summary_counts(rows)
    total = sum(summary["tiers"].values())
    print("=== BB2 Canonical-Asm Scanner ===")
    print(f"Scanned {total} functions.")
    print()
    for tier in (TIER_CERTAIN, TIER_GTE_MIGRATE, TIER_LIKELY, TIER_PURE_C, TIER_MATCHED):
        n = summary["tiers"].get(tier, 0)
        print(f"  {tier:22s} {n:5d}")
    print()
    if summary["tier1_headlines"]:
        print("CERTAIN_ASM breakdown by headline reason:")
        for k, v in sorted(summary["tier1_headlines"].items(), key=lambda x: -x[1]):
            print(f"  {k:28s} {v:5d}")
        print()

    print("Registry consistency:")
    print(f"  inline_asm_canonical.txt  entries: {sum(1 for r in rows if r['in_canonical'])}")
    print(f"  known_blocked.txt         entries: {sum(1 for r in rows if r['in_blocked'])}")
    print()

    if audit["missing_canonical"] or audit["missing_blocked"]:
        print("MISSING -- Tier 1/2 detected but not in any registry:")
        for e in audit["missing_blocked"]:
            print(f"  [BLOCKED]   {e['func']:30s} {e['headline']}")
        for e in audit["missing_canonical"]:
            print(f"  [CANONICAL] {e['func']:30s} {e['headline']}")
        print()
    else:
        print("MISSING: none.")
        print()

    if audit["missing_gte_migrate"]:
        print("GTE_MIGRATE -- COP2-only functions; retire via `dc.sh gte-migrate <func>`:")
        for e in audit["missing_gte_migrate"]:
            print(f"  [gte-migrate] {e['func']:30s} {e['headline']}")
        print()

    if audit["miscategorized_gte_on_canonical"]:
        print("REVIEW -- on inline_asm_canonical.txt with only a cop2_function signal:")
        print("  (may be miscategorized OR may have a Tier-3 reason the scanner can't see;")
        print("   check the canonical-list comment for each)")
        for f in audit["miscategorized_gte_on_canonical"]:
            print(f"  [review] {f}")
        print()

    if audit["unevidenced_canonical"] or audit["unevidenced_blocked"]:
        print("UNEVIDENCED -- on a registry but no Tier 1/2 signal:")
        for f in audit["unevidenced_canonical"]:
            print(f"  [canonical] {f}  (likely Tier-3 hand-coded; manual review)")
        for f in audit["unevidenced_blocked"]:
            print(f"  [blocked]   {f}  (CHECK -- registry says blocked but no signal found)")
        print()


def write_diff(audit: dict, out_path: Path) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)
    lines: list[str] = []
    lines.append("# Proposed additions -- review before merging into the registries.\n")
    lines.append("# Generated by tools/scan_canonical_asm.py\n\n")

    if audit["missing_blocked"]:
        lines.append("# --- append to known_blocked.txt ---\n")
        for e in audit["missing_blocked"]:
            ev = next((s["evidence"] for s in e["signals"]
                       if s["kind"] == e["headline"]), "")
            lines.append(f"{e['func']:30s}{e['headline']:24s}{ev}\n")
        lines.append("\n")

    if audit["missing_canonical"]:
        lines.append("# --- append to inline_asm_canonical.txt ---\n")
        for e in audit["missing_canonical"]:
            signals_summary = ", ".join(s["kind"] for s in e["signals"])
            lines.append(f"{e['func']}  # auto-detected: {signals_summary}\n")
        lines.append("\n")

    if audit["missing_gte_migrate"]:
        lines.append("# --- gte-migrate candidates (do NOT add to inline_asm_canonical.txt) ---\n")
        lines.append("# These functions are COP2-only with no other Tier 1/2 signal.\n")
        lines.append("# Originally C+gte.h macros; retire each via:\n")
        lines.append("#   bash tools/dc.sh gte-migrate <func> --setup\n")
        lines.append("# Then edit permuter/<func>/base.c and integrate the C body.\n")
        for e in audit["missing_gte_migrate"]:
            lines.append(f"# {e['func']}\n")
        lines.append("\n")

    if audit["miscategorized_gte_on_canonical"]:
        lines.append("# --- review: canonical-list entries with only a cop2_function signal ---\n")
        lines.append("# May be miscategorized (should be gte-migrated, not frozen as asm), OR\n")
        lines.append("# the canonical-list comment may document a Tier-3 reason the scanner\n")
        lines.append("# can't detect (custom ABI, hand-scheduled pacing). Check the comment.\n")
        for f in audit["miscategorized_gte_on_canonical"]:
            lines.append(f"# {f}\n")
        lines.append("\n")

    if audit["unevidenced_canonical"]:
        lines.append("# --- review: in inline_asm_canonical.txt but no Tier 1/2 signal ---\n")
        for f in audit["unevidenced_canonical"]:
            lines.append(f"# {f}\n")
        lines.append("\n")

    if audit["unevidenced_blocked"]:
        lines.append("# --- review: in known_blocked.txt but no Tier 1/2 signal ---\n")
        for f in audit["unevidenced_blocked"]:
            lines.append(f"# {f}\n")
        lines.append("\n")

    out_path.write_text("".join(lines), encoding="utf-8")


# --- CLI -------------------------------------------------------------------

def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    g = ap.add_mutually_exclusive_group()
    g.add_argument("--single", metavar="FUNC",
                   help="Classify exactly one function and emit JSON.")
    g.add_argument("--all", action="store_true",
                   help="Scan every function (default).")
    ap.add_argument("--csv", type=Path, default=TMP_DIR / "canonical_asm_scan.csv",
                    help="Write full per-function CSV here (default: tmp/canonical_asm_scan.csv).")
    ap.add_argument("--diff", action="store_true",
                    help="Write proposed registry diffs to tmp/canonical_asm_diff.txt.")
    ap.add_argument("--json", action="store_true",
                    help="Emit full per-function classification as JSON.")
    ap.add_argument("--no-csv", action="store_true",
                    help="Skip CSV output.")
    args = ap.parse_args()

    canonical = load_canonical()
    blocked = load_known_blocked()

    if args.single:
        result = classify_function(args.single)
        result["in_canonical"] = args.single in canonical
        result["in_blocked"] = args.single in blocked
        print(json.dumps(result, indent=2))
        return 0

    funcs = enumerate_functions()
    if not funcs:
        print("No functions found in asm/funcs/ or src/.", file=sys.stderr)
        return 2

    rows: list[dict] = []
    for f in funcs:
        r = classify_function(f)
        r["in_canonical"] = f in canonical
        r["in_blocked"] = f in blocked
        rows.append(r)

    if not args.no_csv:
        write_csv(rows, args.csv)

    audit = _registry_audit(rows)

    if args.json:
        print(json.dumps({"rows": rows, "audit": audit}, indent=2))
        return 0

    print_summary(rows, audit)
    if not args.no_csv:
        print(f"Full CSV: {args.csv.relative_to(ROOT)}")

    if args.diff:
        diff_path = TMP_DIR / "canonical_asm_diff.txt"
        write_diff(audit, diff_path)
        print(f"Diff:     {diff_path.relative_to(ROOT)}")

    inconsistent = bool(audit["missing_canonical"] or audit["missing_blocked"]
                        or audit["missing_gte_migrate"]
                        or audit["miscategorized_gte_on_canonical"]
                        or audit["unevidenced_canonical"] or audit["unevidenced_blocked"])
    return 1 if inconsistent else 0


if __name__ == "__main__":
    sys.exit(main())
