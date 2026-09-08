# Evidence bank — func_800393C8

## s1 (2026-09-08, recon) — chassis re-measure closed the function

- CHASSIS: HEAD chassis = `-mel -msoft-float` (softfloat adopted 2026-09-07). Every pre-migration
  measurement in `brief-2026-08-18.md` / `retired-chassis-2026-08/` was taken on the HARD-float
  chassis and is superseded by the numbers below.
- OBJECT MODEL: `D_800F68E0` — declared `extern s16 D_800F68E0[];` (code6cac.h); census
  g_se_slot_table_180x16 (180 x 0x10-byte SE slot table). MATCHES (measured score 0): the target
  addresses it only as `lui/addiu %hi/%lo(D_800F68E0)` base + literal byte offsets through two
  strength-reduced induction pointers (base, base+0xE) and as `D_800F68E0 + idx*0x10`; the
  candidate's `u8 *slot = (u8 *)D_800F68E0` with literal field offsets reproduces every byte. No
  struct-typing signal was needed to reach 0; the record layout (s16 state @0, u8 age @2, u8 kind
  @3, s16 pos[3] @4/6/8, u16 rot_mode @0xA (12-bit rot | mode<<12), u16 rot1 @0xC, u16 rot2 @0xE)
  is documented here for the sibling func_800395B4 / saSeInit family should anyone type it later.
- Recovered the 2026-08-18 WIP candidate from git (`git show 2f20c593:memory/wip/func_800393C8/candidate.c`;
  the `memory/wip/` directory was deleted in 3e8527af) and applied it over the `INCLUDE_ASM` line.
  `canonical` = C (pure-C target). Sandbox `--disable all` on the current chassis: **10** (121 vs
  123 insns), not the 26 the old brief recorded — the softfloat chassis halved loop.c's LICM
  threshold, so loop 1 no longer hoists the `li -1` sentinel; loop 1 became byte-identical with no
  source change. Diff: `tmp/grind/func_800393C8/s1/diff_wip.txt`.
- Remaining 10 were all in loop 2 (the free-slot search from `D_800A3714`): (a) target hoists
  `li t1,-1` into the loop preheader — the goto-loop spelling in the WIP candidate gets no loop
  notes so nothing is hoisted; (b) target carries the new index through TWO copies (`addiu
  v0,t0,1; move v1,v0; sh v1; ... bnez; move t0,v1`), i.e. two distinct HImode pseudos for the
  loop-carried index and the just-stored index; (c) the `mode` (`arg0 << 12`) seat was t0 vs
  target v1.
- Loop-2 spelling sweep (`tmp/grind/func_800393C8/s1/scores.txt`), all on the current chassis:
  `while (idx < 0xB4) { if (*slot == -1) break; idx = idx + 1; D_800A3714 = idx; slot += 0x10; }`
  = 8 (LICM now hoists the sentinel; loop 2 gets real loop notes). `for` equivalent = 8. `if
  (idx < 0xB4) do { ... } while (D_800A3714 < 0xB4)` = 15 (GCC rotates the inner do/while and
  duplicates the exit test, as the old ledger also saw). `s32 next` temp inside the while = 10
  (124 insns — an SImode temp adds a truncation copy instead of removing one).
- `s16 cur; cur = idx + 1; D_800A3714 = cur; slot += 0x10; idx = cur;` = **2** (123 insns): the
  s16 temp IS the second HImode pseudo; the store comes from it and the loop-carried `idx` is
  re-assigned from it at the bottom (reorg puts that copy in the back-branch delay slot).
  Placement variants (`for (...; idx = cur)`, extra `if (cur >= 0xB4) break;`, `idx =
  D_800A3714` re-read instead of `idx = cur`) all = 2; storing before `slot += 0x10` = 4.
- Dropping the `cur` temp for `D_800A3714 = idx + 1; slot += 0x10; idx = D_800A3714;` = **6**
  (122 insns) — the temp is load-bearing; a plain global re-read does not create the second pseudo.
- Last 2: `mode = arg0 << 12` computed BEFORE the `D_800A3714 == 0xB4` check keeps the pseudo
  live across `lh v1,0(gp); li v0,0xB4; bne` so it cannot take v1. Computing it after the check
  (either as a local or inlined into the `(arg3[0] & 0xFFF) | (arg0 << 12)` store) = **0**; reorg
  then lifts the `sll` from the store block into the `bne` delay slot exactly as in the target.
- `u8 age` (instead of `s32 age; cnt = age & 0xFF`) = 0 as well — the target's `andi v1,t3,0xFF`
  is GCC's zero-extension of the QImode local for the two comparisons, and `addiu v0,t3,1` is the
  unextended `age + 1` byte store. Adopted (no redundant mask in the source).
- Final body (`memory/grind/func_800393C8/candidate.c`, in place in `src/code6cac_c_mid.c`):
  sandbox `--disable all` = **0** (123/123), and a full `verify-oracle` build with the C in
  place produced SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle (no asmfix rules
  remain, so the old +452-byte duplication failure mode is gone).
- Provenance note for the reviewer: the loop-2 shape `cur = idx + 1; D_800A3714 = cur; idx = cur`
  mirrors the already-COMPLETED-C sibling func_800395B4 (`D_800A379C = idx + 1; idx = idx + 1;`,
  src/code6cac_c_mid.c ~1275) — same table-walk / global-index-publish logic.
