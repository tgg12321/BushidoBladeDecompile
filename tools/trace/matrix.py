#!/usr/bin/env python3
"""Restate Q124 captures as deterministic TSVs; never infer symbol meanings.

Coverage cells are phase:cell pairs separated by semicolons. Unknown (?) cells
are retained but do not count toward captures_hit. Dots are omitted within
phase lists; a lone dot means targeted with no hits in fully covered phases.
A dash (-) means the function was not targeted by that capture. capture_name
and other_capture_names are recorded symbols, not current-tree names. Value rows
retain JSONL order within each capture, including duplicate readings and errors.
Value len is int(watch len) from the spec watch, 4 when the watch sets none
(gdbtrace's own default); only an event whose watch index does not resolve
falls back to len(hex_string)//2. The entry column retains the watch index for
end-reading comparisons.
Poll rows (polls.tsv) restate every gdbtrace poll event in JSONL order: the
wall-clock t since arming (not a frame), the phase when the run had a script,
and len from the first spec poll with the same `at` (gdbtrace's default 4 when
it sets none), else len(hex_string)//2. Polls never touch coverage.
Stub leaf IDs include their parent directory to distinguish baseline captures.
A capture that is not q124_complete is refused, except a non-baseline leaf of
a -stub-, -poke- or -knockout- family whose manifest records q124_complete:
false (a compare set keeps its failed runs, reported NOT COMPARABLE): it adds
sources rows only, q124_complete false, and no values, polls or coverage. A
family baseline must be complete.
An events capture (breakpoints, watch or polls, no hit-map declaration and no
hitmap.tsv) adds sources rows only: no coverage column, and its phases.tsv is
hashed but not validated; the same holds for the phases.tsv of such a capture
that also reads values (kind values). A stub or poked capture (kind stub) with
a hit map has its TSVs validated but gets no coverage column; it keeps a
scenarios.tsv row of kind hitmap,stub.
sources.tsv has one row per input file per capture (including the optional
no-name registry). Start time uses manifest start.time/start_time, falling back
to the JSONL start event because existing manifests record only start context.

Regenerate and verify from the repository root:
python tools/trace/matrix.py build --all --no-name docs/naming/phase3/no_name.tsv
python tools/trace/matrix.py check --all --no-name docs/naming/phase3/no_name.tsv
"""

import argparse
import csv
import datetime
import hashlib
import io
import json
import math
import os
from pathlib import Path
import re
import sys

from scenarios import scen
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from tools.trace import store

REPO_ROOT = Path(__file__).resolve().parents[2]


class InvalidCapture(ValueError):
    pass


def tsv(rows):
    out = io.StringIO(newline="\n")
    csv.writer(out, delimiter="\t", lineterminator="\n").writerows(rows)
    return out.getvalue()


def scalar(value):
    if isinstance(value, str):
        return value
    return json.dumps(value, sort_keys=True, separators=(",", ":"))


def address(value):
    if not re.fullmatch(r"0x[0-9a-fA-F]{8}", value):
        raise InvalidCapture("invalid address: " + value)
    return int(value, 16)


def recorded_address(value):
    return f"0x{value:08X}" if type(value) is int else scalar(value)


def require_leaf(path):
    if any(p.parent != path for p in path.rglob("capture.manifest.json")):
        raise InvalidCapture(str(path) + ": manifest directory is not a leaf")


def timestamp(value):
    if isinstance(value, bool):
        raise InvalidCapture("invalid start time")
    try:
        number = float(value)
    except (ValueError, TypeError):
        try:
            dt = datetime.datetime.fromisoformat(value.replace("Z", "+00:00"))
            number = dt.replace(tzinfo=dt.tzinfo or datetime.timezone.utc).timestamp()
        except (ValueError, TypeError, AttributeError):
            raise InvalidCapture("missing or invalid start time") from None
    if not math.isfinite(number):
        raise InvalidCapture("invalid start time")
    return number


def table(text, header):
    rows = list(csv.reader(io.StringIO(text), delimiter="\t", strict=True))
    if not rows or rows[0][:len(header)] != header:
        raise InvalidCapture("invalid TSV header; expected " + repr(header))
    if len(set(rows[0])) != len(rows[0]) or any(not x for x in rows[0]):
        raise InvalidCapture("empty or duplicate TSV column")
    if any(len(row) != len(rows[0]) for row in rows[1:]):
        raise InvalidCapture("inconsistent TSV row width")
    return rows


def family(part):
    return "-stub-" in part or "-poke-" in part or "-knockout-" in part


def load_capture(path):
    path = path.resolve()
    require_leaf(path)
    hashes = {}

    def read(name):
        source = store.resolve(path, name, manifest) if store.is_bulk(name) else path / name
        data = source.read_bytes()
        hashes[name] = hashlib.sha1(data).hexdigest()
        return data.decode("utf-8")

    manifest = json.loads(read("capture.manifest.json"))
    try:
        relative = path.relative_to(REPO_ROOT / "docs/naming/runtime")
    except ValueError:
        relative = Path()
    in_family = any(family(part) for part in relative.parts)
    complete = manifest.get("q124_complete") is True
    failed_leaf = (any(family(part) for part in relative.parts[:-1])
                   and path.name != "baseline"
                   and manifest.get("q124_complete") is False)
    if not complete and not failed_leaf:
        raise InvalidCapture("manifest requires q124_complete: true")
    events = [json.loads(line) for line in read("capture.jsonl").splitlines()]
    if any(not isinstance(e, dict) or "ev" not in e for e in events):
        raise InvalidCapture("malformed JSONL event")
    starts = [e for e in events if e["ev"] == "start"]
    if len(starts) != 1:
        raise InvalidCapture("expected exactly one JSONL start event")
    spec = starts[0].get("spec", {})
    # Poked captures are modified games too: sources only, like stubs. A
    # -stub-, -poke- or -knockout- family (its baseline included) is a
    # compare set.
    is_stub = in_family or any(
        e["ev"] in ("stub", "poke") for e in events) or bool(
        spec.get("stubs") or spec.get("pokes"))
    is_values = bool(spec.get("values")) or any(e["ev"] == "value" for e in events)
    has_map = (path / "hitmap.tsv").exists()
    declares_map = bool(spec.get("hitmap")) or any(e["ev"] == "hitmap" for e in events)
    # breakpoint / watch / poll captures with no hit map: provenance rows only
    is_events = not declares_map and any(spec.get(k) for k in ("breakpoints", "watch", "polls"))
    kind = ("stub" if is_stub else "hitmap" if has_map else "values" if is_values
            else "events" if is_events else "other")
    ident = (path.parent.name + "/" + path.name
             if len(relative.parts) > 1 and family(relative.parts[-2]) else path.name)
    start_context = manifest.get("start", {})
    when = (start_context.get("time") if isinstance(start_context, dict) else None)
    if when is None:
        when = manifest.get("start_time", starts[0].get("time"))
    order = timestamp(when)

    # TSV-less value and stub captures are explicit alternate layouts. A
    # hitmap declaration or either TSV always requires both validated TSVs,
    # except that the phases.tsv of a capture with breakpoints, watches or
    # polls and no hit map (kind events, or values when it also reads
    # values; not a stub) is only recorded in sources.
    unmapped_phases = is_events and not has_map and not is_stub
    mapped = complete and (has_map or ((path / "phases.tsv").exists() and not unmapped_phases)
                           or declares_map or kind == "other")
    if unmapped_phases and (path / "phases.tsv").exists():
        read("phases.tsv")
    functions = {}
    if mapped:
        phases = table(read("phases.tsv"), ["phase", "first_frame", "last_frame",
                                            "screen", "inputs", "screenshots"])
        if len(phases[0]) != 6 or len(phases) < 2:
            raise InvalidCapture("phases.tsv needs phase rows and six columns")
        known = set()
        previous = -1
        for row in phases[1:]:
            phase, first, last = row[:3]
            if not phase or phase in known or ";" in phase or ":" in phase:
                raise InvalidCapture("empty, duplicate or ambiguous phase")
            if not first.isdecimal() or not last.isdecimal():
                raise InvalidCapture("invalid phase frame")
            if int(first) > int(last) or int(first) <= previous:
                raise InvalidCapture("invalid or overlapping phase span")
            previous = int(last)
            known.add(phase)
        rows = table(read("hitmap.tsv"), ["function", "addr"])
        if len(rows[0]) < 3 or not set(rows[0][2:]) <= known:
            raise InvalidCapture("hitmap.tsv has missing or unknown phases")
        for row in rows[1:]:
            name, addr = row[:2]
            addr = address(addr)
            if not name or addr in functions:
                raise InvalidCapture("empty function or duplicate address")
            cells = row[2:]
            if any(not re.fullmatch(r"[.?]|[1-9][0-9]*\+?", c) for c in cells):
                raise InvalidCapture("invalid hitmap cell")
            cell = ";".join(p + ":" + c for p, c in zip(rows[0][2:], cells) if c != ".") or "."
            hit = any(c[0].isdigit() for c in cells)
            functions[addr] = (name, cell, hit)

    values = []
    for event in events:
        if event["ev"] != "value":
            continue
        if "at" not in event or "phase" not in event or not any(
                key in event for key in ("hex", "error")):
            raise InvalidCapture("malformed value event")
        index = event.get("i")
        watches = spec.get("values", [])
        watch = watches[index] if isinstance(index, int) and 0 <= index < len(watches) else {}
        offset = event.get("offset", watch.get("ptr_off", watch.get("offset", "")))
        # gdbtrace reads int(v.get("len", 4)) for every watch; only an event whose
        # watch index does not resolve falls back to the length of its bytes
        length = (int(watch.get("len", 4)) if watch
                  else len(event.get("hex", "")) // 2)
        values.append([ident, scalar(event["phase"]), scalar(event["at"]), scalar(offset),
                       scalar(event.get("hex", "")),
                       scalar(event.get("error", "")), scalar(event.get("frame", "")),
                       scalar(index) if index is not None else "",
                       recorded_address(event.get("addr", "")),
                       recorded_address(event.get("read_at", "")),
                       scalar(event.get("end", "")),
                       scalar(event.get("pause_frame", "")),
                       recorded_address(event.get("ptr", "")), scalar(length)])
    polls = []
    for event in events:
        if event["ev"] != "poll":
            continue
        if not all(key in event for key in ("at", "hex", "t")):
            raise InvalidCapture("malformed poll event")
        spec_poll = next((p for p in spec.get("polls", [])
                          if isinstance(p, dict) and p.get("at") == event["at"]), None)
        length = (int(spec_poll.get("len", 4)) if spec_poll is not None
                  else len(event["hex"]) // 2)
        polls.append([ident, scalar(event.get("phase", "")), scalar(event["at"]),
                      recorded_address(event.get("addr", "")), scalar(event["t"]),
                      scalar(event["hex"]), scalar(length)])
    kinds = sorted(k for k, present in (("hitmap", has_map), ("values", is_values),
                                       ("stub", is_stub), ("events", kind == "events"))
                   if present)
    if not complete:
        values, polls = [], []
    return dict(ident=ident, kind=kind, kinds=kinds, path=path, complete=complete,
                manifest=manifest, script=spec.get("script"), start=when, order=order,
                hashes=hashes, functions=functions, mapped=mapped, values=values,
                polls=polls)


def scenario_rows(captures, all_evidence=False):
    """Join captures by evidence, exact script, or a guarded lib-dir fallback.

    scen evidence currently names spec.json, but manifest paths are also
    accepted. Both refer to the same capture directory. Covers are the keys
    explicitly recorded on each claiming scenario, as in scen's coverage:
    they are the scenario spec's claims, not observations in this capture.
    Multiple claims union covers and list their base-to-derived chains sorted.
    Match precedence is evidence, exact recorded script, then lib-<scenario>-N.
    JSONL's start spec embeds the runkit script referenced by the manifest;
    instrumentation (values/hitmap) is outside that script and is not compared.
    With --all, all evidence must resolve to a capture read by this run,
    including captures without coverage columns. Explicit-capture runs skip
    evidence outside the selected captures. Only mapped captures get rows.
    """
    library = REPO_ROOT / "tools/trace/scenarios"
    try:
        scenarios = {p.stem: scen.load(str(p)) for p in sorted(library.glob("*.json"))
                     if p.name != "targets.json"}
    except scen.ScenError as ex:
        raise InvalidCapture(str(ex)) from None
    def path_key(path):
        return os.path.normcase(str(path.resolve()))

    claims = {path_key(c["path"]): set() for c in captures}

    def chain(name, seen=()):
        if name not in scenarios:
            raise InvalidCapture("no scenario " + repr(name))
        if name in seen:
            raise InvalidCapture("extends cycle through " + name)
        base = scenarios[name].get("extends")
        return chain(base, seen + (name,)) + [name] if base else [name]

    chains = {}
    for name, spec in sorted(scenarios.items()):
        lineage = chain(name)
        chains[name] = "<".join(lineage) if len(lineage) > 1 else "-"
        for evidence in spec.get("evidence", []):
            path = (REPO_ROOT / evidence).resolve()
            key = path_key(path.parent)
            if key not in claims and not all_evidence:
                continue
            if (os.path.normcase(path.name) not in {"spec.json", "capture.manifest.json"}
                    or key not in claims):
                raise InvalidCapture(f"scenario {name}: evidence {evidence!r} "
                                     "is not a capture read by the matrix")
            claims[key].add(name)
    scripts = {}
    try:
        for name in sorted(scenarios):
            script = scen.compile_script(str(library / (name + ".json")))
            scripts.setdefault(json.dumps(script, sort_keys=True), set()).add(name)
    except scen.ScenError as ex:
        raise InvalidCapture(str(ex)) from None
    rows = [["capture", "scenario", "extends", "covers", "kind", "manifest_sha1", "match"]]
    for capture in captures:
        if not capture["mapped"]:
            continue
        names = sorted(claims[path_key(capture["path"])])
        match = "evidence" if names else "-"
        if not names and capture["script"] is not None:
            names = sorted(scripts.get(json.dumps(capture["script"], sort_keys=True), set()))
            if names:
                match = "script"
        if not names:
            convention = re.fullmatch(r"lib-(.+)-[0-9]+", capture["path"].name)
            if convention and convention[1] in scenarios:
                names = [convention[1]]
                match = "dirname"
        covers = sorted({key for name in names for key in scenarios[name].get("covers", [])})
        lineage = sorted({chains[name] for name in names} - {"-"})
        rows.append([capture["ident"], ",".join(names) or "-",
                     ",".join(lineage) or "-", ",".join(covers) or "-",
                     ",".join(capture["kinds"]) or "-",
                     capture["hashes"]["capture.manifest.json"], match])
    return rows


def render(paths, no_name=None, all_evidence=False):
    captures = []
    for path in paths:
        path = Path(path)
        try:
            captures.append(load_capture(path))
        except (OSError, ValueError, TypeError, KeyError, AttributeError, csv.Error) as ex:
            raise InvalidCapture(str(path) + ": " + str(ex)) from None
    captures.sort(key=lambda c: c["ident"])
    if len({c["ident"] for c in captures}) != len(captures):
        raise InvalidCapture("duplicate capture ID")
    registry = set()
    registry_hash = None
    if no_name is not None:
        data = Path(no_name).read_bytes()
        registry_hash = hashlib.sha1(data).hexdigest()
        rows = table(data.decode("utf-8"), ["address"])
        registry = {address(row[0]) for row in rows[1:]}
    # a stub or poked capture's hit map is validated but gives no coverage
    # column: it is a modified game (sources only, like any stub)
    mapped = [c for c in captures if c["mapped"] and c["kind"] != "stub"]
    functions = {}
    for capture in mapped:
        for addr, (name, cell, hit) in capture["functions"].items():
            functions.setdefault(addr, []).append((capture["order"], capture["ident"], name))
    coverage = [["addr", "capture_name", "other_capture_names", "no_name", "captures_hit"]
                + [c["ident"] for c in mapped]]
    for addr, versions in sorted(functions.items()):
        name = max(versions)[2]
        others = ";".join(sorted({v[2] for v in versions} - {name}))
        readings = [c["functions"].get(addr, ("", "-", False)) for c in mapped]
        coverage.append([f"0x{addr:08X}", name, others, int(addr in registry),
                         sum(r[2] for r in readings)] + [r[1] for r in readings])
    values = [["capture", "phase", "symbol/entry", "offset", "bytes",
               "error", "frame", "entry", "addr", "read_at", "end", "pause_frame", "ptr", "len"]]
    polls = [["capture", "phase", "symbol/entry", "addr", "t", "bytes", "len"]]
    sources = [["capture", "kind", "q124_complete", "git_commit", "tools_trace_status",
                "build_sha1", "start_time", "input_file", "input_sha1"]]
    for capture in captures:
        values.extend(capture["values"])
        polls.extend(capture["polls"])
        manifest = capture["manifest"]
        git = manifest.get("git", {})
        trace_status = git.get("tools_trace_status", "")
        hashes = dict(capture["hashes"])
        if registry_hash is not None:
            hashes["no_name.tsv"] = registry_hash
        for filename, sha1 in sorted(hashes.items()):
            sources.append([capture["ident"], capture["kind"],
                            "true" if capture["complete"] else "false",
                            git.get("head", git.get("commit", "")), scalar(trace_status),
                            manifest.get("build", {}).get("exe", {}).get("sha1", ""),
                            scalar(capture["start"]), filename, sha1])
    return {"coverage.tsv": tsv(coverage), "values.tsv": tsv(values),
            "polls.tsv": tsv(polls),
            "sources.tsv": tsv(sources),
            "scenarios.tsv": tsv(scenario_rows(captures, all_evidence))}


def discover(root):
    """Find manifest-bearing captures, excluding the generated matrix subtree."""
    paths = sorted((p.parent for p in root.rglob("capture.manifest.json")
                   if p.relative_to(root).parts[0] != "matrix"),
                  key=lambda p: p.as_posix())
    for path in paths:
        require_leaf(path)
    return paths


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=["build", "check"])
    parser.add_argument("captures", nargs="*")
    parser.add_argument("--all", action="store_true", help="discover all runtime captures")
    parser.add_argument("--out", default="docs/naming/runtime/matrix")
    parser.add_argument("--no-name", type=Path)
    # Intermixed: captures may follow options (argparse before 3.12.7 refuses that).
    args = parser.parse_intermixed_args(argv)
    if not args.captures and not args.all:
        parser.error("provide capture directories or --all")
    try:
        paths = [Path(p) for p in args.captures]
        if args.all:
            paths.extend(discover(REPO_ROOT / "docs/naming/runtime"))
            paths = sorted({p.resolve() for p in paths}, key=lambda p: p.as_posix())
        if not paths:
            raise InvalidCapture("no capture directories found")
        no_name = REPO_ROOT / args.no_name if args.no_name is not None else None
        outputs = render(paths, no_name, all_evidence=args.all)
        out = REPO_ROOT / args.out
        if args.command == "check":
            expected = set(outputs)
            actual = ({p.name for p in out.iterdir() if p.is_file()
                       and p.name != "facts.tsv"} if out.is_dir() else set())
            different = expected ^ actual
            for name, content in outputs.items():
                if not (out / name).is_file() or (out / name).read_bytes() != content.encode("utf-8"):
                    different.add(name)
            if different:
                raise InvalidCapture("matrix differs: " + ", ".join(sorted(different)))
        else:
            out.mkdir(parents=True, exist_ok=True)
            for name, content in outputs.items():
                with (out / name).open("w", encoding="utf-8", newline="\n") as stream:
                    stream.write(content)
    except (OSError, ValueError, csv.Error) as ex:
        print("matrix: " + str(ex), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
