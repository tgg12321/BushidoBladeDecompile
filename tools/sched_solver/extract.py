#!/usr/bin/env python3
"""sched_solver.extract — pull list-scheduler ground truth for one TU.

Runs the real pipeline front (cpp | instrumented cc1 -da) with BB2_SCHED_DEBUG=1
and parses the SCHEDDBG stderr stream into a per-function, per-pass, per-block
model:

  inputs   dependence graph (LOG_LINKS: pred + REG_NOTE_KIND), per-insn LUID,
           function unit, INSN_COST, INSN_CODE, INSN_PRIORITY, INSN_REF_COUNT,
           SCHED_GROUP_P, and the initial (already SCHED_SORTed) ready list
  outputs  the pick sequence (clock, picked insn, full ready-list snapshot),
           plus the schedule_select blocking / best-insn events, the
           actual_hazard unit-blockage refinements, and adjust_priority events

Usage (WSL, snapshot root):
  python3 tools/sched_solver/extract.py <stem> [--out <path>]

Read-only w.r.t. the tree; work files under tmp/sched_solver_work/.
See README.md for the stream grammar and the hook additions it depends on.
"""
import argparse, json, os, re, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
WORK = ROOT / "tmp" / "sched_solver_work"
CPP = ("mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin "
       "-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ "
       "-D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C")
CC1 = ROOT / "tools" / "gcc-2.7.2" / "cc1"              # instrumented
CC1_REF = ROOT / "tools" / "gcc-2.7.2" / "build" / "cc1"  # the build compiler
CC1_FLAGS = ("-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 "
             "-mno-abicalls -fno-builtin -w -mel -msoft-float")


def preprocess(stem: str) -> Path:
    WORK.mkdir(parents=True, exist_ok=True)
    src = ROOT / "src" / f"{stem}.c"
    ifile = WORK / f"{stem}.i"
    subprocess.run(f'{CPP} "{src}" > "{ifile}"', shell=True, cwd=ROOT, check=True,
                   stderr=subprocess.DEVNULL)
    return ifile


def parity(stem: str, ifile: Path) -> bool:
    """The instrumented cc1 must agree byte-for-byte with the build compiler
    on this TU, or the extracted model describes a different compiler."""
    a, b = WORK / f"{stem}.hook.s", WORK / f"{stem}.ref.s"
    for cc, out in ((CC1, a), (CC1_REF, b)):
        subprocess.run(f'"{cc}" {CC1_FLAGS} "{ifile}" -o "{out}"', shell=True,
                       cwd=ROOT, capture_output=True)
    return a.read_bytes() == b.read_bytes()


def run_trace(stem: str, ifile: Path) -> str:
    env = dict(os.environ, BB2_SCHED_DEBUG="1")
    r = subprocess.run(
        f'"{CC1}" {CC1_FLAGS} -da "{ifile}" -o "{WORK / (stem + ".s")}"',
        shell=True, cwd=ROOT, env=env, capture_output=True, text=True,
        errors="replace")
    return r.stderr


# --- SCHEDDBG stream grammar ------------------------------------------------
RE_FUNC = re.compile(r"^SCHEDDBG FUNC func=(\S+) pass=(\d+) nbb=(\d+)")
RE_BLOCK = re.compile(r"^SCHEDDBG block=(\d+) n_insns=(\d+) n_ready=(\d+)")
RE_NODE = re.compile(
    r"^SCHEDDBG   node insn=(\d+) luid=(-?\d+) unit=(-?\d+) icost=(-?\d+) "
    r"code=(-?\d+) pri=(-?\d+) ref=(-?\d+) grp=(\d+)"
    r"(?: bmin=(-?\d+) bmax=(-?\d+))?")
RE_UNITS = re.compile(r"^SCHEDDBG units ((?:n\d+=\d+ ?)+)")
RE_DEP = re.compile(r"^SCHEDDBG     dep insn=(\d+) pred=(\d+) kind=(\d+)")
RE_RDY = re.compile(r"^SCHEDDBG   pos=(\d+) insn=(\d+) pri=(-?\d+) luid=(-?\d+)")
RE_PICK = re.compile(
    r"^SCHEDDBG PICK clock=(\d+) picked=(\d+) \(pri=(-?\d+) luid=(-?\d+)\)")
RE_WAS = re.compile(r"^SCHEDDBG   ready was: \[(.*)\]")
RE_WASITEM = re.compile(r"(\d+)\(p=(-?\d+),l=(-?\d+)\)")
RE_SELB = re.compile(
    r"^SCHEDDBG SELBLOCK clock=(\d+) insn=(\d+) unit=(-?\d+) cost=(\d+)")
RE_SELBEST = re.compile(r"^SCHEDDBG SELBEST clock=(\d+) insn=(\d+) pos=(\d+)")
RE_BLK = re.compile(
    r"^SCHEDDBG BLOCKAGE unit=(-?\d+) clock=(\d+) raw_tick=(-?\d+) "
    r"adj_tick=(-?\d+) maxb=(\d+) exec=(\d+) last=(-?\d+)")
RE_ADJ = re.compile(
    r"^SCHEDDBG ADJPRI insn=(\d+) deaths=(\d+) birth=(\d+) maxpri=(-?\d+) "
    r"pri=(-?\d+)")


def parse(stderr: str) -> list:
    """-> [{func, pass, nbb, blocks: [...]}] in stream order."""
    funcs, cur_f, cur_b = [], None, None

    def newblock(b, n_insns, n_ready):
        return {"b": b, "n_insns": n_insns, "n_ready0": n_ready,
                "nodes": {}, "deps": {}, "ready0": [], "picks": [],
                "selblock": [], "selbest": [], "blockage": [], "adjpri": [],
                "unit_n_insns": {}}

    for line in stderr.splitlines():
        if not line.startswith("SCHEDDBG"):
            continue
        m = RE_FUNC.match(line)
        if m:
            cur_f = {"func": m.group(1), "pass": int(m.group(2)),
                     "nbb": int(m.group(3)), "blocks": []}
            funcs.append(cur_f); cur_b = None
            continue
        if cur_f is None:
            continue
        m = RE_BLOCK.match(line)
        if m:
            cur_b = newblock(int(m.group(1)), int(m.group(2)), int(m.group(3)))
            cur_f["blocks"].append(cur_b)
            continue
        if cur_b is None:
            continue
        m = RE_NODE.match(line)
        if m:
            cur_b["nodes"][m.group(1)] = {
                "luid": int(m.group(2)), "unit": int(m.group(3)),
                "icost": int(m.group(4)), "code": int(m.group(5)),
                "pri": int(m.group(6)), "ref": int(m.group(7)),
                "grp": int(m.group(8)),
                "bmin": int(m.group(9)) if m.group(9) is not None else -1,
                "bmax": int(m.group(10)) if m.group(10) is not None else -1}
            continue
        m = RE_UNITS.match(line)
        if m:
            cur_b["unit_n_insns"] = {
                t.split("=")[0][1:]: int(t.split("=")[1])
                for t in m.group(1).split()}
            continue
        m = RE_DEP.match(line)
        if m:
            cur_b["deps"].setdefault(m.group(1), []).append(
                [int(m.group(2)), int(m.group(3))])
            continue
        m = RE_RDY.match(line)
        if m:
            cur_b["ready0"].append(int(m.group(2)))
            continue
        m = RE_PICK.match(line)
        if m:
            cur_b["picks"].append({"clock": int(m.group(1)),
                                   "insn": int(m.group(2)),
                                   "pri": int(m.group(3)),
                                   "ready": None})
            continue
        m = RE_WAS.match(line)
        if m and cur_b["picks"]:
            cur_b["picks"][-1]["ready"] = [
                [int(a), int(p), int(l)]
                for a, p, l in RE_WASITEM.findall(m.group(1))]
            continue
        m = RE_SELB.match(line)
        if m:
            cur_b["selblock"].append({"clock": int(m.group(1)),
                                      "insn": int(m.group(2)),
                                      "unit": int(m.group(3)),
                                      "cost": int(m.group(4)),
                                      "at_pick": len(cur_b["picks"])})
            continue
        m = RE_SELBEST.match(line)
        if m:
            cur_b["selbest"].append({"clock": int(m.group(1)),
                                     "insn": int(m.group(2)),
                                     "pos": int(m.group(3)),
                                     "at_pick": len(cur_b["picks"])})
            continue
        m = RE_BLK.match(line)
        if m:
            cur_b["blockage"].append({"unit": int(m.group(1)),
                                      "clock": int(m.group(2)),
                                      "raw_tick": int(m.group(3)),
                                      "adj_tick": int(m.group(4)),
                                      "maxb": int(m.group(5)),
                                      "exec": int(m.group(6)),
                                      "last": int(m.group(7)),
                                      "at_pick": len(cur_b["picks"])})
            continue
        m = RE_ADJ.match(line)
        if m:
            cur_b["adjpri"].append({"insn": int(m.group(1)),
                                    "deaths": int(m.group(2)),
                                    "birth": int(m.group(3)),
                                    "maxpri": int(m.group(4)),
                                    "pri": int(m.group(5)),
                                    "at_pick": len(cur_b["picks"])})
    return funcs


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("stem")
    ap.add_argument("--out")
    ap.add_argument("--skip-parity", action="store_true")
    a = ap.parse_args()

    ifile = preprocess(a.stem)
    ok = True if a.skip_parity else parity(a.stem, ifile)
    if not ok:
        print(f"WARNING: instrumented cc1 != build/cc1 on {a.stem} "
              f"(model is NOT the build compiler's)", file=sys.stderr)
    funcs = parse(run_trace(a.stem, ifile))
    model = {"stem": a.stem, "parity": ok, "funcs": funcs}
    out = Path(a.out) if a.out else WORK / f"{a.stem}.sched.json"
    out.write_text(json.dumps(model))
    nb = sum(len(f["blocks"]) for f in funcs)
    npk = sum(len(b["picks"]) for f in funcs for b in f["blocks"])
    p1 = sum(1 for f in funcs if f["pass"] == 1)
    print(f"{a.stem}: parity={ok} funcs={len(funcs)} (pass1={p1} "
          f"pass2={len(funcs)-p1}) blocks={nb} picks={npk} -> {out}")


if __name__ == "__main__":
    main()
