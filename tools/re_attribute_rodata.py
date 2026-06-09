#!/usr/bin/env python3
"""Cascade-math helper for the rodata-cleanup project.

Phase 3 tool per docs/rodata-cleanup-project.md §7.2. Models ld's rodata
segment layout from bb2.ld + the .rodata section sizes of each linked .o,
simulates a proposed edit (segment removal / addition / reorder), and reports
the cascade: every downstream symbol whose resolved address would change.

Usage:
  python3 tools/re_attribute_rodata.py layout
      Print the current rodata segment layout (address, size, source).

  python3 tools/re_attribute_rodata.py simulate-remove <block_name>
      Simulate removing `build/asm/data/<block_name>.o(.rodata)` from
      bb2.ld. Report every downstream symbol's address delta.

  python3 tools/re_attribute_rodata.py compensation <block_name>
      Identify the upstream segment and report what compensation
      (source bytes added to a C file's rodata) would preserve the
      oracle when removing the named block.

The tool does NOT modify any files. It writes diagnostic output to stdout.
"""

from __future__ import annotations

import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
BB2_LD = REPO / "bb2.ld"
BUILD = REPO / "build"
MAIN_RODATA_START = 0x80010000  # from bb2.ld `.main 0x80010000` directive

# ld's SUBALIGN(2) gives 2^2 = 4-byte alignment between input sections.
SEGMENT_ALIGN = 4


# Lines we care about in bb2.ld — `.o(.rodata)` entries within main_RODATA.
RODATA_LINE_RE = re.compile(
    r"^\s*(build/(?:src|asm/data)/\S+?\.o)\(\.rodata\)\s*;"
)


def to_wsl_path(p: Path) -> str:
    """Convert a Windows path (e.g. C:\\Users\\...\\build\\src\\foo.o) to its
    /mnt/c/... equivalent so WSL bash + objdump can read it.

    Spaces in the path are preserved literally; the caller must quote the
    result correctly in shell-substituted commands.
    """
    s = str(p)
    if len(s) >= 2 and s[1] == ":":
        drive = s[0].lower()
        rest = s[2:].replace("\\", "/")
        return f"/mnt/{drive}{rest}"
    return s.replace("\\", "/")


@dataclass
class Segment:
    """One link input segment (`build/<path>.o(.rodata)`)."""

    o_path: str  # e.g. "build/src/code6cac.o" or "build/asm/data/800.rodata_pre.o"
    rodata_size: int  # bytes contributed to the linked output (.rodata section size)
    start_addr: int = 0  # resolved load address; filled in by layout()
    symbols: list[tuple[str, int, int]] = field(default_factory=list)
    # symbols is [(name, offset_in_section, size_in_bytes)]

    @property
    def end_addr(self) -> int:
        return self.start_addr + self.rodata_size

    @property
    def short_name(self) -> str:
        return self.o_path.removeprefix("build/").removesuffix(".o")


def parse_bb2_ld() -> list[str]:
    """Return the ordered list of build/...o paths whose .rodata is linked
    into main_RODATA. Order matches bb2.ld."""
    text = BB2_LD.read_text()
    # Restrict to the main_RODATA region: between `main_RODATA_START = .;`
    # and `main_RODATA_END = .;`.
    start_idx = text.index("main_RODATA_START = .;")
    end_idx = text.index("main_RODATA_END = .;")
    region = text[start_idx:end_idx]
    out = []
    for line in region.splitlines():
        m = RODATA_LINE_RE.match(line)
        if m:
            out.append(m.group(1))
    return out


def objdump_section_size(o_path: Path, section: str = ".rodata") -> int:
    """Return the size of `section` in `o_path` (bytes). 0 if absent."""
    if not o_path.exists():
        return 0
    # Use WSL's objdump; this tool runs on the Windows side.
    result = subprocess.run(
        ["wsl", "bash", "-c", f"mipsel-linux-gnu-objdump -h '{to_wsl_path(o_path)}'"],
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        return 0
    # Section header lines look like:
    #   "  6 .rodata       000000d8  ..."
    for line in result.stdout.splitlines():
        toks = line.split()
        if len(toks) >= 3 and toks[1] == section:
            return int(toks[2], 16)
    return 0


def objdump_section_symbols(
    o_path: Path, section: str = ".rodata"
) -> list[tuple[str, int, int]]:
    """Return [(name, offset_in_section, size)] for each symbol in `section`.

    For .o files we read the symbol table; for sections that contain ASM
    `dlabel SYMBOL ... enddlabel SYMBOL` regions, each dlabel produces a
    symbol at the appropriate offset.
    """
    if not o_path.exists():
        return []
    result = subprocess.run(
        ["wsl", "bash", "-c", f"mipsel-linux-gnu-objdump -t '{to_wsl_path(o_path)}'"],
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        return []
    out: list[tuple[str, int, int]] = []
    # Sym lines vary in flag column width:
    #   "00000000 l    d  .rodata    00000000 .rodata"
    #   "00000000 g       .rodata    00000004 D_800100FC"
    # We find the SECTION column by locating the `section` token, then read
    # ADDR (field 0), SIZE (next field), NAME (one after that).
    for line in result.stdout.splitlines():
        if section not in line:
            continue
        toks = line.split()
        if section not in toks:
            continue
        sec_idx = toks.index(section)
        if sec_idx == 0:
            continue
        try:
            addr = int(toks[0], 16)
            size = int(toks[sec_idx + 1], 16)
            name = toks[sec_idx + 2]
        except (IndexError, ValueError):
            continue
        if name == section:  # the section's own placeholder symbol
            continue
        # Skip the SECTION header itself ("l    d  .rodata  00000000 .rodata")
        # — already handled by the name == section check above, but also any
        # debug-annotation lines that lack a proper name field.
        if name.startswith(".") and name not in ("." + section[1:],):
            # e.g. ".internal", ".hidden" — these are visibility tokens
            # inserted between SIZE and NAME; skip if next token is the
            # actual name.
            if sec_idx + 3 < len(toks):
                name = toks[sec_idx + 3]
            else:
                continue
        # The macro.inc `nonmatching SYMBOL` directive in BB2's asm/data
        # files creates a sibling `SYMBOL.NON_MATCHING` annotation symbol at
        # the same address as the real `SYMBOL`. Drop the annotation form
        # so we don't double-count.
        if name.endswith(".NON_MATCHING"):
            continue
        out.append((name, addr, size))
    return sorted(out, key=lambda t: t[1])


def aligned_next(addr: int, alignment: int) -> int:
    return (addr + alignment - 1) // alignment * alignment


def build_layout(o_paths: list[str]) -> list[Segment]:
    """Build the current rodata segment layout."""
    cursor = MAIN_RODATA_START
    segments: list[Segment] = []
    for op in o_paths:
        full = REPO / op
        size = objdump_section_size(full)
        seg = Segment(o_path=op, rodata_size=size)
        seg.start_addr = aligned_next(cursor, SEGMENT_ALIGN)
        seg.symbols = objdump_section_symbols(full)
        segments.append(seg)
        cursor = seg.start_addr + size
    return segments


def cmd_layout() -> int:
    o_paths = parse_bb2_ld()
    segments = build_layout(o_paths)
    print(
        f"{'idx':>3} {'start':<10} {'end':<10} {'size':>6}  {'path':<50}  symbols"
    )
    print("-" * 110)
    for i, seg in enumerate(segments):
        first_sym = seg.symbols[0][0] if seg.symbols else "-"
        last_sym = seg.symbols[-1][0] if seg.symbols else "-"
        sym_summary = (
            f"{len(seg.symbols)} ({first_sym}…{last_sym})" if seg.symbols else "(none)"
        )
        print(
            f"{i:>3} 0x{seg.start_addr:08X} 0x{seg.end_addr:08X} {seg.rodata_size:>6}  "
            f"{seg.short_name:<50}  {sym_summary}"
        )
    return 0


def find_segment_by_block_name(segments: list[Segment], block_name: str) -> int:
    """Find segment index by short block name (e.g. '101C.rodata_post.s' or
    '101C.rodata_post.o' or '101C.rodata_post'). Returns -1 if not found."""
    target = block_name.removesuffix(".s").removesuffix(".o")
    for i, seg in enumerate(segments):
        if seg.short_name == f"asm/data/{target}":
            return i
        # also accept by full short_name
        if seg.short_name.endswith(target):
            return i
    return -1


def simulate_remove(segments: list[Segment], idx: int) -> list[Segment]:
    """Return a new segment list with segment[idx] removed; addresses
    recomputed from MAIN_RODATA_START."""
    new_paths = [s.o_path for j, s in enumerate(segments) if j != idx]
    return build_layout(new_paths)


def cmd_simulate_remove(block_name: str) -> int:
    o_paths = parse_bb2_ld()
    segments = build_layout(o_paths)
    idx = find_segment_by_block_name(segments, block_name)
    if idx == -1:
        print(f"ERR: block '{block_name}' not found in bb2.ld layout", file=sys.stderr)
        return 2

    target = segments[idx]
    print(f"=== removing segment[{idx}] = {target.short_name} ===")
    print(
        f"   rodata size: {target.rodata_size} bytes  "
        f"({hex(target.start_addr)}..{hex(target.end_addr)})"
    )
    print(f"   symbols: {len(target.symbols)}")
    for name, off, size in target.symbols:
        print(f"     - {name}  @ 0x{target.start_addr + off:08X}  ({size}B)")
    print()

    after = simulate_remove(segments, idx)
    # Map old-segment-by-path -> new-segment
    after_by_path = {s.o_path: s for s in after}

    # Compute symbol-level diff for every downstream segment
    shifted_segments = []
    shifted_symbols = []
    for old_seg in segments[idx + 1 :]:
        new_seg = after_by_path.get(old_seg.o_path)
        if not new_seg:
            continue
        delta = new_seg.start_addr - old_seg.start_addr
        if delta != 0:
            shifted_segments.append((old_seg, new_seg, delta))
            for name, off, size in old_seg.symbols:
                old_addr = old_seg.start_addr + off
                new_addr = new_seg.start_addr + off
                shifted_symbols.append((name, old_addr, new_addr, delta))

    print(f"=== cascade: {len(shifted_segments)} segments shift, "
          f"{len(shifted_symbols)} symbols shift ===")
    if not shifted_symbols:
        print("   ZERO cascade — segment can be removed without compensation")
        return 0

    print(f"   total bytes shifted downstream: "
          f"{sum(s.rodata_size for s, _, _ in shifted_segments)}")
    print(f"   uniform delta: {shifted_segments[0][2]:+d} bytes "
          f"(applies to ALL downstream symbols)")
    print()
    print("=== first 10 shifted symbols ===")
    for name, old, new, delta in shifted_symbols[:10]:
        print(f"   {name:<32} 0x{old:08X} -> 0x{new:08X}  ({delta:+d})")
    if len(shifted_symbols) > 10:
        print(f"   ... and {len(shifted_symbols) - 10} more")
    print()
    print("=== compensation required to preserve oracle ===")
    print(f"   Need {target.rodata_size} bytes of rodata added to an upstream")
    print(f"   segment to absorb the shift. Upstream segment in current layout:")
    if idx == 0:
        print("     (none — this is the first segment; would need to compensate"
              " differently, e.g. shift main_RODATA_START)")
    else:
        upstream = segments[idx - 1]
        print(f"     {upstream.short_name}  size={upstream.rodata_size}B  "
              f"@ 0x{upstream.start_addr:08X}..0x{upstream.end_addr:08X}")
        if upstream.o_path.startswith("build/src/"):
            c_file = upstream.short_name.removeprefix("src/") + ".c"
            print(f"     -> source file: src/{c_file}")
            print(f"     -> compensation: add {target.rodata_size} bytes of rodata")
            print(f"        to src/{c_file} matching the removed block's content")
            print(f"        AND requiring evidence per §8.1 (the bytes must")
            print(f"        plausibly belong to that TU in the original source).")
        else:
            print(f"     -> upstream is another asm/data block, not a C file —")
            print(f"        compensation can't be done via .c source. Either")
            print(f"        a deeper restructure or a different edit op is needed.")
    return 0


def cmd_compensation(block_name: str) -> int:
    """Detailed compensation report: dump the block's raw bytes + upstream
    file info, so a human can vet whether the compensation is feasible."""
    o_paths = parse_bb2_ld()
    segments = build_layout(o_paths)
    idx = find_segment_by_block_name(segments, block_name)
    if idx == -1:
        print(f"ERR: block '{block_name}' not found in bb2.ld layout", file=sys.stderr)
        return 2

    target = segments[idx]
    print(f"=== compensation analysis: {target.short_name} ===")
    print(f"   rodata size: {target.rodata_size} bytes")
    print(f"   address range: 0x{target.start_addr:08X}..0x{target.end_addr:08X}")
    print()

    # Dump the block's raw bytes
    full = REPO / target.o_path
    if full.exists():
        result = subprocess.run(
            [
                "wsl",
                "bash",
                "-c",
                f"mipsel-linux-gnu-objdump -s -j .rodata '{to_wsl_path(full)}'",
            ],
            capture_output=True,
            text=True,
            check=False,
        )
        if result.returncode == 0:
            print("=== block content (objdump -s -j .rodata) ===")
            in_section = False
            for line in result.stdout.splitlines():
                if "Contents of section .rodata" in line:
                    in_section = True
                    print(line)
                elif in_section:
                    if line.strip() == "" or line.startswith("Contents"):
                        break
                    print(line)
            print()

    if idx == 0:
        print("ERR: this is the first segment; no upstream to compensate via")
        return 0
    upstream = segments[idx - 1]
    print(f"=== upstream segment ===")
    print(f"   {upstream.short_name}")
    print(f"   {upstream.rodata_size}B @ 0x{upstream.start_addr:08X}..0x{upstream.end_addr:08X}")
    if upstream.o_path.startswith("build/src/"):
        c_file = REPO / "src" / (
            upstream.short_name.removeprefix("src/") + ".c"
        )
        print(f"   source: {c_file.relative_to(REPO)}")
        print(f"   compensation feasible IF evidence supports adding {target.rodata_size}")
        print(f"   bytes of rodata to this file's source (a static const at the")
        print(f"   end of the file's emitted .rodata section).")
        print()
        print("=== evidence checklist (§8.1) ===")
        print("   [ ] Is there a natural place in the .c file for these bytes?")
        print("      (e.g. a static const array at end-of-file, a string literal,")
        print("       a switch jump table)")
        print("   [ ] Do the bytes match a pattern that source could produce?")
        print("      (e.g. all-zero = a static const initialized to zero;")
        print("       repeated value = an array; pointer-shaped = a dispatch table)")
        print("   [ ] Is there cross-reference evidence (sibling decomps,")
        print("      function-data adjacency, byte signatures matching")
        print("      well-known compiler output)?")
        print("   If ANY answer is no, the compensation is speculative (forbidden")
        print("   per §8.1). Look at the next-upstream C file or consider a")
        print("   different cluster.")
    else:
        # Upstream is another asm/data block — multi-block cascade required.
        print(f"   Upstream is also an asm/data block, not a C file.")
        print(f"   Direct source compensation isn't possible at this position;")
        print(f"   the segment must be retired together with at least one of:")
        print(f"   - the upstream asm/data block (multi-block cascade)")
        print(f"   - by relocating ownership to a different C file upstream")
        print(f"   This case requires deeper analysis.")
    return 0


def main(argv: list[str]) -> int:
    if len(argv) < 2:
        print(__doc__.strip(), file=sys.stderr)
        return 1
    cmd = argv[1]
    if cmd == "layout":
        return cmd_layout()
    elif cmd == "simulate-remove":
        if len(argv) < 3:
            print("usage: simulate-remove <block_name>", file=sys.stderr)
            return 1
        return cmd_simulate_remove(argv[2])
    elif cmd == "compensation":
        if len(argv) < 3:
            print("usage: compensation <block_name>", file=sys.stderr)
            return 1
        return cmd_compensation(argv[2])
    else:
        print(f"unknown command: {cmd}", file=sys.stderr)
        print(__doc__.strip(), file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
