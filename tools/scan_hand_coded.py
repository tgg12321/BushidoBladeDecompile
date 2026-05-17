#!/usr/bin/env python3
"""Detect candidate hand-coded-asm functions by static signature.

The 7 strong signals from memory/feedback_hand_coded_asm_recognition.md
that are tractable to detect from target.s alone:

  S1. Uniform `multu`/`mflo` pacing. Every multu/mflo pair separated
      by exactly 2 instructions, across ≥2 pairs. GCC's scheduler
      produces variable pacing (often 0 when useful work exists).

  S2. Empty-body branch idiom. `bgez $rN, .L_X` whose target label is
      the very next instruction after the delay slot — both paths
      converge with no body. GCC doesn't emit empty-`if` bodies; a
      hand-coder leaves this as an INT_MIN-guard safety annotation.

  S3. Long kernel without callee-save spills. Function ≥40 instructions
      with NO `sw $sN, N($sp)` callee-save saves. GCC would spill under
      any meaningful register pressure of that size; absence indicates
      hand-allocated.

  S4. Front-loaded loads. ≥4 distinct loads from $aN/$sN base in the
      first 8 insns of a ≥40-insn function. GCC schedules pair-by-pair.

  S5. Cluster behavior. Function's opcode-only signature (ignoring
      registers and immediates) matches ≥1 sibling in the same file.

  S6. BIOS jumptable call with function-ID register set in the indirect
      call's delay slot. Pattern:
        addiu $tN, $zero, <0xA0|0xB0|0xC0>   # BIOS table address
        jalr/jr $tN                            # call dispatcher
        addiu $tM, $zero, <small>              # function ID, IN delay slot
      GCC 2.7.2 has no way to express "load $tM in the delay slot of an
      indirect call to $tN" — the kernel dispatcher reads $tM after the
      jump is committed. Like S1/S2, this is GCC-impossible: even one
      occurrence is decisive evidence of hand-coded asm.

  S7. Callee-saved register read with no corresponding save. Any
      $s0-$s7 appears in instruction operands (as source, destination,
      or load/store base) but no `sw $sN, K($sp)` save exists anywhere
      in the function. GCC 2.7.2 unconditionally generates the save when
      it allocates a callee-saved register; absence indicates a custom
      calling convention where the caller has set up $sN as input and
      the callee is permitted to clobber it. ($fp/$30 is excluded — GCC
      2.7.2 with PsyQ flags treats $30 in register-asm-pinned contexts
      as not-callee-saved, so $fp use-without-save can appear in pure-C
      output and is not decisive on its own.) Like S6, even one match is
      decisive evidence of hand-coded asm: there is no C-level mechanism
      to read from $s0 (or write to it) while preventing GCC from
      generating the prologue save. Originated 2026-05-16 from the
      func_8004A808 / func_8004BB68 / func_8004C1F4 / func_80050774
      polygon-dispatch cluster that S1-S6 miss.

Output modes:
  --single <func>  : JSON for one function (consumed by memory_check.py)
  --all            : human-readable sorted list of candidates
  --json           : with --all, machine-readable

Run via `dc.sh scan-hand-coded`.
"""

from __future__ import annotations
import argparse
import json
import re
import sys
from collections import defaultdict
from dataclasses import dataclass, asdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_DIR = ROOT / "asm" / "funcs"

# Instruction line: optional address comment, then mnemonic, then operands.
INSN_RE = re.compile(
    r"^\s*(?:/\*[^*]*\*/\s*)?"
    r"([a-z][a-z0-9]*)\s*"
    r"(.*?)$"
)
LABEL_RE = re.compile(r"^\s*(\.L[A-Za-z0-9_]+|[A-Za-z_][A-Za-z0-9_]*)\s*:\s*$")
GLABEL_RE = re.compile(r"^\s*glabel\s+(\w+)")
ENDLABEL_RE = re.compile(r"^\s*endlabel\s+\w+")

# Loads from a non-$sp base register (arg/saved/temp, not stack-frame ops).
LOAD_OPS = {"lb", "lbu", "lh", "lhu", "lw", "lwl", "lwr"}
STORE_OPS = {"sb", "sh", "sw", "swl", "swr"}

# Callee-saved general registers tracked by S7. $fp ($30) is excluded —
# GCC 2.7.2 with PsyQ flags doesn't always emit the save for $30 when it
# appears via register-asm pin, so a $fp-without-save case is not decisive
# on its own.
CALLEE_SAVES = {f"s{i}" for i in range(8)}
# `sw $reg, K($sp)` — matches `sw $s1, 16($sp)` or `sw $s1, -16($sp)` etc.
SP_REL_SAVE_RE = re.compile(r"^\$(\w+)\s*,\s*-?\w+\(\$sp\)\s*$")


@dataclass
class Insn:
    mnemonic: str
    operands: str
    label: str | None = None  # label attached to this insn (if any)


@dataclass
class FuncSignals:
    func: str
    src_file: str  # asm/funcs/<func>.s
    insn_count: int
    multu_count: int
    s1_uniform_multu: bool
    s1_detail: str
    s2_empty_branch: bool
    s2_detail: str
    s3_no_spills: bool
    s3_detail: str
    s4_front_loads: bool
    s4_detail: str
    s5_cluster: bool
    s5_detail: str
    s6_bios_jumptable: bool
    s6_detail: str
    s7_no_save_callee_use: bool
    s7_detail: str
    score: int      # total signals hit (0-7)
    tier: str       # STRONG / POSSIBLE / TIGHT_C / LOW
    tier_reason: str
    opcode_sig: str  # hash key for cluster detection


def classify_tier(s1: bool, s2: bool, s6: bool, s7: bool, score: int) -> tuple[str, str]:
    """Tier the function by which signals fire.

    S1 (uniform multu pacing), S2 (empty-body branch), S6 (BIOS
    jumptable with delay-slot register setup), and S7 (callee-saved
    register read with no corresponding save) are the "GCC-impossible"
    signals — patterns the compiler has no way to emit. S3/S4/S5 are
    tightness/cluster signals — also common in tight pure-C functions
    (e.g. the calc_fc_frame_8007EC5C GTE family).

    A 3+ score WITHOUT any GCC-impossible signal is most likely a
    tight-C cluster, not hand-coded asm. Conversely, S6 or S7 alone is
    decisive (both patterns are too specific to occur by accident); S1
    or S2 alone is enough to warrant investigation."""
    impossible_hits = [name for name, hit in (("S1", s1), ("S2", s2), ("S6", s6), ("S7", s7)) if hit]
    if s6:
        # S6 is the most specific GCC-impossible pattern: a single match
        # is essentially proof of hand-coded asm. Skip the tightness gate.
        return "STRONG", f"S6 (BIOS jumptable with delay-slot register setup) — GCC-impossible, decisive"
    if s7:
        # S7 is similarly decisive: there is no C-level mechanism to use
        # $sN without forcing GCC to emit the prologue save. Even one
        # unsaved callee-save use proves custom ABI.
        return "STRONG", f"S7 (callee-saved register used without save) — GCC-impossible, decisive"
    if s1 and s2 and score >= 4:
        return "STRONG", "S1+S2 (both GCC-impossible) plus tightness — almost certainly hand-coded"
    if impossible_hits and score >= 3:
        return "STRONG", f"{'+'.join(impossible_hits)} (GCC-impossible) plus 2+ tightness signals"
    if impossible_hits:
        return "POSSIBLE", f"{'+'.join(impossible_hits)} alone is rare in GCC output — investigate"
    if score >= 3:
        return "TIGHT_C", "tight pure-C function or cluster (no GCC-impossible signals)"
    return "LOW", "no strong hand-coded indicators"


def parse_func(path: Path) -> tuple[str, list[Insn]] | None:
    """Return (func_name, insns) for one asm/funcs/<func>.s file."""
    name = None
    insns: list[Insn] = []
    pending_label: str | None = None
    for raw in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        # Strip out the address/byte-comment prefix `/* ADDR HEX */`
        line = re.sub(r"^\s*/\*\s+\w+\s+\w+\s+\w+\s+\*/\s*", "    ", raw)
        line = line.rstrip()
        if not line:
            continue

        m = GLABEL_RE.match(line)
        if m:
            name = m.group(1)
            continue
        if ENDLABEL_RE.match(line):
            break

        m = LABEL_RE.match(line)
        if m:
            pending_label = m.group(1)
            continue

        # Try instruction match. Skip directives starting with `.`
        s = line.strip()
        if s.startswith("."):
            continue
        m = INSN_RE.match(line)
        if not m:
            continue
        mnem = m.group(1)
        ops = m.group(2).strip()
        if not mnem or mnem in ("glabel", "endlabel"):
            continue
        insns.append(Insn(mnemonic=mnem, operands=ops, label=pending_label))
        pending_label = None
    if not name:
        return None
    return name, insns


def signal_multu_pacing(insns: list[Insn]) -> tuple[bool, str, int]:
    """S1: uniform multu→mflo pacing of exactly 2 cycles."""
    spacings: list[int] = []
    i = 0
    while i < len(insns):
        if insns[i].mnemonic == "multu":
            # Find next mflo
            for j in range(i + 1, min(i + 12, len(insns))):
                if insns[j].mnemonic == "mflo":
                    spacings.append(j - i - 1)
                    i = j
                    break
            else:
                pass  # no mflo within window
        i += 1
    if len(spacings) < 2:
        return False, f"only {len(spacings)} multu/mflo pair(s)", len(spacings)
    uniform = all(s == 2 for s in spacings)
    detail = (
        f"{len(spacings)} pairs, spacings={spacings}"
        if not uniform else
        f"{len(spacings)} pairs all 2-cycle"
    )
    return uniform, detail, len(spacings)


def signal_empty_branch(insns: list[Insn]) -> tuple[bool, str]:
    """S2: bgez/bltz where the branch target label is the instruction
    immediately AFTER the delay slot — empty if-body convergence."""
    # Pre-build label→position map
    label_at: dict[str, int] = {}
    for idx, ins in enumerate(insns):
        if ins.label:
            label_at[ins.label] = idx

    matches: list[str] = []
    for idx, ins in enumerate(insns):
        if ins.mnemonic not in ("bgez", "bltz", "beq", "bne", "blez", "bgtz"):
            continue
        # Branch target is the last operand (after ", ")
        parts = [p.strip() for p in ins.operands.split(",")]
        if not parts:
            continue
        target = parts[-1]
        # Stripping any trailing comment
        target = target.split("#")[0].strip()
        if not target.startswith(".L"):
            continue
        if target not in label_at:
            continue
        # Delay slot is idx+1; target should be idx+2 for empty-body
        if label_at[target] == idx + 2:
            matches.append(f"{ins.mnemonic} → {target} (empty body @ insn {idx})")
    if matches:
        return True, "; ".join(matches[:2])
    return False, "no empty-body branches"


def signal_no_spills(insns: list[Insn]) -> tuple[bool, str]:
    """S3: function ≥40 insns with NO callee-save (sw $sX) spills.
    Only counts if function uses ≥3 callee-save-style or many temp regs."""
    if len(insns) < 40:
        return False, f"too short ({len(insns)} < 40 insns)"
    spills = 0
    distinct_regs: set[str] = set()
    for ins in insns:
        # Detect spills: sw $sX/$fp/$ra, K($sp)
        if ins.mnemonic == "sw":
            m = re.match(r"\$(\w+),\s*[-\w()]+\$sp\)", ins.operands)
            if m:
                spills += 1
        # Track distinct registers in operands
        for r in re.findall(r"\$(\w+)", ins.operands):
            if r in ("zero", "sp", "ra", "gp", "fp", "at"):
                continue
            distinct_regs.add(r)
    if spills == 0 and len(distinct_regs) >= 8:
        return True, f"{len(insns)} insns, 0 spills, {len(distinct_regs)} distinct regs"
    return False, f"{len(insns)} insns, {spills} spills, {len(distinct_regs)} distinct regs"


def signal_front_loads(insns: list[Insn]) -> tuple[bool, str]:
    """S4: a 'load burst' of ≥4 loads from non-$sp base within any
    consecutive 8-instruction window. Hand-coders pre-load inputs into
    registers before the math kernel; GCC schedules pair-by-pair.
    The burst can be at function entry OR at the start of an inner kernel
    (e.g. after an angle-lookup prologue)."""
    if len(insns) < 40:
        return False, f"too short ({len(insns)} < 40 insns)"
    best = 0
    best_at = 0
    for start in range(len(insns) - 7):
        window = insns[start:start + 8]
        cnt = sum(
            1 for ins in window
            if ins.mnemonic in LOAD_OPS and "$sp" not in ins.operands
        )
        if cnt > best:
            best = cnt
            best_at = start
    if best >= 4:
        return True, f"{best} loads in 8-insn window @ insn {best_at}"
    return False, f"max load burst was {best} in any 8-insn window"


BIOS_TABLE_ADDRS = (0xA0, 0xB0, 0xC0)
JUMP_TARGET_REG_RE = re.compile(r"^(t\d|k[01]|v[01])$")
DELAY_LOAD_REG_RE = re.compile(r"^(t\d|k[01]|a[0-3]|v[01])$")
REG_IMM_LOAD_RE = re.compile(r"^\$(\w+)\s*,\s*(?:\$zero\s*,\s*)?(.+)$")


def _parse_imm(s: str) -> int | None:
    """Parse a MIPS immediate operand (decimal, 0xHEX, or trailing 0x...)."""
    s = s.split("#")[0].strip()
    # strip surrounding parens or whitespace from any 0(reg) artifacts (shouldn't occur for li)
    s = s.split("(")[0].strip()
    if not s:
        return None
    try:
        return int(s, 0)
    except ValueError:
        return None


def signal_bios_jumptable(insns: list[Insn]) -> tuple[bool, str]:
    """S6: BIOS jumptable call with function-ID register set in the
    indirect call's delay slot. Looks for the canonical PSY-Q BIOS
    pattern:
        addiu $tN, $zero, <0xA0|0xB0|0xC0>   # BIOS table address
        jalr/jr $tN                            # call dispatcher
        addiu $tM, $zero, <small>              # function ID in delay slot

    GCC 2.7.2 cannot pin a register load into an indirect call's delay
    slot via natural C: function-pointer calls compile to `jal/jalr`
    with a nop or scheduling-promoted instruction in the delay slot, and
    there's no syntax to demand the delay slot hold a specific
    `addiu $tM, $zero, IMM`. A single match is decisive evidence."""
    for i, ins in enumerate(insns):
        if ins.mnemonic not in ("jalr", "jr"):
            continue
        # Operands: `jalr $tN` or `jalr $ra, $tN` — last register is the
        # jump target.
        regs = re.findall(r"\$(\w+)", ins.operands)
        if not regs:
            continue
        jump_reg = regs[-1]
        if not JUMP_TARGET_REG_RE.match(jump_reg):
            continue
        # Walk back up to 8 insns looking for the load of $jump_reg with
        # one of the BIOS table addresses.
        loaded_val: int | None = None
        for j in range(i - 1, max(-1, i - 9), -1):
            prev = insns[j]
            if prev.mnemonic not in ("addiu", "li", "ori"):
                continue
            m = REG_IMM_LOAD_RE.match(prev.operands)
            if not m or m.group(1) != jump_reg:
                continue
            val = _parse_imm(m.group(2))
            if val is None:
                continue
            if val in BIOS_TABLE_ADDRS:
                loaded_val = val
            # First write to $jump_reg wins (either match or stop looking).
            break
        if loaded_val is None:
            continue
        # Check the delay slot for an addiu/li loading a small constant
        # into a different register.
        if i + 1 >= len(insns):
            continue
        delay = insns[i + 1]
        if delay.mnemonic not in ("addiu", "li"):
            continue
        m = REG_IMM_LOAD_RE.match(delay.operands)
        if not m:
            continue
        delay_reg = m.group(1)
        if delay_reg == jump_reg:
            continue
        if not DELAY_LOAD_REG_RE.match(delay_reg):
            continue
        delay_val = _parse_imm(m.group(2))
        if delay_val is None or not (0 <= delay_val <= 0xFF):
            continue
        kind = "jalr" if ins.mnemonic == "jalr" else "jr (tail-call)"
        return True, (
            f"BIOS 0x{loaded_val:X}({delay_val:#x}) @ insn {i}: "
            f"{kind} ${jump_reg} with ${delay_reg}={delay_val:#x} in delay slot"
        )
    return False, "no BIOS jumptable call pattern"


def signal_no_save_callee_use(insns: list[Insn]) -> tuple[bool, str]:
    """S7: a callee-saved $sN ($s0-$s7) appears in instruction operands
    (as source, destination, or load/store base) but no `sw $sN, K($sp)`
    save exists in the function. GCC 2.7.2 unconditionally generates the
    save when it allocates a callee-save. Absence means the caller has
    set up $sN as input and the callee is allowed to clobber it (custom
    calling convention, GCC-impossible to express in standard C).

    Saves AND restores (`sw`/`lw $sN, K($sp)`) are excluded from the
    "use" scan — they are prologue/epilogue artifacts paired with the
    save, not function-body uses. $fp/$30 is also excluded — see
    docstring at top of file."""
    saved: set[str] = set()
    used: set[str] = set()
    for ins in insns:
        # Sp-relative loads/stores: pair the save side, skip operand scan.
        if ins.mnemonic in ("sw", "lw"):
            m = SP_REL_SAVE_RE.match(ins.operands)
            if m:
                reg = m.group(1)
                if ins.mnemonic == "sw" and reg in CALLEE_SAVES:
                    saved.add(reg)
                continue
        # All other instructions: any $sN appearance counts as a use.
        for r in re.findall(r"\$(\w+)", ins.operands):
            if r in CALLEE_SAVES:
                used.add(r)
    unsaved = used - saved
    if unsaved:
        sorted_unsaved = sorted(unsaved, key=lambda r: int(r[1:]))
        return True, (
            f"{len(unsaved)} unsaved callee-save use(s): "
            f"{', '.join('$' + r for r in sorted_unsaved)}"
        )
    return False, "all callee-save uses have $sp save (or no callee-saves used)"


def compute_opcode_sig(insns: list[Insn]) -> str:
    """Build a loose opcode-only signature for cluster detection.
    Strips registers, immediates, and operands entirely — keeps the
    SEQUENCE OF MNEMONICS only. A k-mer-on-mnemonics shape comparator
    would be even more permissive; for now the full mnemonic sequence is
    tight enough to find clusters with very similar structures but
    loose enough to ignore register-allocation choice differences.

    Length is capped to the function size, so functions of different
    lengths never collide."""
    if len(insns) < 20:
        return ""
    return "|".join(ins.mnemonic for ins in insns)


def compute_kmer_sig(insns: list[Insn], k: int = 10) -> set[str]:
    """Approximate-match cluster signature: set of k-mers over the
    mnemonic sequence. Two functions are 'cluster siblings' if their
    k-mer Jaccard similarity is high. More permissive than full-sequence
    equality (handles single-instruction reorderings or sin-negation
    position differences)."""
    if len(insns) < k + 5:
        return set()
    mnems = [ins.mnemonic for ins in insns]
    return {"|".join(mnems[i:i + k]) for i in range(len(mnems) - k + 1)}


def analyze_all(asm_dir: Path) -> list[FuncSignals]:
    out: list[FuncSignals] = []

    parsed: list[tuple[Path, str, list[Insn]]] = []
    for p in sorted(asm_dir.glob("*.s")):
        r = parse_func(p)
        if not r:
            continue
        name, insns = r
        if len(insns) < 5:
            continue
        parsed.append((p, name, insns))

    # First pass: k-mer signatures for fuzzy cluster detection.
    # Bucket functions by similar length (within ±20%) before pairwise
    # Jaccard so the comparison is O(N * bucket_size).
    kmer_sigs: dict[str, set[str]] = {}
    by_length: dict[int, list[str]] = defaultdict(list)
    for p, name, insns in parsed:
        kmer_sigs[name] = compute_kmer_sig(insns, k=10)
        # Length bucket: round to nearest 20
        by_length[len(insns) // 20].append(name)

    # For each function, find approximate cluster siblings.
    # Threshold: Jaccard ≥ 0.5 over k-mers indicates strong structural
    # similarity even with register/operand differences. This is the
    # right scale: identical structure with sin-negation flipped ≈ 0.7,
    # different functions same family ≈ 0.5-0.7, unrelated ≈ <0.1.
    cluster_members: dict[str, list[tuple[str, float]]] = defaultdict(list)
    for p, name, insns in parsed:
        my_set = kmer_sigs[name]
        if not my_set:
            continue
        bucket_keys = (len(insns) // 20 - 1, len(insns) // 20, len(insns) // 20 + 1)
        seen: set[str] = set()
        for bk in bucket_keys:
            for cand in by_length.get(bk, []):
                if cand == name or cand in seen:
                    continue
                seen.add(cand)
                cand_set = kmer_sigs[cand]
                if not cand_set:
                    continue
                inter = len(my_set & cand_set)
                if inter == 0:
                    continue
                union = len(my_set | cand_set)
                jacc = inter / union if union else 0.0
                if jacc >= 0.5:
                    cluster_members[name].append((cand, jacc))
        cluster_members[name].sort(key=lambda x: -x[1])

    # Second pass: per-function signals
    for p, name, insns in parsed:
        s1, s1d, multu_count = signal_multu_pacing(insns)
        s2, s2d = signal_empty_branch(insns)
        s3, s3d = signal_no_spills(insns)
        s4, s4d = signal_front_loads(insns)
        siblings = cluster_members.get(name, [])
        s5 = bool(siblings) and len(insns) >= 40
        if s5:
            sib_strs = [f"{n} (jaccard={j:.2f})" for n, j in siblings[:3]]
            tail = "..." if len(siblings) > 3 else ""
            s5d = f"{len(siblings)} approx-sibling(s): {', '.join(sib_strs)}{tail}"
        else:
            s5d = "no high-similarity siblings (jaccard < 0.5)"
        s6, s6d = signal_bios_jumptable(insns)
        s7, s7d = signal_no_save_callee_use(insns)

        score = sum([s1, s2, s3, s4, s5, s6, s7])
        tier, tier_reason = classify_tier(s1, s2, s6, s7, score)
        out.append(FuncSignals(
            func=name,
            src_file=str(p.relative_to(ROOT)),
            insn_count=len(insns),
            multu_count=multu_count,
            s1_uniform_multu=s1, s1_detail=s1d,
            s2_empty_branch=s2, s2_detail=s2d,
            s3_no_spills=s3, s3_detail=s3d,
            s4_front_loads=s4, s4_detail=s4d,
            s5_cluster=s5, s5_detail=s5d,
            s6_bios_jumptable=s6, s6_detail=s6d,
            s7_no_save_callee_use=s7, s7_detail=s7d,
            score=score,
            tier=tier,
            tier_reason=tier_reason,
            opcode_sig=compute_opcode_sig(insns),
        ))
    return out


def analyze_one(asm_dir: Path, func: str) -> FuncSignals | None:
    p = asm_dir / f"{func}.s"
    if not p.exists():
        return None
    # We still need cluster context — but limit to functions sharing a
    # rough length for speed. Simplest: re-run full scan.
    all_sigs = analyze_all(asm_dir)
    for s in all_sigs:
        if s.func == func:
            return s
    return None


def fmt_signals_human(s: FuncSignals) -> str:
    rows = [
        ("S1 multu pacing  ", s.s1_uniform_multu, s.s1_detail),
        ("S2 empty branch  ", s.s2_empty_branch, s.s2_detail),
        ("S3 no spills     ", s.s3_no_spills, s.s3_detail),
        ("S4 front loads   ", s.s4_front_loads, s.s4_detail),
        ("S5 cluster       ", s.s5_cluster, s.s5_detail),
        ("S6 BIOS jumptable ", s.s6_bios_jumptable, s.s6_detail),
        ("S7 unsaved $sN use", s.s7_no_save_callee_use, s.s7_detail),
    ]
    lines = [
        f"  HAND_CODED: tier={s.tier}  score={s.score}/7  ({s.func}, {s.insn_count} insns)",
        f"    Reason: {s.tier_reason}",
    ]
    for label, hit, detail in rows:
        mark = "[X]" if hit else "[ ]"
        lines.append(f"    {mark} {label}{detail}")
    return "\n".join(lines)


def main() -> int:
    ap = argparse.ArgumentParser()
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--single", metavar="FUNC",
                   help="report signals for one function (memory_check.py uses this)")
    g.add_argument("--all", action="store_true",
                   help="scan every asm/funcs/*.s and print ranked candidates")
    ap.add_argument("--json", action="store_true",
                    help="emit JSON instead of human-readable")
    ap.add_argument("--min-score", type=int, default=3,
                    help="--all: only print funcs with score ≥ N (default 3)")
    args = ap.parse_args()

    if args.single:
        s = analyze_one(ASM_DIR, args.single)
        if not s:
            print(f"asm/funcs/{args.single}.s not found", file=sys.stderr)
            return 1
        if args.json:
            print(json.dumps(asdict(s)))
        else:
            print(fmt_signals_human(s))
        return 0

    # --all
    sigs = analyze_all(ASM_DIR)
    # Sort by tier rank (STRONG > POSSIBLE > TIGHT_C > LOW), then score desc.
    tier_rank = {"STRONG": 0, "POSSIBLE": 1, "TIGHT_C": 2, "LOW": 3}
    sigs.sort(key=lambda x: (tier_rank[x.tier], -x.score, -x.insn_count, x.func))

    if args.json:
        candidates = [s for s in sigs if s.tier in ("STRONG", "POSSIBLE", "TIGHT_C")]
        print(json.dumps([asdict(s) for s in candidates], indent=2))
        return 0

    strong = [s for s in sigs if s.tier == "STRONG"]
    possible = [s for s in sigs if s.tier == "POSSIBLE"]
    tight_c = [s for s in sigs if s.tier == "TIGHT_C"]
    low_total = sum(1 for s in sigs if s.tier == "LOW")

    def print_block(title: str, rows: list[FuncSignals], note: str) -> None:
        print(f"## {title}  ({len(rows)} func{'s' if len(rows)!=1 else ''})")
        print(f"   {note}")
        print()
        if not rows:
            print("   (none)")
            print()
            return
        print(f"   {'score':5}  {'insns':5}  {'multus':6}  {'function':40}  signals")
        print(f"   {'-'*95}")
        for s in rows:
            sig_letters = "".join([
                "1" if s.s1_uniform_multu else ".",
                "2" if s.s2_empty_branch else ".",
                "3" if s.s3_no_spills else ".",
                "4" if s.s4_front_loads else ".",
                "5" if s.s5_cluster else ".",
                "6" if s.s6_bios_jumptable else ".",
                "7" if s.s7_no_save_callee_use else ".",
            ])
            note_bits = []
            if s.s5_cluster:
                note_bits.append(f"cluster: {s.s5_detail.split(': ', 1)[-1]}")
            if s.s6_bios_jumptable:
                note_bits.append(f"bios: {s.s6_detail.split(': ', 1)[-1]}")
            if s.s7_no_save_callee_use:
                note_bits.append(f"unsaved: {s.s7_detail.split(': ', 1)[-1]}")
            extra_note = ("  " + "  ".join(note_bits)) if note_bits else ""
            print(f"   {s.score}/7    {s.insn_count:5}  {s.multu_count:6}  {s.func:40}  [{sig_letters}]{extra_note}")
        print()

    print(f"# scan_hand_coded: {len(sigs)} functions scanned in asm/funcs/")
    print(f"# tiers: STRONG={len(strong)}  POSSIBLE={len(possible)}  TIGHT_C={len(tight_c)}  LOW={low_total}")
    print()
    print_block(
        "STRONG candidates — almost certainly hand-coded asm",
        strong,
        "S1 (uniform multu pacing), S2 (empty-body branch), S6 (BIOS "
        "jumptable with delay-slot register setup), and/or S7 (unsaved "
        "callee-save use) fire — all effectively impossible in GCC "
        "output. Treat as canonically-asm absent strong counter-evidence; "
        "if not in inline_asm_canonical.txt, investigate per skill §2.5.b.",
    )
    print_block(
        "POSSIBLE candidates — investigate",
        possible,
        "Exactly one GCC-impossible signal (S1, S2, or S6) fires without "
        "supporting tightness signals. Could be hand-coded, could be an "
        "edge case (e.g. small GTE wrapper). Read the target.s.",
    )
    print_block(
        "TIGHT_C — clusters of tight pure-C, NOT hand-coded",
        tight_c,
        "Multiple tightness signals (S3/S4/S5) but no GCC-impossible "
        "signal. Most likely a pure-C cluster (e.g. GTE 3x3, calc_fc_frame "
        "family). Do NOT treat these as hand-coded — match them via §3.",
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
