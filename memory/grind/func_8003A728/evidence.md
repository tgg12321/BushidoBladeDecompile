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


## s2 (2026-09-02, structural) - chassis: HEAD (9de438a2), -mel, no rules; v2=3 and v3b=24 re-measured identical, chassis UNCHANGED

### What this session changed
The floor is still 3, but the SHAPE at 3 is completely different and the block-1 residual is
now understood end to end. s1 was stuck in a two-horn dilemma: v2 (3) had the right registers
and the wrong lbu slot; v3b (24) had the right order and 24 rotated seats. s2 dissolved the
dilemma's second horn: v3b's rotation is NOT a search problem, it is one mechanical predicate.

### The predicate that explains every 24 (the s1 frontier's dead end)
local-alloc.c:472:  `if (reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1 && ...) reg_qty[i] = -2;`
Only pseudos with EXACTLY ONE REG_DEAD note become local-alloc quantities. Reading the flag
into `packed` gives reg 74 two disjoint live ranges (hash value, then flag) -> two deaths ->
reg 74 is skipped by local-alloc and falls through to global_alloc. By then every block-1 local
temp has already taken v0/v1, so global_alloc hands packed a0. Confirmed by the s2 .lreg slice:
"Register 74 used 12 times across 13 insns in block 1; dies in 2 places" and the greg
disposition "74 in 4" (a0) / "75 in 3" (v1), with NO ";; Register 74 in" line in .lreg.
Measured on FOUR independent flag-into-packed spellings (v3b, v7a, x3, x4) and on two different
hi16 placements: all 24. This is the single fact behind s1's whole H5 frontier.

### The route that worked (structural, all measured with sandbox --disable all)
| form | score | what changed vs the previous row |
|---|---|---|
| v2 (s1 candidate) | 3 | re-measured, chassis unchanged |
| v3b (s1) | 24 | re-measured, chassis unchanged |
| v5a | 24 | v3b minus the two never-used `s32 v0; s32 v1;` locals - BYTE-NEUTRAL (hygiene) |
| v5b/v5c/v5d/v5e | 24 | hi16-before-packed, decl-order swaps, store-before-hi16: all inert on v3b |
| **v6a** | **10** | flag read into `hi16` instead of `packed` -> packed keeps ONE death -> local qty -> v0 |
| v6b | 26 | same idea but with s1's nested one-expression hash - the hash must be in-place steps |
| v6c | 10 | or-dest `packed` instead of a fresh temp - same seat problem as v6a |
| v7a | 24 | in-place ior + flag back into packed - back to the two-deaths predicate |
| v7b | 10 | in-place ior + flag into hi16 - or-dest seat correct, lbu pushed past the store (+nop, 201 insns) |
| v7c | 3 | in-place ior + FRESH `flag` local |
| **v8c** | **6** | v6a with `hi16 = D_800A37C4 << 16;` moved AFTER the first hash step |
| v8a/v8b/v8d/v8e | 14/10/16/14 | the other four placements of that one statement |
| v9b | 5 | v8c + in-place ior + fresh `flag` |
| **w3** | **3** | v9b with the hi16 statement between the D_800A3698 store and the first hash step |
| w1 | 3 | hi16 before the store - byte-identical outcome to w3 |
| w2/w4/w5/w6/w7 | 14/12/12/12/12 | other placements / load-shift splits of the hi16 statement |
| z2/z3 | 3 | flag statement before the ior; `if (D_800A3916 != 0)` with no flag local - both inert |
| z4 | 27 | separate `raw` local for the pre-hash value - re-rotates block 1 |

### w3 = the new candidate (3). Its residual, in full
TGT: ... andi v0,v0,0xffff / or a0,a0,v0 / lbu v0,0(gp) / lui at / sw a0,0(at) / beqz v0
BLD: ... andi v0,v0,0xffff / lbu v1,0(gp) / or a0,a0,v0 / lui at / sw a0,0(at) / beqz v1
Three differing instructions. EVERY other instruction in the function, including all 24 seats
that v3b rotated and the six block-1 schedule slots that v6a/v8c still had wrong, now matches.

### Why the last 3 are hard - the exact sched1 geometry (s2 .sched slice, w3)
Post-sched1 RTL order is 56 (andi) -> 65 (lbu, reg/v:SI 76) -> 58 (ior into reg/v:SI 75)
-> 61 (sw D_800A369C) -> 68 (beqz). Backward list scheduling fills T-1=68, T-2=61, then at T-3
the ready list holds 58 and 65. rank_for_schedule (sched.c:2407-2465) sorts ascending and the
LAST element wins: INSN_PRIORITY(65)=2 (load->branch latency) beats INSN_PRIORITY(58)=1
(alu->store), and 65 also has the larger INSN_LUID, so 65 would win the tie either way. It does
not win because it is function-unit BLOCKED at T-3 (a load immediately preceding a store on the
memory unit, s1's H4 measurement, sched.c:1200-1240). So 58 takes T-3 and 65 drops to T-4.
The target's text requires a STALL at T-3, i.e. 58 must ALSO be unavailable at T-3.
58's LOG_LINKS in the dump are "48, REG_DEP_ANTI 53, 56"; its only dependent is 61 (already at
T-2), so it is ready. The only insn that can make it unready is 65 itself, via an anti-dep -
i.e. the lbu must write a pseudo that 58 READS, and 58 reads exactly two pseudos: 75 (hi16) and
74 (packed). Both branches of that fork are now measured dead on this chassis:
 * lbu writes 74 (packed): two deaths -> global alloc -> a0. 24. (v3b, v7a, x1, x3, x4)
 * lbu writes 75 (hi16), ior writes hi16 in place: then 61 reads 75 too, so the anti-dep pushes
   the lbu PAST the store and maspsx adds a load-delay nop. 6-10. (v9a, v7b)
 * lbu writes 75 (hi16), ior writes a fresh temp T: the order is then EXACTLY right, but in the
   target's own order T is born at the ior, the flag is born at the lbu, and T dies at the sw -
   T and the flag overlap, so T can never share a0 with hi16. `or v0,a0,v0`/`lbu a0` instead of
   `or a0,a0,v0`/`lbu v0`. 6-10. (v8c, v6a, v6c, x2)
That trichotomy is exhaustive for spellings in which block 1 contains only these insns; the
untried axis is therefore adding/removing an insn from block 1, or attacking the function-unit
blockage itself rather than the readiness of 58.

- [s2] chassis re-measured UNCHANGED: v2 = 3, v3b = 24, identical to s1's numbers
- [s2] local-alloc.c:472 `reg_n_deaths[i] == 1` is the gate that sends a twice-dying pseudo to global_alloc; it is the single cause of every 24-score form in this ledger
- [s2] the two never-used `s32 v0; s32 v1;` locals inherited from s1's drafts are byte-neutral (v3b 24 == v5a 24) and are dropped from the candidate
- [s2] w3 (memory/grind/func_8003A728/candidate.c) = 3 with EVERY register seat correct; residual is 3 insns: lbu one slot early + beqz reg
- [s2] rank_for_schedule (sched.c:2407-2465) gives the flag lbu priority 2 vs the ior's 1 and the larger LUID, so source-order/LUID levers cannot flip T-3; only the memory-unit blockage keeps the lbu out
- [s2] statement position of `hi16 = D_800A37C4 << 16;` is worth up to 11 insns on its own (v8a 14 / v8b 10 / v8c 6 / w3 3 / w2 14) - the strongest single structural lever found in block 1

- [s2] Chassis re-measured unchanged at dispatch: s1's v2 still scores 3 and s1's v3b still scores 24, so every s1 spelling conclusion is still chassis-valid.

- [s2] local-alloc.c:472 ('reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1') is the gate behind every 24-score form in this ledger; a pseudo that carries two disjoint live ranges is never given a local-alloc quantity and therefore cannot win the low caller-saved seats.

- [s2] New candidate w3 = 3 with EVERY register seat in the function matching the target; the entire residual is three instructions: 'lbu v1' one slot before the 'or a0,a0,v0' instead of after it, plus the resulting 'beqz v1' vs 'beqz v0'.

- [s2] Post-sched1 RTL for w3 (dumps sliced this session): 56 andi -> 65 lbu (reg 76) -> 58 ior (reg 75) -> 61 sw D_800A369C -> 68 beqz. rank_for_schedule (sched.c:2407-2465) gives 65 priority 2 vs 58's priority 1 AND the larger INSN_LUID, so 65 loses T-3 only because of the memory-unit blockage (load immediately preceding a store), not because of any source-order tie.

- [s2] Because the lbu already outranks the ior on both priority and LUID, source-order and declaration-order levers cannot flip the T-3 decision; the only remaining handles are the readiness of insn 58 (all three anti-dependence carriers now measured) or the function-unit blockage itself.

- [s2] The two never-used 's32 v0; s32 v1;' locals inherited from s1's drafts are byte-neutral (v3b 24 == v5a 24) and have been dropped from the candidate as hygiene.

- [s2] The statement position of 'hi16 = D_800A37C4 << 16;' alone spans 14 / 10 / 6 / 3 / 14 across five placements - the strongest single structural lever measured in block 1.
