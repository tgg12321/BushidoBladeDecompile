# _SsVmKeyOnNow — evidence (manual session 2026-09-24)

Closed from the cold queue floor 301 in one manual session. Measured trail:

| step | spelling | score |
|---|---|---|
| s0 | psyz vm_nowon.c port (psyz itself keeps this INCLUDE_ASM — non-matching) | 22 |
| s1 | psyz pan semantics flipped to SOTN (pan<64 scales volr) but single chL/chR vars | 25 |
| s2 | SOTN two-stage vars (voll_t/volr_t -> voll/volr, first pan stage assigns both) | 12 |
| s3 | chain in volr_t, then `voll_t = volr_t` (SOTN has it the other way round) | 2 (frame only) |
| s4 | `(seq_sep_no & 0xFF00) >> 8` (SOTN + ps2sdk) instead of `(>> 8) & 0xFF` | **0** |

References: SOTN `tmp/sotn-decomp/src/main/psxsdk/libsnd/vmanager.c:116`
(SpuVmKeyOnNow, compiled on PSX), ps2sdk
`tmp/ps2sdk-reference/iop/sound/libsnd2/src/vm/vm_nowon.c`, psyz
`tmp/psyz-ref/decomp/src/libsnd/vm_nowon.c` (wrong pan channel, psyz-only).

## The frame (the last 2 points)

Target frame 16 bytes of locals, never touched. Mechanism (BB2_FRAME_DEBUG census,
`tools/gcc-2.7.2/cc1`): each byte is a reload stack slot (`spill_new_pNNN`, size 8,
align -1) for a pseudo whose only remaining reference is a `(use (reg))` that
combine's `distribute_notes` plants at a CODE_LABEL after a 3->2 combine
(newi2pat kept => elim_i2 == 0 => the REG_DEAD note for i2dest finds no
reference and falls through to the block label).

- Orphan #1 (both spellings): the `&D_80102A78[pos+2]` pitch store — the `sym+4`
  pseudo is shared with the voll/volr stores, so the first fold keeps it.
- Orphan #2: only with `(x & 0xFF00) >> 8`; `(x >> 8) & 0xFF` gives the same
  instructions but no orphan. That is the whole 8-vs-16 difference.

Frame probe: `tmp/_SsVmKeyOnNow/frameprobe.py` (+ `census.sh`), batch compile-only,
`INSTR=1` for the census. Ruled out as frame sources (all stayed 8): param types,
u16/s16/s32 keyon, struct-array `_svm_voice`, pointer-form sreg/dirty stores,
assignment-as-value on the okon globals, chained `voll_t = volr_t = ...`,
`ss` at declaration, VabHdr struct pointer.
