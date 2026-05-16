#!/usr/bin/env python3
"""Audit asm-cheat patterns that bypass the pure-C decomp requirement.

Two patterns make the binary match without the C source actually
corresponding to the emitted bytes:

  1. LARGE SPLICE in regfix.txt
     `func: splice K..N "line1" "line2" ...` with N replacement lines.
     `splice` was designed for small structural transforms (3-line
     example in regfix.py docs). Replacing 100+ lines is essentially
     `replace_with_asmfile` reimplemented in regfix — the C source
     emits nothing useful, the bytes come from the splice content
     verbatim from asm/funcs/<func>.s.

  2. INLINE __asm__ FUNCTION BODY in src/*.c
     `__asm__(".section .text\\n... glabel funcname\\n ... endlabel ...")`
     A multi-instruction `__asm__` block containing `glabel funcname`
     is a file-scope asm function definition. GCC never sees the
     function body as C; the asm text goes straight to gas.

Both patterns are LEGITIMATE for functions whose ORIGINAL implementation
was hand-coded asm (see memory/feedback_hand_coded_asm_recognition.md).
Such functions must be listed in inline_asm_canonical.txt.

Anything not on that list is an UNAUTHORIZED cheat.

Modes:
  --all          : full report, exit 0
  --summary      : one-line counts ("S splices, I inline-asm, B BIOS trampolines"),
                   exit 0. Used by `dc.sh start` for session briefing.
  --func NAME    : check one function; exit 1 if unauthorized cheat found
  --check-new    : compare current state vs HEAD; exit 1 if new cheats added.
                   (Used by the active_func_guard hook before commit.)
  --list-funcs   : print one unauthorized function name per line, exit 0.
                   Used by queue generators to surface cheat-fix work.
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

# Thresholds: tuned to distinguish "small structural transform"
# (legitimate splice usage) from "full-body asm replacement".
LARGE_SPLICE_LINES = 30
GLABEL_BODY_LINES = 5

# 3-instruction BIOS trampoline: addiu $tN, $zero, 0xXX; jr $tN; addiu $tM,...
# Common in text1b_b.c func_80078XXX series. Legitimate but should be
# listed in inline_asm_canonical.txt — recommend adding rather than
# silently allowing.
BIOS_TRAMPOLINE = re.compile(
    r'addiu\s+\$t\d+,\s*\$zero,\s*0x[0-9A-Fa-f]+'
    r'.{0,80}?'
    r'jr\s+\$t\d+'
    r'.{0,80}?'
    r'addiu\s+\$t\d+,\s*\$zero,\s*0x[0-9A-Fa-f]+',
    re.DOTALL,
)


def load_authorized(path="inline_asm_canonical.txt"):
    auth = set()
    p = Path(path)
    if not p.exists():
        return auth
    for line in p.read_text(encoding="utf-8").split("\n"):
        name = line.split("#")[0].strip()
        if name:
            auth.add(name)
    return auth


def scan_regfix_splices(content):
    """Return list of (func, K_orig, N_repl, line_no) for splice rules."""
    cheats = []
    for i, line in enumerate(content.split("\n"), 1):
        m = re.match(r"^(\w+):\s+splice\s+(\d+)\.\.(\d+)\s+(.+)$", line)
        if m:
            func = m.group(1)
            k_orig = int(m.group(3)) - int(m.group(2)) + 1
            n_repl = len(re.findall(r'"[^"]*"', m.group(4)))
            cheats.append((func, k_orig, n_repl, i))
    return cheats


def scan_inline_asm_bodies(content, source_name):
    """Return list of (file, line_no, asm_lines, func_name, is_bios) for
    `__asm__` blocks containing `glabel funcname`."""
    cheats = []
    for m in re.finditer(r"__asm__\s*(?:volatile\s*)?\(([^)]+)\)", content):
        block = m.group(1)
        glabel_match = re.search(r"glabel\s+(\w+)", block)
        if not glabel_match:
            continue
        fname = glabel_match.group(1)
        insn_count = block.count("\\n")
        line_no = content[:m.start()].count("\n") + 1
        # BIOS trampoline: short block (only the 3 trampoline insns plus
        # section/set/glabel/endlabel scaffolding), and the trampoline
        # pattern is present. Reject larger blocks that happen to start
        # with the pattern but continue with .word data tables etc.
        is_bios = insn_count <= 12 and bool(BIOS_TRAMPOLINE.search(block))
        cheats.append((source_name, line_no, insn_count, fname, is_bios))
    return cheats


def get_current_cheats(auth):
    """Scan current working tree. Return (splice_funcs, inline_funcs, bios_funcs)."""
    splice_funcs = set()
    inline_funcs = set()
    bios_funcs = set()

    regfix = Path("regfix.txt")
    if regfix.exists():
        for func, k, n, line in scan_regfix_splices(regfix.read_text(encoding="utf-8")):
            if n >= LARGE_SPLICE_LINES and func not in auth:
                splice_funcs.add(func)

    for src in sorted(Path("src").glob("*.c")):
        for f, l, n, fname, is_bios in scan_inline_asm_bodies(
            src.read_text(encoding="utf-8"), src.name
        ):
            if n < GLABEL_BODY_LINES:
                continue
            if fname in auth:
                continue
            if is_bios:
                bios_funcs.add(fname)
            else:
                inline_funcs.add(fname)

    return splice_funcs, inline_funcs, bios_funcs


def get_head_cheats(auth):
    """Same scan but against HEAD revision (for --check-new diff)."""
    splice_funcs = set()
    inline_funcs = set()

    try:
        head_regfix = subprocess.run(
            ["git", "show", "HEAD:regfix.txt"],
            capture_output=True, text=True, check=True,
        ).stdout
        for func, k, n, line in scan_regfix_splices(head_regfix):
            if n >= LARGE_SPLICE_LINES and func not in auth:
                splice_funcs.add(func)
    except subprocess.CalledProcessError:
        pass

    # All .c files tracked at HEAD
    src_list = subprocess.run(
        ["git", "ls-tree", "-r", "--name-only", "HEAD", "src/"],
        capture_output=True, text=True,
    ).stdout.split("\n")
    for src_path in src_list:
        if not src_path.endswith(".c"):
            continue
        try:
            head_content = subprocess.run(
                ["git", "show", f"HEAD:{src_path}"],
                capture_output=True, text=True, check=True,
            ).stdout
        except subprocess.CalledProcessError:
            continue
        for f, l, n, fname, is_bios in scan_inline_asm_bodies(head_content, src_path):
            if n < GLABEL_BODY_LINES:
                continue
            if fname in auth or is_bios:
                continue
            inline_funcs.add(fname)

    return splice_funcs, inline_funcs


def cmd_all():
    auth = load_authorized()
    regfix_content = Path("regfix.txt").read_text(encoding="utf-8") if Path("regfix.txt").exists() else ""
    splices = scan_regfix_splices(regfix_content)
    full_splices = [s for s in splices if s[2] >= LARGE_SPLICE_LINES]
    unauth_splices = [s for s in full_splices if s[0] not in auth]

    print(f"=== regfix.txt splice rules ===")
    print(f"  {len(splices)} total, {len(full_splices)} full-body (>= {LARGE_SPLICE_LINES} replacement lines)")
    print(f"  {len(unauth_splices)} UNAUTHORIZED:")
    for func, k, n, line in sorted(unauth_splices, key=lambda x: -x[2]):
        print(f"    line {line:5d}  {func:32s}  orig_K={k:4d}  replace_N={n:4d}")

    print()
    print(f"=== Inline __asm__ glabel function bodies in src/*.c ===")
    inline_all = []
    for src in sorted(Path("src").glob("*.c")):
        inline_all.extend(
            scan_inline_asm_bodies(src.read_text(encoding="utf-8"), src.name)
        )
    significant = [b for b in inline_all if b[2] >= GLABEL_BODY_LINES]
    auth_blocks = [b for b in significant if b[3] in auth]
    bios = [b for b in significant if b[4] and b[3] not in auth]
    unauth = [b for b in significant if not b[4] and b[3] not in auth]
    print(f"  {len(significant)} with >= {GLABEL_BODY_LINES} lines:")
    print(f"    {len(auth_blocks):4d} authorized (in inline_asm_canonical.txt)")
    print(f"    {len(bios):4d} BIOS trampolines (3-insn pattern; should be added to authorized list)")
    print(f"    {len(unauth):4d} UNAUTHORIZED")
    for f, l, n, fname, _ in sorted(unauth, key=lambda x: -x[2])[:25]:
        print(f"    {f:25s}:{l:6d}  {fname:25s}  ~{n} lines")
    if len(unauth) > 25:
        print(f"    ... and {len(unauth) - 25} more")

    print()
    total_unauth = len(unauth_splices) + len(unauth)
    if total_unauth > 0:
        print(f"TOTAL UNAUTHORIZED ASM CHEATS: {total_unauth}")
        print(f"  ({len(bios)} BIOS trampolines should also be added to inline_asm_canonical.txt)")
    else:
        print("No unauthorized asm cheats. All inline-asm bodies and full-body splices are accounted for.")
    return 0


def cmd_func(name):
    auth = load_authorized()
    if name in auth:
        return 0
    regfix_content = Path("regfix.txt").read_text(encoding="utf-8") if Path("regfix.txt").exists() else ""
    for func, k, n, line in scan_regfix_splices(regfix_content):
        if func == name and n >= LARGE_SPLICE_LINES:
            print(f"UNAUTHORIZED: {name} has full-body splice in regfix.txt:{line} "
                  f"({n}-line replacement). Add to inline_asm_canonical.txt or do real C decomp.",
                  file=sys.stderr)
            return 1
    for src in sorted(Path("src").glob("*.c")):
        for f, l, n, fname, is_bios in scan_inline_asm_bodies(
            src.read_text(encoding="utf-8"), src.name
        ):
            if fname == name and n >= GLABEL_BODY_LINES and not is_bios:
                print(f"UNAUTHORIZED: {name} has inline __asm__ body in {f}:{l} "
                      f"(~{n} lines). Add to inline_asm_canonical.txt or do real C decomp.",
                      file=sys.stderr)
                return 1
    return 0


def cmd_check_new():
    """Return 1 if working tree has cheats not present at HEAD."""
    auth = load_authorized()
    cur_splice, cur_inline, cur_bios = get_current_cheats(auth)
    head_splice, head_inline = get_head_cheats(auth)

    new_splices = cur_splice - head_splice
    new_inline = cur_inline - head_inline

    if not new_splices and not new_inline:
        return 0

    print("ASM-CHEAT GUARD: new unauthorized asm-cheat patterns since HEAD:", file=sys.stderr)
    for func in sorted(new_splices):
        print(f"  Large splice for {func} in regfix.txt", file=sys.stderr)
    for fname in sorted(new_inline):
        print(f"  Inline __asm__ glabel body for {fname} in src/*.c", file=sys.stderr)
    print(file=sys.stderr)
    print("These patterns make the binary match without the C source", file=sys.stderr)
    print("actually corresponding to the emitted bytes (the bytes come from", file=sys.stderr)
    print("asm injected via splice or file-scope __asm__).", file=sys.stderr)
    print(file=sys.stderr)
    print("If the original was hand-coded asm (see memory/", file=sys.stderr)
    print("feedback_hand_coded_asm_recognition.md), add the function name", file=sys.stderr)
    print("to inline_asm_canonical.txt with a justification comment, then", file=sys.stderr)
    print("re-commit.", file=sys.stderr)
    print(file=sys.stderr)
    print("If it's canonically C (~99% of the codebase), write a real C", file=sys.stderr)
    print("decomp instead of wrapping the asm.", file=sys.stderr)
    return 1


def cmd_summary():
    """One-line counts of unauthorized cheats. For session briefing."""
    auth = load_authorized()
    cur_splice, cur_inline, cur_bios = get_current_cheats(auth)
    n_splice = len(cur_splice)
    n_inline = len(cur_inline)
    n_bios = len(cur_bios)
    total = n_splice + n_inline
    if total == 0 and n_bios == 0:
        print("Cheats:   none — all inline asm and large splices are authorized")
    else:
        parts = []
        if n_splice:
            parts.append(f"{n_splice} splice")
        if n_inline:
            parts.append(f"{n_inline} inline-asm")
        if n_bios:
            parts.append(f"{n_bios} BIOS-trampoline (recommend adding to authorized list)")
        msg = ", ".join(parts)
        print(f"Cheats:   {total} unauthorized ({msg})")
        print(f"          run 'python3 tools/audit_asm_cheats.py --all' to list, or")
        print(f"          'dc.sh next-cheat' to pull a cheat-fix work item")
    return 0


def cmd_list_funcs():
    """Print one unauthorized function name per line."""
    auth = load_authorized()
    cur_splice, cur_inline, cur_bios = get_current_cheats(auth)
    for f in sorted(cur_splice | cur_inline):
        print(f)
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    g = ap.add_mutually_exclusive_group()
    g.add_argument("--all", action="store_true", help="full report of all cheats (default)")
    g.add_argument("--summary", action="store_true", help="one-line cheat counts")
    g.add_argument("--func", help="check one function; non-zero exit if unauthorized")
    g.add_argument("--check-new", action="store_true",
                   help="compare current state vs HEAD; non-zero exit if new cheats added")
    g.add_argument("--list-funcs", action="store_true",
                   help="print unauthorized function names, one per line")
    args = ap.parse_args()
    if args.summary:
        return cmd_summary()
    if args.list_funcs:
        return cmd_list_funcs()
    if args.check_new:
        return cmd_check_new()
    if args.func:
        return cmd_func(args.func)
    return cmd_all()


if __name__ == "__main__":
    sys.exit(main())
