#!/usr/bin/env python3
"""Auto-generate the frame-cascade regfix rules when a function's frame size
differs from target by N bytes.

When GCC picks a smaller (or larger) stack frame than target, the prologue
adjustment, every save/restore $sp-relative offset, and the epilogue adjustment
all shift by the delta. Manual: 20-30 mechanical regfix subst rules. This
tool produces the cascade in one shot.

Usage:
    python3 tools/frame_shift.py <func> [--delta N] [--apply]

Without --delta, auto-detects:
    target_frame -> first `addiu $sp, $sp, -N` (or subu) in asm/funcs/<func>.s
    live_frame   -> same in the live maspsx output via dump_text_indices.py
    delta = target_frame - live_frame

Without --apply, dry-run prints the rules. With --apply, the batch is appended
atomically with rollback on validate_regfix --live failure.
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Prologue/epilogue $sp adjustments in raw maspsx (pre-regfix) output.
# GCC emits `subu $sp,$sp,N` for prologue and `addu $sp,$sp,N` for epilogue
# (some functions use addiu negative). Match both.
SP_ADJ_RE = re.compile(
    r'^(?P<mn>addiu|addu|subu|addi|sub|add)\s+'
    r'\$(?P<rd>sp|29),\s*'
    r'\$(?P<rs>sp|29),\s*'
    r'(?P<imm>-?(?:0x[0-9a-fA-F]+|\d+))\b'
)

# sw/lw with $sp-relative offsets.
SP_MEM_RE = re.compile(
    r'^(?P<mn>sw|lw|swc1|lwc1|swc2|lwc2)\s+'
    r'(?P<reg>\$\w+),\s*'
    r'(?P<off>-?(?:0x[0-9a-fA-F]+|\d+))'
    r'\(\$(?P<base>sp|29)\)\s*$'
)

# splat target asm: "    /* X X X */  mnemonic     operands"
TARGET_INSN_RE = re.compile(
    r'^\s*/\*[^*]*\*/\s*(?P<mn>\w+)\s+(?P<ops>.*?)\s*$'
)
TARGET_SP_MEM_RE = re.compile(
    r'^\$(?P<reg>\w+),\s*(?P<off>-?(?:0x[0-9a-fA-F]+|\d+))\(\$sp\)\s*$'
)
TARGET_SP_ADJ_RE = re.compile(
    r'^\$sp,\s*\$sp,\s*(?P<imm>-?(?:0x[0-9a-fA-F]+|\d+))\s*$'
)

# MIPS register name <-> number conversion (so we can compare $8 vs $t0).
_NUM_TO_NAME = {
    0: 'zero', 1: 'at', 2: 'v0', 3: 'v1', 4: 'a0', 5: 'a1', 6: 'a2', 7: 'a3',
    8: 't0', 9: 't1', 10: 't2', 11: 't3', 12: 't4', 13: 't5', 14: 't6', 15: 't7',
    16: 's0', 17: 's1', 18: 's2', 19: 's3', 20: 's4', 21: 's5', 22: 's6', 23: 's7',
    24: 't8', 25: 't9', 26: 'k0', 27: 'k1', 28: 'gp', 29: 'sp', 30: 'fp', 31: 'ra',
}
_NAME_TO_NUM = {v: k for k, v in _NUM_TO_NAME.items()}


def reg_to_num(r: str) -> int:
    """`$8` or `$t0` -> 8. Returns -1 if unrecognized."""
    if r.startswith('$'):
        r = r[1:]
    if r.isdigit():
        return int(r)
    return _NAME_TO_NUM.get(r, -1)


def parse_imm(s: str) -> int:
    return int(s, 0)


def parse_target_ops(func: str) -> dict[int, tuple[str, int, int]]:
    """Parse asm/funcs/<func>.s and return {target_idx: (mnemonic, reg_num, offset_or_imm)}.

    Only entries that are stack ops (mem with $sp base) or stack adjustments.
    target_idx is the linear instruction index within the function (matches
    dump_text_indices' idx since we share the same instruction sequence in
    the prologue/epilogue/save area).
    """
    path = ROOT / "asm" / "funcs" / f"{func}.s"
    if not path.exists():
        return {}
    out: dict[int, tuple[str, int, int]] = {}
    in_func = False
    insn_idx = 0
    glabel_re = re.compile(rf'^\s*glabel\s+{re.escape(func)}\b')

    for line in path.read_text().splitlines():
        if not in_func:
            if glabel_re.match(line):
                in_func = True
            continue
        # Stop at the next glabel (different function).
        if re.match(r'^\s*glabel\s+', line) and not glabel_re.match(line):
            break
        m = TARGET_INSN_RE.match(line)
        if not m:
            continue
        mn = m.group('mn')
        ops = m.group('ops').strip()

        # Stack memory op
        m2 = TARGET_SP_MEM_RE.match(ops)
        if m2 and mn in ('sw', 'lw', 'swc1', 'lwc1', 'swc2', 'lwc2'):
            rn = reg_to_num('$' + m2.group('reg'))
            off = parse_imm(m2.group('off'))
            out[insn_idx] = (mn, rn, off)
        # Stack adjustment
        elif mn in ('addiu', 'addi', 'addu', 'subu', 'sub', 'add'):
            m3 = TARGET_SP_ADJ_RE.match(ops)
            if m3:
                imm = parse_imm(m3.group('imm'))
                out[insn_idx] = (mn, reg_to_num('$sp'), imm)
        insn_idx += 1

    return out


def get_target_frame(func: str) -> int | None:
    """Read asm/funcs/<func>.s, return target's frame size (positive int)."""
    path = ROOT / "asm" / "funcs" / f"{func}.s"
    if not path.exists():
        print(f"frame-shift: ERROR: {path} not found", file=sys.stderr)
        return None
    for line in path.read_text().splitlines():
        # splat: "    /* X X X */  addiu      $sp, $sp, -0x58"
        m = re.search(
            r'(addiu|subu|addu|addi)\s+\$sp,\s*\$sp,\s*(-?0x[0-9a-fA-F]+|-?\d+)',
            line,
        )
        if m:
            val = parse_imm(m.group(2))
            return abs(val)
    return None


def get_live_insns(func: str) -> dict[int, str] | None:
    """Run dump_text_indices.py, return {idx: instruction_text}."""
    res = subprocess.run(
        [sys.executable, str(ROOT / "tools" / "dump_text_indices.py"), func],
        capture_output=True, text=True, cwd=str(ROOT),
    )
    insns: dict[int, str] = {}
    for line in res.stdout.splitlines():
        m = re.match(r'\s*(\d+):\s*(.*)', line)
        if m:
            insns[int(m.group(1))] = m.group(2).strip()
    return insns or None


def get_live_frame(insns: dict[int, str]) -> int | None:
    """Find prologue stack adjustment in live build, return frame size."""
    for idx in sorted(insns):
        m = SP_ADJ_RE.match(insns[idx])
        if not m:
            continue
        mn = m.group('mn')
        val = parse_imm(m.group('imm'))
        if mn in ('subu', 'sub') and val > 0:
            return val
        if mn in ('addiu', 'addi') and val < 0:
            return abs(val)
        # First non-prologue $sp adjustment encountered means we passed it
        # without finding it (e.g., function uses fp instead). Bail.
        break
    return None


def existing_frame_rules(func: str) -> list[str]:
    """Return existing regfix rules for `func` whose pattern touches $sp."""
    p = ROOT / "regfix.txt"
    if not p.exists():
        return []
    rules = []
    for line in p.read_text().splitlines():
        if not line.startswith(f"{func}:"):
            continue
        if re.search(r'subst.*\\\$sp', line) or re.search(r'subst.*\$sp', line):
            rules.append(line)
    return rules


def build_rules(func: str, delta: int, insns: dict[int, str],
                target_ops: dict[int, tuple[str, int, int]]
                ) -> list[tuple[int, str, str, str]]:
    """Return list of (idx, pattern, replacement, kind).

    For every $sp-relative instruction in mine's live build, look up the same
    idx in target's asm. Use target's actual offset for the replacement.
    Skip if target's idx has a different instruction type (mismatch =
    likely scheduling divergence at this idx; rule would be wrong)."""
    out: list[tuple[int, str, str, str]] = []
    callee_save_nums = set(range(16, 24)) | {30, 31}  # $s0-$s7, $fp, $ra

    for idx in sorted(insns):
        text = insns[idx]
        m = SP_ADJ_RE.match(text)
        if m:
            mn = m.group('mn')
            val = parse_imm(m.group('imm'))
            t_op = target_ops.get(idx)
            # Determine prologue vs epilogue by sign convention.
            # Prologue: subu/sub with positive val, OR addiu/addi with negative val
            is_prologue = (mn in ('subu', 'sub') and val > 0) or \
                          (mn in ('addiu', 'addi') and val < 0)
            is_epilogue = (mn in ('addu', 'add') and val > 0) or \
                          (mn in ('addiu', 'addi') and val > 0)
            if not (is_prologue or is_epilogue):
                continue

            # Use target's exact stack adj imm if available
            if t_op and t_op[1] == reg_to_num('$sp'):
                t_imm = t_op[2]
                pat = rf'{mn}\s+\$sp,\$sp,{val}'
                rep = f'addiu\t$29,$29,{t_imm}'
                kind = 'prologue' if is_prologue else 'epilogue'
                # Check if it's already correct (mine == target).
                if mn == 'addiu' and val == t_imm:
                    continue  # already matches
                out.append((idx, pat, rep, kind))
                continue
            # No target lookup — fall back to delta math (less reliable).
            if is_prologue:
                if mn in ('subu', 'sub'):
                    pat = rf'{mn}\s+\$sp,\$sp,{val}'
                    rep = f'addiu\t$29,$29,-{val + delta}'
                else:  # addiu negative
                    pat = rf'{mn}\s+\$sp,\$sp,{val}'
                    rep = f'addiu\t$29,$29,{val - delta}'
                out.append((idx, pat, rep, 'prologue'))
            else:
                pat = rf'{mn}\s+\$sp,\$sp,{val}'
                rep = f'addiu\t$29,$29,{val + delta}'
                out.append((idx, pat, rep, 'epilogue'))
            continue

        m = SP_MEM_RE.match(text)
        if m:
            mn = m.group('mn')
            reg = m.group('reg')
            off = parse_imm(m.group('off'))
            mine_reg_num = reg_to_num(reg)

            t_op = target_ops.get(idx)
            target_off: int | None = None
            if t_op and t_op[0] == mn and t_op[1] == mine_reg_num:
                target_off = t_op[2]
            elif mine_reg_num in callee_save_nums:
                # Callee-save register: target's offset must follow the +delta
                # shift (these positions are tied to frame size). Use that
                # even if idx-aligned target lookup failed (e.g., scheduling
                # divergence at this idx in target — but the save slot for
                # this register is still at off+delta).
                target_off = off + delta
            else:
                # Spill of a non-callee-save register. We don't know target's
                # offset without the idx-aligned lookup. Skip — emitting a
                # delta-shift rule would be wrong (target may use a different
                # spill slot entirely).
                continue

            if target_off == off:
                # Already correct, no rule needed.
                continue

            esc_reg = reg.replace('$', r'\$')
            pat = rf'{mn}\s+{esc_reg},{off}\(\$sp\)'
            rep = f'{mn}\t{reg},{target_off}($29)'
            if mine_reg_num in callee_save_nums:
                kind = 'save' if mn.startswith('s') else 'restore'
            else:
                kind = 'spill-st' if mn.startswith('s') else 'spill-ld'
            out.append((idx, pat, rep, kind))
    return out


def pre_validate(rules: list[tuple[int, str, str, str]],
                 insns: dict[int, str]) -> list[str]:
    failures = []
    for idx, pat, rep, kind in rules:
        if idx not in insns:
            failures.append(f"  [{kind}] @ {idx}: idx out of bounds")
            continue
        try:
            if not re.search(pat, insns[idx]):
                failures.append(
                    f"  [{kind}] @ {idx}: pattern {pat!r} does not match "
                    f"line {insns[idx]!r}"
                )
        except re.error as e:
            failures.append(f"  [{kind}] @ {idx}: invalid regex {pat!r}: {e}")
    return failures


def append_batch(rules: list[tuple[int, str, str, str]], func: str,
                 delta: int) -> int:
    """Atomically append all rules to regfix.txt; rollback on validate failure."""
    target = ROOT / "regfix.txt"
    original = target.read_bytes() if target.exists() else b""
    text = original.decode("utf-8") if original else ""
    if text and not text.endswith("\n"):
        text += "\n"
    text += f"\n# {func}: frame-shift delta {delta:+d} (auto-generated by frame_shift.py)\n"
    for idx, pat, rep, kind in rules:
        text += f'{func}: subst "{pat}" "{rep}" @ {idx}\n'
    target.write_bytes(text.encode("utf-8"))

    res = subprocess.run(
        [sys.executable, str(ROOT / "tools" / "validate_regfix.py"),
         "--live", "--func", func],
        capture_output=True, text=True, cwd=str(ROOT),
    )
    if res.returncode != 0:
        target.write_bytes(original)
        print("frame-shift: VALIDATION FAILED — rolled back the batch.",
              file=sys.stderr)
        print(res.stdout + res.stderr, file=sys.stderr)
        return 2
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("--delta", type=int, default=None,
                    help="Manual delta (target_frame - live_frame); "
                         "auto-detect if omitted")
    ap.add_argument("--apply", action="store_true",
                    help="Append rules to regfix.txt; default is dry-run")
    args = ap.parse_args()

    existing = existing_frame_rules(args.func)
    if existing:
        print(f"frame-shift: WARNING: {len(existing)} existing $sp-related rule(s) "
              f"in regfix.txt for {args.func}:", file=sys.stderr)
        for line in existing[:5]:
            print(f"  {line}", file=sys.stderr)
        if len(existing) > 5:
            print(f"  ... and {len(existing) - 5} more", file=sys.stderr)
        print("Refusing to add more frame rules. Remove existing ones first or "
              "edit them by hand.", file=sys.stderr)
        return 1

    insns = get_live_insns(args.func)
    if not insns:
        print(f"frame-shift: ERROR: dump_text_indices produced no output for "
              f"{args.func}", file=sys.stderr)
        return 1

    if args.delta is not None:
        delta = args.delta
        print(f"# Using manual delta: {delta:+d}", file=sys.stderr)
    else:
        target_frame = get_target_frame(args.func)
        if target_frame is None:
            print(f"frame-shift: ERROR: could not detect target frame from "
                  f"asm/funcs/{args.func}.s", file=sys.stderr)
            return 1
        live_frame = get_live_frame(insns)
        if live_frame is None:
            print(f"frame-shift: ERROR: could not detect live prologue $sp "
                  f"adjustment", file=sys.stderr)
            return 1
        delta = target_frame - live_frame
        print(f"# target frame: -{target_frame}, live frame: -{live_frame}, "
              f"delta: {delta:+d}", file=sys.stderr)

    if delta == 0:
        print("frame-shift: delta is 0 — frame sizes already match. Nothing to do.")
        return 0

    target_ops = parse_target_ops(args.func)
    rules = build_rules(args.func, delta, insns, target_ops)
    if not rules:
        print("frame-shift: no $sp instructions found. Nothing to do.")
        return 0

    fails = pre_validate(rules, insns)
    if fails:
        print("frame-shift: pre-validation FAILED:", file=sys.stderr)
        for f in fails:
            print(f, file=sys.stderr)
        return 1

    print(f"# Proposed {len(rules)} frame-shift rules for {args.func} "
          f"(delta {delta:+d}):")
    for idx, pat, rep, kind in rules:
        # Show replacement with literal \t for legibility
        rep_show = rep.replace("\t", "\\t")
        print(f"  [{kind:8s}] @ {idx:3d}: {pat!r} -> {rep_show!r}")

    if not args.apply:
        print()
        print("# Dry-run. Re-run with --apply to append rules to regfix.txt.")
        return 0

    rc = append_batch(rules, args.func, delta)
    if rc == 0:
        print(f"frame-shift: appended {len(rules)} rules to regfix.txt. "
              f"Run `make` to verify.")
    return rc


if __name__ == "__main__":
    sys.exit(main())
