# func_800747D8 — INTEGRATION HANDOFF (s10, 2026-09-20)

**Status: BYTES PROVEN ON MAIN.** A full driver build with the three edits below
produced `build/bb2.exe` SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa` —
the oracle — with `func_800747D8` compiled from pure C (no cheat-asm, no
`INCLUDE_ASM`, zero regfix/asmfix rules).

The grind session could only touch `src/text1b.c`; two of the three edits are on
surfaces a grind session may not touch (`Makefile`, a sibling rodata TU), so the
session reverted everything and returned `owner-gated` as an integration handoff.

## Why the sandbox still reports 2/208 and always will

`sandbox func_800747D8 --disable all --diff` reports **0 source-level hunks,
1 operand-only hunk, 27 not-scored**. The single scored hunk is

    target  lw v0,0(at)      # %lo(jtbl_80015A0C)
    ours    lw v0,24(at)     # %lo(.rodata) + 24

i.e. the `%lo` addend of the switch's jump-table base. That addend is decided by
where the compiler-emitted `ADDR_VEC` lands inside `build/src/text1b.o`'s single
`.rodata`, which is a *link-geometry* property, not a C property. The isolated
sandbox links nothing, so it can never score 0 here. The instrument for this
residual is the FULL build, not the sandbox.

## The three edits (exact)

### 1. `Makefile:136` — add `text1b` to `RODATA_ALIGN2_FILES`

    -RODATA_ALIGN2_FILES := ... text1a_c2 text1b_b main
    +RODATA_ALIGN2_FILES := ... text1a_c2 text1b_b text1b main

`tools/gcc-2.7.2/final.c:1515-1518` emits, unconditionally, an
`ASM_OUTPUT_ALIGN(file, exact_log2(BIGGEST_ALIGNMENT / BITS_PER_UNIT))` —
`.align 3`, i.e. 8 bytes — immediately before every `ADDR_VEC` placed in
`.rdata`. `jtbl_80015A0C` sits at `0x80015A0C`, which is **4 mod 8**, so with
`.align 3` in force the table can never land there from ANY 8-aligned section
base. The project already has the fidelity knob for exactly this
(`Makefile:146 rodata_align_fix`, a `sed 's/\.align\t3/.align\t2/'` stage)
and 13 files already opt in — including `text1b_b`, whose `func_80077B30`
table sits at the 4-mod-8 address `0x80015A3C`. `text1b` simply was not on the
list yet.

Census (`tmp/grind/func_800747D8/s10/jtbl_align_census.py`): of the 65 distinct
`jtbl_*` symbols in the binary, **25 are 8-aligned and 40 are 4 mod 8**. ASPSX
2.34 / psylink plainly did not 8-align jump tables; `RODATA_ALIGN2_FILES` is the
project's standing compensation for that.

### 2. `src/text1b.c` — the C body + rodata ownership

See `text1b.c.patch` / `text1b.c.proven` in this directory. Two parts:

* Replace `INCLUDE_ASM("asm/funcs", func_800747D8);` (line 8391 at HEAD) with the
  body in `memory/grind/func_800747D8/candidate.c`.
* Move `D_800159A0`, `jtbl_800159B0` and `jtbl_800159D0` out of
  `src/text1a_b_mid_rodata.c` and into `src/text1b.c`, placed **after**
  `func_8006B578` (which emits `jtbl_80015988`) and **before** `func_800747D8`.
  GCC emits file-scope initialized objects in declaration order, so this makes
  `build/src/text1b.o(.rodata)` a single contiguous run
  `0x80015988 .. 0x80015A20` (24 + 16 + 32 + 60 + 20 = 152 = 0x98 bytes) with
  the `ADDR_VEC` at offset `0x84` → address `0x80015A0C`. The pre-existing
  `extern u8 D_800159A0[];` forward declaration (line 8223 at HEAD) must be
  deleted, since the definition now precedes its use.

### 3. `src/text1a_b_mid_rodata.c` — give up those three, keep the tail word

See `text1a_b_mid_rodata.c.patch`. The file drops `D_800159A0`,
`jtbl_800159B0`, `jtbl_800159D0` and `jtbl_80015A0C`, and gains

    const u32 D_80015A20[1] = { 0x00000000 };

`jtbl_80015A0C` was transcribed as 6 words, but the dispatch is
`sltiu $v0, $v1, 0x5` (asm/funcs/func_800747D8.s:67) — the real table is 5 words,
`0x80015A0C..0x80015A20`. The 6th transcribed word is the independent zero word
at `0x80015A20`, which `text1a_b_mid_rodata.o` must keep supplying.
Resulting section sizes: `text1b.o(.rodata)` 0x98 @ align 2**2,
`text1a_b_mid_rodata.o(.rodata)` 0x1c @ align 2**2. `bb2.ld` needs NO change.

## Verification command actually run (s10)

    bash tools/wsl.sh "cd '/mnt/c/.../Bushido Blade 2 Decompile' && source .venv/bin/activate && \
      make RODATA_ALIGN2_FILES='code6cac code6cac_b code6cac_c code6cac_c0 code6cac_c_ab \
      code6cac_c2 text1a_pre text1a_post text1a_b text1a_c text1a_c2 text1b_b text1b main' check"
    -> OK: bb2 matches!   (sha1sum build/bb2.exe == 62efab4f73f992798c43e8c730aa43baa10bb4fa)

(The `RODATA_ALIGN2_FILES=` command-line override reproduces edit 1 without
modifying the Makefile, which a grind session may not do.)

## Still required before `queue done`

A fresh layer-2 `cheat-reviewer` on the C body (see `../self_vet.md` — the
session's own reading is that the body is ordinary C and claims no sanctioned
family), then `engine queue done func_800747D8`.
