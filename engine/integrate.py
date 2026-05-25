"""Integrated (full-binary) verification — the REAL correctness gate.

The sandbox score guides the loop; this confirms a config/source change actually
preserves the byte-identical build. For redundant-rule retirement: delete a
function's rules from the REAL configs, rebuild the affected file + relink, and
check the final SHA1 == oracle. Configs are backed up and rolled back on any
mismatch, so a sandbox false-zero (a masked control-flow target hiding a real
diff) can never land a broken build — the agent's loop sees the sandbox score,
but nothing commits without this gate passing.
"""
from __future__ import annotations

from pathlib import Path

from . import buildconfig as cfg
from . import cheats
from . import pipeline as P
from . import sandbox

CONFIGS = [cheats.REGFIX, cheats.REGFIX2, cheats.ASMFIX]


def _backup(paths: list[str]) -> dict[str, bytes]:
    # byte-exact (no text-mode line-ending munging)
    return {p: Path(p).read_bytes() for p in paths if Path(p).exists()}


def _restore(backup: dict[str, bytes]) -> None:
    for p, data in backup.items():
        Path(p).write_bytes(data)


def _rebuild_file_and_sha1(stem: str) -> str:
    P.build_c_object(stem, f"build/src/{stem}.o")
    P.link("build")
    P.make_exe("build")
    return P.sha1("build/bb2.exe")


def retire_function(func: str) -> dict:
    """Delete every rule keyed by `func` from the real configs, rebuild its file
    + relink, verify SHA1 == oracle. Rolls back (configs + .o + binary) on
    mismatch so build/ always ends in the canonical, matching state.
    """
    stem = sandbox.func_file(func)
    backup = _backup(CONFIGS)
    try:
        dropped = {}
        for c in CONFIGS:
            txt, d = cheats._filter_text(func, "all", c)
            if d:
                Path(c).write_bytes(txt.encode())  # byte-exact, no newline munging
            dropped[Path(c).name] = d
        total = sum(dropped.values())
        if total == 0:
            return {"func": func, "file": stem, "ok": False,
                    "reason": "no rules keyed by this function", "dropped": dropped}
        sha1 = _rebuild_file_and_sha1(stem)
        ok = sha1 == cfg.ORACLE_SHA1
        if not ok:
            _restore(backup)
            _rebuild_file_and_sha1(stem)  # restore build/ to canonical
        return {"func": func, "file": stem, "ok": ok,
                "dropped": dropped, "total_dropped": total, "sha1": sha1}
    except Exception:
        # Crash-safety: ANY failure mid-retire must restore configs + build/
        # so a partial edit never leaks (the bug that silently dropped 24
        # asmfix rules during validation).
        _restore(backup)
        try:
            _rebuild_file_and_sha1(stem)
        except Exception:
            pass
        raise
