#!/usr/bin/env python3
"""ra_solver.extract — pull global-alloc inputs for one function from cc1 dumps.

Runs the real pipeline front (cpp | cc1 -da) on the function's TU, then parses:
  <i>.greg  — allocation order, allocno conflicts, hard-reg conflicts,
              preferences, final register dispositions (ground truth)
  <i>.flow  — per-register "used N times across M insns; crosses K calls"
  stderr    — ALLOCDBG lines (nrefs/livelen/pri per allocno, sort order)
  <i>.greg RTL text — (reg:MODE NN) pseudo modes

Usage (WSL, repo root, venv active):
  python3 tools/ra_solver/extract.py <func> <stem> [--out model.json] [--inspect]

The TU is compiled in-context (the real source file), matching the true build.
Per-function dump segments are index-matched against the .s `.ent` order.
Read-only with respect to the tree; work files under tmp/ra_solver_work/.
"""
import argparse, json, os, re, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
WORK = ROOT / "tmp" / "ra_solver_work"
CPP = ("mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin "
       "-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ "
       "-D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C")
CC1 = ROOT / "tools" / "gcc-2.7.2" / "cc1"          # instrumented (BB2_* hooks)
CC1_REF = ROOT / "tools" / "gcc-2.7.2" / "build" / "cc1"  # build compiler (parity check)
CC1_FLAGS = ("-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 "
             "-mno-abicalls -fno-builtin -w -mel")


def run_dumps(stem: str) -> dict:
    WORK.mkdir(parents=True, exist_ok=True)
    src = ROOT / "src" / f"{stem}.c"
    ifile = WORK / f"{stem}.i"
    subprocess.run(f'{CPP} "{src}" > "{ifile}"', shell=True, cwd=ROOT, check=True,
                   stderr=subprocess.DEVNULL)
    env = dict(os.environ, BB2_ALLOC_DEBUG="1")
    # cc1 writes dump files next to the input file
    r = subprocess.run(
        f'"{CC1}" {CC1_FLAGS} -da "{ifile}" -o "{WORK / (stem + ".s")}"',
        shell=True, cwd=ROOT, env=env, capture_output=True, text=True)
    out = {"stderr": r.stderr}
    for ext in ("greg", "flow", "lreg"):
        p = Path(str(ifile) + "." + ext)
        out[ext] = p.read_text(errors="replace") if p.exists() else ""
    out["asm"] = (WORK / (stem + ".s")).read_text(errors="replace")
    return out


def ent_order(asm: str) -> list:
    return re.findall(r"\.ent\s+(\w+)", asm)


def split_greg(greg: str) -> list:
    """Split the greg dump into per-function segments on the dump_conflicts
    header ';; N regs to allocate:'. Segment k belongs to the k-th function."""
    idx = [m.start() for m in re.finditer(r"^;; \d+ regs to allocate:", greg, re.M)]
    segs = []
    for k, s in enumerate(idx):
        e = idx[k + 1] if k + 1 < len(idx) else len(greg)
        segs.append(greg[s:e])
    return segs


def split_allocdbg(stderr: str) -> dict:
    """func-name-keyed ALLOCDBG rows + seed_used sets (tagged hook)."""
    rows, seeds = {}, {}
    for line in stderr.splitlines():
        m = re.match(r"ALLOCDBG func=(\S+) seed_used=([\d,]*)", line)
        if m:
            seeds[m.group(1)] = [int(t) for t in m.group(2).split(",") if t]
            continue
        m = re.match(r"ALLOCDBG func=(\S+) ord=(\d+) pseudo=(\d+) "
                     r"hardreg=(-?\d+) nrefs=(-?\d+) livelen=(-?\d+) "
                     r"pri=(-?\d+)", line)
        if m:
            rows.setdefault(m.group(1), []).append(
                {"ord": int(m.group(2)), "pseudo": int(m.group(3)),
                 "hardreg": int(m.group(4)), "nrefs": int(m.group(5)),
                 "livelen": int(m.group(6)), "pri": int(m.group(7))})
    return {"rows": rows, "seeds": seeds}


def split_flow(flow: str) -> list:
    """Per-function flow segments. dump_flow_info output begins with the
    basic-block census; 'Register N used...' lines follow. Functions are
    separated by the ';; Function' banner if present, else by the block
    starting at a line matching '^\\d+ registers.'"""
    banners = [m.start() for m in re.finditer(r"^;; Function", flow, re.M)]
    if banners:
        segs = []
        for k, s in enumerate(banners):
            e = banners[k + 1] if k + 1 < len(banners) else len(flow)
            segs.append(flow[s:e])
        return segs
    idx = [m.start() for m in re.finditer(r"^\d+ registers\.", flow, re.M)]
    segs = []
    for k, s in enumerate(idx):
        e = idx[k + 1] if k + 1 < len(idx) else len(flow)
        segs.append(flow[s:e])
    return segs


def parse_flow_regs(seg: str) -> dict:
    regs = {}
    for m in re.finditer(
            r"Register (\d+) used (-?\d+) times across (-?\d+) insns"
            r"(?:; set (\d+) times?)?(?:.*?crosses (\d+) calls?)?[^\n]*", seg):
        regs[int(m.group(1))] = {
            "nrefs_flow": int(m.group(2)),
            "livelen_flow": int(m.group(3)),
            "calls_crossed": int(m.group(5) or 0)}
    return regs


def _md_kind(rhs: str):
    """Which multiply/divide half a def's RHS produces, or None if the RHS is
    not an MD result at all.  See the call site for the three shapes."""
    head = re.match(r"\s*(\w+):", rhs)
    if not head:
        return None
    op = head.group(1)
    if op in ("mult", "div", "udiv", "umult"):
        return "lo"
    if op in ("mod", "umod"):
        return "hi"
    if op == "truncate" and "lshiftrt:DI" in rhs and "mult:DI" in rhs:
        return "hi"
    return None


def parse_greg_segment(seg: str) -> dict:
    d = {"order": [], "sizes": {}, "conflicts": {}, "hard_conflicts": {},
         "prefs": {}, "dispositions": {}, "modes": {}}
    m = re.search(r";; \d+ regs to allocate:((?:.|\n)*?)\n(?=;;|\n)", seg)
    if m:
        toks = m.group(1).replace("\n;;", " ").split()
        for t in toks:
            mm = re.match(r"(\d+)(?:\+(\d+))?(?:\((\d+)\))?$", t)
            if mm:
                p = int(mm.group(1))
                d["order"].append(p)
                if mm.group(3):
                    d["sizes"][p] = int(mm.group(3))
    for m in re.finditer(r"^;; (\d+) conflicts:([^\n]*)$", seg, re.M):
        p = int(m.group(1))
        toks = m.group(2).split()
        # pseudo conflicts are >= FIRST_PSEUDO (32 on mips); hard reg
        # conflicts are appended as small ints
        d["conflicts"][p] = [int(t) for t in toks if int(t) >= 68]
        d["hard_conflicts"][p] = [int(t) for t in toks if int(t) < 68]
    for m in re.finditer(r"^;; (\d+) preferences:([^\n]*)$", seg, re.M):
        d["prefs"][int(m.group(1))] = [int(t) for t in m.group(2).split()]
    m = re.search(r";; Register dispositions:\n((?:[\d ]+in[^\n]*\n)+)", seg)
    if m:
        for mm in re.finditer(r"(\d+) in (-?\d+)", m.group(1)):
            d["dispositions"][int(mm.group(1))] = int(mm.group(2))
    for mm in re.finditer(r"\(reg:(\w+) (\d+)\)", seg):
        d["modes"].setdefault(int(mm.group(2)), mm.group(1))
    # pseudos that appear ONLY inside bare (use ...) get NO_REGS from
    # regclass and are never allocated (the orphan-USE class)
    d["use_only"] = []
    d["md_class"] = []
    for pp in set(d["order"]):
        occ = [l for l in seg.splitlines() if re.search(rf"\(reg:\w+ {pp}\)", l)]
        if occ and all("(use " in l for l in occ):
            d["use_only"].append(pp)
        if any(re.search(rf"\(set \(reg:\w+ {pp}\)" + r"[^\n]*\((?:mult|div|mod|umult|udiv|umod):", l)
               for l in occ):
            d["md_class"].append(pp)
    return d


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("func")
    ap.add_argument("stem")
    ap.add_argument("--out")
    ap.add_argument("--inspect", action="store_true")
    a = ap.parse_args()

    dumps = run_dumps(a.stem)
    ents = ent_order(dumps["asm"])
    if a.func not in ents:
        sys.exit(f"FATAL: {a.func} not in .ent list ({len(ents)} functions)")
    k = ents.index(a.func)

    gsegs = split_greg(dumps["greg"])
    lsegs = {}
    parts = re.split(r"^;; Function (\w+)$", dumps["lreg"], flags=re.M)
    for i in range(1, len(parts) - 1, 2):
        lsegs[parts[i]] = parts[i + 1]
    ablocks = split_allocdbg(dumps["stderr"])
    fsegs = split_flow(dumps["flow"])

    # greg segments exist only for functions that ran global-alloc with
    # allocnos, so they must be aligned to the `.ent` list before a segment can
    # be attributed to a function.
    #
    # PRIMARY (2026-08-06): index by the func-tagged ALLOCDBG stream.  The
    # BB2_ALLOC_DEBUG hook prints `ALLOCDBG func=NAME ord=... pseudo=...` from
    # inside global_alloc, so the functions carrying ALLOCDBG rows ARE exactly
    # the functions that emitted a greg segment, in the same order — and each
    # one's allocno set is known by NAME rather than inferred.  Alignment
    # becomes a zip plus a verification, with no signature guessing.
    #
    # FALLBACK: the original subset heuristic, kept for dumps produced without
    # the hook (a stock cc1, or BB2_ALLOC_DEBUG unset).  It matches a segment
    # to an ent when the segment's pseudos are a SUBSET of the ent's flow
    # pseudo set, greedily with a feasibility lookahead.  Subset is not an
    # identity: a small segment is a subset of many functions' flow sets, so
    # when sibling signatures nest, the greedy walk consumes a segment at the
    # wrong ent and a later function is left unaligned.
    #
    # MEASURED (code6cac_c, 12 ents / 8 segments): the fallback assigned
    # segments to ents 0,3,4,5,8,9,10,11 — including pad_press_control (ent 0)
    # and func_8003791C (ent 4), which have NO allocnos at all — and skipped
    # func_80037804 (ent 2) and func_80037A20 (ent 7), so extracting
    # func_80037A20 died with "FATAL: no greg segment aligned".  The ALLOCDBG
    # index maps all 8 segments to ents 2,3,5,7,8,9,10,11 with every segment's
    # allocno set EQUAL to its function's ALLOCDBG pseudo set.
    flow_sets = [set(parse_flow_regs(s)) for s in fsegs]
    seg_pseudos = []
    seg_order_sets = []
    for g in gsegs:
        sp = parse_greg_segment(g)
        seg_pseudos.append(set(sp["order"]) | set(sp["dispositions"]))
        seg_order_sets.append(set(sp["order"]))

    def allocdbg_alignment():
        """-> {ent index: segment index} or None when the stream can't index."""
        rows = ablocks["rows"]
        alloc_ents = [ei for ei, e in enumerate(ents) if rows.get(e)]
        if not alloc_ents or len(alloc_ents) != len(gsegs):
            return None
        # Verify rather than trust: every pairing must agree on the allocno set.
        for gi, ei in enumerate(alloc_ents):
            if seg_order_sets[gi] != {r["pseudo"] for r in rows[ents[ei]]}:
                return None
        return {ei: gi for gi, ei in enumerate(alloc_ents)}

    def fits(gi, ei):
        return bool(seg_pseudos[gi]) and seg_pseudos[gi] <= flow_sets[ei]

    # DP feasibility: align(gi, ei) possible iff all segments gi.. fit into
    # ents ei.. in order.
    import functools
    @functools.lru_cache(maxsize=None)
    def feasible(gi, ei):
        if gi == len(gsegs):
            return True
        if ei == len(ents):
            return False
        if fits(gi, ei) and feasible(gi + 1, ei + 1):
            return True
        return feasible(gi, ei + 1)

    greg_for_ent = allocdbg_alignment()
    align_via = "ALLOCDBG"
    if greg_for_ent is None:
        align_via = "flow-subset fallback"
        greg_for_ent = {}
        gi = 0
        for ei in range(len(ents)):
            if gi < len(gsegs) and fits(gi, ei) and feasible(gi + 1, ei + 1):
                # extra disambiguation: only consume if skipping would break
                # feasibility OR the NEXT segment cannot also fit this ent
                # better; prefer consuming (functions rarely skip global alloc).
                greg_for_ent[ei] = gi
                gi += 1
        if gi != len(gsegs):
            sys.exit(f"FATAL: greg alignment consumed {gi}/{len(gsegs)} "
                     f"segments — signature mismatch; run --inspect")
    if a.inspect or os.environ.get("RA_DEBUG"):
        print(f"  align via: {align_via}", file=sys.stderr)
        for ei, gg in sorted(greg_for_ent.items()):
            print(f"  align: ent[{ei}] {ents[ei]} <- greg seg {gg} "
                  f"({sorted(seg_pseudos[gg])[:6]}...)", file=sys.stderr)

    if a.inspect:
        # NB: ablocks is {"rows": {func: [...]}, "seeds": {...}} — printing
        # len(ablocks) reported the DICT's 2 keys, not a block count, which
        # made every TU look like it had "2 allocdbg blocks".
        print(f"functions: {len(ents)}  greg segs: {len(gsegs)}  "
              f"allocdbg funcs: {len(ablocks['rows'])}  flow segs: {len(fsegs)}")
        print(f"target index: {k} ({a.func})  aligned greg seg: "
              f"{greg_for_ent.get(k, 'NONE')}  (via {align_via})")
        if k in greg_for_ent:
            print("--- greg segment head ---")
            print("\n".join(gsegs[greg_for_ent[k]].splitlines()[:15]))
        if ablocks["rows"].get(a.func):
            print("--- allocdbg ---")
            for r in ablocks["rows"][a.func][:12]:
                print(r)
        if k < len(fsegs):
            print("--- flow regs (first 8) ---")
            fr = parse_flow_regs(fsegs[k])
            for p in list(fr)[:8]:
                print(p, fr[p])
        return

    if k not in greg_for_ent:
        sys.exit(f"FATAL: no greg segment aligned to {a.func} — no global "
                 f"allocnos, or alignment failure; run --inspect")

    model = parse_greg_segment(gsegs[greg_for_ent[k]])
    lseg = lsegs.get(a.func, "")
    model["modes"] = {}
    model["use_only"] = []
    model["md_class"] = []
    for pp in set(model["order"]):
        occ = [l for l in lseg.splitlines()
               if re.search(rf"\(reg[/\w]*:\w+ {pp}\)", l)]
        for l in occ:
            mm = re.search(rf"\(reg[/\w]*:(\w+) {pp}\)", l)
            if mm:
                model["modes"].setdefault(pp, mm.group(1))
        real = [l for l in occ
                if "(use " not in l and "expr_list" not in l
                and "insn_list" not in l and "REG_DEAD" not in l
                and "REG_NO_CONFLICT" not in l]
        if not real:
            # zero real-operand appearances (fully folded away, or only
            # inside bare (use)) -> regclass leaves it NO_REGS -> never
            # allocated by find_reg
            model["use_only"].append(pp)
        # MD class: regclass sends a pseudo to $lo/$hi when EVERY def is a
        # multiply/divide result.  The RHS shape decides which half:
        #   mult / div / udiv          -> $lo (65)
        #   mod / umod                 -> $hi (64)
        #   truncate(lshiftrt(mult:DI  -> $hi (64)   the `mulhi` idiom GCC
        #                     ...,32))              emits for division by a
        #                                           constant
        # The third form is why saTan4FireDisp's pseudo 99 (the `/255` colour
        # conversion) was mis-typed as GR_REGS before 2026-08-06: the old
        # regex only looked at the OUTERMOST operator and saw `truncate`.
        defs = re.findall(
            rf"\(set \(reg[/\w]*:\w+ {pp}\)\s*\((.*?)(?=\n\s*\(set |\Z)",
            lseg, re.S)
        kinds = [_md_kind(d) for d in defs]
        if kinds and all(kinds):
            model["md_class"].append(pp)
            model.setdefault("md_reg", {})[pp] = 64 if "hi" in kinds else 65
    model["allocdbg"] = ablocks["rows"].get(a.func, [])
    model["seed_used"] = ablocks["seeds"].get(a.func, [])
    model["flow"] = parse_flow_regs(fsegs[k]) if k < len(fsegs) else {}
    model["func"] = a.func
    model["stem"] = a.stem

    # Enrich with full/copy preference sets via the BB2_FINDREG_DEBUG hook
    # (one cc1 run per allocno; blocks are func-tagged so no ambiguity).
    model["full_prefs"] = {}
    model["copy_prefs"] = {}
    ifile = WORK / f"{a.stem}.i"
    for pp in model["order"]:
        env = dict(os.environ, BB2_FINDREG_DEBUG=str(pp))
        rr = subprocess.run(
            f'"{CC1}" {CC1_FLAGS} "{ifile}" -o /dev/null',
            shell=True, cwd=ROOT, env=env, capture_output=True, text=True)
        blk = re.search(
            rf"FINDREGDBG func={a.func} pseudo={pp} .*?"
            r"own_copy_prefs:([^\n]*)\n"
            r"FINDREGDBG  own_full_prefs:([^\n]*)\n",
            rr.stderr, re.S)
        if blk:
            model["copy_prefs"][pp] = [int(t) for t in blk.group(1).split()]
            model["full_prefs"][pp] = [int(t) for t in blk.group(2).split()]
    out = Path(a.out) if a.out else WORK / f"{a.func}.model.json"
    out.write_text(json.dumps(model, indent=1))
    print(f"model written: {out}  "
          f"(order={len(model['order'])} pseudos, "
          f"dispositions={len(model['dispositions'])})")


if __name__ == "__main__":
    main()
