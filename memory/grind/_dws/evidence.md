# Evidence bank — _dws  (src/display.c, 0x8007CE0C, PsyQ libgpu sys.c `_dws` = LoadImage body)

## s1 (2026-09-07, recon) — MATCHED. Floor 143 (INCLUDE_ASM) -> 32 -> 24 -> 0.

### OBJECT MODEL: every global/aggregate the target touches, declared shape vs evidence
Verdict per symbol, all measured on the score-0 chassis of s1 (sandbox `_dws --disable all` = 0):

- **`D_8009BE78`** (census `g_gpu_disp_x`; psyz/SOTN `info.w`, the VRAM page width limit).
  Declared `extern s16 D_8009BE78;` at src/display.c:26. **MATCHES.** The target does
  `lui/lh %lo(D_8009BE78)` — a *signed* halfword load — and then `slt $v0,$v0,$a1` against
  the signed halfword `rect->w`. A wider or unsigned declaration changes the load mnemonic.
  The DATA MODEL sweep flagged it as "decl: NONE in include/*.h": that is correct and
  harmless — the declaration lives in the .c, not a header.
- **`D_8009BE7A`** (census `g_gpu_disp_y`; `info.h`). Declared `extern s16 D_8009BE7A;` at
  src/display.c:27. **MATCHES**, same `lh` + signed-`slt` evidence.
- **`D_8009BF48`** — NOT a scalar. The target does `lw %lo(D_8009BF48)($v0)` and then
  `lw $v0,0($v0)`: a **double indirection**, i.e. the cell holds a POINTER. display.c
  already declares it correctly as `extern volatile u32 *g_gpu_stat_reg;` (src/display.c:18,
  `g_gpu_stat_reg = 0x8009BF48` in named_syms.txt:48). **MATCHES.** The
  `extern s32 D_8009BF48;` the sweep found in m2c_context.h is an m2c scratch decl, is not
  in the build's include path for display.c, and is contradicted by the second `lw`.
- **`D_8009BF44`** -> `extern volatile u32 *g_gpu_data_reg;` (src/display.c:19). **MATCHES**
  (same pointer-then-deref shape, 6 sites).
- **`D_8009BF4C`** -> `extern volatile u32 *g_gpu_dma_madr;` (src/display.c:20). **MATCHES.**
- **`D_8009BF50`** -> `extern u32 *g_gpu_dma_bcr;` (src/display.c:21). **MATCHES** (note this
  one is deliberately non-volatile in display.c and that is what reaches 0).
- **`D_8009BF54`** -> `extern volatile u32 *g_gpu_dma_chcr;` (src/display.c:22). **MATCHES.**
- **The parameter aggregate — the one real MISMATCH, now fixed.** src/display.c:683-687 carried
  `typedef struct { s32 unk0; s16 x; s16 y; } _GpuChunkHdr_CE0C;`, which models the first GPU
  packet word as one `s32` and puts w/h at the wrong names. The target's own accesses settle it:
  `lh 0x4` / `sh 0x4` and `lh 0x6` / `sh 0x6` are the two clamped halfwords, and `lw 0x0` /
  `lw 0x4` are the two packed GPU packet words. The correct object is Sony's `RECT`:
  `{ s16 x; s16 y; s16 w; s16 h; }` (committed as `GpuRect`). This is the
  [[split-scalars-hide-aggregate]] shape at struct-member granularity. **Measured: with the
  s32-unk0 spelling the clamp/multiply region cannot be reached at all; with `GpuRect` the whole
  function reaches 0.**

### The match, and how it was reached
1. Applied `memory/grind/_dws/psyz-seed.c` (the psyz PsyQ-4.0 transplant a prior sweep had
   banked but never measured) with the `GpuRect` retype. **48 (pre-migration ledger floor) -> 32**,
   143 target insns vs 142 built. The whole prologue, both clamps, the divide expansion, the
   GP0 wait loop, the DMA tail and the epilogue were already instruction-identical on the
   first measurement — including the 0x50 frame and the s0/s1/s2/s3/s4/s5/ra save set.
2. **The single divergence was the odd-word copy loop's spelling.** psyz/SOTN write
   `while (var_s0--)`, which GCC 2.7.2 compiles to "keep a pre-decrement copy, compare it to 0"
   (`move a0,s0` before the loop, `move v1,s0` inside, `beqz`/`bnez`). The target instead
   materialises `-1` in a register hoisted above the loop and compares the *post*-decrement
   value against it (`li v0,-1` / `beq s0,v0` / `li a0,-1` / `bne s0,a0`). Spelling it as
   **`while (--var_s0 != -1)`** (semantically identical for var_s0 >= 0) reproduces exactly
   that. **32 -> 24, and 143/143 insns.**
3. The residual 24 was a **scorer artifact, not codegen**: `engine/score.py::_symtab()` resolves
   `%hi/%lo` pairs only for symbols defined in `LD_SYM_FILES`
   (undefined_funcs_auto.txt / undefined_syms_auto.txt / named_syms.txt). The C side references
   `g_gpu_stat_reg`/`g_gpu_data_reg`/... which ARE in those files, so it resolves to
   `@hi(0x800a)/@lo(0xbf48)`; the reference object (assembled from asm/funcs/_dws.s) references
   `D_8009BF48`/`D_8009BF44`/..., which are NOT in any of the three files (they are `T` symbols
   defined in the asm data segment — `nm build/bb2.elf` shows `8009bf48 T D_8009BF48` and
   `8009bf48 A g_gpu_stat_reg`, the same address), so it keeps the literal `0x0`. Twelve
   `lui`+`lw` pairs x 2 lines = exactly the 24 points. **Both sides link to identical words.**
4. Proven two independent ways:
   - `tmp/grind/_dws/s1/bytecheck.py` applies this object's R_MIPS_HI16 / R_MIPS_LO16 /
     R_MIPS_26 relocations with the real linked symbol addresses (from `nm build/bb2.elf`,
     with `.text`-relative relocs mapped through the object's own symbol table) and compares
     all 143 resulting words against the **shipped encodings carried in asm/funcs/_dws.s's
     per-line comments**: `MISMATCHED WORDS: 0`. This proof does not depend on `build/` and is
     not circular.
   - `verify-oracle --rebuild --allow-dirty`: `build_sha1 =
     62efab4f73f992798c43e8c730aa43baa10bb4fa`, `build_matches: true`.
   After the rebuild the sandbox reads 0 as well (143/143, 0 rules dropped).

### Durable facts worth carrying to the siblings `_clr` and `_drs`
- **`GpuRect { s16 x, y, w, h; }` is the right parameter object for all three** libgpu
  chunk functions; `_GpuChunkHdr_CE0C` (s32 unk0 + s16 x + s16 y) was wrong and is now gone
  from display.c. `_clr`/`_drs` take the same struct.
- **`while (--n != -1)` is BB2's spelling of psyz/SOTN's `while (n--)`** in this unit. `_drs`
  has the identical odd-word copy loop; expect the same substitution to be needed there.
- **The `var_s4` transfer-direction selector is genuine Sony source, not a coercion**:
  sotn-decomp `src/main/psxsdk/libgpu/sys.c:613/620/640` (PSX main binary, GCC 2.7.2 —
  `config/splat.us.main.yaml:43` lists `psxsdk/libgpu/sys`) ships `int var_s4; ... var_s4 = 0;
  ... *GPU_DATA = var_s4 ? 0xB0000000 : CMD_COPY_CPU_TO_VRAM;`, and psyz
  `decomp/src/libgpu/sys.c:765/771` ships the same. GCC 2.7.2 does **not** fold it here; the
  psyz-seed's warning that it would (and the retired 2026-08 chassis's `register asm("s5")`
  pin + empty-`__asm__` barrier built to defend against that fold) was **wrong**. The plain
  `var_s4 = 0;` reaches the target's `addu $s5,$zero,$zero` / `beqz $s5` unaided.
- **Do not hand-expand the `/2`, `%16`, `/16`**: written as C division, GCC emits its own
  round-toward-zero sequence (`srl 31; addu; sra 1` and `sra 5` + `sll 4; subu`) which is
  exactly the target.
- `set_alarm()` / `get_alarm()` take no arguments here. The retired chassis passed
  `get_alarm((u32 *)0xA0000000)` to park the 0xA0000000 constant in `$a0`; that is unnecessary —
  GCC hoists the `lui $a0,0xa000` of the ternary's false arm into the wait loop's delay slots on
  its own.
- **Scorer caveat for the whole libgpu cluster**: any function in this unit whose asm still uses
  the stale `D_8009BF4x` splat names while the C uses `g_gpu_*_reg` will show a false
  `%hi/%lo` distance of 2 points per access. Confirm with `tmp/grind/_dws/s1/bytecheck.py`
  before believing a nonzero residual made only of `lui/lw` pairs.
