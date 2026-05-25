"""Cheat-invisible scoring sandbox — the orchestrator's BASELINE/scoring step.

sandbox_score(func, disable) builds the function's containing file with that
function's cheat rules removed (via cheats.make_overrides) into a per-function
workspace, then scores it against the canonical cheat-on .o. The result is the
honest, cheat-free codegen distance: re-adding a cheat cannot improve it because
the orchestrator owns the filtered config the sandbox compiles against — the
agent only edits C.
"""
from __future__ import annotations

import functools
from pathlib import Path

from . import cheats
from . import fixtures as fx
from . import pipeline
from . import score


@functools.lru_cache(maxsize=1)
def _file_index_cached() -> dict[str, str]:
    return fx._file_index()


def func_file(func: str) -> str:
    stem = _file_index_cached().get(func)
    if stem is None:
        raise KeyError(f"{func}: not found in any build/src/*.o (run `engine build` first)")
    return stem


def sandbox_score(func: str, disable: str = "lost-codegen",
                  strip_tier3: bool = False, workdir: str = "tmp/sandbox") -> dict:
    stem = func_file(func)
    wd = Path(workdir) / func
    ov = cheats.make_overrides(func, disable, str(wd / "cfg"))
    tier3_stripped = 0
    if strip_tier3:
        from . import inlineasm
        src_ovr = str(wd / "src" / f"{stem}.c")
        tier3_stripped = inlineasm.write_stripped(stem, src_ovr)
        ov["src_override"] = src_ovr
    disabled_o = str(wd / f"{stem}.o")
    pipeline.build_c_object(stem, disabled_o, cheat_overrides=ov)
    reference_o = f"build/src/{stem}.o"
    if not Path(reference_o).exists():
        raise FileNotFoundError(f"{reference_o} missing — run `engine build` first")
    res = score.score_func(disabled_o, reference_o, func)
    res.update(func=func, file=stem, disable=disable, strip_tier3=strip_tier3,
               rules_dropped=ov["dropped"], tier3_stripped=tier3_stripped,
               disabled_o=disabled_o)
    return res
