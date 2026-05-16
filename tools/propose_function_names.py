#!/usr/bin/env python3
"""propose_function_names.py -- evidence-backed function-name proposal database.

For each unnamed `func_XXXXXXXX` in `asm/funcs/`, gather every kind of
evidence the analyzer knows about and emit a row to
`docs/naming/proposals.csv`. Per-function evidence detail files are
written to `docs/naming/evidence/<func>.md` for any non-`none` proposal.

This is a PROPOSAL builder, not a renamer. The output rows are
human-reviewable. Applying a proposal is a separate step
(`tools/apply_kengo_names.py` or a hand edit to `named_syms.txt`).

Evidence kinds (in rough descending strength):

  bios_jumptable         deterministic 3-4 insn BIOS A0/B0/C0 trampoline
  syscall_wrapper        raw syscall/break wrapper
  psyq_idiom             memcpy / memset / strcpy / strlen / qsort pattern
  kengo_unique           single Kengo function with same name OR a clean
                         name-unique + size match
  kengo_pattern          same-name multi-match with opcode-sequence agreement
  gte_op                 body contains GTE rtps/rtpt/nclip/etc
  sole_caller_path       exactly one caller, and that caller is named
  call_graph_cluster     >=2 callers are named functions in subsystem X
  string_adjacent        function loads a known string constant
  kengo_size_only        Kengo match but size-only (weakest, downgrades)
  address_neighborhood   adjacent functions are named in subsystem X

Confidence levels:

  high    : bios_jumptable OR syscall_wrapper OR psyq_idiom alone,
            or kengo_unique with size diff <=1 insn,
            or 2+ medium-strength kinds agreeing
  medium  : kengo_unique with larger size diff, kengo_pattern with
            opseq_ratio >=0.5, sole_caller_path, call_graph_cluster
  low     : string_adjacent alone, address_neighborhood alone,
            kengo_size_only, gte_op alone
  none    : no evidence (left for manual decomp + caller study)

Usage:
    python3 tools/propose_function_names.py
    python3 tools/propose_function_names.py --func func_80016514   # one
    python3 tools/propose_function_names.py --no-evidence-files    # CSV only
    python3 tools/propose_function_names.py --confidence high      # filter
"""
from __future__ import annotations

import argparse
import csv
import re
import sys
from collections import defaultdict, Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_FUNCS = ROOT / "asm" / "funcs"
SRC_DIR = ROOT / "src"
NAMED_SYMS = ROOT / "named_syms.txt"
SYMBOL_ADDRS = ROOT / "symbol_addrs.txt"
KENGO_CSV = ROOT / "kengo_matches.csv"
KENGO_DEC = ROOT / "kengo_name_decisions.csv"
KNOWN_PSYQ = ROOT / "known_psyq_stdlib.txt"
UNDEF_SYMS = ROOT / "undefined_syms_auto.txt"
DOCS_DIR = ROOT / "docs" / "naming"
EVIDENCE_DIR = DOCS_DIR / "evidence"
PROPOSALS_CSV = DOCS_DIR / "proposals.csv"


# ---------------------------------------------------------------------------
# Parsing
# ---------------------------------------------------------------------------

INSN_RE = re.compile(
    r"/\*\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f]+)\s+[0-9A-Fa-f]+\s+\*/\s+(\S+)\s*(.*)"
)
SYM_LINE = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*0x([0-9A-Fa-f]+)\s*;")
FUNC_NAME = re.compile(r"^func_([0-9A-Fa-f]{8})$")
ADDR_RE = re.compile(r"\b(80[0-9A-Fa-f]{6}|1F[0-9A-Fa-f]{6})\b", re.IGNORECASE)
HEX_OPRAND = re.compile(r"0x([0-9A-Fa-f]+)")
JAL_RE = re.compile(r"\bjal\s+(\w+)")
J_RE = re.compile(r"^\s*j\s+(\w+)")
SYM_REF_RE = re.compile(r"%(?:hi|lo)\(([A-Za-z_][A-Za-z0-9_]*)\)")
BARE_SYM_RE = re.compile(r"\b(D_[0-9A-Fa-f]{8})\b")


def load_named_addrs() -> dict[str, int]:
    """Read named_syms.txt + symbol_addrs.txt: returns {symbol_name: addr}."""
    out: dict[str, int] = {}
    for f in (SYMBOL_ADDRS, NAMED_SYMS):
        if not f.exists():
            continue
        for line in f.read_text(errors="replace").splitlines():
            m = SYM_LINE.match(line)
            if m:
                out[m.group(1)] = int(m.group(2), 16)
    return out


def load_addr_to_named() -> dict[int, str]:
    """Inverse map: addr -> symbol name (when only one name)."""
    out: dict[int, str] = {}
    for name, addr in load_named_addrs().items():
        # Keep first; prefer named_syms_* over symbol_addrs_*
        if addr not in out:
            out[addr] = name
    return out


def load_symbol_comments() -> dict[str, str]:
    """Parse symbol_addrs.txt for `name = 0xADDR; // comment` -> {name: comment}."""
    out: dict[str, str] = {}
    if not SYMBOL_ADDRS.exists():
        return out
    pat = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*0x[0-9A-Fa-f]+\s*;\s*//\s*(.+)$")
    for line in SYMBOL_ADDRS.read_text(errors="replace").splitlines():
        m = pat.match(line)
        if m:
            out[m.group(1)] = m.group(2).strip()
    return out


def load_rodata_strings() -> dict[int, str]:
    """Scan asm/data/*.rodata*.s for string constants. Returns {addr: str}."""
    out: dict[int, str] = {}
    rodata_dir = ROOT / "asm" / "data"
    if not rodata_dir.exists():
        return out
    pat = re.compile(
        r'/\*\s+[0-9A-Fa-f]+\s+(80[0-9A-Fa-f]{6})\s+\*/\s+\.asciz\s+"((?:[^"\\]|\\.)*)"'
    )
    for f in rodata_dir.glob("*.rodata*.s"):
        for line in f.read_text(errors="replace").splitlines():
            m = pat.search(line)
            if m:
                # Decode common backslash escapes
                s = m.group(2).replace("\\n", "\n").replace("\\t", "\t").replace("\\\\", "\\")
                out[int(m.group(1), 16)] = s
    return out


def load_kengo_matches() -> dict[str, dict]:
    """Read kengo_matches.csv as {bb2_func: row_dict}."""
    out: dict[str, dict] = {}
    if not KENGO_CSV.exists():
        return out
    for row in csv.DictReader(KENGO_CSV.open()):
        out[row["bb2_func"]] = row
    return out


def load_kengo_claim_counts() -> dict[str, int]:
    """For each Kengo name, count how many BB2 funcs claim it. Used to
    suffix when multi-claimant (TU-static cluster pattern)."""
    out: dict[str, int] = defaultdict(int)
    if not KENGO_CSV.exists():
        return out
    for row in csv.DictReader(KENGO_CSV.open()):
        kn = row.get("kengo_name") or ""
        bb2 = row.get("bb2_func") or ""
        if kn and bb2.startswith("func_"):
            out[kn] += 1
    return out


def load_kengo_decisions() -> dict[str, str]:
    """Reviewed decisions (for downgrading proposals previously rejected)."""
    out: dict[str, str] = {}
    if not KENGO_DEC.exists():
        return out
    for row in csv.DictReader(KENGO_DEC.open()):
        nm = (row.get("renamed_to") or "").strip()
        dec = (row.get("decision") or "").strip()
        if nm and dec:
            out[nm] = dec
    return out


def load_known_psyq() -> dict[str, tuple[str, str]]:
    """Read known_psyq_stdlib.txt: {func: (kind, evidence)}."""
    out: dict[str, tuple[str, str]] = {}
    if not KNOWN_PSYQ.exists():
        return out
    for line in KNOWN_PSYQ.read_text(errors="replace").splitlines():
        line = line.split("#", 1)[0].rstrip()
        if not line:
            continue
        parts = re.split(r"\s{2,}", line.strip(), maxsplit=2)
        if len(parts) >= 2:
            func = parts[0].strip()
            kind = parts[1].strip()
            ev = parts[2].strip() if len(parts) >= 3 else ""
            out[func] = (kind, ev)
    return out


def parse_func_asm(func: str) -> dict | None:
    """Return a parsed function dict: {name, addr, insns:[(mnem, ops)],
    callees:set, sym_refs:set, addr_refs:set}.
    """
    p = ASM_FUNCS / f"{func}.s"
    if not p.exists():
        return None
    text = p.read_text(encoding="utf-8", errors="replace")
    insns: list[tuple[str, str]] = []
    callees: set[str] = set()
    sym_refs: set[str] = set()
    addr_refs: set[int] = set()
    j_targets: set[str] = set()
    func_addr = None
    for line in text.splitlines():
        m = INSN_RE.search(line)
        if m:
            mnem, ops = m.group(2), m.group(3).strip()
            insns.append((mnem, ops))
            # Pick first insn's full address as func addr
            if func_addr is None:
                ma = re.search(r"\b(80[0-9A-Fa-f]{6})\b", line)
                if ma:
                    func_addr = int(ma.group(1), 16)
        # callees
        for jm in JAL_RE.finditer(line):
            callees.add(jm.group(1))
        # j targets (only when label is a function-name, not .L*)
        jm2 = J_RE.search(line)
        if jm2:
            tgt = jm2.group(1)
            if not tgt.startswith("."):
                j_targets.add(tgt)
        # symbol refs via %hi/%lo(...)
        for sm in SYM_REF_RE.finditer(line):
            sym_refs.add(sm.group(1))
        # bare D_XXXX refs
        for sm in BARE_SYM_RE.finditer(line):
            sym_refs.add(sm.group(1))
    if func_addr is None:
        m = FUNC_NAME.match(func)
        if m:
            func_addr = int(m.group(1), 16)
    return {
        "name": func,
        "addr": func_addr,
        "insns": insns,
        "callees": callees,
        "sym_refs": sym_refs,
        "addr_refs": addr_refs,
        "j_targets": j_targets,
    }


def parse_all_funcs() -> list[dict]:
    """Parse every asm/funcs/*.s and return list of dicts."""
    out = []
    for p in sorted(ASM_FUNCS.glob("*.s")):
        if p.stem.startswith("D_"):
            continue
        f = parse_func_asm(p.stem)
        if f:
            out.append(f)
    return out


# ---------------------------------------------------------------------------
# Evidence detection
# ---------------------------------------------------------------------------

# Standard PSX BIOS A0/B0/C0 jumptable, indexed lookup.
# Names follow nocash psx-spx (https://psx-spx.consoledev.net/kernelbios/).
# When a BB2 wrapper name has been observed in the project source, the
# corresponding `bios_*` name is used so the proposal matches existing usage.
BIOS_A0_NAMES = {
    # File / std I/O
    0x00: "bios_FileOpen",        # FileOpen(filename, accessmode)
    0x01: "bios_FileSeek",        # FileSeek(fd, offset, seektype)
    0x02: "bios_FileRead",        # FileRead(fd, dst, len)
    0x03: "bios_FileWrite",       # FileWrite(fd, src, len)
    0x04: "bios_FileClose",       # FileClose(fd)
    0x05: "bios_FileIoctl",
    0x06: "bios_exit",
    0x07: "bios_FileGetDeviceFlag",
    0x08: "bios_FileGetc",
    0x09: "bios_FilePutc",
    0x0A: "bios_todigit",
    0x0B: "bios_atof",
    0x0C: "bios_strtoul",
    0x0D: "bios_strtol",
    0x0E: "bios_abs",
    0x0F: "bios_labs",
    0x10: "bios_atoi",
    0x11: "bios_atol",
    0x12: "bios_atob",
    0x13: "bios_setjmp",
    0x14: "bios_longjmp",
    0x15: "bios_strcat",
    0x16: "bios_strncat",
    0x17: "bios_strcmp",
    0x18: "bios_strncmp",
    0x19: "bios_strcpy",
    0x1A: "bios_strncpy",
    0x1B: "bios_strlen",
    0x1C: "bios_index",
    0x1D: "bios_rindex",
    0x1E: "bios_strchr",
    0x1F: "bios_strrchr",
    0x20: "bios_strpbrk",
    0x21: "bios_strspn",
    0x22: "bios_strcspn",
    0x23: "bios_strtok",
    0x24: "bios_strstr",
    0x25: "bios_toupper",
    0x26: "bios_tolower",
    0x27: "bios_bcopy",
    0x28: "bios_bzero",
    0x29: "bios_bcmp",
    0x2A: "bios_memcpy",
    0x2B: "bios_memset",
    0x2C: "bios_memmove",
    0x2D: "bios_memcmp",
    0x2E: "bios_memchr",
    0x2F: "bios_rand",
    0x30: "bios_srand",
    0x31: "bios_qsort",
    0x32: "bios_strtod",
    0x33: "bios_malloc",
    0x34: "bios_free",
    0x35: "bios_lsearch",
    0x36: "bios_bsearch",
    0x37: "bios_calloc",
    0x38: "bios_realloc",
    0x39: "bios_InitHeap",
    0x3A: "bios_SystemErrorExit",
    0x3B: "bios_std_in_getchar",
    0x3C: "bios_std_out_putchar",
    0x3D: "bios_std_in_gets",
    0x3E: "bios_std_out_puts",
    0x3F: "bios_printf",
    0x40: "bios_SystemErrorUnresolvedException",
    0x41: "bios_LoadTest",
    0x42: "bios_Load",
    0x43: "bios_Exec",
    0x44: "bios_FlushCache",
    0x45: "bios_init_a0_b0_c0_vectors",
    0x46: "bios_GPU_dw",
    0x47: "bios_gpu_send_dma",
    0x48: "bios_SendGP1Command",
    0x49: "bios_GPU_cw",
    0x4A: "bios_GPU_cwp",
    0x4B: "bios_send_gpu_linked_list",
    0x4C: "bios_gpu_abort_dma",
    0x4D: "bios_GetGPUStatus",
    0x4E: "bios_gpu_sync",
    0x51: "bios_LoadExec",
    0x54: "bios_CdInit",
    0x55: "bios__bu_init",
    0x56: "bios_CdRemove",
    0x70: "bios__bu_init_A0",
    0x71: "bios_CdInit_A0",
    0x72: "bios_CdRemove_A0",
    0x78: "bios_CdAsyncSeekL",
    0x7C: "bios_CdAsyncGetStatus",
    0x7E: "bios_CdAsyncReadSector",
    0x81: "bios_CdAsyncSetMode",
    0x90: "bios_CdromIoIrqFunc1",
    0x91: "bios_CdromDmaIrqFunc1",
    0x92: "bios_CdromIoIrqFunc2",
    0x93: "bios_CdromDmaIrqFunc2",
    0x94: "bios_CdromGetInt5errCode",
    0x95: "bios_CdInitSubFunc",
    0x96: "bios_AddCDROMDevice",
    0x97: "bios_AddMemCardDevice",
    0x99: "bios_AddDummyTtyDevice",
    0x9C: "bios_SetConf",
    0x9D: "bios_GetConf",
    0x9F: "bios_SetMem",
    0xA0: "bios_SystemErrorUnknown_A0",
    0xA1: "bios_SystemErrorUnknown_A1",
    0xA2: "bios_EnqueueCdIntr_A0",
    0xA3: "bios_DequeueCdIntr_A0",
}

BIOS_B0_NAMES = {
    0x00: "bios_alloc_kernel_memory",
    0x01: "bios_free_kernel_memory",
    0x02: "bios_init_timer",
    0x03: "bios_get_timer",
    0x04: "bios_enable_timer_irq",
    0x05: "bios_disable_timer_irq",
    0x06: "bios_restart_timer",
    0x07: "bios_DeliverEvent",
    0x08: "bios_OpenEvent",
    0x09: "bios_CloseEvent",
    0x0A: "bios_WaitEvent",
    0x0B: "bios_TestEvent",
    0x0C: "bios_EnableEvent",
    0x0D: "bios_DisableEvent",
    0x0E: "bios_OpenThread",
    0x0F: "bios_CloseThread",
    0x10: "bios_ChangeThread",
    0x12: "bios_InitPad",
    0x13: "bios_StartPad",
    0x14: "bios_StopPad",
    0x15: "bios_OutdatedPadInitAndStart",
    0x16: "bios_OutdatedPadGetButtons",
    0x17: "bios_ReturnFromException",
    0x18: "bios_SetDefaultExitFromException",
    0x19: "bios_SetCustomExitFromException",
    0x20: "bios_UnDeliverEvent",
    0x32: "bios_FileOpen_B",
    0x33: "bios_FileSeek_B",
    0x34: "bios_FileRead_B",
    0x35: "bios_FileWrite_B",
    0x36: "bios_FileClose_B",
    0x37: "bios_FileIoctl_B",
    0x38: "bios_exit_B",
    0x39: "bios_FileGetDeviceFlag_B",
    0x3A: "bios_FileGetc_B",
    0x3B: "bios_FilePutc_B",
    0x3C: "bios_std_in_getchar_B",
    0x3D: "bios_std_out_putchar_B",
    0x3E: "bios_std_in_gets_B",
    0x3F: "bios_std_out_puts_B",
    0x40: "bios_chdir_B",
    0x41: "bios_FormatDevice_B",
    0x42: "bios_firstfile_B",
    0x43: "bios_nextfile_B",
    0x44: "bios_FileRename_B",
    0x45: "bios_FileDelete_B",
    0x46: "bios_FileUndelete_B",
    0x47: "bios_AddDevice_B",
    0x48: "bios_RemoveDevice_B",
    0x49: "bios_PrintInstalledDevices_B",
    0x4A: "bios_InitCard",
    0x4B: "bios_StartCard",
    0x4C: "bios_StopCard",
    0x4D: "bios__card_info_subfunc",
    0x4E: "bios__card_write",
    0x4F: "bios__card_read",
    0x50: "bios__new_card",
    0x51: "bios_Krom2RawAdd",
    0x53: "bios_Krom2Offset",
    0x54: "bios_GetLastError",
    0x55: "bios_GetLastFileError",
    0x56: "bios_GetC0Table",
    0x57: "bios_GetB0Table",
    0x58: "bios__card_chan",
    0x59: "bios_testdevice",
    0x5B: "bios_ChangeClearPad",
    0x5C: "bios__card_status",
    0x5D: "bios__card_wait",
}

BIOS_C0_NAMES = {
    0x00: "bios_EnqueueTimerAndVblankIrqs",
    0x01: "bios_EnqueueSyscallHandler",
    0x02: "bios_SysEnqIntRP",
    0x03: "bios_SysDeqIntRP",
    0x04: "bios_get_free_EvCB_slot",
    0x05: "bios_get_free_TCB_slot",
    0x06: "bios_ExceptionHandler",
    0x07: "bios_InstallExceptionHandlers",
    0x08: "bios_SysInitMemory",
    0x09: "bios_SysInitKernelVariables",
    0x0A: "bios_ChangeClearRCnt",
    0x0C: "bios_InitDefInt",
    0x0D: "bios_SetIrqAutoAck",
    0x12: "bios_InstallDevices",
    0x13: "bios_FlushStdInOutPut",
    0x15: "bios_tty_cdevinput",
    0x16: "bios_tty_cdevscan",
    0x17: "bios_tty_circgetc",
    0x18: "bios_tty_circputc",
    0x19: "bios_ioabort",
    0x1A: "bios_set_card_find_mode",
    0x1B: "bios_KernelRedirect",
    0x1C: "bios_PatchA0Table",
    0x1D: "bios_get_card_find_mode",
}


def bios_lookup(table: str, idx: int) -> str:
    """Return the standard PsyQ/nocash name for a BIOS table index, or a
    generic wrapper name if the index is unknown. The name is documented
    in the standard PSX kernel docs (https://psx-spx.consoledev.net/kernelbios/).

    NOTE: a few low-index A0 entries (0x40-0x4B) have shifted between BIOS
    revisions. The name returned is the most commonly-used PsyQ convention,
    but should be verified against caller usage before applying.
    """
    table_map = {"A0": BIOS_A0_NAMES, "B0": BIOS_B0_NAMES,
                 "C0": BIOS_C0_NAMES}.get(table, {})
    return table_map.get(idx, f"bios_{table}_0x{idx:02X}_wrapper")


def is_bios_jumptable(insns) -> tuple[str | None, str | None]:
    """Match: addiu $tX,$zero,0xA0|0xB0|0xC0; jr $tX; ... .word <index>.

    Returns (proposed_name, evidence_str) or (None, None).
    """
    if not 2 <= len(insns) <= 8:
        return None, None
    table = None
    for m, ops in insns:
        if m == "addiu":
            mm = re.search(r"\$t\d+,\s*\$zero,\s*0x([ABCabc]0)", ops)
            if mm:
                table = mm.group(1).upper()
                break
    if not table:
        return None, None
    if not any(m == "jr" and "$t" in o for m, o in insns):
        return None, None
    # Get index (skip the 0xA0/B0/C0 vector; the function index is in the
    # remaining addiu $tN, $zero, <imm>).
    idx = None
    for m, ops in insns:
        if m == "addiu" and "$zero" in ops:
            mm = re.search(r"\$\w+,\s*\$zero,\s*0x([0-9A-Fa-f]+)", ops)
            if mm:
                v = int(mm.group(1), 16)
                if v not in (0xA0, 0xB0, 0xC0):
                    idx = v
                    break
    if idx is None:
        return f"bios_{table}_wrapper", f"BIOS jumptable {table} (no explicit index)"
    name = bios_lookup(table, idx)
    ev = f"BIOS jumptable {table} index 0x{idx:02X} -> {name}"
    return name, ev


def is_syscall_wrapper(insns, func_addr: int | None = None) -> tuple[str | None, str | None]:
    if not 2 <= len(insns) <= 8:
        return None, None
    sx = f"_{func_addr:08X}" if func_addr else ""
    for m, _ in insns:
        if m in ("syscall", "break"):
            return f"syscall_wrapper_{m}{sx}", f"raw {m} instruction in {len(insns)}-insn body"
    return None, None


def is_data_as_code(insns, func_addr: int | None = None) -> tuple[str | None, str | None]:
    """Functions where every insn is `lb $t0, neg_offset($zero)` (data table)."""
    if len(insns) < 4:
        return None, None
    lb_count = 0
    other = 0
    for m, _ in insns:
        if m == "lb":
            lb_count += 1
        elif m != "nop":
            other += 1
    if lb_count >= len(insns) * 0.7 and other == 0:
        sx = f"_{func_addr:08X}" if func_addr else ""
        return (f"data_as_code_lb_table{sx}",
                f"{lb_count}/{len(insns)} are `lb $t0,..($zero)` (data-as-code table)")
    return None, None


def looks_like_psyq(insns, func_addr: int | None = None) -> tuple[str | None, str | None]:
    """Memcpy/memset patterns. Conservative: require the load and store to
    be in the same tight loop window and the function to NOT include calls
    (calls are a sign of higher-level logic, not a copy primitive).
    """
    if not 4 <= len(insns) <= 35:
        return None, None
    mnems = [m for m, _ in insns]
    cnt = Counter(mnems)

    # Disqualify obvious non-memcpy shapes
    if any(m == "jal" for m in mnems):
        return None, None  # has call, not a primitive
    if any(m == "jr" and "$ra" not in o and "$v" in o for m, o in insns):
        return None, None  # indirect-jump dispatch
    if cnt.get("ori", 0) >= 2 or cnt.get("andi", 0) >= 2:
        return None, None  # bit-twiddler
    branch_count = sum(1 for m in mnems if m in ("bne", "bnez", "bgtz", "bgez", "beq", "beqz"))
    if branch_count == 0 or branch_count >= 3:
        return None, None
    # Require backward branch (loop) — see if any branch targets a label
    # whose hex addr is below the current instruction. (Cheap heuristic:
    # presence of `.L` label in branch ops is a loop.)
    if not any(m in ("bne", "bnez", "bgtz", "bgez", "beq", "beqz")
               and re.search(r"\.L[0-9A-Fa-f]+", o) for m, o in insns):
        return None, None

    # Require all stores to be "post-incremented pointer" shape — e.g.,
    # `sb $vN, offset($pN)` where pN gets `addiu $pN, $pN, 1` in body.
    # Cheap proxy: ensure there are at least 2 different addiu adding 1.
    pos_step_count = sum(
        1 for m, o in insns
        if m == "addiu" and re.search(r"(\$\w+),\s*\1,\s*(0x1\b|1\b)", o)
    )
    pointer_step = pos_step_count >= 1 or any(
        m == "addiu" and re.search(r"\$[stva][0-9]+,\s*\$[stva][0-9]+,\s*(0x1\b|1\b)", o)
        for m, o in insns
    )
    if not pointer_step:
        return None, None

    # Tight pairing: find any lbu followed by sb within a 4-insn window.
    # CRITICAL: the lbu source pointer and the sb destination pointer must
    # be DIFFERENT registers -- otherwise it's an in-place transform
    # (halving, increment, bit-twiddle) not a memcpy. We extract the
    # base register from each.
    base_re = re.compile(r"\$\w+,\s*(?:-?0x[0-9A-Fa-f]+|-?\d+)?\(\$(\w+)\)")
    tight_copy = False
    for i, (m, ops) in enumerate(insns):
        if m != "lbu":
            continue
        bm = base_re.search(ops)
        if not bm:
            continue
        src_base = bm.group(1)
        for j in range(i + 1, min(i + 5, len(insns))):
            if insns[j][0] != "sb":
                continue
            bm2 = base_re.search(insns[j][1])
            if not bm2:
                continue
            if bm2.group(1) != src_base:
                # Different base register -> genuine copy
                tight_copy = True
                break
        if tight_copy:
            break

    has_lbu = cnt.get("lbu", 0) >= 1
    has_sb = cnt.get("sb", 0) >= 1
    has_no_load = not any(m in ("lw", "lh", "lhu", "lb", "lbu") for m in mnems)
    has_store = any(m in ("sb", "sw", "sh") for m in mnems)

    base = None
    detail = ""
    if has_lbu and has_sb and tight_copy:
        base, detail = "psyq_memcpy", f"{len(insns)} insns, tight lbu/sb pair loop"
    elif has_store and has_no_load:
        # No load, tight store loop is memset
        base, detail = "psyq_memset", f"{len(insns)} insns, store loop no load"
    if not base:
        return None, None
    sx = f"_{func_addr:08X}" if func_addr else ""
    return f"{base}{sx}", detail


def gte_ops(insns, func_addr: int | None = None) -> tuple[str | None, str | None]:
    """Returns (proposal, evidence) if function does GTE work. Address-suffixed
    name because many functions share `gte_<op>_wrapper`."""
    GTE_ATOMS = {
        "rtps": "rtps", "rtpt": "rtpt", "nclip": "nclip", "avsz3": "avsz3",
        "avsz4": "avsz4", "dpcs": "dpcs", "dpct": "dpct", "intpl": "intpl",
        "sqr": "sqr", "ncs": "ncs", "nct": "nct", "ncds": "ncds",
        "ncdt": "ncdt", "nccs": "nccs", "ncct": "ncct", "cdp": "cdp",
        "cc": "cc", "dpcl": "dpcl", "mvmva": "mvmva",
    }
    cop2 = sum(1 for m, _ in insns if m.lower() in ("lwc2", "swc2", "mtc2", "mfc2", "ctc2", "cfc2"))
    atomic_ops = []
    for m, _ in insns:
        if m.lower() in GTE_ATOMS:
            atomic_ops.append(m.lower())
    if not atomic_ops and cop2 < 3:
        return None, None
    sx = f"_{func_addr:08X}" if func_addr else ""
    op_counter = Counter(atomic_ops)
    if op_counter:
        top, _ = op_counter.most_common(1)[0]
        return f"gte_{top}_wrapper{sx}", (
            f"GTE: {dict(op_counter)} atomic ops, {cop2} cop2 transfers"
        )
    return f"gte_helper{sx}", f"{cop2} cop2 transfers (no atomic op)"


def kengo_proposal(func: str, kengo_row: dict | None,
                    decisions: dict[str, str],
                    claim_counts: dict[str, int] | None = None) -> tuple[str | None, str | None, str, str]:
    """Return (proposed_name, evidence_str, confidence_label, kind).

    confidence_label is 'high'/'medium'/'low' OR '' if no proposal.
    kind is the evidence kind (kengo_unique / kengo_pattern / kengo_size_only / '').
    """
    if not kengo_row or not kengo_row.get("kengo_name"):
        return None, None, "", ""
    kn = kengo_row["kengo_name"]
    conf = kengo_row.get("confidence", "")
    diff = kengo_row.get("diff", "")
    bb2_insns = int(kengo_row.get("bb2_insns") or 0)
    kengo_insns = int(kengo_row.get("kengo_insns") or 0)
    try:
        diff_int = int(diff) if diff != "" else 0
    except ValueError:
        diff_int = 0
    opseq = kengo_row.get("opseq_ratio") or ""
    callee_overlap = kengo_row.get("callee_overlap") or "0.0"
    caller_overlap = kengo_row.get("caller_overlap") or "0.0"

    # Filter out the previously-known false-positive cluster:
    # `katinuki_game_get_katinuki_max_num_*` was Kengo size-only and got
    # demoted across 5 funcs. Avoid re-proposing the bare or suffixed name.
    if kn == "katinuki_game_get_katinuki_max_num":
        return None, None, "", ""
    # Some other prior-flagged groups
    if kn == "motion_SavePreCalcData" and abs(diff_int) >= 3:
        return None, None, "", ""

    # Suppress entries whose decision is `demote` or `rename`
    if decisions.get(kn) in ("rename", "demote"):
        return None, None, "", ""

    bare = kn
    m = FUNC_NAME.match(func)
    sx = f"_{int(m.group(1), 16):08X}" if m else ""
    n_claims = claim_counts.get(kn, 1) if claim_counts else 1
    multi = n_claims > 1
    ev = (f"Kengo CSV: confidence={conf}, bb2_insns={bb2_insns} kengo_insns={kengo_insns} "
          f"diff={diff_int}, callee_overlap={callee_overlap}, caller_overlap={caller_overlap}"
          + (f", opseq_ratio={opseq}" if opseq else "")
          + (f", n_claimants={n_claims}" if multi else ""))

    # If multi-claimant, force suffix so renames don't collide.
    def with_suffix(b: str) -> str:
        return f"{b}{sx}" if multi else b

    if conf == "name-unique" and abs(diff_int) <= 1:
        # Demote to medium if multi-claimant (need disambiguation)
        return with_suffix(bare), ev, ("medium" if multi else "high"), "kengo_unique"
    if conf in ("name-unique", "name-callgraph") and abs(diff_int) <= max(3, bb2_insns * 0.05):
        return with_suffix(bare), ev, "medium", "kengo_unique"
    if conf in ("callgraph", "caller-unique", "caller-callgraph",
                "affinity-unique", "affinity-callgraph"):
        return with_suffix(bare), ev, "medium", "kengo_pattern"
    if conf == "seq-similarity":
        try:
            r = float(opseq)
        except (TypeError, ValueError):
            r = 0.0
        if r >= 0.5:
            return with_suffix(bare), ev, "medium", "kengo_pattern"
        if r >= 0.30:
            # Always address-suffix when seq-similarity (uncertain)
            return f"{bare}{sx}", ev, "low", "kengo_pattern"
        return None, None, "", ""
    if conf == "size-only-unique":
        return f"{bare}{sx}", ev, "low", "kengo_size_only"
    # size-only-ambiguous or no-match
    return None, None, "", ""


# ---------------------------------------------------------------------------
# Call-graph clustering
# ---------------------------------------------------------------------------

def build_call_graph(funcs: list[dict]) -> tuple[dict[str, set[str]], dict[str, set[str]]]:
    """Return (callers_of, callees_of) keyed by func name."""
    callers: dict[str, set[str]] = defaultdict(set)
    callees: dict[str, set[str]] = defaultdict(set)
    func_names = {f["name"] for f in funcs}
    for f in funcs:
        for c in f["callees"]:
            if c in func_names:
                callees[f["name"]].add(c)
                callers[c].add(f["name"])
        # j-target chains (tail-call style)
        for t in f.get("j_targets", set()):
            if t in func_names:
                callees[f["name"]].add(t)
                callers[t].add(f["name"])
    return callers, callees


def subsystem_prefix(name: str) -> str | None:
    """Heuristic: extract subsystem prefix from a named function.

    e.g., `cpu_check_same_dir_timer` -> `cpu`; `replay_camera_Init` ->
    `replay_camera`; `pad_ClearAppliBuffer` -> `pad`; `gpu_SetMode` ->
    `gpu`; `coli_hit_body_weapon` -> `coli`; `single_dojo_*` ->
    `single_dojo`.
    """
    if name.startswith("func_") or name.startswith("D_"):
        return None
    # Common multi-word prefixes first
    for pfx in ("single_dojo", "single_game", "replay_camera", "special_camera",
                "memory_card", "memcard", "saTan", "is_motion", "is_coli",
                "is_action", "is_ki_control", "is_pad", "is_tanren", "is_damage",
                "is_efc", "is_learn", "is_league", "is_stats", "is_status",
                "is_replay", "is_rob_test", "is_test",
                "nm_cpu", "nm_camera", "nm_mario", "nm_replay_cam",
                "nm_special_cam", "nm_mario_cam", "nm_tanren_cam",
                "md_game", "md_dummy", "md_sel", "md_option",
                "am_rmd", "sa_tan", "sa_se", "sa_load", "sa_main", "sa_eft",
                "se_fc", "se_qt", "my_rob", "my_eff", "my_hirahira",
                "hi_curpad", "hi_gnd", "hi_landhit", "hi_gview", "hi_kgm",
                "katinuki_game"):
        if name.lower().startswith(pfx.lower()):
            return pfx.lower()
    # Single-word prefix up to first '_' or transition
    parts = re.split(r"[_]", name)
    if len(parts) >= 2:
        return parts[0].lower()
    return None


def caller_cluster(func_name: str, callers: dict[str, set[str]]) -> tuple[str | None, str | None, list[str]]:
    """Look at named callers; if a single subsystem dominates, return
    (proposed_name, evidence, contributing_callers).
    """
    all_cs = sorted(callers.get(func_name, set()))
    cs = [c for c in all_cs if _is_trustworthy_anchor(c)]
    if not cs:
        return None, None, all_cs
    prefixes = Counter()
    for c in cs:
        pfx = subsystem_prefix(c)
        if pfx:
            prefixes[pfx] += 1
    if not prefixes:
        return None, None, cs
    top_pfx, top_n = prefixes.most_common(1)[0]
    # Require at least 2 callers and the dominant subsystem to be a strict majority.
    if top_n >= 2 and top_n / len(cs) >= 0.5:
        m = FUNC_NAME.match(func_name)
        sx = f"_{int(m.group(1), 16):08X}" if m else ""
        prop = f"{top_pfx}_helper{sx}"
        ev = f"callers={cs[:5]} prefix-dominant={top_pfx} ({top_n}/{len(cs)})"
        return prop, ev, cs
    return None, None, cs


def sole_caller(func_name: str, callers: dict[str, set[str]]) -> tuple[str | None, str | None]:
    cs = list(callers.get(func_name, set()))
    if len(cs) == 1:
        c = cs[0]
        if _is_trustworthy_anchor(c):
            m = FUNC_NAME.match(func_name)
            sx = f"_{int(m.group(1), 16):08X}" if m else ""
            return f"{c}_helper{sx}", f"sole caller is {c}"
    return None, None


# Names that prior triage flagged as misleading. Don't use them as
# subsystem-cluster anchors in address_neighborhood / call_graph_cluster.
# Pattern: bare-name prefix that was demoted/renamed in NAMING_TRIAGE.
KNOWN_BAD_ANCHOR_PREFIXES = {
    "katinuki_game_get_katinuki_max_num",  # all 5 are wrappers, not max-num getters
    "motion_SavePreCalcData",              # 5-claim seq-similarity cluster
    "saTanMainDispGnd",                    # tiny seq-sim group of 3
}


def _is_trustworthy_anchor(nm: str) -> bool:
    """Decide whether a named symbol can drive a cluster prediction.

    Reject:
      - data and synthetic labels (D_, g_str_, jtbl_)
      - address-suffixed Kengo names (uncertainty signal)
      - prior-flagged demoted clusters
    """
    if not nm:
        return False
    if nm.startswith("D_") or nm.startswith("g_str_") or nm.startswith("jtbl_"):
        return False
    if nm.startswith("func_"):
        return False
    # Address-suffix means uncertain Kengo match (per NAMING_TRIAGE.md)
    if re.search(r"_[0-9A-Fa-f]{8}$", nm):
        return False
    for bad in KNOWN_BAD_ANCHOR_PREFIXES:
        if nm.startswith(bad):
            return False
    return True


def address_neighborhood(func_name: str, addr_to_named: dict[int, str]) -> tuple[str | None, str | None]:
    m = FUNC_NAME.match(func_name)
    if not m:
        return None, None
    addr = int(m.group(1), 16)
    # Walk +- 0x200 bytes by 4
    neighbors_named = []
    for delta in range(-0x200, 0x201, 4):
        if delta == 0:
            continue
        nm = addr_to_named.get(addr + delta)
        if nm and _is_trustworthy_anchor(nm):
            neighbors_named.append((delta, nm))
    if not neighbors_named:
        return None, None
    pref_count = Counter()
    for _, nm in neighbors_named:
        pfx = subsystem_prefix(nm)
        if pfx:
            pref_count[pfx] += 1
    if not pref_count:
        return None, None
    top_pfx, top_n = pref_count.most_common(1)[0]
    if top_n >= 2:
        sx = f"_{addr:08X}"
        ev = (f"neighbors within +-0x200: {[n for _,n in neighbors_named[:5]]} "
              f"prefix-dominant={top_pfx} ({top_n}/{len(neighbors_named)})")
        return f"{top_pfx}_local{sx}", ev
    return None, None


def string_evidence(func_data: dict, addr_to_named: dict[int, str],
                    rodata: dict[int, str], sym_comments: dict[str, str]) -> tuple[list[str], list[str]]:
    """Return (named_string_syms, raw_strings) referenced by this function.

    Sorted alphabetically by symbol name for deterministic output across
    runs (set iteration order otherwise varies).
    """
    named: list[str] = []
    raw: list[str] = []
    for sref in sorted(func_data["sym_refs"]):
        # If sref points at a known g_str_*
        m_d = re.match(r"^D_([0-9A-Fa-f]{8})$", sref)
        if m_d:
            saddr = int(m_d.group(1), 16)
            # Check rodata raw
            if saddr in rodata:
                s = rodata[saddr]
                clean = repr(s[:40])
                raw.append(f"0x{saddr:08X}={clean}")
            # Check if known named
            if saddr in addr_to_named:
                nm = addr_to_named[saddr]
                if nm.startswith("g_str_") or "str" in nm.lower():
                    cmt = sym_comments.get(nm, "")
                    named.append(f"{nm}" + (f" // {cmt}" if cmt else ""))
        elif sref.startswith("g_str_") or "str" in sref.lower():
            cmt = sym_comments.get(sref, "")
            named.append(f"{sref}" + (f" // {cmt}" if cmt else ""))
    return named, raw


# ---------------------------------------------------------------------------
# Naming heuristics (string-based)
# ---------------------------------------------------------------------------

STRING_PROPOSAL_HINTS = [
    # (substring needle in the string, slug for the func)
    ("CHANBARA", "chanbara_"),
    ("PRACTICE", "practice_"),
    ("KATINUKI", "katinuki_"),
    ("STAGE", "stage_"),
    ("SAVE", "save_"),
    ("LOAD", "load_"),
    ("memory card", "memcard_"),
    ("MEMORY CARD", "memcard_"),
    ("WIN", "win_"),
    ("OVER FLOW", "overflow_"),
    ("ROB INIT", "rob_init_"),
    ("eff_init", "eff_init_"),
    ("prim over", "prim_overflow_"),
    ("BAD ", "bad_"),
    ("ERROR", "error_"),
    ("FAILED", "fail_"),
    ("INIT", "init_"),
    ("DEBUG", "debug_"),
    ("DRAWSYNC", "drawsync_"),
    ("CLEARIMAGE", "clearimage_"),
    ("LOADIMAGE", "loadimage_"),
    ("STOREIMAGE", "storeimage_"),
    ("SetDispMask", "setdispmask_"),
    ("CHAKUTI", "chakuti_"),
    ("HOM_", "home_"),
    ("KAMAE", "kamae_"),
    ("KAISHAKU", "kaishaku_"),
    ("CD ", "cd_"),
    ("SOUND ID", "sound_id_"),
    ("OPENING", "opening_"),
    ("NDATA", "ndata_"),
    ("SLUS", "exe_id_"),
]


def string_naming_hint(raw_strings: list[str], named_strings: list[str], addr: int) -> tuple[str | None, str | None]:
    """If the function loads strings with recognizable meaning, propose a slug."""
    if not raw_strings and not named_strings:
        return None, None
    candidates: list[str] = []
    for s in raw_strings + named_strings:
        for needle, slug in STRING_PROPOSAL_HINTS:
            if needle.lower() in s.lower():
                candidates.append(slug)
                break
    if not candidates:
        return None, None
    top = Counter(candidates).most_common(1)[0][0]
    sx = f"_{addr:08X}"
    sample = (raw_strings + named_strings)[:3]
    return f"{top}func{sx}", f"loads strings: {sample}"


# ---------------------------------------------------------------------------
# Main analyzer
# ---------------------------------------------------------------------------

CONF_RANK = {"none": 0, "low": 1, "medium": 2, "high": 3}


def confidence_label(evidence_kinds: list[str]) -> str:
    """Roll up an overall confidence from per-kind labels."""
    if not evidence_kinds:
        return "none"
    # Tier each kind
    HIGH_KINDS = {"bios_jumptable", "syscall_wrapper", "psyq_idiom",
                  "kengo_unique_high", "data_as_code"}
    MED_KINDS = {"kengo_unique_med", "kengo_pattern_med",
                 "sole_caller_path", "call_graph_cluster"}
    INFO_KINDS = {"string_adjacent_info"}
    rated = [k for k in evidence_kinds if k not in INFO_KINDS]
    n_high = sum(1 for k in rated if k in HIGH_KINDS)
    n_med = sum(1 for k in rated if k in MED_KINDS)
    n_low = sum(1 for k in rated if k not in HIGH_KINDS and k not in MED_KINDS)
    if n_high >= 1:
        return "high"
    if n_med >= 2:
        return "high"
    if n_med >= 1:
        return "medium"
    if n_low >= 2:
        return "medium"
    if n_low >= 1:
        return "low"
    return "none"


def best_proposed_name(evidence: list[dict]) -> str:
    """Pick the strongest evidence's proposed name."""
    if not evidence:
        return ""
    # Rank: HIGH > MED > LOW
    score = {"high": 3, "medium": 2, "low": 1}
    ev_sorted = sorted(evidence, key=lambda e: -score.get(e.get("rank", "low"), 1))
    return ev_sorted[0].get("name") or ""


def analyze_one(func_data: dict, ctx: dict) -> dict:
    """Return the proposal record for one function."""
    func = func_data["name"]
    addr = func_data["addr"]
    insns = func_data["insns"]
    evidence: list[dict] = []

    # 1. BIOS jumptable
    prop, ev = is_bios_jumptable(insns)
    if prop:
        evidence.append({"kind": "bios_jumptable", "rank": "high",
                         "name": prop, "detail": ev})

    # 2. syscall wrapper
    prop, ev = is_syscall_wrapper(insns, addr)
    if prop:
        evidence.append({"kind": "syscall_wrapper", "rank": "high",
                         "name": prop, "detail": ev})

    # 3. Data-as-code
    prop, ev = is_data_as_code(insns, addr)
    if prop:
        evidence.append({"kind": "data_as_code", "rank": "high",
                         "name": prop, "detail": ev})

    # 4. PsyQ idiom. We trust ONLY our live detector for the high-confidence
    # tag, because known_psyq_stdlib.txt was generated by an older scanner
    # that over-matched (e.g., halving loops with same-base lbu/sb). When
    # known_psyq disagrees, demote to `low` and explain why.
    prop, ev = looks_like_psyq(insns, addr)
    if prop:
        psyq_known = ctx["known_psyq"].get(func)
        if psyq_known and psyq_known[0].startswith("psyq_"):
            ev = f"{ev}; agrees with known_psyq_stdlib.txt={psyq_known[0]}"
        evidence.append({"kind": "psyq_idiom", "rank": "high",
                         "name": prop, "detail": ev})
    else:
        # Live detector said no; if the legacy scanner said yes, flag as
        # low-confidence with an explanation so a reviewer can confirm/reject.
        psyq_known = ctx["known_psyq"].get(func)
        if psyq_known and psyq_known[0].startswith("psyq_"):
            sx = f"_{addr:08X}" if addr else ""
            evidence.append({"kind": "psyq_idiom_legacy", "rank": "low",
                             "name": f"{psyq_known[0]}_legacy{sx}",
                             "detail": (f"known_psyq_stdlib.txt: {psyq_known[1]}; "
                                        "live detector rejected (likely in-place "
                                        "transform, not a memcpy/memset)")})

    # 5. GTE op
    prop, ev = gte_ops(insns, addr)
    if prop:
        evidence.append({"kind": "gte_op", "rank": "low",
                         "name": prop, "detail": ev})

    # 6. Kengo evidence
    kn_prop, kn_ev, kn_conf, kn_kind = kengo_proposal(func,
                                                        ctx["kengo"].get(func),
                                                        ctx["kengo_decisions"],
                                                        ctx.get("kengo_claims"))
    if kn_prop:
        # Map kn_conf+kn_kind to internal rank kind
        if kn_conf == "high":
            rk = "kengo_unique_high"
        elif kn_conf == "medium":
            rk = "kengo_unique_med" if kn_kind == "kengo_unique" else "kengo_pattern_med"
        else:
            rk = "kengo_size_only"
        evidence.append({"kind": kn_kind, "rank": kn_conf,
                         "name": kn_prop, "detail": kn_ev, "_rank_internal": rk})

    # 7. Sole caller
    prop, ev = sole_caller(func, ctx["callers"])
    if prop:
        evidence.append({"kind": "sole_caller_path", "rank": "medium",
                         "name": prop, "detail": ev})

    # 8. Call graph cluster
    if not any(e["kind"] == "sole_caller_path" for e in evidence):
        prop, ev, cs = caller_cluster(func, ctx["callers"])
        if prop:
            evidence.append({"kind": "call_graph_cluster", "rank": "medium",
                             "name": prop, "detail": ev})

    # 9. Address neighborhood
    prop, ev = address_neighborhood(func, ctx["addr_to_named"])
    if prop:
        evidence.append({"kind": "address_neighborhood", "rank": "low",
                         "name": prop, "detail": ev})

    # 10. String evidence
    named_str, raw_str = string_evidence(func_data, ctx["addr_to_named"],
                                          ctx["rodata"], ctx["sym_comments"])
    if named_str or raw_str:
        prop, ev = string_naming_hint(raw_str, named_str, addr)
        if prop:
            evidence.append({"kind": "string_adjacent", "rank": "low",
                             "name": prop, "detail": ev})
        else:
            # Informational only — don't count for confidence ranking but
            # surface in the evidence file for human review.
            evidence.append({"kind": "string_adjacent_info", "rank": "info",
                             "name": "",
                             "detail": f"refs strings: {(named_str + raw_str)[:5]}"})

    # Compute final rolled-up confidence
    kinds_for_rank = []
    for e in evidence:
        if "_rank_internal" in e:
            kinds_for_rank.append(e["_rank_internal"])
        else:
            kinds_for_rank.append(e["kind"])
    confidence = confidence_label(kinds_for_rank) if evidence else "none"

    # Pick the best proposed name
    pname = best_proposed_name([e for e in evidence if e.get("name")])

    return {
        "address": f"0x{addr:08X}" if addr else "",
        "current_name": func,
        "proposed_name": pname,
        "confidence": confidence,
        "evidence_summary": "; ".join(
            f"{e['kind']}={e['name'] or '?'}" for e in evidence if e.get("kind")
        ) if evidence else "",
        "evidence": evidence,  # raw list, not in CSV
    }


# ---------------------------------------------------------------------------
# Output writers
# ---------------------------------------------------------------------------

def write_proposals_csv(rows: list[dict], path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    cols = ["address", "current_name", "proposed_name", "confidence",
            "evidence_summary", "evidence_detail_file"]
    with path.open("w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(cols)
        for r in sorted(rows, key=lambda r: r["address"]):
            evfile = ""
            if r["evidence"]:
                evfile = f"docs/naming/evidence/{r['current_name']}.md"
            w.writerow([r["address"], r["current_name"], r["proposed_name"],
                        r["confidence"], r["evidence_summary"], evfile])


def write_evidence_file(row: dict, ctx: dict, func_data: dict) -> None:
    """Per-function evidence detail markdown."""
    EVIDENCE_DIR.mkdir(parents=True, exist_ok=True)
    out = EVIDENCE_DIR / f"{row['current_name']}.md"
    lines: list[str] = []
    lines.append(f"# {row['current_name']} -- naming evidence")
    lines.append("")
    lines.append(f"- Address: {row['address']}")
    lines.append(f"- Size: {len(func_data['insns'])} instructions")
    lines.append(f"- Asm file: `asm/funcs/{row['current_name']}.s`")
    # caller info
    callers = sorted(ctx["callers"].get(row["current_name"], set()))
    if callers:
        lines.append(f"- Callers ({len(callers)}): {callers[:10]}"
                     + (" ..." if len(callers) > 10 else ""))
    else:
        lines.append("- Callers: none observed")
    callees = sorted(func_data["callees"])
    if callees:
        lines.append(f"- Callees ({len(callees)}): {callees[:10]}"
                     + (" ..." if len(callees) > 10 else ""))
    lines.append(f"- Proposed name: `{row['proposed_name']}`")
    lines.append(f"- Confidence: **{row['confidence']}**")
    lines.append("")
    lines.append("## Evidence")
    lines.append("")
    for e in row["evidence"]:
        lines.append(f"### {e['kind']}  (rank={e['rank']})")
        lines.append("")
        if e.get("name"):
            lines.append(f"- proposed: `{e['name']}`")
        lines.append(f"- detail: {e['detail']}")
        lines.append("")
    # Add a small disassembly excerpt
    lines.append("## First 20 instructions")
    lines.append("")
    lines.append("```")
    for m, ops in func_data["insns"][:20]:
        lines.append(f"  {m:<10s} {ops}")
    lines.append("```")
    out.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--func", help="Only analyze one function (e.g. func_80016514)")
    ap.add_argument("--no-evidence-files", action="store_true",
                    help="Skip writing per-function evidence markdown")
    ap.add_argument("--confidence", choices=["high", "medium", "low", "none"],
                    help="Filter CSV output to >= this confidence")
    ap.add_argument("--limit", type=int, default=0,
                    help="Cap analyzed funcs (debugging only)")
    args = ap.parse_args()

    print("Loading context...", file=sys.stderr)
    ctx = {
        "addr_to_named": load_addr_to_named(),
        "kengo": load_kengo_matches(),
        "kengo_decisions": load_kengo_decisions(),
        "kengo_claims": load_kengo_claim_counts(),
        "known_psyq": load_known_psyq(),
        "rodata": load_rodata_strings(),
        "sym_comments": load_symbol_comments(),
    }
    print(f"  named symbols: {len(ctx['addr_to_named'])}", file=sys.stderr)
    print(f"  kengo rows: {len(ctx['kengo'])}", file=sys.stderr)
    print(f"  rodata strings: {len(ctx['rodata'])}", file=sys.stderr)
    print(f"  known psyq: {len(ctx['known_psyq'])}", file=sys.stderr)

    # Always parse the full function universe so the call graph is complete.
    print("Parsing all functions...", file=sys.stderr)
    all_funcs = parse_all_funcs()
    if args.limit:
        all_funcs = all_funcs[:args.limit]
    print(f"  {len(all_funcs)} functions", file=sys.stderr)

    print("Building call graph...", file=sys.stderr)
    callers, callees = build_call_graph(all_funcs)
    ctx["callers"] = callers
    ctx["callees"] = callees

    # Filter the analysis universe to a single func if requested.
    if args.func:
        funcs = [f for f in all_funcs if f["name"] == args.func]
        if not funcs:
            print(f"  {args.func}: not found", file=sys.stderr)
            return 1
    else:
        funcs = all_funcs

    print("Analyzing...", file=sys.stderr)
    rows: list[dict] = []
    for f in funcs:
        if not f["name"].startswith("func_"):
            continue  # only propose for unnamed
        row = analyze_one(f, ctx)
        rows.append(row)
        if not args.no_evidence_files and row["evidence"]:
            write_evidence_file(row, ctx, f)

    # Stats
    by_conf = Counter(r["confidence"] for r in rows)
    print("\nProposal counts:", file=sys.stderr)
    for k in ("high", "medium", "low", "none"):
        print(f"  {k:<8s} {by_conf.get(k, 0):>5d}", file=sys.stderr)
    print(f"  total    {len(rows):>5d}", file=sys.stderr)

    # Filter
    if args.confidence:
        thr = CONF_RANK[args.confidence]
        rows = [r for r in rows if CONF_RANK[r["confidence"]] >= thr]
        print(f"  filtered to {args.confidence}+: {len(rows)}", file=sys.stderr)

    # Write CSV
    write_proposals_csv(rows, PROPOSALS_CSV)
    print(f"\nWrote {PROPOSALS_CSV.relative_to(ROOT)}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
