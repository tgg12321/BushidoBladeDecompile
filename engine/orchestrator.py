"""Deterministic controller — Phase 2a: autonomous redundant-rule retirement.

scan_file(stem): ONE build with all rules stripped, then score every keyed
function in the file. distance 0 == that function's rules are redundant (the C
already compiles to the right bytes without them). Because regfix rules are
function-scoped, the all-stripped build gives each function the same bytes it
would have with only its own rules removed — so one build scores the whole file.

retire_redundant(funcs): for each distance-0 candidate, integrate.retire_function
deletes its rules, rebuilds, and confirms the full-build SHA1 still matches. The
integrated SHA1 is the real gate; a sandbox false-zero cannot land a broken
build (it rolls back).

This is the no-model first rung. Model-driven gap-closing (nonzero distances) is
a later layer.
"""
from __future__ import annotations

from . import cheats
from . import fixtures as fx
from . import integrate
from . import pipeline as P
from . import score


def _keyed_by_file(idx: dict[str, str]) -> dict[str, list[str]]:
    byfile: dict[str, list[str]] = {}
    for func in cheats.all_keyed_functions():
        stem = idx.get(func)
        if stem:
            byfile.setdefault(stem, []).append(func)
    return byfile


def scan_file(stem: str, idx: dict | None = None, workdir: str = "tmp/scan") -> dict:
    """{func: distance} for every keyed function in `stem` from one stripped build."""
    idx = idx if idx is not None else fx._file_index()
    funcs = sorted(f for f in cheats.all_keyed_functions() if idx.get(f) == stem)
    if not funcs:
        return {}
    ov = cheats.empty_overrides(f"{workdir}/{stem}/cfg")
    stripped_o = f"{workdir}/{stem}/{stem}.o"
    P.build_c_object(stem, stripped_o, cheat_overrides=ov)
    ref_o = f"build/src/{stem}.o"
    out = {}
    for func in funcs:
        try:
            out[func] = score.score_func(stripped_o, ref_o, func)["score"]
        except KeyError:
            out[func] = None  # keyed symbol that isn't a function in this .o
    return out


def scan_all() -> dict:
    idx = fx._file_index()
    byfile = _keyed_by_file(idx)
    return {stem: scan_file(stem, idx=idx) for stem in sorted(byfile)}


def redundant_in(scan_result: dict) -> list[str]:
    """Functions whose rules are redundant (distance 0), flattened + sorted."""
    out = []
    for per_func in scan_result.values():
        out += [f for f, d in per_func.items() if d == 0]
    return sorted(out)


def retire_redundant(funcs: list[str]) -> list[dict]:
    return [integrate.retire_function(f) for f in funcs]
