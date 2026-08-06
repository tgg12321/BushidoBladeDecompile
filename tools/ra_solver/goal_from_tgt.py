#!/usr/bin/env python3
"""ra_solver.goal_from_tgt — goal derivation for `replace_with_asmfile` functions.

WHY THIS EXISTS (measured 2026-08-06)
-------------------------------------
`goal_from_asm.py` derives the goal by aligning two TEXT streams, `<stem>.hon.s`
against `<stem>.tgt.s`, and reading register substitutions off aligned pairs
whose register-blanked skeletons are IDENTICAL. That identity test is what keeps
it honest — a pair differing in mnemonic or immediate is a different
instruction, not a rename.

For a function wired with an asmfix `replace_with_asmfile` rule, the test can
never pass, so the derivation silently yields NOTHING:

  * `<stem>.tgt.s` carries that function as the split asm file's own text — a
    `glabel F` / `endlabel F` block whose instruction lines are prefixed
    `/* offset addr bytes */`. `goalmap.asm_body` skips `/*` lines and looks for
    `F:`, so it raises KeyError or returns a fragment.
  * Even after normalising the labels, the two sides are different LANGUAGES.
    `.hon.s` is assembler SOURCE (maspsx output); the target block is
    DISASSEMBLY. They spell the same instruction differently:

        ours    subu $sp,$sp,144        decimal, no spaces
        target  addiu $sp, $sp, -0x90   hex, comma-space, macro already expanded

    Both assemble to the same word, but every skeleton compares unequal.

The same blindness hits `inverse_compose.py classify`, which compares
register-blanked multisets: it sees a wholly different instruction multiset and
reports **PRE-RA / rtl_shape** — the one verdict meaning "stop, no RA or
scheduler perturbation can reach this". On func_80089F3C it said PRE-RA with a
287-vs-318 insn gap when the true codegen residual was ZERO.

THE FIX
-------
Do not canonicalise the text. There are too many assembler macro spellings for a
rewrite rule set to be trustworthy, and a wrong normalisation would produce
confident fiction — the exact failure this module exists to remove.

Instead compare the OBJECTS, which objdump renders canonically on BOTH sides:

    ours    tmp/sandbox/<func>/<stem>.o   the cheat-stripped honest build
    target  build/src/<stem>.o            the canonical build; for a
                                          replace_with_asmfile function this
                                          block IS the split asm file's bytes

`engine.score.normalized_insns(..., mask=True)` disassembles a single function
and masks control-flow targets, so relocation-shifted branch destinations do not
masquerade as differences. Alignment and attribution then reuse the PROVEN
machinery unchanged — `goalmap.align` and `goal_from_asm.attribute`.

This is a NEW file on purpose: `goal_from_asm.py` and its 702-function corpus
path are untouched by construction, so the corpus numbers cannot move.

CAVEAT — what this still cannot see
-----------------------------------
Operands carrying a relocation render unrelocated in OUR object (`lw a0,0(at)`)
and resolved in the reference (`lw a0,11588(at)`). Those pairs differ in
immediate, so the skeleton test correctly SKIPS them and no substitution is read
off them. That is deliberate: it is the same honesty rule goal_from_asm applies.
It means register exchanges occurring ONLY on relocated operands are invisible
here; `tools/relocsim.py` is the instrument for that class.

Usage (WSL, repo root, venv active):
  python3 tools/ra_solver/goal_from_tgt.py goal     <stem> <func> [--model M.json]
                                                    [--json out.json] [--show]
  python3 tools/ra_solver/goal_from_tgt.py classify <stem> <func>
"""
import argparse
import json
import re
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / "tools" / "sched_solver"))
sys.path.insert(0, str(Path(__file__).resolve().parent))

from engine import score                        # noqa: E402
from goalmap import align                       # noqa: E402
from goal_from_asm import attribute, rname      # noqa: E402
from pseudo_scope import scopes, candidates     # noqa: E402

REGNAMES = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
            "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
            "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
            "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"]
NUM = {n: i for i, n in enumerate(REGNAMES)}
# objdump prints bare register names, with no `$` prefix
REG_RE = re.compile(r"\b(?:" + "|".join(REGNAMES) + r")\b")


def obj_insns(stem, func, which):
    p = (f"tmp/sandbox/{func}/{stem}.o" if which == "ours"
         else f"build/src/{stem}.o")
    if not (ROOT / p).exists():
        raise SystemExit(
            f"missing {p} — for 'ours' run `engine sandbox {func} --disable all`; "
            f"for 'target' run `engine build`.")
    # RELATIVE path on purpose: engine.score builds its objdump command by
    # string interpolation without quoting, so an absolute path containing
    # spaces (this repo lives under "Bushido Blade 2 Decompile") yields an empty
    # symbol table and a bogus "not found". The engine only ever passes relative
    # paths from the repo root, which is why it never hits this.
    return score.normalized_insns(p, func, mask=True)


def regs_of(text):
    return [NUM[m.group(0)] for m in REG_RE.finditer(text)]


def blank_regs(text):
    return REG_RE.sub("#", text)


def substitutions(ours, tgt, verbose=False):
    """-> (Counter{(our, target): n}, detail). Same honesty rule as
    goal_from_asm.substitutions: only aligned pairs with IDENTICAL
    register-blanked skeletons contribute."""
    amap, _ = align(ours, tgt, "ours->tgt", verbose)
    subs, detail, skipped = Counter(), [], 0
    for i, j in enumerate(amap):
        if j is None:
            continue
        a, b = ours[i], tgt[j]
        if blank_regs(a) != blank_regs(b):
            skipped += 1
            continue
        ra, rb = regs_of(a), regs_of(b)
        if len(ra) != len(rb):
            skipped += 1
            continue
        pairs = [(x, y) for x, y in zip(ra, rb) if x != y]
        if pairs:
            for p in pairs:
                subs[p] += 1
            detail.append((i, j, a, b, pairs))
    return subs, detail, skipped


def cmd_classify(a):
    ours = obj_insns(a.stem, a.func, "ours")
    tgt = obj_insns(a.stem, a.func, "target")
    print(f"{a.func} ({a.stem}): ours {len(ours)} insns, target {len(tgt)} insns"
          "   [object-level: replace_with_asmfile-safe]")
    mo, mt = Counter(map(blank_regs, ours)), Counter(map(blank_regs, tgt))
    only_o, only_t = mo - mt, mt - mo
    nops = all(k.strip() == "nop" for k in (only_o + only_t))
    if (only_o or only_t) and nops:
        # A nop difference is DOWNSTREAM of RA (maspsx inserts load-delay nops
        # after the fact, so their count follows from what the schedule put in
        # the delay slot). It therefore does NOT settle the question on its own,
        # and must not short-circuit the RA check the way a real multiset
        # difference does — an RA exchange can coexist with it.
        print("\nSCHED component: nop-only multiset difference "
              f"(ours {sum(only_o.values())}, target {sum(only_t.values())})")
        print("  next tool: inverse_sched.py")
        subs, _, _ = substitutions(ours, tgt)
        if subs:
            print("\nAND an RA component — nops are downstream, so this is "
                  "still live:")
            for (o, t), n in sorted(subs.items(), key=lambda kv: -kv[1]):
                print(f"    {rname(o)} -> {rname(t)}   x{n}")
            print("  next tool: inverse.py (goal via `goal_from_tgt.py goal`)")
        else:
            print("\n  no register substitutions — the nops are the whole "
                  "residual.")
        return 0
    if only_o or only_t:
        print("\nFIRST DIVERGENCE: PRE-RA")
        print("  next tool: none — the residual is upstream of every model\n")
        print("  instruction shapes present in ONE stream only:")
        for k, n in list(only_o.items())[:12]:
            print(f"    ours only  : {k}   x{n}")
        for k, n in list(only_t.items())[:12]:
            print(f"    target only: {k}   x{n}")
        return 0
    subs, _, _ = substitutions(ours, tgt)
    if subs:
        print("\nFIRST DIVERGENCE: RA")
        print("  next tool: inverse.py (goal via `goal_from_tgt.py goal`)\n")
        for (o, t), n in sorted(subs.items(), key=lambda kv: -kv[1]):
            print(f"    {rname(o)} -> {rname(t)}   x{n}")
    else:
        if ours == tgt:
            print("\nNO DIVERGENCE: the two streams are identical.")
        else:
            print("\nFIRST DIVERGENCE: SCHED")
            print("  texts match as a multiset, order differs")
            print("  next tool: inverse_sched.py")
    return 0


_BR = re.compile(r"^(b|bal|beq|bne|blez|bgtz|bltz|bgez|beqz|bnez|j|jal|jr|jalr)\b")


def _block_starts(ours):
    """Basic-block start indices of the emitted stream. A block ends after a
    branch plus its delay slot; a branch TARGET starts one. Targets are masked
    to '@' here, so only the fall-through rule applies — which is why the
    resulting ordinals are compared against cc1's numbering, not trusted blind
    (see the confidence note printed with the results)."""
    starts = {0}
    for i, t in enumerate(ours):
        if _BR.match(t) and not t.startswith("jr"):
            starts.add(min(i + 2, len(ours)))
    return sorted(s for s in starts if s < len(ours))


def _block_of_index(starts, i):
    b = 0
    for k, s in enumerate(starts):
        if i >= s:
            b = k
    return b


def _scoped_attribution(a, ours, detail, disp):
    """Narrow attribution per substitution SITE by block scope, then union per
    goal register. Emits a goal entry only where exactly one candidate survives."""
    block_of, multi, _ = scopes(a.stem, a.func)
    starts = _block_starts(ours)
    holders = {}
    for p, r in disp.items():
        if r is not None and r >= 0:
            holders.setdefault(r, []).append(p)

    per_reg = {}
    for i, j, x, y, pairs in detail:
        blk = _block_of_index(starts, i)
        for o, t in pairs:
            cand = candidates(holders.get(o, []), blk, block_of, multi)
            key = (o, t)
            if key in per_reg:
                per_reg[key] &= set(cand)
            else:
                per_reg[key] = set(cand)

    print(f"\nscope-narrowed attribution ({len(starts)} emitted blocks; "
          f"{len(block_of)} block-local + {len(multi)} multi-block pseudos):")
    goal = {}
    for (o, t), cand in sorted(per_reg.items(), key=lambda kv: -len(kv[1])):
        allh = len(holders.get(o, []))
        c = sorted(cand)
        if len(c) == 1:
            goal[c[0]] = t
            print(f"  {rname(o)} -> {rname(t)}: {allh} holder(s) narrowed to "
                  f"UNIQUE pseudo {c[0]}")
        elif c:
            print(f"  {rname(o)} -> {rname(t)}: {allh} holder(s) narrowed to "
                  f"{len(c)} -> {c} (still ambiguous, no goal entry)")
        else:
            print(f"  {rname(o)} -> {rname(t)}: {allh} holder(s) narrowed to "
                  f"NONE — the register is a local-alloc quantity or a hard "
                  f"operand here, not a global allocno; use the `local` backend")
    print(f"\ngoal: {json.dumps({str(k): v for k, v in goal.items()})}")
    print("  confidence: block ordinals are derived from the emitted stream's "
          "fall-through\n  structure; cross-check a surprising attribution "
          "against `tools/blockmap.py`.")
    if a.json and goal:
        Path(a.json).write_text(json.dumps(
            {str(k): v for k, v in goal.items()}, indent=2))
        print(f"written: {a.json}")
    return 0


def cmd_goal(a):
    ours = obj_insns(a.stem, a.func, "ours")
    tgt = obj_insns(a.stem, a.func, "target")
    subs, detail, skipped = substitutions(ours, tgt, a.show)
    print(f"{a.func} ({a.stem}): ours {len(ours)} / target {len(tgt)} insns; "
          f"{len(detail)} renamed pair(s), {skipped} pair(s) skipped "
          f"(skeleton differs — reloc/immediate, not a rename)\n")
    if not subs:
        print("no register substitutions — this is not an RA residual, or the "
              "exchange lives only on relocated operands (see relocsim.py).")
        return 0
    print("register substitutions:")
    for (o, t), n in sorted(subs.items(), key=lambda kv: -kv[1]):
        print(f"  {rname(o)} -> {rname(t)}   x{n}")
    if a.show:
        print("\nper-instruction detail:")
        for i, j, x, y, pairs in detail:
            ps = ", ".join(f"{rname(p)}->{rname(q)}" for p, q in pairs)
            print(f"  ours[{i:3d}] {x:38s} tgt[{j:3d}] {y:38s} {ps}")
    if not a.model:
        print("\n(no --model: register-level only. Pass the model.json to "
              "attribute these onto pseudos.)")
        return 0
    model = json.loads(Path(a.model).read_text())
    disp = {int(k): v for k, v in model["dispositions"].items()}
    if a.scope:
        return _scoped_attribution(a, ours, detail, disp)
    goal, notes = attribute(subs, disp)
    print("\nattribution:")
    for n in notes:
        print(f"  {n}")
    print(f"\ngoal: {json.dumps({str(k): v for k, v in goal.items()})}")
    if a.json:
        Path(a.json).write_text(json.dumps(
            {str(k): v for k, v in goal.items()}, indent=2))
        print(f"written: {a.json}")
    return 0


def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="mode", required=True)
    g = sub.add_parser("goal", help="derive {pseudo: target hardreg}")
    g.add_argument("stem")
    g.add_argument("func")
    g.add_argument("--model", help="model.json (enables pseudo attribution)")
    g.add_argument("--json", help="write the derived goal here")
    g.add_argument("--show", action="store_true")
    g.add_argument("--scope", action="store_true",
                   help="narrow attribution per substitution site by .lreg block scope (see pseudo_scope.py)")
    g.set_defaults(fn=cmd_goal)
    c = sub.add_parser("classify", help="which model owns the residual")
    c.add_argument("stem")
    c.add_argument("func")
    c.set_defaults(fn=cmd_classify)
    a = ap.parse_args()
    return a.fn(a)


if __name__ == "__main__":
    sys.exit(main())
