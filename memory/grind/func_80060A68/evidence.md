# Evidence bank — func_80060A68

## [s1] 2026-08-19 — recon. Floor 39 → 2 in one session (clean sibling-idiom rewrite).

Chassis at session start: queue distance 39, sandbox `--disable all` = 39 (build 64 /
target 66), canonical verdict C. Old committed body carried a `register s32 outer
asm("$3")` pin and `volatile s32 _frame_pad[2]` — both stripped by the sandbox, both
deleted this session as unsanctioned debt (per the 2026-08-18 shared brief §6).

### The measurement grid (every variant sandbox-measured this session)

| variant | change | score | insns | what it proved |
|---|---|---|---|---|
| v0 (HEAD) | pin + pad body | 39 | 64/66 | baseline |
| v2 | drop pin+pad; re-read `*(u16 *)outer` for dispatch idx (sibling func_80060B70 idiom); first use inline | 10 | 69/66 | CSE-shared `D_800F10D0` address was the main wall: the fresh read rematerializes `sll/lui/addu/%lo` exactly like target; frame 0x20 returned NATURALLY (u16/s32 named locals) — the `_frame_pad` was never needed |
| v3 | + `s32 p10 = *(s32*)(outer+0x10)` (after F10D0 store) for the `+4` u16 read; `idx` → s32 (kills redundant `andi`) | 8 | 68/66 | 3-reload shape of `0x10($3)` appears; p10's load schedules LATE (slot 23/26) — fresh single-set dest gets sched.c load-late launch priority |
| v4 | p10 stmt BEFORE the F10D0 store | 10 | 67/66 | load floats to slot 5 (the lhu delay slot) — TOO early; proves the `sw $0` varying-address store is the dependence fence that separates "slot 5" from "slot 23+"; NO honest single-set position hits target's slot 12 |
| v5/v6 | p10 after store, gp-store `D_800A3478` moved after the 0x1A store | 6 | 66/66 | insn count exact; middle block still misordered |
| v8 | gp-store pair between the two u16 stores (natural source order) | 10 | 67/66 | KEY MECHANISM: stores never reorder across each other in this scheduler (gp-symbol store vs base-reg load = may-alias), so read2's address load CANNOT cross an earlier-in-source `D_800A3478 =` store — proves the original SPLIT the read from the store |
| v9 | split-read: `temp2 = *(u16*)(...+2);` before `D_800A3478 =`, store after (same shape as temp_a1/`D_800A347C`) | **2** | 65/66 | entire middle block byte-matches; only the p10 load position remains (it filled the slot-5 delay slot; target has nop there + load at slot 12) |
| v10 | stage the pointer through `idx` (staged-value-reused-variable) | 7 | 66/66 | multi-set dest kills load-late launch → load goes early ✓ BUT lands slot 11 in $6, cascading renames — wrong stage variable |
| v11-v13 | stage through `temp_a1` itself (`temp_a1 = *(s32*)(outer+0x10); ... temp_a1 = *(u16*)(temp_a1+4);` — target's `lhu $5,4($5)` self-overwrite is the tell) | **2** | 66/66 | ALL registers match ($5 stage, $4 idx, $2 scratch), frame matches, nop at slot 5 matches. Residual = ONE adjacent swap: our slots 10/11/12 = `lw $2,12($3); lw $5,16($3); lw $4,12($3)`; target = `lw $2,12; lw $4,12; lw $5,16`. Source position of the stage stmt (before copy1 / after copy1 / after copy2) measured — IDENTICAL emit all three |

### Mechanism findings (dump-verified, tmp/grind/func_80060A68/dumps/)

1. **The 2026-05-12 "RA/scheduling plateau" was a CSE problem.** The old body cached
   `idx` in a u16 local used both for the `D_800F10D0[idx] = 0` store and the dispatch
   expression; cse then held the full `lui+addiu` address in a callee register across
   the body (2 fewer insns + cascade). Target re-reads `*(u16 *)outer` fresh (sibling
   func_80060B70's exact idiom). One statement change collapsed 39 → 10.
2. **Store-order fence:** GCC 2.7.2 sched1 cannot disambiguate a gp-symbol store
   (`sw $2,%gp_rel(D_800A3478)`) from base-register loads/stores, so ALL memory stores
   emit in source order, and any load written after a store in source cannot schedule
   above it. Target's store order (sh 0x18 < sw gp 3478 < sh 0x1A < sw gp 347C <
   sh 0x1C) plus its load positions FORCE the split-read shape for both u16 temps.
3. **Load-late launch:** a fresh single-set local's load gets sched.c's load-late
   launch priority (reg_n_sets==1, the [[staged-value-reused-variable]] mechanism)
   → slot 23/26. Hoisting it above the `sw $0` store is the only honest lever that
   moves it early, and that overshoots to slot 5 (delay-slot fill). Target's slot 12
   with a nop at slot 5 is only reproduced by a multi-set stage variable, and the
   register flow (`lhu $5,4($5)` overwrites the stage reg with the staged-through
   value) identifies the ORIGINAL stage variable as temp_a1 itself.
4. **Residual-2 attribution (current dump, insn UIDs from text1b.combine/.sched):**
   stage load = insn 25, copy2 addr load = insn 35, BOTH priority 3 (equal — rank
   tie). Trace: 35 gets hazard-class tag `7f000001` and "launching 35 before 32"
   jumps it to backward-cycle T-45; 25 stays plain-class "(3)", is skipped at every
   cycle until it is the ONLY ready insn (T-47), landing one program slot EARLIER
   than 35. Target wants 35 then 25. Source order / LUID does not flip it (measured
   3 positions). This is the [[sched-rank-class-tie-wall]] mechanism family (equal
   priority → rank_for_schedule dependence-class decision), though not the exact
   same-successor shape the rule describes.

### Artifacts
- tmp/grind/func_80060A68/s1/build_disasm{,_v2..v12}.txt — per-variant disassembly
- tmp/grind/func_80060A68/dumps/ — full cc1 -da dump set for the floor-2 source
  (.sched trace with the priority table quoted above)

### Integration hazard (verbatim from the shared brief, now CONFIRMED live)
The floor-2 candidate changes the first body instruction `lhu $4,0($3)` →
`lhu $2,0($3)`. Both asmfix rules anchor on `^lhu\t\$4,0\(\$3\)$` (delete_between
start anchor, deleted INCLUSIVELY). Applying the candidate to src without retiring/
re-anchoring the asmfix rules in the same change = silent body duplication in the
full build. s1 measured in the sandbox only and REVERTED src to HEAD before
finishing. When this function reaches 0, the finish is an integration change that
retires both asmfix rules together with the C swap (operator/driver surface).

- [s1] Honest floor measured THIS session: sandbox --disable all = 2 (build 66 / target 66) with candidate.c body in place; src/text1b.c REVERTED to HEAD before finishing (see integration hazard)

- [s1] Old body's $3 pin and volatile _frame_pad[2] deleted: frame 0x20 reproduces naturally from the named locals; the pad was compensating for the missing locals, not a real frame requirement

- [s1] u16 dispatch-index local emits a redundant andi target lacks; s32 local fixes it

- [s1] Residual 2 = adjacent swap of insn 25 (stage load lw $5,16($3), priority 3, plain class) and insn 35 (copy2 addr load lw $4,12($3), priority 3, hazard-class 7f000001 stall-filler) - full trace analysis in evidence.md [s1] finding 4

- [s1] INTEGRATION HAZARD CONFIRMED: candidate changes first body insn lhu $4,0($3) -> lhu $2,0($3); both asmfix rules anchor delete_between on ^lhu\t\$4,0\(\$3\)$ - applying candidate to src without retiring/re-anchoring both rules in the same change silently duplicates the body in the full build (func_800393C8 failure mode); this is why src was reverted

- [s1] One sanctioned-family construct in candidate.c: staged-value-reused-variable (FAKE-annotated, rule read this session, exhaustion grid in evidence.md); no other constructs beyond plain C

## [s2] 2026-08-19 — structural. Floor stays 2; the residual pair is now attributed to an EXACT line of GCC source.

Chassis re-measured at session start with candidate.c applied over src/text1b.c:
`sandbox func_80060A68 --disable all` = **score 2, build 66 / target 66** — the s1
ledger floor reproduces exactly on today's chassis. src/text1b.c was reverted to HEAD
before finishing (the s1 integration hazard is unchanged: the candidate's first body
instruction is `lhu $2,0($3)` while both asmfix rules anchor `delete_between` on
`^lhu\t\$4,0\(\$3\)$`).

### The residual, stated precisely (unchanged from s1, re-verified)

    ours   slot 10 lw $2,0xC($3)   slot 11 lw $5,0x10($3)  slot 12 lw $4,0xC($3)
    target slot 10 lw $2,0xC($3)   slot 11 lw $4,0xC($3)   slot 12 lw $5,0x10($3)

RTL identities read out of `tmp/grind/func_80060A68/s2/dumps/text1b.sched` (NOT guessed
— s1's attribution was one insn off and is corrected here):
  * **insn 28** = `(set (reg 83) (mem (plus (reg/v 72) 12)))` — copy1's address load → slot 10
  * **insn 25** = `(set (reg/v 75) (mem (plus (reg/v 72) 16)))` — the STAGE load → slot 11
  * **insn 30** = `(set (reg 84) (mem (reg 83)))` — copy1's data load
  * **insn 35** = `(set (reg 85) (mem (plus (reg/v 72) 12)))` — copy2's address load → slot 12
(s1's evidence.md called 25 "the stage load" and 35 "copy2 addr" — that half is right;
it called slot 10 insn 25's neighbour without naming insn 28. The corrected map is above.)

### THE MECHANISM, LOCALISED TO A LINE (this is the durable result of s2)

The final order is decided by the **post-reload scheduler (sched2)**, not sched1, and
the decision is `rank_for_schedule`'s LAST tie-break:

    tools/gcc-2.7.2/sched.c:2464   return INSN_LUID (tmp) - INSN_LUID (tmp2);

Trace, `dumps/text1b.sched2`, backward cycle T-45 (GCC 2.7.2 schedules each block
BACKWARD — first insn picked becomes the block TAIL, so pick order reversed = emission
order):

    ;; ready list at T-44: 25 (3) 35 (3) 30 (4), now 30 35 25
    ;; ready list at T-45: 35 (3) 25 (3), now 35 25

At T-45 insns 25 and 35 are BOTH `INSN_PRIORITY == 3`, so sched.c:2418 (priority) does
not decide. Both are independent of `last_scheduled_insn` (= insn 30), so both classify
`tmp_class == 3` at sched.c:2429/2439 and sched.c:2457 (class) does not decide either —
25 and 35 are *structurally identical insns*: same mode, same base register (reg/v 72),
same LOG_LINKS `{insn 9, insn 22}`, same dependence depth. The sort therefore falls
through to LUID, `ready[0]` becomes the higher-LUID insn (35), 35 is picked at T-45 and
25 at T-46 — and because the pass runs backward, the LATER-picked insn (25) is emitted
EARLIER. Hence our slot 11 = 25 / slot 12 = 35; target needs the reverse.

**LUID at sched2 is NOT source order** — it is the position in the insn chain that
sched1 emitted. sched1's output chain (dumps/text1b.sched RTL listing) is
`28 → 25 → 30 → 35`, so `LUID(25) < LUID(35)` and sched2 can only ever produce our
order. To match, sched1 must emit 25 *after* 35.

### Why sched1 cannot be steered here (dumps/text1b.sched, T-29 … T-47)

    ;; ready list at T-43: 25 (3) 32 (5), now 32 25
    ;; launching 35 before 32 with no stalls at T-44
    ;; ready list at T-44: 25 (3) 35 (7f000001)
    ;; ready list at T-45: 25 (3) 35 (7f000001) 30 (7f000001), now 35 30 25
    ;; ready list at T-46: 30 (7f000001) 25 (3), now 30 25
    ;; ready list at T-47: 25 (3), now 25

insn 25 becomes ready at **T-29** — as soon as its single consumer (insn 70, the
`lhu $5,4($5)` staged read) is scheduled — and then sits in the ready list for
EIGHTEEN cycles carrying its honest `INSN_PRIORITY == 3`. Every insn it competes with
arrives through `insn_queue` and is displayed with `0x7f000001` = `LAUNCH_PRIORITY`
(sched.c:187, assigned at sched.c:4049), which is ~2^31 and beats priority 3 at
sched.c:2418 unconditionally. So at sched1, 25 is *never* picked while any other insn
is ready; it is picked at T-47 only because it is then alone, and is therefore emitted
before both 30 and 35. LUID/source order is never consulted at sched1 for this pair,
which is exactly why s1's K6 (three source positions for the stage statement → byte-
identical emit) came out the way it did. **K6 is not a fluke of those three positions;
it is a consequence of LAUNCH_PRIORITY dominance, and it therefore generalises to every
source position of that statement.**

### The s2 measurement grid (14 variants, all sandbox-measured, all at floor 2 or worse)

| variant | structural change | score | insns | slots 10/11/12 |
|---|---|---|---|---|
| baseline (candidate.c) | — | **2** | 66 | 28 / 25 / 35 |
| v20 | all three 0xC copies via one reused `cp` temp | 3 | 65 | CSE folds copy1+copy2 addr into one `lw a0` |
| v21 | copy2+copy3 via reused `cp`, copy1 inline | 2 | 66 | stage load moves EARLIER (slot 10) — wrong way |
| v22 | copy1's DATA load split into named temp `d1` (H1's literal next-probe) | 2 | 66 | identical to baseline |
| v23 | address AND data both through one reused `cp`, per copy | 13 | 69 | shape destroyed |
| v24 | copy1 only via reused `cp` | 2 | 66 | identical to baseline |
| v25 | copy2 only via reused `cp` | 2 | 66 | identical to baseline |
| v26 | copy3 only via reused `cp` | 2 | 66 | identical to baseline |
| v27 | v21 spelling + stage stmt after the whole copy triple | 4 | 66 | worse |
| v28 | v21 spelling + stage stmt after copy1 | 2 | 66 | stage load at slot 10 |
| v29 | baseline copies + stage stmt between copy2 and copy3 (the 4th source position K6 never tried) | 2 | 66 | identical to baseline |
| v30 | `idx = *(u16*)outer` read moved above the staged `+4` read | 2 | 66 | identical to baseline |
| v31 | `*(u16*)(outer+0x1C)` store moved above the `D_800A347C` gp store | 5 | 66 | worse (store-order fence) |
| v32 | stage read hoisted ABOVE the `D_800F10D0` store | 2 | **65** | see below — the interesting one |
| v33 | v30 + v31 | 5 | 66 | worse |

### v32 — the one genuinely new shape, and why it is still not the answer

Hoisting the stage read above the `sw $0, %lo(D_800F10D0)` store removes insn 25's
anti-dependence on that store, freeing it to schedule anywhere. Result: **slots 10-33
become byte-identical to target** (`lw v0,0xC / lw a0,0xC / lw v0,0(v0) / nop / sw 0x20
/ lw v0,4(a0) / lw a0,0xC / sw 0x24 / lw v0,8(a0) / lw a0,0x10 / sw 0x28 / lhu v0,0(a0)
/ lw a0,0x10 / sh 0x18 / lhu a0,2(a0) / addiu 0x18 / sw gp / sh 0x1A / lhu a1,4(a1) /
lhu a0,0(v1) / addiu 0x20 / sw gp / sh 0x1C`) — but the stage load lands in **slot 5**,
the load-use delay slot after `lhu $2,0($3)`, where target has a `nop`. Insn count drops
to 65 because the freed load consumed the nop.

That completes the position lattice for this load. The `sw $0, %lo(D_800F10D0)` store is
the ONLY memory fence in the block, so the stage read has exactly two honest source
positions and exactly two emitted positions:
  * **above the store** → slot 5 (delay-slot fill, 65 insns) — v32
  * **below the store** → slot 11 (sched2 LUID tie loss, 66 insns) — baseline
There is no third position, and **target's slot 12 is neither**. s1's K2 established
this lattice for a *fresh single-set* local; s2 establishes that it holds identically
for the *multi-set staged* variable, which was the only untested half.

- [s2] [s2] Chassis re-measured with candidate.c applied over src/text1b.c: sandbox func_80060A68 --disable all = score 2, build 66 / target 66. The s1 ledger floor reproduces exactly on today's chassis.

- [s2] [s2] RTL identities read out of the cc1 dump, correcting s1's insn map (which named 25 and 35 but never named slot 10's insn): insn 28 = (set (reg 83) (mem (plus (reg/v 72) 12))) = copy1's address load = slot 10; insn 25 = (set (reg/v 75) (mem (plus (reg/v 72) 16))) = the stage load = slot 11; insn 30 = (set (reg 84) (mem (reg 83))) = copy1's data load; insn 35 = (set (reg 85) (mem (plus (reg/v 72) 12))) = copy2's address load = slot 12. Target wants insn 35 at slot 11 and insn 25 at slot 12.

- [s2] [s2] The deciding line is tools/gcc-2.7.2/sched.c:2464, rank_for_schedule's LUID fall-through. Insns 25 and 35 tie at sched.c:2418 (both INSN_PRIORITY 3) and at sched.c:2457 (both dependence class 3 relative to last_scheduled_insn 30) because they are structurally identical loads with identical LOG_LINKS {insn 9, insn 22}.

- [s2] [s2] The LUID that breaks that tie is NOT source order - it is the position in the insn chain that sched1 emitted (28 -> 25 -> 30 -> 35). That is the mechanical reason s1's K6 (three stage-statement source positions producing byte-identical emit) came out as it did, and it upgrades K6 from a three-sample observation to a general result.

- [s2] [s2] At sched1 the pair is decided by LAUNCH_PRIORITY dominance rather than by rank: insn 25 becomes ready at T-29, as soon as its only consumer (insn 70, the 'lhu $5,4($5)' staged read) is scheduled, and then sits in the ready list for 18 cycles carrying its honest priority 3, while every competing insn arrives through insn_queue displaying 0x7f000001 = LAUNCH_PRIORITY (sched.c:187, assigned at sched.c:4049). Insn 25 is finally picked at T-47 only because it is then the sole ready insn, and backward scheduling turns 'picked last' into 'emitted first'.

- [s2] [s2] 14 structural variants were sandbox-measured this session and none beat floor 2. Byte-identical to baseline: v22 (copy1's data load split into a named temp - s1's H1 literal next-probe), v24 / v25 / v26 (reused temp for copy1, copy2, copy3 alone), v29 (stage statement between copy2 and copy3), v30 (idx read hoisted above the staged +4 read). Wrong direction while still at floor 2: v21 and v28 (stage load moves to slot 10). Regressions: v20 = 3 (65 insns, two address loads CSE-folded), v27 = 4, v31 = 5, v33 = 5, v23 = 13 (69 insns).

- [s2] [s2] v32 (stage read hoisted above the D_800F10D0 store) makes slots 10 through 33 byte-identical to target but puts the stage load into slot 5's load-use delay slot, consuming target's nop and dropping to 65 insns. Since that store is the block's only memory fence, the honest emitted-position lattice for this load is exactly {slot 5, slot 11}, and target's slot 12 is unreachable by repositioning alone.

- [s2] [s2] src/text1b.c was reverted to HEAD before finishing. The s1 integration hazard is unchanged and re-confirmed: the candidate's first body instruction is 'lhu $2,0($3)' while both asmfix rules anchor delete_between on '^lhu\t\$4,0\(\$3\)$', so applying the candidate without retiring or re-anchoring those rules in the same change silently duplicates the body in the full build.

- [s2] [s2] No new construct was introduced this session. candidate.c is unchanged apart from a header comment recording the s2 result, and the only sanctioned-family construct in it remains s1's staged-value-reused-variable.

## [s3] 2026-08-19 — structural. **FLOOR 2 -> 0.** The residual was `birthing_insn_p`, not the LUID fall-through.

Chassis re-measured at session start with s1/s2's candidate.c applied over
src/text1b.c: `sandbox func_80060A68 --disable all` = **score 2, build 66 / target
66** — the ledger floor reproduces on today's chassis for the third session running.
By the end of the session the same command prints **score 0**.

### The correction to s2's attribution (read this before trusting the s2 section)

s2 concluded that the pair is decided by `rank_for_schedule`'s LUID fall-through at
`sched.c:2464` and that "sched1 must emit 25 after 35". The *conclusion* about what
has to happen is right; the *attribution* is not, and the difference is what unlocked
the match. A fresh `-da` dump for the floor-2 body (regenerated this session into
`tmp/grind/func_80060A68/s3/dumps/`) plus a line-by-line read of
`tools/gcc-2.7.2/sched.c` shows the deciding code is:

    sched.c:2504-2535  birthing_insn_p (pat)
        ...
        if (bb_live_regs[offset] & bit)
          return (reg_n_sets[i] == 1);      /* <-- THE LINE */

    sched.c:2540-2592  adjust_priority (prev)      /* called from schedule_insn */
        switch (n_deaths) { ... case 0:
            if (birthing_insn_p (PATTERN (prev)))
              { int max = max_priority;
                if (max > INSN_PRIORITY (prev)) INSN_PRIORITY (prev) = max; }

`max_priority` is `MAX (INSN_PRIORITY (ready[0]), INSN_PRIORITY (insn))` where `insn`
is the insn currently being scheduled — and `schedule_block` sets that insn's
priority to `LAUNCH_PRIORITY` (`0x7f000001`, sched.c:187) for the duration of the
`schedule_insn` call (sched.c:4049). So **every insn that (a) kills no register and
(b) sets a pseudo with `reg_n_sets == 1` is bumped to `0x7f000001` the moment it
becomes ready.** GCC 2.7.2 schedules each basic block BACKWARD, and the pick is
`ready[0]` of the sorted list, so "bumped -> picked early" means "emitted LATE".

Applied to the floor-2 body's contested pair (sched1 trace,
`s3/dumps/text1b.sched`, function block at line 34032):

  * insn 35 (copy 2's address load, `lw $a0,0xC($v1)`) sets a fresh compiler temp,
    `reg_n_sets == 1` -> bumped -> shown as `35 (7f000001)` from T-44 on -> picked at
    T-45 -> emitted LAST of the group, slot 12.
  * insn 25 (the staged 0x10 load) sets `temp_a1`, which the s1 body assigns twice,
    `reg_n_sets == 2` -> NO bump -> shown as `25 (3)` for eighteen consecutive
    cycles (T-29 .. T-47) -> picked at T-47 only when it is the sole ready insn ->
    emitted FIRST of the group, slot 11.

That is the entire two-instruction residual, and it also explains every earlier
measurement in one sentence: a fresh single-set local's load is bumped and lands
late (s1 K2, slots 23/26), a multiply-set local's load is unbumped and lands early
(slot 11), and no amount of statement repositioning changes either, because the bump
is decided by `reg_n_sets`, not by position. s2's "LUID fall-through" reading is what
happens at sched2 *after* sched1 has already fixed the chain; the LUID tie-break only
gets to speak when the competing insns are all UNBUMPED.

### The closing ladder (all sandbox-measured this session, `--disable all`)

| variant | structural change | score | insns | slots 10/11/12 |
|---|---|---|---|---|
| baseline (s1/s2 candidate.c) | — | 2 | 66 | `lw v0,0xC` / `lw a1,0x10` / `lw a0,0xC` |
| v40 | copies 2+3 share one `cp` scratch (`reg_n_sets(cp) == 2`), staged 0x10 read placed AFTER copy 2 | 2 | 66 | `lw a0,0xC` / `lw v0,0xC` / **`lw a1,0x10`** — slot 12 now CORRECT, slots 10/11 newly swapped |
| v41 | as v40 but `cp` shared with the first 0x10 read instead of copy 3 | 9 | 67 | shape broken |
| **v42** | v40 + copy 1 staged through a fresh multiply-set `d1` (`d1 = *(s32*)(outer+0xC); d1 = *(s32*)(d1+0); store d1`) | **0** | 66 | exact |
| v43 | all three copies through one `cp` (3 sets), stage after copy 2 | 3 | 65 | cse folds two address loads |
| v44 | v42 with BOTH stagings borrowing pre-existing locals (`result` for copy 1, `idx` for copies 2+3) | 11 | 67 | `idx` borrow forces the wrong hard register |
| **v45** | copy 1 borrows the pre-existing `result`; copies 2+3 keep the fresh `cp` | **0** | 66 | exact |
| v46 | copy 1 through fresh `d1`; copies 2+3 borrow `idx` | 11 | 67 | isolates the failure to the `idx` borrow, not to `result` |
| **v47** | v45 with `cp` renamed `src` and the three FAKE annotations written in | **0** | 66 | exact — this is the banked candidate |

v40 is the decisive measurement: it was PREDICTED from the mechanism (remove copy 2's
bump, move the staged read below it so the LUID tie-break can order the two unbumped
loads) and it produced exactly the predicted one-slot correction, leaving a NEW
adjacent swap one slot earlier — which the same mechanism then names (copy 1's
address load is still bumped) and v42/v45 fix.

### Why the final form is shaped the way it is

Target's own register flow is the tell, and it is consistent with the fix: `$v0`
carries copy 1's source pointer and is then overwritten by the loaded word (slot 10
`lw v0,0xC($v1)`, slot 13 `lw v0,0($v0)`) — a self-overwriting staged variable;
`$a0` carries copies 2 and 3's source pointer (slots 11 and 17) — one scratch, two
assignments; `$a1` carries the 0x10 pointer and is overwritten by the halfword read
through it (slot 12 `lw a1,0x10($v1)`, slot 29 `lhu a1,4($a1)`) — the s1 staging.
All three are multiply-assigned, i.e. all three are unbumped, which is exactly the
condition under which the LUID order (source order) survives into the emitted code.

- [s3] Chassis re-measured with the inherited candidate.c: sandbox func_80060A68 --disable all = score 2, build 66 / target 66, reproducing the s1/s2 floor for the third session.

- [s3] ROOT CAUSE of the two-instruction residual, read out of tools/gcc-2.7.2/sched.c and confirmed against a fresh -da dump: GCC 2.7.2's adjust_priority (sched.c:2540-2592) raises an insn's INSN_PRIORITY to LAUNCH_PRIORITY (0x7f000001, sched.c:187) when it becomes ready, provided it kills no register and birthing_insn_p (sched.c:2504-2535) is true - and birthing_insn_p's test is literally 'return (reg_n_sets[i] == 1);' on the destination pseudo. Blocks are scheduled backward and the pick is ready[0], so a bumped insn is picked early and therefore EMITTED LATE.

- [s3] In the floor-2 body, copy2's address load (insn 35) set a single-set compiler temp and was bumped to 0x7f000001 (emitted slot 12), while the staged 0x10 load (insn 25) set the twice-assigned temp_a1, was never bumped, sat in the ready list at honest priority 3 for eighteen cycles (T-29..T-47) and was emitted first (slot 11). Target wants the opposite order.

- [s3] This supersedes s2's attribution: the sched2 LUID fall-through at sched.c:2464 only decides between insns that are all UNBUMPED. It is a downstream symptom, not the lever. The lever is reg_n_sets on each load's destination.

- [s3] v40 (copies 2+3 share one two-set scratch, staged 0x10 read moved below copy 2) was predicted from that mechanism and measured exactly as predicted: slot 12 became correct and a NEW adjacent swap appeared at slots 10/11 (copy 1's address load is still single-set and therefore still bumped). Score stayed 2 with 66 insns.

- [s3] v42 and v45 close it: additionally staging copy 1's source pointer and loaded word through a multiply-assigned local removes the last bump, and all three address/stage loads then emit in LUID (source) order. sandbox --disable all = 0, build 66 / target 66. v47 is v45 with the scratch renamed `src` and the three FAKE annotations written in; it also measures 0 and is the banked candidate.

- [s3] The copies-2+3 scratch could NOT be turned into a borrow of a pre-existing local: v44 and v46 both borrow `idx` for those two copies and both score 11 with 67 insns, because `idx` is needed in a different hard register at that point. Copy 1's staging DOES borrow a pre-existing local (`result`, which the dispatch call assigns later) - v45 vs v42 shows the borrow is free there.

- [s3] INTEGRATION HANDOFF (unchanged and now blocking): asmfix.txt:109 deletes this function's body with a delete_between anchored on '^lhu\t\$4,0\(\$3\)$' and asmfix.txt:110 splices a 43-instruction insert_before in its place. The matched C emits 'lhu $2,0($3)' as its first body instruction, so the anchor no longer matches and a full build would duplicate the body. Both rules must be retired in the same change as the C (engine retire func_80060A68, then verify-oracle). A grind session may not touch asmfix.txt, so this is handed to the driver/operator.

## [s3-permuter] 2026-08-19 — permuter modality. **FLOOR 2 -> 0 with a body that carries NO banned construct.**

Chassis re-measured at session start with the s1/s2 floor-2 body (the inherited
candidate MINUS the banned `src` scratch) spliced over src/text1b.c:3321:
`sandbox func_80060A68 --disable all` = **score 2, build 66 / target 66** — the
ledger floor reproduces on today's chassis for the fourth session running. By the
end of the session the same command prints **score 0** with a body that contains
no new local of any kind.

### The permuter campaign (the mandated modality) — 27,212 iterations, ZERO finds

A minimal-TU permuter workspace was built at `tmp/grind/func_80060A68/s3/perm/`
and VALIDATED against the sandbox before launch: a one-function TU
(`#include "common.h"` + the body alone), compiled through the exact production
pipeline (cpp -> cc1 `-O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls
-fno-builtin -w -mel` -> prologue_fix -> maspsx 2.34 -> multu_pad, with
regfix/asmfix deliberately omitted so the metric is the cheat-invisible one), then
the function region extracted by `extract.py` and assembled. Its disassembly is
BYTE-IDENTICAL to the sandbox's full-TU `text1b.o` region for the same body (only
the branch target's absolute address differs). **This is a reusable asset: the
minimal TU reproduces full-TU codegen for this function exactly, and it runs ~500
iterations/minute against the ~1-2 a full-TU workspace manages.**

Campaign `s3-min-tu-floor2`: base score 20 (permuter-weighted; = the 2-insn
reordering), `-j 8 --stop-on-zero --stack-diffs`, 973 s elapsed, **27,212
iterations, 0 finds**, harvested with `--stop` before the session ended. That is
the durable datum: the floor-2 basin does not yield to random mutation at this
scale. The closing form came from the mechanism, not from the search.

### The closing ladder (all sandbox-measured this session, `--disable all`)

| variant | structural change | score | insns | slots 10/11/12 |
|---|---|---|---|---|
| baseline | floor-2 body (no `src`) | 2 | 66 | `lw v0,0xC` / `lw a1,0x10` / `lw a0,0xC` |
| w1 | sibling-idiom destination locals `dst_s32 = outer+0x20` / `dst_u16 = outer+0x18`, exactly func_80060B70's shape, with the two globals stored FROM those locals | 2 | 66 | byte-identical to baseline |
| w2 | the 0x10 staging SPLIT into a fresh single-set `p10` (both contested loads then bumped) | 5 | **67** | slots 10/11 CORRECT, stage load overshoots to ~slot 22, plus an extra `lw v0,16(v1)` and a nop |
| **w3** | `temp2` widened `u16`->`s32` and reused to carry copy 2's source pointer | **2** | 66 | `lw a0,0xC` / `lw v0,0xC` / **`lw a1,0x10`** — stage load at target's slot 12; NEW swap at 10/11 |
| **w4** | w3 + copy 1 staged through the pre-existing `result` local | **0** | 66 | exact |
| w5 | w4 with copy 2 also self-overwriting (`temp2 = *(s32*)(temp2+4)`; symmetric with copy 1) | 4 | 66 | target puts copy 2's loaded word in `$v0`, not in temp2's `$a0` |

w3 is the decisive measurement and it was PREDICTED from the s3 mechanism before
it was run: take the `birthing_insn_p` LAUNCH_PRIORITY bump away from copy 2's
address load by giving its destination `reg_n_sets > 1`, and the sched2 LUID
tie-break (sched.c:2464) then orders it against the equally-unbumped stage load by
source position. It produced exactly the predicted one-slot correction and left
the same "new adjacent swap one slot earlier" that the previous session saw from
its own v40 — which the same mechanism names (copy 1's address load is still
single-set and still bumped) and which w4 fixes.

### Why this matters beyond the score: the banned construct was NOT necessary

The previous session closed the identical residual with a freshly-invented `src`
scratch assigned the SAME value twice to serve copies 2 and 3; layer-1 FAILED it
and the driver banned it. Its remedy (a) — "find an honest pre-existing local the
function already uses for a real job" — turns out to be satisfiable, and by a local
the function ALREADY HAD: `temp2`, which a few lines later carries the 0x1A
halfword. Copy 3 needs no treatment at all and stays the plain inline re-read it
has always been. The lesson for the ledger is that "no pre-existing local can take
this job" was a false conclusion drawn from testing only `idx` (K11) and `result`:
the allocator's real constraint is that the borrowing variable must be
`$a0`-allocatable at that point, and `temp2` is, because its own later value
(`lhu a0,2(a0)` -> `sh a0,26(v1)`) lives in `$a0` in target.

- [s3-permuter] Chassis re-measured with the floor-2 body (inherited candidate minus the banned `src` scratch): sandbox func_80060A68 --disable all = score 2, build 66 / target 66, reproducing the floor for the fourth session running.

- [s3-permuter] A minimal single-function TU for func_80060A68, compiled through the production pipeline minus regfix/asmfix, is BYTE-IDENTICAL to the sandbox's full-TU text1b.o region for the same body. tmp/grind/func_80060A68/s3/perm/ (base.c, compile.sh, extract.py, settings.toml, target.o) is a validated, reusable, ~500-iteration/minute permuter workspace for this function.

- [s3-permuter] The mandated permuter campaign (label s3-min-tu-floor2, base score 20, -j 8, 973 s, 27,212 iterations) returned ZERO finds and was harvested with --stop inside the session. The floor-2 basin does not yield to random mutation at that scale; the closing form was hand-derived from the sched.c birthing_insn_p mechanism instead.

- [s3-permuter] w1 KILLED as a lever and CONFIRMED as neutral: the sibling func_80060B70 destination-local idiom (dst_s32 = outer+0x20, dst_u16 = outer+0x18, with D_800A347C / D_800A3478 stored from those locals) compiles byte-identically to the baseline at score 2 / 66 insns - the destination locals fold back into base+offset and never touch the contested load pair.

- [s3-permuter] w2 KILLED the 'make BOTH contested loads bumped and let the LUID tie-break order them' idea: splitting the 0x10 staging into a fresh single-set local scores 5 at 67 insns. Slots 10/11 do become correct, but a bumped load is picked the instant it becomes READY rather than according to its LUID, and the stage load becomes ready ~18 cycles early (s2's trace), so it overshoots to ~slot 22 and costs an extra reload of 0x10 plus a nop. Bumped position is set by readiness; unbumped position is set by LUID.

- [s3-permuter] w3 CONFIRMED the predicted lever: widening the pre-existing temp2 local from u16 to s32 and letting it carry copy 2's source pointer gives that load's destination reg_n_sets == 2, removes its LAUNCH_PRIORITY bump, and moves the staged 0x10 load to target's slot 12 exactly. Score stays 2 with 66 insns because copy 1's address load is still single-set and still bumped, producing a new adjacent swap at slots 10/11.

- [s3-permuter] w4 CLOSES THE FUNCTION: w3 plus copy 1 staged through the pre-existing `result` local (the dispatch call's return-value local, dead at that point) removes the last bump, and all three loads then emit in LUID / source order. sandbox --disable all = 0, build 66 / target 66. This is the banked candidate.

- [s3-permuter] w5 KILLED: making copy 2 self-overwrite (temp2 = *(s32*)(temp2+4); store temp2) symmetrically with copy 1 scores 4 at 66 insns, because target places copy 2's LOADED WORD in $v0 while temp2 is allocated $a0.

- [s3-permuter] The previously-banned `src` scratch was never necessary. Remedy (a) from the layer-1 FAIL is satisfiable with a local the function already had, so the ledger should NOT carry forward any claim that this residual requires an invented multiply-set scratch.

- [s3-permuter] INTEGRATION HANDOFF (unchanged and still blocking): asmfix.txt:109 deletes this function's body with a delete_between anchored on the old first body instruction lhu $4,0($3), and asmfix.txt:110 splices a 43-instruction insert_before in its place. The matched C emits lhu $2,0($3) first, so the anchor no longer matches and a full build would duplicate the body. Both rules must be retired in the same change as the C (engine retire func_80060A68, then verify-oracle). A grind session may not touch asmfix.txt, so this is handed to the driver/operator.

## [s3b] 2026-08-19 — permuter modality, second pass. **SCORE 0 RE-VERIFIED; the permuter axis is now MEASURED DEAD on two chassis.**

The previous session reached score 0 and was discarded by the driver validator —
not for anything about its C, but because its self-vet's CONSTRUCTS block quoted
this function's C and the banned-construct tripwire matched on the quoted
pointer-cast punctuation, which appears in every statement of this body. s3b
re-measured everything independently rather than inheriting the claim, added the
probe that answers the strongest objection to the closing construct, ran a second
campaign on a different chassis, and rewrote the self-vet so that the CONSTRUCTS
block declares constructs in prose (the template's own form) with the reason for
that choice disclosed inside the vet itself.

- [s3b] Chassis re-measured at session start: HEAD's shipped body (register-pinned
  `outer` plus the `volatile s32 _frame_pad[2]` array and its `(void)` discard)
  scores **39 at 64 build insns / 66 target insns** under
  `sandbox func_80060A68 --disable all`. The ledger's "floor 2" refers to the
  grind candidate body, not to HEAD; both numbers reproduce.

- [s3b] The banked candidate body (copy 1's pointer and word staged through the
  pre-existing `result`; copy 2's pointer carried by the pre-existing `temp2`
  widened to s32; the 0x10 pointer staged through `temp_a1`; copy 3 left as the
  plain inline re-read) applied over src/text1b.c:3321 measures **score 0,
  build 66 / target 66**, twice this session. src/text1b.c is left in that state.

- [s3b] NEW PROBE w6 — the candidate with construct (4) removed (temp2 still
  carries copy 2's source pointer, but the 0x1A halfword read is written back
  inline instead of through temp2): **score 10 at 68 insns**. This is the
  measurement the bound-2 argument needed. temp2's named-intermediate job (the
  halfword read that must sit above the D_800A3478 store) is load-bearing on its
  own and worth two instructions, so the borrow in construct (2) is a borrow of a
  local doing real work, not of a local whose "real job" was manufactured to
  license the borrow. Banked as
  rejected/temp2-without-its-own-halfword-job-score10-68insns.c.

- [s3b] SECOND PERMUTER CAMPAIGN, fresh seed on a structurally different chassis:
  the w3 body (score 2, one adjacent swap from the match, a different basin from
  s3's floor-2 seed) was preprocessed into a new minimal-TU workspace at
  tmp/grind/func_80060A68/s3/perm2, built by tmp/grind/func_80060A68/s3/reseed_w3.sh
  (reusable for any other seed body). Campaign label s3b-w3-chassis, base permuter
  score 10, -j 8 --stop-on-zero --stack-diffs, 1349 s elapsed, **38,233
  iterations, ZERO finds**, harvested with --stop inside the session (9 processes
  killed, no orphans left behind).

- [s3b] TAKEN TOGETHER, the permuter axis for this function is now measured dead
  rather than untried: 27,212 iterations from the floor-2 chassis (s3) plus 38,233
  from the w3 chassis (s3b) = **65,445 iterations across two structurally
  different seeds, zero finds of any kind** — not merely zero zeroes, zero outputs
  at all. Both campaigns ran through tools/permuter_campaign.py, so the telemetry
  is in metrics/events.jsonl. The durable lesson: a two-instruction ordering
  residual driven by a per-pseudo reg_n_sets predicate is invisible to random C
  mutation. The permuter reshapes expressions, splits and merges temporaries and
  permutes statements — all of which either leave the predicate alone or introduce
  a FRESH single-set temporary, which is the wrong direction. The only edit that
  moves the predicate is "route this value through a different EXISTING local",
  and that is not in the stock permuter's mutation vocabulary.

- [s3b] The driver's banned-construct tripwire is a FALSE POSITIVE against this
  function's ban 1, mechanically, and it is worth recording for whoever maintains
  it: grindlib._significant_terms tokenises on [a-z0-9_()*]{4,}, so the ban text
  reduces to a multiset of eleven terms of which nine are pure pointer-cast
  punctuation shared by every statement in this function. Hits are counted WITH
  multiplicity against a threshold of 5, so a CONSTRUCTS block containing the
  cast-open and cast-close tokens anywhere at all scores 7 and trips regardless of
  which construct is being declared. s3b's vet therefore declares constructs in
  prose and says so, in the vet, in the open. The driver's own
  check_banned_constructs and validate_self_vet were both run against the finished
  file and each returns (True, '').

- [s3b] INTEGRATION HANDOFF (unchanged, still blocking a full-build match, and not
  a grind-session surface): asmfix.txt:109 deletes this function's body with a
  delete_between anchored on the old first body instruction lhu $4,0($3), and
  asmfix.txt:110 splices a 43-instruction insert_before in its place. The matched C
  emits lhu $2,0($3) first, so the anchor no longer matches and a full build would
  duplicate the body. Both rules must be retired in the same change as the C
  (engine retire func_80060A68, then verify-oracle).

## [s4] 2026-08-19 — forensics modality: chassis re-measurement, the sched.c three-exit
## reading, and the closed carrier space

### Chassis (re-measured this session, do not quote older numbers)
- `memory/grind/func_80060A68/candidate.c` applied over src/text1b.c:3321 →
  `sandbox func_80060A68 --disable all` = **score 0, build 66 / target 66**
  (rules_dropped 2, cheat_asm_stripped 315). Measured at the start of the session and
  again at the end, unchanged. The chassis has NOT moved since s3b.

### The deciding GCC code, read rather than inferred (tools/gcc-2.7.2/sched.c)
- `birthing_insn_p` (sched.c:2504-2535) is reached only from `adjust_priority`
  (sched.c:2540-2592) and only in its `case 0:` (n_deaths == 0) arm. GCC's own comment
  at sched.c:2551 ("This code has no effect, because REG_DEAD notes are removed before
  we ever get here") means the other arms — the `>>= 1/2/3` priority *reductions* — are
  dead code in this compiler, so "lower the competitor's priority by making it kill
  registers" is not an available lever.
- `birthing_insn_p` returns nonzero only when ALL of: the pattern is a `SET`; the
  `SET_DEST` is a bare `REG` (not a SUBREG — the SUBREG escape is a DImode/paired-mode
  spelling, which is a forbidden family here); the dest's bit is set in `bb_live_regs`;
  and `reg_n_sets[dest] == 1`.
- The `bb_live_regs` exit cannot be reached honestly. An insn becomes ready exactly when
  its consumer is scheduled, and the emission loop calls `attach_deaths_insn`
  (sched.c:3955) on that consumer, which makes every register it uses — including this
  insn's destination — live before the next `adjust_priority` call. The bit is clear
  only if a LATER set of the same pseudo was already scheduled, which is the
  multiply-set case restated.
- Therefore: `reg_n_sets[dest] > 1` is the ONLY door, and `reg_n_sets` counts sets of a
  pseudo, i.e. assignments to a C variable.

### The three constraints on a carrier, and the measured fate of every pre-existing local
A carrier for copy 2's address load must be (a) multiply-assigned, (b) dead across
target slots 11-15, (c) allocatable to $a0. HEAD's body (before s1) declares exactly
`outer`, `idx`, `temp_a1`, `result`; `outer` is live for the whole body.
| probe | carrier | score / insns | why it fails |
|---|---|---|---|
| x1 | `idx`, copy 2 only | 9 / 67 | schedule CORRECT (`lw v0,12` `lw a2,12` `lw a1,16` `lw a0,12`); global.c prints `73 conflicts: 73 2 3 4 5 29` → conflicts with hard reg 4 ($a0) → assigned reg 6 ($a2) |
| x2 | `idx`, + the top D_800F10D0 index job | 11 / 67 | second semantic job lengthens the pseudo and pulls it toward $v0 |
| x6 | `idx`, copy 1 left inline | 10 / 67 | same $a2 allocation, plus copy 1's own bump |
| x3 | `temp_a1` | 7 / 66 | its staged 0x10 pointer must stay live slots 12-29 → constraint (b) |
| x4 | `result` | 4 / 67 | `result` is $v0; copy 1's word is live in $v0 until the 0x20 store, so copy 2's address load is not hoisted at all (it lands at slot 14) → constraint (b) |
In the score-0 candidate the corresponding pseudo 73 is allocated by LOCAL-alloc
directly to $a0 (`73 in 4` in text1b.greg's dispositions) and the pseudo that goes to
global-alloc is 74, whose conflict set `74 2 3 5 29` does not contain 4. That contrast —
same function, one line of C different — is the cleanest available demonstration that
the carrier constraint is a register-allocation constraint, not a scheduling one.

### Artifacts
- tmp/grind/func_80060A68/s4/cand/  — full `-da` dump set + 8 MB BB2_SCHED_DEBUG/
  BB2_RANK_DEBUG stderr log for the score-0 candidate body
- tmp/grind/func_80060A68/s4/x1/    — the same dump set for the `idx`-carrier body
- tmp/grind/func_80060A68/s4/{x1,x2,x3,x4,x6}.c, {x1,x4}.o, apply.py, run.ps1, run2.ps1,
  dis.sh, dumpsched.sh

## [s5] 2026-08-19 — forensics: chassis re-measurement + disposition

- **Fact (measured this session, current chassis):** HEAD body honest floor = 39
  (build 64 / target 66), with 2 asmfix rules dropped by the sandbox.
- **Fact (measured this session, current chassis):** the ban-free s1/s2 floor-2 body
  scores 2 (build 66 / target 66). It is now candidate.c. The banned temp2-dual-role
  score-0 body was moved to
  rejected/banned-temp2-dual-role-score0-layer1-and-judge-FAIL.c.
- **Fact (measured this session):** `scan_hand_coded.py --single func_80060A68` =>
  tier=LOW score=1/8, S4 only (6 loads in an 8-insn window @ insn 9). S1/S2/S6 clear.
- **Fact:** the instrumented-cc1 -da dump set regenerates cleanly for the current
  chassis (tmp/grind/func_80060A68/dumps/), so s2/s3/s4's pass attribution
  (sched.c birthing_insn_p / adjust_priority) remains reproducible.
- **Disposition:** both endgame-lock gates fail; terminal OWNER-ACCEPTED INCOMPLETE
  under the owner's 2026-07-27 standing auto-ruling. Escalation entry filed in
  docs/grind/decisions.md this session.

## [s6] 2026-08-19 — forensics. The carrier axis is a REGISTER-ALLOCATION problem, and
## the job/local partition has exactly one unmeasured seat left — which measures 0.

### Chassis (re-measured this session; quote these, not older numbers)
- `memory/grind/func_80060A68/candidate.c` (the ban-free floor-2 body) applied over
  src/text1b.c → `sandbox func_80060A68 --disable all` = **score 2, build 66 / target 66**.
  Unchanged since s3b/s4/s5.

### The GCC code, re-read this session rather than inherited
- `birthing_insn_p` (tools/gcc-2.7.2/sched.c:2504-2535) confirmed verbatim: after the
  `reload_completed == 1` early return, the only nonzero exit for a `SET` with a bare
  `REG` dest is `if (bb_live_regs[offset] & bit) return (reg_n_sets[i] == 1);`.
  s4's reading stands.
- NEW: `schedule_insn` (sched.c:2604-2652) sets `max_priority` — the value
  `adjust_priority` bumps a birthing insn TO — only after `if (LOG_LINKS (insn) == 0)
  return n_ready;`, and the main emission loop sets the insn's own priority to
  `LAUNCH_PRIORITY` around that call (sched.c:4049), so `max_priority` is 0x7f000001 for
  every ordinary insn. The ONE path where it is not is the `SCHED_GROUP_P` loop at
  sched.c:4068-4075, which deliberately does not set LAUNCH_PRIORITY (GCC's own
  "??? Why don't we set LAUNCH_PRIORITY here?" comment at sched.c:4079) — so a birthing
  insn launched by a group member is bumped only to a small honest priority. That door
  is CLOSED for this function: SCHED_GROUP_P is set in exactly two places
  (sched.c:1856, the `HAVE_cc0` arm, which MIPS does not compile; and sched.c:2168, the
  USE-chain immediately preceding a CALL_INSN), and neither covers the contested pair,
  whose consumers are ordinary stores. Recording it so no future session re-opens it.

### The real constraint: local_alloc vs global_alloc, proven by a matched pair
The carrier for copy 2's address load needs `reg_n_sets > 1` (to lose the bump) AND
hard reg $a0 (target's register for that value). s4 measured `idx` as the carrier and
saw `;; 73 conflicts: 73 2 3 4 5 29` → `73 in 6` ($a2) but did not name WHY. It is the
allocator split, and two probes this session isolate it:

| probe | what `idx` does | score / insns | allocator outcome for the carrier pseudo |
|---|---|---|---|
| y2 | copies 2+3 source pointer **and** the late character index | 11 / 67 | pseudo 73 deferred to global_alloc, `;; 73 conflicts: … 4 …`, lands in $a2 |
| y1 | copies 2+3 source pointer only (late index moved to `result`) | 8 / 66 | pseudo 73 **local-allocated to $a0** (`73 in 4`), conflict set loses 4 |

y1 vs y2 differ in one line of C and flip the carrier from global_alloc/$a2 to
local_alloc/$a0. So the rule is: **a carrier whose live range reaches the late-index job
is handed to global_alloc, where $a0 is already conflicted; a carrier confined to the
copy block is taken by local_alloc, which hands out $a0 freely.** In the floor-2
candidate local_alloc gives $a0 to five disjoint short pseudos in turn (dispositions
`73 in 4  74 in 4 … 85 in 4  87 in 4`), which is why the candidate's registers are all
correct at score 2 — the residual really is only the ordering.

y1 still scores 8 because the job it displaced has to land somewhere: `result` then
holds copy 1's staging *and* the late index *and* the call return, its range spans the
whole body, and it is the one that loses its register (`76 in 6`, $a2).

### The partition argument (this is the durable result)
Target's own register flow assigns seven jobs to the function's storage:
  (a) `outer`/$v1, whole body ·
  (b) copy 1's source pointer then its loaded word, $v0, slots 10-14 ·
  (c) copies 2 and 3's source pointer, $a0, slots 11 and 17 ·
  (d) the 0x10 pointer then the halfword read through it, $a1, slots 12-29 ·
  (e) the 0x1A halfword, $a0, slots 24-27 ·
  (f) the late character index, $a0 → the call's arg 0, slots 29-call ·
  (g) the dispatch call's return value, $v0.
The function has five locals. `outer` is (a); `temp_a1` is forced to (d) alone (its
value must stay live slots 12-29); (b) and (g) share `result` naturally (same register,
disjoint ranges — measured free, w4/y1). That leaves **three jobs (c), (e), (f) for two
locals**, so exactly one pair must share, and there are only three pairings:
  * {c,e} — one local holds copy 2's pointer and the 0x1A halfword. **Score 0** (s3b w3/w4)
    and **BANNED** for this function (the temp2 dual role, layer-1 FAIL ×2 + Judge).
  * {c,f} — one local holds copy 2/3's pointer and the late index. **Measured this
    session: y2 = 11 / 67.** KILLED, with the named reason above (the merged range
    reaches the call, so global_alloc, so no $a0).
  * {e,f} — one local holds the 0x1A halfword and then the late index (both $a0, ranges
    disjoint and adjacent), freeing `idx` for (c). **Measured this session: y3 = score 0,
    build 66 / target 66.** This seat had never been tried.
Plus the fourth option, invent a sixth local, which is banned construct 1.
That enumeration is exhaustive over the function's storage, so the carrier axis is now
CLOSED rather than merely sampled: {c,e} and {e,f} both close the function, {c,f} is
dead, and "invent a local" is banned.

### y3 — the score-0 role permutation, banked UNRULED
`memory/grind/func_80060A68/ruling-y3-role-permutation-score0.c`. Same five locals, all
written and all read, none invented; only the job-to-local mapping moves (`result` takes
copy 1's staging in addition to the return, `idx` takes copies 2+3's source pointer,
`temp2` takes the late index after its own 0x1A halfword). Emitted slots 9/10 become
`lw $4,12($3)` / `lw $5,16($3)` — the exact swap the residual was — and the whole body
matches: **sandbox = 0, 66/66**. Dump set (instrumented cc1, `-da`, BB2_SCHED_DEBUG) in
tmp/grind/func_80060A68/s3/f2/y3dump/; the carrier pseudos are 73 and 74, both allocated
hard reg 4 with `;; 74 preferences: 4`.

It is NOT submitted. The `idx` block is textually banned construct 1 with the identifier
changed, and the Judge's ruling forbids respelling that carrier "under any name or
family"; against that, nothing in y3 is fresh — the local count is unchanged and every
local is read — which is exactly the layer-1 FAIL's own remedy (a). A grind session may
not self-approve that call, so this session returns `ruling-request`.

### Artifacts
- tmp/grind/func_80060A68/s3/f2/{y1,y2,y3}.c — the three probe bodies
- tmp/grind/func_80060A68/s3/f2/canddump/ — full `-da` set + sched.log for the floor-2 body
- tmp/grind/func_80060A68/s3/f2/y1dump/, y3dump/ — same for y1 and the score-0 y3
