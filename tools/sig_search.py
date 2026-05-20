#!/usr/bin/env python3
"""sig_search -- resolve called-function arg counts by matching the TARGET.

Companion to sig_reconcile.py. The C declaration surface (arg COUNT of called
function pointers) controls cross-jump merging (see
.claude/rules/cross-jump-call-merge.md): GCC merges indirect calls whose
CALL_INSN_FUNCTION_USAGE is equal, and arg count differentiates them. m2c
over-infers signatures, so the build under-counts call sites.

This tool searches the arg-count of each fn-ptr handler (from its declared count
DOWN to 1 -- m2c over-infers, real <= declared) and scores each configuration by:
    (1)  |build_jalr - target_jalr|     (PRIMARY -- structural call-site count)
    (2)  alignment-immune edit distance vs target  (SECONDARY)
The search is GROUNDED in the target's real jalr count + asm, so it resolves the
arg counts the way the original was actually compiled -- it does not minimise
diff blindly, so it cannot manufacture a cheat.

Usage:
  python3 tools/sig_search.py <func> [--apply] [--max-args N] [--dir D]
"""
import argparse
import difflib
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from sig_reconcile import disasm  # noqa: E402

JALR_W = 1000  # weight: one missing/extra call site dominates the score


def norm_insns(obj, func):
    out = []
    for i in disasm(obj, func):
        ops = re.sub(r"[0-9a-fA-F]+ <[^>]+>", "T", i["ops"])
        ops = re.sub(r"\s+", " ", ops).strip()
        out.append(f"{i['mnem']} {ops}")
    return out


def jalr_count(obj, func):
    return sum(1 for i in disasm(obj, func) if i["mnem"] == "jalr")


def split_args(s):
    """Split a call's argument string on top-level commas (paren-aware)."""
    args, depth, cur = [], 0, ""
    for ch in s:
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        if ch == "," and depth == 0:
            args.append(cur)
            cur = ""
        else:
            cur += ch
    if cur.strip():
        args.append(cur)
    return args


def truncate_calls(txt, name, n):
    """Rewrite every call `name(...)` to keep only its first n arguments."""
    out, i, pat = [], 0, name + "("
    while True:
        j = txt.find(pat, i)
        if j < 0:
            out.append(txt[i:])
            break
        # require a non-identifier char before the name (avoid substring hits)
        if j > 0 and (txt[j - 1].isalnum() or txt[j - 1] == "_"):
            out.append(txt[i:j + len(pat)])
            i = j + len(pat)
            continue
        out.append(txt[i:j])
        k = j + len(pat)
        depth, start, end = 1, k, None
        while k < len(txt) and depth > 0:
            if txt[k] == "(":
                depth += 1
            elif txt[k] == ")":
                depth -= 1
                if depth == 0:
                    end = k
            k += 1
        args = split_args(txt[start:end])
        kept = ", ".join(a.strip() for a in args[:n])
        out.append(f"{name}({kept})")
        i = k
    return "".join(out)


def apply_config(base_text, config):
    txt = base_text
    for name, n in config.items():
        params = "void" if n == 0 else ", ".join(["int"] * n)
        txt = re.sub(
            r"extern\s+[\w\s\*]+?\(\s*\*\s*" + re.escape(name) + r"\s*\)\s*\([^)]*\)\s*;",
            f"extern void (*{name})({params});",
            txt,
        )
        txt = truncate_calls(txt, name, n)
    return txt.replace("\r\n", "\n")


def find_handlers(base_text):
    """fn-ptr externs that are actually called in the body -> declared arg count."""
    handlers = {}
    for m in re.finditer(
        r"extern\s+[\w\s\*]+?\(\s*\*\s*(\w+)\s*\)\s*\(([^)]*)\)\s*;", base_text
    ):
        name, params = m.group(1), m.group(2).strip()
        if re.search(r"(?<![\w])" + re.escape(name) + r"\s*\(", base_text.replace(m.group(0), "")):
            cnt = 0 if params in ("", "void") else len([p for p in params.split(",") if p.strip()])
            handlers[name] = cnt
    return handlers


def score(pdir, func, base_text, config, target_insns, target_jalr, cache):
    key = tuple(sorted(config.items()))
    if key in cache:
        return cache[key]
    trial = Path("/tmp/sig_trial.c")
    trial.write_bytes(apply_config(base_text, config).encode("utf-8"))
    subprocess.run(
        ["bash", str(pdir / "compile.sh"), str(trial), "-o", "/tmp/sig_trial.o"],
        capture_output=True, text=True
    )
    if not Path("/tmp/sig_trial.o").exists() or Path("/tmp/sig_trial.o").stat().st_size == 0:
        cache[key] = (10 ** 9, None, None)
        return cache[key]
    bi = norm_insns("/tmp/sig_trial.o", func)
    bj = sum(1 for x in bi if x.startswith("jalr"))
    edits = sum(
        max(o[2] - o[1], o[4] - o[3])
        for o in difflib.SequenceMatcher(None, bi, target_insns).get_opcodes()
        if o[0] != "equal"
    )
    s = abs(bj - target_jalr) * JALR_W + edits
    cache[key] = (s, bj, edits)
    return cache[key]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("func")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--max-args", type=int, default=4)
    ap.add_argument("--dir", default=None)
    args = ap.parse_args()

    pdir = Path(args.dir) if args.dir else Path("permuter") / args.func
    base_c = pdir / "base.c"
    target_o = pdir / "target.o"
    if not base_c.exists() or not target_o.exists():
        print(f"ERROR: need {base_c} and {target_o}", file=sys.stderr)
        return 1

    base_text = base_c.read_bytes().decode("utf-8")
    target_insns = norm_insns(str(target_o), args.func)
    target_jalr = jalr_count(str(target_o), args.func)
    handlers = find_handlers(base_text)
    if not handlers:
        print("no fn-ptr handler calls found in base.c"); return 1

    print(f"target jalr={target_jalr}; handlers (declared arg counts): {handlers}")
    import random
    rng = random.Random(20260520)
    cache = {}

    def descend(config):
        improved, p = True, 0
        while improved and p < 4:
            improved, p = False, p + 1
            for h in handlers:
                cur = config[h]
                best_n = cur
                best = score(pdir, args.func, base_text, config, target_insns, target_jalr, cache)
                for n in range(1, min(handlers[h], args.max_args) + 1):
                    if n == cur:
                        continue
                    trial = dict(config); trial[h] = n
                    sc = score(pdir, args.func, base_text, trial, target_insns, target_jalr, cache)
                    if sc[0] < best[0]:
                        best, best_n = sc, n
                if best_n != cur:
                    config[h] = best_n
                    improved = True
        return config

    start = score(pdir, args.func, base_text, dict(handlers), target_insns, target_jalr, cache)
    print(f"start (all-declared): jalr={start[1]} edits={start[2]} score={start[0]}")
    # multi-restart coordinate descent escapes the local minima that a single
    # all-declared descent falls into (it lowers the wrong handler first).
    seeds = [dict(handlers), {h: 1 for h in handlers}]
    hlist = list(handlers)
    # structured seeds: dispatch tables typically have most handlers sharing one
    # signature with one or two "special" arms (extra arg, or an intervening call).
    for rest in (2, 3, 4):  # all-uniform
        seeds.append({h: min(rest, handlers[h]) for h in handlers})
    for special in hlist:   # one handler kept at its max, the rest uniform
        for rest in (2, 3):
            seeds.append({h: (min(handlers[h], args.max_args) if h == special else min(rest, handlers[h]))
                          for h in handlers})
    for _ in range(4):
        seeds.append({h: rng.randint(1, min(handlers[h], args.max_args)) for h in handlers})
    for seed in seeds:
        descend(dict(seed))
        if len(cache) > 220:  # compile budget guard
            break
    # global best across EVERYTHING evaluated (score already prefers jalr match,
    # then min edits) -- captures any good config stumbled on during any descent.
    best_key = min(cache, key=lambda k: cache[k][0])
    config = dict(best_key)
    final = cache[best_key]
    print(f"\nBEST: config={config}  jalr={final[1]} (target {target_jalr})  edits={final[2]}")
    if final[1] == target_jalr:
        print("  -> jalr count MATCHES target. Structure reconciled.")
    else:
        print("  -> jalr still off; may need control-flow (jump vs fall-through), "
              "not just arg count.")

    if args.apply:
        base_c.write_bytes(apply_config(base_text, config).encode("utf-8"))
        print(f"applied best config to {base_c}")
    else:
        print("(dry run; pass --apply to write base.c)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
