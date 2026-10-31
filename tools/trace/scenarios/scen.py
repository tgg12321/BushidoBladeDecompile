#!/usr/bin/env python3
"""scen: the trace scenario library (tools/trace/scenarios/).

A scenario is a reproducible scripted run of the retail game: pad inputs
by frame, phase labels and screenshots, compiled into the "script" block
of a gdbtrace spec (tools/trace/runkit.py). Instrumentation (hit map,
value watches, stubs) is added separately, so one scenario serves every
kind of capture (Q124: the spec, the manifest and the outputs make it
re-runnable by a reviewer).

Files:
  <name>.json            a scenario
  segments/<name>.json   a reusable piece (same keys, no end_frame)
  targets.json           what the library must reach: modes, menus,
                         stages, characters, key events

Scenario keys (all optional except end_frame, unless "extends" gives it):
  description  one line: what the run does and what it shows
  covers       target keys reached, each backed by the evidence capture's
               screenshots (targets.json)
  evidence     committed captures (spec.json paths, repo-relative) whose
               "script" this scenario compiles to exactly (`verify`); a
               tools/trace/knockout.py run counts when its script is
               exactly knockout_script(this scenario's, its frames)
  extends      another scenario: start from its compiled lists
  phase_covers {"PHASE": [keys]}: covers keys that hold for that whole
               compiled phase only (e.g. the stage of one story fight);
               not claimed for the whole run (keep them out of "covers"
               unless they are), backed like covers by an evidence capture
  refines      a scenario this one re-runs unchanged: the same end_frame,
               inputs, pokes and memcards, every phase mark of it kept,
               only phase marks and screenshots added (`verify`), so a
               capture of this one splits a capture of that one's phases
  use          segment names, merged in order
  end_frame    last frame; inputs released after it, phases and
               screenshots at or after it are dropped (cut runs)
  inputs       "FRAME:BTN[+BTN..][/HOLD][@2]"  (hold 6, pad 1 by default)
  repeat       [{"from": F, "every": N, "count": K,
                 "steps": ["+OFF:BTN|BTN2|BTN3/HOLD", ...]}]
               a list "A|B|C" cycles by iteration
  phases       "FRAME:name"
  shots        "FRAME:name"
  shots_drop   screenshot names removed (after extends/use)
  pokes        [{"at": ADDR, "hex": BYTES, "phase": NAME}]: game state the
               run needs (e.g. an unlock word), written when the phase
               starts (gdbtrace pokes); part of the run, so a build puts
               them ahead of any instrument pokes and `verify` requires
               them at the head of each evidence capture's spec "pokes"
  memcards     {"1"|"2": "blank"|"unformatted"}: the memory card each slot
               starts with (default blank: formatted, empty); compiled to
               the spec's "memcards" (gdbtrace writes the card), and
               `verify` requires the evidence spec to name the same cards

  python tools/trace/scenarios/scen.py list
  python tools/trace/scenarios/scen.py show <name> [--instrument I]
  python tools/trace/scenarios/scen.py build <name> --out spec.json [--instrument I]
  python tools/trace/scenarios/scen.py verify       # compile all, match evidence
  python tools/trace/scenarios/scen.py coverage     # targets reached / missing
  python tools/trace/scenarios/scen.py run <name>... (--out-root DIR | --dir D) \\
      [--instrument I] [--bios B] [--emulator E] [--cue C]
      one headless gdbtrace run per scenario, all at once (the pool caps
      how many emulators run; the rest wait for a slot)

A <name> ending in .json is a draft file outside the library (exploring a
new flow); its "use" and "extends" still resolve in the library.

--instrument: "hitmap" (default: every game function, per phase),
"none" (script only: screenshots and phases), or a JSON file whose keys
other than "script" are merged in (values, watch, stubs, breakpoints).
Emulator files: --emulator / --cue / --bios, else BB2_REDUX / BB2_CUE /
BB2_BIOS, else <main>/tmp/pcsx-redux/pcsx-redux.exe and
<main>/Bushido Blade 2 (USA).cue (the BIOS has no default).
"""

from __future__ import annotations

import argparse
import glob
import json
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TRACE = os.path.dirname(HERE)
REPO = os.path.dirname(os.path.dirname(TRACE))
sys.path.insert(0, TRACE)

import runkit  # noqa: E402

RUNTIME = os.path.join("docs", "naming", "runtime")
DEFAULT_HOLD = 6
TOKEN_RE = re.compile(r"(\d+):([A-Z0-9_|+]+)(?:/(\d+))?(?:@([12]))?")
STEP_RE = re.compile(r"\+(\d+):([A-Z0-9_|+]+)(?:/(\d+))?(?:@([12]))?")
MARK_RE = re.compile(r"(\d+):(\S+)")
KEYS = {"description", "covers", "evidence", "extends", "use", "end_frame",
        "inputs", "repeat", "phases", "shots", "shots_drop", "segment",
        "pokes", "memcards", "phase_covers", "refines"}
POKE_KEYS = {"at", "hex", "phase"}
INSTRUMENT = {
    "none": {"max_events": 200000, "duration_s": 7200},
    "hitmap": {"max_events": 200000, "duration_s": 7200,
               "hitmap": {"tus": ["main/*"],
                          "exclude": ["main/psxsdk/*", "main/d_*"],
                          "per_phase": 1}},
}


class ScenError(Exception):
    pass


def main_checkout() -> str:
    r = subprocess.run(["git", "-C", HERE, "rev-parse", "--path-format=absolute",
                        "--git-common-dir"], capture_output=True, text=True)
    if r.returncode != 0 or not r.stdout.strip():
        return REPO
    return os.path.dirname(r.stdout.strip())


def load(path: str) -> dict:
    with open(path, encoding="utf-8") as f:
        d = json.load(f)
    if not isinstance(d, dict):
        raise ScenError(f"{path}: not a JSON object")
    extra = set(d) - KEYS
    if extra:
        raise ScenError(f"{path}: unknown keys {sorted(extra)}")
    return d


def scenario_path(name: str) -> str:
    if name.endswith(".json"):  # a draft outside the library (exploration)
        return os.path.abspath(name)
    return os.path.join(HERE, name + ".json")


def short(name: str) -> str:
    return os.path.basename(name)[:-5] if name.endswith(".json") else name


def segment_path(name: str) -> str:
    return os.path.join(HERE, "segments", name + ".json")


def names() -> list[str]:
    return sorted(os.path.basename(p)[:-5]
                  for p in glob.glob(os.path.join(HERE, "*.json"))
                  if os.path.basename(p) != "targets.json")


def press_list(buttons: str, where: str) -> list[str]:
    press = buttons.split("+")
    bad = set(press) - runkit.BUTTONS
    if bad:
        raise ScenError(f"{where}: unknown buttons {sorted(bad)}")
    return press


def input_rec(frame: int, buttons: str, hold, pad, where: str) -> dict:
    rec = {"frame": frame, "press": press_list(buttons, where),
           "hold": int(hold) if hold else DEFAULT_HOLD}
    if pad == "2":
        rec["pad"] = 2
    return rec


def parse_inputs(tokens, where: str) -> list[dict]:
    out = []
    for t in tokens:
        m = TOKEN_RE.fullmatch(t)
        if not m or "|" in m.group(2):
            raise ScenError(f"{where}: bad input {t!r}")
        out.append(input_rec(int(m.group(1)), m.group(2), m.group(3),
                             m.group(4), where))
    return out


def expand_repeat(reps, where: str) -> list[dict]:
    out = []
    for r in reps:
        try:
            start, every, count = int(r["from"]), int(r["every"]), int(r["count"])
            steps = r["steps"]
        except (KeyError, TypeError, ValueError):
            raise ScenError(f"{where}: repeat needs from/every/count/steps")
        parsed = []
        for s in steps:
            m = STEP_RE.fullmatch(s)
            if not m:
                raise ScenError(f"{where}: bad repeat step {s!r}")
            parsed.append(m.groups())
        for k in range(count):
            for off, buttons, hold, pad in parsed:
                cyc = buttons.split("|")
                out.append(input_rec(start + k * every + int(off),
                                     cyc[k % len(cyc)], hold, pad, where))
    return out


def parse_marks(tokens, where: str, what: str) -> list[dict]:
    out = []
    for t in tokens:
        m = MARK_RE.fullmatch(t)
        if not m:
            raise ScenError(f"{where}: bad {what} {t!r}")
        out.append({"frame": int(m.group(1)), "name": m.group(2)})
    return out


def parse_pokes(pokes, where: str) -> list[dict]:
    out = []
    for pk in pokes:
        if not isinstance(pk, dict) or set(pk) != POKE_KEYS or \
                not all(isinstance(v, str) and v for v in pk.values()):
            raise ScenError(f"{where}: a poke is {{\"at\", \"hex\", "
                            f"\"phase\"}} strings, not {pk!r}")
        try:
            data = bytes.fromhex(pk["hex"])
        except ValueError:
            data = b""
        if not data:
            raise ScenError(f"{where}: poke {pk['at']}: bad hex {pk['hex']!r}")
        out.append(dict(pk))
    return out


def parse_memcards(cards, where: str) -> dict:
    """{"1"|"2": kind} as given, checked (runkit.check_memcards)."""
    try:
        runkit.check_memcards(cards)
    except ValueError as e:
        raise ScenError(f"{where}: {e}")
    return dict(cards or {})


def merge(acc: dict, d: dict, where: str):
    acc["memcards"].update(parse_memcards(d.get("memcards"), where))
    acc["pokes"] += parse_pokes(d.get("pokes", []), where)
    acc["inputs"] += parse_inputs(d.get("inputs", []), where)
    acc["inputs"] += expand_repeat(d.get("repeat", []), where)
    acc["phases"] += parse_marks(d.get("phases", []), where, "phase")
    acc["screenshots"] += parse_marks(d.get("shots", []), where, "shot")


def collect(name: str, seen=()) -> tuple[dict, int | None, dict]:
    if name in seen:
        raise ScenError(f"extends cycle through {name}")
    path = scenario_path(name)
    if not os.path.exists(path):
        raise ScenError(f"no scenario {name!r} ({path})")
    d = load(path)
    if d.get("segment"):
        raise ScenError(f"{name}: is a segment")
    if "extends" in d:
        acc, end, _ = collect(d["extends"], seen + (name,))
    else:
        acc, end = {"inputs": [], "phases": [], "screenshots": [],
                    "pokes": [], "memcards": {}}, None
    for seg in d.get("use", []):
        sp = segment_path(seg)
        if not os.path.exists(sp):
            raise ScenError(f"{name}: no segment {seg!r}")
        merge(acc, load(sp), sp)
    merge(acc, d, path)
    drop = set(d.get("shots_drop", []))
    unknown = drop - {s["name"] for s in acc["screenshots"]}
    if unknown:
        raise ScenError(f"{name}: shots_drop names no screenshot: "
                        f"{sorted(unknown)}")
    acc["screenshots"] = [s for s in acc["screenshots"]
                          if s["name"] not in drop]
    end = d.get("end_frame", end)
    if isinstance(end, int):  # only inherited and generated items are cut
        own = (parse_inputs(d.get("inputs", []), path),
               parse_marks(d.get("phases", []), path, "phase")
               + parse_marks(d.get("shots", []), path, "shot"))
        late = [i for i in own[0] if i["frame"] + i["hold"] > end] + \
            [m for m in own[1] if m["frame"] >= end]
        if late:
            raise ScenError(f"{name}: listed after end_frame {end}: {late}")
    return acc, end, d


def compile_script(name: str) -> dict:
    acc, end, _ = collect(name)
    if not isinstance(end, int):
        raise ScenError(f"{name}: no end_frame")

    def key(x):
        return x["frame"]
    script = {
        "end_frame": end,
        "phases": sorted([p for p in acc["phases"] if p["frame"] < end],
                         key=key),
        "inputs": sorted([i for i in acc["inputs"]
                          if i["frame"] + i["hold"] <= end], key=key),
        "screenshots": sorted([s for s in acc["screenshots"]
                               if s["frame"] < end], key=key),
    }
    try:
        runkit.check_script(script)
    except SystemExit as e:
        raise ScenError(f"{name}: {e}")
    return script


KNOCKOUT_SHOT = re.compile(r"knockout\.(\d+)")


def knockout_script(script: dict, frames) -> dict:
    """The script tools/trace/knockout.py runs: the scenario's own inputs,
    phases and shots, plus one knockout.<frame> shot per compare frame and
    knockout.live at the end (end_frame extended if the frames need it)."""
    frames = sorted(set(frames))
    script = json.loads(json.dumps(script))
    live = max(script["end_frame"] - 1, frames[-1] + 60)
    script["end_frame"] = max(script["end_frame"], live + 1)
    shots = [{"frame": n, "name": f"knockout.{n}"} for n in frames]
    shots.append({"frame": live, "name": "knockout.live"})
    if {s["name"] for s in script["screenshots"]} & \
            {s["name"] for s in shots}:
        raise ValueError("scenario uses reserved knockout screenshot names")
    script["screenshots"] = sorted(script["screenshots"] + shots,
                                   key=lambda s: s["frame"])
    return script


def knockout_frames(script) -> list[int] | None:
    """The compare frames of a knockout.py run's script, else None."""
    if not isinstance(script, dict) or \
            not isinstance(script.get("screenshots"), list):
        return None
    names = [s.get("name") for s in script["screenshots"]
             if isinstance(s, dict)]
    if "knockout.live" not in names:
        return None
    frames = [int(m.group(1)) for m in
              (KNOCKOUT_SHOT.fullmatch(str(n)) for n in names) if m]
    return frames or None


def compile_pokes(name: str) -> list[dict]:
    """The scenario's own pokes; each phase must be a script phase other
    than the first (gdbtrace's rule: nothing is set up at power-on)."""
    acc, _, _ = collect(name)
    phases = [p["name"] for p in compile_script(name)["phases"]]
    for pk in acc["pokes"]:
        if pk["phase"] not in phases[1:]:
            raise ScenError(f"{name}: poke {pk['at']}: phase must be a script "
                            f"phase other than the first, not {pk['phase']!r}")
    return acc["pokes"]


def compile_memcards(name: str) -> dict:
    """The cards the run starts with, slots left blank omitted (a spec
    without "memcards" gets blank cards)."""
    acc, _, _ = collect(name)
    return {k: v for k, v in sorted(acc["memcards"].items()) if v != "blank"}


def instrument(arg: str | None) -> dict:
    if not arg or arg in INSTRUMENT:
        return dict(INSTRUMENT[arg or "hitmap"])
    with open(arg, encoding="utf-8") as f:
        extra = json.load(f)
    extra.pop("script", None)
    return {**INSTRUMENT["none"], **extra}


def build_spec(name: str, inst: str | None) -> dict:
    spec = instrument(inst)
    spec["script"] = compile_script(name)
    pokes = compile_pokes(name)
    if pokes:
        spec["pokes"] = pokes + spec.get("pokes", [])
    spec.pop("memcards", None)  # the cards are the scenario's, not the
    cards = compile_memcards(name)  # instrument's
    if cards:
        spec["memcards"] = cards
    return spec


def write_json(path: str, obj):
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        json.dump(obj, f, indent=2)
        f.write("\n")


def first_diff(a, b, path="script") -> str:
    if type(a) is not type(b):
        return f"{path}: {a!r} != {b!r}"
    if isinstance(a, dict):
        for k in sorted(set(a) | set(b)):
            if k not in a or k not in b:
                return f"{path}.{k}: only in {'library' if k in a else 'capture'}"
            if a[k] != b[k]:
                return first_diff(a[k], b[k], f"{path}.{k}")
    if isinstance(a, list):
        for i, (x, y) in enumerate(zip(a, b)):
            if x != y:
                return first_diff(x, y, f"{path}[{i}]")
        if len(a) != len(b):
            return f"{path}: {len(a)} items in the library, {len(b)} in the capture"
    return f"{path}: {a!r} != {b!r}"


def load_targets() -> dict:
    with open(os.path.join(HERE, "targets.json"), encoding="utf-8") as f:
        return json.load(f)


def target_keys(t: dict) -> set[str]:
    return {k for group in t["groups"].values() for k in group["keys"]}


# -------------------------------------------------------------- commands

def cmd_list(args) -> int:
    for n in names():
        d = load(scenario_path(n))
        if d.get("segment"):
            continue
        s = compile_script(n)
        print(f"{n:32} end {s['end_frame']:6}  {len(s['inputs']):3} inputs "
              f"{len(s['phases']):3} phases {len(s['screenshots']):3} shots  "
              f"evidence {len(d.get('evidence', []))}")
        if d.get("description"):
            print(f"    {d['description']}")
    return 0


def cmd_show(args) -> int:
    print(json.dumps(build_spec(args.name, args.instrument), indent=2))
    return 0


def cmd_build(args) -> int:
    write_json(args.out, build_spec(args.name, args.instrument))
    print(f"scen: wrote {args.out}")
    return 0


def refinement_problems(name: str, base: str) -> list[str]:
    """Why `name` is not `base` re-run with only phase marks and
    screenshots added (empty: it refines it)."""
    mine, theirs = compile_script(name), compile_script(base)
    problems = []
    for key in ("end_frame", "inputs"):
        if json.dumps(mine[key], sort_keys=True) !=                 json.dumps(theirs[key], sort_keys=True):
            problems.append(first_diff(theirs[key], mine[key], key))
    marks = {(p["frame"], p["name"]) for p in mine["phases"]}
    lost = [p for p in theirs["phases"] if (p["frame"], p["name"]) not in marks]
    if lost:
        problems.append(f"phase marks of {base} not kept: {lost}")
    if json.dumps(compile_pokes(name), sort_keys=True) !=             json.dumps(compile_pokes(base), sort_keys=True):
        problems.append("pokes differ")
    if compile_memcards(name) != compile_memcards(base):
        problems.append("memcards differ")
    return problems


def phase_cover_problems(name: str, d: dict, keys: set[str]) -> list[str]:
    covers = d.get("phase_covers", {})
    if not isinstance(covers, dict):
        return ["phase_covers is not an object"]
    phases = {p["name"] for p in compile_script(name)["phases"]}
    problems = [] if not covers or d.get("evidence") else [
        "phase_covers without an evidence capture"]
    for phase, tags in sorted(covers.items()):
        if phase not in phases:
            problems.append(f"phase_covers: no compiled phase {phase!r}")
        if not isinstance(tags, list) or not tags or                 not all(isinstance(t, str) for t in tags):
            problems.append(f"phase_covers {phase!r}: not a list of keys")
            continue
        if set(tags) - keys:
            problems.append(f"phase_covers {phase!r}: keys not in "
                            f"targets.json: {sorted(set(tags) - keys)}")
    return problems


def cmd_verify(args) -> int:
    ok = True
    keys = target_keys(load_targets())
    claimed = {}
    for n in names():
        try:
            _, _, d = collect(n)
            script = compile_script(n)
            pokes = compile_pokes(n)
            cards = compile_memcards(n)
        except ScenError as e:
            print(f"FAIL {n}: {e}")
            ok = False
            continue
        bad = set(d.get("covers", [])) - keys
        if bad:
            print(f"FAIL {n}: covers keys not in targets.json: {sorted(bad)}")
            ok = False
        if d.get("covers") and not d.get("evidence"):
            print(f"FAIL {n}: covers without an evidence capture")
            ok = False
        try:
            problems = phase_cover_problems(n, d, keys)
            if "refines" in d:
                problems += [f"refines {d['refines']}: {p}" for p in
                             refinement_problems(n, d["refines"])]
        except ScenError as e:
            problems = [str(e)]
        for problem in problems:
            print(f"FAIL {n}: {problem}")
            ok = False
        for ev in d.get("evidence", []):
            p = os.path.join(REPO, ev)
            claimed.setdefault(os.path.normcase(os.path.abspath(p)), []).append(n)
            try:
                with open(p, encoding="utf-8") as f:
                    spec = json.load(f)
                want = spec.get("script")
                want_pokes = spec.get("pokes", [])
                if not isinstance(want_pokes, list):
                    raise ValueError(f"spec \"pokes\" is not a list: "
                                     f"{want_pokes!r}")
                want_pokes = want_pokes[:len(pokes)]
                want_cards = {str(k): v for k, v in
                              runkit.check_memcards(spec.get("memcards"))
                              .items() if v != "blank"}
            except (OSError, ValueError, AttributeError) as e:
                print(f"FAIL {n}: evidence {ev}: {e}")
                ok = False
                continue
            expect, how = script, ""
            frames = knockout_frames(want)
            if frames:
                try:
                    expect = knockout_script(script, frames)
                    how = " (knockout run)"
                except ValueError as e:
                    print(f"FAIL {n}: evidence {ev}: {e}")
                    ok = False
                    continue
            if json.dumps(want, sort_keys=True) != \
                    json.dumps(expect, sort_keys=True):  # 1 != 1.0 != true
                print(f"FAIL {n}: evidence {ev}: {first_diff(expect, want)}")
                ok = False
            elif json.dumps(want_pokes, sort_keys=True) != \
                    json.dumps(pokes, sort_keys=True):
                print(f"FAIL {n}: evidence {ev}: "
                      f"{first_diff(pokes, want_pokes, 'pokes')}")
                ok = False
            elif want_cards != cards:
                print(f"FAIL {n}: evidence {ev}: memcards {want_cards} != "
                      f"the scenario's {cards}")
                ok = False
            else:
                print(f"ok   {n} == {ev}{how}")
    for p in sorted(glob.glob(os.path.join(REPO, RUNTIME, "**", "spec.json"),
                              recursive=True)):
        with open(p, encoding="utf-8") as f:
            if "script" not in json.load(f):
                continue
        if os.path.normcase(os.path.abspath(p)) not in claimed:
            print(f"FAIL committed capture not in the library: "
                  f"{os.path.relpath(p, REPO)}")
            ok = False
    print("scen verify:", "PASS" if ok else "FAIL")
    return 0 if ok else 1


def cmd_coverage(args) -> int:
    t = load_targets()
    by_key = {}
    for n in names():
        d = load(scenario_path(n))
        for k in d.get("covers", []):
            by_key.setdefault(k, []).append(n)
    missing = 0
    for gname, group in t["groups"].items():
        print(f"{gname}{'  (list still open: ' + group['open'] + ')' if group.get('open') else ''}")
        for k in group["keys"]:
            note = group.get("notes", {}).get(k)
            got = by_key.get(k)
            if not got:
                missing += 1
            print(f"  {'+' if got else '-'} {k:28} "
                  f"{', '.join(got) if got else 'MISSING'}"
                  f"{'  [' + note + ']' if note else ''}")
    print(f"coverage: {missing} target(s) not reached")
    return 0


def cmd_run(args) -> int:
    main = main_checkout()
    emu = args.emulator or os.environ.get("BB2_REDUX") or \
        os.path.join(main, "tmp", "pcsx-redux", "pcsx-redux.exe")
    cue = args.cue or os.environ.get("BB2_CUE") or \
        os.path.join(main, "Bushido Blade 2 (USA).cue")
    bios = args.bios or os.environ.get("BB2_BIOS")
    if bool(args.out_root) == bool(args.dir):
        raise SystemExit("scen: give --out-root, or --dir with one scenario")
    if args.dir and len(args.names) != 1:
        raise SystemExit("scen: --dir takes exactly one scenario")
    for what, p in (("emulator", emu), ("cue", cue), ("bios", bios)):
        if not p or not os.path.exists(p):
            raise SystemExit(f"scen: {what} not found ({p}); pass --{what}")
    if args.dir:  # e.g. the committed capture dir docs/naming/runtime/<capture>
        args.out_root, args.names = os.path.dirname(os.path.abspath(args.dir)), \
            [args.names[0]]
    dirs = [os.path.basename(os.path.abspath(args.dir))] if args.dir else \
        [short(n) for n in args.names]
    if len(set(dirs)) != len(dirs):
        raise SystemExit(f"scen: two runs would share an output dir: {dirs}")
    specs = [(n, os.path.join(args.out_root, s), build_spec(n, args.instrument))
             for n, s in zip(args.names, dirs)]  # all compile before any launch
    procs = []
    for n, d, spec_obj in specs:
        spec = os.path.join(d, "spec.json")
        write_json(spec, spec_obj)
        cmd = [sys.executable, os.path.join(TRACE, "gdbtrace.py"), "run",
               "--spec", spec, "--out", os.path.join(d, "capture.jsonl"),
               "--launch", "pcsx-redux", "--emulator", emu, "--cue", cue,
               "--bios", bios, "--headless",
               "--slot-wait", str(args.slot_wait)]
        # a committed capture dir (--dir) keeps only the capture's own files
        log_path = os.path.join(main, "tmp", "scen-logs",
                                os.path.basename(d) + ".run.log") \
            if args.dir else os.path.join(d, "run.log")
        os.makedirs(os.path.dirname(log_path), exist_ok=True)
        log = open(log_path, "w", encoding="utf-8", newline="\n")
        procs.append((n, d, subprocess.Popen(cmd, stdout=log,
                                             stderr=subprocess.STDOUT), log))
        print(f"scen: started {n} -> {d}")
    ok = True
    for n, d, p, log in procs:
        rc = p.wait()
        log.close()
        try:
            with open(os.path.join(d, "capture.manifest.json"),
                      encoding="utf-8") as f:
                m = json.load(f)
        except (OSError, ValueError):
            m = {}
        done = bool(m.get("q124_complete"))
        ok &= rc == 0 and done
        print(f"scen: {n}: exit {rc} q124_complete={done}"
              f"{'' if done else ' ' + str(m.get('q124_missing', ''))}")
    return 0 if ok else 1


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("list")
    p = sub.add_parser("show")
    p.add_argument("name")
    p.add_argument("--instrument")
    p = sub.add_parser("build")
    p.add_argument("name")
    p.add_argument("--out", required=True)
    p.add_argument("--instrument")
    sub.add_parser("verify")
    sub.add_parser("coverage")
    p = sub.add_parser("run")
    p.add_argument("names", nargs="+")
    p.add_argument("--out-root", help="one <out-root>/<scenario>/ per run")
    p.add_argument("--dir", help="one scenario: capture into exactly this dir")
    p.add_argument("--instrument")
    p.add_argument("--emulator")
    p.add_argument("--cue")
    p.add_argument("--bios")
    p.add_argument("--slot-wait", type=float, default=86400.0,
                   help="seconds a run waits for a free emulator slot")
    args = ap.parse_args(argv)
    try:
        return {"list": cmd_list, "show": cmd_show, "build": cmd_build,
                "verify": cmd_verify, "coverage": cmd_coverage,
                "run": cmd_run}[args.cmd](args)
    except ScenError as e:
        print(f"scen: {e}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
