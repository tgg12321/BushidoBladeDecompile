#!/usr/bin/env python3
"""Extract a label range from asm/funcs/<func>.s and emit asmfix
delete_between + insert_before rules to replace mine's slice with target's
exact asm.

Used as a last-resort: when GCC's scheduler / register allocator decisions
cannot be reproduced from C-level options + permuter + regfix, lift the
target's asm for that block directly. Wraps the manual slice-extraction +
escaping that bit func_8006BD28's inner loop.

Usage:
    python3 tools/asmfix_slice.py <func> <start_label> <end_label> [--apply]

The slice is target's `asm/funcs/<func>.s` content from start_label
(inclusive) to end_label (exclusive). The tool emits:

  - rename rules to remap mine's GCC-numbered labels to target's address-
    keyed labels (so delete_between's anchors match in the live build)
  - delete_between "^<start>:$" "^<end>:$"
  - insert_before "^<end>:$" "<slice>"

Without --apply, dry-run prints the rules. With --apply, they are appended
to asmfix.txt.

Limitations:
  - Auto-rename is best-effort: only labels at positions where mine and
    target agree on the surrounding instruction count are remapped. If
    you have additional branches into the slice from outside, you may
    need to add rename rules manually.
  - The tool does not validate against a live build (asmfix has no
    pre-pipeline simulation). Run `make` after --apply.
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# splat line: "    /* X X X */  mnemonic     operands"
SPLAT_INSN_RE = re.compile(
    r'^\s*/\*[^*]*\*/\s*(?P<mn>\w+)(?:\s+(?P<ops>.*?))?\s*$'
)
# Label line in splat: ".L8006BDD4:" or "glabel func_..."
SPLAT_LABEL_RE = re.compile(r'^\s*(?P<label>\.L[0-9A-Fa-f]+):\s*$')


def parse_target_asm(func: str) -> tuple[list[tuple[str, str]], dict[str, int]]:
    """Parse asm/funcs/<func>.s.

    Returns (entries, label_idx) where:
      entries: list of (kind, content) where kind is 'label' or 'insn'
      label_idx: {label_name: instruction_index_just_after_label}
    """
    path = ROOT / "asm" / "funcs" / f"{func}.s"
    if not path.exists():
        raise FileNotFoundError(f"asm/funcs/{func}.s not found")

    entries: list[tuple[str, str]] = []
    label_idx: dict[str, int] = {}
    insn_count = 0
    in_func = False
    glabel_re = re.compile(rf'^\s*glabel\s+{re.escape(func)}\b')

    for line in path.read_text().splitlines():
        if not in_func:
            if glabel_re.match(line):
                in_func = True
            continue
        # Inside the function. Stop at the next glabel (different function).
        if re.match(r'^\s*glabel\s+', line) and not glabel_re.match(line):
            break
        m = SPLAT_LABEL_RE.match(line)
        if m:
            entries.append(('label', m.group('label')))
            label_idx[m.group('label')] = insn_count
            continue
        m = SPLAT_INSN_RE.match(line)
        if m:
            mn = m.group('mn')
            ops_raw = m.group('ops')
            if ops_raw:
                ops = re.sub(r'\s+', ' ', ops_raw).strip().replace(', ', ',')
                entries.append(('insn', f"{mn}\t{ops}"))
            else:
                entries.append(('insn', mn))
            insn_count += 1

    return entries, label_idx


def parse_live_labels(func: str) -> dict[str, int]:
    """Run dump_text_indices and return {label_name: idx_just_after_label}."""
    res = subprocess.run(
        [sys.executable, str(ROOT / "tools" / "dump_text_indices.py"), func],
        capture_output=True, text=True, cwd=str(ROOT),
    )
    label_idx: dict[str, int] = {}
    pending_labels: list[str] = []
    for line in res.stdout.splitlines():
        # ":" prefix lines are labels/pseudo-ops in dump_text_indices
        m = re.match(r'\s*:\s*(\.L\w+):\s*$', line)
        if m:
            pending_labels.append(m.group(1))
            continue
        m = re.match(r'\s*(\d+):\s*', line)
        if m:
            idx = int(m.group(1))
            for lbl in pending_labels:
                label_idx[lbl] = idx
            pending_labels.clear()
    return label_idx


def labels_in_slice(entries: list[tuple[str, str]],
                    start_idx: int, end_idx: int) -> set[str]:
    """Return labels defined within [entries[start_idx], entries[end_idx])."""
    out: set[str] = set()
    for kind, content in entries[start_idx:end_idx]:
        if kind == 'label':
            out.add(content)
    return out


def labels_referenced_in_slice(entries: list[tuple[str, str]],
                                start_idx: int, end_idx: int) -> set[str]:
    """Return labels referenced (as branch/jump targets) within the slice."""
    out: set[str] = set()
    label_ref_re = re.compile(r'(\.L[0-9A-Fa-f]+)\b')
    for kind, content in entries[start_idx:end_idx]:
        if kind == 'insn':
            for m in label_ref_re.finditer(content):
                out.add(m.group(1))
    return out


def find_entry_indices(entries: list[tuple[str, str]],
                       start_label: str, end_label: str) -> tuple[int, int]:
    """Find entry positions (inclusive) of start_label and end_label."""
    start_pos = end_pos = -1
    for i, (k, v) in enumerate(entries):
        if k == 'label' and v == start_label:
            start_pos = i
        elif k == 'label' and v == end_label:
            end_pos = i
            break
    if start_pos < 0:
        raise ValueError(f"start label {start_label!r} not found")
    if end_pos < 0:
        raise ValueError(f"end label {end_label!r} not found")
    if end_pos <= start_pos:
        raise ValueError(f"end label appears before start label")
    return start_pos, end_pos


def build_slice_text(entries: list[tuple[str, str]],
                     start_pos: int, end_pos: int) -> str:
    """Build the multi-line asm body for insert_before, escaped for asmfix."""
    lines: list[str] = []
    for kind, content in entries[start_pos:end_pos]:
        if kind == 'label':
            lines.append(f"{content}:")
        else:
            lines.append(content)
    # asmfix decodes \n -> newline. Use literal \n in the file.
    return "\\n".join(lines)


def detect_renames(target_label_idx: dict[str, int],
                   live_label_idx: dict[str, int],
                   needed: set[str]) -> list[tuple[str, str]]:
    """For each needed target label, find the live build's label at the same idx.

    Returns list of (live_label, target_label) pairs to rename.
    """
    # invert live: idx -> set of labels at that idx
    live_at: dict[int, set[str]] = {}
    for lbl, idx in live_label_idx.items():
        live_at.setdefault(idx, set()).add(lbl)

    out: list[tuple[str, str]] = []
    for tgt_label in needed:
        if tgt_label in live_label_idx:
            continue  # already exists in mine, no rename needed
        tgt_idx = target_label_idx.get(tgt_label)
        if tgt_idx is None:
            continue
        live_labels = live_at.get(tgt_idx)
        if not live_labels:
            continue
        # Pick the lexicographically-smallest live label as the rename source
        # (typically `.L<low_N>` which is GCC's first auto-numbered label
        # at that position).
        live_label = sorted(live_labels)[0]
        out.append((live_label, tgt_label))
    return out


def append_batch(rules: list[str], func: str) -> int:
    """Append rules to asmfix.txt."""
    target = ROOT / "asmfix.txt"
    original = target.read_bytes() if target.exists() else b""
    text = original.decode("utf-8") if original else ""
    if text and not text.endswith("\n"):
        text += "\n"
    text += f"\n# {func}: asmfix-slice replacement (auto-generated by asmfix_slice.py)\n"
    for r in rules:
        text += r + "\n"
    target.write_bytes(text.encode("utf-8"))
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("start_label",
                    help="Label that begins the slice (exists in target's asm/funcs/<func>.s)")
    ap.add_argument("end_label",
                    help="Label that ends the slice (slice goes up to but not including end_label)")
    ap.add_argument("--rename", action="append", default=[],
                    metavar="OLD=NEW",
                    help="Explicit rename hint (use when auto-detect fails). "
                         "Can be repeated. Example: --rename .L766=.L8006BDD4")
    ap.add_argument("--apply", action="store_true",
                    help="Append rules to asmfix.txt; default is dry-run")
    args = ap.parse_args()

    explicit_renames: list[tuple[str, str]] = []
    for r in args.rename:
        if "=" not in r:
            print(f"asmfix-slice: ERROR: --rename {r!r} must be OLD=NEW",
                  file=sys.stderr)
            return 1
        old, new = r.split("=", 1)
        explicit_renames.append((old.strip(), new.strip()))

    try:
        entries, target_label_idx = parse_target_asm(args.func)
    except FileNotFoundError as e:
        print(f"asmfix-slice: ERROR: {e}", file=sys.stderr)
        return 1

    try:
        start_pos, end_pos = find_entry_indices(entries, args.start_label, args.end_label)
    except ValueError as e:
        print(f"asmfix-slice: ERROR: {e}", file=sys.stderr)
        return 1

    # Determine which labels referenced in the slice are NOT defined in the
    # slice — those need to exist (via rename) in mine for branches to resolve.
    defined = labels_in_slice(entries, start_pos, end_pos)
    referenced = labels_referenced_in_slice(entries, start_pos, end_pos)
    boundary_anchors = {args.start_label, args.end_label}
    needed_in_mine = (referenced - defined) | boundary_anchors

    live_label_idx = parse_live_labels(args.func)
    auto_renames = detect_renames(target_label_idx, live_label_idx, needed_in_mine)
    # Drop auto-renames that conflict with explicit hints
    explicit_targets = {tgt for _, tgt in explicit_renames}
    explicit_sources = {src for src, _ in explicit_renames}
    auto_renames = [(s, t) for s, t in auto_renames
                    if t not in explicit_targets and s not in explicit_sources]
    renames = explicit_renames + auto_renames

    slice_body = build_slice_text(entries, start_pos, end_pos)
    rules: list[str] = []
    for live_lbl, tgt_lbl in renames:
        rules.append(f'{args.func}: rename "{live_lbl}" "{tgt_lbl}"')
    rules.append(
        f'{args.func}: delete_between "^{re.escape(args.start_label)}:$" '
        f'"^{re.escape(args.end_label)}:$"'
    )
    rules.append(
        f'{args.func}: insert_before "^{re.escape(args.end_label)}:$" "{slice_body}"'
    )

    print(f"# asmfix-slice for {args.func}: {args.start_label} .. {args.end_label}")
    print(f"# Slice contains {end_pos - start_pos} entries "
          f"({sum(1 for k, _ in entries[start_pos:end_pos] if k == 'insn')} instructions, "
          f"{len(defined)} labels)")
    if renames:
        print(f"# Auto-detected {len(renames)} rename(s) for boundary/external labels:")
        for src, dst in renames:
            print(f"  rename {src!r} -> {dst!r}")
    unmatched = needed_in_mine - {tgt for _, tgt in renames} - set(live_label_idx)
    if unmatched:
        # Build inverse: idx -> labels in mine, sorted, for nearby-label hints.
        live_at: dict[int, set[str]] = {}
        for lbl, idx in live_label_idx.items():
            live_at.setdefault(idx, set()).add(lbl)
        live_indices = sorted(live_at)
        print(f"# WARNING: {len(unmatched)} referenced label(s) have no obvious source in mine:")
        for lbl in sorted(unmatched):
            tgt_idx = target_label_idx.get(lbl)
            # Nearest mine labels
            nearby = []
            if tgt_idx is not None and live_indices:
                # Take 3 mine labels closest to tgt_idx
                ranked = sorted(live_indices, key=lambda i: abs(i - tgt_idx))[:3]
                for mi in ranked:
                    for ml in sorted(live_at[mi]):
                        nearby.append(f"{ml}@{mi}")
            print(f"  {lbl} (target idx {tgt_idx}) — nearest mine labels: {', '.join(nearby) if nearby else '(none)'}")
            print(f"    Hint: --rename {nearby[0].split('@')[0] if nearby else '<your_label>'}={lbl}")
    print()
    print(f"# Proposed {len(rules)} asmfix rule(s):")
    for r in rules:
        # Truncate long insert_before for legibility
        if len(r) > 240:
            print(f"  {r[:200]}... [truncated, {len(r)} chars total]")
        else:
            print(f"  {r}")

    if not args.apply:
        print()
        print("# Dry-run. Re-run with --apply to append to asmfix.txt.")
        return 0

    append_batch(rules, args.func)
    print(f"asmfix-slice: appended {len(rules)} rule(s) to asmfix.txt. "
          f"Run `make` to verify.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
