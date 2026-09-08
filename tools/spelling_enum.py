#!/usr/bin/env python3
"""Systematic spelling enumerator for a small residual (the `enumerate` grind modality).

The permuter mutates randomly and its metric is not the engine's; a hand search
covers dozens of spellings per session. For a residual that sits in ONE block
(a register seat, a scheduler tie, a delay-slot pick) the useful search space is
small and STRUCTURED: which sub-expressions are named locals, in what order the
declarations sit, and which way round each commutative product is written.
This tool enumerates that space exhaustively and writes one candidate file per
spelling, ready for `tools/sweep_variants.py` (or any scorer that takes a
directory of complete function bodies).

Mark the region in a candidate body (a complete function definition):

    /* ENUM-BEGIN */
    s32 ax = cx - x0;
    s32 dz = z2 - z0;
    s32 dx = x2 - x0;
    s32 az = cz - z0;
    kc = dz * ax - dx * az;
    kp = dz * (px - x0) - dx * (pz - z0);
    if ((kc ^ kp) >= 0)
    /* ENUM-END */
        return 1;

Rules inside the region (one statement per line):
  * `TYPE name = expr;`  — a NAMED local. Axis 1: keep it, or INLINE it
    (substitute `(expr)` at every use in the region and drop the declaration).
    Axis 2: the surviving declarations are emitted in every order that respects
    def-before-use. Write the region in the FULLY-NAMED form; inlining is what
    produces the CSE-merged spellings.
  * `name = expr;` — an assignment to an outer variable. Reordered among
    themselves when independent (def-before-use respected).
  * anything else (the `if`, a `return`) is an ANCHOR: stays last, in order.
  * Axis 3 (`--swaps`): every `A * B` / `A + B` whose operands are a single
    identifier or a parenthesised group is also emitted with the operands
    swapped (2^k spellings for k products; keep k small or use --no-swaps
    first and sweep swaps on the winners).

Semantics are preserved by construction: every variant computes the same
values in a valid order — the tool changes spelling only, never meaning. A low
score is a PROPOSAL, not a winner: vet the form against the cheat catalog
before adopting it (no-new-park-categories §auto-search).

Usage:
    python3 tools/spelling_enum.py --candidate memory/grind/<func>/candidate.c \
        --out tmp/grind/<func>/enum [--no-swaps] [--max 5000] [--seed-order]
    python3 tools/sweep_variants.py --func <func> --file <stem> --variants tmp/grind/<func>/enum --json
"""
from __future__ import annotations

import argparse
import itertools
import re
import sys
from pathlib import Path

BEGIN = "/* ENUM-BEGIN */"
END = "/* ENUM-END */"
_DECL_RE = re.compile(r"^\s*(?P<type>(?:const\s+)?(?:unsigned\s+|signed\s+)?[A-Za-z_]\w*(?:\s*\*)?)\s+"
                      r"(?P<name>[A-Za-z_]\w*)\s*=\s*(?P<expr>.+?);\s*$")
_ASSIGN_RE = re.compile(r"^\s*(?P<name>[A-Za-z_]\w*)\s*=\s*(?P<expr>.+?);\s*$")
_OPERAND = r"(?:[A-Za-z_]\w*|\([^()]*\))"
_PRODUCT_RE = re.compile(rf"(?<![\w)])({_OPERAND})\s*([*+])\s*({_OPERAND})(?![\w(])")


class Stmt:
    def __init__(self, kind, text, name=None, expr=None, ctype=None):
        self.kind, self.text, self.name, self.expr, self.ctype = kind, text, name, expr, ctype

    def uses(self, names):
        src = self.expr if self.expr is not None else self.text
        return {n for n in names if re.search(rf"\b{re.escape(n)}\b", src)}


def parse_region(lines):
    stmts = []
    for raw in lines:
        s = raw.strip()
        if not s:
            continue
        m = _DECL_RE.match(s)
        if m and m.group("type") not in ("return",):
            stmts.append(Stmt("decl", s, m.group("name"), m.group("expr"), m.group("type")))
            continue
        m = _ASSIGN_RE.match(s)
        if m:
            stmts.append(Stmt("assign", s, m.group("name"), m.group("expr")))
            continue
        stmts.append(Stmt("anchor", s))
    return stmts


def substitute(expr, name, repl):
    return re.sub(rf"\b{re.escape(name)}\b", repl, expr)


def inline_variant(stmts, inlined):
    """Return a new statement list with the given decl names inlined."""
    out = []
    exprs = {s.name: s.expr for s in stmts if s.kind == "decl"}
    # resolve inlined expressions transitively (an inlined var may use another)
    def resolved(name, seen=()):
        e = exprs[name]
        for n in inlined:
            if n != name and re.search(rf"\b{re.escape(n)}\b", e) and n not in seen:
                e = substitute(e, n, "(" + resolved(n, seen + (name,)) + ")")
        return e
    for s in stmts:
        if s.kind == "decl" and s.name in inlined:
            continue
        e = s.expr
        if e is not None:
            for n in inlined:
                if re.search(rf"\b{re.escape(n)}\b", e):
                    e = substitute(e, n, "(" + resolved(n) + ")")
            text = (f"{s.ctype} {s.name} = {e};" if s.kind == "decl" else f"{s.name} = {e};")
            out.append(Stmt(s.kind, text, s.name, e, s.ctype))
        else:
            t = s.text
            for n in inlined:
                if re.search(rf"\b{re.escape(n)}\b", t):
                    t = substitute(t, n, "(" + resolved(n) + ")")
            out.append(Stmt("anchor", t))
    return out


def orderings(stmts):
    """All def-before-use orders: decls first (any valid order), then assigns
    (any valid order), then anchors in place."""
    decls = [s for s in stmts if s.kind == "decl"]
    assigns = [s for s in stmts if s.kind == "assign"]
    anchors = [s for s in stmts if s.kind == "anchor"]
    dnames = [d.name for d in decls]

    def valid(seq):
        defined = set()
        for s in seq:
            if s.uses(dnames) - defined:
                return False
            if s.kind == "decl":
                defined.add(s.name)
        return True

    # a decl that no other decl depends on, and that depends on nothing, is free
    for dperm in itertools.permutations(decls):
        if not valid(dperm):
            continue
        for aperm in itertools.permutations(assigns):
            # assigns may read decls (all defined by now) and each other
            anames = [a.name for a in assigns]
            ok, defined = True, set()
            for a in aperm:
                if a.uses(anames) - defined:
                    ok = False
                    break
                defined.add(a.name)
            if ok:
                yield list(dperm) + list(aperm) + anchors


def swap_variants(text):
    """Yield text with every subset of commutative products swapped."""
    spans = list(_PRODUCT_RE.finditer(text))
    if not spans:
        yield text
        return
    for mask in itertools.product((0, 1), repeat=len(spans)):
        out, pos = [], 0
        for m, bit in zip(spans, mask):
            out.append(text[pos:m.start()])
            a, op, b = m.group(1), m.group(2), m.group(3)
            out.append(f"{b} {op} {a}" if bit else f"{a} {op} {b}")
            pos = m.end()
        out.append(text[pos:])
        yield "".join(out)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--candidate", required=True, help="candidate body carrying ENUM-BEGIN/END markers")
    ap.add_argument("--out", required=True, help="output directory for variant .c files")
    ap.add_argument("--no-swaps", action="store_true", help="skip axis 3 (commutative swaps)")
    ap.add_argument("--max", type=int, default=5000, help="refuse to emit more than this many variants")
    ap.add_argument("--keep-inlined", default="", help="comma list of names that must stay NAMED")
    ap.add_argument("--list", action="store_true", help="only print the count and axis summary")
    a = ap.parse_args()

    text = Path(a.candidate).read_text(encoding="utf-8", errors="replace")
    if BEGIN not in text or END not in text:
        print(f"markers {BEGIN} / {END} not found in {a.candidate}", file=sys.stderr)
        return 2
    pre, rest = text.split(BEGIN, 1)
    region, post = rest.split(END, 1)
    indent = re.match(r"[ \t]*", pre.splitlines()[-1] if pre.splitlines() else "").group(0)
    stmts = parse_region(region.splitlines())
    decls = [s for s in stmts if s.kind == "decl"]
    must_keep = {n for n in a.keep_inlined.split(",") if n}
    inlineable = [d.name for d in decls if d.name not in must_keep]

    variants = {}
    for k in range(len(inlineable) + 1):
        for inl in itertools.combinations(inlineable, k):
            base = inline_variant(stmts, set(inl))
            for order in orderings(base):
                body = "\n".join(indent + s.text for s in order)
                texts = [body] if a.no_swaps else list(swap_variants(body))
                for t in texts:
                    key = re.sub(r"\s+", " ", t)
                    if key in variants:
                        continue
                    variants[key] = t
                    if len(variants) > a.max:
                        print(f"more than --max {a.max} variants; narrow the region or use --no-swaps",
                              file=sys.stderr)
                        return 2
    print(f"region: {len(decls)} named locals ({', '.join(d.name for d in decls)}), "
          f"{sum(1 for s in stmts if s.kind == 'assign')} assignments, "
          f"{sum(1 for s in stmts if s.kind == 'anchor')} anchors -> {len(variants)} distinct spellings"
          + ("" if a.no_swaps else " (with commutative swaps)"))
    if a.list:
        return 0
    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    for old in out.glob("v*.c"):
        old.unlink()
    width = len(str(len(variants)))
    for i, t in enumerate(variants.values()):
        p = out / f"v{i:0{width}d}.c"
        with open(p, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(pre.rstrip(" \t") + t + "\n" + post.lstrip("\n"))
    print(f"wrote {len(variants)} variants to {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
