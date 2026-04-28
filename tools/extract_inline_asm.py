#!/usr/bin/env python3
"""Extract / locate / replace inline `__asm__()` blocks in BB2 source files.

Many of the 399 inline-asm functions live in src/*.c as raw `__asm__("glabel
<name>...")` blocks. The existing permuter pipeline expects the function body
to be a real C function, not an asm block, so `permuter_setup.sh`'s cpp+strip
path fails on these. This tool fills that gap.

Subcommands:

    locate <func>
        Print the .c file and the [start_line, end_line) range of the inline
        asm block defining <func>. Exit 1 if not found.

    extract <func> [--out FILE]
        Extract the asm body from the inline block to clean MIPS asm (one
        instruction per line, with .L<addr>: labels). Useful when no
        asm/funcs/<func>.s exists.

    verify <func>
        Compare the inline-asm body against asm/funcs/<func>.s. Reports any
        instruction-level mismatch. Exit 0 if equivalent.

    setup <func>
        Set up permuter/<func>/ for an inline-asm function. Uses
        asm/funcs/<func>.s as the target if present (preferred); otherwise
        falls back to inline extraction. Generates base.c via m2c, copies
        compile.sh, writes settings.toml. Same shape as permuter_setup.sh
        but works for inline-asm functions.

    replace <func> <c_file>
        Replace the inline `__asm__()` block in src/ with the C function from
        <c_file>. Used after a match. Atomic — writes through a tempfile, LF
        endings.

Exit codes: 0 = success, 1 = not found / wrong shape, 2 = error.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = ROOT / "src"
ASM_FUNCS = ROOT / "asm" / "funcs"
PERMUTER_DIR = ROOT / "permuter"
M2C = ROOT / "tools" / "m2c" / "m2c.py"
M2C_CONTEXT = ROOT / "include" / "m2c_context.h"
PERMUTER_COMPILE = ROOT / "tools" / "permuter_compile.sh"
STRUCT_FIX = ROOT / "tools" / "struct_copy_fix.py"
FIX_GTE = ROOT / "tools" / "fix_gte_asm.py"

ASM_BLOCK_START = re.compile(r'^\s*__asm__\s*\(\s*$')
ASM_BLOCK_END = re.compile(r'^\s*\)\s*;\s*$')
GLABEL_RE = re.compile(r'glabel\s+(\w+)')

# Captures the C-quoted asm content: "<body>\n"
# The body may contain \t, spaces, register sigils, etc.
QUOTED_LINE = re.compile(r'^\s*"(.*)"\s*$')


def find_block(src_path, func_name):
    """Find the [start_line, end_line] (1-based, inclusive) of the inline
    `__asm__(...)` block defining `func_name`. Returns None if not found."""
    with open(src_path, encoding="utf-8") as f:
        lines = f.readlines()

    in_block = False
    start = None
    block_func = None
    for i, line in enumerate(lines, 1):
        if not in_block:
            if ASM_BLOCK_START.match(line):
                in_block = True
                start = i
                block_func = None
            continue
        m = GLABEL_RE.search(line)
        if m and block_func is None:
            block_func = m.group(1)
        if ASM_BLOCK_END.match(line):
            if block_func == func_name:
                return (start, i, lines)
            in_block = False
            start = None
            block_func = None
    return None


def find_block_in_any_src(func_name):
    """Search every src/*.c for the function's inline asm block.
    Returns (src_path, start, end, lines) or None."""
    for src in sorted(SRC_DIR.glob("*.c")):
        result = find_block(src, func_name)
        if result is not None:
            start, end, lines = result
            return (src, start, end, lines)
    return None


def parse_asm_body(block_lines):
    """Given the raw lines of an `__asm__(...)` block (start..end inclusive),
    extract the MIPS asm instructions and labels as plain text. Strips the
    .set directives and the C-quoting machinery."""
    out = []
    for raw in block_lines[1:-1]:  # skip __asm__( and );
        m = QUOTED_LINE.match(raw)
        if not m:
            continue
        body = m.group(1)
        # Strip trailing \n escape
        if body.endswith("\\n"):
            body = body[:-2]
        # Decode common backslash escapes
        body = body.replace("\\t", "\t").replace("\\\"", "\"")
        stripped = body.strip()
        # Drop the toolchain .set noise — we'll add canonical ones later
        if stripped.startswith(".set"):
            continue
        out.append(body)
    return out


def _imm_to_int(tok):
    """Parse a possibly-signed hex or decimal literal. Return None if not numeric."""
    t = tok.strip()
    sign = 1
    if t.startswith("-"):
        sign = -1
        t = t[1:]
    elif t.startswith("+"):
        t = t[1:]
    try:
        if t.lower().startswith("0x"):
            return sign * int(t, 16)
        return sign * int(t, 10)
    except ValueError:
        return None


def normalize_insn(line):
    """Normalize an asm line for verify-against-target. Drops comments,
    collapses whitespace, normalizes hex/decimal immediates, removes
    whitespace around commas. Keeps labels intact."""
    s = line.strip()
    s = re.sub(r"#.*$", "", s)
    s = re.sub(r"/\*.*?\*/", "", s)
    s = re.sub(r"\s+", " ", s).strip()
    s = re.sub(r"\s*,\s*", ",", s)
    s = re.sub(r"\s*\(\s*", "(", s)
    s = re.sub(r"\s*\)\s*", ")", s)
    # Normalize numeric immediates (-0x1 / -1, 0x10 / 16) to plain int form
    def repl(m):
        v = _imm_to_int(m.group(0))
        return str(v) if v is not None else m.group(0)
    s = re.sub(r"-?(?:0x[0-9a-fA-F]+|\d+)", repl, s)
    return s


def asm_funcs_lines(func_name):
    """Read asm/funcs/<func>.s and return the textual instruction lines
    (with /* */ address comments stripped, glabel/endlabel kept as labels)."""
    p = ASM_FUNCS / f"{func_name}.s"
    if not p.exists():
        return None
    out = []
    for raw in p.read_text(encoding="utf-8").splitlines():
        # Strip /* OFFS VRAM ENC */ comments
        body = re.sub(r"/\*[^*]*\*/", "", raw)
        body = body.strip()
        if not body:
            continue
        # Convert glabel / endlabel into asm-style labels for comparison
        m = re.match(r"glabel\s+(\w+)", body)
        if m:
            out.append(f"{m.group(1)}:")
            continue
        if body.startswith("endlabel"):
            continue
        out.append(body)
    return out


def cmd_locate(args):
    hit = find_block_in_any_src(args.func)
    if hit is None:
        print(f"ERROR: no inline __asm__ block for {args.func} in src/", file=sys.stderr)
        return 1
    src, start, end, _ = hit
    print(f"{src.relative_to(ROOT)}:{start}-{end}")
    return 0


def cmd_extract(args):
    hit = find_block_in_any_src(args.func)
    if hit is None:
        print(f"ERROR: no inline __asm__ block for {args.func} in src/", file=sys.stderr)
        return 1
    src, start, end, lines = hit
    body = parse_asm_body(lines[start - 1:end])
    out = "\n".join(body) + "\n"
    if args.out:
        Path(args.out).write_text(out, encoding="utf-8", newline="\n")
        print(f"Wrote {args.out} ({len(body)} lines)", file=sys.stderr)
    else:
        sys.stdout.write(out)
    return 0


def cmd_verify(args):
    hit = find_block_in_any_src(args.func)
    if hit is None:
        print(f"ERROR: no inline __asm__ block for {args.func} in src/", file=sys.stderr)
        return 1
    src, start, end, lines = hit
    inline = [normalize_insn(s) for s in parse_asm_body(lines[start - 1:end])]
    inline = [s for s in inline if s and not s.startswith(".set")]
    target = asm_funcs_lines(args.func)
    if target is None:
        print(f"ERROR: asm/funcs/{args.func}.s not found", file=sys.stderr)
        return 1
    target = [normalize_insn(s) for s in target]
    target = [s for s in target if s]

    # Strip the leading `<func>:` label for comparison
    if target and target[0].startswith(f"{args.func}:"):
        target = target[1:]
    # Strip the leading glabel asm directive if present
    if inline and inline[0].startswith("glabel"):
        inline = inline[1:]

    if inline == target:
        print(f"{args.func}: inline asm matches asm/funcs/{args.func}.s ({len(inline)} insns)")
        return 0

    # Show first differing line
    n = min(len(inline), len(target))
    for i in range(n):
        if inline[i] != target[i]:
            print(f"DIFF at insn {i}:")
            print(f"  inline: {inline[i]}")
            print(f"  target: {target[i]}")
            break
    if len(inline) != len(target):
        print(f"LENGTH: inline={len(inline)} target={len(target)}")
    return 1


def write_target_s(func_name, dest):
    """Write a permuter-friendly target.s for func_name. Prefers
    asm/funcs/<func>.s; falls back to inline-asm extraction."""
    p = ASM_FUNCS / f"{func_name}.s"
    if p.exists():
        # Replicate permuter_setup.sh's transformation:
        #   glabel X -> X:, drop endlabel, strip /* */ comments
        with open(p, encoding="utf-8") as f:
            raw = f.read()
        lines = []
        for line in raw.splitlines():
            line = re.sub(r"^glabel (.*)", r"\1:", line)
            if line.strip().startswith("endlabel"):
                continue
            line = re.sub(r"/\*[^*]*\*/  ", "", line)
            lines.append(line)
        body = "\n".join(lines)
    else:
        hit = find_block_in_any_src(func_name)
        if hit is None:
            return False
        src, start, end, src_lines = hit
        parsed = parse_asm_body(src_lines[start - 1:end])
        # Convert glabel <name> -> <name>:
        out = []
        for line in parsed:
            line = re.sub(r"glabel\s+(\w+)", r"\1:", line)
            out.append(line)
        body = "\n".join(out)
    header = ".set noat\n.set noreorder\n\n.text\n\n"
    Path(dest).write_text(header + body + "\n", encoding="utf-8", newline="\n")
    return True


def run(cmd, **kwargs):
    return subprocess.run(cmd, capture_output=True, text=True, cwd=str(ROOT), **kwargs)


# Shared post-processor (handles m2c warning leak, M2C_* macros, pseudo-var
# decls, undeclared GameObj/SVECTOR types, unary-* fixes, externs)
sys.path.insert(0, str(Path(__file__).resolve().parent))
from m2c_post import make_compilable as _mp_make_compilable
from m2c_post import TYPEDEFS  # re-export


def stub_base(func):
    return (
        f"// TODO: m2c failed for {func}; decompile manually\n"
        + TYPEDEFS
        + f"\nvoid {func}(void) {{\n    // TODO\n}}\n"
    )


def postprocess_m2c(c_code, func_name):
    """Make m2c output compile in the permuter sandbox. See m2c_post.py."""
    return _mp_make_compilable(c_code, func_name, include_gte=False)


def cmd_setup(args):
    func = args.func
    work = PERMUTER_DIR / func
    work.mkdir(parents=True, exist_ok=True)

    # 1. Build target.s (prefer asm/funcs/<func>.s).
    target_s = work / "target.s"
    if not write_target_s(func, target_s):
        print(f"ERROR: cannot stage target.s for {func}", file=sys.stderr)
        return 1

    # 2. Fix GTE/COP2 mnemonics so GNU as accepts them
    if FIX_GTE.exists():
        run([sys.executable, str(FIX_GTE), str(target_s)])

    # 3. Assemble target.o
    target_o = work / "target.o"
    r = run([
        "mipsel-linux-gnu-as", "-march=r3000", "-mtune=r3000",
        "-no-pad-sections", "-O1", "-G0",
        str(target_s), "-o", str(target_o),
    ])
    if r.returncode != 0:
        print(f"WARN: target.s did not assemble cleanly:\n{r.stderr}", file=sys.stderr)

    # Validate .text is non-empty
    r = run(["mipsel-linux-gnu-objdump", "-h", str(target_o)])
    text_size = None
    for line in r.stdout.splitlines():
        if " .text " in line:
            parts = line.split()
            if len(parts) >= 3:
                text_size = parts[2]
                break
    if text_size in (None, "00000000"):
        print("ERROR: target.o has empty .text section — assembly probably dropped instructions",
              file=sys.stderr)
        return 1

    # 4. Generate base.c via m2c
    base_c = work / "base.c"
    asm_for_m2c = ASM_FUNCS / f"{func}.s"
    if not asm_for_m2c.exists():
        # Fall back to extracting inline asm to a temp asm file
        tmp_asm = work / "_inline_extract.s"
        if not write_target_s(func, tmp_asm):
            print(f"ERROR: cannot extract asm for {func}", file=sys.stderr)
            return 1
        asm_for_m2c = tmp_asm

    m2c_cmd = [sys.executable, str(M2C), "--valid-syntax", "-t", "mipsel-gcc-c"]
    if M2C_CONTEXT.exists():
        m2c_cmd += ["--context", str(M2C_CONTEXT)]
    m2c_cmd.append(str(asm_for_m2c))
    r = run(m2c_cmd, timeout=60)
    base_text = r.stdout.strip()
    if r.returncode != 0 or func not in base_text:
        # Stub fallback
        base_text = stub_base(func)
    else:
        # Apply struct copy fix if available
        if STRUCT_FIX.exists():
            r2 = subprocess.run(
                [sys.executable, str(STRUCT_FIX), "/dev/stdin", "--asm", str(asm_for_m2c)],
                input=base_text, capture_output=True, text=True, cwd=str(ROOT),
            )
            if r2.returncode == 0 and r2.stdout.strip():
                base_text = r2.stdout
        base_text = postprocess_m2c(base_text, func)

    base_c.write_text(base_text, encoding="utf-8", newline="\n")

    # 5. compile.sh + settings.toml
    if PERMUTER_COMPILE.exists():
        shutil.copy(str(PERMUTER_COMPILE), str(work / "compile.sh"))
        os.chmod(str(work / "compile.sh"), 0o755)
    (work / "settings.toml").write_text("compiler = \"gcc\"\n",
                                        encoding="utf-8", newline="\n")

    print(f"Set up permuter/{func}/ — target.s, target.o, base.c, compile.sh, settings.toml")
    print(f"  base.c size: {len(base_text)} bytes")
    return 0


def cmd_replace(args):
    func = args.func
    c_path = Path(args.c_file)
    if not c_path.exists():
        print(f"ERROR: {c_path} not found", file=sys.stderr)
        return 2
    hit = find_block_in_any_src(func)
    if hit is None:
        print(f"ERROR: no inline __asm__ block for {func} in src/", file=sys.stderr)
        return 1
    src, start, end, lines = hit

    new_body = c_path.read_text(encoding="utf-8").rstrip()
    # Drop a trailing newline-less newline so we end with \n
    new_body = new_body.replace("\r\n", "\n").rstrip() + "\n"

    new_lines = lines[:start - 1] + [new_body] + lines[end:]
    out_text = "".join(new_lines)
    # Atomic write through tempfile, LF endings
    tmp = src.with_suffix(src.suffix + ".tmp")
    tmp.write_bytes(out_text.replace("\r\n", "\n").encode("utf-8"))
    tmp.replace(src)
    print(f"Replaced inline __asm__({func}) in {src.relative_to(ROOT)} "
          f"(was lines {start}-{end}, now {len(new_body.splitlines())} lines)")
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("locate", help="Print src file and line range of inline asm")
    p.add_argument("func")
    p.set_defaults(func_=cmd_locate)

    p = sub.add_parser("extract", help="Extract inline asm to plain MIPS asm")
    p.add_argument("func")
    p.add_argument("--out", default=None)
    p.set_defaults(func_=cmd_extract)

    p = sub.add_parser("verify", help="Verify inline asm matches asm/funcs/<func>.s")
    p.add_argument("func")
    p.set_defaults(func_=cmd_verify)

    p = sub.add_parser("setup", help="Set up permuter/<func>/ for inline-asm function")
    p.add_argument("func")
    p.set_defaults(func_=cmd_setup)

    p = sub.add_parser("replace", help="Replace inline __asm__ block with matched C")
    p.add_argument("func")
    p.add_argument("c_file")
    p.set_defaults(func_=cmd_replace)

    args = ap.parse_args()
    return args.func_(args)


if __name__ == "__main__":
    sys.exit(main())
