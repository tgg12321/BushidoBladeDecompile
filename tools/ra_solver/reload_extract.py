#!/usr/bin/env python3
"""ra_solver.reload_extract — parse the BB2_RELOAD_DEBUG stream into JSON.

Input:  tmp/reload_work/<stem>.reload.log  (produced by reload_harvest.sh)
Output: tmp/reload_work/<stem>.reload.json

Per function we record, in stream order, the reload spill events:

  order        potential_reload_regs (spill preference order), per-hard-reg
               uses counts, bad_spill_regs
  spills       new_spill_reg decisions (pass, idx, regno, class, needs)
  spill_hard   spill_hard_reg calls (pass, regno, class, cant_eliminate)
  kickouts     pseudos evicted from a spilled hard reg (pass, spillreg,
               pseudo, had, nrefs, bb)
  retries      retry_global_alloc calls: entry state (forbidden / conflicts /
               used_so_far / nrefs / livelen / calls), the find_reg calls it
               made (alt, acc, the pass-0 and pass-1 exclusion sets, prefs,
               best_reg) and the final `got`
  needs        per-pass max_needs / max_groups / max_nongroups

`RELOADDBG order` is emitted once per function by order_regs_for_reload, which
runs at the top of `reload` — so it is the natural per-function segmenter, and
every event after it (until the next one) belongs to that function.  Every line
is also func-tagged, so segmentation is checked, not assumed.
"""
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
WORK = ROOT / "tmp" / "reload_work"

RE_ORDER = re.compile(r"^RELOADDBG order func=(\S+) prr=(\S*)$")
RE_USES = re.compile(r"^RELOADDBG  uses=(\S*)$")
RE_HRS = re.compile(r"^\S+DBG\s+(\w+):(.*)$")
RE_NEEDS = re.compile(r"^RELOADDBG needs func=(\S+) pass=(\d+)(.*)$")
RE_NEED1 = re.compile(r"(\w+)\((\d+)\):n=(-?\d+),g=(-?\d+),ng=(-?\d+)")
RE_TAIL = re.compile(r"new_bb_needs=(\d+) changed=(\d+)")
RE_SPILL = re.compile(
    r"^RELOADDBG new_spill_reg func=(\S+) pass=(\d+) idx=(\d+) regno=(\d+) "
    r"class=(\d+) n_spills=(\d+) need=(-?\d+) nongroup=(-?\d+)$")
RE_SHR = re.compile(
    r"^RELOADDBG spill_hard_reg func=(\S+) pass=(\d+) regno=(\d+) class=(\d+) "
    r"cant_eliminate=(\d+) global=(\d+)$")
RE_KICK = re.compile(
    r"^RELOADDBG kickout func=(\S+) pass=(\d+) spillreg=(\d+) pseudo=(\d+) "
    r"had=(-?\d+) nrefs=(-?\d+) bb=(-?\d+)$")
RE_RETRY = re.compile(
    r"^RETRYDBG func=(\S+) pseudo=(\d+) allocno=(-?\d+) nrefs=(-?\d+) "
    r"livelen=(-?\d+) calls=(-?\d+)$")
RE_RESULT = re.compile(r"^RETRYDBG  result func=(\S+) pseudo=(\d+) got=(-?\d+)$")
RE_FR = re.compile(
    r"^FINDREGDBG func=(\S+) pseudo=(\d+) alt=(\d+) acc=(\d+) retry=(\d+)$")
RE_FRCLASS = re.compile(r"^FINDREGDBG  class=(\d+) mode=(\d+) size=(\d+)$")
RE_FRBEST = re.compile(
    r"^FINDREGDBG  best func=(\S+) pseudo=(\d+) alt=(\d+) acc=(\d+) "
    r"best_reg=(-?\d+)$")


def _regs(s):
    return [int(x) for x in s.split() if x.strip()]


def parse(path: Path) -> dict:
    funcs = {}
    cur = None          # current function record
    retry = None        # current retry_global_alloc record
    fr = None           # current find_reg record
    for line in path.read_text(errors="replace").splitlines():
        m = RE_ORDER.match(line)
        if m:
            name = m.group(1)
            cur = funcs.setdefault(name, {
                "func": name, "prr": [], "uses": {}, "bad_spill_regs": [],
                "needs": [], "spills": [], "spill_hard": [], "kickouts": [],
                "retries": [],
            })
            # reload() can run only once per function, but be defensive:
            # a second `order` line for the same name means a fresh pass.
            cur["prr"] = [int(x) for x in m.group(2).split(",") if x]
            retry = fr = None
            continue
        if cur is None:
            continue

        m = RE_USES.match(line)
        if m:
            for pair in m.group(1).split(","):
                if pair:
                    r, u = pair.split(":")
                    cur["uses"][int(r)] = int(u)
            continue

        m = RE_NEEDS.match(line)
        if m:
            rec = {"pass": int(m.group(2)), "classes": {}}
            for c in RE_NEED1.finditer(m.group(3)):
                rec["classes"][c.group(1)] = {
                    "class": int(c.group(2)), "n": int(c.group(3)),
                    "g": int(c.group(4)), "ng": int(c.group(5))}
            t = RE_TAIL.search(m.group(3))
            if t:
                rec["new_bb_needs"] = int(t.group(1))
                rec["changed"] = int(t.group(2))
            cur["needs"].append(rec)
            continue

        m = RE_SPILL.match(line)
        if m:
            cur["spills"].append({
                "pass": int(m.group(2)), "idx": int(m.group(3)),
                "regno": int(m.group(4)), "class": int(m.group(5)),
                "n_spills": int(m.group(6)), "need": int(m.group(7)),
                "nongroup": int(m.group(8))})
            continue

        m = RE_SHR.match(line)
        if m:
            cur["spill_hard"].append({
                "pass": int(m.group(2)), "regno": int(m.group(3)),
                "class": int(m.group(4)),
                "cant_eliminate": int(m.group(5)), "global": int(m.group(6)),
                "forbidden_after": []})
            continue

        m = RE_KICK.match(line)
        if m:
            cur["kickouts"].append({
                "pass": int(m.group(2)), "spillreg": int(m.group(3)),
                "pseudo": int(m.group(4)), "had": int(m.group(5)),
                "nrefs": int(m.group(6)), "bb": int(m.group(7))})
            continue

        m = RE_RETRY.match(line)
        if m:
            retry = {"pseudo": int(m.group(2)), "allocno": int(m.group(3)),
                     "nrefs": int(m.group(4)), "livelen": int(m.group(5)),
                     "calls": int(m.group(6)), "find_reg": [], "got": None}
            # pair it with the kickout that caused it (the most recent one)
            if cur["kickouts"]:
                k = cur["kickouts"][-1]
                retry["pass"] = k["pass"]
                retry["spillreg"] = k["spillreg"]
                retry["had"] = k["had"]
            cur["retries"].append(retry)
            fr = None
            continue

        m = RE_RESULT.match(line)
        if m:
            if retry is not None:
                retry["got"] = int(m.group(3))
            retry = None
            fr = None
            continue

        m = RE_FR.match(line)
        if m:
            fr = {"alt": int(m.group(3)), "acc": int(m.group(4)),
                  "retry": int(m.group(5)), "sets": {}}
            if retry is not None:
                retry["find_reg"].append(fr)
            continue

        m = RE_FRCLASS.match(line)
        if m and fr is not None:
            fr["class"] = int(m.group(1))
            fr["mode"] = int(m.group(2))
            fr["size"] = int(m.group(3))
            continue

        m = RE_FRBEST.match(line)
        if m:
            if fr is not None:
                fr["best_reg"] = int(m.group(5))
            continue

        m = RE_HRS.match(line)
        if m:
            key, body = m.group(1), _regs(m.group(2))
            if key == "forbidden_after" and cur["spill_hard"]:
                cur["spill_hard"][-1]["forbidden_after"] = body
            elif key in ("forbidden", "conflicts", "used_so_far") and retry is not None \
                    and not fr:
                retry[key] = body
            elif key == "bad_spill_regs":
                cur["bad_spill_regs"] = body
            elif fr is not None:
                fr["sets"][key] = body
            continue
    return funcs


def main(argv):
    stems = argv[1:] or sorted(
        p.name[:-len(".reload.log")] for p in WORK.glob("*.reload.log"))
    total = 0
    for stem in stems:
        log = WORK / f"{stem}.reload.log"
        if not log.exists():
            print(f"{stem}: no log", file=sys.stderr)
            continue
        funcs = parse(log)
        (WORK / f"{stem}.reload.json").write_text(json.dumps(funcs, indent=1))
        hot = {k: len(v["retries"]) for k, v in funcs.items() if v["retries"]}
        total += sum(hot.values())
        if hot:
            print(f"{stem}: " + ", ".join(
                f"{k}={n}" for k, n in sorted(hot.items(), key=lambda x: -x[1])))
    print(f"TOTAL retry_global_alloc calls: {total}")


if __name__ == "__main__":
    main(sys.argv)
