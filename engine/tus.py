"""Translation-unit discovery: the one place that knows where C sources live.

A TU id is the path under src/ without `.c`, in posix form (`main/368E4`,
`main/psxsdk/libcomb/comb`). `src/<id>.c` and `build/src/<id>.o` are its source
and object; flat ids are just ids without a slash. Bare basenames are never an
identity: two modules may share one (`libgpu/sys`, `libcd/sys`).

The LINKED set comes from bb2.ld, never from a glob of build/src: that directory
keeps objects of retired TUs, and a sorted glob let a stale one shadow the live
owner of a function (restructure plan R5). `check()` is the bb2.ld consistency
audit (owner decision Q106 D8: bb2.ld stays hand-maintained, plus this checker).
"""
from __future__ import annotations

import re
from pathlib import Path

SRC = Path("src")
LD_SCRIPT = Path("bb2.ld")
RENAMES = Path("tools/tu_renames.tsv")

# `build/src/<id>.o(<section>);` — one input-section line of bb2.ld.
_LD_LINE_RE = re.compile(r"^\s*build/src/(\S+?)\.o\((\.\w+)\);", re.M)


def tu_id(path: str | Path, src: str | Path = SRC) -> str:
    """`src/a/b.c` -> `a/b` (also accepts a path already relative to src/)."""
    p = Path(path)
    try:
        p = p.relative_to(src)
    except ValueError:
        pass
    return p.with_suffix("").as_posix()


def src_path(tid: str) -> str:
    return f"src/{tid}.c"


def obj_path(tid: str, build_dir: str = "build") -> str:
    return f"{build_dir}/src/{tid}.o"


def arg_id(value: str) -> str:
    """argparse type for a TU argument: `main/368E4`, `src/main/368E4.c` or a nested
    id; must name an existing src/<id>.c ('' passes through as "unset")."""
    if not value:
        return value
    tid = value.replace("\\", "/")
    if tid.startswith("src/"):
        tid = tid[len("src/"):]
    if tid.endswith(".c"):
        tid = tid[:-2]
    if not Path(src_path(tid)).is_file():
        import argparse
        hint = [t for t in src_tus() if t.rsplit("/", 1)[-1] == tid.rsplit("/", 1)[-1]]
        raise argparse.ArgumentTypeError(
            f"no TU {tid!r} (src/{tid}.c missing)"
            + (f"; same basename: {', '.join(hint)}" if hint else ""))
    return tid


def src_tus(src: str | Path = SRC) -> list[str]:
    """Every C source under src/, recursively, as sorted TU ids."""
    root = Path(src)
    return sorted(tu_id(p, root) for p in root.rglob("*.c") if p.is_file())


def ld_sections(ld: str | Path = LD_SCRIPT, text: str | None = None) -> dict[str, list[str]]:
    """{section: [TU ids in link order]} from bb2.ld's build/src lines."""
    if text is None:
        text = Path(ld).read_text(encoding="utf-8")
    out: dict[str, list[str]] = {}
    for m in _LD_LINE_RE.finditer(text):
        out.setdefault(m.group(2), []).append(m.group(1))
    return out


def linked_tus(ld: str | Path = LD_SCRIPT, text: str | None = None) -> list[str]:
    """TU ids bb2.ld links, in first-appearance order."""
    if text is None:
        text = Path(ld).read_text(encoding="utf-8")
    seen: dict[str, None] = {}
    for m in _LD_LINE_RE.finditer(text):
        seen.setdefault(m.group(1), None)
    return list(seen)


def check(ld: str | Path = LD_SCRIPT, src: str | Path = SRC,
          text: str | None = None, lists: dict[str, set] | None = None) -> list[str]:
    """bb2.ld consistency problems ([] = consistent):
    every linked object has a source; every source is linked; no TU appears
    twice in one section; one object order holds across all sections (the
    per-section orders merge without a cycle); every flag-list entry is a TU."""
    if text is None:
        text = Path(ld).read_text(encoding="utf-8")
    sections = ld_sections(ld, text)
    linked = {t for ids in sections.values() for t in ids}
    odd = [ln.strip() for ln in text.splitlines()
           if "build/src/" in ln and not _LD_LINE_RE.match(ln)]
    problems = [f"bb2.ld line not of the form `build/src/<id>.o(.<section>);`: {ln}" for ln in odd]
    on_disk = set(src_tus(src))
    problems += [f"bb2.ld links build/src/{t}.o but src/{t}.c does not exist"
                 for t in sorted(linked - on_disk)]
    problems += [f"src/{t}.c is not linked by bb2.ld (no build/src/{t}.o line)"
                 for t in sorted(on_disk - linked)]
    succ: dict[str, set[str]] = {t: set() for t in linked}
    for sec, ids in sections.items():
        dup = sorted({t for t in ids if ids.count(t) > 1})
        problems += [f"bb2.ld section {sec} links build/src/{t}.o more than once" for t in dup]
        for a, b in zip(ids, ids[1:]):
            if a != b:
                succ[a].add(b)
    cycle = _find_cycle(succ)
    if cycle:
        problems.append("bb2.ld object order differs between sections: "
                        + " -> ".join(cycle)
                        + " (" + _order_witness(sections, cycle) + ")")
    return problems + flag_list_problems(src, lists)


# engine/buildconfig.py per-file flag sets, keyed by TU id (mirrors the Makefile).
FLAG_LISTS = ("GP_FILES", "PSYQ_LIBRARY_FILES", "EXPAND_LB_FILES", "EXPAND_LH_FILES", "NO_SR_FILES")


def flag_list_problems(src: str | Path = SRC, lists: dict[str, set] | None = None) -> list[str]:
    """A flag-list entry naming no TU is a flag silently lost (a moved or
    renamed file keeps its bytes only if its lists follow it)."""
    if lists is None:
        from . import buildconfig as cfg
        lists = {n: set(getattr(cfg, n)) for n in FLAG_LISTS}
    on_disk = set(src_tus(src))
    return [f"engine/buildconfig.py {name} names {t!r}, which is not a TU (no src/{t}.c)"
            for name, ids in lists.items() for t in sorted(set(ids) - on_disk)]


def _find_cycle(succ: dict[str, set[str]]) -> list[str] | None:
    WHITE, GREY, BLACK = 0, 1, 2
    color = {n: WHITE for n in succ}
    stack: list[str] = []

    def visit(n: str) -> list[str] | None:
        color[n] = GREY
        stack.append(n)
        for m in sorted(succ[n]):
            if color[m] == GREY:
                return stack[stack.index(m):] + [m]
            if color[m] == WHITE:
                c = visit(m)
                if c:
                    return c
        stack.pop()
        color[n] = BLACK
        return None

    for n in sorted(succ):
        if color[n] == WHITE:
            c = visit(n)
            if c:
                return c
    return None


def _order_witness(sections: dict[str, list[str]], cycle: list[str]) -> str:
    """Which section puts each consecutive cycle pair in that order."""
    parts = []
    for a, b in zip(cycle, cycle[1:]):
        for sec, ids in sections.items():
            if any(x == a and y == b for x, y in zip(ids, ids[1:])):
                parts.append(f"{sec}: {a} before {b}")
                break
    return "; ".join(parts)


def renames(path: str | Path = RENAMES) -> list[tuple[str, str, str]]:
    """(old id, new id, commit) rows of tools/tu_renames.tsv, in file order."""
    p = Path(path)
    if not p.exists():
        return []
    rows = []
    for line in p.read_text(encoding="utf-8").splitlines():
        if not line.strip() or line.startswith("#"):
            continue
        cols = line.split("\t")
        rows.append((cols[0], cols[1], cols[2] if len(cols) > 2 else ""))
    return rows


def resolve(tid: str, path: str | Path = RENAMES) -> str:
    """Follow the rename map from a historical id to today's id."""
    nxt = {old: new for old, new, _c in renames(path)}
    seen = set()
    while tid in nxt and tid not in seen:
        seen.add(tid)
        tid = nxt[tid]
    return tid


def main(argv: list[str] | None = None) -> int:
    import argparse
    ap = argparse.ArgumentParser(prog="engine.tus",
                                 description="TU discovery and the bb2.ld consistency check")
    ap.add_argument("cmd", choices=["check", "list", "linked"])
    a = ap.parse_args(argv)
    if a.cmd == "list":
        print("\n".join(src_tus()))
        return 0
    if a.cmd == "linked":
        print("\n".join(linked_tus()))
        return 0
    problems = check()
    for p in problems:
        print(f"FAIL: {p}")
    if not problems:
        print(f"OK: bb2.ld links all {len(src_tus())} TUs, one object order across sections")
    return 1 if problems else 0


if __name__ == "__main__":
    raise SystemExit(main())
