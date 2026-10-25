#!/usr/bin/env python3
"""Build/check frame-stamped screen observations, never infer names or roles.

Inputs are the same index blobs and unmodified Q124 captures as events.py.
Z0 hits describe the entered function; watch hits use events.py's recorded
site resolution. Screens are the first same-phase screenshot on a frame
after each hit, not proof that the function drew that screen. Hit frames are the Lua
pause frame of the stop (gdbtrace.frame_fields): a screenshot on frame F is
taken in the Vsync handler as F starts, before any code that runs in F, so
the screen for a hit on F is the first shot on a frame after F. Boundary
screenshots run before the phase change (runkit.ACTION_ORDER); shots before the
first phase belong to runkit.first_phase. Old frameless captures emit no rows.
Mixed groups retain missing-frame diagnostics. Hit maps credit only their
configured first hits per phase, not all calls. PNG hashes are manifest facts.
"""

import argparse
import csv
import hashlib
import io
import json
from pathlib import Path, PureWindowsPath
import subprocess
import sys

REPO_ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(REPO_ROOT))
from tools.trace import runkit
from tools.trace.scan import events

HEADER = ["addr", "name", "capture", "phase", "first_frame", "last_frame",
          "hits_with_frame", "screens", "coverage", "inconclusive", "reason"]


def screenshots(spec, manifest):
    script = spec.get("script") or {}
    phases = sorted(script.get("phases", []), key=lambda p: p.get("frame", 0))
    hashes = {PureWindowsPath(o["path"]).name: o.get("sha1", "")[:12]
              for o in manifest.get("outputs", [])}
    result = []
    for shot in script.get("screenshots", []):
        frame = events.integer(shot["frame"], "screenshot frame")
        phase = runkit.first_phase(script)
        for p in phases:
            if p.get("frame", 0) < frame:
                phase = p["name"]
        name = shot["name"]
        digest = hashes.get("capture." + name + ".png", "")
        label = f"{name}@{frame}" + ("#" + digest if digest else "")
        result.append((frame, name, phase, label))
    return sorted(result)


def function(event, resolved, coverage):
    if event["kind"] == 0:
        addr = events.canonical(event["addr"])
        names = {r["capture_name"] for r in coverage
                 if events.canonical(int(r["addr"], 0)) == addr and r["capture_name"]}
        # The recorded Z0 target is already a symbol, not a new name.
        name = next(iter(names)) if len(names) == 1 else event["at"]
        return addr, name, set()
    pc = events.site(event)
    name, attrib = resolved.get(((event["at"], event["kind"]), pc), ("?", "none"))
    starts = {events.canonical(int(r["addr"], 0)) for r in coverage
              if r["capture_name"] == name}
    reasons = set()
    if name == "?":
        reasons.add("unresolved_function")
    if attrib == "coverage_next_start_bound":
        reasons.add(attrib)
    if len(starts) != 1:
        reasons.add("unknown_function_address")
    return next(iter(starts)) if len(starts) == 1 else None, name, reasons


def render(runtime=None):
    csv.field_size_limit(2**31 - 1)
    runtime = Path(runtime) if runtime is not None else REPO_ROOT / "docs/naming/runtime"
    blobs = events.index_inputs(runtime)
    coverage = events.coverage_rows(blobs.get(runtime / "matrix/coverage.tsv"))
    groups = {}
    for path in sorted(p for p in blobs if p.name == "capture.manifest.json"):
        manifest = json.loads(blobs[path])
        if manifest.get("q124_complete") is not True:
            continue
        relative = path.parent.relative_to(runtime)
        raw = blobs[path.with_name("capture.jsonl")]
        records = [json.loads(line) for line in raw.decode("utf-8").splitlines()]
        starts = [e for e in records if e.get("ev") == "start"]
        if len(starts) != 1:
            raise ValueError(str(path) + ": expected one start")
        spec = starts[0].get("spec", {})
        if events.modified(relative, spec, records):
            continue
        hits = [e for e in records if e.get("ev") == "hit"]
        if not any(events.integer(e.get("frame"), "hit frame") is not None for e in hits):
            continue
        digest = hashlib.sha1(raw).hexdigest()
        capture = relative.as_posix() + "@" + digest[:12]
        resolved = events.report_sites(blobs.get(path.with_name("report.md")),
                                       records, digest, coverage)
        events.resolve_ranges(records, resolved, events.coverage_ranges(coverage))
        shots = screenshots(spec, manifest)
        ends = [e for e in records if e.get("ev") == "end"]
        if len(ends) > 1:
            raise ValueError(str(path) + ": multiple end records")
        end = ends[0] if ends else None
        declared = events.declarations(spec)
        for hit in hits:
            if hit.get("stub_call") or hit.get("stub_leak"):
                continue
            addr, name, reasons = function(hit, resolved, coverage)
            phase = hit.get("phase") or "?"
            if phase == "?":
                reasons.add("unknown_phase")
            key = ("?" if addr is None else f"0x{addr:08X}", name, capture, phase)
            group = groups.setdefault(key, dict(frames=[], screens=set(),
                                                coverage=set(), reasons=set()))
            group["reasons"].update(reasons)
            entry = declared.get((events.normalized_target(hit["at"]), hit["kind"]), {})
            # Run limits as events.py reports them; reaching a per-phase
            # credit is the coverage column's caveat, not a defect.
            group["reasons"].update(
                r for r in events.limits(spec, records, end,
                                         entry or {"at": hit["at"]}, hit)
                if not r.startswith("per_phase_reached:"))
            if hit.get("hm"):
                credit = spec.get("hitmap", {}).get("per_phase", 1)
                group["coverage"].add(f"hitmap:first {credit} hits per phase only")
            elif "per_phase" in entry:
                group["coverage"].add(f"breakpoint:first {entry['per_phase']} hits per phase only")
            else:
                group["coverage"].add("recorded hits only; capture limits apply")
            frame = events.integer(hit.get("frame"), "hit frame")
            if frame is None:
                group["reasons"].add("hits_without_frame")
                continue
            group["frames"].append(frame)
            later = [s for s in shots if s[2] == phase and s[0] > frame]
            if later:
                group["screens"].add(later[0][3])
            else:
                group["reasons"].add("no_later_screenshot")
    out = io.StringIO(newline="\n")
    writer = csv.writer(out, delimiter="\t", lineterminator="\n",
                        quoting=csv.QUOTE_NONE, quotechar=None)
    writer.writerow(HEADER)
    for key, group in sorted(groups.items()):
        frames = group["frames"]
        if not frames:
            continue
        row = [*key, min(frames), max(frames), len(frames),
               ";".join(sorted(group["screens"])), ";".join(sorted(group["coverage"])),
               int(bool(group["reasons"])), ";".join(sorted(group["reasons"]))]
        if any(any(c in str(cell) for c in "\t\r\n") for cell in row):
            raise ValueError("tab/newline in TSV cell")
        writer.writerow(row)
    return out.getvalue()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=["build", "check"])
    parser.add_argument("--runtime", default="docs/naming/runtime")
    parser.add_argument("--out", default="docs/naming/runtime/evidence/screens.tsv")
    args = parser.parse_args(argv)
    try:
        content = render(REPO_ROOT / args.runtime).encode("utf-8")
        out = REPO_ROOT / args.out
        if args.command == "check":
            if not out.is_file() or out.read_bytes() != content:
                raise ValueError("screens differ: " + str(out))
        else:
            out.parent.mkdir(parents=True, exist_ok=True)
            out.write_bytes(content)
    except (OSError, ValueError, KeyError, TypeError, csv.Error,
            subprocess.CalledProcessError) as ex:
        print("screens: " + str(ex), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
