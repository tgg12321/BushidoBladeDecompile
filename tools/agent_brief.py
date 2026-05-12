#!/usr/bin/env python3
"""Print a complete agent context dump for a single function.

When an agent starts a session on a specific function, it needs:
  - Classification verdict (size, blockers, recommendation)
  - The target assembly
  - The current m2c-generated C
  - The live pipeline diff (gen_regfix output)
  - Recipe suggestions from the library
  - Kengo PS2-sequel reference if available
  - Prior tabled notes from archive/tabled_attempts/

Running each of these as separate `dc.sh` commands costs 5+ tool calls and
some token overhead per call. This combines them into one structured
report ready for the agent to read once and start work.

Usage:
    python3 tools/agent_brief.py <func>
    python3 tools/agent_brief.py <func> --no-asm     # skip the raw asm
    python3 tools/agent_brief.py <func> --json       # machine-readable

Designed to be the FIRST thing an agent runs on a function.
"""
from __future__ import annotations

import argparse
import csv
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TOOLS = ROOT / "tools"
ASM_FUNCS = ROOT / "asm" / "funcs"
SRC_DIR = ROOT / "src"
PERMUTER = ROOT / "permuter"
TABLED = ROOT / "archive" / "tabled_attempts"
KENGO_MATCHES = ROOT / "kengo_matches.csv"


# Strip our TU-static disambiguation suffix so renamed BB2 funcs can be
# looked up against Kengo's bare names (e.g. mario_getMarioVoiceData_8001B3C0
# is one of several BB2 instances of Kengo's mario_getMarioVoiceData).
_ADDR_SUFFIX = re.compile(r"_[0-9A-Fa-f]{8}$")


def strip_addr_suffix(name: str) -> str:
    return _ADDR_SUFFIX.sub("", name)


# Confidence tier descriptions for the agent. Helps it weight the
# Kengo reference appropriately: strong tiers can be trusted as a
# structural template; weak tiers are starting points only.
_CONFIDENCE_BLURB = {
    "name-unique":        "STRONG — BB2 and Kengo share the exact name",
    "name-callgraph":     "STRONG — name match plus call-graph overlap",
    "callgraph":          "STRONG — call-graph overlap with Kengo",
    "caller-unique":      "STRONG — hard caller-graph constraint (single survivor)",
    "caller-callgraph":   "STRONG — caller constraint plus Jaccard",
    "affinity-unique":    "MODERATE — narrowed by per-file Kengo-author affinity",
    "affinity-callgraph": "MODERATE — affinity plus Jaccard",
    "seq-similarity":     "WEAK — opcode-sequence dominance only; verify carefully",
    "size-only-unique":   "WEAK — only one Kengo candidate by size; no semantic signal",
}


def run(cmd: list[str], timeout: int = 60) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, capture_output=True, text=True,
                          cwd=str(ROOT), timeout=timeout)


def section(title: str) -> str:
    return f"\n{'=' * 60}\n{title}\n{'=' * 60}\n"


def classify(func: str) -> tuple[dict, str]:
    """Returns (parsed dict, raw text)."""
    r = run([sys.executable, str(TOOLS / "classify_func.py"), func, "--json"], 30)
    try:
        d = json.loads(r.stdout) if r.stdout.strip() else {}
    except json.JSONDecodeError:
        d = {}
    raw = run([sys.executable, str(TOOLS / "classify_func.py"), func], 30).stdout
    return d, raw


def asm_body(func: str, max_lines: int | None = None) -> str:
    p = ASM_FUNCS / f"{func}.s"
    if not p.exists():
        return f"(no asm/funcs/{func}.s)"
    text = p.read_text(encoding="utf-8")
    if max_lines and len(text.splitlines()) > max_lines:
        lines = text.splitlines()
        head = "\n".join(lines[:max_lines])
        return f"{head}\n... ({len(lines) - max_lines} more lines truncated; full file at asm/funcs/{func}.s)"
    return text


def base_c(func: str) -> tuple[str, bool]:
    """Returns (text, is_post_processed). Strips the typedef preamble for
    readability — the agent already knows what u8/s32/GameObj/etc are."""
    p = PERMUTER / func / "base.c"
    if not p.exists():
        return f"(no permuter/{func}/base.c -- run `dc.sh inline-setup` or `dc.sh attempt`)", False
    text = p.read_text(encoding="utf-8")

    # Heuristic: drop everything up to the first non-typedef function decl.
    lines = text.splitlines()
    # Find the first line that looks like a function decl: `<type> <name>(args)`
    func_start = None
    for i, line in enumerate(lines):
        if re.match(rf"^[\w\s\*]+\b{re.escape(func)}\s*\(", line):
            func_start = i
            break
    if func_start is None:
        return text, False

    # Walk back to keep any extern decls right above the function
    keep_from = func_start
    while keep_from > 0:
        prev = lines[keep_from - 1].strip()
        if prev.startswith("extern ") or prev.startswith("/* "):
            keep_from -= 1
        else:
            break

    return "\n".join(lines[keep_from:]), True


def m2c_structural_reading(func: str) -> str:
    """Surface STRUCTURE-revealing observations from m2c's base.c body.

    m2c often reads the original C's *expression-level* structure better
    than agents do (it has no GCC-allocator bias). When the agent gets
    walled by register cascade, re-reading m2c's body for high-level
    shape clues (esp. whether a store is folded INTO an expression that
    contains a call, vs. statements on separate lines) is often the unlock.

    This function extracts those clues at the top of the brief so the
    agent doesn't have to scroll past the typedef preamble to see them.

    Observations surfaced:
      - Function calls inside other expressions (suggests delay-slot fill
        opportunity; classic store-before-jal pattern).
      - Pre-call writes to memory (`*ptr = ...; jal(...);`) — same hint.
      - Pre-computed temps fed to calls vs. inline-computed args.
      - Whether the function reads back a value via the same pointer it
        just stored to (suggests store + reload-from-memory + cast pattern).
    """
    body, ok = base_c(func)
    if not ok:
        return ""
    # Find the function's brace body.
    m = re.search(
        rf"\b{re.escape(func)}\s*\([^)]*\)\s*\{{(.*)\n\}}",
        body, re.DOTALL,
    )
    if not m:
        return ""
    bd = m.group(1)

    observations = []

    # 1) Find statements where a function call appears INSIDE a larger
    #    expression (e.g., `*(ptr) = ... + jal(...);` or `func(jal(...))`).
    nested_call_lines = []
    for line in bd.splitlines():
        ls = line.strip()
        if not ls or ls.startswith("/*") or ls.startswith("//"):
            continue
        # Skip control-flow constructs — the "function call nested in
        # condition" of an if/while is a control-flow shape, not a
        # delay-slot fill candidate.
        if re.match(r"^(if|while|for|switch|else if)\b", ls):
            continue
        # We're looking for: a function call that appears INSIDE a
        # store-RHS expression or some arithmetic, with a store target
        # or arithmetic operator visible on the SAME line. This is the
        # classic store-before-jal / delay-slot-fill shape.
        if not re.search(r"\b[a-zA-Z_]\w*\s*\([^)]*\)", ls):
            continue
        stripped = re.sub(r"\b[a-zA-Z_]\w*\s*\([^()]*\)", "@CALL@", ls)
        # The line must:
        #   (a) reference a memory store (`*ptr = ...`) or a typed cast
        #       deref on the LHS (`*(s32 *)... =`), OR
        #   (b) compute an arithmetic expression *containing* the call.
        # AND must NOT be a plain function-call statement (`fn(x);`).
        has_store = bool(re.search(r"\*\s*\(?[\w]+.*?=", stripped))
        has_arith_around = bool(
            re.search(r"@CALL@\s*[+\-*<<>>|&^]", stripped)
            or re.search(r"[+\-*<<>>|&^]\s*@CALL@", stripped)
        )
        bare_call_stmt = re.fullmatch(r"\s*@CALL@\s*;\s*", stripped)
        if (has_store or has_arith_around) and not bare_call_stmt:
            nested_call_lines.append(ls)
    if nested_call_lines:
        observations.append(
            "Call(s) embedded in larger expressions — this is m2c's read of "
            "the original C's delay-slot-fill structure. Replicate the "
            "expression shape verbatim in src/ (don't hoist the call into "
            "a separate temp). See feedback_store_before_jal.md."
        )
        for ln in nested_call_lines[:3]:
            observations.append(f"  • {ln[:120]}")

    # 2) Pre-call store followed by call (and possibly read-back).
    lines = [ln.rstrip() for ln in bd.splitlines() if ln.strip() and not ln.strip().startswith("/*")]
    pre_call_store = False
    for i, ln in enumerate(lines[:-1]):
        if re.match(r"\s*\*\s*\(", ln) and "=" in ln and not "==" in ln:
            # Looks like a memory store.
            nxt = lines[i + 1] if i + 1 < len(lines) else ""
            if re.search(r"\b[a-zA-Z_]\w*\s*\(", nxt) and "=" in nxt:
                pre_call_store = True
                break
    if pre_call_store:
        observations.append(
            "Pre-call store-to-memory immediately followed by a function "
            "call — same delay-slot-fill hint. GCC can move the sh into the "
            "jal's delay slot if the C order is `store; call;`."
        )

    if not observations:
        return ""
    return "\n".join(observations)


def is_bridged(func: str) -> bool:
    """True if asmfix.txt has an active (non-commented) replace_with_asmfile
    rule for this function. Bridged functions have no C body to diff against,
    so gen_regfix output is meaningless (every target instruction is "missing")."""
    asmfix = ROOT / "asmfix.txt"
    if not asmfix.is_file():
        return False
    needle = f"{func}: replace_with_asmfile"
    for line in asmfix.read_text(encoding="utf-8", errors="replace").splitlines():
        stripped = line.lstrip()
        if stripped.startswith("#"):
            continue
        if stripped.startswith(needle):
            return True
    return False


def is_retiring(func: str) -> bool:
    """True if asmfix.txt has a `# RETIRE: <func>: replace_with_asmfile` line
    (bridge commented out, retirement in progress)."""
    asmfix = ROOT / "asmfix.txt"
    if not asmfix.is_file():
        return False
    needle = f"# RETIRE: {func}: replace_with_asmfile"
    for line in asmfix.read_text(encoding="utf-8", errors="replace").splitlines():
        if line.lstrip().startswith(needle):
            return True
    return False


def function_state(func: str) -> str:
    """One-line state describing where this function is in the lifecycle.
    Surfaced at the top of agent-brief so the agent picks the right tools."""
    if is_bridged(func):
        return ("BRIDGED (active replace_with_asmfile in asmfix.txt). The "
                "linked binary contains the original asm bytes, not C codegen. "
                "To start pure-C work: `dc.sh retire " + func + "`.")
    if is_retiring(func):
        return ("RETIRING (bridge commented out as `# RETIRE: ...`). The C "
                "body in src/ is now the source of truth. Iterate, then "
                "verify with `dc.sh build-active " + func + "` (NOT plain "
                "`dc.sh verify` — that's bridge-blind).")
    return "NORMAL (no bridge in asmfix.txt; standard decomp queue work)."


def gen_regfix(func: str) -> str:
    if is_bridged(func):
        return ("(skipped — function is currently BRIDGED via "
                "replace_with_asmfile in asmfix.txt; gen_regfix output would "
                "show every target instruction as 'missing'. Run "
                "`dc.sh retire " + func + "` first to comment out the bridge, "
                "then re-run agent-brief to see the real diff.)")
    r = run([sys.executable, str(TOOLS / "gen_regfix.py"), func], 60)
    return r.stdout if r.returncode == 0 else f"(gen_regfix failed: {r.stderr.strip()[:200]})"


def recipe_suggestions(func: str) -> str:
    r = run([sys.executable, str(TOOLS / "recipes.py"), "suggest", func], 30)
    return r.stdout.strip() or "(no recipe matches)"


def kengo_csv_row(func: str) -> dict | None:
    """Look up a BB2 function in kengo_matches.csv. Tries the literal
    name first, then the suffix-stripped form for our renamed funcs."""
    if not KENGO_MATCHES.exists():
        return None
    candidates = [func]
    stripped = strip_addr_suffix(func)
    if stripped != func:
        candidates.append(stripped)
    for r in csv.DictReader(KENGO_MATCHES.open()):
        if r["bb2_func"] in candidates and r["kengo_name"]:
            return r
    return None


def kengo_metadata(func: str) -> str:
    """One-paragraph header summarizing the Kengo match: confidence,
    source path, scoring signals. Empty string if no match."""
    row = kengo_csv_row(func)
    if not row:
        return ""
    conf = row.get("confidence", "")
    blurb = _CONFIDENCE_BLURB.get(conf, conf)
    lines = []
    lines.append(f"Kengo equivalent: {row['kengo_name']}")
    if row.get("kengo_source_file"):
        lines.append(f"Kengo source:     {row['kengo_source_file']}")
    lines.append(f"Confidence:       {conf}  ({blurb})")
    sigs = []
    if row.get("callee_overlap") and float(row["callee_overlap"]) > 0:
        sigs.append(f"callee-J={row['callee_overlap']}")
    if row.get("caller_overlap") and float(row["caller_overlap"]) > 0:
        sigs.append(f"caller-J={row['caller_overlap']}")
    if row.get("opseq_ratio") and float(row["opseq_ratio"]) > 0:
        sigs.append(f"opseq={row['opseq_ratio']}")
    if row.get("n_candidates"):
        sigs.append(f"candidates={row['n_candidates']}")
    bb2_i = row.get("bb2_insns", "?")
    k_i = row.get("kengo_insns", "?")
    sigs.append(f"insns BB2/Kengo={bb2_i}/{k_i}")
    if sigs:
        lines.append("Signals:          " + ", ".join(sigs))
    STRONG_TIERS = {"name-unique", "name-callgraph", "callgraph",
                    "caller-unique", "caller-callgraph"}
    if conf not in STRONG_TIERS:
        lines.append("Caveat:           this match is weaker — confirm by "
                     "reading the Kengo asm below before relying on it.")
    return "\n".join(lines)


def kengo_callee_hints(func: str) -> str:
    """List BB2 callees this function jals that have a confident Kengo
    name in kengo_matches.csv but are still `func_XXXX` in our build.
    Useful naming hints — the agent sees what each unnamed callee
    probably does without having to dig."""
    asm = ASM_FUNCS / f"{func}.s"
    if not asm.exists() or not KENGO_MATCHES.exists():
        return ""
    callees: set[str] = set()
    for line in asm.read_text(encoding="utf-8", errors="replace").splitlines():
        m = re.search(r"\bjal\s+(\w+)", line)
        if m:
            callees.add(m.group(1))
    if not callees:
        return ""

    STRONG = {"name-unique", "name-callgraph", "callgraph",
              "caller-unique", "caller-callgraph",
              "affinity-unique", "affinity-callgraph"}
    hints: list[tuple[str, str, str]] = []
    for r in csv.DictReader(KENGO_MATCHES.open()):
        bb2 = r["bb2_func"]
        if not bb2.startswith("func_"):
            continue
        if bb2 not in callees or not r["kengo_name"]:
            continue
        opr = float(r.get("opseq_ratio") or 0)
        conf = r["confidence"]
        if conf in STRONG:
            hints.append((bb2, r["kengo_name"], conf))
        elif conf == "seq-similarity" and opr >= 0.40:
            hints.append((bb2, r["kengo_name"], f"{conf} opseq={opr:.2f}"))
        elif conf == "size-only-ambiguous" and opr >= 0.40:
            # Low absolute confidence but the opcode similarity is strong;
            # still worth surfacing as a naming hint for the agent.
            hints.append((bb2, r["kengo_name"], f"{conf} opseq={opr:.2f}"))
    if not hints:
        return ""
    hints.sort()
    return "\n".join(
        f"  {bb2:<22s} ~= {kn:<40s} ({conf})"
        for bb2, kn, conf in hints
    )


def kengo_ref(func: str) -> str:
    kr = TOOLS / "kengo_ref.py"
    if not kr.exists():
        return ""
    # For BB2 funcs that have been renamed with a `_<ADDR>` suffix, the
    # bare name lookup in Kengo is via the stripped form.
    lookup = strip_addr_suffix(func)
    r = run([sys.executable, str(kr), lookup, "--bb2"], 15)
    text = (r.stdout or r.stderr).strip()
    if not text or "no kengo match" in text.lower() or "not found" in text.lower():
        return ""
    return text


def tabled_notes(func: str) -> str:
    if not TABLED.exists():
        return ""
    matches = list(TABLED.glob(f"{func}_*.md")) + list(TABLED.glob(f"{func}_*.c"))
    if not matches:
        return ""
    parts = []
    for p in sorted(matches):
        parts.append(f"--- {p.relative_to(ROOT)} ---")
        try:
            parts.append(p.read_text(encoding="utf-8", errors="replace"))
        except Exception as e:
            parts.append(f"(error reading: {e})")
    return "\n".join(parts)


def existing_regfix(func: str) -> str:
    p = ROOT / "regfix.txt"
    if not p.exists():
        return ""
    out = []
    for line in p.read_text(encoding="utf-8").splitlines():
        if line.startswith(f"{func}:") or line.startswith(f"# {func}:"):
            out.append(line)
    return "\n".join(out)


def src_neighbors(func: str, src_relpath: str | None) -> str:
    """Show the function names immediately before and after this one in src/.
    Helps an agent figure out the surrounding subsystem for naming."""
    if not src_relpath:
        return ""
    p = ROOT / src_relpath
    if not p.exists():
        return ""
    text = p.read_text(encoding="utf-8", errors="ignore")
    func_decl_re = re.compile(
        r"^(?:static\s+)?(?:[\w_]+\s*\*?\s+){1,3}(\w+)\s*\([^;]*\)\s*\{",
        re.MULTILINE,
    )
    glabel_re = re.compile(r"glabel\s+(\w+)")

    funcs_in_order: list[str] = []
    for m in re.finditer(
        rf"(?:{func_decl_re.pattern})|(?:glabel\s+(\w+))",
        text, re.MULTILINE,
    ):
        n = m.group(1) or m.group(2)
        if n:
            funcs_in_order.append(n)

    if func not in funcs_in_order:
        return ""
    i = funcs_in_order.index(func)
    before = funcs_in_order[max(0, i - 3):i]
    after = funcs_in_order[i + 1:i + 4]
    return f"prev: {', '.join(before) or '(start of file)'}\nnext: {', '.join(after) or '(end of file)'}"


def build_brief(func: str, include_asm: bool = True,
                asm_max_lines: int = 200) -> dict:
    cls_dict, cls_raw = classify(func)
    src_kind = cls_dict.get("src", {}).get("kind") if cls_dict else None
    src_relpath = cls_dict.get("src", {}).get("file") if cls_dict else None

    return {
        "func": func,
        "function_state": function_state(func),
        "m2c_reading": m2c_structural_reading(func),
        "classify": cls_raw.strip(),
        "classify_dict": cls_dict,
        "asm_body": asm_body(func, asm_max_lines if include_asm else 0)
                    if include_asm else "(skipped via --no-asm)",
        "base_c": base_c(func)[0],
        "gen_regfix": gen_regfix(func).strip(),
        "recipe_suggestions": recipe_suggestions(func),
        "kengo_metadata": kengo_metadata(func),
        "kengo_callee_hints": kengo_callee_hints(func),
        "kengo": kengo_ref(func),
        "tabled_notes": tabled_notes(func),
        "existing_regfix_rules": existing_regfix(func),
        "src_neighbors": src_neighbors(func, src_relpath),
    }


def render_text(b: dict) -> str:
    out = []
    out.append(f"AGENT BRIEF: {b['func']}")

    if b.get("function_state"):
        out.append(section("Function state"))
        out.append(b["function_state"])

    if b.get("m2c_reading"):
        out.append(section("READ ME FIRST: m2c's structural reading"))
        out.append(
            "m2c (the decompiler that generated this function's base.c) often "
            "reads the original C's expression-level shape better than an "
            "agent eyeballing the asm. Below are observations from m2c's "
            "output that frequently matter for matching:")
        out.append("")
        out.append(b["m2c_reading"])

    out.append(section("Classification"))
    out.append(b["classify"] or "(classify failed)")

    if b["src_neighbors"]:
        out.append(section("Source neighbors (subsystem context)"))
        out.append(b["src_neighbors"])

    if b["kengo_metadata"] or b["kengo"]:
        out.append(section("Kengo PS2 sequel reference"))
        if b["kengo_metadata"]:
            out.append(b["kengo_metadata"])
            out.append("")
        if b["kengo"]:
            out.append(b["kengo"])

    if b["kengo_callee_hints"]:
        out.append(section("Kengo-derived names for anonymous BB2 callees"))
        out.append("These are call targets in this function that are still "
                   "`func_XXXX` in BB2 but have a confident Kengo equivalent. "
                   "Useful for naming and understanding what each call does.")
        out.append("")
        out.append(b["kengo_callee_hints"])

    if b["tabled_notes"]:
        out.append(section("Prior tabled-attempt notes"))
        out.append(b["tabled_notes"])

    if b["existing_regfix_rules"]:
        out.append(section("Existing regfix rules for this function"))
        out.append(b["existing_regfix_rules"])

    out.append(section("Target assembly"))
    out.append(b["asm_body"])

    out.append(section("m2c output (current permuter/<func>/base.c)"))
    out.append(b["base_c"])

    out.append(section("Live pipeline diff (gen_regfix)"))
    out.append(b["gen_regfix"])

    if b["recipe_suggestions"] and "no recipe matches" not in b["recipe_suggestions"]:
        out.append(section("Recipe library suggestions"))
        out.append(b["recipe_suggestions"])

    out.append(section("Standard workflow"))
    out.append("\n".join([
        "  dc.sh attempt <func>            # full mechanical pipeline (classify -> setup -> smart -> permute -> regfix)",
        "  dc.sh near-miss <func> --apply  # auto-converter (byte_arith / drain_delay / plain reg substs)",
        "  dc.sh debug permuter/<func>     # full permuter --debug diff",
        "  dc.sh add-regfix <func> <op> <args>  # validated regfix rule editor",
        "  dc.sh inline-replace <func> <c>      # integrate matched C body into src/",
        "  dc.sh verify <func>             # binary-level verify against original",
    ]))
    return "\n".join(out)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("--no-asm", action="store_true",
                    help="Skip the raw target assembly section")
    ap.add_argument("--asm-max-lines", type=int, default=60,
                    help="Truncate asm if longer than N lines (default: 60; was 200)")
    ap.add_argument("--full", action="store_true",
                    help="Disable all truncation (asm + base.c + gen_regfix)")
    ap.add_argument("--json", action="store_true",
                    help="Emit a machine-readable JSON dict instead of text")
    args = ap.parse_args()

    if args.full:
        args.asm_max_lines = 10000
    # Resolve raw `func_<addr>` queue names to the canonical asm/funcs/
    # stem if the function has been renamed-in-asm. Without this the
    # brief comes back empty ("no asm/funcs/<raw>.s") for any rename.
    sys.path.insert(0, str(TOOLS))
    from canonical_funcname import canonicalize
    func = canonicalize(args.func)
    if func != args.func:
        print(f"(input '{args.func}' resolved to canonical '{func}' from asm/funcs/)",
              file=sys.stderr)
    b = build_brief(func, include_asm=not args.no_asm,
                    asm_max_lines=args.asm_max_lines)
    if args.json:
        print(json.dumps(b, indent=2))
    else:
        print(render_text(b))
    return 0


if __name__ == "__main__":
    sys.exit(main())
