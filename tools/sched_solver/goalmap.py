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

    free = set(inss)
    for i in dels:
        cand = [j for j in free if ka[i] == kb[j]] or \
               [j for j in free if skel(a[i]) == skel(b[j])]
        if cand:
            j = min(cand, key=lambda x: abs(x - i))
            amap[i] = j
            free.discard(j)
            stats["moved"] += 1
    stats["delete"] = len(dels) - stats["moved"]
    stats["insert"] = len(free)
    if verbose:
        print(f"  align {label}: |A|={len(a)} |B|={len(b)} {stats}")
    return amap, sm.get_opcodes()


# --------------------------------------------------------------------------
def build_map(root: Path, stem: str, func: str, verbose=False):
    """-> dict with uid->target-position and the intermediate streams."""
    S = root / "tmp" / "sched_map"
    cc1 = [t for t, _ in asm_body(S / f"{stem}.cc1.s", func)]
    hon = [t for t, _ in asm_body(S / f"{stem}.hon.s", func)]
    tgt = [t for t, _ in asm_body(S / f"{stem}.tgt.s", func)]
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
    # hop 3: honest -> target
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
def goal_for_block(m, block_uids, ours=None):
    """Order BLOCK_UIDS as sched.c's PICK order for target's emission order.

    schedule_block builds each block backwards and PREPENDS every pick, so the
    pick sequence recorded in the dump is the REVERSE of the emitted order.
    The goal therefore reverses target's emission order too.

    reorg duplicates delay-slot insns, so a UID can occupy two cc1 slots; the
    occurrence inside this block's own index range is the right one."""
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
    a = ap.parse_args()
    root = Path(a.root)
    m = build_map(root, a.stem, a.func, verbose=True)

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
                goal, un, interp = goal_for_block(m, bu, ours)
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
