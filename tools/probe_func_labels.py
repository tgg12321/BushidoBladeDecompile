#!/usr/bin/env python3
"""Print the cc1 `.L<N>` labels a function emits, in document order, 1-indexed.

This is the tool `.claude/rules/global-label-drift-sibling-cheat.md` and
`.claude/rules/canonical-asm-authorization-recipe.md` both reference as
`bash tmp/probe_func_labels.sh <func>` — which never existed in the tree.

Use it to migrate a hardcoded-`.L<N>` regfix/asmfix rule to a drift-robust
`{lbl#N}` slot: the slot index is the label's 1-indexed position HERE. Slots are
function-local, so they survive any TU-wide renumbering caused by adding or
removing an earlier function's body.

The stream is reconstructed exactly as `tools/asmfix.py` sees it — the full
per-file pipeline truncated immediately BEFORE the asmfix stage — so the
positions printed are the ones `{lbl#N}` will resolve against.

    python3 tools/probe_func_labels.py SetPacketData
    python3 tools/probe_func_labels.py mk_leaf_newpos --map .L631
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

from engine import pipeline, score  # noqa: E402

_LABEL_DEF = re.compile(r'^\s*(\.L\d+):\s*$', re.MULTILINE)


def stem_of(func: str) -> str:
    for o in sorted(Path("build/src").glob("*.o")):
        try:
            if func in score._o_func_table(str(o)):
                return o.stem
        except Exception:
            continue
    raise SystemExit(f"{func}: not found in any build/src/*.o (run `engine build` first)")


def stream_before_asmfix(stem: str) -> str:
    """The assembly text as it reaches tools/asmfix.py."""
    cmd = pipeline.c_pipeline_cmd(stem, "/dev/null")
    stages = cmd.split(" | ")
    keep = []
    for s in stages:
        if "asmfix.py" in s:
            break
        keep.append(s)
    r = subprocess.run(" | ".join(keep), shell=True, cwd=ROOT,
                       capture_output=True, text=True)
    if not r.stdout:
        raise SystemExit(f"empty stream for {stem}\nSTDERR:\n{r.stderr[:2000]}")
    return r.stdout


def func_block(text: str, func: str) -> str:
    """asmfix's own notion of a function block: `<func>:` .. `.end <func>`."""
    m = re.search(rf'(?m)^{re.escape(func)}:\s*$', text)
    if not m:
        raise SystemExit(f"{func}: no `{func}:` label in the pre-asmfix stream")
    end = re.search(rf'(?m)^\s*\.end\s+{re.escape(func)}\s*$', text[m.start():])
    return text[m.start():m.start() + end.end()] if end else text[m.start():]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("func")
    ap.add_argument("--map", action="append", default=[],
                    help="a hardcoded .L<N> to map to its slot; repeatable")
    a = ap.parse_args()

    stem = stem_of(a.func)
    labels = _LABEL_DEF.findall(func_block(stream_before_asmfix(stem), a.func))

    print(f"{a.func}  (src/{stem}.c)  {len(labels)} cc1 label(s), document order:")
    for i, lab in enumerate(labels, 1):
        marker = "   <--" if lab in a.map else ""
        print(f"  {{lbl#{i}}}  {lab}{marker}")

    if a.map:
        print("\nmigration mapping:")
        for want in a.map:
            if want in labels:
                print(f"  {want}  ->  {{lbl#{labels.index(want) + 1}}}")
            else:
                print(f"  {want}  ->  NOT EMITTED by {a.func} — the rule does not "
                      f"reference one of this function's own labels; a slot "
                      f"reference cannot express it. Investigate before migrating.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
