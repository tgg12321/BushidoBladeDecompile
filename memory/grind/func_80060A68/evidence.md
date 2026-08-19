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
