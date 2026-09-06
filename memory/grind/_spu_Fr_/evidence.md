# Evidence bank — _spu_Fr_

## s1 (2026-09-06, recon) — CLOSED at sandbox 0

- OBJECT MODEL: D_800A2CDC (g_spu_base_addr, `extern s32`) MATCHES — used as
  the SPU register base for the two `*(volatile u16 *)(base+0x1A6/0x1AA)`
  accesses, identical shape to _spu_init/_spu_t. D_800A2D2C (g_spu_xfer_pending,
  `extern s32`) MATCHES — plain `sw` of 1, no volatile needed (measured 0).
  D_800A2CE0 / D_800A2CE4 / D_800A2CE8 (g_spu_dma_{madr,bcr,chcr}_reg_ptr):
  MISMATCH as declared `extern s32 *` (measured score 2); they are Sony
  _spu_madr/_spu_bcr/_spu_chcr, `.data` words 0x1F8010C0/0x1F8010C4/0x1F8010C8
  (asm/data/7D920.data.s:33123,33129,33135) = DMA channel 4 MADR/BCR/CHCR, i.e.
  inside the MMIO range covered by `.claude/rules/mmio-volatile-type-level.md`.
  Declared `extern volatile s32 *D_800A2CEx;` (pointee volatile, pointer plain):
  _spu_Fr_ sandbox 0 AND sibling _spu_t (the only other C user, main.c _spu_t
  case-store block) stays sandbox 0 (164/164). engine/volatile_cheats.py:30-33
  explicitly classes pointer-to-volatile externs as legitimate (no allowlist
  entry; same shape as the pre-existing `extern volatile u32 *D_800A2CEC;` and
  `g_spu_dma_ctrl` in main.c).
- Reference body: sotn-decomp src/main/psxsdk/libspu/spu.c `_spu_r_` (fetched
  2026-09-06) is statement-for-statement the BB2 body, with WASTE_TIME() inlined
  twice per site; BB2's build calls the out-of-line `_spu_Fw1ts` once per site.
  Target frame 0x20 (=16 arg bytes + s0/s1/ra) leaves NO room for SOTN's
  vestigial `volatile s32 sp0, sp4` locals, so they are not present here
  (contrast _spu_FwriteByIO, memory/closer/phase3-progress.md:345-360).
- The 2-insn residual mechanism (pass-attributed from dumps, not guessed): with
  plain pointer stores, post-sched2 order is `call _spu_FsetDelayR; li a0;
  ori a0; lw v0,D_800A2CE0; sll s0; sw s1,0(v0); lw v0,D_800A2CE4;
  ori s0,s0,0x10; sw s0,0(v0)` (dumps/main.sched2 insns 37,68,69,40,46,42,45,
  47,49). reorg's fill_simple_delay_slots forward walk (reorg.c:3044-3160)
  fills call2's slot with the sll (walking through call3 with maybe_never) and
  call3's slot with the ori — nothing between call3 and the ori sets/refs s0,
  and `ori` cannot trap. Target has the nop in call3's slot. With the pointee
  volatile, the skipped `sw s1,0(v0)` is a volatile MEM: mark_set_resources
  sets `set.volatil` (reorg.c:363/668) and resource_conflicts_p
  (reorg.c:752-760) returns TRUE for EVERY later trial once either side carries
  `volatil`, so the ori can no longer be lifted over the madr store — exactly
  the target shape (s1/v4.s: `#.set volatile` around both DMA-register stores,
  ori sits in the lw load delay).
- cc1psx calibration (tools/cc1psx_wrapper.sh on the plain-pointer TU,
  tmp/grind/_spu_Fr_/s1/v1_psx.s): the ORIGINAL compiler also lifts the ori
  into the FsetDelayR slot from the plain-pointer C — the residual was source
  typing, not a fork divergence.
- Per-site casts `*(volatile s32 *)D_800A2CE0 = ...` (s1/v4_volatile_ptrs.c)
  also measure 0 but are the wrong spelling under the MMIO rule (declaration-
  level is the sanctioned form); superseded by the declaration form.
