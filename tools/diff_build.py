#!/usr/bin/env python3
"""Side-by-side asm diff: target vs build-pipeline output for a function.

Unlike `dc.sh debug` (which shows permuter standalone vs target — useful
for permuter exploration but diverges from the actual build), this tool
shows what the BUILD pipeline produces vs the target binary. Aligns
instructions by sequential index so you can spot reorderings, register
choices, and ins/del at a glance.

Usage:
    python3 tools/diff_build.py <func>
    bash tools/dc.sh diff <func>
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_FUNCS = ROOT / "asm" / "funcs"


# ANSI colors
RESET = "\033[0m"
RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
DIM = "\033[2m"
BOLD = "\033[1m"


def find_source_file(func_name: str) -> Path | None:
    src_dir = ROOT / "src"
    decl_re = re.compile(rf'\b{re.escape(func_name)}\s*\(', re.MULTILINE)
    for c in sorted(src_dir.glob("*.c")):
        try:
            text = c.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        if decl_re.search(text):
            return c
    return None


def parse_target_asm(func_name: str) -> list[str] | None:
    """Read asm/funcs/<func>.s and extract just the instruction strings."""
    p = ASM_FUNCS / f"{func_name}.s"
    if not p.exists():
        return None
    out: list[str] = []
    # Format: /* OFFSET ADDRESS BYTES */  INSTRUCTION
    insn_re = re.compile(r'/\*\s*[\dA-Fa-f]+\s+[\dA-Fa-f]+\s+[\dA-Fa-f]+\s*\*/\s+(.+)$')
    for line in p.read_text(encoding="utf-8").splitlines():
        m = insn_re.match(line.strip())
        if m:
            out.append(normalize_insn(m.group(1).strip()))
    return out


def parse_build_asm(func_name: str) -> tuple[list[str] | None, str | None]:
    """Run dump_text_indices and extract instructions for a function."""
    src = find_source_file(func_name)
    if src is None:
        return None, f"{func_name} not found in src/"
    sys.path.insert(0, str(ROOT / "tools"))
    try:
        from dump_text_indices import build_pipeline_cmd  # type: ignore
    except ImportError:
        return None, "dump_text_indices not importable"
    cmd = build_pipeline_cmd(ROOT, src)
    result = subprocess.run(
        ["bash", "-c", cmd], capture_output=True, text=True, cwd=str(ROOT)
    )
    if not result.stdout.strip():
        return None, f"pipeline failed: {result.stderr[:300]}"

    # Run regfix.py over the output to get post-regfix view
    regfix_cmd = (
        f"{cmd} | python3 tools/regfix.py "
        f"| REGFIX_CONFIG=regfix_stage2.txt python3 tools/regfix.py "
        f"| python3 tools/asmfix.py"
    )
    result2 = subprocess.run(
        ["bash", "-c", regfix_cmd], capture_output=True, text=True, cwd=str(ROOT)
    )
    asm_text = result2.stdout if result2.stdout.strip() else result.stdout

    in_func = False
    out: list[str] = []
    func_label = re.compile(rf'^{re.escape(func_name)}:$')
    for line in asm_text.splitlines():
        s = line.strip()
        if func_label.match(s):
            in_func = True
            continue
        if in_func:
            if re.match(rf'^\s*\.end\s+{re.escape(func_name)}', s):
                break
            if not s or s.startswith(".") or s.startswith("#"):
                continue
            if s.endswith(":"):
                continue
            out.append(normalize_insn(s))
    return out, None


_REG_ALIAS = {
    "$zero": "$0", "$at": "$1", "$v0": "$2", "$v1": "$3",
    "$a0": "$4", "$a1": "$5", "$a2": "$6", "$a3": "$7",
    "$t0": "$8", "$t1": "$9", "$t2": "$10", "$t3": "$11",
    "$t4": "$12", "$t5": "$13", "$t6": "$14", "$t7": "$15",
    "$s0": "$16", "$s1": "$17", "$s2": "$18", "$s3": "$19",
    "$s4": "$20", "$s5": "$21", "$s6": "$22", "$s7": "$23",
    "$t8": "$24", "$t9": "$25", "$k0": "$26", "$k1": "$27",
    "$gp": "$28", "$sp": "$29", "$fp": "$30", "$s8": "$30",
    "$ra": "$31",
}
_OPCODE_ALIAS = {
    "move": "addu",        # `move $a, $b` = `addu $a, $b, $zero`
    "b": "j",              # `b L` = `j L` (unconditional)
    "beqz": "beq",         # `beqz $a, L` = `beq $a, $0, L`
    "bnez": "bne",         # `bnez $a, L` = `bne $a, $0, L`
    "negu": "subu",
    "li": "addiu",         # `li $a, K` may be `addiu $a, $0, K` or `lui+ori`
    "jr": "j",             # `jr $X` and `j $X` (with reg operand) encode the same
}


def normalize_insn(s: str) -> str:
    """Normalize for byte-identical comparison: register names, opcode
    aliases, integer formats, whitespace."""
    # Strip comments
    s = re.sub(r"#.*$", "", s)
    # Lowercase the opcode (target uses lowercase, mine sometimes mixed)
    s = s.strip()
    # Replace register name aliases ($v0 -> $2, etc.).
    # \b doesn't fire around $, so use a custom boundary: the alias
    # must be followed by a non-alphanumeric / non-underscore char (or
    # end of string), and preceded by a non-alphanumeric / non-$ char
    # (we already split tokens by whitespace, so any preceding char
    # works). Simplest: match the alias only when followed by a
    # delimiter.
    def _replace_reg(m):
        return _REG_ALIAS[m.group(0)]
    # Sort by length descending so $s8 (vs $s) doesn't get mis-substituted.
    keys = sorted(_REG_ALIAS.keys(), key=lambda k: -len(k))
    pat = "|".join(re.escape(k) for k in keys)
    s = re.sub(rf"({pat})(?![A-Za-z0-9_])", _replace_reg, s)
    # Normalize hex constants: 0x1C -> 28, 0xFFFF -> 65535 (compare as ints)
    def _hex_to_dec(m: re.Match) -> str:
        try:
            return str(int(m.group(0), 16))
        except ValueError:
            return m.group(0)
    s = re.sub(r"\b0x[0-9A-Fa-f]+\b", _hex_to_dec, s)
    # Normalize whitespace (commas, tabs, multiple spaces)
    s = re.sub(r"\s*,\s*", ",", s)
    s = re.sub(r"\s+", " ", s).strip()
    # Apply opcode aliases (first whitespace-separated token).
    parts = s.split(" ", 1)
    if parts and parts[0] in _OPCODE_ALIAS:
        parts[0] = _OPCODE_ALIAS[parts[0]]
        s = " ".join(parts)
    # `addu $X,$Y,N` (where N is a literal) is byte-equivalent to
    # `addiu $X,$Y,N`. Normalize.
    m = re.match(r"^addu (\$\d+),(\$\d+),(-?\d+)$", s)
    if m:
        s = f"addiu {m.group(1)},{m.group(2)},{m.group(3)}"
    # `subu $X,$Y,N` (literal positive) -> `addiu $X,$Y,-N`
    m = re.match(r"^subu (\$\d+),(\$\d+),(\d+)$", s)
    if m:
        s = f"addiu {m.group(1)},{m.group(2)},-{m.group(3)}"
    # `jalr $rs` = `jalr $31, $rs` = `jal $31, $rs` (link reg = $31 default)
    m = re.match(r"^jal(?:r)? (\$31,)?(\$\d+)$", s)
    if m:
        s = f"jalr {m.group(2)}"
    return s


def color_diff(target: list[str], mine: list[str]) -> None:
    """Print side-by-side aligned by index. Mark differences."""
    # We don't try to do LCS alignment — just zip by position. The dump
    # is already index-aligned so this works. If counts differ, the
    # tail is clearly marked.
    n = max(len(target), len(mine))
    target_w = 50

    same = 0
    diff = 0
    extra_target = 0
    extra_mine = 0

    print(f"  {'#':>4}  {'target (asm/funcs/<func>.s)':<{target_w}} | mine (build pipeline)")
    print(f"  {'-' * 4}  {'-' * target_w} | {'-' * target_w}")

    for i in range(n):
        t = target[i] if i < len(target) else ""
        m = mine[i] if i < len(mine) else ""

        if t == m:
            color = ""
            mark = " "
            same += 1
        elif t and not m:
            color = RED
            mark = "-"
            extra_target += 1
        elif m and not t:
            color = GREEN
            mark = "+"
            extra_mine += 1
        else:
            color = YELLOW
            mark = "!"
            diff += 1

        t_disp = t if len(t) <= target_w else t[: target_w - 1] + "…"
        m_disp = m if len(m) <= target_w else m[: target_w - 1] + "…"

        print(f"  {color}{mark} {i:>3}  {t_disp:<{target_w}} | {m_disp}{RESET}")

    print()
    print(f"  same: {same}    differ: {diff}    target-only: {extra_target}    "
          f"mine-only: {extra_mine}")
    print(f"  target insns: {len(target)}    mine insns: {len(mine)}    "
          f"delta: {len(mine) - len(target):+}")


def main() -> int:
    if len(sys.argv) != 2:
        print("Usage: diff_build.py <func>", file=sys.stderr)
        return 1
    func = sys.argv[1]
    target = parse_target_asm(func)
    if target is None:
        print(f"ERROR: asm/funcs/{func}.s not found", file=sys.stderr)
        return 1
    mine, err = parse_build_asm(func)
    if err:
        print(f"ERROR: {err}", file=sys.stderr)
        return 1
    if mine is None or not mine:
        print(f"ERROR: empty build output for {func}", file=sys.stderr)
        return 1
    print(f"=== {func} ===")
    print(f"  target: asm/funcs/{func}.s ({len(target)} insns)")
    print(f"  mine:   build pipeline post-regfix/asmfix ({len(mine)} insns)")
    print()
    color_diff(target, mine)
    return 0


if __name__ == "__main__":
    sys.exit(main())
