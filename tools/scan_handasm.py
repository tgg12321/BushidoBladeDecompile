#!/usr/bin/env python3
"""Scan asm/funcs/*.s for hand-written-asm signatures.

Hand-written asm tends to use patterns that natural cc1 output doesn't:
  * double-bgez: bgez before AND after negu (INT_MIN handling)
  * consistent multu/mflo 2-instruction padding (always exactly 2 nops between)
  * heavy use of $t7/$t8/$t9 (cc1 prefers $v0/$v1/$a0-$a3/$t0-$t2)
  * dead arg-saves (addu $t<N>, $a<M>, $0 with $t<N> never re-read in func)

A function with 2+ of these signatures is a strong candidate for
replace_with_asmfile rather than pure-C decomp. Flags are independent —
not every signature is conclusive on its own.

Excludes functions already in asmfix.txt (replace_with_asmfile entries).
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_FUNCS = ROOT / "asm" / "funcs"
ASMFIX = ROOT / "asmfix.txt"

INSN_RE = re.compile(r"^\s+/\*[^*]+\*/\s+(\w+)(?:\s+(.*))?$")
LABEL_RE = re.compile(r"^\s*\.L[0-9A-Fa-f_]+\s*:\s*$")


def parse_func_insns(asm_path: Path) -> list[tuple[str, str]]:
    """Return list of (mnemonic, operands) — labels skipped."""
    out: list[tuple[str, str]] = []
    for line in asm_path.read_text(encoding="utf-8").splitlines():
        m = INSN_RE.match(line)
        if not m:
            continue
        mnem = m.group(1)
        ops = (m.group(2) or "").strip()
        out.append((mnem, ops))
    return out


def already_asmfixed() -> set[str]:
    if not ASMFIX.exists():
        return set()
    funcs = set()
    pat = re.compile(r"^(\w+)\s*:\s*replace_with_asmfile\b")
    for line in ASMFIX.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        m = pat.match(line)
        if m:
            funcs.add(m.group(1))
    return funcs


def sig_double_bgez(insns: list[tuple[str, str]]) -> int:
    """bgez ... ; negu ; bgez again — INT_MIN handling. Count occurrences."""
    count = 0
    for i in range(len(insns) - 4):
        if insns[i][0] != "bgez":
            continue
        # Look ahead a few insns for negu followed by another bgez
        window = [insns[j][0] for j in range(i + 1, min(i + 6, len(insns)))]
        if "negu" in window and "bgez" in window[window.index("negu"):]:
            count += 1
    return count


def sig_multu_pad(insns: list[tuple[str, str]]) -> tuple[int, int]:
    """Returns (multu_count, padded_count) — padded means >=2 nops/insns
    between the multu and the consuming mflo."""
    multu_count = 0
    padded = 0
    i = 0
    while i < len(insns):
        mnem, _ = insns[i]
        if mnem in ("mult", "multu"):
            multu_count += 1
            # Look ahead for mflo/mfhi
            for j in range(i + 1, min(i + 8, len(insns))):
                if insns[j][0] in ("mflo", "mfhi"):
                    gap = j - i - 1
                    if gap >= 2:
                        padded += 1
                    break
        i += 1
    return multu_count, padded


def sig_t79_prominence(insns: list[tuple[str, str]]) -> tuple[int, int]:
    """Count instructions touching $t7/$t8/$t9 vs total instructions."""
    total = len(insns)
    high_t = 0
    pat = re.compile(r"\$t[789]\b")
    for _, ops in insns:
        if pat.search(ops):
            high_t += 1
    return high_t, total


def sig_dead_arg_save(insns: list[tuple[str, str]]) -> int:
    """addu $tN, $aM, $0 (or $zero) — if early in function and $tN
    is not the only access path. Heuristic: save instruction in first 5
    insns where the moved arg is also read within the next 10 insns."""
    if len(insns) < 4:
        return 0
    save_re = re.compile(r"\$(t\d|s\d|v\d),\s*\$(a\d),\s*\$(?:zero|0)")
    count = 0
    for i in range(min(5, len(insns))):
        mnem, ops = insns[i]
        if mnem != "addu":
            continue
        m = save_re.match(ops)
        if not m:
            continue
        dst = m.group(1)
        src = m.group(2)
        # Look at next 10 insns: is the original $aM still being read?
        # (real "dead arg-save" — the save is fake because the arg is
        # also kept live in $aM)
        for j in range(i + 1, min(i + 11, len(insns))):
            if re.search(rf"\${src}\b", insns[j][1] or ""):
                count += 1
                break
    return count


def classify(asm_path: Path) -> dict:
    insns = parse_func_insns(asm_path)
    if not insns:
        return None
    db = sig_double_bgez(insns)
    mu, mp = sig_multu_pad(insns)
    ht, tot = sig_t79_prominence(insns)
    da = sig_dead_arg_save(insns)
    score = 0
    reasons = []
    if db >= 1:
        score += 2
        reasons.append(f"double-bgez x{db}")
    if mu >= 2 and mp == mu:
        score += 2
        reasons.append(f"multu always padded ({mp}/{mu})")
    elif mu >= 4 and mp / mu >= 0.8:
        score += 1
        reasons.append(f"multu mostly padded ({mp}/{mu})")
    if tot > 0 and ht / tot >= 0.30 and tot >= 10:
        score += 2
        reasons.append(f"$t7-$t9 use {ht}/{tot} ({100*ht//tot}%)")
    elif tot > 0 and ht / tot >= 0.20 and tot >= 10:
        score += 1
        reasons.append(f"$t7-$t9 use {ht}/{tot} ({100*ht//tot}%)")
    if da >= 1:
        score += 1
        reasons.append(f"dead arg-save x{da}")
    return {"score": score, "reasons": reasons, "insns": tot}


def main(argv: list[str]) -> int:
    skip = already_asmfixed()
    min_score = 3
    if "--min" in argv:
        min_score = int(argv[argv.index("--min") + 1])
    show_all = "--all" in argv

    candidates = []
    for sf in sorted(ASM_FUNCS.glob("*.s")):
        fname = sf.stem
        if fname in skip:
            continue
        result = classify(sf)
        if result is None:
            continue
        if result["score"] >= min_score or show_all:
            candidates.append((fname, result))

    candidates.sort(key=lambda x: -x[1]["score"])
    print(f"# Hand-asm signature scan — flagged {len(candidates)} functions (min_score={min_score})")
    print(f"# Excluding {len(skip)} already-asmfix'd functions")
    print()
    for fname, r in candidates:
        print(f"{fname:36} score={r['score']} insns={r['insns']:4}  {'; '.join(r['reasons'])}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
