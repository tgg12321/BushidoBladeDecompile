#!/usr/bin/env python3
"""ra_solver.simulate — faithful Python replication of GCC 2.7.2 global.c
allocation (allocno_compare sort + prune_preferences + find_reg) for MIPS.

Inputs: the model.json from extract.py. Outputs: simulated dispositions for
the global allocnos + a validation diff against the dump's ground truth.

Semantics sources (tools/gcc-2.7.2):
  global.c allocno_compare (pri desc, tie -> lower allocno = lower pseudo),
  global.c prune_preferences (reverse-order someone_prefers accumulation),
  global.c find_reg (used1 assembly, pass0/pass1, copy-pref then full-pref
  upgrade within compatible class),
  global.c post-assign conflict propagation (assigned reg IORed into the
  hard_reg_conflicts of every conflicting allocno),
  mips.h FIXED_REGISTERS / CALL_USED_REGISTERS / FIRST_PSEUDO_REGISTER=68,
  FRAME_POINTER_REGNUM=30 in no_global_alloc_regs.

Known simplifications (each can surface in validation and would then be
refined): single register class GR_REGS for integer pseudos; copy/full
preference sets approximated by the dump's single preferences line;
HARD_REGNO_MODE_OK true for all GPRs in HI/SI, even-reg-only for 2-word
modes.
"""
import json, math, sys
from pathlib import Path

FIRST_PSEUDO = 68
FIXED = {0, 1, 26, 27, 28, 29, 31, 67}
NO_GLOBAL = set()   # $30/fp is allocatable once the frame pointer is eliminated (-O2 norm)
CALL_USED = set(range(0, 16)) | {24, 25, 26, 27, 28, 29, 31} | \
    set(range(32, 52)) | {64, 65, 66, 67}
GR_REGS = set(range(0, 32))
TWO_WORD_MODES = {"DI", "DF", "CDI"}


def floor_log2(x):
    return x.bit_length() - 1 if x > 0 else 0


def pri(nrefs, livelen, size):
    if livelen <= 0:
        return -1_000_000
    return int((float(floor_log2(nrefs) * nrefs) / livelen) * 10000 * size)


def nregs(mode):
    return 2 if mode in TWO_WORD_MODES else 1


def mode_ok(regno, mode):
    if regno not in GR_REGS:
        return False
    if mode in TWO_WORD_MODES:
        return regno % 2 == 0
    return True


class Sim:
    trace = False

    def __init__(self, model):
        self.m = model
        self.order_dump = model["order"]
        flow = {int(k): v for k, v in model["flow"].items()}
        # ALLOCDBG (global-time) values override flow-time ones for the
        # priority arithmetic; the raw flow live length (reg_live_length) is
        # kept separately because global_alloc's >=0 allocability check
        # reads THAT array, not allocno_live_length.
        for f in flow.values():
            f["reg_live_length"] = f.get("livelen_flow", 1)
        for row in model.get("allocdbg", []):
            f = flow.setdefault(row["pseudo"], {})
            f.setdefault("reg_live_length", f.get("livelen_flow", 1))
            f["nrefs_flow"] = row["nrefs"]
            f["livelen_flow"] = row["livelen"]
        self.flow = flow
        self.modes = {int(k): v for k, v in model["modes"].items()}
        self.conf = {int(k): set(v) for k, v in model["conflicts"].items()}
        self.hard_conf = {int(k): set(v) for k, v in model["hard_conflicts"].items()}
        self.prefs = {int(k): set(v) for k, v in model.get("prefs", {}).items()}
        self.disp = {int(k): v for k, v in model["dispositions"].items()}
        self.pseudos = list(self.order_dump)
        self.use_only = set(model.get("use_only", []))
        self.md_class = set(model.get("md_class", []))
        for p in self.pseudos:
            self.conf.setdefault(p, set())
            self.hard_conf.setdefault(p, set())
            self.prefs.setdefault(p, set())

    def size(self, p):
        return nregs(self.modes.get(p, "SI"))

    def my_pri(self, p):
        f = self.flow.get(p, {})
        return pri(f.get("nrefs_flow", 1), f.get("livelen_flow", 1), self.size(p))

    def sort_order(self):
        return sorted(self.pseudos, key=lambda p: (-self.my_pri(p), p))

    def calls_crossed(self, p):
        return self.flow.get(p, {}).get("calls_crossed", 0)

    def simulate(self, order=None, overrides=None):
        """Run the allocation. `overrides` optionally patches per-pseudo
        attributes: {pseudo: {nrefs,livelen,calls_crossed,prefs,conf_add:[...]}}"""
        ov = overrides or {}

        def attr(p, key, default):
            if p in ov and key in ov[p]:
                return ov[p][key]
            f = self.flow.get(p, {})
            return {"nrefs": f.get("nrefs_flow", 1),
                    "livelen": f.get("livelen_flow", 1),
                    "calls": f.get("calls_crossed", 0)}.get(key, default)

        def p_pri(p):
            return pri(attr(p, "nrefs", 1), attr(p, "livelen", 1), self.size(p))

        full_prefs_in = {int(k): set(v)
                         for k, v in self.m.get("full_prefs", {}).items()}
        copy_prefs_in = {int(k): set(v)
                         for k, v in self.m.get("copy_prefs", {}).items()}
        conf = {p: set(self.conf[p]) for p in self.pseudos}
        for p in ov:
            for q in ov[p].get("conf_add", []):
                if p in conf:
                    conf[p].add(q)
                if q in conf:
                    conf[q].add(p)
        hard_conf = {p: set(self.hard_conf[p]) for p in self.pseudos}
        prefs = {p: set(ov.get(p, {}).get("prefs", self.prefs[p]))
                 for p in self.pseudos}

        if order is None:
            order = sorted(self.pseudos, key=lambda p: (-p_pri(p), p))

        # prune_preferences: least- to most-important; someone_prefers[a] =
        # union of full prefs of LOWER-priority conflicting allocnos (minus
        # own-preferred overlap unless smaller size).
        someone = {p: set() for p in self.pseudos}
        pruned_prefs = {}
        for i in range(len(order) - 1, -1, -1):
            a = order[i]
            temp_excl = set(hard_conf[a])
            if attr(a, "calls", 0) == 0:
                temp_excl |= FIXED
            else:
                temp_excl |= CALL_USED
            temp_excl |= (set(range(FIRST_PSEUDO)) - GR_REGS)
            pruned_prefs[a] = prefs[a] - temp_excl
            # prune_preferences prunes hard_reg_copy_preferences with the same
            # mask (global.c:897); keep them SEPARATE from hard_reg_preferences
            # because find_reg upgrades in two stages, copy prefs first.
            pruned_prefs.setdefault("_copy", {})
            pruned_prefs["_copy"][a] = copy_prefs_in.get(a, set()) - temp_excl
            # someone_prefers accumulates the FULL preference sets of
            # lower-priority conflicting allocnos (pruned the same way)
            pruned_full = full_prefs_in.get(a, prefs[a]) - temp_excl
            pruned_prefs.setdefault("_full", {})
            pruned_prefs["_full"][a] = pruned_full
            for j in range(i + 1, len(order)):
                b = order[j]
                if b in conf[a] or a in conf[b]:
                    t = set(pruned_prefs["_full"].get(b, set()))
                    if self.size(b) <= self.size(a):
                        t -= pruned_prefs["_full"][a]
                    someone[a] |= t

        assigned = {}
        # global.c seeds regs_used_so_far with call-used + regs_ever_live +
        # local-alloc renumberings; the tagged hook dumps the exact set.
        used_so_far = set(self.m.get("seed_used") or CALL_USED)
        for a in order:
            # global_alloc skips allocnos whose reg_live_length is negative;
            # use-only pseudos have class NO_REGS and can never be allocated.
            rll = self.flow.get(a, {}).get("reg_live_length", 1)
            if rll < 0 or attr(a, "livelen", 1) < 0 or a in self.use_only:
                continue
            if a in self.md_class:
                assigned[a] = 65      # LO_REGS: mult/div results land in $lo
                continue
            size = self.size(a)
            mode = self.modes.get(a, "SI")
            used1 = set()
            if attr(a, "calls", 0) == 0:
                used1 |= FIXED
            else:
                used1 |= CALL_USED
            used1 |= NO_GLOBAL
            used1 |= (set(range(FIRST_PSEUDO)) - GR_REGS)
            used1 |= hard_conf[a]

            def scan(used):
                for r in range(FIRST_PSEUDO):
                    if r in used or not mode_ok(r, mode):
                        continue
                    if all((r + j) not in used for j in range(1, size)):
                        return r
                return -1

            # pass 0: also exclude never-used and someone-else-preferred
            used0 = used1 | (set(range(FIRST_PSEUDO)) - used_so_far) | someone[a]
            best = scan(used0)
            if best < 0:
                best = scan(used1)

            # preference upgrade, in GCC's TWO stages (global.c find_reg,
            # "First do this for those register with copy preferences, then
            # all preferred registers"): scan hard_reg_copy_preferences
            # ascending and, on a hit, `goto no_prefs` — the plain-preference
            # stage is SKIPPED entirely. Only if no copy pref fits does
            # hard_reg_preferences get its turn.
            if best >= 0:
                def upgrade(cands):
                    for r in sorted(cands - used1):
                        if mode_ok(r, mode) and \
                           all((r + j) not in used1 for j in range(1, size)):
                            return r
                    return -1

                r = upgrade(pruned_prefs.get("_copy", {}).get(a, set()))
                if r < 0:
                    r = upgrade(pruned_prefs[a])
                if r >= 0:
                    best = r

            if self.trace:
                import sys as _s
                print(f"TRACE a={a} pri={p_pri(a)} calls={attr(a,'calls',0)} "
                      f"hard_conf={sorted(x for x in hard_conf[a] if x<32)} "
                      f"someone={sorted(someone[a])} best={best} "
                      f"prefs={sorted(pruned_prefs[a])}", file=_s.stderr)
            if best >= 0:
                assigned[a] = best
                for j in range(size):
                    used_so_far.add(best + j)
                for b in self.pseudos:
                    if b != a and (a in conf[b] or b in conf[a]):
                        for j in range(size):
                            hard_conf[b].add(best + j)
        return order, assigned


def main():
    model = json.loads(Path(sys.argv[1]).read_text())
    s = Sim(model)
    if "--trace" in sys.argv:
        s.trace = True

    order, assigned = s.simulate()
    dump_order = s.order_dump
    ok_order = (order == dump_order)
    print(f"sort order: {'MATCH' if ok_order else 'MISMATCH'}")
    if not ok_order:
        print(f"  sim : {order}")
        print(f"  dump: {dump_order}")

    mism = []
    for p in dump_order:
        want = s.disp.get(p)
        got = assigned.get(p)
        tag = "ok" if want == got else "XX"
        if want != got:
            mism.append(p)
        print(f"  {tag} pseudo {p}: sim={got} dump={want} "
              f"pri={s.my_pri(p)} calls={s.calls_crossed(p)} "
              f"mode={s.modes.get(p,'SI')}")
    print(f"dispositions: {len(dump_order)-len(mism)}/{len(dump_order)} match")
    sys.exit(0 if ok_order and not mism else 1)


if __name__ == "__main__":
    main()
