#!/usr/bin/env python3
"""Build/check per-TU runtime evidence, without inferring function roles.

All relative CLI paths are repository-relative. Capture and phase identifiers
are copied verbatim; summaries describe only the supplied facts.
"""

import argparse
import csv
import io
import json
from pathlib import Path
import re
import sys

REPO_ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(REPO_ROOT))
from engine.tus import ld_sections
from tools.trace.scan.facts import read_table

HEADER = ["addr", "capture_name", "no_name", "status", "summary", "captures",
          "inconclusive"]
KINDS = {"reach", "never_hit", "phase_only", "capture_only"}


def tu_start(tu, src):
    basename = tu.rsplit("/", 1)[-1]
    # ROM-offset filenames use uppercase hex. Lowercase SDK module names
    # such as c67 and a08 happen to be hex strings, but are not ROM offsets.
    if re.fullmatch(r"[0-9A-F]+", basename):
        return 0x80010000 + int(basename, 16) - 0x800
    path = Path(src) / (tu + ".c")
    if not path.is_file():
        return None
    comment = re.match(r"\s*/\*(.*?)\*/", path.read_text(encoding="utf-8"), re.S)
    if comment:
        match = re.search(r"\.text\b.*?(0x8[0-9A-Fa-f]{7})\b", comment[1], re.S)
        if match:
            return int(match[1], 16)
    return None


def tu_range(tu, ld, src):
    order = ld_sections(ld).get(".text", [])
    if tu not in order:
        raise ValueError("TU not in .text order: " + tu)
    start = tu_start(tu, src)
    if start is None:
        raise ValueError("unresolvable TU: " + tu)
    following = order[order.index(tu) + 1:]
    end = tu_start(following[0], src) if following else None
    # Skipping an unresolvable neighbour would file its functions under this TU.
    if following and end is None:
        raise ValueError("unresolvable TU after " + tu + ": " + following[0])
    if end is not None and end <= start:
        raise ValueError("non-increasing TU range: " + tu)
    return start, end


def source_hashes(path):
    _, rows = read_table(path, ["capture", "input_file", "input_sha1"])
    hashes = {}
    for row in rows:
        if row["input_file"] != "capture.jsonl":
            continue
        capture, digest = row["capture"], row["input_sha1"].lower()
        if not re.fullmatch(r"[0-9a-f]{40}", digest):
            raise ValueError("invalid capture.jsonl SHA1: " + capture)
        if capture in hashes and hashes[capture] != digest:
            raise ValueError("conflicting capture.jsonl SHA1: " + capture)
        hashes[capture] = digest
    return hashes


def function_note(rows, hashes):
    details = {r["kind"]: json.loads(r["fact"]) for r in rows}
    reach = details.get("reach")
    phase = details.get("phase_only")
    parts = []
    if reach is None:
        status = "untargeted"
        parts.append("no reach fact")
    else:
        targeted, hit = reach["targeted"], reach["hit"]
        comparison = details.get("capture_only", {})
        unknown = reach.get("unknown", {})
        quiet = comparison.get("armed_never_hit", [
            c for c in targeted if c not in hit and c not in unknown])
        # A definite status needs captures that show it: never_hit only when
        # every targeted capture is all `.`, capture_only only with at least
        # one all-`.` capture; anything else resting on `?` cells is unknown.
        if not targeted:
            status = "untargeted"
        elif set(hit) == set(targeted):
            status = "hit_all"
        elif not hit:
            status = "never_hit" if set(quiet) == set(targeted) else "unknown"
        else:
            status = "capture_only" if quiet else "unknown"
        name_hit = len(hit) <= len(quiet)
        hit_text = f"hit in {len(hit)}/{len(targeted)} captures"
        if name_hit and hit:
            hit_text += " (" + ", ".join(hit) + ")"
        parts.append(hit_text)
        if quiet:
            text = f"armed, never hit in {len(quiet)}"
            if not name_hit:
                text += " (" + ", ".join(quiet) + ")"
            parts.append(text)
    if phase is not None:
        status += "+phase_only"
        parts.append("phases: " + "; ".join(
            f"{capture} {','.join(phases) if phases else '(none)'} "
            f"of {len(phase['covered_phases'][capture])} covered"
            for capture, phases in phase["hit_phases"].items()))
    # Reach records unknown cells; capture_only also records captures without hits.
    unknown_cells = (reach or {}).get("unknown", {})
    if unknown_cells:
        parts.append("unknown (?) cells: " + "; ".join(
            c + " " + ",".join(p) for c, p in unknown_cells.items()))
    unknown_captures = details.get("capture_only", {}).get("unknown", [])
    if unknown_captures:
        parts.append("unknown (?) captures: " + ", ".join(unknown_captures))
    captures = sorted({c for r in rows for c in r["captures"].split(";") if c})
    for capture in captures:
        if capture not in hashes:
            raise ValueError("missing capture.jsonl source: " + capture)
    inconclusive = any(r["inconclusive"] == "1" for r in rows)
    reasons = sorted({json.loads(r["fact"])["reason"] for r in rows
                      if r["inconclusive"] == "1" and json.loads(r["fact"]).get("reason")})
    if reasons:
        parts.append("inconclusive: " + ", ".join(reasons))
    first = rows[0]
    if any((r["capture_name"], r["no_name"]) !=
           (first["capture_name"], first["no_name"]) for r in rows):
        raise ValueError("conflicting function identity: " + first["addr"])
    return dict(addr=first["addr"], capture_name=first["capture_name"],
                no_name=first["no_name"], status=status, summary="; ".join(parts),
                captures=";".join(c + "@" + hashes[c][:12] for c in captures),
                inconclusive=str(int(inconclusive)))


def render(tu, facts, sources, ld, src):
    start, end = tu_range(tu, ld, src)
    _, rows = read_table(facts, ["kind", "addr", "capture_name", "no_name",
                                "fact", "captures", "inconclusive"])
    groups = {}
    for row in rows:
        if row["kind"] not in KINDS:
            continue
        addr = int(row["addr"], 16)
        if start <= addr and (end is None or addr < end):
            groups.setdefault(addr, []).append(row)
    hashes = source_hashes(sources)
    stream = io.StringIO(newline="\n")
    writer = csv.DictWriter(stream, HEADER, delimiter="\t", lineterminator="\n")
    writer.writeheader()
    writer.writerows(function_note(groups[addr], hashes) for addr in sorted(groups))
    return stream.getvalue()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=["build", "check"])
    parser.add_argument("--tu", action="append", default=[])
    parser.add_argument("--facts", default="docs/naming/runtime/matrix/facts.tsv")
    parser.add_argument("--sources", default="docs/naming/runtime/matrix/sources.tsv")
    parser.add_argument("--out-dir", default="docs/naming/runtime/evidence")
    parser.add_argument("--ld", default="bb2.ld")
    parser.add_argument("--src", default="src")
    args = parser.parse_args(argv)
    try:
        out_dir = REPO_ROOT / args.out_dir
        tus = args.tu if args.command == "build" else [
            p.stem.replace("__", "/") for p in sorted(out_dir.glob("*.tsv"))]
        if args.command == "build" and not tus:
            raise ValueError("build requires --tu")
        for tu in tus:
            if not re.fullmatch(r"[A-Za-z0-9_/-]+", tu) or ".." in tu or tu.startswith("/"):
                raise ValueError("invalid TU id: " + tu)
            content = render(tu, REPO_ROOT / args.facts, REPO_ROOT / args.sources,
                             REPO_ROOT / args.ld, REPO_ROOT / args.src).encode("utf-8")
            out = out_dir / (tu.replace("/", "__") + ".tsv")
            if args.command == "check":
                if out.read_bytes() != content:
                    raise ValueError("notes differ: " + str(out))
            else:
                out.parent.mkdir(parents=True, exist_ok=True)
                out.write_bytes(content)
    except (OSError, ValueError, KeyError, TypeError, csv.Error) as ex:
        print("notes: " + str(ex), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
