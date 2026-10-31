#!/usr/bin/env python3
"""Stdlib scenario compiler checks; no emulator or real-library writes."""

import contextlib
import io
import json
from pathlib import Path
import sys
import tempfile

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))

import scen  # noqa: E402


def check(cond, msg, detail="condition was false"):
    print(("ok  " + msg) if cond else ("FAIL " + msg + ": " + detail))
    return bool(cond)


def equal(actual, expected):
    if actual != expected:
        raise AssertionError(f"{actual!r} != {expected!r}")


def rejects(action, text):
    try:
        action()
    except scen.ScenError as exc:
        if text not in str(exc):
            raise AssertionError(f"expected {text!r} in {str(exc)!r}")
    else:
        raise AssertionError("expected ScenError")


def write(root, name, data):
    path = root / (name + ".json")
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="\n") as stream:
        json.dump(data, stream)
        stream.write("\n")
    return str(path)


def main():
    ok = True

    def run(name, action):
        nonlocal ok
        try:
            action()
        except (Exception, SystemExit) as exc:
            detail = f"{type(exc).__name__}: {exc}"
            ok &= check(False, name, " ".join(detail.splitlines()))
        else:
            ok &= check(True, name)

    tokens = [
        ("6100:CROSS", {"frame": 6100, "press": ["CROSS"], "hold": 6}),
        ("9500:RIGHT/40", {"frame": 9500, "press": ["RIGHT"],
                           "hold": 40}),
        ("6640:CROSS@2", {"frame": 6640, "press": ["CROSS"],
                          "hold": 6, "pad": 2}),
        ("100:L1+R1/10@2", {"frame": 100, "press": ["L1", "R1"],
                            "hold": 10, "pad": 2}),
    ]
    for token, expected in tokens:
        run("input " + token, lambda t=token, e=expected: equal(
            scen.parse_inputs([t], "test"), [e]))
    for token in ("100:cross", "100:CROSS@3", "100:A|B", "100:NOPE"):
        run("reject input " + token, lambda t=token: rejects(
            lambda: scen.parse_inputs([t], "test"), "test:"))

    repeat = [{"from": 100, "every": 100, "count": 5,
               "steps": ["+0:RIGHT/40", "+50:TRIANGLE|CIRCLE|SQUARE"]}]
    expected = []
    for k in range(5):
        expected.extend([
            {"frame": 100 + k * 100, "press": ["RIGHT"], "hold": 40},
            {"frame": 150 + k * 100,
             "press": [["TRIANGLE", "CIRCLE", "SQUARE"][k % 3]],
             "hold": 6},
        ])
    run("repeat frames and cycling buttons", lambda: equal(
        scen.expand_repeat(repeat, "test"), expected))

    real_here = scen.HERE
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        scen.HERE = directory
        try:
            write(root, "base", {
                "end_frame": 300,
                "inputs": ["94:CROSS", "95:RIGHT", "110:LEFT"],
                "phases": ["99:before", "100:edge", "110:after"],
                "shots": ["99:before", "100:edge", "110:after"],
            })
            write(root, "cut", {"extends": "base", "end_frame": 100})
            cut = {
                "end_frame": 100,
                "inputs": [{"frame": 94, "press": ["CROSS"], "hold": 6}],
                "phases": [{"frame": 99, "name": "before"}],
                "screenshots": [{"frame": 99, "name": "before"}],
            }
            run("inherited inputs and marks cut at boundary", lambda: equal(
                scen.compile_script("cut"), cut))

            write(root, "repeat-cut", {
                "end_frame": 140, "repeat": repeat,
            })
            run("repeat inputs cut with release at boundary kept",
                lambda: equal(scen.compile_script("repeat-cut")["inputs"],
                              [expected[0]]))

            late = [
                ("input release", {"inputs": ["95:CROSS"]}),
                ("input past end", {"inputs": ["110:CROSS"]}),
                ("phase at end", {"phases": ["100:late"]}),
                ("phase past end", {"phases": ["110:late"]}),
                ("shot at end", {"shots": ["100:late"]}),
                ("shot past end", {"shots": ["110:late"]}),
            ]
            for label, data in late:
                path = write(root, "late", {"end_frame": 100, **data})
                run("reject own " + label, lambda p=path: rejects(
                    lambda: scen.compile_script(p), "listed after"))

            write(root, "drop", {"extends": "base",
                                 "shots_drop": ["edge", "after"]})
            run("extends inherits end and shots_drop removes names",
                lambda: equal(scen.compile_script("drop"), {
                    **scen.compile_script("base"),
                    "screenshots": [{"frame": 99, "name": "before"}],
                }))
            write(root, "bad-drop", {"extends": "base",
                                     "shots_drop": ["absent"]})
            run("reject unknown shots_drop", lambda: rejects(
                lambda: scen.compile_script("bad-drop"), "absent"))

            segment = write(root, "segments/piece", {
                "segment": True, "inputs": ["10:START"],
                "phases": ["0:boot"], "shots": ["20:title"],
            })
            write(root, "use", {"end_frame": 100, "use": ["piece"]})
            run("use segment merges all lists", lambda: equal(
                scen.compile_script("use"), {
                    "end_frame": 100,
                    "inputs": [{"frame": 10, "press": ["START"],
                                "hold": 6}],
                    "phases": [{"frame": 0, "name": "boot"}],
                    "screenshots": [{"frame": 20, "name": "title"}],
                }))
            write(root, "missing", {"end_frame": 100, "use": ["absent"]})
            run("reject missing segment", lambda: rejects(
                lambda: scen.compile_script("missing"), "no segment"))
            run("reject segment as scenario", lambda: rejects(
                lambda: scen.compile_script(segment), "is a segment"))
            write(root, "cycle-a", {"extends": "cycle-b"})
            write(root, "cycle-b", {"extends": "cycle-a"})
            run("reject extends cycle", lambda: rejects(
                lambda: scen.compile_script("cycle-a"), "extends cycle"))
            write(root, "unknown", {"end_frame": 100, "typo": []})
            run("reject unknown top-level key", lambda: rejects(
                lambda: scen.compile_script("unknown"), "unknown keys"))

            write(root, "unsorted", {
                "end_frame": 100,
                "inputs": ["70:RIGHT", "10:CROSS", "40:LEFT"],
                "phases": ["70:last", "0:boot", "40:middle"],
                "shots": ["70:last", "10:first", "40:middle"],
            })
            for key, frames in (("inputs", [10, 40, 70]),
                                ("phases", [0, 40, 70]),
                                ("screenshots", [10, 40, 70])):
                run("sorted " + key, lambda k=key, f=frames: equal(
                    [x["frame"] for x in scen.compile_script("unsorted")[k]],
                    f))

            for name, data, message in (
                ("phase-clash", {"phases": ["10:a", "10:b"]},
                 "two phases start on one frame"),
                ("shot-clash", {"shots": ["10:a", "20:a"]},
                 "screenshot names must be unique"),
            ):
                write(root, name, {"end_frame": 100, **data})
                run("runkit error: " + name, lambda n=name, m=message:
                    rejects(lambda: scen.compile_script(n), m))

            poke = {"at": "0x80106A50", "hex": "FFF33F05", "phase": "edge"}
            write(root, "poked", {"extends": "base", "pokes": [poke]})
            write(root, "poked-child", {"extends": "poked"})
            inst_pokes = write(root, "inst-pokes", {
                "pokes": [{"at": "0x80100000", "hex": "01",
                           "phase": "after"}]})
            run("scenario pokes go ahead of instrument pokes", lambda: equal(
                scen.build_spec("poked", inst_pokes)["pokes"],
                [poke, {"at": "0x80100000", "hex": "01", "phase": "after"}]))
            run("pokes leave the script alone", lambda: equal(
                scen.build_spec("poked", "none")["script"],
                scen.compile_script("base")))
            run("no scenario pokes: spec has none", lambda: equal(
                "pokes" in scen.build_spec("base", "none"), False))
            run("extends inherits pokes", lambda: equal(
                scen.compile_pokes("poked-child"), [poke]))
            for label, pk, message in (
                ("first phase", {**poke, "phase": "before"}, "other than"),
                ("unknown phase", {**poke, "phase": "nope"}, "other than"),
                ("bad hex", {**poke, "hex": "XY"}, "bad hex"),
                ("missing key", {"at": "0x80106A50", "hex": "00"},
                 "a poke is"),
                ("extra key", {**poke, "when": "x"}, "a poke is"),
            ):
                path = write(root, "bad-poke", {"extends": "base",
                                                "pokes": [pk]})
                run("reject poke " + label, lambda p=path, m=message:
                    rejects(lambda: scen.compile_pokes(p), m))
            write(root, "poke-cut", {"extends": "poked", "end_frame": 100})
            run("reject poke whose phase the cut drops", lambda: rejects(
                lambda: scen.build_spec("poke-cut", "none"), "other than"))

            run("instrument hitmap preset", lambda: equal(
                scen.instrument("hitmap"), {
                    "max_events": 200000, "duration_s": 7200,
                    "hitmap": {"tus": ["main/*"],
                               "exclude": ["main/psxsdk/*", "main/d_*"],
                               "per_phase": 1},
                }))
            run("instrument none preset", lambda: equal(
                scen.instrument("none"),
                {"max_events": 200000, "duration_s": 7200}))
            inst = write(root, "instrument", {
                "values": [{"at": "0x80010000"}], "duration_s": 123,
                "script": {"end_frame": 999},
            })
            run("instrument JSON merges values and ignores script",
                lambda: equal(scen.instrument(inst), {
                    "max_events": 200000, "duration_s": 123,
                    "values": [{"at": "0x80010000"}],
                }))
        finally:
            scen.HERE = real_here

    def verify_library():
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            result = scen.main(["verify"])
        if result != 0:
            raise AssertionError(output.getvalue().strip())

    def verify_pokes():
        real = (scen.HERE, scen.REPO)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            scen.HERE = str(root / "scen")
            scen.REPO = str(root)
            try:
                write(root / "scen", "targets", {"groups": {}})
                poke = {"at": "0x80106A50", "hex": "FFF33F05",
                        "phase": "menu"}
                write(root / "scen", "p", {
                    "end_frame": 100, "phases": ["0:boot", "50:menu"],
                    "pokes": [poke],
                    "evidence": ["docs/naming/runtime/c/spec.json"]})
                script = scen.compile_script("p")
                extra = {"at": "0x80100000", "hex": "01", "phase": "menu"}
                for label, pokes, want in (
                    ("verify: scenario pokes then instrument pokes",
                     [poke, extra], 0),
                    ("verify: capture missing the poke", [], 1),
                    ("verify: capture poke changed",
                     [{**poke, "hex": "00"}], 1),
                    ("verify: capture pokes reordered", [extra, poke], 1),
                    ("verify: capture pokes null", None, 1),
                ):
                    write(root / "docs/naming/runtime/c", "spec",
                          {"script": script, "pokes": pokes})
                    output = io.StringIO()
                    with contextlib.redirect_stdout(output):
                        got = scen.main(["verify"])
                    run(label, lambda g=got, w=want, o=output.getvalue():
                        equal((g, o.strip().splitlines()[-1]),
                              (w, "scen verify: " +
                               ("PASS" if w == 0 else "FAIL"))))
            finally:
                scen.HERE, scen.REPO = real

    def verify_knockout():
        real = (scen.HERE, scen.REPO)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            scen.HERE = str(root / "scen")
            scen.REPO = str(root)
            try:
                write(root / "scen", "targets", {"groups": {}})
                write(root / "scen", "k", {
                    "end_frame": 100, "phases": ["0:boot", "50:menu"],
                    "shots": ["60:menu.first"],
                    "evidence": ["docs/naming/runtime/c/spec.json"]})
                script = scen.compile_script("k")
                ko = scen.knockout_script(script, [30, 20])
                run("knockout_script: shots added, end_frame kept",
                    lambda: equal((ko["end_frame"], [
                        (s["frame"], s["name"]) for s in ko["screenshots"]]),
                        (100, [(20, "knockout.20"), (30, "knockout.30"),
                               (60, "menu.first"),
                               (99, "knockout.live")])))
                late = scen.knockout_script(script, [90])
                run("knockout_script: end_frame extended for a late frame",
                    lambda: equal((late["end_frame"],
                                   late["screenshots"][-1]),
                                  (151, {"frame": 150,
                                         "name": "knockout.live"})))
                run("knockout_script leaves the scenario script alone",
                    lambda: equal(len(script["screenshots"]), 1))
                moved = json.loads(json.dumps(ko))
                moved["screenshots"][1]["frame"] = 71
                extra = json.loads(json.dumps(ko))
                extra["inputs"] = [{"frame": 55, "press": ["CROSS"],
                                    "hold": 6}]
                nolive = json.loads(json.dumps(ko))
                nolive["screenshots"] = [s for s in nolive["screenshots"]
                                         if s["name"] != "knockout.live"]
                for label, want_script, want in (
                    ("verify: knockout run of the scenario", ko, 0),
                    ("verify: knockout run, shot frame not its name",
                     moved, 1),
                    ("verify: knockout run with an extra input", extra, 1),
                    ("verify: knockout shots without knockout.live",
                     nolive, 1),
                ):
                    write(root / "docs/naming/runtime/c", "spec",
                          {"script": want_script})
                    output = io.StringIO()
                    with contextlib.redirect_stdout(output):
                        got = scen.main(["verify"])
                    run(label, lambda g=got, w=want, o=output.getvalue():
                        equal((g, o.strip().splitlines()[-1]),
                              (w, "scen verify: " +
                               ("PASS" if w == 0 else "FAIL"))))
            finally:
                scen.HERE, scen.REPO = real

    def verify_memcards():
        real = (scen.HERE, scen.REPO)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            scen.HERE = str(root / "scen")
            scen.REPO = str(root)
            try:
                write(root / "scen", "targets", {"groups": {}})
                write(root / "scen", "m", {
                    "end_frame": 100, "phases": ["0:boot", "50:menu"],
                    "memcards": {"1": "unformatted", "2": "blank"},
                    "evidence": ["docs/naming/runtime/c/spec.json"]})
                write(root / "scen", "m-child", {"extends": "m"})
                write(root / "scen", "m-reset", {
                    "extends": "m", "memcards": {"1": "blank"}})
                run("memcards: blank slots omitted from the spec",
                    lambda: equal(scen.build_spec("m", "none")["memcards"],
                                  {"1": "unformatted"}))
                run("memcards: extends inherits them", lambda: equal(
                    scen.compile_memcards("m-child"), {"1": "unformatted"}))
                run("memcards: a child sets a slot back to blank",
                    lambda: equal("memcards" in scen.build_spec(
                        "m-reset", "none"), False))
                for label, cards, message in (
                    ("slot", {"3": "blank"}, "slot"),
                    ("kind", {"1": "full"}, "kind"),
                    ("not an object", ["unformatted"], "not an object"),
                ):
                    path = write(root / "drafts", "bad-cards", {
                        "end_frame": 100, "phases": ["0:boot"],
                        "memcards": cards})
                    run("reject memcards " + label, lambda p=path, m=message:
                        rejects(lambda: scen.compile_memcards(p), m))
                script = scen.compile_script("m")
                for label, cards, want in (
                    ("verify: capture names the same card", {"1":
                     "unformatted"}, 0),
                    ("verify: capture lists the blank slot too",
                     {"1": "unformatted", "2": "blank"}, 0),
                    ("verify: capture without memcards", None, 1),
                    ("verify: capture card in the other slot",
                     {"2": "unformatted"}, 1),
                ):
                    spec = {"script": script}
                    if cards is not None:
                        spec["memcards"] = cards
                    write(root / "docs/naming/runtime/c", "spec", spec)
                    output = io.StringIO()
                    with contextlib.redirect_stdout(output):
                        got = scen.main(["verify"])
                    run(label, lambda g=got, w=want, o=output.getvalue():
                        equal((g, o.strip().splitlines()[-1]),
                              (w, "scen verify: " +
                               ("PASS" if w == 0 else "FAIL"))))
                with tempfile.TemporaryDirectory() as cards_dir:
                    for kind, head in (("blank", b"MC"),
                                       ("unformatted", bytes(2))):
                        path = Path(cards_dir) / (kind + ".mcd")
                        scen.runkit.CARD_KINDS[kind][0](str(path))
                        data = path.read_bytes()
                        run(f"runkit card {kind}: 128 KiB, header "
                            f"{head!r}", lambda d=data, h=head: equal(
                                (len(d), d[:2]), (128 * 1024, h)))
            finally:
                scen.HERE, scen.REPO = real

    def verify_refines():
        real = (scen.HERE, scen.REPO)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            scen.HERE = str(root / "scen")
            scen.REPO = str(root)
            try:
                write(root / "scen", "targets", {"groups": {"stages": {
                    "keys": ["stage:x", "stage:y"]}}})
                write(root / "scen", "b", {
                    "end_frame": 100, "phases": ["0:boot", "50:story"],
                    "inputs": ["10:CROSS"]})
                split = {"extends": "b", "refines": "b",
                         "phases": ["70:st2"], "shots": ["80:st2.80"],
                         "phase_covers": {"st2": ["stage:x"]},
                         "evidence": ["docs/naming/runtime/c/spec.json"]}
                write(root / "scen", "r", split)
                write(root / "docs/naming/runtime/c", "spec",
                      {"script": scen.compile_script("r")})
                keys = {"stage:x", "stage:y"}
                run("refines: phases and shots only",
                    lambda: equal(scen.refinement_problems("r", "b"), []))
                write(root / "scen", "r", {**split, "inputs": ["90:START"]})
                run("refines: an added input is refused", lambda: equal(
                    [q.split(":")[0] for q in scen.refinement_problems("r", "b")],
                    ["inputs"]))
                write(root / "scen", "r", {**split, "end_frame": 90})
                run("refines: a cut run is refused", lambda: equal(
                    len(scen.refinement_problems("r", "b")), 1))
                write(root / "scen", "r", split)
                for label, covers, want in (
                    ("phase_covers: ok", {"st2": ["stage:x"]}, 0),
                    ("phase_covers: unknown phase", {"st9": ["stage:x"]}, 1),
                    ("phase_covers: another target key", {"st2": ["stage:y"]}, 0),
                    ("phase_covers: key not a target", {"st2": ["stage:z"]}, 1),
                    ("phase_covers: empty list", {"st2": []}, 1),
                ):
                    run(label, lambda c=covers, w=want: equal(len(
                        scen.phase_cover_problems("r", {**split, "phase_covers": c},
                                                  keys)), w))
                bare = {k: v for k, v in split.items() if k != "evidence"}
                run("phase_covers: no evidence capture", lambda: equal(
                    scen.phase_cover_problems("r", bare, keys),
                    ["phase_covers without an evidence capture"]))
                output = io.StringIO()
                with contextlib.redirect_stdout(output):
                    got = scen.main(["verify"])
                run("verify: refining scenario passes", lambda g=got, o=output.getvalue():
                    equal((g, o.strip().splitlines()[-1]), (0, "scen verify: PASS")))
            finally:
                scen.HERE, scen.REPO = real

    verify_pokes()
    verify_refines()
    verify_knockout()
    verify_memcards()
    run("real library verify", verify_library)
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
