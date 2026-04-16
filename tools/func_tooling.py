#!/usr/bin/env python3
"""Shared helpers for per-function decompilation tooling."""

from __future__ import annotations

import difflib
import glob
import os
import re
import shlex
import shutil
import struct
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parent.parent
ORIGINAL_EXE = PROJECT_ROOT / "disc" / "SLUS_006.63"
CC1 = PROJECT_ROOT / "tools" / "gcc-2.7.2" / "build" / "cc1"
MASPSX = PROJECT_ROOT / "tools" / "maspsx" / "maspsx.py"
REGFIX = PROJECT_ROOT / "tools" / "regfix.py"
REGFIX_STAGE2 = PROJECT_ROOT / "regfix_stage2.txt"
ASM_FILE = PROJECT_ROOT / "asm" / "6CAC.s"
UNDEF_FUNCS = PROJECT_ROOT / "undefined_funcs_auto.txt"
UNDEF_SYMS = PROJECT_ROOT / "undefined_syms_auto.txt"
NAMED_SYMS = PROJECT_ROOT / "named_syms.txt"
SRC_DIR = PROJECT_ROOT / "src"
ASM_FUNCS_DIR = PROJECT_ROOT / "asm" / "funcs"
CODEX_LAB_DIR = PROJECT_ROOT / "codex_lab"

CPP = "mipsel-linux-gnu-cpp"
AS = "mipsel-linux-gnu-as"
LD = "mipsel-linux-gnu-ld"
OBJCOPY = "mipsel-linux-gnu-objcopy"
NM = "mipsel-linux-gnu-nm"
OBJDUMP = "mipsel-linux-gnu-objdump"

CPP_FLAGS = ["-Iinclude", "-undef", "-Wall", "-lang-c", "-fno-builtin"]
CPP_DEFS = (
    "-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx "
    "-D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL "
    "-D_LANGUAGE_C -DLANGUAGE_C"
).split()
CC_FLAGS = [
    "-O2",
    "-G0",
    "-funsigned-char",
    "-quiet",
    "-mcpu=3000",
    "-mips1",
    "-mno-abicalls",
    "-fno-builtin",
    "-w",
]
CC_FLAGS_GP = [
    "-O2",
    "-G8",
    "-funsigned-char",
    "-quiet",
    "-mcpu=3000",
    "-mips1",
    "-mno-abicalls",
    "-fno-builtin",
    "-w",
]
AS_FLAGS = ["-Iinclude", "-march=r3000", "-mtune=r3000", "-no-pad-sections", "-O1", "-G0"]
MASPSX_FLAGS = [
    "--expand-div",
    "--aspsx-version=2.34",
    "--sdata-syms=sdata_syms.txt",
    "--sdata-funcs=sdata_funcs.txt",
    "--sdata-exclude=sdata_exclude.txt",
    "--expand-lb",
    "--expand-lb-funcs=expand_lb_funcs.txt",
]
MASPSX_FLAGS_GP = [
    "--expand-div",
    "--aspsx-version=2.34",
    "--dont-force-G0",
    "--sdata-syms=sdata_syms.txt",
    "-G8",
]

LOAD_ADDR = 0x80010000
EXE_HEADER = 0x800

GP_FILES = set()
EXPAND_LB_FILES = {"code6cac_b"}
EXPAND_LH_FILES = set()
RODATA_ALIGN2_FILES = {
    "code6cac",
    "code6cac_b",
    "code6cac_c",
    "code6cac_c0",
    "code6cac_c_ab",
    "code6cac_c2",
    "text1a",
    "text1a_b",
    "text1a_c",
    "main",
}


@dataclass(frozen=True)
class FuncInfo:
    name: str
    addr: int
    size: int
    asm_path: Path | None = None
    source_kind: str = "unknown"


def stem_for_path(path: str | Path) -> str:
    return Path(path).stem


def project_rel_path(path: str | Path) -> str:
    candidate = Path(path)
    try:
        resolved = candidate.resolve()
        try:
            candidate = resolved.relative_to(PROJECT_ROOT)
        except ValueError:
            candidate = resolved
    except OSError:
        pass
    return candidate.as_posix()


def cc_flags_for(path: str | Path) -> list[str]:
    return CC_FLAGS_GP if stem_for_path(path) in GP_FILES else CC_FLAGS


def maspsx_flags_for(path: str | Path) -> list[str]:
    stem = stem_for_path(path)
    flags = list(MASPSX_FLAGS_GP if stem in GP_FILES else MASPSX_FLAGS)
    if stem in EXPAND_LB_FILES and "--expand-lb" not in flags:
        flags.append("--expand-lb")
    if stem in EXPAND_LH_FILES and "--expand-lh" not in flags:
        flags.append("--expand-lh")
    return flags


def parse_func_info_from_asm(path: str | Path) -> FuncInfo | None:
    asm_path = Path(path)
    addrs = []
    for line in asm_path.read_text().splitlines():
        match = re.search(r"/\*\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f]+)\s+", line)
        if match:
            addrs.append(int(match.group(1), 16))
    if not addrs:
        return None
    name = asm_path.stem
    start = addrs[0]
    size = addrs[-1] + 4 - start
    return FuncInfo(name=name, addr=start, size=size, asm_path=asm_path, source_kind="split_asm")


def get_all_func_info() -> dict[str, FuncInfo]:
    funcs: dict[str, FuncInfo] = {}

    content = ASM_FILE.read_text()
    for match in re.finditer(r"nonmatching\s+(\w+)\s*,\s*(0x[0-9A-Fa-f]+|\d+)", content):
        name = match.group(1)
        size = int(match.group(2), 0)
        addr_match = re.match(r"func_([0-9A-Fa-f]+)", name)
        if not addr_match:
            continue
        addr = int(addr_match.group(1), 16)
        funcs[name] = FuncInfo(name=name, addr=addr, size=size, asm_path=ASM_FILE, source_kind="main_asm")

    for path in sorted(glob.glob(str(ASM_FUNCS_DIR / "*.s"))):
        info = parse_func_info_from_asm(path)
        if info is None or info.name in funcs:
            continue
        funcs[info.name] = info

    return funcs


def infer_source_file(func_name: str) -> Path | None:
    body_needle = re.compile(rf"\b{re.escape(func_name)}\s*\(")
    include_asm_needle = re.compile(
        rf'INCLUDE_ASM\("asm/funcs",\s*{re.escape(func_name)}\s*\);'
    )
    matches = []
    for path in sorted(SRC_DIR.glob("*.c")):
        text = path.read_text()
        if body_needle.search(text) or include_asm_needle.search(text):
            matches.append(path)
    if len(matches) == 1:
        return matches[0]
    return None


def find_split_asm_path(func_name: str) -> Path | None:
    direct = ASM_FUNCS_DIR / f"{func_name}.s"
    if direct.exists():
        return direct
    info = get_all_func_info().get(func_name)
    if info and info.asm_path and info.asm_path != ASM_FILE:
        return info.asm_path
    return None


def run_command(cmd: list[str], *, input_bytes: bytes | None = None, env: dict[str, str] | None = None) -> bytes:
    cmd = [str(part) for part in cmd]
    env = env or os.environ.copy()

    use_wsl = False
    if os.name == "nt":
        cmd0_path = Path(cmd[0])
        native_exts = {".exe", ".bat", ".cmd", ".com", ".py"}
        use_wsl = shutil.which(cmd[0]) is None
        if cmd0_path.exists() and cmd0_path.suffix.lower() not in native_exts:
            use_wsl = True
    if use_wsl:
        def to_wsl_path(value: str) -> str:
            drive_match = re.match(r"^([A-Za-z]):[\\/](.*)$", value)
            if drive_match:
                drive = drive_match.group(1).lower()
                rest = drive_match.group(2).replace("\\", "/")
                return f"/mnt/{drive}/{rest}"
            return value.replace("\\", "/")

        wsl_root = PROJECT_ROOT.drive[0].lower() + PROJECT_ROOT.as_posix()[2:]
        wsl_root = f"/mnt/{wsl_root}"
        wsl_cmd = [to_wsl_path(part) for part in cmd]
        exports = []
        for key, value in env.items():
            if os.environ.get(key) == value:
                continue
            exports.append(f"export {key}={shlex.quote(to_wsl_path(value))}")
        export_prefix = "; ".join(exports)
        if export_prefix:
            export_prefix += "; "
        shell_cmd = f"cd {shlex.quote(wsl_root)} && {export_prefix}{' '.join(shlex.quote(part) for part in wsl_cmd)}"
        result = subprocess.run(
            ["wsl", "bash", "-lc", shell_cmd],
            input=input_bytes,
            capture_output=True,
        )
    else:
        result = subprocess.run(
            cmd,
            input=input_bytes,
            capture_output=True,
            cwd=PROJECT_ROOT,
            env=env,
        )
    if result.returncode != 0:
        stderr = result.stderr.decode(errors="replace")
        raise RuntimeError(f"command failed: {' '.join(cmd)}\n{stderr}")
    return result.stdout


def apply_rodata_align_fix(text: str, c_file: str | Path) -> str:
    if stem_for_path(c_file) not in RODATA_ALIGN2_FILES:
        return text
    return text.replace(".align\t3", ".align\t2")


def compile_stage_outputs(c_file: str | Path) -> dict[str, bytes]:
    c_file = project_rel_path(c_file)
    stages: dict[str, bytes] = {}
    stages["cpp"] = run_command([CPP, *CPP_FLAGS, *CPP_DEFS, c_file])
    stages["cc1"] = run_command([str(CC1), *cc_flags_for(c_file)], input_bytes=stages["cpp"])
    stages["prologue_fix"] = run_command(
        ["python3", "tools/prologue_fix.py"],
        input_bytes=stages["cc1"],
    )
    stages["maspsx"] = run_command(
        ["python3", str(MASPSX), *maspsx_flags_for(c_file)],
        input_bytes=stages["prologue_fix"],
    )
    maspsx_text = apply_rodata_align_fix(stages["maspsx"].decode("utf-8"), c_file)
    stages["maspsx"] = maspsx_text.encode("utf-8")
    stages["regfix1"] = run_command(
        ["python3", str(REGFIX)],
        input_bytes=stages["maspsx"],
    )
    env = os.environ.copy()
    env["REGFIX_CONFIG"] = str(REGFIX_STAGE2)
    stages["regfix2"] = run_command(
        ["python3", str(REGFIX)],
        input_bytes=stages["regfix1"],
        env=env,
    )
    return stages


def assemble_asm_text(asm_text: bytes, output_obj: str | Path) -> None:
    run_command([AS, *AS_FLAGS, "-o", str(output_obj)], input_bytes=asm_text)


def compile_c_file_to_object(c_file: str | Path, output_obj: str | Path) -> dict[str, bytes]:
    stages = compile_stage_outputs(c_file)
    assemble_asm_text(stages["regfix2"], output_obj)
    return stages


def get_obj_functions(obj_file: str | Path) -> list[tuple[int, int, str]]:
    try:
        stdout = run_command([NM, "-n", str(obj_file)]).decode("utf-8")
    except RuntimeError:
        return []

    symbols = []
    for line in stdout.strip().splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[1] == "T":
            symbols.append((int(parts[0], 16), parts[2]))

    header = run_command([OBJDUMP, "-h", str(obj_file)]).decode("utf-8")
    text_size = 0
    for line in header.splitlines():
        if ".text" not in line:
            continue
        parts = line.split()
        for i, part in enumerate(parts):
            if part == ".text":
                text_size = int(parts[i - 1], 16)
                break

    funcs = []
    for idx, (addr, name) in enumerate(symbols):
        if idx + 1 < len(symbols):
            size = symbols[idx + 1][0] - addr
        else:
            size = text_size - addr
        funcs.append((addr, size, name))
    return funcs


def generate_func_symbols(all_funcs: dict[str, FuncInfo], obj_func_names: set[str], sym_file: str | Path) -> None:
    with open(sym_file, "w", encoding="utf-8") as handle:
        for name, info in sorted(all_funcs.items(), key=lambda item: item[1].addr):
            if name not in obj_func_names:
                handle.write(f"{name} = 0x{info.addr:08X};\n")


def link_object_at_vram_base(
    obj_file: str | Path,
    base_addr: int,
    output_bin: str | Path,
    *,
    all_funcs: dict[str, FuncInfo] | None = None,
) -> None:
    all_funcs = all_funcs or get_all_func_info()
    tmpdir = Path(tempfile.mkdtemp())
    try:
        ld_script = tmpdir / "check.ld"
        func_syms = tmpdir / "func_syms.ld"
        elf_out = tmpdir / "check.elf"

        ld_script.write_text(
            "SECTIONS {\n"
            f"    .text 0x{base_addr:08X} : SUBALIGN(4) {{ *(.text) }}\n"
            "    /DISCARD/ : { *(*) }\n"
            "}\n",
            encoding="utf-8",
        )

        obj_func_names = {name for _, _, name in get_obj_functions(obj_file)}
        generate_func_symbols(all_funcs, obj_func_names, func_syms)

        try:
            run_command(
                [
                    LD,
                    "-nostdlib",
                    "--no-check-sections",
                    "--unresolved-symbols=ignore-all",
                    "--noinhibit-exec",
                    "-T",
                    str(ld_script),
                    "-T",
                    str(func_syms),
                    "-T",
                    str(UNDEF_FUNCS),
                    "-T",
                    str(UNDEF_SYMS),
                    "-T",
                    str(NAMED_SYMS),
                    str(obj_file),
                    "-o",
                    str(elf_out),
                ]
            )
        except RuntimeError as exc:
            stderr = str(exc).strip()
            errors = [line for line in stderr.splitlines() if line and "warning" not in line.lower()]
            if errors:
                raise RuntimeError(stderr)

        run_command([OBJCOPY, "-O", "binary", "-j", ".text", str(elf_out), str(output_bin)])
    finally:
        shutil.rmtree(tmpdir)


def link_object_at_function_address(
    obj_file: str | Path,
    func_name: str,
    output_bin: str | Path,
    *,
    all_funcs: dict[str, FuncInfo] | None = None,
) -> None:
    all_funcs = all_funcs or get_all_func_info()
    if func_name not in all_funcs:
        raise KeyError(f"{func_name} not found in asm metadata")
    link_object_at_vram_base(obj_file, all_funcs[func_name].addr, output_bin, all_funcs=all_funcs)


def get_original_bytes(addr: int, size: int) -> bytes:
    file_offset = EXE_HEADER + (addr - LOAD_ADDR)
    with open(ORIGINAL_EXE, "rb") as handle:
        handle.seek(file_offset)
        return handle.read(size)


def compare_word_sequences(expected: bytes, actual: bytes, addr: int) -> tuple[bool, list[str], int]:
    lines = []
    diff_words = 0
    min_len = min(len(expected), len(actual))
    if len(expected) != len(actual):
        lines.append(f"Size mismatch: expected {len(expected)} bytes, got {len(actual)} bytes")
    shown = 0
    for offset in range(0, min_len, 4):
        if expected[offset:offset + 4] == actual[offset:offset + 4]:
            continue
        diff_words += 1
        if shown < 40 and offset + 4 <= min_len:
            exp_word = struct.unpack("<I", expected[offset:offset + 4])[0]
            act_word = struct.unpack("<I", actual[offset:offset + 4])[0]
            lines.append(f"0x{addr + offset:08X}: expected 0x{exp_word:08X}, got 0x{act_word:08X}")
            shown += 1
    return diff_words == 0 and len(expected) == len(actual), lines, diff_words


def extract_function_asm(asm_text: str, func_name: str) -> str:
    start_pat = re.compile(rf"^(?:glabel\s+{re.escape(func_name)}|{re.escape(func_name)}:)\s*$", re.MULTILINE)
    start_match = start_pat.search(asm_text)
    if not start_match:
        raise ValueError(f"{func_name} not found in asm text")
    start = start_match.start()
    end_pat = re.compile(rf"^\s*\.end\s+{re.escape(func_name)}\s*$", re.MULTILINE)
    end_match = end_pat.search(asm_text, start)
    if not end_match:
        raise ValueError(f"end of {func_name} not found in asm text")
    end = asm_text.find("\n", end_match.end())
    if end == -1:
        end = len(asm_text)
    return asm_text[start:end + 1]


def extract_function_text_from_source(source_text: str, func_name: str) -> str:
    pattern = re.compile(rf"\b{re.escape(func_name)}\s*\(")
    match = pattern.search(source_text)
    if not match:
        raise ValueError(f"{func_name} not found in source text")

    start = source_text.rfind("\n", 0, match.start())
    start = 0 if start == -1 else start + 1

    brace_start = source_text.find("{", match.end())
    if brace_start == -1:
        raise ValueError(f"opening brace for {func_name} not found")

    depth = 0
    in_string = False
    string_delim = ""
    escape = False
    for pos in range(brace_start, len(source_text)):
        ch = source_text[pos]
        if in_string:
            if escape:
                escape = False
            elif ch == "\\":
                escape = True
            elif ch == string_delim:
                in_string = False
            continue
        if ch in ("'", '"'):
            in_string = True
            string_delim = ch
            continue
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                end = pos + 1
                while end < len(source_text) and source_text[end] in "\r\n":
                    end += 1
                return source_text[start:end]
    raise ValueError(f"closing brace for {func_name} not found")


def sanitize_filename(name: str) -> str:
    return re.sub(r"[^A-Za-z0-9_.-]+", "_", name)


def strip_comments(line: str) -> str:
    if "#" in line:
        return line.split("#", 1)[0].rstrip()
    return line.rstrip()


def extract_instruction_records(asm_text: str) -> list[dict[str, object]]:
    records = []
    pending_labels: list[str] = []
    insn_idx = 0
    for raw_line in asm_text.splitlines():
        stripped = raw_line.strip()
        if not stripped:
            continue
        if stripped.endswith(":"):
            pending_labels.append(stripped[:-1])
            continue
        if stripped.startswith(".") or stripped.startswith("#"):
            continue
        if stripped.startswith("glabel "):
            pending_labels.append(stripped.split()[1])
            continue
        records.append(
            {
                "idx": insn_idx,
                "text": raw_line,
                "labels": list(pending_labels),
            }
        )
        pending_labels.clear()
        insn_idx += 1
    return records


def build_label_to_index_map(asm_text: str) -> dict[str, int]:
    mapping = {}
    for record in extract_instruction_records(asm_text):
        for label in record["labels"]:
            mapping[str(label)] = int(record["idx"])
    return mapping


def build_index_to_label_map(asm_text: str) -> dict[int, str]:
    mapping = {}
    for record in extract_instruction_records(asm_text):
        labels = [label for label in record["labels"] if str(label).startswith(".L")]
        if labels:
            mapping[int(record["idx"])] = str(labels[0])
    return mapping


def remap_label_tokens(text: str, src_label_to_idx: dict[str, int], dst_idx_to_label: dict[int, str]) -> str:
    def replace(match: re.Match[str]) -> str:
        label = match.group(1)
        suffix = match.group(2) or ""
        if label not in src_label_to_idx:
            return match.group(0)
        idx = src_label_to_idx[label]
        if idx not in dst_idx_to_label:
            return match.group(0)
        return f"{dst_idx_to_label[idx]}{suffix}"

    return re.sub(r"(\.L[\w$]+)((?:[+-]\d+)?)", replace, text)


def diff_text(a_text: str, b_text: str, *, from_name: str = "a", to_name: str = "b") -> str:
    return "".join(
        difflib.unified_diff(
            a_text.splitlines(keepends=True),
            b_text.splitlines(keepends=True),
            fromfile=from_name,
            tofile=to_name,
        )
    )


def write_stage_outputs(output_dir: str | Path, func_name: str, stage_texts: dict[str, str]) -> list[Path]:
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    written = []
    for stage_name, text in stage_texts.items():
        path = output_dir / f"{sanitize_filename(func_name)}.{stage_name}.s"
        path.write_text(text, encoding="utf-8")
        written.append(path)
    return written
