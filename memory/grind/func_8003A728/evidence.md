# Evidence bank — func_8003A728

## s1 (2026-09-02, recon) — chassis: post-migration main (aa90295c), -mel, no rules

### Baseline
- `canonical func_8003A728` → verdict C, pure-C distance 38 (200 target insns). Pure-C target.
- The pre-migration banked body (`retired-chassis-2026-08/body.c`) is cheat-laden
  (register pins on s0/s2, `asm volatile("" ::: "memory")`, `asm("D_800A36C0")`
  alias renames, `*(volatile s32 *)` cast) and carried 11 regfix rules. It was NOT
  used; the clean draft below was written from the asm + the matched siblings.
- No duplicate/sibling lead: `tmp/duplicates_leads.txt` has no entry for the function.
  Sibling `func_8003A5A0` (matched C, same file) uses the same hash idiom
  (`v0 = a1 ^ (a1 >> 16); v0 = v0 ^ (a0 >> 16); v0 = v0 & 0xFFFF;`) — the
  original programmer's step-wise scratch-variable style is proven in this TU.
- Call `func_8003A450(&D_800A3698)` (target loads `lui/addiu a0,D_800A3698`
  before the jal) while the matched prototype is `s32 func_8003A450(void)`. The
  file already carries `typedef s32 (*FuncBufType)(void *);` for this call shape;
  `((FuncBufType)func_8003A450)(&D_800A3698)` reproduces the a0 setup exactly.

### Floor ladder (all `sandbox --disable all`, each form in tmp/grind/func_8003A728/s1/)
| form | score | what changed |
|---|---|---|
| draft1.c | 38 | clean structured C; control flow already fully aligned (all branches match) |
| v2a.c | 37 | draft1 + `s32 c0lo = (u16)D_800A36C0` (was `u16 c0lo`) |
| v2b.c | 34 | v2a + multi-set `s32 t` staging for the three `& 0xF` loads |
| **v2.c** | **3** | v2b + the u16 low-half loads written into the existing `buf8` local |
| v3a.c | 5 | v2 + hash written as in-place steps on `packed` (no flag reuse) |
| v3b.c | 24 | v3a + `packed = D_800A3916; if (packed != 0)` — ORDER now exact, seats rotated |
| v4a.c | 24 | v3b with `^=` compound steps + `hi16 \| (packed & 0xFFFF)` |
| v4b.c | 26 | v3b with a single `packed = (...) & 0xFFFF` step |

### Mechanisms established (dump-attributed, tmp/grind/func_8003A728/dumps + s1/*.lreg/.greg/.sched)
1. **`or` operand order in the `D_800A38A0 != 0` arm** (`or v0,v0,v1` target vs
   `or v0,v1,v0` ours): with `u16 c0lo` the pseudo is `reg/v:HI` and combine's
   commutative canonicalisation (combine.c:2973, "put a complex expression first")
   swaps the operands once the `zero_extend(reg:HI)` is substituted. An `s32`
   (SImode) `c0lo` holding `(u16)D_800A36C0` keeps expansion order. KILLED the
   HImode spelling, CONFIRMED s32 (v2a, −1).
2. **`lh` vs `lhu` at the three `D_800A36C2 / D_800A36D2 & 0xF` sites**: reading the
   `s16` global directly gives `(set (reg:HI) (mem:HI))` via `movhi_internal2`,
   which mips_move_1word prints as `lhu`, and the `& 15` is applied to a subreg.
   Staging through a multi-set `s32 t` (`t = D_800A36C2; ... = t & 0xF;`, assigned
   three times across the two arms) yields `extendhisi2` (`lh`) + `andi`; combine
   does not fold the sign-extend into the `and` because `t` is a multi-set pseudo
   (combine.c:726 `reg_n_sets > 1` guard on nonzero-bits reasoning). CONFIRMED
   (v2b, −3 = all three sites).
3. **Callee-saved rotation s0/s1/s2 (+ `lhu s0` in both arms, + prologue store order,
   + `andi a0,s0` on the second call path)**: target allocates buf8→s0, param
   copy→s1, lower→s2; a fresh temp for the u16 low halves cannot reach s0 (it does
   not cross a call, so global.c hands it the first free caller-saved reg). Writing
   `buf8 = (u16)D_800A3698;` / `buf8 = (u16)D_800A36D0;` into the existing `buf8`
   local makes the u16 loads part of the call-crossing pseudo (reg 73): its
   ref count rises above the param copy's, global.c allocates it first → s0, then
   param → s1, lower → s2. CONFIRMED (v2, −31). The second call path's
   `func_8003A6FC(buf8 & 0xFFFF)` is NOT CSE'd with `lower` because it sits in a
   different extended basic block — plain recomputation, no trick needed.
4. **The last 3 (v2): `lbu D_800A3916` placement.** Target: `or a0,a0,v0; lbu v0;
   sw a0,D_800A369C; beqz v0`. Ours: `lbu v1; or a0,a0,v0; sw; beqz v1`. sched1
   dump (block 1, reverse list scheduling): T-1 beqz(63), T-2 sw(57);
   T-3 ready = {sw D_800A3698 (48), or (55), lbu (61)}; the lbu is
   function-unit-BLOCKED for 1 cycle (memory unit: a load placed immediately
   before a store costs `max(1, bmax(load)=2 − bmax(store)=1 + 1) = 2`, so
   `unit_tick 5 → adjusted 4 > clock 3`, sched.c:1200-1240 `actual_hazard_this_
   instance`; the same block fires at T-12 for insn 44). So with 48 or 55 READY at
   T-3 the lbu can never sit at T-3. The only way to the target text is a STALL at
   T-3 (empty ready list) followed by lbu at T-4: this requires both 48 and 55 to
   have an unscheduled successor at T-3, i.e. the lbu's DESTINATION pseudo must be
   read by both the packed store (48) and the hash `or` (55). That is exactly one
   scratch variable carrying packed → hash-low → flag (sibling style):
   `packed ^= packed >> 16; packed ^= hi16 >> 16; packed &= 0xFFFF;
   D_800A369C = hi16 | packed; packed = D_800A3916; if (packed != 0)`.
   v3b MEASURED: the order becomes exactly the target's (`or; lbu; sw; beqz`).
   CONFIRMED mechanism; the residual moved to RA.
5. **v3b's new residual = pure RA seat rotation (24 insns, same instructions, same
   order)**: `inverse_compose.py classify` (object path) → FIRST DIVERGENCE: RA.
   greg dispositions (s1/v3b.greg): packed (reg 74) → a0 (target v0), hi16 (75) →
   v1 (target a0), block-local chain temps rotate with them; all callee-saved seats
   (72→s1, 73→s0, 96→s2, 76→a1, 79→v1) stay correct. In v3b `hi16` (75) became a
   LOCAL-alloc qty (`;; Register 75 in 3` in .lreg) — its uses now all sit in one
   block after the in-place chain — whereas the target's a0 seat needs v0 taken by
   packed first. No REG_ALLOC_ORDER in mips.h → numeric order; global.c find_reg
   pass-0 skips regs not yet in `regs_used_so_far`. Not yet solved.
6. `sched_solver` extract for code6cac_c_mid: parity=True, 41 funcs, 876 blocks —
   the model is usable on this TU. `perturb.py --atoms luid,luid_move --depth 2`
   on the v2 state ran >25 min without finishing (whole-function search) and was
   stopped; the manual dump reading above made it unnecessary for this residual.
   Re-run it with `--block` pinned to block 1 if a solver vector is wanted.

### Unmeasured / open
- Whether the flag-into-`packed` reuse is an ordinary scratch-variable idiom
  (sibling func_8003A5A0 proves the hash-step style but not a flag reuse) or needs
  the staged-value-reused-variable FAKE annotation — decide at candidate time.

- [s1] canonical: verdict C, distance 38, 200 insns; no duplicate lead in tmp/duplicates_leads.txt

- [s1] control flow of the clean draft is fully aligned at 38 (every branch matches); all residuals are combine/RA/sched1

- [s1] v2 (candidate.c) = 3: diff is only `lbu v1; or a0,a0,v0` vs target `or a0,a0,v0; lbu v0` (+ beqz reg)

- [s1] sched1 dump block 1: T-3 ready {48 sw packed, 55 or, 61 lbu}; lbu blocked 1 cycle by the memory-unit load-before-store hazard; or picked at T-3

- [s1] v3b greg dispositions: 72->s1, 73->s0, 96->s2, 76->a1, 79->v1 correct; 74 (packed)->a0 and 75 (hi16)->v1 wrong; hi16 became a local-alloc qty in v3b

- [s1] sched_solver extract on code6cac_c_mid: parity=True, 876 blocks; whole-function perturb depth 2 did not finish in 25 min (use --block 1 next time)

- [s1] matched sibling func_8003A5A0 uses the same step-wise hash idiom (v0 = a1 ^ (a1 >> 16); v0 = v0 ^ (a0 >> 16); v0 = v0 & 0xFFFF), supporting the in-place scratch-variable spelling

- [s1] src/code6cac_c_mid.c currently carries v2 (the score-3 candidate) in place of the INCLUDE_ASM line
