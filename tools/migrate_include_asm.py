#!/usr/bin/env python3
"""asm-until-matched migration (owner ruling 2026-08-19, .claude/rules/asm-until-matched.md).

Per function: bank the committed chassis + rule stack into the ledger, replace
the C body with `INCLUDE_ASM("asm/funcs", <func>);`, delete every
regfix/regfix_stage2/asmfix rule keyed to it, rebuild, and verify the full-build
SHA1 against the oracle. Rollback on any failure. Run under WSL python (LF).

Usage:
  python3 tools/migrate_include_asm.py --funcs f1,f2 [--dry-run]
  python3 tools/migrate_include_asm.py --batch N [--dry-run]   # next N queue items not yet migrated
"""
import argparse
import datetime
import json
import os
import re
import shutil
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
os.chdir(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from engine import inlineasm  # noqa: E402

ORACLE = "62efab4f73f992798c43e8c730aa43baa10bb4fa"
WITH_JTBL = False  # set by --with-jtbl (wave 2)
EXE = "disc/SLUS_006.63"
VRAM0, FILE0 = 0x80010000, 0x800


def jtbl_size_from_obj(stem, sym):
    """Symbol size from the current oracle-green TU object's symtab."""
    try:
        r = subprocess.run(["mipsel-linux-gnu-objdump", "-t", f"build/src/{stem}.o"],
                           capture_output=True, text=True, timeout=120)
        for ln in r.stdout.split("\n"):
            if ln.endswith(" " + sym) and ".rodata" in ln:
                return int(ln.split()[4], 16)
    except Exception:
        pass
    return 0


def write_jtbl_rodata(stem, func, sym):
    """Generate asm/rodata/<sym>.s in the established splat convention:
    `.word .L<vram>` label refs into the paired function asm. Returns None on
    success, else a refusal reason."""
    out = f"asm/rodata/{sym}.s"
    if os.path.isfile(out):
        return None
    # GCC-compiled switch tables are ANONYMOUS locals — no symtab size. The
    # table is self-terminating instead: every entry MUST be a text address
    # whose .L<vram> label exists in THIS function's asm (jump tables cannot
    # target outside the function), so scan words until one fails both tests.
    # A stray trailing word passing both is ~impossible; the oracle gate
    # backstops regardless.
    addr = int(sym.split("_")[1], 16)
    func_asm = rd(f"asm/funcs/{func}.s")
    words = []
    with open(EXE, "rb") as f:
        f.seek(addr - VRAM0 + FILE0)
        while True:
            raw = f.read(4)
            if len(raw) < 4:
                break
            w = int.from_bytes(raw, "little")
            if not (VRAM0 <= w < 0x80090000) or f".L{w:08X}" not in func_asm:
                break
            words.append(w)
    if len(words) < 2:
        return f"jtbl {sym}: self-terminating scan found only {len(words)} entries"
    lines = [f"nonmatching {sym}", "", f"dlabel {sym}"]
    lines += [f"    .word .L{w:08X}" for w in words]
    wr(out, "\n".join(lines) + "\n")
    return None
RULE_FILES = ["regfix.txt", "regfix_stage2.txt", "asmfix.txt"]
TODAY = datetime.date.today().isoformat()
BANK_DIR = "retired-chassis-2026-08"


def rd(p):
    with open(p, "rb") as f:
        return f.read().decode("utf-8")


def wr(p, s):
    with open(p, "wb") as f:
        f.write(s.encode("utf-8"))


def rule_lines(func):
    """[(file, lineno, text)] for every rule keyed to func."""
    out = []
    pat = re.compile(r"^" + re.escape(func) + r"\s*:")
    for rf in RULE_FILES:
        if not os.path.isfile(rf):
            continue
        for i, ln in enumerate(rd(rf).split("\n")):
            if pat.match(ln.strip()):
                out.append((rf, i, ln))
    return out


def sandbox_floor(func):
    """Honest pre-migration floor via the engine sandbox. None on failure."""
    try:
        r = subprocess.run(
            ["python3", "-m", "engine.cli", "sandbox", func, "--disable", "all"],
            capture_output=True, text=True, timeout=600)
        m = re.search(r'"score"\s*:\s*(\d+)', r.stdout)
        return int(m.group(1)) if m else None
    except Exception:
        return None


def build_sha1():
    # Engine clean-driver build, NOT bare make: Windows-side writes leave
    # future-dated files that make's clock-skew handling treats as up to date,
    # so make can exit "successfully" having built nothing (observed 2026-08-19
    # during the prototype run — sha1 never printed, tool saw None).
    r = subprocess.run(["python3", "-m", "engine.cli", "build"],
                       capture_output=True, text=True, timeout=3600)
    m = re.search(r"sha1[\"\s:]+([0-9a-f]{40})", r.stdout + r.stderr)
    if m:
        return m.group(1), r.stdout + r.stderr
    return None, r.stdout + r.stderr


def strip_asm_constructs(s):
    """Remove register-pin specifiers and __asm__/asm blocks before the
    rodata-literal scan. Quotes inside them are CODE (pin names, templates,
    constraints), not .rodata sources — leaving them in false-fails every
    pinned body (observed 2026-08-24: all 10 parked pin-carriers were
    refused as 'body emits .rodata' purely on their own cheat-asm quotes)."""
    out = []
    i, n = 0, len(s)
    opener = re.compile(r"(?:__asm__|(?<![\w])asm)\s*(?:volatile\s*)?\(")
    while i < n:
        m = opener.search(s, i)
        if not m:
            out.append(s[i:])
            break
        out.append(s[i:m.start()])
        j, depth, in_str = m.end(), 1, False
        while j < n and depth:
            c = s[j]
            if in_str:
                if c == "\\":
                    j += 1
                elif c == '"':
                    in_str = False
            elif c == '"':
                in_str = True
            elif c == "(":
                depth += 1
            elif c == ")":
                depth -= 1
            j += 1
        i = j
    return "".join(out)


def find_body_span(text, func):
    span = inlineasm._func_body_span(text, func)
    if span is None:
        return None
    lo, hi = span
    # _func_body_span's lo is the SIGNATURE start; its hi is already one PAST
    # the closing brace (engine/inlineasm._match_brace contract) — do NOT +1,
    # that eats the following newline and glues the next definition onto the
    # INCLUDE_ASM line (observed 2026-08-19: two adjacent functions dropped out
    # of the COMPLETED census because their signatures no longer sat at line
    # start). Snap lo to its line start; return hi as-is.
    line_start = text.rfind("\n", 0, lo) + 1
    if hi <= line_start:
        return None  # defensive: never hand back an inverted span
    return line_start, hi


def migrate_one(func, stem, dry, defer_build=False):
    src = f"src/{stem}.c"
    asm = f"asm/funcs/{func}.s"
    text = rd(src)
    if f'INCLUDE_ASM("asm/funcs", {func});' in text:
        return "already-migrated", []
    if not os.path.isfile(asm):
        return f"REFUSED: {asm} missing", []
    jtbls = sorted(set(re.findall(r"jtbl_[0-9A-F]{8}", rd(asm))))
    if jtbls and not WITH_JTBL:
        # The raw asm references a switch jump table that only exists as
        # compiler OUTPUT of the C body's switch (rodata cleanup 2026-06-09
        # moved jtbls out of asm/data). Removing the body would orphan the
        # symbol at link (observed batch 1: func_800324D0 / jtbl_800105A0).
        # Wave-2 (--with-jtbl) supplies them via the established
        # INCLUDE_RODATA("asm/rodata", jtbl_X) convention.
        return "REFUSED: asm references a C-generated jtbl_ symbol (deferred)", []
    span = find_body_span(text, func)
    if span is None:
        return f"REFUSED: no C body span for {func} in {src}", []
    lo, hi = span
    body = text[lo:hi]
    # rodata-emitting bodies: since the 2026-06-09 rodata cleanup, string
    # literals / const arrays / FP literals in a body are the SOURCE of that
    # function's .rodata. Removing such a body shifts the section layout and
    # breaks the byte-match (observed sweep batch 2: MISMATCH). Defer them.
    stripped = re.sub(r"/\*.*?\*/", "", body, flags=re.S)
    stripped = re.sub(r"//[^\n]*", "", stripped)
    stripped = strip_asm_constructs(stripped)
    if ('"' in stripped or re.search(r"\bconst\b", stripped)
            or re.search(r"\b\d+\.\d+f?\b", stripped)):
        return ("REFUSED: body emits .rodata (string/const/FP literal) — "
                "removal shifts section layout (deferred)"), []
    rules = rule_lines(func)
    if dry:
        return ("DRY: would bank %d-line body + %d rule(s), splice INCLUDE_ASM"
                % (body.count("\n") + 1, len(rules))), rules

    floor = sandbox_floor(func)

    led = f"memory/grind/{func}"
    bank = os.path.join(led, BANK_DIR)
    os.makedirs(bank, exist_ok=True)
    wr(os.path.join(bank, "body.c"), body + "\n")
    wr(os.path.join(bank, "rules.txt"),
       "".join(f"{rf}:{no + 1}: {ln}\n" for rf, no, ln in rules) or "(no rules)\n")
    pin = {"floor": floor, "measured": TODAY, "source": "pre-migration chassis",
           "banked_body": f"{BANK_DIR}/body.c", "rules_retired": len(rules)}
    wr(os.path.join(led, "migration_pin.json"), json.dumps(pin, indent=2) + "\n")

    # Wave 2: supply C-generated jump tables via the established
    # INCLUDE_RODATA convention (mirrors code6cac_b.c:162's committed pattern:
    # rodata line(s) directly before the function's text line).
    created_rodata = []
    if jtbls and WITH_JTBL:
        for sym in jtbls:
            why = write_jtbl_rodata(stem, func, sym)
            if why:
                for p in created_rodata:
                    os.remove(p)
                return f"REFUSED (wave2): {why}", []
            if os.path.isfile(f"asm/rodata/{sym}.s"):
                created_rodata.append(f"asm/rodata/{sym}.s")

    # splice — with an explicit trailing newline whenever the tail doesn't
    # start with one (a body ending "}" flush against the next definition
    # glued get_alarm/_version on 2026-08-19 and silently dropped a COMPLETED
    # function from the census parse).
    line = ""
    if jtbls and WITH_JTBL:
        line = "".join(f'INCLUDE_RODATA("asm/rodata", {s});\n' for s in jtbls)
    line += f'INCLUDE_ASM("asm/funcs", {func});'
    tail = text[hi:]
    if not tail.startswith("\n"):
        line += "\n"
    new_text = text[:lo] + line + tail
    wr(src, new_text)
    # delete rules (by exact line content, from each file)
    originals = {}
    for rf in RULE_FILES:
        if not os.path.isfile(rf):
            continue
        originals[rf] = rd(rf)
        keep = [ln for ln in originals[rf].split("\n")
                if not re.match(r"^" + re.escape(func) + r"\s*:", ln.strip())]
        wr(rf, "\n".join(keep))

    if defer_build:
        return f"EDITED (build deferred): floor-pin {floor}, {len(rules)} rule(s) retired", rules

    sha, log = build_sha1()
    if sha != ORACLE:
        # rollback everything
        wr(src, text)
        for rf, orig in originals.items():
            wr(rf, orig)
        for p in created_rodata:
            try:
                os.remove(p)
            except OSError:
                pass
        shutil.rmtree(bank, ignore_errors=True)
        try:
            os.remove(os.path.join(led, "migration_pin.json"))
        except OSError:
            pass
        tail = "\n".join(log.split("\n")[-15:])
        return f"FAILED: sha1={sha} != oracle — ROLLED BACK. Build tail:\n{tail}", rules
    return f"OK: floor-pin {floor}, {len(rules)} rule(s) retired, SHA1 MATCH", rules


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--funcs", default="")
    ap.add_argument("--batch", type=int, default=0)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--batch-verify", action="store_true",
                    help="edit all targets first, then ONE engine build + SHA1 "
                         "gate for the whole batch (git restores on mismatch)")
    ap.add_argument("--with-jtbl", action="store_true",
                    help="wave 2: supply C-generated jtbls via INCLUDE_RODATA")
    a = ap.parse_args()
    global WITH_JTBL
    WITH_JTBL = a.with_jtbl

    q = json.load(open("engine/queue.json"))
    by_func = {it["func"]: it for it in q["items"]}
    if a.funcs:
        targets = [f.strip() for f in a.funcs.split(",") if f.strip()]
    else:
        skip = set()
        if os.path.isfile("tmp/migration-deferred.txt"):
            skip = {ln.split("\t")[0] for ln in
                    rd("tmp/migration-deferred.txt").splitlines() if ln.strip()}
        targets = []
        for it in q["items"]:
            if it.get("status") not in ("active", "parked"):
                continue
            if it["func"] in skip:
                continue
            try:
                if f'INCLUDE_ASM("asm/funcs", {it["func"]});' in rd(f'src/{it["file"]}.c'):
                    continue  # already migrated
            except OSError:
                continue
            targets.append(it["func"])
            if len(targets) >= a.batch:
                break

    failed = []
    edited = []
    for func in targets:
        it = by_func.get(func)
        if not it:
            print(f"{func}: NOT IN QUEUE — skipped")
            continue
        res, _ = migrate_one(func, it["file"], a.dry_run,
                             defer_build=a.batch_verify)
        print(f"{func}: {res}", flush=True)
        if res.startswith("EDITED"):
            edited.append(func)
        if res.startswith(("FAILED", "REFUSED")):
            failed.append(func)
            with open("tmp/migration-deferred.txt", "a", encoding="utf-8") as f:
                f.write(f"{func}\t{res.splitlines()[0]}\n")
    if a.batch_verify and edited and not a.dry_run:
        sha, log = build_sha1()
        if sha == ORACLE:
            print(f"\nBATCH VERIFIED: {len(edited)} function(s), SHA1 MATCH")
        else:
            print(f"\nBATCH FAILED: sha1={sha} != oracle. NOT rolling back "
                  f"automatically in batch mode — use `git checkout -- src "
                  f"regfix.txt regfix_stage2.txt asmfix.txt` (ledger banks are "
                  f"additive and safe to keep), or bisect with per-function mode.")
            print("\n".join(log.split("\n")[-15:]))
            sys.exit(1)
    if failed:
        print(f"\n{len(failed)} deferred: {', '.join(failed)}")
        sys.exit(1)


if __name__ == "__main__":
    main()
