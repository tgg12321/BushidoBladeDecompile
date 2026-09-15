# Evidence bank — _drs

## s1 (recon, 2026-09-15) — matched on the first measurement

- **Baseline:** `canonical _drs` → pure-C target (verdict C, distance 28 <= 50). Target is
  161 insns, 3 back-edges, 0 rules, 0 cheat-asm.
- **Body applied:** the psyz-seed transplant (`memory/grind/_drs/psyz-seed.c`, PsyQ 4.0
  `sys.c:787-833` `_drs`) with the spelling re-aligned to the ALREADY-COMMITTED sibling `_dws`
  (`src/display.c:745`): same clamp ternaries, same `(w*h+1)/2`, same `% 16` / `/ 16`
  split, `while (--var_s0 != -1)` read loop, `if (size)` DMA tail. The only structural
  differences from `_dws` are the ones the seed header predicted: no direction selector
  (constant `0xC0000000`), a second wait loop on status bit `0x08000000`, read loop
  `*data++ = *g_gpu_data_reg`, DMA words `0x04000003` / `0x01000200`.
- **Sandbox first measurement: 28** with 161/161 insns. Side-by-side objdump
  (`tmp/grind/_drs/s1/sidebyside.txt`) shows EVERY instruction identical (mnemonic +
  registers + immediates). The normalized-insn diff (`tmp/grind/_drs/s1/norm.sh`) shows
  all 28 diffs are lui/lw pairs where the REFERENCE side is `lui v0,0x0 / lw v0,0(v0)`
  (unresolved) and the built side is `@hi(0x800a) / @lo(0xbf48|bf44|bf4c|bf50|bf54)`
  (resolved). Cause: the reference object (build/src/display.o from the last full build)
  still held the INCLUDE_ASM body, which references the splat dlabels `D_8009BF44/48/4C/50/54`
  living only in `asm/data/7D920.data.s` — invisible to `engine/score.py::_symtab()`
  ([[score-symtab-blind-to-asm-data-dlabels]]). The C side uses `g_gpu_stat_reg` etc.
  (named_syms.txt) at the same addresses. 14 pairs × 2 = 28 exactly. `_dws` and `_clr`
  (both matched C in the reference) score 0 under the same scorer, confirming the artifact
  is reference-side.
- **Oracle proof:** `engine build` (full clean build) with the C in place →
  `sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle, **MATCH**.
- **Sandbox re-measured after the rebuild: 0** (161/161) — reference now holds the C body
  whose bytes the SHA1 just proved.
- **OBJECT MODEL:** (per the brief's DATA MODEL flags)
  - `D_8009BE78` (g_gpu_disp_x, s16 @ 0x8009BE78): declared `extern s16 D_8009BE78;` at
    `src/display.c:27` (TU-local, which is why include/*.h has no decl). **MATCHES** — measured
    score 0 / SHA1 match; the `lh`/`slt` clamp codegen is byte-identical.
  - `D_8009BE7A` (g_gpu_disp_y, s16 @ 0x8009BE7A): `extern s16 D_8009BE7A;` at
    `src/display.c:28`. **MATCHES** (score 0 / SHA1).
  - `D_8009BF48` / `D_8009BF44` / `D_8009BF4C` / `D_8009BF50` / `D_8009BF54`: these are the
    splat dlabel spellings of `g_gpu_stat_reg` / `g_gpu_data_reg` / `g_gpu_dma_madr` /
    `g_gpu_dma_bcr` / `g_gpu_dma_chcr` (named_syms.txt:48-52, symbol_addrs.txt:59-63),
    declared `extern volatile u32 *` (bcr: `extern u32 *`) at `src/display.c:18-22` — the
    same declarations `_dws` matched with. **MATCHES** (score 0 / SHA1). The "no address
    row" flag in the brief is a naming-index gap (the address rows exist under the `g_gpu_*`
    names), not an object-model mismatch.
- **Constructs in the diff:** none beyond ordinary C. `*(s32 *)&rect->x` / `&rect->w` are
  real packed-word reads of the GPU packet (the GPU consumes `(x,y)` and `(w,h)` as one
  32-bit word each) and are the identical spelling already committed in `_clr`
  (`src/display.c:717`) and `_dws` (`src/display.c:771`). No FAKE annotation, no new
  volatile, no new declarations.
