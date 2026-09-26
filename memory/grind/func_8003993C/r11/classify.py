"""Classify permuter finds in r11/perm_pv by what the diff adds.
carrier : an existing per-value local (window/sel/entry_a/entry_b/i/idx/prog/save40/save58/e/p) is
          assigned a value it does not hold in pv.c (i.e. re-creates a multi-value variable)
copy    : a fresh `new_var*` local bound to a copy/address of an existing local or expression
other   : anything else (operand-order, arithmetic respellings, ...)
usage: python3 classify.py
"""
import os
import re

D = os.path.join(os.path.dirname(os.path.abspath(__file__)), "perm_pv")
base = open(os.path.join(D, "base.c")).read()
base_assign = set(re.findall(r"^\s*((?:window|sel|entry_a|entry_b|i|idx|prog|save40|save58|e|p|rob)\s*=[^;]*);", base, re.M))
rows = []
for d in os.listdir(D):
    m = re.match(r"output-(\d+)-(\d+)$", d)
    if not m:
        continue
    try:
        diff = open(os.path.join(D, d, "diff.txt")).read()
    except FileNotFoundError:
        continue
    plus = [l[1:].strip() for l in diff.splitlines() if l.startswith("+") and not l.startswith("+++")]
    kind = "other"
    for l in plus:
        a = re.match(r"((?:window|sel|entry_a|entry_b|i|idx|prog|save40|save58|e|p|rob)\s*=[^;=][^;]*);", l)
        if a and a.group(1) not in base_assign:
            kind = "carrier"
            break
        if re.search(r"\bnew_var\w*\s*=", l):
            kind = "copy"
    rows.append((int(m.group(1)), d, kind, " || ".join(plus)[:160]))
for r in sorted(rows):
    print("%5d %-16s %-8s %s" % r)
