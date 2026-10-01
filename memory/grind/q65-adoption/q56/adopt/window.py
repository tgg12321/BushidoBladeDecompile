#!/usr/bin/env python3
"""window.py <tree> <obj> <sym> : the per-file-gp-model.md cut-position test for one proven boundary.
In object <obj> (the oracle build of <tree>), symbol <sym> is reached gp-relative by some functions and by a
direct lui/%lo load/store by another, so a boundary lies between them. The window runs from the last function
that must stay in the earlier part (the last gp user before the first direct user) through the first function
that must be in the later part (that direct user). For every cut position in the window it tests:
  - no gp-reached small symbol is reached gp from both sides of the cut (rule: none shared across it);
  - rodata: whether the later part would own compiled rodata (a function after the cut referencing .rodata),
    and which item would start it.
Prints a markdown record; the conventional position is immediately before the direct user."""
import re, subprocess, sys
T, OBJ, SYM = sys.argv[1], sys.argv[2], sys.argv[3]
# optional: a rodata-align recorded window "after <A> up to <B>"; positions outside it fail rodata conditions
# 2-3 for a boundary set under that rule
RW = sys.argv[4:6] if len(sys.argv) > 5 else None
LOADSTORE = {"lb", "lbu", "lh", "lhu", "lw", "lwl", "lwr", "sb", "sh", "sw", "swl", "swr"}
out = subprocess.run(f"mipsel-linux-gnu-objdump -dr --no-show-raw-insn -M no-aliases '{T}/build/src/{OBJ}.o'",
                     shell=True, capture_output=True, text=True).stdout.splitlines()
order, gp, direct, rod = [], {}, {}, {}
fn, lui, last = None, {}, None
for ln in out:
    m = re.match(r"^[0-9a-f]+ <([^>]+)>:$", ln)
    if m:
        fn = m.group(1); order.append(fn); lui = {}; continue
    r = re.match(r"^\s*[0-9a-f]+:\s+(R_MIPS_\w+)\s+(\S+)$", ln)
    if r and last and fn:
        kind, sym = r.group(1), re.sub(r"\+0x[0-9a-f]+$", "", r.group(2))
        if kind == "R_MIPS_GPREL16":
            gp.setdefault(fn, set()).add(sym)
        elif kind == "R_MIPS_HI16":
            lui[last[1].split(",")[0]] = sym
        elif kind == "R_MIPS_LO16" and last[0] in LOADSTORE:
            b = re.search(r"\((\w+)\)", last[1])
            if b and lui.get(b.group(1)) == sym:
                direct.setdefault(fn, set()).add(sym)
        if sym.startswith((".rodata", "$LC", ".LC")) and kind in ("R_MIPS_HI16", "R_MIPS_LO16"):
            rod.setdefault(fn, set()).add(sym)
        continue
    i = re.match(r"^\s*[0-9a-f]+:\s+(\S+)\s*(.*)$", ln)
    if i:
        last = (i.group(1), i.group(2))
        regs = re.findall(r"\b(zero|at|v[01]|a[0-3]|t[0-9]|s[0-8]|k[01]|gp|sp|fp|ra)\b", last[1])
        if regs and last[0] != "lui" and last[0] not in ("sb", "sh", "sw", "swl", "swr") and not last[0].startswith(("b", "j")):
            lui.pop(regs[0], None)
first_direct = next(f for f in order if SYM in direct.get(f, ()))
k1 = order.index(first_direct)
last_gp = max(i for i, f in enumerate(order[:k1]) if SYM in gp.get(f, ()))
print(f"### Boundary in `{OBJ}` for `{SYM}`\n")
print(f"- gp user that must stay in the earlier part: `{order[last_gp]}`; direct (lui/%lo) user that must be in "
      f"the later part: `{first_direct}`.")
print(f"- Window: cut before any of {', '.join('`%s`' % f for f in order[last_gp + 1:k1 + 1])}.\n")
print("| cut before | gp symbols reached from both sides | later part's first compiled-rodata user | in the rodata-align window | result |")
print("|---|---|---|---|---|")
for c in range(last_gp + 1, k1 + 1):
    left = set().union(*[gp.get(f, set()) for f in order[:c]])
    right = set().union(*[gp.get(f, set()) for f in order[c:]])
    shared = sorted(left & right)
    rfirst = next((f for f in order[c:] if rod.get(f)), None)
    inw = True
    if RW:
        lo_i = order.index(RW[0]) if RW[0] in order else -1
        hi_i = order.index(RW[1]) if RW[1] in order else len(order)
        inw = lo_i < c <= hi_i
    ok = not shared and inw
    print(f"| `{order[c]}` | {', '.join(shared) or 'none'} | {('`%s`' % rfirst) if rfirst else 'none'} | "
          f"{('yes' if inw else 'no') if RW else 'n/a'} | {'survives' if ok else 'fails'}{' (conventional)' if c == k1 else ''} |")
