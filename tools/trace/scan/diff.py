#!/usr/bin/env python3
"""Build/check observed only-tag and differential-scenario hit tables.

only_x rows are capture-level facts: hitting captures, armed never-hit zeros,
and every covered tag. They are ALWAYS inconclusive with capture_level_only:
true, never a Q132 "only X" conclusion on their own. Other checks retain
diagnostic reasons (unknowns, confounds, phase comparisons), not conclusions.
Single-scenario hits emit one row with tag/axis '-'; tags stay in scenarios.tsv.
Covers tags may be incomplete; partial/untargeted captures are never zeros.

Conclusive differential evidence is in pair rows: one tag differs on the same
WHOLE_CAPTURE_AXES axis (mode/stage), all hit-side replicas hit, all zero-side
replicas are armed and never hit, and hits span multiple matrix scenarios.
Matrix-wide checks reject zero-tag hits, hit-tag zeros, and untagged hits.
Pairs require matching manifest start kinds, memory-card start hashes, BIOS,
disc cue/track hashes, emulator exe/main hashes and version, and display mode.
The imported gdbtrace.SAME_INPUTS also requires matching trace-tool hashes
and build executable/ELF hashes.
Absent headless means windowed, as in gdbtrace.compare for older manifests.
Missing provenance is unknown. Only the harness's recorded power-on start is
supported; other starts (including savestates, for which it defines no schema)
are unknown and their full records are compared. Poked/stubbed specs and any
state-related spec extension make pairs inconclusive.
Pair hit_scenarios lists only that pair's hit side. Pair rows say nothing
about third-value captures on the axis and are never only-X conclusions.
Pair phases/windows/inputs come from spec.json's script, verified against
manifest.spec.sha1. TSV names, windows and expanded inputs must agree with
that script; drift is inconclusive. Script inputs default to hold 4 (runkit),
while scen-generated scripts record their explicit default hold 6.
Phase-name sequences must match. Hits must follow the first phase with
differing inputs, and subsequent inputs must match through the last hit phase,
relative to each phase's first_frame, preserving buttons and order. Identical
sequences have no divergence; an unparseable input on either side anywhere
from the start through the last hit phase makes the pair inconclusive,
including identical unparseable text: unknown inputs never match.
From divergence through the last hit, every zero phase must have duration
last_frame-first_frame at least the hit phase's, and its last_frame must be
less than the zero run's maximum last_frame (the run continues beyond it).
Inputs use scen's /N holds (default 6); the held interval [frame, frame+N)
is clipped into every overlapping phase before comparing relative inputs.
Segments retain carried-in versus pressed-here edges and whether a press
immediately follows a same-pad/button hold with no released frame between.
Presses outside their listed phase window make that phase unparseable.
Before divergence, phase lengths must match exactly as well as relative
inputs. Both runs must have contiguous phases from the first phase through
the last hit phase (including the zero side's corresponding window): each
phase starts at the preceding last_frame + 1. Together these checks allow
only a uniform shift before divergence. Runkit numbers from power-on frame 0;
we require the same recorded first_frame on both sides (a shared nonzero
origin is allowed, but a one-sided shift is not). Default /6 and explicit
/6 holds, and the TSV (2P) versus @2 pad spellings, are normalized identically.
The divergence check reads phase position only: the immediately following
phase (e.g. t_load containing confirm) is accepted if its relative inputs match.
Missing phases make rows inconclusive. Current versus-2p stage captures were
not recorded as controlled pairs; controlled-pair scenarios belong to trace-a.

Hit lists/reasons are compact sorted JSON. Inconclusive is 0/1. zero_outside,
zero_captures, axis_values_covered and covered_tags reference only_x_sets.tsv
(set_id, kind captures|values|tags, members). IDs are the first 12 SHA1 hex of
sorted compact JSON members. Each used set is written once; empty cells do
not reference sets. Check rebuilds both tables byte-for-byte. Axis values use
all scenarios.tsv captures except stub ones (kind includes stub: a stubbed
or poked run is a modified game); covered_tags includes all axes of every matrix
capture on every row. No names or roles are inferred.
All relative CLI paths are repository-relative.
"""

import argparse
import csv
import hashlib
import io
import itertools
import json
from pathlib import Path
import re
import sys

REPO_ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(REPO_ROOT))
from tools.trace.scan.facts import hit_phases, read_table
from tools.trace import runkit
sys.path.insert(0, str(REPO_ROOT / "tools/trace"))  # gdbtrace uses sibling imports
from tools.trace.gdbtrace import SAME_INPUTS, manifest_part

HEADER = ["kind", "addr", "capture_name", "no_name", "tag", "axis",
          "other_tag", "hit_captures", "hit_scenarios", "zero_outside",
          "axis_values_covered", "zero_captures", "covered_tags", "inconclusive", "reason"]
SET_HEADER = ["set_id", "kind", "members"]
SET_COLUMNS = {"zero_outside": "captures", "zero_captures": "captures",
               "axis_values_covered": "values", "covered_tags": "tags"}
# These tags describe a run as a whole; menu/event/char/weapon tags do not.
WHOLE_CAPTURE_AXES = {"mode", "stage"}


def compact(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"))


def state(text):
    if text in (".", "-"):
        return text, False, []
    cells = {}
    for pair in text.split(";"):
        phase, cell = pair.rsplit(":", 1)
        if not phase or not re.fullmatch(r"\?|[1-9][0-9]*\+?", cell):
            raise ValueError("invalid coverage cell: " + pair)
        cells[phase] = cell
    hits = hit_phases(cells)
    return ("hit" if hits else "?"), "?" in cells.values(), hits


def row(kind, entry, tag, hits, scenarios, reasons, **fields):
    if kind == "only_x":
        reasons = dict(reasons, capture_level_only=True)
    result = dict.fromkeys(HEADER, "")
    result.update(kind=kind, addr=entry["addr"], capture_name=entry["capture_name"],
                  no_name=entry["no_name"], tag=tag, axis=tag.split(":", 1)[0],
                  hit_captures=compact(sorted(hits)),
                  hit_scenarios=compact(sorted(scenarios)),
                  inconclusive=str(int(bool(reasons))), reason=compact(reasons))
    result.update(fields)
    return result


def confounds(axis, hits, zeros, tags, axes):
    def values(capture, other):
        return {t for t in tags[capture] if t.split(":", 1)[0] == other}

    reasons = {}
    if any(not values(c, axis) for c in zeros):
        reasons["unknown_on_axis"] = [axis]
    unknown, separating, separating_tags = [], {}, set()
    for other in sorted(set(axes) - {axis}):
        hit_values = set().union(*(values(c, other) for c in hits))
        zero_values = set().union(*(values(c, other) for c in zeros))
        if all(values(c, other) for c in hits) and any(
                not values(c, other) for c in zeros):
            unknown.append(other)
        if zeros and hit_values and hit_values.isdisjoint(zero_values):
            separating[other] = dict(hit=sorted(hit_values), zero=sorted(zero_values))
        if zeros:
            common = set.intersection(*(values(c, other) for c in hits))
            separating_tags.update(common - zero_values)
    if unknown:
        reasons["unknown_on_other_axis"] = unknown
    if separating:
        reasons["confounded_with_axis"] = separating
    if separating_tags:
        reasons["confounded_with_tag"] = sorted(separating_tags)
    return reasons


def relative_inputs(phases):
    """scen.py FRAME:BTN[+BTN][/HOLD][@PAD]; TSVs also spell @2 as (2P)."""
    expanded = [[] for _ in phases]
    invalid = set()
    presses = []
    for index, phase in enumerate(phases):
        for item in phase["inputs"].split(",") if phase["inputs"] not in ("", "-") else []:
            token = item.replace("(2P)", "@2")
            match = re.fullmatch(r"(\d+):([A-Z0-9_+]+)(?:/(\d+))?(?:@([12]))?", token)
            if not match or int(match[3] or 6) < 1:
                invalid.add(index)
                continue
            frame, hold, pad = int(match[1]), int(match[3] or 6), int(match[4] or 1)
            outside = not int(phase["first_frame"]) <= frame <= int(phase["last_frame"])
            if outside:
                invalid.add(index)
            presses.append((frame, hold, match[2], pad, outside))
    for frame, hold, buttons, pad, outside in presses:
        adjacent = any(p == pad and f + h == frame and set(b.split("+")) & set(buttons.split("+"))
                       for f, h, b, p, _ in presses)
        for target, span in enumerate(phases):
            first, last = int(span["first_frame"]), int(span["last_frame"])
            start, end = max(frame, first), min(frame + hold, last + 1)
            if start < end:
                expanded[target].append((start - first, buttons, pad, end - start,
                                         frame < first, adjacent))
                if outside:
                    invalid.add(target)
    for index, phase in enumerate(phases):
        phase["_relative_inputs"] = None if index in invalid else tuple(expanded[index])


def capture_start(path):
    """Read runkit.inputs_record / gdbtrace.run provenance, never follow paths."""
    manifest_path, spec_path = path / "capture.manifest.json", path / "spec.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8")) if manifest_path.is_file() else {}
    spec = json.loads(spec_path.read_text(encoding="utf-8")) if spec_path.is_file() else None
    fields, unknown = {}, []
    for keys in [("start", "kind"), ("disc", "cue")] + SAME_INPUTS:
        scalar = keys[-1] in ("kind", "version")
        field = ".".join(keys) + ("" if scalar or keys[-1] == "sha1_at_start" else ".sha1")
        value = manifest_part(manifest, keys)
        hashes = value if isinstance(value, list) else [value]
        if not value or (not scalar and any(
                not re.fullmatch(r"[0-9a-fA-F]{40}", str(h)) for h in hashes)) or (
                keys[-1] == "version" and str(value).startswith("unknown")):
            unknown.append(field)
        else:
            fields[field] = (value if scalar else [h.lower() for h in value]
                             if isinstance(value, list) else value.lower())
    if fields.get("start.kind") != "power-on: emulator held at the reset vector until armed":
        unknown.append("start (unsupported kind)")
        fields["start.record"] = manifest.get("start")
    fields["emulator.headless"] = manifest.get("emulator", {}).get("headless", False)
    if not isinstance(fields["emulator.headless"], bool):
        unknown.append("emulator.headless")
    if not manifest_path.is_file():
        unknown.append("capture.manifest.json")
    if spec is None:
        unknown.append("spec.json")

    def alters(value):
        if isinstance(value, dict):
            return any(bool(v) and (k in ("pokes", "stubs") or "state" in k.lower()
                                   or alters(v)) for k, v in value.items())
        return isinstance(value, list) and any(alters(v) for v in value)

    return fields, unknown, alters(spec)


def recorded_phases(path, tsv):
    """Use the hashed script, comparing the handwritten phase annotation to it."""
    capture = path.name
    try:
        raw = (path / "spec.json").read_bytes()
        manifest = json.loads((path / "capture.manifest.json").read_text(encoding="utf-8"))
        if hashlib.sha1(raw).hexdigest() != manifest.get("spec", {}).get("sha1"):
            return None, {"spec_unverified": [capture]}
        script = json.loads(raw)["script"]
        runkit.check_script(script)
    except (OSError, ValueError, KeyError, TypeError, SystemExit):
        return None, {"spec_unverified": [capture]}
    marks = sorted(script.get("phases", []), key=lambda p: p["frame"])
    if not marks or marks[0]["frame"] != 0:
        marks.insert(0, dict(frame=0, name=runkit.first_phase(script)))
    spans = []
    for i, mark in enumerate(marks):
        first = mark["frame"]
        last = (marks[i + 1]["frame"] if i + 1 < len(marks) else script["end_frame"]) - 1
        inputs = []
        for press in script.get("inputs", []):
            if first <= press["frame"] <= last:
                inputs.append(str(press["frame"]) + ":" + "+".join(press["press"]) +
                              "/" + str(press.get("hold", 4)) + "@" + str(press.get("pad", 1)))
        spans.append(dict(phase=mark["name"], first_frame=first, last_frame=last,
                          inputs=",".join(inputs)))
    relative_inputs(spans)
    observed = {p["phase"]: p for p in tsv or []}
    expected = {p["phase"]: p for p in spans}
    differs = set(observed) ^ set(expected)
    if list(observed) != list(expected):
        differs.update(observed)
        differs.update(expected)
    for name in set(observed) & set(expected):
        a, b = observed[name], expected[name]
        if (int(a["first_frame"]), int(a["last_frame"]), a["_relative_inputs"]) != (
                b["first_frame"], b["last_frame"], b["_relative_inputs"]):
            differs.add(name)
    reasons = {"phases_tsv_differs_from_spec": {capture: sorted(differs)}} if differs else {}
    return spans, reasons


def start_reasons(captures, starts):
    unknown, altered, differences = [], [], {}
    fields = {}
    for capture in sorted(captures):
        record, missing, changes = starts[capture]
        unknown.extend(capture + ":" + field for field in missing)
        if changes:
            altered.append(capture)
        for field, value in record.items():
            fields.setdefault(field, {})[compact(value)] = value
    for field, values in fields.items():
        if len(values) > 1:
            differences[field] = [values[key] for key in sorted(values)]
    reasons = {}
    if unknown:
        reasons["start_state_unknown"] = sorted(unknown)
    if differences:
        reasons["start_state_differs"] = differences
    if altered:
        reasons["spec_alters_state"] = altered
    return reasons


def divergence_reasons(hits, zeros, phases, pair):
    """Compare every hitting capture against every zero capture, conservatively."""
    differing, early, later, incomparable = {}, {}, set(), []
    unparseable = set()
    shorter = set()
    timing = set()
    gaps = {}
    starts = {}
    for capture, hit_names in sorted(hits.items()):
        left = phases[capture]
        if left is None:
            if not pair:
                incomparable.append(capture)
            continue
        names = [r["phase"] for r in left]
        comparable = False
        for zero in sorted(zeros):
            right = phases[zero]
            if right is None:
                continue
            other_names = [r["phase"] for r in right]
            if names != other_names:
                if pair:
                    differing[capture], differing[zero] = names, other_names
                continue
            comparable = True
            divergence = next((i for i, (a, b) in enumerate(zip(left, right))
                               if a["_relative_inputs"] is None or b["_relative_inputs"] is None
                               or a["_relative_inputs"] != b["_relative_inputs"]), len(names))
            before = [p for p in hit_names if p not in names or names.index(p) <= divergence]
            if before:
                early.setdefault(capture, set()).update(before)
            if pair:
                last_hit = max((names.index(p) for p in hit_names if p in names), default=-1)
                if int(left[0]["first_frame"]) != int(right[0]["first_frame"]):
                    starts[capture] = int(left[0]["first_frame"])
                    starts[zero] = int(right[0]["first_frame"])
                for label, spans in ((capture, left), (zero, right)):
                    missing = {names[i] for i in range(1, last_hit + 1)
                               if int(spans[i]["first_frame"])
                               != int(spans[i - 1]["last_frame"]) + 1}
                    if missing:
                        gaps.setdefault(label, set()).update(missing)
                timing.update(names[i] for i in range(divergence)
                              if int(left[i]["last_frame"]) - int(left[i]["first_frame"])
                              != int(right[i]["last_frame"]) - int(right[i]["first_frame"]))
                run_end = max(int(p["last_frame"]) for p in right)
                shorter.update(names[i] for i in range(divergence, last_hit + 1)
                               if int(right[i]["last_frame"]) - int(right[i]["first_frame"])
                               < int(left[i]["last_frame"]) - int(left[i]["first_frame"])
                               or int(right[i]["last_frame"]) >= run_end)
                unparseable.update(names[i] for i in range(last_hit + 1)
                                   if left[i]["_relative_inputs"] is None
                                   or right[i]["_relative_inputs"] is None)
                for i in range(divergence + 1, last_hit + 1):
                    a, b = left[i]["_relative_inputs"], right[i]["_relative_inputs"]
                    if a is None or b is None or a != b:
                        later.add(names[i])
        if not pair and not comparable:
            incomparable.append(capture)
    reasons = {}
    if differing:
        reasons["phase_lists_differ"] = differing
    if early:
        reasons["hit_at_or_before_divergence"] = {
            c: [p for p in hits[c] if p in early[c]] for c in sorted(early)}
    if later:
        reasons["inputs_differ_after_divergence"] = sorted(later)
    if unparseable:
        reasons["inputs_unparseable"] = sorted(unparseable)
    if shorter:
        reasons["zero_side_shorter"] = sorted(shorter)
    if timing:
        reasons["timing_differs_before_divergence"] = sorted(timing)
    if gaps:
        reasons["phase_gap"] = {c: sorted(gaps[c]) for c in sorted(gaps)}
    if starts:
        reasons["run_start_differs"] = starts
    if incomparable:
        reasons["no_comparable_zero_capture"] = sorted(incomparable)
    return reasons


def render(matrix):
    matrix = Path(matrix)
    columns, entries = read_table(matrix / "coverage.tsv",
                                  ["addr", "capture_name", "no_name", "captures_hit"])
    captures = columns[columns.index("captures_hit") + 1:]
    _, source = read_table(matrix / "scenarios.tsv", ["capture", "scenario", "covers"])
    tags, scenario, groups, scenario_tags, axes = {}, {}, {}, {}, {}
    for entry in source:
        capture, name = entry["capture"], entry["scenario"]
        covers = set(entry["covers"].split(",")) if entry["covers"] not in ("", "-") else set()
        if capture in tags:
            raise ValueError("duplicate scenario capture: " + capture)
        stub = "stub" in entry.get("kind", "").split(",")
        for tag in covers:
            if not re.fullmatch(r"[^:,\s]+:[^:,\s]+", tag):
                raise ValueError("invalid covers tag: " + tag)
            axis, value = tag.split(":", 1)
            if not stub:
                axes.setdefault(axis, set()).add(value)
        tags[capture], scenario[capture] = covers, name
        if name in scenario_tags and scenario_tags[name] != covers:
            raise ValueError("conflicting covers for scenario: " + name)
        scenario_tags[name] = covers
        if capture in captures:
            groups.setdefault(name, []).append(capture)
    if set(captures) - tags.keys():
        raise ValueError("coverage captures missing scenarios")
    phases = {}
    starts = {c: capture_start(matrix.parent / c) for c in captures}
    for capture in captures:
        path = matrix.parent / capture / "phases.tsv"
        phases[capture] = read_table(path, ["phase", "first_frame", "last_frame", "inputs"])[1] if path.is_file() else None
        if phases[capture] is not None:
            names = [r["phase"] for r in phases[capture]]
            if not names or len(names) != len(set(names)):
                raise ValueError("empty or duplicate phases: " + capture)
            for phase in phases[capture]:
                if int(phase["last_frame"]) < int(phase["first_frame"]):
                    raise ValueError("negative phase window: " + capture)
            relative_inputs(phases[capture])
    recorded = {c: recorded_phases(matrix.parent / c, phases[c]) for c in captures}
    pair_phases = {c: recorded[c][0] for c in captures}
    matrix_axes = {tag.split(":", 1)[0] for c in captures for tag in tags[c]}
    pairs = []
    for a, b in itertools.combinations(sorted(groups), 2):
        left, right = scenario_tags[a] - scenario_tags[b], scenario_tags[b] - scenario_tags[a]
        if len(left) == len(right) == 1:
            pairs.extend([(a, b, next(iter(left)), next(iter(right))),
                          (b, a, next(iter(right)), next(iter(left)))])
    rows = []
    for entry in entries:
        states = {c: state(entry[c]) for c in captures}
        hits = {c for c in captures if states[c][0] == "hit"}
        if not hits:
            continue
        hit_scenarios = {scenario[c] for c in hits}
        phases_hit = {c: states[c][2] for c in hits}
        if len(hit_scenarios) == 1:
            rows.append(row("only_x", entry, "-", hits, hit_scenarios,
                            {"single_hit_scenario": sorted(hit_scenarios)},
                            zero_outside=compact(sorted(
                                c for c in captures if scenario[c] not in hit_scenarios
                                and states[c][0] == "."))))
        shared_tags = (set.intersection(*(tags[c] for c in sorted(hits)))
                       if len(hit_scenarios) > 1 else set())
        for tag in sorted(shared_tags):
            outside = {c for c in captures if tag not in tags[c]}
            if not any(states[c][0] != "-" for c in outside):
                continue
            reasons = {}
            axis = tag.split(":", 1)[0]
            zeros = {c for c in outside if states[c][0] == "."}
            if axis not in WHOLE_CAPTURE_AXES:
                reasons["within_capture_axis"] = axis
            else:
                multi_value = sorted(c for c in hits if sum(
                    t.split(":", 1)[0] == axis for t in tags[c]) > 1)
                if multi_value:
                    reasons["multi_value_capture"] = multi_value
                reasons.update(confounds(axis, hits, zeros, tags, matrix_axes))
                if axis == "stage":
                    reasons.update(divergence_reasons(phases_hit, zeros, phases, pair=False))
            for key, status in (("outside_partial", "?"), ("outside_not_targeted", "-")):
                unknown = sorted(c for c in outside if states[c][0] == status)
                if unknown:
                    reasons[key] = unknown
            rows.append(row("only_x", entry, tag, hits, hit_scenarios, reasons,
                            zero_outside=compact(sorted(zeros)),
                            axis_values_covered=compact(sorted(axes[tag.split(":", 1)[0]]))))
        for a, b, tag, other in pairs:
            hit = hits.intersection(groups[a])
            if not hit or not all(states[c][0] == "." for c in groups[b]):
                continue
            partial = sorted(c for c in groups[a] if states[c][1])
            untargeted = sorted(c for c in groups[a] if states[c][0] == "-")
            reasons = start_reasons(set(groups[a]) | set(groups[b]), starts)
            for c in sorted(set(groups[a]) | set(groups[b])):
                for key, value in recorded[c][1].items():
                    if isinstance(value, dict):
                        reasons.setdefault(key, {}).update(value)
                    else:
                        reasons.setdefault(key, []).extend(value)
            if len(hit_scenarios) == 1:
                reasons["single_hit_scenario"] = sorted(hit_scenarios)
            pair_axes = {t.split(":", 1)[0] for t in (tag, other)}
            if len(pair_axes) != 1 or not pair_axes <= WHOLE_CAPTURE_AXES:
                reasons["within_capture_axis"] = (next(iter(pair_axes))
                                                  if len(pair_axes) == 1 else sorted(pair_axes))
            if partial:
                reasons["hit_side_partial"] = partial
            if untargeted:
                reasons["hit_side_not_targeted"] = untargeted
            not_reproduced = sorted(c for c in groups[a] if states[c][0] == ".")
            if not_reproduced:
                reasons["hit_side_not_reproduced"] = not_reproduced
            elsewhere = sorted(c for c in hits - set(groups[a]) - set(groups[b])
                               if other in tags[c])
            if elsewhere:
                reasons["zero_tag_hit_elsewhere"] = elsewhere
            outside_pair = set(captures) - set(groups[a]) - set(groups[b])
            hit_tag_zeros = sorted(c for c in outside_pair
                                   if tag in tags[c] and states[c][0] == ".")
            if hit_tag_zeros:
                reasons["hit_tag_zero_elsewhere"] = hit_tag_zeros
            axis = tag.split(":", 1)[0]
            untagged = sorted(c for c in hits if not any(
                t.split(":", 1)[0] == axis for t in tags[c]))
            if untagged:
                reasons["hit_in_untagged_capture"] = untagged
            reasons.update(divergence_reasons({c: phases_hit[c] for c in hit},
                                              groups[b], pair_phases, pair=True))
            rows.append(row("pair", entry, tag, hit, [a], reasons, other_tag=other,
                            zero_captures=compact(sorted(groups[b]))))
    sets = {}
    covered_tags = compact(sorted(set().union(*(tags[c] for c in captures))))
    for item in rows:
        used_captures = set(json.loads(item["hit_captures"]))
        for column in ("zero_outside", "zero_captures"):
            if item[column]:
                used_captures.update(json.loads(item[column]))
        missing = sorted(c for c in used_captures if phases[c] is None)
        if missing:
            reasons = json.loads(item["reason"])
            reasons["phases_missing"] = missing
            item.update(inconclusive="1", reason=compact(reasons))
        item["covered_tags"] = covered_tags
        for column, kind in SET_COLUMNS.items():
            members = item[column]
            if not members:
                continue
            set_id = hashlib.sha1(members.encode("utf-8")).hexdigest()[:12]
            entry = dict(set_id=set_id, kind=kind, members=members)
            if set_id in sets and sets[set_id] != entry:
                raise ValueError("set ID collision or conflicting kind: " + set_id)
            sets[set_id] = entry
            item[column] = set_id
    rows.sort(key=lambda r: tuple(r[c] for c in HEADER))
    return (table(HEADER, rows),
            table(SET_HEADER, [sets[key] for key in sorted(sets)]))


def table(header, rows):
    stream = io.StringIO(newline="")
    writer = csv.DictWriter(stream, header, delimiter="\t", lineterminator="\n")
    writer.writeheader()
    writer.writerows(rows)
    return stream.getvalue()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=["build", "check"])
    parser.add_argument("--matrix", default="docs/naming/runtime/matrix")
    parser.add_argument("--out", default="docs/naming/runtime/evidence/only_x.tsv")
    parser.add_argument("--sets-out", help="defaults to only_x_sets.tsv beside --out")
    args = parser.parse_args(argv)
    try:
        contents = render(REPO_ROOT / args.matrix)
        out = REPO_ROOT / args.out
        sets_out = (REPO_ROOT / args.sets_out if args.sets_out
                    else out.with_name("only_x_sets.tsv"))
        if out.resolve() == sets_out.resolve():
            raise ValueError("table and sets output paths must differ")
        for path, content in zip((out, sets_out), contents):
            if args.command == "check":
                if not path.is_file() or path.read_bytes() != content.encode("utf-8"):
                    raise ValueError("diff table differs: " + str(path))
            else:
                path.parent.mkdir(parents=True, exist_ok=True)
                with path.open("w", encoding="utf-8", newline="") as stream:
                    stream.write(content)
    except (OSError, ValueError, KeyError, csv.Error) as ex:
        print("diff: " + str(ex), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
