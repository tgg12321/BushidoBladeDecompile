#!/usr/bin/env python3
"""Build/check docs/naming/runtime/evidence/events.tsv from committed Q124 captures.

All relative CLI paths are repository-relative. Counts describe recorded watch
hits and explicitly declared breakpoints, never hit-map or stub entries. A
capture from a modified game (spec stubs or pokes, or stub/poke records: the
test tools/trace/matrix.py uses for kind stub) contributes no rows. Values are
compact JSON lists (call values are a0..a3 tuples), sorted and capped at eight.
values_omitted counts additional distinct values. All inputs are exact index
blobs, not working-tree files. Site PCs print KSEG0 aliases, including BIOS
(0xBFC0xxxx prints 0x9FC0xxxx). coverage_next_start_bound is informational:
the coverage list supplies the upper bound rather than an explicit end/size.
Rows stay separate per capture, instruction site, and access offset/width.
Values describe only the intersection of the access and watched byte range,
decoded little-endian. store_offset/store_width describe the original instruction
relative to the watched target (also for reads); calls leave these columns empty.
partial_store denotes a firm observation of fewer than all watched bytes and
does not alone make a row inconclusive. Event frames are not inferred. Functions
come only from committed report site lists or bounded coverage function ranges.
No ELF or historical build path is accessed. The report's
top-ten site list is joined by gdbtrace's exact Counter.most_common ordering
(frequency, then first occurrence); unresolved/truncated sites stay unknown.
Raw addresses in that list do not establish a function. An unresolved site makes
every row for that capture/target/kind/phase inconclusive. No roles are inferred.
"""

import argparse
from collections import Counter, defaultdict
import csv
import hashlib
import io
import json
from pathlib import Path
import re
import subprocess
import sys

REPO_ROOT = Path(__file__).resolve().parents[3]
HEADER = ["target", "kind", "site_function", "phase", "site", "attrib",
          "store_offset", "store_width", "count", "values", "values_omitted",
          "captures", "inconclusive", "reason"]


def canonical(pc):
    if 0x80000000 <= pc < 0xC0000000:
        return 0x80000000 | (pc & 0x1FFFFFFF)
    return pc


def normalized_target(target):
    match = re.fullmatch(r"(0x|D_)([0-9A-Fa-f]{8})(.*)", target)
    if match:
        return match[1] + f"{canonical(int(match[2], 16)):08X}" + match[3]
    return target


def declarations(spec):
    result = {}
    for key, kind in (("breakpoints", 0), ("watch", None)):
        for entry in spec.get(key, []):
            watch_kind = kind if kind == 0 else {"write": 2, "read": 3, "access": 4}[
                entry.get("kind", "write")]
            retained = dict(entry)
            if watch_kind:
                retained["len"] = int(entry.get("len", 4))
                if retained["len"] <= 0:
                    raise ValueError("watch len must be positive")
            result[(normalized_target(entry["at"]), watch_kind)] = retained
    return result


def selected(event, declared):
    return (event.get("ev") == "hit" and not any(
        event.get(flag) for flag in ("hm", "stub_call", "stub_leak")) and
        (normalized_target(event["at"]), event["kind"]) in declared)


def modified(spec, events):
    # Hits after a stub or poke went in are from a modified game.
    return bool(spec.get("stubs") or spec.get("pokes")) or any(
        e["ev"] in ("stub", "poke") for e in events)


def limits(spec, events, end, entry, event):
    reasons = set()
    counter = f"Z{event['kind']}:{entry['at']}"
    if end is None:
        return {"missing_end_record", f"hit_count_mismatch:{counter}"}
    if end.get("script_ended") is not True:
        reasons.add("script_not_ended")
    max_events = int(spec.get("max_events", 5000))
    if spec.get("hitmap") and "max_events" not in spec:
        hitmap = next((e for e in events if e["ev"] == "hitmap"), {})
        max_events = (len(hitmap.get("targets", [])) *
                      int(spec["hitmap"].get("per_phase", 1)) *
                      (len(spec.get("script", {}).get("phases", [])) + 1) + 5000)
    if end.get("events", 0) >= max_events:
        reasons.add(f"max_events_reached:{max_events}")
    # Runner totals include hm/stub hits and survive phase resets/disarming.
    recorded = sum(e.get("ev") == "hit" and e.get("kind") == event["kind"]
                   and e.get("at") == entry["at"] for e in events)
    if end.get("hits", {}).get(counter) != recorded:
        reasons.add(f"hit_count_mismatch:{counter}")
    maximum = int(entry.get("max_hits", 1 << 30))
    if end.get("hits", {}).get(counter, 0) >= maximum:
        reasons.add(f"max_hits_reached:{counter}:{maximum}")
    if "per_phase" in entry:
        count = sum(e.get("ev") == "hit" and e.get("kind") == event["kind"]
                    and normalized_target(e.get("at", "")) == normalized_target(event["at"])
                    and e.get("phase") == event.get("phase") for e in events)
        if count >= int(entry["per_phase"]):
            reasons.add(f"per_phase_reached:{entry['per_phase']}")
    return reasons


def integer(value, field):
    if value is None or value == "?":
        return None
    if type(value) is not int or value < 0:
        raise ValueError("invalid " + field)
    return value


def raw_site(event):
    if event["kind"] == 0:
        ra = integer(event.get("ra"), "ra")
        return None if ra is None else (ra - 8) & 0xFFFFFFFF
    access = event.get("access")
    if access is None or access == "?":
        return None
    if not isinstance(access, dict):
        raise ValueError("invalid access object")
    pc = integer(access.get("pc"), "access.pc")
    return pc


def site(event):
    pc = raw_site(event)
    return None if pc is None else canonical(pc)


def coverage_rows(raw):
    if raw is None:
        return []
    reader = csv.DictReader(io.StringIO(raw.decode("utf-8")), delimiter="\t")
    if not {"addr", "capture_name"} <= set(reader.fieldnames or []):
        raise ValueError("coverage.tsv missing addr/capture_name")
    return sorted(reader, key=lambda r: canonical(int(r["addr"], 0)))


def coverage_ranges(rows):
    """Bound each recorded start by the next listed start; never extend the last.

    The matrix is a capture-time function list, not a current-tree symbol map.
    An explicit end_addr/size, when supplied, bounds a range more tightly. A
    changed/ambiguous recorded name is not selected from other_capture_names.
    """
    functions = []
    for i, row in enumerate(rows):
        start = canonical(int(row["addr"], 0))
        end = canonical(int(rows[i + 1]["addr"], 0)) if i + 1 < len(rows) else start
        if row.get("end_addr"):
            end = min(end, canonical(int(row["end_addr"], 0))) if end > start else canonical(int(row["end_addr"], 0))
        elif row.get("size"):
            end = min(end, start + int(row["size"], 0)) if end > start else start + int(row["size"], 0)
        if row["capture_name"] and end > start:
            functions.append((start, end, row["capture_name"],
                              not (row.get("end_addr") or row.get("size"))))
    return functions


def resolve_ranges(events, resolved, functions):
    for event in events:
        if event["ev"] != "hit":
            continue
        pc = site(event)
        key = ((event["at"], event["kind"]), pc)
        if pc is None or key in resolved:
            continue
        covering = [(end - start, name, inferred) for start, end, name, inferred in functions
                    if start <= pc < end]
        if covering:
            smallest = min(size for size, _, _ in covering)
            names = {name for size, name, _ in covering if size == smallest}
            if len(names) == 1:
                inferred = any(bound for size, _, bound in covering if size == smallest)
                resolved[key] = (names.pop(), "coverage_next_start_bound" if inferred else "coverage")


def report_sites(raw, events, digest, coverage):
    """Invert the report's lossless top-site projection, not a symbol guess.

    Symbols.name is one-to-one for canonical PCs (its offset is retained).
    Counter preserves first occurrence for ties. Hit-map hits are explicitly
    excluded by gdbtrace.report and therefore have no site list to invert.
    """
    if raw is None:
        return {}
    text = raw.decode("utf-8")
    if "capture SHA1 " + digest not in text:
        raise ValueError("report.md does not cite capture SHA1 " + digest)
    groups = defaultdict(list)
    for event in events:
        if event.get("ev") == "hit" and not event.get("hm"):
            groups[(event["at"], event["kind"])].append(event)
    resolved = {}
    starts = {}
    for row in coverage:
        if row["capture_name"]:
            starts.setdefault(row["capture_name"], set()).add(canonical(int(row["addr"], 0)))
    for section in re.split(r"^## ", text, flags=re.M)[1:]:
        heading = re.match(r"(.+?)(?: \(Z([234]) watch\))?: \d+ hits\n", section)
        if not heading:
            continue
        key = (heading[1], int(heading[2] or 0))
        if key not in groups:
            continue
        block = re.search(r"(?:Call sites \(ra-8\)|Accessing instructions):\n"
                          r"((?:- `[^`]+` x\d+\n)+)", section)
        if not block:
            continue
        labels = re.findall(r"^- `([^`]+)` x(\d+)$", block[1], re.M)
        sites = Counter(site(e) for e in groups[key])
        # Partial access records cannot be reversed through a stop-pc label.
        if None in sites:
            continue
        ranked = sites.most_common(10)
        if [n for _, n in ranked] != [int(n) for _, n in labels]:
            raise ValueError("report site counts disagree with capture: " + key[0])
        for (pc, _), (label, _) in zip(ranked, labels):
            encoded = re.fullmatch(r"func_([0-9A-Fa-f]{8})(?:\+0x([0-9A-Fa-f]+))?", label)
            if encoded and pc != canonical(int(encoded[1], 16) + int(encoded[2] or "0", 16)):
                raise ValueError("report site address disagrees with capture: " + label)
            if not re.fullmatch(r"[A-Za-z_.$][\w.$]*(?:\+0x[0-9A-Fa-f]+)?", label):
                continue
            name, _, offset = label.partition("+0x")
            start = pc - int(offset or "0", 16)
            if name in starts and starts[name] != {start}:
                raise ValueError("report named offset disagrees with function start: " + label)
            starts[name] = {start}
            resolved[(key, pc)] = (name, "report")
    return resolved


def observation(event, resolved, entry=None):
    if type(event.get("kind")) is not int or event["kind"] not in (0, 2, 3, 4):
        raise ValueError("invalid hit kind (expected Z0/Z2/Z3/Z4)")
    target = event.get("at")
    if not isinstance(target, str) or not target:
        raise ValueError("hit requires at target")
    phase = event.get("phase")
    if phase is not None and not isinstance(phase, str):
        raise ValueError("invalid phase")
    phase = phase or "?"
    pc = site(event)
    function, attrib = resolved.get(((target, event["kind"]), pc), ("?", "none"))
    if pc is not None and 0x1FC00000 <= (pc & 0x1FFFFFFF) < 0x1FC80000:
        function = "BIOS"
        attrib = "bios"
    store_offset, store_width = "", ""
    reasons = set()
    if attrib == "coverage_next_start_bound":
        reasons.add(attrib)
        attrib = "coverage"
    if "?" in target or "?" in phase:
        reasons.add("unknown_target_or_phase")
    if function == "?":
        reasons.add("unresolved_function")
    if event["kind"] == 0:
        kind = "call"
        args = event.get("args")
        if args is None or args == "?":
            value = None
        elif not isinstance(args, list) or len(args) != 4:
            raise ValueError("call args must contain a0..a3")
        else:
            value = tuple(integer(v, "argument") for v in args)
        if value is None or None in value:
            reasons.add("partial_args")
    else:
        access = event.get("access")
        if not isinstance(access, dict):
            kind = {2: "write", 3: "read", 4: "?"}[event["kind"]]
            value = None
            reasons.add("partial_access")
            store_offset = store_width = "?"
        else:
            store = access.get("store")
            if store is not None and store != "?" and type(store) is not bool:
                raise ValueError("invalid access.store")
            kind = "write" if store is True else "read" if store is False else "?"
            if event["kind"] in (2, 3):
                expected = "write" if event["kind"] == 2 else "read"
                if kind != "?" and kind != expected:
                    raise ValueError("access.store contradicts watch kind")
                kind = expected
            if store is None or store == "?":
                reasons.add("partial_access")
            for field in ("addr", "width"):
                if integer(access.get(field), "access." + field) is None:
                    reasons.add("partial_access")
            value = integer(access.get("value"), "access.value")
            if entry is not None:
                watched = integer(event.get("addr"), "addr")
                address = integer(access.get("addr"), "access.addr")
                width = integer(access.get("width"), "access.width")
                if None in (watched, address, width):
                    reasons.add("partial_access")
                    store_offset = store_width = "?"
                else:
                    if width <= 0:
                        raise ValueError("access.width must be positive")
                    watched, address = watched & 0x1FFFFFFF, address & 0x1FFFFFFF
                    length = entry["len"]
                    store_offset, store_width = address - watched, width
                    first, last = max(address, watched), min(address + width, watched + length)
                    if first >= last:
                        raise ValueError("access does not overlap watched range")
                    if value is not None:
                        value = (value >> (8 * (first - address))) & ((1 << (8 * (last - first))) - 1)
                    if first != watched or last != watched + length:
                        reasons.add("partial_store" if kind == "write" else "partial_read")
        if value is None:
            reasons.add("missing_value")
    return (normalized_target(target), kind, function, phase,
            "?" if pc is None else f"0x{pc:08X}", attrib,
            str(store_offset), str(store_width)), value, reasons


def capped(values):
    # Integer/tuple sort, with unknown components last; never sort decimal text.
    ordered = sorted(values, key=lambda v: (v is None, tuple(
        (x is None, x or 0) for x in v) if isinstance(v, tuple) else (v or 0,)))
    return json.dumps(ordered[:8], separators=(",", ":")), max(0, len(ordered) - 8)


def index_inputs(runtime):
    """Read runtime inputs in one binary cat-file batch, preserving blob bytes."""
    tracked = subprocess.run(["git", "ls-files", "-z"], cwd=REPO_ROOT,
                             check=True, capture_output=True).stdout.decode("utf-8").split("\0")
    prefix = runtime.relative_to(REPO_ROOT).as_posix() + "/"
    names = sorted(n for n in tracked if n.startswith(prefix) and
                   (n.endswith(("/capture.manifest.json", "/capture.jsonl", "/report.md"))
                    or n == prefix + "matrix/coverage.tsv"))
    if not names:
        return {}
    raw = subprocess.run(["git", "cat-file", "--batch"], cwd=REPO_ROOT,
                         input="".join(":" + n + "\n" for n in names).encode("utf-8"),
                         check=True, capture_output=True).stdout
    stream = io.BytesIO(raw)
    blobs = {}
    for name in names:
        header = stream.readline().split()
        if len(header) != 3 or header[1] != b"blob":
            raise ValueError("missing index blob: " + name)
        size = int(header[2])
        blobs[REPO_ROOT / name] = stream.read(size)
        if len(blobs[REPO_ROOT / name]) != size or stream.read(1) != b"\n":
            raise ValueError("invalid index blob: " + name)
    return blobs


def render(runtime=None):
    runtime = Path(runtime) if runtime is not None else REPO_ROOT / "docs/naming/runtime"
    blobs = index_inputs(runtime)
    groups = {}
    coverage = coverage_rows(blobs.get(runtime / "matrix/coverage.tsv"))
    functions = coverage_ranges(coverage)
    for manifest_path in sorted(p for p in blobs if p.name == "capture.manifest.json"):
        try:
            relative = manifest_path.parent.relative_to(runtime)
        except ValueError:
            continue
        try:
            manifest = json.loads(blobs[manifest_path])
            if manifest.get("q124_complete") is not True:
                continue
            raw = blobs[manifest_path.with_name("capture.jsonl")]
            digest = hashlib.sha1(raw).hexdigest()
            events = []
            for line, text in enumerate(raw.decode("utf-8").splitlines(), 1):
                try:
                    event = json.loads(text)
                    if not isinstance(event, dict) or not isinstance(event.get("ev"), str):
                        raise ValueError("expected event object with ev")
                    # Validate hits before interpreting report projections.
                    if event["ev"] == "hit":
                        observation(event, {})
                    events.append(event)
                except ValueError as ex:
                    raise ValueError(f"capture.jsonl:{line}: {ex}") from ex
            starts = [e for e in events if e["ev"] == "start"]
            if len(starts) != 1:
                raise ValueError("expected exactly one start record")
            spec = starts[0].get("spec", {})
            if modified(spec, events):
                continue
            declared = declarations(spec)
            if not any(selected(e, declared) for e in events):
                continue
            resolved = report_sites(blobs.get(manifest_path.with_name("report.md")), events, digest, coverage)
            resolve_ranges(events, resolved, functions)
            ends = [e for e in events if e["ev"] == "end"]
            if len(ends) > 1:
                raise ValueError("multiple end records")
            end = ends[0] if ends else None
            capture = relative.as_posix() + "@" + digest[:12]
            for event in events:
                if not selected(event, declared):
                    continue
                entry = declared[(normalized_target(event["at"]), event["kind"])]
                key, value, reasons = observation(event, resolved, entry)
                reasons.update(limits(spec, events, end,
                                      declared[(normalized_target(event["at"]), event["kind"])],
                                      event))
                group = groups.setdefault((capture, *key), dict(count=0, values=set(),
                                                     captures=set(), reasons=set()))
                group["count"] += 1
                group["values"].add(value)
                group["captures"].add(capture)
                group["reasons"].update(reasons)
        except (OSError, ValueError, KeyError, TypeError) as ex:
            raise ValueError(f"{manifest_path.parent}: {ex}") from ex
    out = io.StringIO(newline="\n")
    writer = csv.writer(out, delimiter="\t", lineterminator="\n", quoting=csv.QUOTE_NONE,
                        quotechar=None)
    def write_row(cells):
        if any("\t" in str(cell) or "\n" in str(cell) or "\r" in str(cell) for cell in cells):
            raise ValueError("tab/newline in TSV cell")
        writer.writerow(cells)
    write_row(HEADER)
    unresolved = {(key[0], key[1], key[2], key[4]) for key in groups if key[3] == "?"}
    for key, group in sorted(groups.items()):
        reasons = group["reasons"]
        if (key[0], key[1], key[2], key[4]) in unresolved:
            reasons.add("group_has_unresolved_site")
        write_row([*key[1:], group["count"], *capped(group["values"]),
                         ";".join(sorted(group["captures"])),
                         int(bool(reasons - {"partial_store", "partial_read", "coverage_next_start_bound"})),
                         ";".join(sorted(reasons))])
    return out.getvalue()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=["build", "check"])
    parser.add_argument("--runtime", default="docs/naming/runtime")
    parser.add_argument("--out", default="docs/naming/runtime/evidence/events.tsv")
    args = parser.parse_args(argv)
    try:
        content = render(REPO_ROOT / args.runtime).encode("utf-8")
        out = REPO_ROOT / args.out
        if args.command == "check":
            if not out.is_file() or out.read_bytes() != content:
                raise ValueError("events differ: " + str(out))
        else:
            out.parent.mkdir(parents=True, exist_ok=True)
            out.write_bytes(content)
    except (OSError, ValueError, csv.Error, subprocess.CalledProcessError) as ex:
        print("events: " + str(ex), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
