#!/usr/bin/env python3
"""List COMPLETED-C functions (the committed pure-C matches) for the fleet's
re-audit patrol. Completed = (every function defined in build/src/*.o) minus
(functions still in engine/queue.json = INCOMPLETE) minus (canonical-asm
functions, which are authorized rather than pure-C). One name per line on stdout.

Run from the repo root inside WSL with the venv active (seed_reaudit.ps1 does this).
Best-effort: prints nothing and exits 0 if the engine internals are unavailable,
so the fleet still re-audits its own fresh completions.
"""
from __future__ import annotations

import json
import os
import sys
from pathlib import Path


def main() -> int:
    try:
        sys.path.insert(0, os.getcwd())  # repo root, so `engine` package imports
        from engine import pipeline as P
        from engine import score
        from engine import cheats
    except Exception as e:  # pragma: no cover
        print(f"# list_completed: engine import failed: {e}", file=sys.stderr)
        return 0

    queue_funcs: set[str] = set()
    qp = Path("engine/queue.json")
    if qp.exists():
        try:
            for it in json.loads(qp.read_text()).get("items", []):
                queue_funcs.add(it["func"])
        except Exception as e:
            print(f"# list_completed: queue read failed: {e}", file=sys.stderr)

    try:
        canon = set(cheats.canonical_asm_funcs())
    except Exception:
        canon = set()

    all_funcs: set[str] = set()
    for stem in P.c_stems():
        o = f"build/src/{stem}.o"
        if not Path(o).exists():
            continue
        try:
            all_funcs.update(score._o_func_table(o).keys())
        except Exception:
            continue

    import re
    is_data = re.compile(r"^D_[0-9A-Fa-f]+$")  # data labels, not functions
    completed = sorted(f for f in all_funcs
                       if f not in queue_funcs and f not in canon and not is_data.match(f))
    for f in completed:
        print(f)
    print(f"# list_completed: {len(completed)} completed-C functions", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
