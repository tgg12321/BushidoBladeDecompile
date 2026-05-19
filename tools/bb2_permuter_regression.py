#!/usr/bin/env python3
"""
bb2_permuter_regression.py -- Phase 0 regression suite for the targeted permuter.

Goal: prove that for each function in the hand-picked suite,
  (a) we can locate its inline-move-aliasing blocks
  (b) stripping the aliasing breaks the byte match (load-bearing check)
  (c) the EXISTING decomp-permuter does NOT reconstruct it from the stripped baseline
      (if it does, that function is too easy and shouldn't be in the regression set)

Once Phase 0 establishes a real regression set, Phase 1 builds the
auto-aliasing pass and re-runs the suite to confirm reconstruction.

Subcommands:
  strip <func>          Strip aliasing from one function. Writes
                        regression/<func>/{expected.c, stripped.c, metadata.json}.
  strip-all             Run strip on every suite function.
  show <func>           Print the located aliasing blocks (for inspection).
  load-bearing <func>   Replace src/ with stripped version, rebuild, confirm
                        SHA1 breaks. Restores src/ afterwards. (Run from main
                        repo path -- needs the full build pipeline.)
  baseline <func>       Run upstream decomp-permuter on stripped.c, save the
                        best score reached. (Run from main repo path.)
  report                Aggregate results across the suite.

Design constraints (locked in docs/targeted_permuter_plan.md):
  - Pin set scope: $t0-$t9 + $v0-$v1 + $s0-$s7 (20 regs)
  - Output mode: save to permuter/<func>/output-*/, no auto-integration
  - Scoring: mips-gcc only
  - No audit hook in Phase 0/1.
"""
from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path


# -- CRLF-safe file I/O -------------------------------------------------------
# Per project AGENTS.md + memory/workflow/crlf-via-edit-write.md: build files
# in src/, regfix.txt, etc. MUST stay LF. Python's default text-mode I/O on
# Windows silently converts LF -> CRLF on write, breaking the toolchain
# (maspsx + cc1 + the SHA1 cascade). All file I/O in this tool uses binary
# mode so the round-trip is byte-perfect.

def read_text_lf(path: Path) -> str:
    """Read a file as text, preserving its original line endings."""
    return path.read_bytes().decode("utf-8", errors="ignore")


def write_text_lf(path: Path, content: str) -> None:
    """Write text without any Windows CRLF translation. Caller is responsible
    for ensuring `content` has the desired line endings (typically '\\n').
    """
    path.write_bytes(content.encode("utf-8"))

# -- Regression suite ---------------------------------------------------------

SUITE = [
    {"func": "AddTbpOfst_80047EE8",     "file": "src/text1b.c",      "tier": "core",   "expected_blocks": 1},
    {"func": "func_80019310",           "file": "src/code6cac.c",    "tier": "core",   "expected_blocks": 1},
    {"func": "calc_fc_frame_800203B4",  "file": "src/code6cac.c",    "tier": "core",   "expected_blocks": 3},
    {"func": "func_8002D320",           "file": "src/code6cac_b.c",  "tier": "core",   "expected_blocks": 2},
    {"func": "func_8002EA24",           "file": "src/code6cac_b.c",  "tier": "core",   "expected_blocks": 2},
    {"func": "saSeInit",                "file": "src/code6cac_b.c",  "tier": "stretch","expected_blocks": 4},
    {"func": "DispSchoolBG",            "file": "src/code6cac_b.c",  "tier": "stretch","expected_blocks": 6},
]


def get_entry(func: str) -> dict | None:
    return next((e for e in SUITE if e["func"] == func), None)


# -- Function body location ---------------------------------------------------

def find_function_body(src: str, func: str) -> tuple[int, int] | None:
    """Locate `func` in `src` and return (start_line, end_line) of the function
    body (0-indexed, inclusive of the def line and the closing brace).

    Returns None if not found.
    """
    lines = src.split("\n")
    # Match the function definition line. Permissive: allow leading `static`,
    # `register`, etc., and pointer-return types. We anchor on the function
    # name followed by `(`.
    pattern = re.compile(rf"^\s*[\w\s\*]+?\b{re.escape(func)}\s*\(")
    start = None
    for i, line in enumerate(lines):
        # Skip forward declarations (terminated by `;` on same line)
        if pattern.match(line) and ";" not in line:
            start = i
            break
    if start is None:
        return None

    # Find opening brace (may be on same line or next)
    brace_line = start
    while brace_line < len(lines) and "{" not in lines[brace_line]:
        brace_line += 1
    if brace_line >= len(lines):
        return None

    # Brace-count forward to find matching close
    depth = 0
    in_block_comment = False
    end = None
    for j in range(brace_line, len(lines)):
        line = lines[j]
        # Strip comments to avoid braces inside them confusing the counter.
        # We do a stateful pass to handle /* ... */ that spans multiple lines.
        cleaned = []
        k = 0
        while k < len(line):
            if in_block_comment:
                idx = line.find("*/", k)
                if idx == -1:
                    break
                in_block_comment = False
                k = idx + 2
            else:
                if line.startswith("/*", k):
                    in_block_comment = True
                    k += 2
                elif line.startswith("//", k):
                    break
                else:
                    cleaned.append(line[k])
                    k += 1
        cleaned_str = "".join(cleaned)
        # Also strip string literals (cheap: any "...")
        cleaned_str = re.sub(r'"[^"]*"', '""', cleaned_str)
        cleaned_str = re.sub(r"'[^']*'", "''", cleaned_str)
        for ch in cleaned_str:
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    end = j
                    break
        if end is not None:
            break

    if end is None:
        return None
    return (start, end)


# -- Aliasing block detection -------------------------------------------------

ALIAS_ASM_RE = re.compile(
    r'__asm__\s+(?:volatile|__volatile__)\s*\(\s*'
    r'"'
    r'(?:move\s+%0\s*,\s*%1'
    r'|addu\s+%0\s*,\s*%1\s*,\s*\$zero)'
    r'"'
    r'\s*:\s*"=r"\s*\(\s*(\w+)\s*\)\s*'
    r':\s*"r"\s*\(\s*(\w+)\s*\)'
)

REGISTER_PIN_RE = re.compile(
    r'^(\s*)register\s+'
    r'(.+?)'              # type (anything non-greedy; may include *, const, volatile)
    r'\b(\w+)\s+'         # variable name (last word before `asm`)
    r'asm\s*\(\s*"\$?(\w+)"\s*\)\s*'
    r'(?:=\s*([^;]+))?'   # optional init
    r';'
)


def find_aliasing_blocks(body_lines: list[str]) -> list[dict]:
    """Find every inline-move-aliasing block in the function body lines.

    Returns a list of block-dicts with keys:
      asm_line       (int)       line index of __asm__ line in body
      asm_text       (str)       full original line text
      dst_var        (str)       destination C variable name
      src_var        (str)       source C variable name
      dst_pin_line   (int|None)  line index of the register pin for dst
      src_pin_line   (int|None)  line index of the register pin for src
      dst_pin_text   (str|None)
      src_pin_text   (str|None)
      dst_pin_reg    (str|None)  the pinned register, e.g. "12" or "t4"
      src_pin_reg    (str|None)
    """
    blocks = []
    for i, line in enumerate(body_lines):
        m = ALIAS_ASM_RE.search(line)
        if not m:
            continue
        dst, src = m.group(1), m.group(2)
        # Walk backwards up to 30 lines to find the register pin declarations
        # that bound dst and src. We accept pins anywhere in the function above
        # the asm line, but cap the search to 30 lines for performance.
        dst_pin_line, src_pin_line = None, None
        dst_pin_text, src_pin_text = None, None
        dst_pin_reg, src_pin_reg = None, None
        for j in range(i - 1, max(-1, i - 60), -1):
            pm = REGISTER_PIN_RE.match(body_lines[j])
            if not pm:
                continue
            pvar = pm.group(3)
            preg = pm.group(4)
            if pvar == dst and dst_pin_line is None:
                dst_pin_line = j
                dst_pin_text = body_lines[j]
                dst_pin_reg = preg
            elif pvar == src and src_pin_line is None:
                src_pin_line = j
                src_pin_text = body_lines[j]
                src_pin_reg = preg
            if dst_pin_line is not None and src_pin_line is not None:
                break
        blocks.append({
            "asm_line": i,
            "asm_text": line,
            "dst_var": dst,
            "src_var": src,
            "dst_pin_line": dst_pin_line,
            "src_pin_line": src_pin_line,
            "dst_pin_text": dst_pin_text,
            "src_pin_text": src_pin_text,
            "dst_pin_reg": dst_pin_reg,
            "src_pin_reg": src_pin_reg,
        })
    return blocks


# -- Stripping ----------------------------------------------------------------

def strip_aliasing(body_lines: list[str], blocks: list[dict]) -> str:
    """Produce a stripped version of the body:
      - replace each asm line with `dst = src;` (semantic-equivalent)
      - remove the `register asm("$N")` annotations from dst/src pin lines
        but keep any initializer expression intact.

    Also strips any leading INLINE_MOVE_ALIASING comment block that documents
    the aliasing (best-effort -- looks for the comment header within 12 lines
    above the asm).
    """
    asm_lines = {b["asm_line"]: b for b in blocks}
    pin_lines: dict[int, str] = {}
    for b in blocks:
        if b["dst_pin_line"] is not None:
            pin_lines[b["dst_pin_line"]] = b["dst_var"]
        if b["src_pin_line"] is not None:
            pin_lines[b["src_pin_line"]] = b["src_var"]

    # Detect INLINE_MOVE_ALIASING comment blocks to also strip.
    comment_drop = set()
    for b in blocks:
        # Walk up from the asm line; find the start of a `/* INLINE_MOVE_ALIASING:`
        # comment, and drop all lines from its start through its `*/` end.
        i = b["asm_line"] - 1
        while i >= 0 and i >= b["asm_line"] - 12:
            line = body_lines[i]
            if "INLINE_MOVE_ALIASING" in line:
                # Walk further up to the start of the comment
                start = i
                while start > 0 and "/*" not in body_lines[start]:
                    start -= 1
                # Walk forward from start to find the */
                end = start
                while end < len(body_lines) and "*/" not in body_lines[end]:
                    end += 1
                for k in range(start, min(end + 1, len(body_lines))):
                    comment_drop.add(k)
                break
            i -= 1

    out = []
    for i, line in enumerate(body_lines):
        if i in comment_drop:
            continue
        if i in asm_lines:
            b = asm_lines[i]
            indent = re.match(r"^(\s*)", line).group(1)
            out.append(f"{indent}{b['dst_var']} = {b['src_var']};")
            continue
        if i in pin_lines:
            m = REGISTER_PIN_RE.match(line)
            if m:
                indent = m.group(1)
                typestr = m.group(2).strip()
                varname = m.group(3)
                # group(5) is the init expression (or None)
                init = m.group(5)
                if init is not None:
                    out.append(f"{indent}{typestr} {varname} = {init.strip()};")
                else:
                    out.append(f"{indent}{typestr} {varname};")
                continue
        out.append(line)
    return "\n".join(out)


# -- Subcommand handlers ------------------------------------------------------

def _read_body(root: Path, entry: dict) -> tuple[list[str], int, int]:
    """Read the function body lines and return (body_lines, start, end)."""
    src_path = root / entry["file"]
    if not src_path.exists():
        raise FileNotFoundError(f"{src_path} not found")
    src = read_text_lf(src_path)
    rng = find_function_body(src, entry["func"])
    if rng is None:
        raise RuntimeError(f"Could not locate {entry['func']} in {src_path}")
    start, end = rng
    lines = src.split("\n")
    return lines[start:end + 1], start, end


def cmd_show(args):
    root = Path(args.project_root).resolve()
    entry = get_entry(args.func)
    if entry is None:
        sys.exit(f"{args.func} not in regression suite")
    body_lines, start, end = _read_body(root, entry)
    blocks = find_aliasing_blocks(body_lines)
    print(f"# {entry['func']} ({entry['file']}, tier={entry['tier']})")
    print(f"# Function body: src lines {start + 1}..{end + 1}  ({end - start + 1} lines)")
    print(f"# Expected aliasing blocks: {entry['expected_blocks']}")
    print(f"# Found aliasing blocks:    {len(blocks)}")
    if len(blocks) != entry["expected_blocks"]:
        print(f"# WARNING: count mismatch (suite says {entry['expected_blocks']}, scan found {len(blocks)})")
    for b in blocks:
        asm_abs = start + b["asm_line"] + 1
        print()
        print(f"  block @ src line {asm_abs}:")
        print(f"    asm        : {b['asm_text'].strip()}")
        print(f"    dst var    : {b['dst_var']}  (pin=${b['dst_pin_reg']})" if b["dst_pin_reg"] else f"    dst var    : {b['dst_var']}  (no pin found)")
        print(f"    src var    : {b['src_var']}  (pin=${b['src_pin_reg']})" if b["src_pin_reg"] else f"    src var    : {b['src_var']}  (no pin found)")
        if b["dst_pin_text"]:
            print(f"    dst pin    : line {start + b['dst_pin_line'] + 1}: {b['dst_pin_text'].strip()}")
        if b["src_pin_text"]:
            print(f"    src pin    : line {start + b['src_pin_line'] + 1}: {b['src_pin_text'].strip()}")


def cmd_strip(args):
    root = Path(args.project_root).resolve()
    entry = get_entry(args.func)
    if entry is None:
        sys.exit(f"{args.func} not in regression suite")
    body_lines, start, end = _read_body(root, entry)
    blocks = find_aliasing_blocks(body_lines)
    if not blocks:
        sys.exit(f"No aliasing blocks found in {args.func}")

    expected_body = "\n".join(body_lines)
    stripped_body = strip_aliasing(body_lines, blocks)

    out_dir = root / "regression" / args.func
    out_dir.mkdir(parents=True, exist_ok=True)

    write_text_lf(out_dir / "expected.c", expected_body)
    write_text_lf(out_dir / "stripped.c", stripped_body)

    metadata = {
        "func": args.func,
        "file": entry["file"],
        "tier": entry["tier"],
        "expected_blocks": entry["expected_blocks"],
        "found_blocks": len(blocks),
        "src_line_start": start + 1,
        "src_line_end": end + 1,
        "blocks": [
            {
                "asm_line_relative": b["asm_line"],
                "asm_line_absolute": start + b["asm_line"] + 1,
                "dst_var": b["dst_var"],
                "src_var": b["src_var"],
                "dst_pin_reg": b["dst_pin_reg"],
                "src_pin_reg": b["src_pin_reg"],
                "dst_pin_line_absolute": (start + b["dst_pin_line"] + 1) if b["dst_pin_line"] is not None else None,
                "src_pin_line_absolute": (start + b["src_pin_line"] + 1) if b["src_pin_line"] is not None else None,
            }
            for b in blocks
        ],
    }
    write_text_lf(out_dir / "metadata.json", json.dumps(metadata, indent=2))

    print(f"{args.func}: {len(blocks)} aliasing block(s) found, {len(blocks)} stripped")
    print(f"  expected.c  : {len(expected_body)} bytes  ({end - start + 1} lines)")
    print(f"  stripped.c  : {len(stripped_body)} bytes")
    print(f"  metadata    : {out_dir / 'metadata.json'}")


def cmd_strip_all(args):
    for entry in SUITE:
        args.func = entry["func"]
        try:
            cmd_strip(args)
        except SystemExit as e:
            print(f"  SKIP {entry['func']}: {e}")
        except Exception as e:
            print(f"  ERROR {entry['func']}: {e}")


def cmd_diff(args):
    """Print diff between expected.c and stripped.c for inspection."""
    root = Path(args.project_root).resolve()
    out_dir = root / "regression" / args.func
    exp = out_dir / "expected.c"
    strip = out_dir / "stripped.c"
    if not exp.exists() or not strip.exists():
        sys.exit(f"Run `strip {args.func}` first")
    subprocess.run(["diff", "-u", str(exp), str(strip)])


# -- Load-bearing check -------------------------------------------------------

def _splice_function(src_path: Path, func: str, new_body: str) -> str:
    """Splice new_body into src_path replacing func's body. Returns the
    original src content so the caller can restore.

    Uses binary I/O via read_text_lf/write_text_lf to preserve LF line
    endings (Windows text-mode Python would convert LF -> CRLF on write
    and break the build pipeline).
    """
    src = read_text_lf(src_path)
    rng = find_function_body(src, func)
    if rng is None:
        raise RuntimeError(f"Could not locate {func} in {src_path}")
    start, end = rng
    lines = src.split("\n")
    spliced = lines[:start] + new_body.split("\n") + lines[end + 1:]
    write_text_lf(src_path, "\n".join(spliced))
    return src


def cmd_load_bearing(args):
    """Replace func's body in src/ with stripped.c, run a clean rebuild, and
    confirm the SHA1 BREAKS. Restores src/ afterwards regardless of build
    outcome.

    Must run from a project root with the full build pipeline.
    """
    root = Path(args.project_root).resolve()
    out_dir = root / "regression" / args.func
    if not (out_dir / "stripped.c").exists():
        sys.exit(f"Run `strip {args.func}` first")

    entry = get_entry(args.func)
    src_path = root / entry["file"]
    stripped = read_text_lf(out_dir / "stripped.c")

    print(f"[1/3] Saving original {src_path}...")
    original = read_text_lf(src_path)
    backup = out_dir / "src_backup.c"
    write_text_lf(backup, original)

    try:
        print(f"[2/3] Splicing stripped body into {src_path}...")
        _splice_function(src_path, args.func, stripped)

        # Need a clean .o rebuild for the affected file (cache invalidation)
        # The build command runs through WSL. We point at the project root.
        file_stem = Path(entry["file"]).stem  # e.g., "text1b"
        # Touch the source file so make picks it up; also pre-delete the .o
        # for safety.
        clean_cmd = f"rm -f build/src/{file_stem}.o"

        print(f"[3/3] Running clean rebuild + verify...")
        wsl_root = str(root).replace("C:", "/mnt/c").replace("\\", "/")
        # Spaces in path -> WSL needs proper quoting
        wsl_root_quoted = '"' + wsl_root + '"'
        build_cmd = (
            f'cd {wsl_root_quoted} && {clean_cmd} && '
            f'bash tools/dc.sh build 2>&1 | tail -5'
        )
        result = subprocess.run(
            ["wsl", "bash", "-c", build_cmd],
            capture_output=True,
            text=True,
            timeout=300,
        )
        print(result.stdout)
        if result.stderr:
            print("STDERR:", result.stderr, file=sys.stderr)

        # Check if SHA1 broke. Order matters: "matches!" is the only
        # success signal; everything else is a failure (which is what we
        # WANT for the load-bearing test — we want the stripped version
        # to NOT match the original SHA1).
        out = result.stdout
        if "OK: bb2 matches!" in out or "bb2 matches!" in out:
            verdict = "LOAD_BEARING_FAIL"
            note = "stripped version STILL MATCHES — aliasing was not load-bearing (regfix or other coverage masks?)"
        else:
            verdict = "LOAD_BEARING_OK"
            note = "stripped version breaks the match — aliasing is genuinely load-bearing"

        # Save result
        result_data = {
            "func": args.func,
            "verdict": verdict,
            "note": note,
            "build_output_tail": out.split("\n")[-15:],
        }
        write_text_lf(out_dir / "load_bearing_result.json",
                      json.dumps(result_data, indent=2))
        print(f"\n  Verdict: {verdict}")
        print(f"  Note:    {note}")
    finally:
        print(f"[restore] Restoring original {src_path}...")
        write_text_lf(src_path, original)
        # Sanity check: confirm what we wrote back is byte-identical to the
        # original we read. If not, something went wrong with the round-trip
        # and we want a loud error (NOT a silent CRLF corruption of src/).
        restored = read_text_lf(src_path)
        if restored != original:
            print(
                f"ERROR: src restore is not byte-identical to original "
                f"({len(restored)} bytes restored vs {len(original)} bytes original). "
                f"Backup retained at {backup}.",
                file=sys.stderr,
            )
        else:
            backup.unlink(missing_ok=True)


def cmd_load_bearing_all(args):
    """Run load-bearing check on every suite function. Each test takes
    ~30-60s for a clean partial rebuild + SHA1 verify.
    """
    failed = []
    for entry in SUITE:
        print(f"\n{'=' * 70}")
        print(f"=== {entry['func']} ({entry['tier']})")
        print(f"{'=' * 70}")
        args.func = entry["func"]
        try:
            cmd_load_bearing(args)
        except Exception as e:
            print(f"ERROR: {e}")
            failed.append((entry["func"], str(e)))
    if failed:
        print(f"\n{len(failed)} failures:")
        for f, e in failed:
            print(f"  {f}: {e}")


def cmd_setup_baseline(args):
    """Prepare permuter/<func>_baseline/ with stripped C as base.c.

    Workflow:
      1. Run `dc.sh inline-setup <func>` (which builds target.s, target.o
         from asm/funcs/<func>.s and generates a stub base.c via m2c).
      2. Replace the body of the m2c-generated base.c with our stripped body
         (preserves the standalone wrapping: typedefs, externs, struct decls).
      3. Result: permuter/<func>_baseline/ is ready for decomp-permuter.

    The user then runs upstream decomp-permuter manually with their preferred
    budget:
        python3 tools/decomp-permuter/permuter.py permuter/<func>_baseline \\
            --stop-on-zero --best-only -j 6
    """
    root = Path(args.project_root).resolve()
    entry = get_entry(args.func)
    if entry is None:
        sys.exit(f"{args.func} not in regression suite")

    out_dir = root / "regression" / args.func
    if not (out_dir / "stripped.c").exists():
        sys.exit(f"Run `strip {args.func}` first")
    stripped_body = read_text_lf(out_dir / "stripped.c")

    # 1. Use dc.sh inline-setup to stage the permuter dir
    print(f"[1/3] Running dc.sh inline-setup {args.func}...")
    wsl_root = str(root).replace("C:", "/mnt/c").replace("\\", "/")
    setup_cmd = f'cd "{wsl_root}" && bash tools/dc.sh inline-setup {args.func} 2>&1'
    result = subprocess.run(
        ["wsl", "bash", "-c", setup_cmd],
        capture_output=True,
        text=True,
        timeout=120,
    )
    print(result.stdout.splitlines()[-3:] if result.stdout else "")

    permuter_dir = root / "permuter" / args.func
    if not (permuter_dir / "base.c").exists():
        sys.exit(f"inline-setup failed; permuter/{args.func}/base.c missing")

    # 2. Rename the dir to *_baseline to keep it separate from any active
    #    decomp work on this function.
    baseline_dir = root / "permuter" / f"{args.func}_baseline"
    if baseline_dir.exists():
        shutil.rmtree(baseline_dir)
    shutil.copytree(permuter_dir, baseline_dir)
    print(f"[2/3] Copied to permuter/{args.func}_baseline/")

    # 3. Substitute the m2c-generated function body with our stripped body.
    base_c_path = baseline_dir / "base.c"
    base_c = read_text_lf(base_c_path)
    # Find the function in base.c and replace its body.
    rng = find_function_body(base_c, args.func)
    if rng is None:
        sys.exit(f"Could not locate {args.func} in {base_c_path}")
    start, end = rng
    base_lines = base_c.split("\n")
    spliced = base_lines[:start] + stripped_body.split("\n") + base_lines[end + 1:]
    write_text_lf(base_c_path, "\n".join(spliced))
    print(f"[3/3] Replaced {args.func} body in base.c with stripped version")
    print()
    print(f"Baseline dir ready: {baseline_dir}")
    print(f"Run permuter manually:")
    print(f"  python3 tools/decomp-permuter/permuter.py permuter/{args.func}_baseline \\")
    print(f"      --stop-on-zero --best-only -j 6")


def cmd_baseline_record(args):
    """Run upstream decomp-permuter on permuter/<func>_baseline/ for a
    bounded time and capture the best score reached. Records to
    regression/<func>/baseline_result.json.

    Time budget: passed via --time-seconds (default 180s = 3 min).
    """
    root = Path(args.project_root).resolve()
    entry = get_entry(args.func)
    if entry is None:
        sys.exit(f"{args.func} not in regression suite")

    baseline_dir = root / "permuter" / f"{args.func}_baseline"
    if not (baseline_dir / "base.c").exists():
        sys.exit(f"Run `setup-baseline {args.func}` first")

    out_dir = root / "regression" / args.func
    log_path = out_dir / "baseline_run.log"

    print(f"[1/2] Running upstream permuter on {baseline_dir} for {args.time_seconds}s...")
    wsl_root = str(root).replace("C:", "/mnt/c").replace("\\", "/")
    cmd = (
        f'cd "{wsl_root}" && source .venv/bin/activate && '
        f'timeout {args.time_seconds} python3 tools/decomp-permuter/permuter.py '
        f'permuter/{args.func}_baseline --stop-on-zero --best-only -j 6 2>&1'
    )
    result = subprocess.run(
        ["wsl", "bash", "-c", cmd],
        capture_output=True,
        text=True,
        timeout=args.time_seconds + 60,
    )
    log = result.stdout + "\n--STDERR--\n" + result.stderr
    write_text_lf(log_path, log)

    # Parse best score from log
    scores = re.findall(r"score = (\d+)", log)
    best = min(int(s) for s in scores) if scores else None
    iters = len(re.findall(r"iteration \d+", log))
    base_score = None
    base_m = re.search(r"base score = (\d+)", log)
    if base_m:
        base_score = int(base_m.group(1))

    record = {
        "func": args.func,
        "time_seconds": args.time_seconds,
        "iterations": iters,
        "base_score": base_score,
        "best_score": best,
        "improvement": (base_score - best) if (base_score and best is not None) else None,
        "log_tail": log.split("\n")[-10:],
    }
    write_text_lf(out_dir / "baseline_result.json", json.dumps(record, indent=2))
    print(f"[2/2] Recorded: iterations={iters}, base_score={base_score}, best_score={best}")
    if best == 0:
        print(f"  ⚠ MATCH found by upstream permuter — this function is NOT a valid regression target")
    elif base_score == best:
        print(f"  ✓ Upstream permuter plateaued — this is a real regression target for Phase 1 to beat")
    else:
        print(f"  ~ Upstream improved {base_score} -> {best} but not to 0 — still a regression target")


def cmd_report(args):
    """Print regression suite status across all functions."""
    root = Path(args.project_root).resolve()
    print(f"{'Func':<30} {'Tier':<8} {'Blocks':>6} {'Strip':<10} {'Load-Bearing':<18} {'Baseline':<15}")
    print("-" * 95)
    for entry in SUITE:
        out_dir = root / "regression" / entry["func"]
        meta_path = out_dir / "metadata.json"
        lb_path = out_dir / "load_bearing_result.json"
        baseline_path = out_dir / "baseline_result.json"

        if meta_path.exists():
            meta = json.loads(read_text_lf(meta_path))
            strip_status = f"{meta['found_blocks']}/{meta['expected_blocks']}"
        else:
            meta = None
            strip_status = "not run"

        if lb_path.exists():
            lb = json.loads(read_text_lf(lb_path))
            lb_status = lb["verdict"]
        else:
            lb_status = "not run"

        if baseline_path.exists():
            bl = json.loads(read_text_lf(baseline_path))
            bl_status = f"score={bl.get('best_score', '?')}"
        else:
            bl_status = "not run"

        print(f"{entry['func']:<30} {entry['tier']:<8} {meta.get('found_blocks', '?') if meta_path.exists() else '?':>6} {strip_status:<10} {lb_status:<18} {bl_status:<15}")


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument(
        "--project-root",
        default=".",
        help="Project root to read src/ from (default: cwd). Set this if running "
             "from the worktree but pointing at the main repo path for src/.",
    )
    sub = p.add_subparsers(dest="cmd", required=True)

    p_show = sub.add_parser("show", help="Print aliasing blocks for inspection")
    p_show.add_argument("func")
    p_show.set_defaults(handler=cmd_show)

    p_strip = sub.add_parser("strip", help="Strip aliasing from one function")
    p_strip.add_argument("func")
    p_strip.set_defaults(handler=cmd_strip)

    p_strip_all = sub.add_parser("strip-all", help="Strip all suite functions")
    p_strip_all.set_defaults(handler=cmd_strip_all)

    p_diff = sub.add_parser("diff", help="Show diff(expected, stripped)")
    p_diff.add_argument("func")
    p_diff.set_defaults(handler=cmd_diff)

    p_lb = sub.add_parser("load-bearing", help="Splice stripped into src/, rebuild, verify break, restore")
    p_lb.add_argument("func")
    p_lb.set_defaults(handler=cmd_load_bearing)

    p_lb_all = sub.add_parser("load-bearing-all", help="Run load-bearing across every suite function")
    p_lb_all.set_defaults(handler=cmd_load_bearing_all)

    p_setup = sub.add_parser("setup-baseline", help="Prep permuter/<func>_baseline/ with stripped base.c")
    p_setup.add_argument("func")
    p_setup.set_defaults(handler=cmd_setup_baseline)

    p_brec = sub.add_parser("baseline-record", help="Run upstream permuter on baseline dir, record best score")
    p_brec.add_argument("func")
    p_brec.add_argument("--time-seconds", type=int, default=180, help="Permuter time budget (default 180)")
    p_brec.set_defaults(handler=cmd_baseline_record)

    p_rep = sub.add_parser("report", help="Aggregate regression suite results")
    p_rep.set_defaults(handler=cmd_report)

    args = p.parse_args()
    args.handler(args)


if __name__ == "__main__":
    main()
