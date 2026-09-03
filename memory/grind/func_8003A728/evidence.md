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


## s3 (2026-09-02, structural) - chassis: HEAD (1c8fa981), -mel, no rules; w3 re-measured 3, chassis UNCHANGED

### What this session did
s2 left a "trichotomy" argument that was exhaustive only by assertion. s3 replaced it with a
DIRECT READING of the sched1 decision (instrumented cc1, BB2_SCHED_DEBUG=1) and then closed the
dependence space by construction. The floor is still 3 (w3 remains the candidate), but the block-1
residual is now a closed enumeration rather than an argument, and the exact GCC predicate that
blocks the only register-consistent shape is identified and cited.

### The sched1 trace, verbatim (tmp/grind/func_8003A728/s3/code6cac_c_mid.sched, block 1)

    ;; ready list at T-1: 68 (7fffff53), now 68        <- beqz
    ;; ready list at T-2: 61 (2), now 61               <- sw D_800A369C  (ONLY insn ready)
    ;; launching 65 before 61 with no stalls at T-3
    ;; ready list at T-3: 58 (2) 65 (7f000001)
    ;; blocking insn 65 for 1 cycles, now 58           <- the lbu is queued, the ior takes T-3
    ;; launching 65 before 58 with no stalls at T-4
    ;; ready list at T-4: 56 (2) 65 (7f000001), now 65 56

and the hazard arithmetic behind "blocking insn 65", from the instrumented build
(tmp/grind/func_8003A728/s3/sched_debug.txt):

    SCHEDDBG BLOCKAGE unit=0 clock=3 raw_tick=5 adj_tick=4 maxb=3 exec=65 last=61
    SCHEDDBG SELBLOCK clock=3 insn=65 unit=0 cost=1

i.e. after the STORE issued at clock 2 the memory unit tick is 2 + max_blockage(3) = 5; the
load-after-store blockage function returns 2, so adj_tick = 5 + 2 - 3 = 4 > clock 3 and
actual_hazard (sched.c:2685) returns cost 1. mips.md:153-161 is the source of the asymmetry: the
memory unit ready delay is 2 for a load (r3000) and 1 for a store, and only the load-after-store
direction produces a non-zero blockage. Nothing about the SOURCE changes this: the flag read is a
byte load and the D_800A369C write is a store, so the pair is fixed.

### Consequence: the target text is reachable ONLY through a T-3 stall
Backward list scheduling emits in decreasing clock. beqz is forced to T-1 and insn 61 is the ONLY
insn ready at T-2 (the trace shows a one-element ready list), so the store is pinned at T-2. The
load can therefore never occupy T-3. The target text (or / lbu / lui+sw / beqz) is consistent with
exactly one schedule: T-5=58 (or), T-4=65 (lbu), T-3=STALL, T-2=61 (sw), T-1=68 (beqz). A stall at
T-3 requires the ready list at T-3 to be EMPTY, i.e. insn 58 must not be ready, i.e. 65 must be a
DEPENDENT of 58 (in the backward sense of sched.c an insn is ready only once every insn that
depends on it has been scheduled). 58 is an `or` reg-reg with no function unit, so it can never be
blocked; only a dependence can hold it back.

### The dependence space, closed by construction (not by assertion)
65 is a load with no register inputs, so a dependence 58 -> 65 can only be:
  (a) ANTI - 65 writes a pseudo that 58 READS. 58 reads exactly two pseudos, hi16 and packed.
  (b) OUTPUT - 65 writes the DEST of 58.
and the additional constraint is that insn 61 (the store) must NOT also read the pseudo 65 writes,
otherwise 61 becomes a dependent of 65 too, 61 is pushed below the lbu, and the lbu lands AFTER the
store (maspsx then inserts a load-delay nop, 201 insns). With the store reading the dest of 58,
that leaves exactly five arrangements, ALL now measured on this chassis:

| # | shape | carrier | score | why it fails |
|---|---|---|---|---|
| 1 | `hi16 = hi16 \| packed; D_800A369C = hi16; packed = D_800A3916;` (v3b/z0) | packed (anti) | 24 | order EXACT; packed gets two REG_DEAD notes -> local-alloc.c:471 -> global_alloc -> a0 |
| 2 | `T = hi16 \| packed; D_800A369C = T; hi16 = D_800A3916;` (v6a/x2) | hi16 (anti) | 6-10 | order exact; pseudo 75 would need a0 (hi16 uses) AND v0 (flag use) at once |
| 3 | `hi16 = hi16 \| packed; D_800A369C = hi16; hi16 = D_800A3916;` (v7b/v9a) | hi16 (anti), store reads it too | 6-10 | lbu pushed past the store, +nop |
| 4 | `packed = hi16 \| packed; D_800A369C = packed; hi16 = D_800A3916;` (s3 y6) | hi16 (anti), or-dest = packed | 10 | same one-pseudo-two-seats conflict as #2 |
| 5 | `flag = hi16 \| packed; D_800A369C = flag; flag = D_800A3916;` (s3 y1) | or-dest (OUTPUT) | 6 | the store reads the or-dest, so 61 depends on 65: lbu emitted AFTER the sw, +nop, 201 insns |

Arrangements 4 and 5 are new this session and are exactly the "fourth arrangement" the s2 frontier
asked for; both are now dead. For this insn set there is no sixth carrier.

### Why arrangement 1 (the only register-CONSISTENT one) resists spelling
Arrangement 1 is the shape whose register geometry AGREES with the target: the target really does
use $v0 for both the hash accumulator and the flag (`andi v0,v0,0xffff` ... `lbu v0,0(gp)` ...
`beqz v0`), so packed and the flag sharing one seat is what the target bytes show. The failure is
purely allocator ordering, measured this session on the z0 dumps:
  * .lreg: "Register 74 used 12 times across 12 insns in block 1; dies in 2 places" - two disjoint
    live ranges, so the `reg_n_deaths[i] == 1` conjunct at tools/gcc-2.7.2/local-alloc.c:471 fails
    and reg 74 never becomes a local quantity.
  * .greg dispositions: 74 in 4 (a0), 75 in 3 (v1). local_alloc runs FIRST and hands $v0 to the
    first block-1 local quantity it processes (the sra temps) and $v1 to hi16; by the time
    global_alloc reaches reg 74 (12 refs, the highest-priority allocno in the block) $v0 is already
    conflicted across the whole of 74's span, so 74 takes $a0 and the 24-insn rotation follows.
  * The obvious counter-lever - remove the competing block-1 local quantities so $v0 is free at
    global_alloc time - was measured and is INERT: naming the two sra results (z10, z11, z13) and
    staging one of them through the existing multi-block `t` local (zt3, zt4) reproduce 24
    byte-for-byte, because combine re-collapses the named temps and the qty landscape does not
    move. Staging BOTH sra results through `t` (zt 33, zt2 21) is strictly worse - it perturbs the
    `& 0xF` staging sites in the later arms.

### s3 measurement table (all `sandbox --disable all`, forms in tmp/grind/func_8003A728/s3/)
| form | score | shape |
|---|---|---|
| w3 (candidate) | 3 | re-measured, chassis unchanged |
| y1 | 6 | or-dest `flag` reused for the flag read (OUTPUT dep) - 201 insns, lbu after the sw |
| y2 | 25 | `packed = hi16 \| packed; D_800A369C = packed; packed = D_800A3916;` |
| y3 | 5 | `D_800A369C = hi16 \| packed;` with no named or-dest + fresh flag |
| y5 | 6 | or-dest `packed`, fresh flag - `or v0,v0,a0` operands swapped |
| y6 | 10 | or-dest `packed`, flag into hi16 (the s2 frontier fourth arrangement) |
| z0 | 24 | v3b re-spelled on the w3 chassis (control) |
| z10/z11/z13 | 24 | named / shared / or-dest-shared sra temps - byte-neutral on z0 |
| z12 | 12 | named shared sra temp on the w3 (fresh-flag) chassis - NOT neutral there |
| zt/zt2 | 33/21 | both sra results staged through the existing `t` local |
| zt3/zt4 | 24 | one sra result staged through `t` - byte-neutral on z0 |

- [s3] Chassis re-measured unchanged at dispatch: w3 = 3, residual still exactly three insns (lbu one slot early + beqz reg).
- [s3] The sched1 blockage is now READ, not inferred: SCHEDDBG BLOCKAGE unit=0 clock=3 raw_tick=5 adj_tick=4 maxb=3 exec=65 last=61 -> actual_hazard (sched.c:2685) cost 1. mips.md:153-161 gives the memory unit ready delay 2 for a load and 1 for a store, so only load-after-store blocks.
- [s3] Insn 61 (sw D_800A369C) is the ONLY insn ready at T-2 (";; ready list at T-2: 61 (2), now 61"), so the store is pinned immediately before the branch and the flag load cannot occupy T-3; the target text is reachable only via an empty ready list at T-3.
- [s3] The dependence space that can empty the T-3 ready list is closed for this insn set: 65 has no register inputs, so only an ANTI dep on one of the two pseudos 58 reads (hi16, packed) or an OUTPUT dep on the dest of 58 can do it, and the store must not read the same pseudo. All five resulting arrangements are measured (24 / 6-10 / 6-10 / 10 / 6).
- [s3] Arrangement 1 (v3b/z0, flag into packed) is the only one whose register geometry agrees with the target bytes ($v0 carries both the hash accumulator and the flag there); its failure is allocator ORDERING - reg 74 has two REG_DEAD notes, fails local-alloc.c:471, and global_alloc reaches it after local_alloc has already given $v0 to a block-1 local quantity.
- [s3] Removing the competing block-1 local quantities does not free $v0: naming the sra temps (z10/z11/z13) or staging one through the multi-block `t` (zt3/zt4) is byte-identical to z0 at 24 because combine re-collapses them; staging both (zt 33, zt2 21) damages the later `& 0xF` sites.
- [s3] Tooling note for the next session: `wsl.exe bash tmp/grind/func_8003A728/s3/dbg.sh` runs the instrumented cc1 (tools/gcc-2.7.2/cc1) over the TU with BB2_SCHED_DEBUG=1 into s3/sched_debug.txt plus a full -da dump set; `wsl.exe bash tmp/grind/func_8003A728/s3/meas.sh` prints the pairdiff. `tools/wsl.sh` does NOT work from the Bash tool on this host (no wsl on PATH there) - call `wsl.exe bash <script>` from PowerShell instead, and apply bodies with s3/apply.py (latin-1, sentinel-delimited).

- [s3] Chassis re-measured unchanged at dispatch: memory/grind/func_8003A728/candidate.c (w3) scores 3 and the residual is still exactly three insns - `lbu v1,0(gp)` one slot before the `or a0,a0,v0` instead of after it, plus the resulting `beqz v1` vs `beqz v0`.

- [s3] The sched1 decision is now READ rather than inferred. Block-1 trace: T-1 = 68 (beqz), T-2 = 61 (sw D_800A369C, the ONLY insn ready), T-3 ready = {58 (pri 2), 65 (pri 0x7f000001)} with `;; blocking insn 65 for 1 cycles, now 58`, T-4 = 65. The instrumented build prints the arithmetic: SCHEDDBG BLOCKAGE unit=0 clock=3 raw_tick=5 adj_tick=4 maxb=3 exec=65 last=61, SELBLOCK cost=1.

- [s3] mips.md:153-161 is the source of the asymmetry: the `memory` function unit has ready delay 2 for a load on r3000 and 1 for a store, and only the load-after-store direction yields a non-zero blockage - so no source spelling can let the flag lbu sit at T-3 behind the D_800A369C store.

- [s3] Because the store is pinned at T-2 and the load is blocked at T-3, the target text (or / lbu / lui+sw / beqz) is consistent with exactly one schedule: T-5 = or, T-4 = lbu, T-3 = STALL, T-2 = sw, T-1 = beqz. A stall requires an EMPTY ready list at T-3, i.e. insn 58 must not be ready, i.e. the lbu must be a dependent of the ior.

- [s3] The flag load has no register inputs, so the only dependences that can make the ior unready are an ANTI dep on one of the two pseudos the ior reads (hi16, packed) or an OUTPUT dep on the ior's destination - and the store must not read that same pseudo. That yields exactly five arrangements and all five are now measured: 24 (v3b/z0, carrier packed), 6-10 (v6a/x2, carrier hi16 with a fresh or-dest), 6-10 (v7b/v9a, carrier hi16 in place), 10 (s3 y6, or-dest packed + carrier hi16), 6 (s3 y1, output dep on the or-dest).

- [s3] Arrangement 1 (v3b/z0) is the only one whose register geometry agrees with the target's own bytes - the target uses $v0 for the hash accumulator (`andi v0,v0,0xffff`) AND for the flag (`lbu v0,0(gp)`, `beqz v0`) - and it already reproduces the target's instruction ORDER exactly. Its 24-insn residual is pure allocator ordering.

- [s3] z0 dumps pin that ordering: .lreg reports reg 74 (packed) with two REG_DEAD notes, which fails the `reg_n_deaths[i] == 1` conjunct at tools/gcc-2.7.2/local-alloc.c:471, so 74 never becomes a local quantity; .greg then shows `74 in 4` ($a0) and `75 in 3` ($v1) because local_alloc has already given $v0 to a block-1 local temp spanning 74's range.

- [s3] Naming or re-staging the block-1 `sra` temporaries does not change that landscape - z10/z11/z13/zt3/zt4 are byte-identical to z0 at 24 because combine re-collapses them - so the counter-lever of freeing $v0 before global_alloc is not reachable by temp spelling alone.

- [s3] Tooling for the next session: `wsl.exe bash tmp/grind/func_8003A728/s3/dbg.sh` runs the instrumented cc1 with BB2_SCHED_DEBUG=1 and writes s3/sched_debug.txt plus a full -da dump set; `wsl.exe bash tmp/grind/func_8003A728/s3/meas.sh` prints the pairdiff; s3/apply.py swaps a body file into src/code6cac_c_mid.c (latin-1, sentinel-delimited). `bash tools/wsl.sh` does NOT work from the Bash tool on this host - wsl is not on that PATH; call wsl.exe from PowerShell.

## s4 (2026-09-02, permuter) — chassis: HEAD (8fbb8b41), -mel, no rules; w3 re-measured 3, chassis UNCHANGED. **FLOOR 0 — full build SHA1 == oracle.**

### Result
`sandbox func_8003A728 --disable all` = **0**, and `verify-oracle` reports
`build_sha1 = 62efab4f73f992798c43e8c730aa43baa10bb4fa`, `build_matches: true`, with the
candidate body in `src/code6cac_c_mid.c`. The entire delta from the s2/s3 candidate (w3, 3) is
ONE construct: a `zero` constant-holder local set in block 1 and read once at the tail test
`if (D_800A3730 != zero || (D_800A36C0 & 0x40000000))`, carrying the mandatory `/* FAKE */`
annotation. Everything else in the body is unchanged from w3.

### Chassis + kill re-audit (mandated; all `sandbox --disable all` on HEAD 8fbb8b41)
| form | s3 score | s4 re-measure |
|---|---|---|
| w3 (candidate.c as inherited) | 3 | 3 |
| z0 (arrangement 1, flag into `packed`) | 24 | 24 |
| y1 (arrangement 5, OUTPUT dep on the or-dest) | 6 | 6 |
Every banked instance kill therefore still holds on the current chassis. `fake_ablate.py` had no
units to ablate (the inherited candidate carried NO `/* FAKE */` annotations), so the ablation was
run by hand over the three carried constructs instead:
| ablation of the w3 body | score |
|---|---|
| buf8 low-half reuse -> fresh `low` local | **36** (the reuse is worth 33 insns) |
| multi-set `s32 t` staging -> direct `D_800A36C2 & 0xF` reads | 6 |
| `s32 c0lo` -> `u16 c0lo` | 4 |
No carried construct sat on the block-1 pseudos the s3 kills targeted, so none of the s1-s3 kills
was FAKE-carrier-confounded.

### The permuter workspace (reusable)
`tmp/perm_8003A728_s4/` — full-TU compile + per-function extraction, built by
`tmp/grind/func_8003A728/s4/mkws.sh` (which calls `mktgt.sh` for `target.o` and copies
`compile.sh`). Validation before launch: `base.o` vs `target.o` mnemonic diff was EXACTLY the
known 3-insn residual (`lbu v1` one slot early + `beqz v1`) and nothing else.
Gotcha for future sessions: the `cut -c33-` idiom copied from the func_8003C714 workspace scripts
CUTS THE WHOLE INSTRUCTION AWAY when objdump prints small (section-relative) addresses — every
line compares equal and a broken workspace looks perfect. Use
`awk -F'\t' 'NF>=3 {print $3"\t"$4}'` instead (`tmp/grind/func_8003A728/s4/dis.sh`).

### The campaign
  launch : `python3 tools/permuter_campaign.py launch --func func_8003A728 --dir tmp/perm_8003A728_s4 --label s4-w3-candidate-chassis -j 6 --stop-on-zero`
  base_score 70 (permuter weighted metric; objdump distance 3)
  **found score 0 at iteration 201, 10.4 s after launch**; harvest --stop, elapsed 146.4 s,
  finds_total 1, best_new_score 0.
A second campaign on a structurally different chassis (`tmp/perm_8003A728_s4z/`, the z0 /
arrangement-1 body, base_score 225, label s4-z0-arrangement1-chassis, -j 5) ran in parallel and
was harvested + stopped once the first campaign closed: 3 finds, best 70, no improvement on its
own base. Both campaigns are stopped; neither outlives the session.

### The find, and its minimisation
The permuter's `output-0-1/source.c` added TWO locals to the w3 body:
`int new_var2; new_var2 = 0;` in block 1, read as `D_800A3730 != new_var2` at the tail, and
`int new_var; new_var = 8;` read as `*(s32 *)(a0 + new_var)`. Hand-minimised on the real chassis:
| form | score |
|---|---|
| p1 both holders | 0 |
| **p2 zero-holder alone** | **0** |
| p3 eight-holder alone | 3 (byte-neutral — DROPPED) |
| f1 zero-holder as a statement in block 1 (`s32 zero; ... zero = 0;`) | **0** |
| f2 zero-holder as a declaration initializer (`s32 zero = 0;`) | 3 |
The statement position matters: the initializer form is hoisted out of block 1 and is inert. The
committed form is f1.

### Mechanism (dump-attributed this session, tmp/grind/func_8003A728/dumps/)
1. `.flow` shows the holder as `(insn 24 ... (set (reg/v:SI 80) (const_int 0)) ...
   (expr_list:REG_EQUAL (const_int 0)))` inside block 1.
2. `.sched` shows insn 24 STILL PRESENT in sched1's block-1 insn list
   (`;; insn[  24]: priority = 1, ref_count = 1`) and scheduled at T-2 —
   `;; ready list at T-2: 24 (7f000001) 64 (2), now 24 64`. That is the slot the D_800A369C store
   (insn 64) occupied in every previous form, and s3 proved the store being pinned there is
   exactly what function-unit-blocked the D_800A3916 flag lbu out of T-3 (memory-unit
   load-after-store, sched.c:2685 + mips.md:153-161). With the store displaced, the lbu takes T-3
   (`;; launching 68 before 24 with no stalls at T-3`) and the emitted order becomes the
   target's.
3. `.lreg` has NO entry for register 80: `update_equiv_regs` (tools/gcc-2.7.2/local-alloc.c:947,
   constant case at local-alloc.c:1031) reads the REG_EQUAL note, substitutes the constant at the
   single use and deletes the set. So the holder costs ZERO instructions — the function is 200
   insns, like the target.
This is the s3 frontier's second hypothesis (change the block-1 INSN SET rather than the
dependence carrier) CONFIRMED, with the twist that the added insn does not have to survive to the
output: it only has to exist during sched1.

### Natural alternatives measured (all worse — this is the lever-exhaustion record for the FAKE)
Every ordinary-C way of adding a block-1 quantity that carries a REAL value:
| form | score |
|---|---|
| q1 `lower = buf8 & 0xFFFF;` hoisted, used in `packed` and both `func_8003A6FC` calls | 29 |
| q2 same hoist, used only in `packed` | 5 |
| q3 `fld = D_800A3730;` staged, used in `packed` and the tail test | 14 |
| q4 `hdr = *(s16 *)a0;` staged | 3 |
| q5 `mode = D_800A3870 & 3;` staged | 6 |
Together with s2's five `hi16`-placement measurements and s3's five-arrangement dependence
closure, the modality ladder for this residual is spent: no ordinary-C form of the block-1 insn
set reaches 0, and the only measured form that does is the sanctioned constant-holder.

- [s4] FLOOR 0. `sandbox func_8003A728 --disable all` = 0 and `verify-oracle` = build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa / build_matches true, with the s4 candidate body in src/code6cac_c_mid.c (measured on HEAD 8fbb8b41).
- [s4] The whole delta from the inherited w3 candidate (3) is ONE `/* FAKE */`-annotated constant-holder local: `s32 zero;` declared, `zero = 0;` as a statement in block 1, read once as `D_800A3730 != zero`. p2/f1 = 0; the same holder written as a declaration initializer (f2) is inert at 3, so the STATEMENT POSITION inside block 1 is load-bearing.
- [s4] Mechanism, read from this session's dumps: `.flow` insn 24 is `(set (reg/v:SI 80) (const_int 0))` with a REG_EQUAL note; `.sched` still lists insn 24 in block 1 and schedules it at T-2, displacing the D_800A369C store (insn 64) out of the slot before the branch that had function-unit-blocked the flag lbu since s1; `.lreg` has no register 80 at all, because update_equiv_regs (tools/gcc-2.7.2/local-alloc.c:947, constant case at :1031) substitutes the constant and deletes the insn. The holder therefore emits no instruction and the function is 200 insns like the target.
- [s4] This CONFIRMS the s3 frontier's block-1-insn-set hypothesis, with the refinement that the added insn need not survive to the output — it only has to exist during sched1.
- [s4] Every ordinary-C alternative that adds a block-1 quantity carrying a REAL value was measured and is worse: hoisting `buf8 & 0xFFFF` into a local (29 used at all three sites, 5 used only in `packed`), staging D_800A3730 (14), staging `*(s16 *)a0` (3), staging `D_800A3870 & 3` (6).
- [s4] Kill re-audit on HEAD 8fbb8b41: w3 = 3, z0 = 24, y1 = 6 — every banked instance kill still holds on the current chassis. fake_ablate had no units (the inherited candidate carried no FAKE annotations), so the three carried constructs were ablated by hand: removing the buf8 low-half reuse costs 33 insns (3 -> 36), removing the multi-set `t` staging costs 3 (3 -> 6), `s32 c0lo` -> `u16 c0lo` costs 1 (3 -> 4).
- [s4] Permuter campaign telemetry: tmp/perm_8003A728_s4 (label s4-w3-candidate-chassis, base_score 70, -j 6, --stop-on-zero) found score 0 at iteration 201, 10.4 s after launch; the parallel structurally-different chassis campaign tmp/perm_8003A728_s4z (z0 body, base_score 225, -j 5) produced 3 finds with best 70 and no improvement. Both harvested with --stop before the session ended.
- [s4] Workspace gotcha worth carrying forward: the `cut -c33-` mnemonic-extraction idiom copied from the func_8003C714 permuter scripts silently cuts the ENTIRE instruction away when objdump prints small section-relative addresses, so a mis-built workspace compares byte-perfect. Use `awk -F'\t' 'NF>=3 {print $3"\t"$4}'` (tmp/grind/func_8003A728/s4/dis.sh) to validate base.o against target.o.
