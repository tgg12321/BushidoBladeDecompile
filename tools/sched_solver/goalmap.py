#!/usr/bin/env python3
"""goalmap — express TARGET's instruction order in our RTL insn UIDs.

The chain, left to right, each hop verified:

  target .s  --difflib-->  honest .s  --difflib-->  cc1 .s  --index-->  .dbr UIDs

* `cc1 .s`     raw cc1 output.  The `.dbr` dump (final RTL, post-reorg) lists
  exactly the body instructions in emission order, so the two index-align 1:1;
  the epilogue is a UID-less suffix (GCC emits it from `function_epilogue`,
  not from RTL).
* `honest .s`  + prologue_fix | maspsx | multu_pad.  Same instructions plus
  inserted nops and a few macro expansions -- an insertion-only diff.
* `target .s`  + regfix | regfix_stage2 | asmfix.  These are the cheat stages,
  so this stream is the TARGET byte order by construction (the tree builds
  SHA1-identical to the original EXE).

Given a sched2 block's UID set, ordering those UIDs by their target position
yields the goal order for perturb.py.
"""
import argparse, difflib, json, re, sys
from pathlib import Path

INSN_OPEN = re.compile(
    r"^(\s*)\((insn|jump_insn|call_insn|code_label|barrier|note)"
    r"(?:[:/][^\s(]*)?\s+(\d+)")

# Maximum displacement accepted when pairing a leftover delete against a
# leftover insert (see align()).  A scheduling difference moves an instruction
# WITHIN its basic block, so the plausible range is a block length; the largest
# blocks in this corpus are ~20 insns and reorg can displace a delay-slot insn
# a little further.  Pairings beyond this are duplicate instruction text in a
# different block, not a move.
MAX_MOVE = 24

SKIP_DIR = (".set", ".frame", ".mask", ".fmask", ".ent", ".end", ".align",
            ".loc", ".size", ".type", ".globl", ".text", ".rdata", ".data",
            ".word", ".byte", ".half", ".space", ".ascii", ".sdata",
            ".section", ".comm", ".lcomm", ".file", ".ident", ".extern",
            ".gpword", ".p2align", ".previous", ".sbss", ".bss")


# --------------------------------------------------------------------------
def dbr_uids(root: Path, stem: str, func: str, work="tmp/sched_solver_work"):
    """Final RTL emission order for FUNC: [uid] (code_labels dropped).

    reorg.c's delay-slot filling is UNDONE here, because it runs after sched2
    and the model's pick order is the pre-reorg one.  A filled slot appears as
    `(insn N ... (sequence[ (jump_insn BR) (insn SLOT) ]))`; the wrapper N is
    not emitted, and the slot insn came from BEFORE the branch, so the
    pre-reorg order is SLOT then BR -- the children are emitted reversed.
    When reorg filled the slot by COPYING an insn from the branch target, that
    uid already occurs earlier in the stream; the copy is dropped."""
    lines = (root / work / f"{stem}.i.dbr").read_text(
        errors="replace").split("\n")
    banners = [i for i, l in enumerate(lines) if l.startswith(";; Function ")]
    hits = [i for i in banners if lines[i].split()[2] == func]
    if not hits:
        raise KeyError(f"{func} not in {stem}.i.dbr")
    i0 = hits[-1]
    i1 = next((s for s in banners if s > i0), len(lines))
    out, seq, seqs = [], None, []
    for l in lines[i0:i1]:
        m = INSN_OPEN.match(l)
        if not m:
            continue
        indent, kind, uid = len(m.group(1)), m.group(2), int(m.group(3))
        if indent == 0 and seq is not None:
            if len(seq) >= 2:
                seqs.append(tuple(seq[:2]))       # (branch idx, slot idx)
            seq = None
        if kind in ("note", "barrier", "code_label"):
            continue
        if indent == 0 and "(sequence[" in l:
            seq = []                              # wrapper uid is not emitted
            continue
        out.append(uid)
        if seq is not None and indent > 0:
            seq.append(len(out) - 1)
    if seq is not None and len(seq) >= 2:
        seqs.append(tuple(seq[:2]))
    return out, seqs


def asm_body(path: Path, func: str):
    """[(text, srcline)] for FUNC's body in an assembly file."""
    lines = path.read_text(errors="replace").split("\n")
    start = None
    for i, l in enumerate(lines):
        if l.strip().startswith(func + ":") or l.strip() == func + ":":
            start = i
            break
    if start is None:
        raise KeyError(f"{func} not found in {path.name}")
    out = []
    for n in range(start + 1, len(lines)):
        t = lines[n].strip()
        if t.startswith(".end"):
            break
        # another function's label ends the body
        if re.match(r"^[A-Za-z_][\w.$]*:$", t) and not t.startswith(("$L", ".L", "L")):
            break
        if not t or t.startswith("#") or t.startswith("/*"):
            continue
        if t.endswith(":"):
            continue
        if any(t.startswith(d) for d in SKIP_DIR) or t.startswith("."):
            continue
        out.append((re.sub(r"\s+", " ", t.replace("\t", " ")), n + 1))
    return out


# --------------------------------------------------------------------------
def key(text):
    """Alignment key: mnemonic + operands, whitespace- and comment-normalized."""
    t = text.split("#")[0].strip()
    t = re.sub(r"\s+", " ", t)
    return t


def skel(t):
    """Operand skeleton with register names blanked -- for renamed moves."""
    return re.sub(r"(?<![\w$.])(zero|at|v[01]|a[0-3]|t\d|s[0-7]|k[01]|gp|sp|fp|ra"
                  r"|\$\d+)(?![\w])", "#", key(t))


def align(a, b, label="", verbose=False):
    """Alignment of two instruction-text lists -> amap[i] = j or None.

    difflib alone is NOT sufficient here.  SequenceMatcher only ever produces
    MONOTONE alignments, so an instruction that MOVED is reported as a delete
    at one place plus an insert at another -- and a monotone map is exactly what
    a scheduling difference is not.  A second pass therefore pairs the leftover
    deletes against the leftover inserts (exact text first, then same operand
    skeleton), and those pairs are the reordering signal this whole tool exists
    to find."""
    ka, kb = [key(x) for x in a], [key(x) for x in b]
    sm = difflib.SequenceMatcher(a=ka, b=kb, autojunk=False)
    amap = [None] * len(a)
    dels, inss = [], []
    stats = {"equal": 0, "replace": 0, "delete": 0, "insert": 0, "moved": 0}
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            stats["equal"] += i2 - i1
            for k in range(i2 - i1):
                amap[i1 + k] = j1 + k
        elif tag == "replace":
            n = min(i2 - i1, j2 - j1)
            stats["replace"] += n
            for k in range(n):
                amap[i1 + k] = j1 + k
            dels += list(range(i1 + n, i2))
            inss += list(range(j1 + n, j2))
        elif tag == "delete":
            dels += list(range(i1, i2))
        else:
            inss += list(range(j1, j2))

    # Pair the leftover deletes against the leftover inserts.  These pairs are
    # the reordering signal, and getting them wrong is worse than not pairing
    # at all: a mis-pair produces a goal order that is not a topological order
    # of the block's dependences, i.e. a schedule the compiler could not have
    # produced.
    #
    # The original pass walked `dels` in ascending order and gave each one its
    # nearest free insert.  That is greedy in the WRONG order: with duplicate
    # instruction text (the same `li $2,252` / `sb $2,12($17)` appearing in
    # several blocks) an early delete takes a far-away slot that a later delete
    # needed, and the later one is then left unmapped.  MEASURED on
    # func_80072CD4: h36 `sb $2,12($17)` was paired to t55, 19 positions away,
    # while h39 `li $2,252` got NO target slot — and the resulting goals for
    # blocks 4 and 5 failed the topological check with 5 and 3 violations.
    #
    # Two changes, both conservative:
    #   * assign GLOBALLY in order of increasing cost rather than per-delete —
    #     exact-text matches before skeleton matches, then nearest first, so the
    #     cheapest pairs are made before any far-fetched one is considered;
    #   * bound the distance (MAX_MOVE).  A scheduling move is a within-block
    #     displacement; a pairing that spans far more than a block is duplicate
    #     text, not a move.  Beyond the bound, leave the instruction unmapped
    #     and let the caller's interpolation state the weaker, honest claim
    #     ("unchanged relative to its neighbours") instead of a fiction.
    free = set(inss)
    cands = []
    for i in dels:
        for j in free:
            if abs(i - j) > MAX_MOVE:
                continue
            if ka[i] == kb[j]:
                cands.append((0, abs(i - j), i, j))
            elif skel(a[i]) == skel(b[j]):
                cands.append((1, abs(i - j), i, j))
    taken_i = set()
    for _tier, _d, i, j in sorted(cands):
        if i in taken_i or j not in free:
            continue
        amap[i] = j
        free.discard(j)
        taken_i.add(i)
        stats["moved"] += 1
    stats["delete"] = len(dels) - stats["moved"]
    stats["insert"] = len(free)
    if verbose:
        print(f"  align {label}: |A|={len(a)} |B|={len(b)} {stats}")
    return amap, sm.get_opcodes()


# --------------------------------------------------------------------------
_MACRO_MEM_OPS = {"lw", "sw", "lh", "lhu", "sh", "lb", "lbu", "sb",
                  "lwl", "lwr", "swl", "swr", "ulw", "usw"}


def _macro_expand_counts(lines):
    """Per honest-text line, how many object insns GNU as assembles it to.

    The maspsx output still contains assembler macros; under this project's
    flags (-G0, explicit sdata lists) the two forms that reach the honest
    stream both expand via $at/lui to exactly 2 insns:
      * `la  $r,SYM[+off]`      -> lui + addiu
      * `<mem> $r,SYM[+off]`    -> lui + <mem>   (bare symbol, no `(base)`)
    Everything else (including `<mem> $r,off($base)` and la of a small
    constant already spelled addiu by cc1) is 1:1.  A gp-relative symbol
    would assemble to ONE insn and break this estimate -- the caller's
    checksum against the real object count catches that honestly."""
    counts = []
    for t in lines:
        parts = t.split(None, 1)
        op = parts[0] if parts else ""
        rest = parts[1] if len(parts) > 1 else ""
        last = rest.split(",")[-1].split("#")[0].strip()
        symbolic = ("(" not in last
                    and (last[:1].isalpha() or last[:1] in "._"))
        if (op == "la" or op in _MACRO_MEM_OPS) and symbolic:
            counts.append(2)
        else:
            counts.append(1)
    return counts


def build_map(root: Path, stem: str, func: str, verbose=False, target=None,
              target_object=None, ours_object=None):
    """-> dict with uid->target-position and the intermediate streams.

    TARGET pins the target stream to a specific file.  This matters as soon as
    you iterate: regfix rules are indexed to HEAD's instruction positions, so
    regenerating `<stem>.tgt.s` from an EDITED source produces fiction.  Capture
    it once from clean HEAD (`<stem>.tgt.head.s`) and pass it on every later
    round -- target is the original binary and does not change with our edits.

    TARGET_OBJECT (owner ruling 2026-08-25, func_800645B0 packet): for an
    INCLUDE_ASM-routed function src carries no C body, so no src-derived
    `<stem>.tgt.s` can contain the target stream.  Pass the canonical build
    object (build/src/<stem>.o -- its block IS the split asm file's bytes) as
    TARGET_OBJECT and the cheat-stripped sandbox object
    (tmp/sandbox/<func>/<stem>.o) as OURS_OBJECT.  Both sides are then objdump
    renderings in the SAME language, the hop-3 alignment runs object-vs-object,
    and the hon text stream maps to its object 1:1 by position (enforced)."""
    S = root / "tmp" / "sched_map"
    cc1 = [t for t, _ in asm_body(S / f"{stem}.cc1.s", func)]
    hon = [t for t, _ in asm_body(S / f"{stem}.hon.s", func)]
    if target_object:
        if not ours_object:
            raise RuntimeError("target_object requires ours_object (the "
                               "cheat-stripped sandbox .o for the same source "
                               "state as <stem>.hon.s)")
        import sys as _sys
        if str(root) not in _sys.path:
            _sys.path.insert(0, str(root))
        from engine import score as _score
        tgt = _score.normalized_insns(str(target_object), func, mask=True)
        hon_obj = _score.normalized_insns(str(ours_object), func, mask=True)
        # The honest TEXT stream still carries assembler macros (`la`,
        # bare-symbol memory ops) that GNU as expands to lui+op pairs, so a
        # macro-bearing function's text is SHORTER than its object and a bare
        # 1:1 length check falsely reports a different source state (first hit:
        # CD_sync 2026-08-30, 140 text lines vs 160 object insns).  Model the
        # expansion per line; the summed count is the same-source check.
        t2o_counts = _macro_expand_counts(hon)
        if sum(t2o_counts) != len(hon_obj):
            raise RuntimeError(
                f"{func}: honest object has {len(hon_obj)} insns but "
                f"{stem}.hon.s body has {len(hon)} lines "
                f"(macro-expanded estimate {sum(t2o_counts)}) -- either the "
                f"sandbox object was built from a DIFFERENT source state than "
                f"the sched_map streams (re-run mkasm.sh and the sandbox on "
                f"the same tree), or a macro form is missing from "
                f"_macro_expand_counts (e.g. a gp-relative symbol assembling "
                f"to ONE insn).")
        hon_t2o, _pos = [], 0
        for _n in t2o_counts:
            hon_t2o.append(_pos)
            _pos += _n
        if verbose:
            print(f"  target from object {target_object} "
                  f"(ours: {ours_object})")
    else:
        tpath = Path(target) if target else S / f"{stem}.tgt.s"
        tgt = [t for t, _ in asm_body(tpath, func)]
        if verbose and target:
            print(f"  target pinned to {tpath}")
    uids, seqs = dbr_uids(root, stem, func)

    if verbose:
        print(f"{func}: cc1={len(cc1)} hon={len(hon)} tgt={len(tgt)} "
              f"dbr_uids={len(uids)}")

    # hop 1: cc1 index -> uid (prefix-aligned; epilogue is a UID-less suffix)
    if len(uids) > len(cc1):
        raise RuntimeError(f"{func}: more RTL insns ({len(uids)}) than cc1 asm "
                           f"lines ({len(cc1)}) -- alignment assumption broken")
    cc1_uid = {i: uids[i] for i in range(len(uids))}
    tail = cc1[len(uids):]
    if verbose and tail:
        print(f"  epilogue tail ({len(tail)}): {tail}")

    # hop 2: cc1 -> honest
    c2h, _ = align(cc1, hon, "cc1->hon", verbose)
    # hop 3: honest -> target (object mode aligns the objdump renderings, which
    # share a language; hon text index -> object index via hon_t2o, the
    # macro-expansion prefix map checksummed against the real object above)
    if target_object:
        h2t, ops = align(hon_obj, tgt, "honobj->tgtobj", verbose)
    else:
        h2t, ops = align(hon, tgt, "hon->tgt", verbose)

    # cc1 index -> target position, with interpolation for unmapped slots.
    # An instruction that difflib could not pair (a MISSING/EXTRA cluster, e.g.
    # a maspsx nop or a regfix insertion) still needs an order slot, or it drops
    # out of the goal entirely.  Interpolating between its nearest mapped
    # neighbours states the weakest claim that keeps it in place: "unchanged
    # relative to the instructions around it".
    tpos_raw = {}
    for i in range(len(cc1)):
        j = c2h[i]
        if target_object and j is not None:
            j = hon_t2o[j]        # text index -> first expanded object index
        k = h2t[j] if j is not None else None
        tpos_raw[i] = float(k) if k is not None else None
    mapped = sorted(i for i, v in tpos_raw.items() if v is not None)
    tpos, interp = {}, set()
    for i in range(len(cc1)):
        if tpos_raw[i] is not None:
            tpos[i] = tpos_raw[i]
            continue
        lo = max((x for x in mapped if x < i), default=None)
        hi = min((x for x in mapped if x > i), default=None)
        if lo is None and hi is None:
            tpos[i] = float(i)
        elif lo is None:
            tpos[i] = tpos_raw[hi] - (hi - i) * 1e-3
        elif hi is None:
            tpos[i] = tpos_raw[lo] + (i - lo) * 1e-3
        else:
            span = tpos_raw[hi] - tpos_raw[lo]
            tpos[i] = tpos_raw[lo] + span * (i - lo) / (hi - lo)
        interp.add(i)

    if verbose and seqs:
        print(f"  filled delay slots: "
              + ", ".join(f"{cc1_uid.get(b,'?')}/{cc1_uid.get(s,'?')}"
                          for b, s in seqs))

    uid_idx = {}
    for i, u in cc1_uid.items():
        uid_idx.setdefault(u, []).append(i)
    return {"cc1": cc1, "hon": hon, "tgt": tgt, "uids": uids,
            "cc1_uid": cc1_uid, "uid_idx": uid_idx, "tpos": tpos,
            "interp": interp, "hon_tgt_ops": ops, "c2h": c2h, "h2t": h2t}


# --------------------------------------------------------------------------
def goal_for_block(m, block_uids, ours=None, blk=None):
    """Order BLOCK_UIDS as sched.c's PICK order for target's emission order.

    schedule_block builds each block backwards and PREPENDS every pick, so the
    pick sequence recorded in the dump is the REVERSE of the emitted order.
    The goal therefore reverses target's emission order too.

    reorg duplicates delay-slot insns, so a UID can occupy two cc1 slots; the
    occurrence inside this block's own index range is the right one."""
    if blk is not None:
        m = dict(m, _deps=blk["deps"])
    idxs = {}
    unresolved = []
    singles = [m["uid_idx"][u][0] for u in block_uids
               if len(m["uid_idx"].get(u, [])) == 1]
    lo, hi = (min(singles), max(singles)) if singles else (0, len(m["cc1"]))
    for u in block_uids:
        cand = m["uid_idx"].get(u, [])
        if not cand:
            unresolved.append(u)          # USE/CLOBBER: scheduled, never emitted
            continue
        inside = [c for c in cand if lo - 2 <= c <= hi + 2]
        idxs[u] = inside[0] if inside else cand[0]
    interp = [u for u, i in idxs.items() if i in m["interp"]]

    # Both sides are compared in POST-reorg space, so reorg.c's delay-slot
    # displacement -- which happens after sched2 and is not modelled -- cancels
    # instead of having to be guessed at.  The result is then carried back into
    # the model's PRE-reorg space through our own known reorg permutation:
    #
    #   P  our pre-reorg  order = the pick sequence, reversed
    #   O  our post-reorg order = the .dbr order
    #   T  target's post-reorg order = O's UIDs sorted by target position
    #
    # sigma(i) = position of P[i] in O, so goal_pre[i] = T[sigma(i)]: whatever
    # reorg did to the insn our schedule put at slot i, it does to the insn
    # target's schedule has in the corresponding slot.
    O = sorted(idxs, key=lambda u: idxs[u])
    T = sorted(idxs, key=lambda u: (m["tpos"][idxs[u]], idxs[u]))
    P = [u for u in reversed(ours or O) if u in idxs]
    opos = {u: i for i, u in enumerate(O)}
    goal_pre = [T[opos[u]] for u in P]

    # The sigma composition assumes reorg applied the SAME positional
    # permutation to both sides.  That holds when both filled the same delay
    # slot from the same place, and breaks when they did not -- e.g. our reorg
    # pulls an insn several positions forward into a slot and target's leaves it
    # alone.  The composition then scrambles the goal into something that is not
    # even a topological order.  Detect that and fall back to target's own
    # post-reorg order, which needs no assumption about reorg at all; it is only
    # a slightly weaker claim (it keeps target's delay-slot placement).
    if _violates(m, idxs, goal_pre):
        direct = list(T)
        if not _violates(m, idxs, direct):
            goal_pre = direct
    order = list(reversed(goal_pre))

    # A UID with no emitted instruction (USE / CLOBBER) still occupies a slot in
    # the pick sequence.  It has no target evidence, so hold it where OUR
    # schedule has it: immediately before the same successor.
    if unresolved and ours:
        for u in unresolved:
            i = ours.index(u)
            nxt = next((v for v in ours[i + 1:] if v in order), None)
            order.insert(order.index(nxt) if nxt in order else len(order), u)
    return order, unresolved, interp


def _violates(m, idxs, emit):
    """Does EMIT (emission order) break a LOG_LINKS dependence? Needs the deps,
    which goal_for_block does not carry, so this uses the block passed via
    m['_deps'] when available and otherwise reports False (the caller's
    topo_violations check still gates the result)."""
    deps = m.get("_deps")
    if not deps:
        return False
    pos = {u: i for i, u in enumerate(emit)}
    for k, preds in deps.items():
        u = int(k)
        if u not in pos:
            continue
        for pred, _kind in preds:
            if pred in pos and pos[pred] > pos[u]:
                return True
    return False


def topo_violations(blk, goal):
    """A goal order must be a topological order of the block's LOG_LINKS.

    Every insn depends on its predecessors, so in EMISSION order a predecessor
    must precede its consumer.  A goal that violates this is not a schedule the
    compiler could ever produce -- in practice it means the target alignment
    mis-paired two identical instruction texts (the same `la SYM` / `addu r,1100`
    cluster appearing in two blocks is the usual culprit).  Reporting it beats
    handing an impossible goal to the search."""
    emit = list(reversed(goal))
    pos = {u: i for i, u in enumerate(emit)}
    bad = []
    for k, preds in blk["deps"].items():
        u = int(k)
        if u not in pos:
            continue
        for pred, kind in preds:
            if pred in pos and pos[pred] > pos[u]:
                bad.append((pred, u, kind))
    return bad


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("root")
    ap.add_argument("stem")
    ap.add_argument("func")
    ap.add_argument("--model")
    ap.add_argument("--pass", dest="passno", type=int, default=2)
    ap.add_argument("--show", action="store_true")
    ap.add_argument("--target", help="pin the target stream to this .s file "
                                     "(REQUIRED once the source is edited)")
    a = ap.parse_args()
    root = Path(a.root)
    m = build_map(root, a.stem, a.func, verbose=True, target=a.target)

    if a.show:
        print("\n idx  uid | cc1                              | honest -> target")
        for i, t in enumerate(m["cc1"]):
            u = m["cc1_uid"].get(i, "")
            j = m["c2h"][i]
            k = m["h2t"][j] if j is not None else None
            print(f"{i:4d} {str(u):>4} | {t:40s} | h{j} t{k} "
                  f"{m['tgt'][k] if k is not None else '<<< NO TARGET SLOT'}")

    if a.model:
        model = json.loads(Path(a.model).read_text())
        for f in model["funcs"]:
            if f["func"] != a.func or f["pass"] != a.passno:
                continue
            for blk in f["blocks"]:
                bu = [int(k) for k, n in blk["nodes"].items()
                      if not n.get("extern")]
                ours = [p["insn"] for p in blk["picks"]]
                goal, un, interp = goal_for_block(m, bu, ours, blk)
                same = goal == ours
                ndiff = sum(1 for x, y in zip(goal, ours) if x != y)
                print(f"\nblock {blk['b']}: {blk['n_insns']} insns "
                      f"{'GOAL == OURS (identity)' if same else f'GOAL DIFFERS ({ndiff} slots)'}"
                      + (f"  [unresolved: {un}]" if un else "")
                      + (f"  [interpolated: {interp}]" if interp else ""))
                if not same:
                    bad = topo_violations(blk, goal)
                    if bad:
                        print(f"  *** GOAL INVALID: {len(bad)} dependence "
                              f"violation(s) {bad[:4]} -- the target alignment "
                              f"mis-paired duplicate instruction text here; "
                              f"do not search this block")
                    print(f"  ours: {ours}")
                    print(f"  goal: {goal}")
                    print("  --goal-order " + ",".join(str(x) for x in goal))
                    print("  emission order (reverse of the pick order):")
                    op, gp = list(reversed(ours)), list(reversed(goal))
                    for n, (o, g) in enumerate(zip(op, gp)):
                        i = m["uid_idx"].get(o, [None])[0]
                        j = m["uid_idx"].get(g, [None])[0]
                        ot = m["cc1"][i] if i is not None else "(no asm)"
                        gt = m["cc1"][j] if j is not None else "(no asm)"
                        mark = "   " if o == g else " * "
                        print(f"   {n:3d}{mark}ours {o:>4} {ot:34s} "
                              f"target {g:>4} {gt}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
