# Evidence bank — D_80082050

## s1 (2026-07-18, recon)
- Baseline: canonical verdict C, 180/180 insns, sandbox `--disable all` distance 1, 0 regfix/asmfix rules.
- The body at HEAD is the PsyQ 4.0 LIBCD cdread.c `cb_read` clone; all volatile externs it uses
  (D_800A14D4..D_800A1500) are operator-granted in `volatile_extern_allowlist.txt` (§3 grants,
  2026-07-10). No cheat-asm is attributed to this function; the sandbox's 7 file-wide strips all land
  in OTHER system.c functions (lines 135/182/562 register pins, line 476 volatile alias).
- Positional diff of the cheat-stripped sandbox object vs the ORIGINAL EXE bytes at 0x80082050:
  0/180 mnemonic, register, branch-target, or resolved-immediate differences
  (tmp/grind/D_80082050/s1/cmp.py, cmp2.py, resolve.py; all jal/hi16/lo16 targets verified against
  build/bb2.map addresses).
- The engine's single counted diff (engine-normalized, tmp/grind/D_80082050/s1/engdiff.py):
  insn 34 `addiu a0,a0,%lo(D_80082320)` — reference .o addend 0x2294 vs sandbox .o addend 0x18F4.
  Both are exactly `D_80082050_offset + 0x2D0` in their respective objects (symbol tables verified):
  each object is internally consistent and links to the same VA. The divergence exists only because
  the file-wide cheat strip shrinks earlier system.c functions by 0x9A0 bytes, shifting the static
  sibling's section offset. **Scorer artifact, not a codegen diff** — score.py's "relocations render
  identically at the .o level" assumption breaks for section-local (static) symbol addends.
- Fix (byte-neutral, measured s1): give `D_80082320` external linkage (drop `static` at decl+def).
  The relocation then targets the named global with addend 0 in BOTH objects. Full clean build after
  the edit: SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (oracle MATCH) — bytes identical.
  Sandbox `--disable all` after reference rebuild: **distance 0** (180/180).
- Note for siblings: `D_80082050` itself remains static; its own address-taken reference inside
  saEft00Add (system.c:1141 cdrom_SetCallbackB) can produce the same artifact when scoring
  saEft00Add. Same remedy would apply there if it surfaces.
