#!/usr/bin/env python3
"""Build/check observed facts from coverage, values and capture phase tables.

The fact column is compact JSON. Inconclusive is 0/1; reasons, when present,
are in fact.reason. Value identities are (symbol, offset, len), retained in
fact.offset and fact.len. End readings compare only with the preceding reading
of the same watch index in the same capture; missing/failed comparisons are
unknown. capture_only compares captures, without inferring modes. value_range
splits each changing reading into little-endian unsigned words (4 bytes when
len is a multiple of 4, else 2, else 1) and, for each word that takes more than
one value, gives distinct/min/max and a per-capture trend in frame order. Phase lists
follow phases.tsv run order (JSONL order for TSV-less value captures). All CLI relative
paths are repository-relative. Facts describe only the supplied captures.
Readings from kind-stub captures (stubbed or poked: a modified game) give no
value facts. A pointer read (values.tsv ptr set) describes the bytes at that
pointer: when the successful readings of one (symbol, offset, len) went
through more than one pointer they describe different objects, so they give
one inconclusive value_changes row (reason pointers_differ) listing the
pointers per capture, and no value bytes or value_range rows.
"""

import argparse
import csv
import io
import json
from pathlib import Path
import re
import sys

REPO_ROOT = Path(__file__).resolve().parents[3]
HEADER = ["kind", "addr", "capture_name", "no_name", "fact", "captures",
          "inconclusive"]


def read_table(path, required):
    # A value watch can read a whole buffer (values.tsv holds 1.5M-char
    # cells), past csv's 131072 default; 2**31-1 fits a C long on Windows.
    csv.field_size_limit(2**31 - 1)
    with Path(path).open(encoding="utf-8", newline="") as stream:
        reader = csv.DictReader(stream, delimiter="\t")
        if not reader.fieldnames or not set(required) <= set(reader.fieldnames):
            raise ValueError(str(path) + ": missing columns")
        rows = list(reader)
        if any(None in row or None in row.values() for row in rows):
            raise ValueError(str(path) + ": inconsistent row width")
        return reader.fieldnames, rows


def fact_row(kind, item, fact, captures, reason=""):
    if reason:
        fact = dict(fact, reason=reason)
    return dict(kind=kind, addr=item["addr"],
                capture_name=item.get("capture_name", ""),
                no_name=str(int(item.get("no_name", False))),
                fact=json.dumps(fact, sort_keys=True, separators=(",", ":")),
                captures=";".join(sorted(captures)), inconclusive=str(int(bool(reason))))


def hit_phases(cells):
    return [p for p, cell in cells.items() if cell[0].isdigit()]


def reach(item):
    cells = item["cells"]
    hits = {c: hit_phases(v) for c, v in cells.items() if hit_phases(v)}
    unknown = {c: [p for p, v in phases.items() if v == "?"]
               for c, phases in cells.items() if "?" in phases.values()}
    return [fact_row("reach", item, dict(targeted=sorted(cells), hit=sorted(hits),
                                         hit_phases=hits, unknown=unknown), cells,
                     "unknown_cells" if unknown else "")]


def never_hit(item):
    cells = item["cells"]
    if not cells or any(hit_phases(v) for v in cells.values()):
        return []
    unknown = any("?" in v.values() for v in cells.values())
    return [fact_row("never_hit", item, dict(targeted=sorted(cells)), cells,
                     "unknown_cells" if unknown else "")]


def phase_only(item):
    cells = item["cells"]
    hits = {c: hit_phases(v) for c, v in cells.items()}
    if not any(hits.values()) or any(len(hits[c]) == len(v) for c, v in cells.items()):
        return []
    unknown = any("?" in v.values() for v in cells.values())
    return [fact_row("phase_only", item, dict(hit_phases=hits,
                     covered_phases={c: list(v) for c, v in cells.items()}), cells,
                     "unknown_cells_outside_hit_phases" if unknown else "")]


def capture_only(item):
    cells = item["cells"]
    hit = sorted(c for c, v in cells.items() if hit_phases(v))
    quiet = sorted(c for c, v in cells.items() if all(x == "." for x in v.values()))
    unknown = sorted(set(cells) - set(hit) - set(quiet))
    if not hit or not (quiet or unknown):
        return []
    return [fact_row("capture_only", item, dict(hit=hit, armed_never_hit=quiet,
                     unknown=unknown), cells, "unknown_captures" if unknown else "")]


def value_fact(item, readings, kind):
    seen = {}
    errors = []
    previous = {}
    ends = []
    for row in readings:
        capture, phase = row["capture"], row["phase"]
        value = row["bytes"]
        error = row["error"]
        if error:
            errors.append(dict(capture=capture, phase=phase, error=error))
        else:
            seen.setdefault(value, {}).setdefault(capture, {})[phase] = None
        watch = (capture, row["entry"])
        if row["end"] == "true":
            before = previous.get(watch) if row["entry"] else None
            ends.append(dict(capture=capture, phase=phase, entry=row["entry"],
                             differs=None if error or before is None else value != before))
        previous[watch] = None if error else value
    observed_kind = "value_changes" if len(seen) > 1 else "value_constant"
    if kind != observed_kind:
        return []
    def ordered(capture, phases):
        order = item.get("phase_order", {}).get(capture, list(phases))
        return [p for p in order if p in phases] + [p for p in phases if p not in order]

    details = dict(offset=item["offset"], len=item["len"],
                   values={v: {c: ordered(c, p) for c, p in captures.items()}
                           for v, captures in seen.items()}, end=ends, errors=errors)
    reason = "read_errors" if errors else "no_successful_readings" if not seen else ""
    return [fact_row(kind, item, details, {r["capture"] for r in readings}, reason)]


def value_constant(item, readings):
    return value_fact(item, readings, "value_constant")


def value_changes(item, readings):
    return value_fact(item, readings, "value_changes")


def word_width(length):
    return 4 if length % 4 == 0 else 2 if length % 2 == 0 else 1


def trend(values):
    if len(set(values)) == 1:
        return "constant"
    if all(a <= b for a, b in zip(values, values[1:])):
        return "nondecreasing"
    if all(a >= b for a, b in zip(values, values[1:])):
        return "nonincreasing"
    return "mixed"


def value_range(item, readings):
    good = [r for r in readings if not r["error"]]
    errors = len(readings) - len(good)
    lengths = {len(r["bytes"]) // 2 for r in good}
    if len({r["bytes"] for r in good}) < 2:
        return []
    if len(lengths) != 1 or any(len(r["bytes"]) % 2 for r in good):
        raise ValueError("value bytes disagree with len: " + item["addr"])
    if any(not r["frame"].isdigit() for r in good):
        raise ValueError("value reading without a frame: " + item["addr"])
    length = lengths.pop()
    width = word_width(length)
    series = {}
    for row in sorted(good, key=lambda r: (int(r["frame"]), int(r["entry"] or 0))):
        raw = bytes.fromhex(row["bytes"])
        words = [int.from_bytes(raw[i:i + width], "little")
                 for i in range(0, length, width)]
        series.setdefault(row["capture"], []).append((row["frame"], words))
    rows = []
    for index in range(length // width):
        per = {c: [w[index] for _, w in s] for c, s in series.items()}
        seen = sorted({v for values in per.values() for v in values})
        if len(seen) < 2:
            continue
        digits = "0" + str(2 * width) + "X"
        detail = dict(offset=item["offset"], len=item["len"], word_offset=index * width,
                      width=width, distinct=len(seen), min=format(seen[0], digits),
                      max=format(seen[-1], digits),
                      trend={c: trend(v) for c, v in per.items()},
                      readings={c: len(v) for c, v in per.items()}, errors=errors)
        rows.append(fact_row("value_range", item, detail, {r["capture"] for r in readings},
                             "read_errors" if errors else ""))
    return rows


def render(matrix, no_name, runtime=None):
    matrix = Path(matrix)
    runtime = Path(runtime) if runtime is not None else REPO_ROOT / "docs/naming/runtime"
    _, registry = read_table(no_name, ["address"])
    registry = {r["address"].upper() for r in registry}
    columns, coverage = read_table(matrix / "coverage.tsv",
                                  ["addr", "capture_name", "no_name", "captures_hit"])
    captures = columns[5:]
    _, sources = read_table(matrix / "sources.tsv", ["capture", "kind", "input_file"])
    kinds = {r["capture"]: r["kind"] for r in sources}
    phases = {}
    for capture in captures:
        if kinds.get(capture) != "hitmap":
            raise ValueError("coverage capture is not a hitmap: " + capture)
        path = runtime / capture
        _, spans = read_table(path / "phases.tsv", ["phase", "first_frame", "last_frame", "screen"])
        header, _ = read_table(path / "hitmap.tsv", ["function", "addr"])
        phases[capture] = header[2:]
        if not phases[capture] or not set(phases[capture]) <= {r["phase"] for r in spans}:
            raise ValueError("unknown or missing hitmap phases: " + capture)
        phases[capture] = [r["phase"] for r in spans if r["phase"] in phases[capture]]
    rows = []
    for entry in coverage:
        cells = {}
        for capture in captures:
            text = entry[capture]
            if text == "-":
                continue
            values = dict.fromkeys(phases[capture], ".")
            if text != ".":
                for pair in text.split(";"):
                    phase, cell = pair.split(":")
                    if phase not in values or not re.fullmatch(r"\?|[1-9][0-9]*\+?", cell):
                        raise ValueError("invalid coverage cell: " + pair)
                    values[phase] = cell
            cells[capture] = values
        item = dict(addr=entry["addr"], capture_name=entry["capture_name"],
                    no_name=entry["addr"].upper() in registry, cells=cells)
        for scanner in (reach, never_hit, phase_only, capture_only):
            rows.extend(scanner(item))
    _, readings = read_table(matrix / "values.tsv",
                             ["capture", "phase", "symbol/entry", "offset", "len", "entry", "bytes", "error", "end",
                              "frame"])
    readings = [r for r in readings if kinds.get(r["capture"]) != "stub"]
    phase_order = {}
    for capture in sorted({r["capture"] for r in readings}):
        path = runtime / capture / "phases.tsv"
        if path.is_file():
            _, spans = read_table(path, ["phase"])
            phase_order[capture] = [r["phase"] for r in spans]
    groups = {}
    for row in readings:
        groups.setdefault((row["symbol/entry"], row["offset"], row["len"]), []).append(row)
    for (symbol, offset, length), readings in sorted(groups.items()):
        # The registry includes addresses, not current-tree or recorded names.
        addr = "0x" + symbol[2:] if re.fullmatch(r"D_[0-9A-Fa-f]{8}", symbol) else symbol
        item = dict(addr=symbol, offset=offset, len=length, phase_order=phase_order,
                    no_name=addr.upper() in registry)
        pointers = {}
        for row in readings:
            if not row["error"] and row.get("ptr"):
                pointers.setdefault(row["ptr"], set()).add(row["capture"])
        if len(pointers) > 1:
            rows.append(fact_row("value_changes", item, dict(
                offset=offset, len=length,
                pointers={p: sorted(c) for p, c in pointers.items()}),
                {r["capture"] for r in readings}, "pointers_differ"))
            continue
        for scanner in (value_constant, value_changes, value_range):
            rows.extend(scanner(item, readings))
    rows.sort(key=lambda r: (r["kind"], r["addr"], r["fact"], r["captures"]))
    stream = io.StringIO(newline="\n")
    writer = csv.DictWriter(stream, HEADER, delimiter="\t", lineterminator="\n")
    writer.writeheader()
    writer.writerows(rows)
    return stream.getvalue()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=["build", "check"])
    parser.add_argument("--matrix", default="docs/naming/runtime/matrix")
    parser.add_argument("--no-name", default="docs/naming/phase3/no_name.tsv")
    parser.add_argument("--out", default="docs/naming/runtime/matrix/facts.tsv")
    args = parser.parse_args(argv)
    try:
        content = render(REPO_ROOT / args.matrix, REPO_ROOT / args.no_name).encode("utf-8")
        out = REPO_ROOT / args.out
        if args.command == "check":
            if not out.is_file() or out.read_bytes() != content:
                raise ValueError("facts differ: " + str(out))
        else:
            out.parent.mkdir(parents=True, exist_ok=True)
            out.write_bytes(content)
    except (OSError, ValueError, KeyError, csv.Error) as ex:
        print("facts: " + str(ex), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
