#!/usr/bin/env python3
"""sched_solver.simulate — Python replication of GCC 2.7.2 sched.c schedule_block.

Replays, per basic block, the backward list scheduler:

  * the ready list as a sliding window (`ready += 1` in C == popping the front)
  * SCHED_SORT: swap_sort when exactly one insn was added, qsort otherwise,
    NOTHING when nothing was added (the stale-order case that makes the
    scheduler order-dependent, not purely priority-driven)
  * rank_for_schedule: priority desc -> dependence class vs last_scheduled_insn
    (3 independent/latency-1 > 2 anti/output > 1 data) -> INSN_LUID desc
  * schedule_select: per priority-group, queue insns blocked by a function-unit
    hazard, then among the survivors pick the one with the largest potential
    hazard; move it to the front
  * schedule_insn: decrement each predecessor's ref count, and when it reaches
    zero either make it ready (cost <= 1) or queue it for `cost` cycles
  * the INSN_TICK / insn_queue[128] cycle machinery, and the stall search

Machine model (MIPS r3000, from config/mips/mips.md + insn-attrtab.c):
  unit -1 = none, 0 = memory (max_blockage 3), 1 = imuldiv (max_blockage 69),
  2/3/4 = FP adder/mult/divide (unused by this build).
  ADJUST_COST on MIPS zeroes the cost of any anti/output dependence, so only
  true (kind 0) data dependences can carry a latency above 1.

Two inputs are HARVESTED rather than derived, exactly as ra_solver harvests
`seed_used`: the per-insn INSN_PRIORITY after priority() (the critical-path
computation is an input to the pick order, not part of it) and the
adjust_priority birthing_insn_p flag, which depends on bb_live_regs liveness
the dump does not carry. Both are recorded per block by extract.py.

Usage:
  python3 tools/sched_solver/simulate.py <model.json> [--func NAME] [--pass N]
                                         [--trace] [--json out.json]
"""
import argparse, json, sys
from functools import cmp_to_key
from pathlib import Path

DONE_PRIORITY = -1
LAUNCH_PRIORITY = 0x7f000001
INSN_QUEUE_SIZE = 128
MAX_BLOCKAGE = 112          # > 1, so schedule_select runs

# function_units[] rows that this build can reach (insn-attrtab.c)
MAX_BLOCKAGE_OF = {0: 3, 1: 69, 2: 4, 3: 8, 4: 112}


class Block:
    def __init__(self, blk, use_hooked_blockage=True):
        self.n_insns = blk["n_insns"]
        self.nodes = {int(k): v for k, v in blk["nodes"].items()}
        self.deps = {int(k): [tuple(d) for d in v]
                     for k, v in blk["deps"].items()}
        self.ready0 = list(blk["ready0"])
        self.truth = [p["insn"] for p in blk["picks"]]
        self.truth_clocks = [p["clock"] for p in blk["picks"]]
        # harvested adjust_priority birthing flags, keyed by insn uid
        self.birth = {}
        for a in blk.get("adjpri", []):
            self.birth[a["insn"]] = a["birth"]
        self.use_hooked_blockage = use_hooked_blockage
        self.unit_n = {int(k): v
                       for k, v in blk.get("unit_n_insns", {}).items()}
        # schedule_insns creates one dummy insn up front to hang dependences
        # off of; it lives outside head..next_tail so the node dump misses it.
        # It is never scheduled -- give it inert attributes.
        for lst in self.deps.values():
            for pred, _ in lst:
                self.nodes.setdefault(pred, {
                    "luid": -1, "unit": -1, "icost": 1, "code": -1,
                    "pri": 0, "ref": 1 << 30, "grp": 0, "extern": 1})

    # --- machine model ---------------------------------------------------
    def unit(self, uid):
        return self.nodes[uid]["unit"]

    def icost(self, uid):
        return self.nodes[uid]["icost"]

    def insn_cost(self, prev, kind, used):
        """sched.c insn_cost + the MIPS ADJUST_COST."""
        if kind != 0:                       # anti / output -> cost 0 -> 1
            return 1
        if self.nodes[used]["code"] < 0:    # a USE never forces a wait
            return 1
        c = self.icost(prev)
        return c if c >= 1 else 1


class Sim:
    def __init__(self, blk: Block, trace=False):
        self.B = blk
        self.trace = trace
        self.pri = {u: n["pri"] for u, n in blk.nodes.items()}
        self.ref = {u: n["ref"] for u, n in blk.nodes.items()}
        self.tick = {u: 0 for u in blk.nodes}
        self.queue = {}                      # q index -> [uid]
        self.unit_tick = {}                  # unit -> tick
        self.unit_last = {}                  # unit -> uid
        self.last_scheduled = None
        self.max_priority = 0
        self.out = []
        self.clocks = []

    # --- rank_for_schedule ------------------------------------------------
    def dep_class(self, cand):
        """Class of CAND relative to last_scheduled_insn (3/2/1)."""
        ls = self.last_scheduled
        link = None
        for pred, kind in self.B.deps.get(ls, []):
            if pred == cand:
                link = kind
                break
        if link is None or self.B.insn_cost(cand, link, ls) == 1:
            return 3
        return 1 if link == 0 else 2

    def rank(self, x, y):
        """C rank_for_schedule(&x, &y); negative keeps x first."""
        v = self.pri[y] - self.pri[x]
        if v:
            return v
        if self.last_scheduled is not None:
            v = self.dep_class(y) - self.dep_class(x)
            if v:
                return v
        return self.B.nodes[y]["luid"] - self.B.nodes[x]["luid"]

    def sched_sort(self, ready, new_ready, n_ready):
        """SCHED_SORT(READY, NEW_READY, OLD_READY) over ready[0:new_ready]."""
        d = new_ready - n_ready
        if d == 1:
            # swap_sort: insertion of the last element only
            insn = ready[new_ready - 1]
            i = new_ready - 2
            while i >= 0 and self.rank(ready[i], insn) >= 0:
                ready[i + 1] = ready[i]
                i -= 1
            ready[i + 1] = insn
        elif d > 1:
            head = sorted(ready[:new_ready], key=cmp_to_key(self.rank))
            ready[:new_ready] = head
        # d == 0: no sort at all -- the order carried over from last cycle

    # --- function units ---------------------------------------------------
    def blockage(self, unit, exec_uid, last_uid):
        """function_units[unit].blockage_function (insn, unit_last_insn).

        insn-attrtab.c generates these from mips.md; rather than re-derive the
        generated expression, the value was MEASURED off the BB2_SCHED_DEBUG
        BLOCKAGE hook (which prints raw_tick / adj_tick / max_blockage, so
        blockage = adj_tick + max_blockage - raw_tick) and fitted:

          memory  (unit 0): max (1, bmax(last) - bmax(exec) + 1)
                            -- exactly reproduces all four observed
                            (exec.bmax, last.bmax) combinations
          imuldiv (unit 1): the executing insn's own issue delay: its ready
                            cost when that exceeds 1 (imul 12, idiv 35 on
                            r3000), else 3 (the hilo issue delay)

        validate.py --blockage re-checks this against every hook row, so the
        fit is falsifiable rather than assumed."""
        ex, la = self.B.nodes[exec_uid], self.B.nodes[last_uid]
        if unit == 0:
            return max(1, la.get("bmax", 1) - ex.get("bmax", 1) + 1)
        if unit == 1:
            c = self.B.icost(exec_uid)
            return c if c > 1 else 3
        return max(self.B.icost(exec_uid), 1)

    def actual_hazard(self, unit, uid, clock, cost):
        if unit < 0:
            return cost
        tick = self.unit_tick.get(unit, 0)
        if tick - clock > cost:
            last = self.unit_last.get(unit)
            if last is not None:
                tick += self.blockage(unit, uid, last) - MAX_BLOCKAGE_OF.get(unit, 1)
            if tick - clock > cost:
                cost = tick - clock
        return cost

    def potential_hazard(self, unit, uid, cost):
        """sched.c potential_hazard, verbatim:
             ncost  = minb * 0x40 + maxb
             ncost *= (unit_n_insns[unit] - 1) * 0x1000 + unit
        with (minb, maxb) = blockage_range(unit, insn) when the unit has a
        blockage_range_function (memory and imuldiv both do), else the unit's
        max_blockage.  unit_n_insns is the block's per-unit insn census, built
        by prepare_unit during the priority() pass -- both harvested per block.
        Note the multiply: a unit with exactly one insn in the block yields
        ncost 0, so the term only ever breaks ties on contended units."""
        if unit < 0:
            return cost
        n = self.B.nodes[uid]
        minb = maxb = MAX_BLOCKAGE_OF.get(unit, 1)
        if maxb > 1:
            if n.get("bmax", -1) >= 0:
                minb, maxb = n["bmin"], n["bmax"]
            if maxb > 1:
                ncost = minb * 0x40 + maxb
                ncost *= (self.B.unit_n.get(unit, 0) - 1) * 0x1000 + unit
                if ncost > cost:
                    cost = ncost
        return cost

    def schedule_unit(self, unit, uid, clock):
        if unit < 0:
            return
        self.unit_last[unit] = uid
        self.unit_tick[unit] = clock + MAX_BLOCKAGE_OF.get(unit, 1)

    def schedule_select(self, ready, n_ready, clock):
        new_ready = n_ready
        best_insn = 0
        i = 0
        pri = self.pri[ready[0]]
        while i < n_ready:
            opri = pri
            j = i + 1
            while j < n_ready:
                pri = self.pri[ready[j]]
                if pri != opri:
                    break
                j += 1
            q = 0
            for k in range(i, j):
                uid = ready[k]
                cost = self.actual_hazard(self.B.unit(uid), uid, clock, 0)
                if cost != 0:
                    q += 1
                    ready[k] = None
                    self.queue_insn(uid, cost)
            new_ready -= q
            if j - i - q == 0:
                i = j
                continue
            if j - i - q > 1:
                best_cost = -1
                for k in range(i, j):
                    uid = ready[k]
                    if uid is None:
                        continue
                    c = self.potential_hazard(self.B.unit(uid), uid, 0)
                    if c > best_cost:
                        best_cost = c
                        best_insn = k
            break
        if best_insn != 0:
            for i in range(best_insn, 0, -1):
                ready[i - 1], ready[i] = ready[i], ready[i - 1]
        if new_ready < n_ready:
            keep = [u for u in ready[:n_ready] if u is not None]
            ready[:len(keep)] = keep
        del ready[new_ready:]
        return new_ready

    # --- queue -------------------------------------------------------------
    def queue_insn(self, uid, n_cycles):
        nq = (self.q_ptr + n_cycles) % INSN_QUEUE_SIZE
        self.queue.setdefault(nq, []).append(uid)

    # --- schedule_insn -----------------------------------------------------
    def schedule_insn(self, uid, ready, n_ready, clock):
        if MAX_BLOCKAGE > 1:
            self.schedule_unit(self.B.unit(uid), uid, clock)
        deps = self.B.deps.get(uid)
        if not deps:
            return n_ready
        new_ready = n_ready
        if n_ready > 0:
            self.max_priority = max(self.pri[ready[0]], self.pri[uid])
        else:
            self.max_priority = self.pri[uid]
        for prev, kind in deps:
            cost = self.B.insn_cost(prev, kind, uid)
            self.ref[prev] -= 1
            if self.ref[prev] != 0:
                if cost > 1:
                    self.tick[prev] = max(self.tick[prev], clock + cost)
            else:
                if self.tick[prev] - clock > cost:
                    cost = self.tick[prev] - clock
                self.adjust_priority(prev)
                if cost <= 1:
                    ready.append(prev)
                    new_ready += 1
                else:
                    self.queue_insn(prev, cost)
        return new_ready

    def adjust_priority(self, prev):
        """Pass 1 only.  REG_DEAD notes are gone by this point (sched.c says so
        in a comment), so n_deaths is always 0 and only the birthing_insn_p
        branch can fire; that flag is harvested from the ADJPRI hook."""
        if self.reload_completed:
            return
        if self.B.birth.get(prev):
            if self.max_priority > self.pri[prev]:
                self.pri[prev] = self.max_priority

    # --- schedule_block ----------------------------------------------------
    def run(self, reload_completed):
        self.reload_completed = reload_completed
        B = self.B
        ready = list(B.ready0)
        n_ready = len(ready)
        new_ready = n_ready
        self.q_ptr = 0
        clock = 0
        sched_n = 0
        guard = 0
        while sched_n < B.n_insns:
            guard += 1
            if guard > 20000:
                return self.out, self.clocks, "runaway"
            self.q_ptr = (self.q_ptr + 1) % INSN_QUEUE_SIZE
            clock += 1
            for uid in self.queue.pop(self.q_ptr, []):
                ready.append(uid)
                new_ready += 1
            if new_ready == 0:
                stalls = 1
                while stalls < INSN_QUEUE_SIZE:
                    qq = (self.q_ptr + stalls) % INSN_QUEUE_SIZE
                    if self.queue.get(qq):
                        for uid in self.queue.pop(qq):
                            ready.append(uid)
                            new_ready += 1
                        break
                    stalls += 1
                if stalls >= INSN_QUEUE_SIZE:
                    return self.out, self.clocks, "empty-queue"
                self.q_ptr = (self.q_ptr + stalls) % INSN_QUEUE_SIZE
                clock += stalls
            self.sched_sort(ready, new_ready, n_ready)
            if MAX_BLOCKAGE > 1:
                new_ready = self.schedule_select(ready, new_ready, clock)
                if new_ready == 0:
                    n_ready = 0
                    continue
            n_ready = new_ready
            uid = ready[0]
            self.last_scheduled = uid
            if self.trace:
                print(f"  T-{clock} pick {uid} pri={self.pri[uid]} "
                      f"ready={ready[:n_ready]}")
            self.out.append(uid)
            self.clocks.append(clock)
            ready.pop(0)
            n_ready -= 1
            sched_n += 1
            self.pri[uid] = LAUNCH_PRIORITY
            new_ready = self.schedule_insn(uid, ready, n_ready, clock)
            self.pri[uid] = DONE_PRIORITY
            if B.nodes[uid]["grp"]:
                return self.out, self.clocks, "sched-group"
        return self.out, self.clocks, None


def sim_block(blk, reload_completed, trace=False):
    B = Block(blk)
    s = Sim(B, trace)
    out, clocks, err = s.run(reload_completed)
    return {"truth": B.truth, "sim": out, "truth_clocks": B.truth_clocks,
            "sim_clocks": clocks, "err": err,
            "match": err is None and out == B.truth,
            "clock_match": err is None and clocks == B.truth_clocks,
            "n_insns": B.n_insns}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("model")
    ap.add_argument("--func")
    ap.add_argument("--pass", dest="passno", type=int)
    ap.add_argument("--trace", action="store_true")
    ap.add_argument("--json")
    ap.add_argument("--quiet", action="store_true")
    a = ap.parse_args()

    model = json.loads(Path(a.model).read_text())
    tot = ok = clk_ok = 0
    per_func, results = {}, []
    for f in model["funcs"]:
        if a.func and f["func"] != a.func:
            continue
        if a.passno and f["pass"] != a.passno:
            continue
        for blk in f["blocks"]:
            r = sim_block(blk, f["pass"] == 2, a.trace)
            r.update(func=f["func"], pass_=f["pass"], b=blk["b"])
            results.append(r)
            tot += 1
            ok += bool(r["match"])
            clk_ok += bool(r["clock_match"])
            k = (f["func"], f["pass"])
            p = per_func.setdefault(k, [0, 0])
            p[0] += 1
            p[1] += bool(r["match"])
            if a.trace and not r["match"]:
                print(f"MISMATCH {f['func']} pass{f['pass']} block{blk['b']} "
                      f"err={r['err']}\n  truth={r['truth']}\n  sim  ={r['sim']}")
    if a.json:
        Path(a.json).write_text(json.dumps(results))
    if not a.quiet:
        pct = 100.0 * ok / tot if tot else 0.0
        cpct = 100.0 * clk_ok / tot if tot else 0.0
        print(f"blocks {ok}/{tot} order-exact ({pct:.1f}%), "
              f"{clk_ok}/{tot} clock-exact ({cpct:.1f}%)")
        exact = sum(1 for (fn, ps), (n, k) in per_func.items() if n == k)
        print(f"functions fully exact: {exact}/{len(per_func)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
