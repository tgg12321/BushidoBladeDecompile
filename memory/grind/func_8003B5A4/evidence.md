# Evidence bank — func_8003B5A4

## Session s1 (2026-08-07, recon) — CLOSED to sandbox 0

- **Baseline:** canonical verdict C (179 insns, distance 19, "pure-C target"). Sandbox
  `--disable all` = 19 with 10 regfix rules dropped (regfix.txt lines 1294-1307: a
  `$3 <-> $4` swap + substs + reorder over maspsx 36-49, and `$2 -> $5` substs +
  reorder over 61-66) and 4 file-level cheat-asm blocks stripped (none inside this
  function's body).
- **Function shape:** command-byte interpreter loop. `do { cmd = *(u8*)D_800A3844++; switch(cmd) }
  while (!done)` over jtbl_80010D1C (21 cases, default exits loop). s0 = done flag,
  s1 = &D_8010277D (chardata).
- **Diff at floor 19** (tmp/grind/func_8003B5A4/s1/rawdiff.txt + normdiff.py): two clusters,
  both the SAME mechanism — cases 3 and 1 in the old C cached the byte(s) through an
  `s32 data` local before storing the advanced pointer back to D_800A3844. Target
  instead: load pointer into a fresh local, store p+1 to the global FIRST, then load the
  byte THROUGH the pointer; the pointer dies at its last load so its register is reused
  for the byte (case 3: p in $a0, byte0 $v1, byte1 loaded directly into $v0 at its store
  site; case 1: p in $a1 — $a0 already holds the preloaded &D_80101EC8 call arg — and
  `lbu $a1, 0($a1)` reuses it).
- **Fix (pure C, no FAKE):** per-case `u8 *p = (u8 *)D_800A3844;` + target statement
  order (global-advance store before byte load; `chardata[2] = p[1];` loaded at its use
  site instead of pre-cached). Sandbox went 19 -> 0 in one step.
- **True-zero verification:** raw objdump-vs-target diff (normdiff.py) shows exactly ONE
  residual line: `lw v0,116(at)` vs `lw v0,0(at)` — the R_MIPS_LO16 addend of
  jtbl_80010D1C (offset 0x74 inside this TU's .rodata in the isolated object). This is a
  relocation/link-layer artifact, not codegen (cf. memory
  sandbox-lo16-text-addend-false-distance); the integrated link places the table at the
  referenced address. No register or ordering diffs remain.
- **Remaining to COMPLETED-C (operator/driver steps, outside grind-session surface):**
  retire the 10 regfix.txt rules for func_8003B5A4 (lines 1294-1307) + full-build SHA1
  verify + layer-2 cheat-reviewer. The candidate body contains no volatile coercion, no
  dead stores, no pins, no FAKE constructs — plain pointer locals with live uses only.

## Artifacts (s1)
- tmp/grind/func_8003B5A4/s1/diffdump.sh — extract+normalize ours-vs-target
- tmp/grind/func_8003B5A4/s1/normdiff.py — normalizing differ
- tmp/grind/func_8003B5A4/s1/ours.txt / target.txt / rawdiff.txt — floor-0 state
