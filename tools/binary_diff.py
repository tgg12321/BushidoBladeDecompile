#!/usr/bin/env python3
"""binary_diff.py — shape-aligned binary diff with register-aware analysis.

Background:
    `dc.sh verify-c` and the older `dc.sh diff` align target.s vs mine.o by
    INDEX. When mine's length != target's length, every instruction past the
    first divergence shows as "different" — a single missing/extra instruction
    cascades to a 140-diff report. Worse, build_active.py feeds that cascaded
    byte-count to iter_log, which then flags a spurious REGRESSION when adding
    a delete rule legitimately drops a line.

    `tools/diff_align.py` already does sequence alignment on the binary words
    with relocation masking, but its output is a human-readable side-by-side
    aimed at the verify cycle, not the regfix-generation cycle.

What this adds:
    1. SHAPE alignment via difflib on opcode + register-stripped operand
       template. Cascade-immune by construction.
    2. Per-row CATEGORY classification:
         - MATCH         — bytes identical (after relocation mask)
         - REG-RENAME    — shape identical, register tokens differ
         - BRANCH-OFFSET — opcode + registers identical, branch immediate differs
         - STRUCTURAL    — different shape (replace/insert/delete in alignment)
    3. `count` mode exports {structural, rename, branch_offset} JSON. Used by
       iter_log via build_active to drive cascade-immune REGRESSION detection.
    4. `gen-substs` mode emits ready-to-apply regfix rules:
         <func>: subst "<mine-regex>" "<target-text>" @ <maspsx-idx>
       The maspsx index is the index in the post-regfix stream (live
       dump_text_indices --post-regfix). All registers canonicalized to numeric
       form ($v0 -> $2) to match maspsx output.
    5. `side-by-side` mode is a coloured human display with categories.

Why this matters (lessons from the exec_game match):
    - Auto-generating subst rules from a shape-aligned diff would have
      compressed ~4 hours of manual subst-writing into one command.
    - A cascade-immune REGRESSION flag would have stopped the agent thrashing
      across what looked like "140 -> 0 -> 140" oscillation but was actually
      "STRUCTURAL fixed -> REG-RENAME left -> bad subst regressed registers."
    - Branch-offset detection separated from STRUCTURAL routes to the
      `.word`-encoding escape hatch (see tools/word_branches.py).

Usage:
    python3 tools/binary_diff.py side-by-side <func>
    python3 tools/binary_diff.py gen-substs <func> [--apply]
    python3 tools/binary_diff.py count <func>
    python3 tools/binary_diff.py branch-offsets <func>

Library:
    from binary_diff import compute_diffs
    d = compute_diffs(func)
    d.structural_count, d.rename_count, d.branch_offset_count
    d.rows  # list of AlignedRow
"""
from __future__ import annotations

import argparse
import difflib
import json
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_FUNCS = ROOT / "asm" / "funcs"
BUILD = ROOT / "build" / "src"


# ---------------------------------------------------------------------------
# Register canonicalization
# ---------------------------------------------------------------------------

# ABI -> numeric MIPS GPR names. objdump uses ABI; target.s and maspsx
# usually use numeric. Canonicalize to numeric ($2, $3, ...).
_ABI_TO_NUM = {
    "zero": "0", "at": "1",
    "v0": "2", "v1": "3",
    "a0": "4", "a1": "5", "a2": "6", "a3": "7",
    "t0": "8", "t1": "9", "t2": "10", "t3": "11",
    "t4": "12", "t5": "13", "t6": "14", "t7": "15",
    "s0": "16", "s1": "17", "s2": "18", "s3": "19",
    "s4": "20", "s5": "21", "s6": "22", "s7": "23",
    "t8": "24", "t9": "25",
    "k0": "26", "k1": "27",
    "gp": "28", "sp": "29", "fp": "30", "s8": "30", "ra": "31",
}

_REG_TOKEN_RE = re.compile(r"\$(\w+)")

# objdump emits bare ABI register names without the $ prefix: `lui v0,0x0`
# (where target.s uses `lui $2,0x0`). Normalize both forms to `$<num>`.
# Lookbehind: prev char must NOT be word-char or `$`, so `D_v0_func` doesn't
# get clobbered. `\b` on the right ensures whole-token match.
_BARE_ABI_RE = re.compile(
    r"(?<![\w$])(zero|at|v0|v1|a0|a1|a2|a3|t0|t1|t2|t3|t4|t5|t6|t7|"
    r"t8|t9|s0|s1|s2|s3|s4|s5|s6|s7|k0|k1|gp|sp|fp|ra)\b"
)


def canon_reg(tok: str) -> str:
    """`$v0` or `$2` -> `$2`. `zero` -> `$0`."""
    name = tok.lstrip("$")
    if name.isdigit():
        return "$" + name
    return "$" + _ABI_TO_NUM.get(name, name)


def canon_regs_in_line(line: str) -> str:
    """Normalize register tokens.

    Handles both `$v0`/`$2` (assembler form) and bare `v0` (objdump form).
    The bare-ABI substitution runs after the `$X` one so we never double-
    process a token (a `$v0` becomes `$2`, not `$$2`).
    """
    line = _REG_TOKEN_RE.sub(lambda m: canon_reg(m.group(0)), line)
    line = _BARE_ABI_RE.sub(lambda m: "$" + _ABI_TO_NUM[m.group(1)], line)
    return line


# ---------------------------------------------------------------------------
# Instruction parsing
# ---------------------------------------------------------------------------

# Branch opcodes: PC-relative, 16-bit signed offset / 4. The immediate field
# is the resolved offset in objdump output; mine vs target can differ in this
# field even when shapes match (different label positions due to extra/missing
# instructions earlier). Classify these specially.
_BRANCH_OPS = {
    "b", "beq", "bne", "blez", "bgtz", "bltz", "bgez",
    "bltzal", "bgezal", "beql", "bnel", "blezl", "bgtzl",
    "bltzl", "bgezl", "bc0f", "bc0t", "bc1f", "bc1t", "bc2f", "bc2t",
    "beqz", "bnez",  # pseudos
}

# Jump opcodes: 26-bit absolute targets. j/jal in unlinked .o show 0 for the
# relocation; in target.s show the actual address. Mask for alignment.
_JUMP_OPS = {"j", "jal"}

_MNE_RE = re.compile(r"^\s*([a-z][a-z0-9.]*)\b\s*(.*)$")


def parse_instr(line: str) -> tuple[str, str]:
    """Return (mnemonic, operands) after register canonicalization."""
    line = canon_regs_in_line(line.strip())
    m = _MNE_RE.match(line)
    if not m:
        return ("", line)
    return (m.group(1), m.group(2).strip())


# ---------------------------------------------------------------------------
# Loading mine and target
# ---------------------------------------------------------------------------

def find_object_for_func(func: str) -> Path | None:
    """Same logic as diff_align.find_object_for_func — use nm to find the
    object file that DEFINES (not just references) the symbol."""
    if not BUILD.is_dir():
        return None
    for o in sorted(BUILD.glob("*.o")):
        try:
            out = subprocess.check_output(
                ["mipsel-linux-gnu-nm", str(o)], stderr=subprocess.DEVNULL
            ).decode()
        except subprocess.CalledProcessError:
            continue
        for line in out.splitlines():
            parts = line.strip().split()
            if len(parts) >= 3 and parts[1] in ("T", "t") and parts[2] == func:
                return o
    return None


def read_mine(func: str) -> list[tuple[int, str]]:
    """objdump build/src/<file>.o for <func>. Returns [(word, instr_text), ...]
    with registers canonicalized to numeric form."""
    o = find_object_for_func(func)
    if o is None:
        raise FileNotFoundError(f"no built .o file contains {func} (run make?)")
    out = subprocess.check_output(
        ["mipsel-linux-gnu-objdump", "-d", "-EL", str(o)]
    ).decode()
    rows: list[tuple[int, str]] = []
    in_func = False
    for line in out.splitlines():
        if f"<{func}>:" in line:
            in_func = True
            continue
        if in_func and re.match(r"^[0-9a-f]+\s+<", line):
            break
        if not in_func:
            continue
        m = re.match(r"\s+[0-9a-f]+:\s+([0-9a-f]+)\s+(.*)", line)
        if m:
            word = int(m.group(1), 16)
            text = canon_regs_in_line(m.group(2).strip())
            rows.append((word, text))
    return rows


def read_target(func: str) -> list[tuple[int, str]]:
    """Parse asm/funcs/<func>.s. Returns [(word, instr_text), ...]."""
    p = ASM_FUNCS / f"{func}.s"
    if not p.is_file():
        raise FileNotFoundError(str(p))
    line_re = re.compile(r"\s+/\*\s+\S+\s+\S+\s+([0-9A-Fa-f]+)\s+\*/\s+(.*)")
    rows: list[tuple[int, str]] = []
    for line in p.read_text(encoding="utf-8").splitlines():
        m = line_re.match(line)
        if m and len(m.group(1)) == 8:
            hb = m.group(1)
            be = hb[6:8] + hb[4:6] + hb[2:4] + hb[0:2]
            word = int(be, 16)
            text = canon_regs_in_line(m.group(2).strip())
            rows.append((word, text))
    return rows


# ---------------------------------------------------------------------------
# Shape extraction (register-blind canonical for difflib alignment)
# ---------------------------------------------------------------------------

# After register canonicalization, replace every `$N` with `$R` so that
# alignment is register-blind. Replace numeric immediates and offsets with
# `<N>` so that branch-offset / relocation drift doesn't break alignment.
_NUM_RE = re.compile(r"-?(?:0x[0-9a-fA-F]+|\d+)")


_BRANCH_JUMP_OPS = _BRANCH_OPS | _JUMP_OPS


def shape(instr_text: str) -> str:
    """Register-blind, immediate-blind canonical for alignment.

    Examples:
        `addiu $5,$0,1`               -> `addiu $R,$R,<N>`
        `lw $2,0($3)`                 -> `lw $R,<N>($R)`
        `bltz $2,5ba8 <exec_game+0xec>` -> `bltz $R,<T>`
        `bltz $2,.L410`               -> `bltz $R,<T>`
    """
    # Drop the `<sym+offset>` annotation that objdump adds to branches/jumps.
    s = re.sub(r"\s*<[^>]*>\s*$", "", instr_text)
    # Drop register-load comments like `# 800a2d3c <D_800A2D3C>`.
    s = re.sub(r"\s*#.*$", "", s)
    s = _REG_TOKEN_RE.sub("$R", s)
    s = _NUM_RE.sub("<N>", s)
    # For branch/jump opcodes, the trailing operand is a PC-relative target.
    # mine (objdump) shows it as bare hex `5ba8`; target.s shows it as a
    # local label `.L410`. Neither is reliably matched by the numeric regex
    # above, so normalize both forms to `<T>` after the fact.
    m = _MNE_RE.match(s)
    if m and m.group(1) in _BRANCH_JUMP_OPS:
        mne = m.group(1)
        ops = m.group(2)
        if "," in ops:
            head, _, _ = ops.rpartition(",")
            s = f"{mne} {head.strip()},<T>"
        else:
            s = f"{mne} <T>"
    return s


# ---------------------------------------------------------------------------
# Bytewise comparison + relocation masking (from diff_align approach)
# ---------------------------------------------------------------------------

_LS_OPCODES = {
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
    0x28, 0x29, 0x2A, 0x2B, 0x2E,
}


def mask_for_compare(word: int) -> int:
    """Mask out relocation-only bits for word equality after alignment.

    Conservatively masks low-16 of lui/load/store/ori/addiu and low-26 of
    j/jal. Branch immediates (low-16 of branch ops) are masked too — branch
    label drift is detected via the BRANCH-OFFSET category instead."""
    op = (word >> 26) & 0x3F
    if op == 0x0F:  # lui
        return word & 0xFFFF0000
    if op in (0x02, 0x03):  # j, jal
        return word & 0xFC000000
    if op == 0x01:  # REGIMM (bltz, bgez, ...)
        return word & 0xFFFF0000
    if op in (0x04, 0x05, 0x06, 0x07, 0x14, 0x15, 0x16, 0x17):
        # beq, bne, blez, bgtz, beql, bnel, blezl, bgtzl
        return word & 0xFFFF0000
    if op in _LS_OPCODES:
        return word & 0xFFFF0000
    if op in (0x09, 0x0D):  # addiu, ori
        return word & 0xFFFF0000
    return word


# ---------------------------------------------------------------------------
# Aligned-row representation
# ---------------------------------------------------------------------------

@dataclass
class AlignedRow:
    mine_idx: int | None       # 0-based index into mine (objdump) or None
    target_idx: int | None     # 0-based index into target.s or None
    mine_word: int | None
    target_word: int | None
    mine_text: str             # canonicalized text (or "" for None side)
    target_text: str
    category: str              # MATCH / REG-RENAME / BRANCH-OFFSET / STRUCTURAL

    @property
    def is_diff(self) -> bool:
        return self.category != "MATCH"


@dataclass
class DiffReport:
    func: str
    rows: list[AlignedRow] = field(default_factory=list)
    mine_len: int = 0
    target_len: int = 0

    @property
    def structural_count(self) -> int:
        return sum(1 for r in self.rows if r.category == "STRUCTURAL")

    @property
    def rename_count(self) -> int:
        return sum(1 for r in self.rows if r.category == "REG-RENAME")

    @property
    def branch_offset_count(self) -> int:
        return sum(1 for r in self.rows if r.category == "BRANCH-OFFSET")

    @property
    def total_diff(self) -> int:
        return sum(1 for r in self.rows if r.is_diff)


def _is_branch_op(word: int) -> bool:
    op = (word >> 26) & 0x3F
    return op in (0x01, 0x04, 0x05, 0x06, 0x07, 0x14, 0x15, 0x16, 0x17)


def classify(mine_word: int | None, target_word: int | None,
             mine_text: str, target_text: str) -> str:
    """Classify an aligned position.

    Both words are present (else STRUCTURAL by definition — one side is empty).
    """
    if mine_word is None or target_word is None:
        return "STRUCTURAL"
    if mine_word == target_word:
        return "MATCH"
    mw = mask_for_compare(mine_word)
    tw = mask_for_compare(target_word)
    if mw == tw:
        # The unmasked low bits differ (relocation slot or branch offset).
        # For branches, the low 16 is the PC-relative offset — surface as
        # BRANCH-OFFSET so fix-branch-drift can target it.
        if _is_branch_op(mine_word):
            return "BRANCH-OFFSET"
        # All other masked-equal cases are relocation slots that the
        # linker resolves identically to what target.s already shows. The
        # CHECKED .o (post-link via Makefile) would mask to identical
        # bytes — for our purposes this is MATCH.
        return "MATCH"
    # Masked bits differ. Mnemonic could still be the same (`move $X,$Y`
    # encodes to `addu $X,$Y,$0` — `move` is a pseudo). Compare the
    # opcode/funct nibbles modulo registers.
    # For now: if upper opcode and lower funct nibbles agree but register
    # positions differ -> REG-RENAME.
    op_m = (mine_word >> 26) & 0x3F
    op_t = (target_word >> 26) & 0x3F
    if op_m != op_t:
        return "STRUCTURAL"
    if op_m == 0:
        funct_m = mine_word & 0x3F
        funct_t = target_word & 0x3F
        if funct_m != funct_t:
            return "STRUCTURAL"
    # Same opcode (and funct for SPECIAL): masked diff is in the register
    # fields. Classify as register rename.
    return "REG-RENAME"


def compute_diffs(func: str) -> DiffReport:
    """Align mine.o vs target.s on MASKED words (relocation-immune). Each
    aligned row is then classified into MATCH / REG-RENAME / BRANCH-OFFSET /
    STRUCTURAL based on the bit-level diff and opcode positions.

    Alignment on masked words is what diff_align.py also uses; it correctly
    handles `0(reg)` vs `%lo(SYM)(reg)`, `0x1` vs `1`, comma whitespace,
    and pseudo expansions (`move` vs `addu zero`) — they all mask-equal.
    """
    mine = read_mine(func)
    target = read_target(func)
    mine_masked = [mask_for_compare(w) for w, _ in mine]
    target_masked = [mask_for_compare(w) for w, _ in target]

    sm = difflib.SequenceMatcher(a=mine_masked, b=target_masked, autojunk=False)
    rep = DiffReport(func=func, mine_len=len(mine), target_len=len(target))

    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            for k in range(i2 - i1):
                mw, mt = mine[i1 + k]
                tw, tt = target[j1 + k]
                cat = classify(mw, tw, mt, tt)
                rep.rows.append(AlignedRow(
                    mine_idx=i1 + k, target_idx=j1 + k,
                    mine_word=mw, target_word=tw,
                    mine_text=mt, target_text=tt,
                    category=cat,
                ))
        elif tag == "replace":
            # Inside a replace block, words don't mask-equal. They could
            # still be REG-RENAME if the opcode matches — let classify()
            # decide. Pair them up positionally as far as possible.
            mlen = i2 - i1
            tlen = j2 - j1
            common = min(mlen, tlen)
            for k in range(common):
                mw, mt = mine[i1 + k]
                tw, tt = target[j1 + k]
                cat = classify(mw, tw, mt, tt)
                rep.rows.append(AlignedRow(
                    mine_idx=i1 + k, target_idx=j1 + k,
                    mine_word=mw, target_word=tw,
                    mine_text=mt, target_text=tt,
                    category=cat if cat != "MATCH" else "STRUCTURAL",
                ))
            for k in range(common, mlen):
                mw, mt = mine[i1 + k]
                rep.rows.append(AlignedRow(
                    mine_idx=i1 + k, target_idx=None,
                    mine_word=mw, target_word=None,
                    mine_text=mt, target_text="",
                    category="STRUCTURAL",
                ))
            for k in range(common, tlen):
                tw, tt = target[j1 + k]
                rep.rows.append(AlignedRow(
                    mine_idx=None, target_idx=j1 + k,
                    mine_word=None, target_word=tw,
                    mine_text="", target_text=tt,
                    category="STRUCTURAL",
                ))
        elif tag == "delete":
            for k in range(i2 - i1):
                mw, mt = mine[i1 + k]
                rep.rows.append(AlignedRow(
                    mine_idx=i1 + k, target_idx=None,
                    mine_word=mw, target_word=None,
                    mine_text=mt, target_text="",
                    category="STRUCTURAL",
                ))
        elif tag == "insert":
            for k in range(j2 - j1):
                tw, tt = target[j1 + k]
                rep.rows.append(AlignedRow(
                    mine_idx=None, target_idx=j1 + k,
                    mine_word=None, target_word=tw,
                    mine_text="", target_text=tt,
                    category="STRUCTURAL",
                ))

    return rep


# ---------------------------------------------------------------------------
# Subst generation (gen-substs mode)
# ---------------------------------------------------------------------------

def get_maspsx_indices(func: str) -> list[str] | None:
    """Run `dc.sh dump-text <func> --post-regfix` and parse the per-line
    text. Returns the list of maspsx-stream lines (indexed by position), or
    None if dump-text isn't available."""
    try:
        out = subprocess.check_output(
            ["bash", "tools/dc.sh", "dump-text", func, "--post-regfix"],
            cwd=str(ROOT),
            stderr=subprocess.STDOUT,
        ).decode(errors="ignore")
    except (subprocess.CalledProcessError, FileNotFoundError):
        return None
    # Format from dump_text_indices.py (matches existing convention):
    #   "  IDX  TEXT"  (some tools indent; some don't)
    lines = []
    for ln in out.splitlines():
        m = re.match(r"\s*(\d+)\s+(.*)$", ln)
        if m:
            lines.append((int(m.group(1)), m.group(2)))
    if not lines:
        return None
    # Sort by index and return text only (idx == position in list).
    lines.sort(key=lambda x: x[0])
    return [t for _, t in lines]


# A regfix `subst` operates on the maspsx-stream line at idx N. To generate
# subst rules from a binary-aligned diff, we need to map the binary
# instruction (post-pseudo-expansion) back to the maspsx-line.
#
# Strategy:
#  1. Get the maspsx stream from `dump-text --post-regfix`.
#  2. Match each REG-RENAME binary row to the maspsx line that produced it.
#     For non-pseudo instructions this is one-to-one. For lui/lw pairs from
#     a `lw $X, SYM` pseudo, the mapping is two binary rows -> one maspsx
#     line. The subst rule operates on the maspsx line, so we emit only
#     one rule covering the relevant register-rename.

def emit_substs(rep: DiffReport, maspsx_lines: list[str] | None) -> list[str]:
    """Build regfix subst rules for REG-RENAME rows. Returns list of full
    "<func>: subst ..." lines."""
    out: list[str] = []
    seen_idx: set[int] = set()
    func = rep.func
    if maspsx_lines is None:
        out.append(f"# {func}: dump-text --post-regfix unavailable; cannot map to maspsx idx.")
        out.append(f"# {func}: regenerate after applying register renames at the binary level.")
        return out

    # Build a search index: shape -> list of maspsx indices with that shape.
    # We'll match each rename row by shape similarity to the closest unused
    # maspsx line.
    shape_to_idx: dict[str, list[int]] = {}
    for idx, line in enumerate(maspsx_lines):
        s = shape(line)
        shape_to_idx.setdefault(s, []).append(idx)

    for r in rep.rows:
        if r.category != "REG-RENAME":
            continue
        s = shape(r.mine_text)
        candidates = shape_to_idx.get(s, [])
        # Prefer the first unused candidate.
        idx = None
        for c in candidates:
            if c not in seen_idx:
                idx = c
                break
        if idx is None:
            out.append(f"# {func}: could not locate maspsx idx for {r.mine_text!r}")
            continue
        seen_idx.add(idx)
        # Build pattern + replacement. Pattern = mine's text with regs escaped;
        # replacement = target's text (registers from target).
        # Use the maspsx-line as the pattern source (closer to what regfix sees).
        ms_line = maspsx_lines[idx]
        pattern = build_subst_pattern(ms_line, r.mine_text, r.target_text)
        replacement = build_subst_replacement(ms_line, r.mine_text, r.target_text)
        out.append(f'{func}: subst "{pattern}" "{replacement}" @ {idx}')
    return out


def _escape_regex(s: str) -> str:
    """Escape regex meta-characters but preserve $ for $N register tokens."""
    out = []
    for c in s:
        if c in r"\.[]^$|+*?(){}":
            out.append("\\" + c)
        elif c == "\t":
            out.append("\\s+")
        else:
            out.append(c)
    # Collapse runs of whitespace into \s+
    return re.sub(r"\s+", r"\\s+", "".join(out))


def build_subst_pattern(ms_line: str, mine_text: str, target_text: str) -> str:
    """Build a regex pattern for the subst rule.

    Use the maspsx-line as the source (that's what regfix sees pre-build).
    Most renames are register-only, so the pattern should match the mnemonic
    + the specific register sequence currently in mine, with whitespace as \\s+.
    """
    # Just escape the maspsx line literally — it's the safest pattern.
    return _escape_regex(ms_line.strip())


def build_subst_replacement(ms_line: str, mine_text: str, target_text: str) -> str:
    """Build the replacement string. Take the maspsx line shape but swap in
    target's registers."""
    # Strategy: parse mnemonic + operands from ms_line; replace operand list
    # with target_text's operands. Use \t as the separator (matches existing
    # regfix.txt convention).
    m_ms = _MNE_RE.match(ms_line.strip())
    m_tg = _MNE_RE.match(target_text.strip())
    if not (m_ms and m_tg):
        return target_text.strip()
    mne = m_ms.group(1)
    target_ops = m_tg.group(2)
    if target_ops:
        return f"{mne}\\t{target_ops}"
    return mne


# ---------------------------------------------------------------------------
# Branch-offset extraction (for word_branches)
# ---------------------------------------------------------------------------

def emit_word_branches(rep: DiffReport) -> list[dict]:
    """For each BRANCH-OFFSET row, return enough info to emit a `.word`
    subst rule that hard-codes the target's encoding."""
    out = []
    for r in rep.rows:
        if r.category != "BRANCH-OFFSET":
            continue
        if r.target_word is None:
            continue
        # The full target word (32-bit big-endian as an int) is what should
        # be assembled. Emit it as .word\t0xXXXXXXXX.
        out.append({
            "mine_idx": r.mine_idx,
            "target_idx": r.target_idx,
            "mine_text": r.mine_text,
            "target_text": r.target_text,
            "target_word_hex": f"0x{r.target_word:08X}",
        })
    return out


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

GREEN = "\033[32m"
RED = "\033[31m"
YELLOW = "\033[33m"
CYAN = "\033[36m"
MAGENTA = "\033[35m"
DIM = "\033[2m"
RESET = "\033[0m"


def cmd_count(args):
    rep = compute_diffs(args.func)
    print(json.dumps({
        "func": args.func,
        "mine_len": rep.mine_len,
        "target_len": rep.target_len,
        "structural": rep.structural_count,
        "rename": rep.rename_count,
        "branch_offset": rep.branch_offset_count,
        "total": rep.total_diff,
    }, indent=2))
    return 0


def cmd_side_by_side(args):
    rep = compute_diffs(args.func)
    width = 56

    def fmt_text(t: str) -> str:
        if len(t) > width - 2:
            return t[:width - 5] + "..."
        return t

    def color_cat(cat: str) -> str:
        return {
            "MATCH": GREEN,
            "REG-RENAME": YELLOW,
            "BRANCH-OFFSET": MAGENTA,
            "STRUCTURAL": RED,
        }.get(cat, "")

    use_color = sys.stdout.isatty() and not args.no_color
    header = f"{'mine':<6} {'mine text':<{width}}  {'target text':<{width}}  category"
    print(header)
    print("-" * len(header))
    for r in rep.rows:
        if r.category == "MATCH" and not args.all:
            continue
        col = color_cat(r.category) if use_color else ""
        rst = RESET if use_color else ""
        mi = f"{r.mine_idx}" if r.mine_idx is not None else "  -"
        print(f"{mi:>4}  {col}{fmt_text(r.mine_text):<{width}}{rst}  "
              f"{col}{fmt_text(r.target_text):<{width}}{rst}  {col}{r.category}{rst}")

    print()
    print(f"Summary: mine={rep.mine_len} target={rep.target_len}  "
          f"STRUCTURAL={rep.structural_count}  REG-RENAME={rep.rename_count}  "
          f"BRANCH-OFFSET={rep.branch_offset_count}")
    if rep.structural_count == 0 and rep.rename_count == 0 and rep.branch_offset_count > 0:
        print()
        print(f"  → Pure branch-offset end-game ({rep.branch_offset_count} diff(s)).")
        print(f"    Use: bash tools/dc.sh fix-branch-drift {args.func}")
    elif rep.structural_count == 0 and rep.rename_count > 0:
        print()
        print(f"  → No structural differences — only register renames.")
        print(f"    Use: bash tools/dc.sh gen-substs {args.func} --apply")
    return 0


def cmd_gen_substs(args):
    rep = compute_diffs(args.func)
    if rep.rename_count == 0:
        print(f"No REG-RENAME rows for {args.func}.")
        if rep.structural_count > 0:
            print(f"  ({rep.structural_count} STRUCTURAL diff(s) remain — "
                  f"gen-substs only handles register renames.)")
        return 0
    maspsx = get_maspsx_indices(args.func)
    lines = emit_substs(rep, maspsx)
    if not lines:
        print("No subst rules emitted.")
        return 0
    if args.apply:
        rf = ROOT / "regfix.txt"
        with rf.open("a", encoding="utf-8") as f:
            f.write(f"\n# auto: binary_diff gen-substs for {args.func}\n")
            for ln in lines:
                f.write(ln + "\n")
        print(f"Appended {len(lines)} line(s) to regfix.txt")
    else:
        print(f"# Dry run for {args.func} (use --apply to append to regfix.txt):")
        for ln in lines:
            print(ln)
    return 0


def cmd_branch_offsets(args):
    rep = compute_diffs(args.func)
    info = emit_word_branches(rep)
    if not info:
        print(f"No BRANCH-OFFSET rows for {args.func}.")
        return 0
    print(json.dumps(info, indent=2))
    return 0


def main() -> int:
    p = argparse.ArgumentParser(prog="binary_diff", description=__doc__.split("\n")[0])
    sub = p.add_subparsers(dest="cmd", required=True)

    s = sub.add_parser("count", help="Print {structural, rename, branch_offset, total} as JSON")
    s.add_argument("func")
    s.set_defaults(handler=cmd_count)

    s = sub.add_parser("side-by-side", help="Human-readable side-by-side diff")
    s.add_argument("func")
    s.add_argument("--all", action="store_true", help="Include MATCH rows")
    s.add_argument("--no-color", action="store_true")
    s.set_defaults(handler=cmd_side_by_side)

    s = sub.add_parser("gen-substs", help="Emit regfix subst rules for register renames")
    s.add_argument("func")
    s.add_argument("--apply", action="store_true",
                   help="Append rules to regfix.txt instead of printing")
    s.set_defaults(handler=cmd_gen_substs)

    s = sub.add_parser("branch-offsets", help="Dump BRANCH-OFFSET rows as JSON")
    s.add_argument("func")
    s.set_defaults(handler=cmd_branch_offsets)

    args = p.parse_args()
    return args.handler(args)


if __name__ == "__main__":
    sys.exit(main())
