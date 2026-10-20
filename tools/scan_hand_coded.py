#!/usr/bin/env python3
"""Detect candidate hand-coded-asm functions by static signature.

Signals detectable from asm/funcs/<func>.s alone:

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
      gte_ProjectPackedVertices / func_8004BB68 / func_8004C1F4 / func_80050774
      polygon-dispatch cluster that S1-S6 miss.

  S8. Redundant low-bits mask before a discarding left-shift.
        andi $D, $Y, M        # M = 2^m - 1 (m contiguous low bits)
        sll  $D, $D, S        # S >= 32 - m, so the shift discards exactly
                              # the bits the mask cleared -> mask redundant
      The andi result is single-use (the sll overwrites $D). GCC's combine
      pass always eliminates a single-use redundant mask before a left
      shift (subreg-of-AND simplification), so GCC never emits it; a
      hand-coder writes the explicit mask. Verified 2026-05-21 on
      func_8007EDBC: our decompals fork AND the original cc1psx (Sony GCC
      2.7.2.SN.1) both elide it, so a target that KEEPS it was hand-written.
      Like S6/S7, GCC-impossible — even one occurrence is decisive.
      Originated 2026-05-21 from func_8007EDBC (display.c packed
      fixed-point multiply, `andi $t1,$t0,0xFFFF; sll $t1,$t1,16`); ~38
      pure-C forms + cc1psx + combine.c:1458 analysis confirmed no C source
      reproduces it. Future agents: STRONG here = stop trying pure C.

  S9-S12 (added 2026-10-15; S9-S11 owner-approved as gate-class evidence,
  S12 informational) read the
  original instruction WORD from each line's `/* OFF ADDR WORD */` comment.
  Each was measured at 0 hits over the 1129 compiled-C functions in
  asm/funcs (C body in src/, not INCLUDE_ASM, not in
  inline_asm_canonical.txt). Hold-out, the 991 functions of the PsyQ 4.0
  library modules built from .C files: S9 0/991, S12 0/991, S11 1/991
  (libgcc2's __builtin_saveregs, a top-level asm() block).

  S9.  Overflow-trapping `add` / `addi` / `sub`. GCC 2.7.2 emits only
       addu/addiu/subu (mips.md addsi3/subsi3/negsi2/abssi2).
  S10. A jump table whose entries land inside a different function
       (labels are per-function: stmt.c case_stack, c-decl.c label levels).
       Table words come from the oracle EXE (disc/SLUS_006.63).
  S11. A callee-saved register used as a parameter or result: read in the
       entry block before any write (save excluded), or written but never
       read (restore excluded). Covers what S7 misses: a dispatcher that
       saves every callee-saved register yet hands values through them.
  S12. A `nop` no delay slot, load delay, HI/LO, mult/div or coprocessor
       interlock requires (e.g. `addiu; nop; jal`). Informational only:
       not gate-class, never decides a tier.

Gate class (GATE_CLASS): S1, S2, S6-S11. S6-S11 are DECISIVE (one hit =
STRONG); S1/S2 need tightness corroboration. The tier — not the signal
list — is what judge-sole-gate rule 3 and the Grinder driver read.

Output modes:
  --single <func>  : JSON for one function (consumed by memory_check.py)
  --all            : human-readable sorted list of candidates
  --json           : with --all, machine-readable

Run directly: `python3 tools/scan_hand_coded.py --single <func>` (the retired
`dc.sh scan-hand-coded` wrapper just forwarded here).
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
# splat marks every jump-table target with `jlabel .L<ADDR>` (no colon).
JLABEL_RE = re.compile(r"^\s*jlabel\s+(\.L[A-Za-z0-9_]+)\s*$")
DLABEL_RE = re.compile(r"^\s*[ad]label\s+\w+")  # data / alias labels: not insns
# `/* 3ACE0 8004A4E0 C8FFBD27 */` = ROM offset, vram address, the word's bytes
# in file (little-endian) order.
ADDR_WORD_RE = re.compile(r"^\s*/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{8})\s*\*/")

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
    addr: int | None = None   # vram address from the `/* OFF ADDR WORD */` comment
    word: int | None = None   # the original 32-bit instruction word
    jtarget: bool = False     # a splat `jlabel` (jump-table target) sits here


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
    s8_redundant_mask_shift: bool
    s8_detail: str
    s9_trapping_arith: bool
    s9_detail: str
    s10_cross_func_jtbl: bool
    s10_detail: str
    s11_custom_cs_abi: bool
    s11_detail: str
    s12_hazard_free_nop: bool
    s12_detail: str
    gate_signals: list  # which gate-class (GCC-impossible) signals fired
    score: int      # total signals hit (0-12)
    tier: str       # STRONG / POSSIBLE / TIGHT_C / LOW
    tier_reason: str
    opcode_sig: str  # hash key for cluster detection


# The gate class: GCC-impossible signals, the evidence judge-sole-gate rule 3
# accepts. DECISIVE ones make a function STRONG on a single hit; S1/S2 need
# tightness corroboration (classify_tier).
GATE_CLASS = ("S1", "S2", "S6", "S7", "S8", "S9", "S10", "S11")
# analyze_all skips functions shorter than this; --single reports them LOW.
SCAN_FLOOR = 5
FLOOR_REASON = f"under the {SCAN_FLOOR}-instruction scan floor"
DECISIVE = {
    "S6": "BIOS jumptable with delay-slot register setup",
    "S7": "callee-saved register used without save",
    "S8": "redundant mask before discarding left-shift",
    "S9": "overflow-trapping add/addi/sub",
    "S10": "jump table entry inside another function",
    "S11": "callee-saved register used as a parameter/result",
}
# S12 (hazard-free nop) is measured and reported but is NOT gate-class: the
# owner approved trapping arithmetic, cross-function jump tables and
# register conventions GCC cannot produce (2026-10-15); S12 needs its own
# ruling before it may decide a tier.


def classify_tier(hits: dict[str, bool], score: int) -> tuple[str, str]:
    """Tier the function by which signals fire (`hits`: "S1".."S12" -> bool).

    The GATE_CLASS signals are "GCC-impossible" — patterns the compiler
    has no way to emit; each DECISIVE one was measured at zero hits across
    every compiled-C function in the corpus. S3/S4/S5 are tightness/cluster
    signals — also common in tight pure-C functions (e.g. the
    calc_fc_frame_8007EC5C GTE family).

    A 3+ score WITHOUT any GCC-impossible signal is most likely a
    tight-C cluster, not hand-coded asm. Conversely, one DECISIVE signal
    is proof; S1 or S2 alone is enough to warrant investigation."""
    s1, s2 = hits.get("S1", False), hits.get("S2", False)
    impossible_hits = [name for name in GATE_CLASS if hits.get(name)]
    decisive = [name for name in DECISIVE if hits.get(name)]
    if decisive:
        return "STRONG", (" + ".join(f"{k} ({DECISIVE[k]})" for k in decisive)
                          + " — GCC-impossible, decisive")
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
    pending_jtarget = False
    for raw in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        aw = ADDR_WORD_RE.match(raw)
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

        m = LABEL_RE.match(line) or JLABEL_RE.match(line)
        if m:
            pending_label = m.group(1)
            pending_jtarget = pending_jtarget or line.lstrip().startswith("jlabel")
            continue
        if DLABEL_RE.match(line):
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
        addr = word = None
        if aw:
            addr = int(aw.group(1), 16)
            word = int.from_bytes(bytes.fromhex(aw.group(2)), "little")
        insns.append(Insn(mnemonic=mnem, operands=ops, label=pending_label,
                          addr=addr, word=word, jtarget=pending_jtarget))
        pending_label = None
        pending_jtarget = False
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


MASK_SHIFT_RE = re.compile(r"^\$(\w+),\s*\$(\w+),\s*(\S+)")


def signal_redundant_mask_shift(insns: list[Insn]) -> tuple[bool, str]:
    """S8: redundant low-bits mask before a discarding left-shift.

    Pattern: `andi $D, $Y, M` immediately followed by `sll $D, $D, S`,
    where M = 2^m - 1 (the m contiguous low bits) and S >= 32 - m.

    The `sll` shifts $D left by S, discarding everything at bit position
    (32 - S) and above. Since S >= 32 - m, the only bits of $Y that
    survive the shift are bits 0..(m-1) — exactly the bits the mask keeps.
    So the mask's clearing of bits m..31 is mathematically redundant, and
    the andi result is single-use (the very next insn, the sll,
    overwrites $D).

    GCC's combine pass eliminates a single-use redundant mask before a
    left shift (it simplifies `(subreg:HI (and X 0xFFFF))` -> `(subreg:HI
    X)`, and the analogous `(ashift (and X M) S)` -> `(ashift X S)`), so
    GCC NEVER emits the andi here. A hand-coder writes the explicit mask.
    Verified 2026-05-21 on func_8007EDBC: BOTH our decompals fork and the
    original cc1psx (Sony GCC 2.7.2.SN.1) elide it — so a target that
    KEEPS it was hand-written, not compiled from C. GCC-impossible:
    even one occurrence is decisive.

    NOTE: only the same-register form `sll $D,$D,S` is matched, which
    guarantees the andi result is single-use (consumed + overwritten by
    the sll). A different-dest `sll $X,$D,S` could leave $D live for a
    second use (legitimate multi-use C), so it is intentionally NOT
    flagged."""
    matches: list[str] = []
    for i in range(len(insns) - 1):
        a, b = insns[i], insns[i + 1]
        if a.mnemonic != "andi" or b.mnemonic != "sll":
            continue
        ma = MASK_SHIFT_RE.match(a.operands)
        mb = MASK_SHIFT_RE.match(b.operands)
        if not ma or not mb:
            continue
        a_dst, _a_src, a_imm = ma.group(1), ma.group(2), ma.group(3)
        b_dst, b_src, b_sh = mb.group(1), mb.group(2), mb.group(3)
        # sll must read AND write the andi's destination (single-use).
        if b_dst != a_dst or b_src != a_dst:
            continue
        mask = _parse_imm(a_imm)
        shift = _parse_imm(b_sh)
        if mask is None or shift is None or mask <= 0:
            continue
        # mask must be a contiguous run of low bits: 2^m - 1.
        if mask & (mask + 1):
            continue
        m_bits = mask.bit_length()
        if shift >= 32 - m_bits:
            matches.append(
                f"andi ${a_dst},{a_imm} ; sll ${a_dst},{shift} @ insn {i} "
                f"(mask {mask:#x} = low {m_bits} bits, redundant before <<{shift})"
            )
    if matches:
        return True, "; ".join(matches[:2]) + (" ..." if len(matches) > 2 else "")
    return False, "no redundant mask-before-shift"


# ---------------------------------------------------------------------------
# Word-level decoding for S9-S12. These signals read the ORIGINAL instruction
# word from the `/* OFF ADDR WORD */` comment, not splat's mnemonic spelling
# (`beqz`, `move`, `negu` ... are pseudo-op renderings of the same words).

CS_REGS = frozenset(range(16, 24)) | {30}          # $s0-$s7, $fp
REG_NAMES = ("zero at v0 v1 a0 a1 a2 a3 t0 t1 t2 t3 t4 t5 t6 t7 "
             "s0 s1 s2 s3 s4 s5 s6 s7 t8 t9 k0 k1 gp sp fp ra").split()
_LOAD_OPS = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26}
_STORE_OPS = {0x28, 0x29, 0x2A, 0x2B, 0x2E}
_SP = 29


def _op(w: int) -> int:
    return w >> 26


def _rs(w: int) -> int:
    return (w >> 21) & 31


def _rt(w: int) -> int:
    return (w >> 16) & 31


def _rd(w: int) -> int:
    return (w >> 11) & 31


def _funct(w: int) -> int:
    return w & 63


def is_trapping_arith(w: int) -> str | None:
    """`add` / `addi` / `sub` (the overflow-trapping forms), else None."""
    if _op(w) == 0x08:
        return "addi"
    if _op(w) == 0 and _funct(w) in (0x20, 0x22):
        return "add" if _funct(w) == 0x20 else "sub"
    return None


def is_branch_or_jump(w: int) -> bool:
    op = _op(w)
    if op == 0:
        return _funct(w) in (0x08, 0x09)          # jr, jalr
    if op in (0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07):
        return True
    return op in (0x10, 0x11, 0x12, 0x13) and _rs(w) == 0x08  # bcNf/bcNt


def regs_read_written(w: int) -> tuple[set[int], set[int]]:
    """GPRs an instruction word reads / writes (R3000 ISA, $zero dropped)."""
    op, rs, rt, rd, fn = _op(w), _rs(w), _rt(w), _rd(w), _funct(w)
    r: set[int] = set()
    d: set[int] = set()
    if op == 0:
        if fn in (0x00, 0x02, 0x03):                    # sll/srl/sra
            r, d = {rt}, {rd}
        elif fn in (0x04, 0x06, 0x07):                  # sllv/srlv/srav
            r, d = {rs, rt}, {rd}
        elif fn == 0x08:                                 # jr
            r = {rs}
        elif fn == 0x09:                                 # jalr
            r, d = {rs}, {rd}
        elif fn in (0x10, 0x12):                         # mfhi/mflo
            d = {rd}
        elif fn in (0x11, 0x13):                         # mthi/mtlo
            r = {rs}
        elif 0x18 <= fn <= 0x1B:                         # mult/multu/div/divu
            r = {rs, rt}
        elif 0x20 <= fn <= 0x27 or fn in (0x2A, 0x2B):  # add..nor, slt/sltu
            r, d = {rs, rt}, {rd}
    elif op == 0x01:                                     # bltz/bgez(al)
        r = {rs}
        if rt in (0x10, 0x11):
            d = {31}
    elif op == 0x03:                                     # jal
        d = {31}
    elif op in (0x04, 0x05):                             # beq/bne
        r = {rs, rt}
    elif op in (0x06, 0x07):                             # blez/bgtz
        r = {rs}
    elif 0x08 <= op <= 0x0E:                             # addi..xori
        r, d = {rs}, {rt}
    elif op == 0x0F:                                     # lui
        d = {rt}
    elif op in (0x10, 0x12):                             # cop0 / cop2 moves
        if rs in (0x00, 0x02):                           # mfcN/cfcN
            d = {rt}
        elif rs in (0x04, 0x06):                         # mtcN/ctcN
            r = {rt}
    elif op in _LOAD_OPS:
        r, d = {rs}, {rt}
        if op in (0x22, 0x26):                           # lwl/lwr merge into rt
            r = {rs, rt}
    elif op in _STORE_OPS:
        r = {rs, rt}
    elif op in (0x32, 0x3A):                             # lwc2/swc2
        r = {rs}
    r.discard(0)
    d.discard(0)
    return r, d


def signal_trapping_arith(insns: list[Insn]) -> tuple[bool, str]:
    """S9: an overflow-trapping `add` / `addi` / `sub`.

    GCC 2.7.2's MIPS backend emits integer addition and subtraction only as
    the non-trapping `addu` / `addiu` / `subu` (mips.md addsi3_internal /
    subsi3_internal / negsi2 / abssi2); its `add` templates are
    mips.c:1140-1145, behind HALF_PIC_P(), which the PsyQ `-mno-abicalls
    -G0` build never enables, and mips.c:1171 (mips_move_1word PLUS),
    unreachable because movsi's general_operand rejects a raw PLUS. Decoded from the instruction
    WORD (opcode 0x08, or SPECIAL funct 0x20 / 0x22), so splat's spelling
    cannot hide or fake a hit. GCC-impossible: one occurrence is decisive."""
    hits = []
    for i, ins in enumerate(insns):
        if ins.word is None:
            continue
        kind = is_trapping_arith(ins.word)
        if kind:
            where = f"{ins.addr:08X}" if ins.addr is not None else f"insn {i}"
            hits.append(f"{kind} @ {where}")
    if hits:
        return True, f"{len(hits)} trapping op(s): " + ", ".join(hits[:4]) + (
            " ..." if len(hits) > 4 else "")
    return False, "no add/addi/sub"


_GTE_CMD = lambda w: _op(w) == 0x12 and _rs(w) >= 0x10  # noqa: E731


def _nop_explained(insns: list[Insn], i: int) -> str | None:
    """Why the nop at insns[i] may be compiler/assembler output, or None.

    Deliberately generous (every exemption widens what counts as "could be
    compiled"); with P the previous instruction, P2 the one before it and
    N the next non-nop instruction:
      - P is a nop                -> part of a run; the run's head decides
      - P is a branch/jump        -> delay slot (GCC `%#` fill, mips.c:3618)
      - P is a load, lwc2, mfc0/mfc2/cfc2 -> load-delay slot (maspsx,
        GCC epilogue mips.c:5200)
      - P or P2 is mfhi/mflo      -> HI/LO hazard (maspsx)
      - P is mult/div, break, syscall, rfe, a cop0/cop2 move or command,
        swc2; or N is a GTE command -> hazard/interlock padding (div
        expansion; PsyQ inline_c.h GTE macros)
      - first instruction, or no N -> entry / alignment padding."""
    if i == 0:
        return "entry"
    k = i + 1
    while k < len(insns) and insns[k].word == 0:
        k += 1
    if k == len(insns):
        return "tail padding"
    p, n = insns[i - 1].word, insns[k].word
    p2 = insns[i - 2].word if i >= 2 else None
    if p is None or n is None:
        return "undecodable"
    if p == 0:
        return "nop run"
    if is_branch_or_jump(p):
        return "delay slot"
    if _op(p) in _LOAD_OPS or _op(p) == 0x32 or (
            _op(p) in (0x10, 0x12) and _rs(p) in (0x00, 0x02)):
        return "load delay"
    for q in (p, p2):
        if q is not None and _op(q) == 0 and _funct(q) in (0x10, 0x12):
            return "HI/LO hazard"
    if _op(p) == 0 and (0x18 <= _funct(p) <= 0x1B or _funct(p) in (0x0C, 0x0D)):
        return "mult/div/trap"
    if _op(p) in (0x10, 0x12) or _op(p) == 0x3A or _GTE_CMD(n):
        return "coprocessor interlock"
    return None


def signal_hazard_free_nop(insns: list[Insn]) -> tuple[bool, str]:
    """S12: an explicit `nop` that no delay slot or hazard requires.

    Optimised GCC 2.7.2 emits a nop only to fill an empty delay slot (`%#`,
    mips.c:3618) or after the epilogue load (mips.c:5200); maspsx adds them
    only for load-delay and HI/LO hazards and the `div` expansion
    (maspsx/__init__.py:1070-1083); PsyQ's inline GTE macros pad around
    cop2 commands. Unoptimised GCC also emits a bare nop at a label or loop
    head (emit_nop, stmt.c:547), so a nop carrying a label, and any nop in a
    function that sets $fp from $sp (the -O0 frame shape), is exempt. A nop
    outside all of those (e.g. `addiu; nop; jal`) was typed by a human. See
    _nop_explained for the exact exemptions."""
    if any(w is not None and _op(w) == 0 and _funct(w) in (0x21, 0x25)
           and _rd(w) == 30 and _SP in (_rs(w), _rt(w))
           for w in (ins.word for ins in insns)):
        return False, "sets $fp from $sp (possible -O0 frame): not evaluated"
    hits = []
    for i, ins in enumerate(insns):
        if ins.word != 0 or ins.label:
            continue
        if _nop_explained(insns, i) is None:
            where = f"{ins.addr:08X}" if ins.addr is not None else f"insn {i}"
            hits.append(where)
    if hits:
        return True, f"{len(hits)} hazard-free nop(s) @ " + ", ".join(hits[:4]) + (
            " ..." if len(hits) > 4 else "")
    return False, "every nop fills a delay slot or covers a hazard"


def _s16(w: int) -> int:
    v = w & 0xFFFF
    return v - 0x10000 if v & 0x8000 else v


def _cs_reads_writes(w: int) -> tuple[set[int], set[int]]:
    """regs_read_written for S11: an lwl/lwr destination counts as a pure
    write (GCC emits them as a pair that defines the whole register)."""
    r, d = regs_read_written(w)
    if _op(w) in (0x22, 0x26):
        r = r - {_rt(w)}
    return r, d


def _cs_save_slots(insns: list[Insn]) -> dict[int, tuple[int, int, int]]:
    """{reg: (insn index, base reg, offset)} of each callee-saved
    register's prologue save: the first `sw $sN/$fp, K(base)` that comes
    before any write of that register, with base $sp or a register
    computed from $sp (GCC's MIPS_TEMP2 base for frames over 32 KB,
    mips.c save_restore_insns). Later stores of the same register are
    ordinary uses (outgoing args, spills)."""
    saves: dict[int, tuple[int, int, int]] = {}
    written: set[int] = set()
    sp_derived = {_SP}
    for i, ins in enumerate(insns):
        w = ins.word
        r, d = regs_read_written(w)
        if (_op(w) == 0x2B and _rs(w) in sp_derived and _rt(w) in CS_REGS
                and _rt(w) not in written and _rt(w) not in saves):
            saves[_rt(w)] = (i, _rs(w), _s16(w))
        if r & sp_derived and _op(w) not in _LOAD_OPS:
            sp_derived |= d
        written |= d
    return saves


def signal_custom_cs_abi(insns: list[Insn]) -> tuple[bool, str]:
    """S11: a callee-saved register ($s0-$s7, $fp) used as a register
    PARAMETER or RESULT rather than a preserved register.

      (a) live-in: in the entry block (up to and including the first
          branch/jump/call and its delay slot) the register is READ before
          any write, its prologue save excluded — the caller handed a
          value in through $sN;
      (b) dead-write: the register is WRITTEN in the body (the epilogue
          restore from its save slot excluded) but never READ anywhere in
          the function (the save excluded) — the value exists only for a
          callee or the caller.

    Under the standard ABI only $a0-$a3/$sp/$gp/$ra are live on entry to
    compiled code, and flow.c's insn_dead_p deletes a set of a dead
    register (the only hard-register exemptions are global register
    variables and the frame pointer, flow.c:1758). (a) is limited to the
    entry block because an uninitialised C local can legitimately be read
    on a later path. S7 catches a callee that never saves the register;
    S11 also catches a dispatcher that saves every callee-saved register
    yet hands values to its callees through them. Compiled-C escape paths,
    none present in the corpus: a global register variable, an
    uninitialised local read in the entry block. Assumes splat's function
    boundaries: a mis-split compiled function would read live-in $sN in its
    second half, so re-run the corpus measurement after any re-split."""
    if not insns or any(ins.word is None for ins in insns):
        return False, "undecodable (instruction without its word comment)"
    saves = _cs_save_slots(insns)
    save_idx = {v[0] for v in saves.values()}
    slots = {(v[1], v[2]) for v in saves.values()}
    live_in: set[int] = set()
    written: set[int] = set()
    for i, ins in enumerate(insns):
        r, d = _cs_reads_writes(ins.word)
        if i not in save_idx:
            live_in |= (r & CS_REGS) - written
        written |= d
        if i > 0 and is_branch_or_jump(insns[i - 1].word):
            break           # delay slot of the entry block's branch done
    ever_read: set[int] = set()
    first_write: dict[int, int] = {}
    for i, ins in enumerate(insns):
        w = ins.word
        r, d = _cs_reads_writes(w)
        if i in save_idx:
            r = r - {_rt(w)}
        if _op(w) == 0x23 and _rt(w) in saves and (
                (_rs(w), _s16(w)) in slots or (_rs(w) == _SP and _s16(w) == saves[_rt(w)][2])):
            d = d - {_rt(w)}  # restore from a save slot
        ever_read |= r & CS_REGS
        for reg in d & CS_REGS:
            first_write.setdefault(reg, ins.addr)
    parts = []
    if live_in:
        parts.append("live-in " + ", ".join("$" + REG_NAMES[r] for r in sorted(live_in)))
    dead = sorted(r for r in first_write if r not in ever_read)
    if dead:
        parts.append("written-never-read " + ", ".join(
            f"${REG_NAMES[r]} (first @ {first_write[r]:08X})" for r in dead))
    if parts:
        return True, "; ".join(parts)
    return False, "callee-saved registers follow the standard ABI"


def signal_cross_function_jtbl(func: str, insns: list[Insn],
                               jtbl_targets: dict[int, list[int]],
                               owner_of: dict[int, str]) -> tuple[bool, str]:
    """S10: a jump table referenced by this function has an entry that lands
    inside a DIFFERENT function's body.

    GCC 2.7.2 compiles one FUNCTION_DECL to completion before the next
    (toplev.c rest_of_compilation); switch case labels and computed-goto
    labels are scoped to that function (stmt.c case_stack, c-decl.c
    push_label_level/pop_label_level). No C source can emit a table entry
    that resolves into another function. `jtbl_targets` maps a table
    address to its entries (read from the oracle EXE, see
    load_jtbl_targets); `owner_of` maps a jlabel address to its function.
    Assumes splat's function boundaries (see S11)."""
    tables = sorted({int(m, 16) for ins in insns
                     for m in re.findall(r"%lo\(jtbl_([0-9A-Fa-f]{8})\)", ins.operands)})
    hits = []
    for t in tables:
        foreign = [(e, owner_of.get(e)) for e in jtbl_targets.get(t, [])
                   if owner_of.get(e) not in (None, func)]
        if foreign:
            others = sorted({o for _, o in foreign})
            hits.append(f"jtbl_{t:08X}: {len(foreign)}/{len(jtbl_targets[t])} "
                        f"entries inside {', '.join(others)}")
    if hits:
        return True, "; ".join(hits)
    if tables and not jtbl_targets:
        return False, "oracle EXE unavailable — not evaluated"
    return False, f"{len(tables)} jump table(s), all targets local"


EXE_PATH = ROOT / "disc" / "SLUS_006.63"
EXE_HEADER = 0x800
EXE_LOAD = 0x80010000


def load_jtbl_targets(parsed: list[tuple[Path, str, list[Insn]]],
                      exe_path: Path = EXE_PATH
                      ) -> tuple[dict[int, list[int]], dict[int, str]]:
    """({table addr: [entry addrs]}, {jlabel addr: owning function}).

    Entries are read from the oracle EXE: consecutive words from the table
    address, each of which must be a splat `jlabel` address somewhere in
    asm/funcs (splat labels every real table target), stopping at the next
    jtbl_ symbol. Returns ({}, owner_of) if the EXE is absent."""
    owner_of: dict[int, str] = {}
    tables: set[int] = set()
    for _p, name, insns in parsed:
        for ins in insns:
            if ins.jtarget and ins.addr is not None:
                owner_of[ins.addr] = name
            for m in re.findall(r"%lo\(jtbl_([0-9A-Fa-f]{8})\)", ins.operands):
                tables.add(int(m, 16))
    if not exe_path.exists():
        return {}, owner_of
    data = exe_path.read_bytes()
    order = sorted(tables)
    out: dict[int, list[int]] = {}
    for idx, t in enumerate(order):
        limit = order[idx + 1] if idx + 1 < len(order) else t + 4 * 1024
        entries = []
        a = t
        while a < limit:
            off = a - EXE_LOAD + EXE_HEADER
            if off < EXE_HEADER or off + 4 > len(data):
                break
            v = int.from_bytes(data[off:off + 4], "little")
            if v not in owner_of:
                break
            entries.append(v)
            a += 4
        out[t] = entries
    return out, owner_of


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
        if len(insns) < SCAN_FLOOR:
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

    jtbl_targets, owner_of = load_jtbl_targets(parsed)

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
        s8, s8d = signal_redundant_mask_shift(insns)
        s9, s9d = signal_trapping_arith(insns)
        s10, s10d = signal_cross_function_jtbl(name, insns, jtbl_targets, owner_of)
        s11, s11d = signal_custom_cs_abi(insns)
        s12, s12d = signal_hazard_free_nop(insns)

        hits = {"S1": s1, "S2": s2, "S6": s6, "S7": s7, "S8": s8,
                "S9": s9, "S10": s10, "S11": s11, "S12": s12}
        score = sum([s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12])
        tier, tier_reason = classify_tier(hits, score)
        out.append(FuncSignals(
            func=name,
            src_file=p.relative_to(ROOT).as_posix(),
            insn_count=len(insns),
            multu_count=multu_count,
            s1_uniform_multu=s1, s1_detail=s1d,
            s2_empty_branch=s2, s2_detail=s2d,
            s3_no_spills=s3, s3_detail=s3d,
            s4_front_loads=s4, s4_detail=s4d,
            s5_cluster=s5, s5_detail=s5d,
            s6_bios_jumptable=s6, s6_detail=s6d,
            s7_no_save_callee_use=s7, s7_detail=s7d,
            s8_redundant_mask_shift=s8, s8_detail=s8d,
            s9_trapping_arith=s9, s9_detail=s9d,
            s10_cross_func_jtbl=s10, s10_detail=s10d,
            s11_custom_cs_abi=s11, s11_detail=s11d,
            s12_hazard_free_nop=s12, s12_detail=s12d,
            gate_signals=[k for k in GATE_CLASS if hits.get(k)],
            score=score,
            tier=tier,
            tier_reason=tier_reason,
            opcode_sig=compute_opcode_sig(insns),
        ))
    return out


def floor_record(p: Path, name: str, insns: list[Insn]) -> FuncSignals:
    """The LOW record of a function under the scan floor: no signal ran, so
    none fired (the --all bulk dump likewise leaves it LOW)."""
    off = {f: False for f in FuncSignals.__dataclass_fields__
           if f.startswith("s") and not f.endswith("_detail") and f[1].isdigit()}
    det = {f: f"not scanned ({FLOOR_REASON})" for f in FuncSignals.__dataclass_fields__
           if f.endswith("_detail")}
    return FuncSignals(
        func=name, src_file=p.relative_to(ROOT).as_posix(), insn_count=len(insns),
        multu_count=sum(1 for i in insns if i.mnemonic == "multu"),
        **off, **det, gate_signals=[], score=0, tier="LOW",
        tier_reason=FLOOR_REASON, opcode_sig="")


def analyze_one(asm_dir: Path, func: str) -> FuncSignals | None:
    p = asm_dir / f"{func}.s"
    if not p.exists():
        return None
    r = parse_func(p)
    if r and len(r[1]) < SCAN_FLOOR:
        return floor_record(p, r[0], r[1])
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
        ("S8 redundant mask", s.s8_redundant_mask_shift, s.s8_detail),
        ("S9 trapping arith ", s.s9_trapping_arith, s.s9_detail),
        ("S10 foreign jtbl  ", s.s10_cross_func_jtbl, s.s10_detail),
        ("S11 $sN as param  ", s.s11_custom_cs_abi, s.s11_detail),
        ("S12 hazard-free nop ", s.s12_hazard_free_nop, s.s12_detail),
    ]
    lines = [
        f"  HAND_CODED: tier={s.tier}  score={s.score}/12  ({s.func}, {s.insn_count} insns)",
        f"    Reason: {s.tier_reason}",
        f"    Gate-class signals: {', '.join(s.gate_signals) or 'none'}",
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
    args = ap.parse_args()

    if args.single:
        s = analyze_one(ASM_DIR, args.single)
        if not s:
            print(f"asm/funcs/{args.single}.s not found or unparseable", file=sys.stderr)
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
                "8" if s.s8_redundant_mask_shift else ".",
                "9" if s.s9_trapping_arith else ".",
                "A" if s.s10_cross_func_jtbl else ".",
                "B" if s.s11_custom_cs_abi else ".",
                "C" if s.s12_hazard_free_nop else ".",
            ])
            note_bits = []
            if s.s5_cluster:
                note_bits.append(f"cluster: {s.s5_detail.split(': ', 1)[-1]}")
            if s.s6_bios_jumptable:
                note_bits.append(f"bios: {s.s6_detail.split(': ', 1)[-1]}")
            if s.s7_no_save_callee_use:
                note_bits.append(f"unsaved: {s.s7_detail.split(': ', 1)[-1]}")
            if s.s8_redundant_mask_shift:
                note_bits.append(f"mask: {s.s8_detail.split(' @ ', 1)[0]}")
            if s.s10_cross_func_jtbl:
                note_bits.append(f"jtbl: {s.s10_detail}")
            if s.s11_custom_cs_abi:
                note_bits.append(f"abi: {s.s11_detail}")
            extra_note = ("  " + "  ".join(note_bits)) if note_bits else ""
            print(f"   {s.score:2}/12  {s.insn_count:5}  {s.multu_count:6}  {s.func:40}  [{sig_letters}]{extra_note}")
        print()

    print(f"# scan_hand_coded: {len(sigs)} functions scanned in asm/funcs/")
    print(f"# tiers: STRONG={len(strong)}  POSSIBLE={len(possible)}  TIGHT_C={len(tight_c)}  LOW={low_total}")
    print()
    print_block(
        "STRONG candidates — almost certainly hand-coded asm",
        strong,
        "A decisive gate-class signal (S6-S11) fires, or S1+S2 with "
        "tightness — all effectively impossible in GCC output. Signal "
        "letters: 1-9 = S1-S9, A = S10, B = S11, C = S12. Treat as "
        "canonically-asm absent strong counter-evidence; the grant path "
        "is .claude/rules/judge-sole-gate.md rule 3.",
    )
    print_block(
        "POSSIBLE candidates — investigate",
        possible,
        "S1 or S2 (the non-decisive gate-class signals) fires without "
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
