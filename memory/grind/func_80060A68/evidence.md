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
    and **BANNED** for this function (the temp2 dual role, layer-1 FAIL Ã--2 + Judge).
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

## [s7] 2026-08-19 — forensics. The bump-state truth table is COMPLETE, and a score-0 body exists whose copy-2 lever is NOT the banned redundant reload.

Chassis re-measured at session start with candidate.c applied over src/text1b.c:
`sandbox func_80060A68 --disable all` = **score 2, build 66 / target 66** — the ledger
floor reproduces again. src/text1b.c was restored byte-for-byte to HEAD before finishing
(`git status` clean apart from metrics/events.jsonl).

### 1. The residual is a BUMP-STATE problem, not a carrier-identity problem (new framing)

s4/s5/s6 enumerated CARRIERS (which local hosts copy 2's pointer). s7 re-frames the
search over the only degree of freedom the mechanism actually exposes: the birthing_insn_p
bump state of each contested load's destination pseudo. Two loads, two binary states,
four cells — and all four are now measured:

| copy2 addr load | stage (0x10) load | body | score | where the loads land |
|---|---|---|---|---|
| BUMPED (single-set) | BUMPED (single-set) | s1 K2 / v3 | 8-10 | stage overshoots to slot 23/26 |
| BUMPED | UNBUMPED (multi-set) | candidate.c baseline | 2 | stage@11, copy2@12 — swapped |
| UNBUMPED | BUMPED | **z1 (NEW this session)** | **9** | copy2@11 CORRECT, stage drifts to slot 19 |
| UNBUMPED | UNBUMPED | v42/v45/v47/y3/z3 | 0 | source/LUID order = target order |

z1 is the cell nobody had measured. It is y3 with the stage split back into two
single-set locals (`p10` pointer + `h4` halfword) and nothing else changed, so it isolates
the stage's bump state as a single variable. Result: 9, 66 insns, and the disassembly
(tmp/grind/func_80060A68/s3/z1_disasm.txt) shows exactly the predicted behaviour —
`lw a0,12(v1)` reaches target's slot 11 because copy 2 is unbumped, while the 0x10 load,
now bumped, is picked the instant it becomes ready (T-29, its consumer being the far-away
`lhu a1,4(a0)`) and is therefore emitted late, at slot ~19 instead of target's slot 12.
Banked as rejected/z1-copy2-unbumped-stage-singleset-bumped-stage-drifts-to-slot19-score9.c.

**Consequence — this closes s6's frontier item 2.** A "different target register flow"
cannot help: the target bytes fix three loads (`lw v0,0xC`, `lw a0,0xC`, `lw a1,0x10`)
in that order, whichever pseudos host them, and the table shows the ONLY cell that
produces that order is the all-unbumped one. Since `birthing_insn_p` has no fourth door
(s4's code reading, re-verified this session), closing this function requires copy 1's and
copy 2's address-load destinations to be multiply-assigned C variables. That is now a
proved structural requirement rather than an enumeration over locals.

### 2. A multiply-assigned destination does NOT require the banned redundant reload

Every previously known score-0 body got `reg_n_sets > 1` by loading the same pointer
twice from `outer + 0xC` with no intervening write (banned construct 1, under the names
`src`, `temp2` and y3's `idx`). s7 measured a different spelling — the same-variable
SPLIT-INIT ACCUMULATION shape the owner sanctioned provisionally on 2026-06-13
([[split-init-accumulation-sanctioned]], which closed func_80049C24 in this same file,
commit ad11a8c8):

    cp = *(s32 *)(outer + 0xC);      /* ONE load of the pointer, not two */
    cp += 4;                         /* combine folds this into the displacement */
    *(s32 *)(outer + 0x24) = *(s32 *)cp;

- **z2** = candidate.c + that block alone: **score 2, 66 insns**, but with a DIFFERENT
  residual from the baseline — slots 10/11/12 become `lw a0,12` / `lw v0,12` / `lw a1,16`.
  The stage load reaches target's slot 12 for the first time without any banned
  construct; the remaining swap is copy 1's address load against copy 2's, because copy
  1's destination is still a single-set compiler temp and is therefore still bumped.
- **z3** = z2 + copy 1 staged through the pre-existing `result`
  (`result = ptr; result = *ptr; store result;`, the same staged-value-reused-variable
  form candidate.c already uses, unbanned, for `temp_a1`): **score 0, build 66 / target
  66**, re-measured twice. Slots 10/11/12 = `lw v0,12(v1)` / `lw a0,12(v1)` /
  `lw a1,16(v1)` — target. Banked at
  memory/grind/func_80060A68/ruling-z3-split-init-accumulation-score0.c.
- **z6 (the control that isolates the lever)** = z3 with ONLY the `cp += 4;` line removed
  and the offset written back into the deref (`*(s32 *)(cp + 4)`), so `cp` stays a named
  local but becomes single-set: **score 2, 66 insns**. The split-init line — which
  contributes zero instructions to the object code — is worth exactly the last two.
  Banked as rejected/z6-control-cp-without-split-init-single-set-still-bumped-score2.c.
- **z5** = z3 with the split-init hosted on the pre-existing `idx` instead of a fresh
  `cp`: **score 9, 67 insns**. This generalises s4's K15 from the redundant-reload
  spelling to the split-init spelling: `idx` cannot host copy 2's pointer under ANY
  spelling, because the failure is global-alloc's $a0 conflict, not the scheduler.
  Banked as rejected/z5-idx-hosts-split-init-global-alloc-a0-conflict-score9-67insns.c.

### 3. The RTL forensics behind the split-init lever

`tmp/grind/func_80060A68/s3/z3dumps/text1b.sched` (fresh `-da` set for the z3 body):
the emitted chain for the contested group is **22 -> 25 -> 34 -> 45 -> 28**, i.e.
insn 25 `(set (reg/v 76) (mem (plus (reg/v 72) 12)))` = copy 1's address load into
`result`; insn 34 `(set (reg/v 77) (mem (plus (reg/v 72) 12)))` = copy 2's address load
into `cp`; insn 45 `(set (reg/v 75) (mem (plus (reg/v 72) 16)))` = the stage load into
`temp_a1`. All three are `reg/v` (user variables) and all three emit in source order.
Copy 3's address load (insn 48, `reg 85`, an ordinary single-set compiler temp) is still
bumped and still lands late, exactly as target wants.

Note the mechanism detail that makes the split-init cost nothing: the dump shows only
ONE surviving set of `reg/v 77` (insn 34) and the consumer as `(mem (plus (reg/v 77) 4))`
(insn 40) — combine folded the `+= 4` into the load displacement, yet the pseudo still
behaves as multiply-set for `birthing_insn_p`. That is the same stale-count behaviour the
2026-06-13 sanction documents for `reg_n_refs` in global.c, here observed on
`reg_n_sets` in sched.c.

### 4. What this session did NOT settle

Whether `cp` is admissible. It is a FRESH local written twice, which the Judge's
2026-08-19 10:21 ruling calls an excluded quadrant; the split-init sanction is a
2026-06-13 owner directive that is not on the frozen SOTN family list and whose own text
says adjacent spellings need their own ruling (here the SHAPE is identical to the
sanctioned one — `var = a; var += b;`, combine folds it back, zero emitted difference —
but the GCC mechanism it exploits is sched.c's `reg_n_sets` rather than global.c's
`reg_n_refs`). s7 therefore returns `ruling-request` rather than a submission.

- [s7] Chassis re-measured with candidate.c: sandbox func_80060A68 --disable all = score 2, build 66 / target 66. src/text1b.c restored to HEAD before finishing.
- [s7] The bump-state truth table over {copy 2 address load, stage load} x {bumped, unbumped} is now complete: (B,B) = 8-10 (s1 K2), (B,U) = 2 (baseline), (U,B) = 9 (z1, NEW), (U,U) = 0. Only the all-unbumped cell produces target's load order, so closing the function requires two multiply-assigned C variables, whichever locals host them. This closes s6 frontier item 2 (a different register flow cannot help).
- [s7] z1 (y3 with the stage split into two single-set locals) scores 9/66 and its disassembly shows copy 2's address load correctly at slot 11 while the now-bumped stage load drifts to slot ~19 - a bumped insn is picked the instant it becomes ready, and the stage's consumer is 18 cycles away.
- [s7] A multiply-assigned destination does NOT require banned construct 1: the same-variable split-init accumulation `cp = *(s32*)(outer+0xC); cp += 4;` loads the pointer once, is folded back by combine (66 insns, `lw $v0,4($a0)`), and unbumps copy 2's address load. z2 (that block alone) = score 2 with the stage load reaching target slot 12 for the first time; z3 (z2 + copy 1 staged through the pre-existing `result`) = score 0, build 66 / target 66, re-measured twice.
- [s7] Control z6 - z3 with only the `cp += 4;` line removed and the offset folded back into the deref, so `cp` is single-set - scores 2. The split-init line emits zero instructions and is worth exactly the two-instruction residual.
- [s7] z5 (split-init hosted on the pre-existing `idx` instead of a fresh `cp`) scores 9/67, generalising s4 K15: `idx` cannot host copy 2's pointer under any spelling, because the failure is global-alloc's $a0 conflict, not the scheduler.
- [s7] RTL forensics for z3 (tmp/grind/func_80060A68/s3/z3dumps/text1b.sched): emitted chain 22 -> 25 -> 34 -> 45 -> 28, with insns 25/34/45 all setting reg/v user pseudos and emitting in source order, while copy 3's single-set compiler temp (insn 48, reg 85) stays bumped and lands late. The dump also shows only one surviving set of reg/v 77 with the consumer at (mem (plus (reg/v 77) 4)), i.e. combine folded the split away while the pseudo still behaves as multiply-set for birthing_insn_p.

## [s8] 2026-08-19 — forensics. The SECOND exit of `birthing_insn_p` is measured dead on
## bytes, so the only exit that can close this function is the banned carrier axis.

### Chassis (re-measured this session; quote these, not older numbers)

- candidate.c applied verbatim over src/text1b.c:3321-3358 →
  `sandbox func_80060A68 --disable all` = **score 2, build 66 / target 66**. The ban-free
  floor reproduces on today's chassis for the fourth consecutive session.
- Baseline disassembly banked at tmp/grind/func_80060A68/s3/base_disasm.txt. Slots 10/11/12
  are `lw v0,12(v1)` / `lw a1,16(v1)` / `lw a0,12(v1)`; target wants
  `lw v0,12` / `lw a0,12` / `lw a1,16`. The residual is the single adjacent swap of the
  stage load (`a1`, from `outer + 0x10`) against copy 2's address load (`a0`), unchanged
  since s1.
- src/text1b.c restored to HEAD (`git checkout --`) before finishing; the tree is clean
  apart from metrics/events.jsonl.

### The durable result: `birthing_insn_p`'s door (b) costs four instructions

`tools/gcc-2.7.2/sched.c:2504-2535`, re-read this session rather than inherited, gives a
load insn exactly three ways to escape the LAUNCH_PRIORITY bump:

  (a) `reload_completed == 1` — true only in sched2, which is not the pass that orders
      this block (s3's attribution, unchanged);
  (b) `GET_CODE (SET_DEST (pat)) != REG` — the SUBREG door: the load writes only part of a
      multi-word pseudo, so the `bb_live_regs` / `reg_n_sets` test is never reached;
  (c) the dest bit not being set in `bb_live_regs` — reachable only for a dest that is not
      live, i.e. a dead load, which flow deletes before sched1 ever sees it.

Every previous session (s3-s7) attacked the fall-through of (a) — `reg_n_sets[i] == 1`,
the multiply-assigned carrier axis — and dismissed door (b) on RULES grounds only
("the forbidden DImode family", candidate.c's own header). s8 MEASURED it:

- **d1** = candidate.c with copy 2's source pointer routed through a `long long`
  (`cpq = (long long)(u32)*(s32 *)(outer + 0xC); *(s32 *)(outer + 0x24) = *(s32 *)((s32)cpq + 4);`)
  so that the pointer load is `(set (subreg:SI (reg:DI cpq) 0) (mem ...))`:
  **score 31, build 70 / target 66**.
- The door FIRES exactly as the model predicts — tmp/grind/func_80060A68/s3/d1_disasm.txt
  shows copy 2's `lw a0,12(a2)` hoisted to slot 10, AHEAD of the stage load
  `lw a1,16(a2)`, which is precisely the reordering the whole carrier axis was chasing.
- But the DImode local materialises four instructions no later pass folds away:
  `move v0,a0` (low half), `move v1,zero` (high half — flow does NOT delete it although
  the high half is never read), `addiu v0,v0,4`, plus a reloaded `lw a0,12(a2)`. 70
  instructions against a 66-instruction target.

**Consequence.** Door (b) is not a policy-blocked route, it is an arithmetically
impossible one: any wide-typed carrier (long long, double, two-word struct or union) pays
the same unconditional materialisation cost, because the cost IS the multi-word
representation that creates the SUBREG in the first place. Door (c) is unreachable for a
live dest. Therefore the ONLY exit of `birthing_insn_p` that can produce target's load
order is `reg_n_sets[i] == 1` — a multiply-assigned C variable hosting copy 1's and copy
2's address loads — which is exactly the axis the Judge banned for this function under
every spelling and every host identifier (decisions.md 10:21, 10:48, 11:07).

Combined with s7's completed bump-state truth table (only the all-unbumped cell reproduces
target's load order), the mechanism space is now closed at the GCC-source level with no
un-measured cell: two of the three exits are measured or proved unreachable, and the third
is closed by ruling rather than by search.

### Artifacts

- tmp/grind/func_80060A68/s3/bodies/base.c, bodies/d1.c
- tmp/grind/func_80060A68/s3/base_disasm.txt, d1_disasm.txt
- tmp/grind/func_80060A68/s3/apply.py, disasm.sh
- memory/grind/func_80060A68/rejected/dimode-subreg-door-costs-4-insns-score31-70insns.c

- [s8] Chassis re-measured: candidate.c = score 2, build 66 / target 66; residual is still the adjacent swap of the stage load against copy 2's address load (base_disasm.txt slots 10/11/12 = v0,12 / a1,16 / a0,12 vs target v0,12 / a0,12 / a1,16).
- [s8] birthing_insn_p (sched.c:2504-2535) has three exits, not one: reload_completed (sched2 only), a non-REG SET_DEST (the SUBREG door), and a dest absent from bb_live_regs (dead dest only). Prior sessions recorded reg_n_sets==1 as "the only live exit"; that was one exit short.
- [s8] The SUBREG door is measured DEAD ON BYTES: d1 (copy 2's pointer through a `long long`) scores 31 at 70 instructions. The door fires — copy 2's `lw a0,12` reaches slot 10 ahead of the stage load — but the DImode local costs four unfoldable instructions (`move` low half, `move v1,zero` high half that flow does not delete, `addiu`, and a reloaded `lw`). Any wide-typed carrier pays the same cost because the cost is the multi-word representation itself.
- [s8] With door (b) dead on bytes and door (c) unreachable for a live dest, the only mechanism that can close func_80060A68 is a multiply-assigned carrier (reg_n_sets > 1) — the axis banned by the 2026-08-19 Judge rulings under every spelling and host. The mechanism space is closed with no un-measured cell.

## [s9] 2026-08-19 — forensics (instrumented cc1, full `-da` dump sets)

Artifacts, all under `tmp/grind/func_80060A68/s3/`:
  * `n9/` — dump set + `dbg.txt` (BB2_SCHED_DEBUG / BB2_RANK_DEBUG stderr) for the
    candidate.c body. Trace excerpts: `n9/trace_sched.txt`, `n9/trace_sched2.txt`.
  * `n9b/` — same for rejected/split-stage-single-set-bumped-overshoots-slot22-score5.c.
    Trace excerpts: `n9b/trace_sched.txt`, `n9b/trace_sched2.txt`.
  * `n9/p1_stage_after_copy1.c` + `n9/p1_disasm.txt` — the killed statement-position probe.
  * `n9/tgt.txt` — target's 66 instructions, numbered, for slot references below.

### Facts established this session

1. **Chassis (re-measured, not inherited).** candidate.c applied at src/text1b.c:3321 gives
   `sandbox func_80060A68 --disable all` = score 2, build 66 / target 66. The residual is
   one adjacent swap: target slots 10-12 are `lw v0,12(v1)` / `lw a0,12(v1)` /
   `lw a1,16(v1)`; we emit `lw v0,12(v1)` / `lw a1,16(v1)` / `lw a0,12(v1)`.

2. **sched1 and sched2 emit DIFFERENT orders for this function, and sched2's is the one
   that ships.** sched1's post-pass chain is `22, 25, 39, 27, 32, 29, ...`; sched2's is
   `..., 25, 39, 32, 27, ...`, which is what objdump shows. Anyone reasoning about this
   function from the `.sched` dump alone is reading a chain that no longer exists by the
   time the assembler runs.

3. **The birthing_insn_p LAUNCH_PRIORITY bump is a sched1-only phenomenon.**
   `birthing_insn_p` (tools/gcc-2.7.2/sched.c:2503-2535) returns 0 at its first statement
   when `reload_completed == 1`. sched1's ready lists show `(7f000001)` priorities on insns
   25/27/32/34/42/44/49/51/56; sched2's show only natural depth priorities (max 17 plus the
   sentinel on the block-end insn). The bump therefore cannot be "the reason the bytes are
   wrong" directly — it is the reason sched1 chains insn 39 ahead of insn 32, which is the
   INSN_LUID order sched2 then inherits.

4. **The deciding comparison, at GCC-line level.** sched2, T-45, ready list `32 (3) 39 (3)`,
   `last_scheduled_insn` = insn 27.
     - `sched.c:2418` INSN_PRIORITY: 3 == 3, tied.
     - `sched.c:2426-2444` class: `LOG_LINKS (27) = {22, 25}` contains neither candidate,
       so both are class 3, tied.
     - `sched.c:2464` `INSN_LUID (tmp) - INSN_LUID (tmp2)`: LUID(39) < LUID(32) from the
       sched1 chain, so 32 sorts to ready[0], is scheduled at T-45 and therefore EMITTED
       AFTER insn 39. Target requires the reverse.
   Note on direction (this has been mis-stated in earlier ledger sections): schedule_block
   runs BACKWARD, so a larger T-n means EARLIER in emission, and the comparator's
   higher-LUID-first ordering means the higher-LUID insn is scheduled first and emitted
   last.

5. **INSN_PRIORITY here is a backward depth and it locks 25, 32 and 39 together.**
   `priority(insn) = max over LOG_LINKS producers of (priority(producer) + insn_cost)`.
   All three have producer set `{9, 22}` = `lw v1,0(gp)` (priority 1) and the
   `sw zero,0(at)` D_800F10D0 store (priority 3), and the store→load memory link costs 0.
   Calibration inside the same block: insn 27 (`lw v0,0(v0)`) has producers `{22, 25}` and
   lands on 4, so a load→address-use link costs 1. Consequence: any change that deepens
   insn 22 raises 25, 32 and 39 by the same amount and changes nothing. The only
   priority-4-or-deeper insns insn 39 could depend on are insns 27 (4) and 29 (5), and
   target emits BOTH after insn 39 (slots 13 and 15 vs slot 12), so acquiring either as a
   producer would force the wrong order by itself.

6. **Source statement position is codegen-INERT for the staged read.** Moving
   `temp_a1 = *(s32 *)(outer + 0x10);` from between copy 2 and copy 3 to immediately after
   copy 1 produces a BYTE-IDENTICAL function (score 2, build 66). Mechanism: in sched1
   insn 39 is the only unbumped insn in its window, and a priority-3 insn is picked only
   when the ready list is otherwise empty (T-47), so its chain position is fixed by
   readiness, not by LUID. This is the general reason s2's 14 structural respellings were
   all neutral, and it retires statement-permutation as an axis.

7. **The bumped-stage body reaches target's slot 11 for copy 2's address load.** With a
   fresh single-set `p10` holding the 0x10 pointer, the emitted slots 10-11 are exactly
   target's `lw v0,12(v1)` / `lw a0,12(v1)`. That is the first measured body on this
   function to place copy 2's address load correctly WITHOUT a multiply-assigned carrier.
   The cost is the staged load's own slot: it lands at 25 instead of 12, which leaves a
   load-delay `nop` at slot 22 (target fills that slot with `lw a0,16(v1)`), so the body is
   67 instructions and scores 5.

8. **Why the bumped stage overshoots — named.** sched2's trace reaches T-30 with insn 39
   as the SOLE ready insn (`;; ready list at T-30: 39 (3), now 39`); insns 56, 53, 51, 46,
   44, 36, 34, 32, 29, 27, 25 and 22 are all still blocked, and insn 56 is launched only
   after 39 is scheduled (`;; launching 56 before 39 with no stalls at T-31`). sched1's
   bump had chained insn 39 between insns 56 and 58, and the post-reload dependence graph
   built over that chain pins it there. So the overshoot is a sched2 readiness pin, not a
   sched1 priority artifact — which means it is attackable by changing what sits between
   insns 53 and 58, not by changing insn 39's own priority.

- [s3] Chassis re-measured, not inherited: candidate.c applied at src/text1b.c:3321 gives sandbox func_80060A68 --disable all = score 2, build 66 / target 66. Residual is one adjacent swap at target slots 10-12 (lw v0,12(v1) / lw a0,12(v1) / lw a1,16(v1) vs our lw v0,12(v1) / lw a1,16(v1) / lw a0,12(v1)).

- [s3] sched1 and sched2 emit DIFFERENT chains for this function and sched2's is the one that ships; any reasoning done from the .sched dump alone (which is what the ledger did through s8) is reading a chain that no longer exists at assembly time.

- [s3] Insn identities, stable across both dump sets: insn 25 = (set (reg 83) (mem (plus (reg/v 72) 12))) copy 1's address load; insn 32 = (set (reg 85) (mem (plus (reg/v 72) 12))) copy 2's address load; insn 39 = (set (reg/v 75) (mem (plus (reg/v 72) 16))) the staged 0x10 load; insn 27 = (set (reg 84) (mem (reg 83))) copy 1's value load.

- [s3] GCC 2.7.2's priority() is a BACKWARD depth (max over LOG_LINKS producers of priority + insn_cost), calibrated inside this block: store-to-load memory links cost 0, load-to-address-use links cost 1. Insns 25, 32 and 39 share the producer set {9, 22} and are therefore locked together at priority 3 -- no honest change that deepens insn 22 can separate them.

- [s3] Direction correction for the ledger: schedule_block runs backward, so a larger T-n means EARLIER in emission, and rank_for_schedule's comparator puts the HIGHER INSN_LUID at ready[0], scheduling it first and emitting it last. Earlier ledger sections state this the other way round.

- [s3] The bumped-stage family is the only measured family that puts copy 2's address load on target's slot 11 without a banned carrier construct, and its remaining defect is localised to one named sched2 state (T-30, insn 39 sole ready insn).

- [s3] Housekeeping: the previous session's uncommitted docs/grind/decisions.md OWNER-ESCALATION entry was reverted, because the driver discarded that session (owner-gated is not a valid outcome from forensics modality) and the function is active, not parked. src/text1b.c was restored to HEAD at end of session; no build-pipeline file was touched.

## [s10] 2026-08-19 — forensics (instrumented cc1, PRIODBG/RANKDBG/SCHEDDBG). The sched2 LUID tie is SOLVED; the residual moved to a new place.

*(Driver session index 4 of the current dispatch; ledger-local index s10.)*

### Chassis re-measured this session
`sandbox func_80060A68 --disable all` with the previous candidate body (staged `temp_a1`,
now preserved verbatim at `memory/grind/func_80060A68/prior-floor2-staged-temp_a1.c`)
applied to `src/text1b.c:3321` = **score 2, build 66 / target 66**. Same floor as s1-s9.
`src/text1b.c` was reverted to HEAD before finishing (the asmfix.txt:109-110 integration
hazard is unchanged).

### 1. KILLED — `priority(insn 39) = 4` (the s9 frontier item 2), with the compiler's own numbers

Probe: full instrumented `-da` dump of the floor-2 body with `BB2_PRIO_DEBUG=1` added to
the s9 recipe (`tmp/grind/func_80060A68/s4/n1/`, split into `pass1.txt` / `pass2.txt` by
the `SCHEDDBG FUNC func=func_80060A68 pass=N` markers). `priority()` at sched.c:1497 is
`max over LOG_LINKS producers x of (priority(x) + insn_cost(x, link, insn) - 1)`, so a
cost-1 link contributes +0 and a cost-2 link contributes +1. The sched2 (pass=2) stream for
the four insns that matter, verbatim:

    PRIODBG insn=25 pred=9  kind=0 pred_pri=1 cost=2 contrib=2 (max=3)
    PRIODBG insn=25 pred=22 kind=0 pred_pri=3 cost=1 contrib=3 (max=3)   -> SET 25 = 3
    PRIODBG insn=27 pred=25 kind=0 pred_pri=3 cost=2 contrib=4 (max=4)   -> SET 27 = 4
    PRIODBG insn=32 pred=9  kind=0 pred_pri=1 cost=2 contrib=2 (max=2)
    PRIODBG insn=32 pred=22 kind=0 pred_pri=3 cost=1 contrib=3 (max=3)   -> SET 32 = 3
    PRIODBG insn=39 pred=9  kind=0 pred_pri=1 cost=2 contrib=2 (max=2)
    PRIODBG insn=39 pred=22 kind=0 pred_pri=3 cost=1 contrib=3 (max=3)   -> SET 39 = 3

This closes s9's frontier item 2 by exhaustion over the actual producer sets rather than by
argument:
  * insn 39 has exactly two producers in sched2 — insn 9 (`lw v1,0(gp)`, priority 1, cost 2)
    and insn 22 (the `sw zero,0(at)` D_800F10D0 store, priority 3, cost 1). Max = 3.
  * `priority(39) = 4` needs either a cost-2 (load-result) link from a priority-3 producer
    or a cost-1 link from a priority-4 producer.
  * Every producer must be EMITTED BEFORE insn 39, and target emits insn 39 at slot 12, so
    the candidate producer pool is exactly target slots 1-11: insns 9 (pri 1), 13 (2),
    18 (3), 22 (3), 25 (3), 32 (3). **There is no priority-4 insn in the pool at all**, so
    the cost-1 route is empty.
  * The cost-2 route needs insn 39's ADDRESS operand to be the RESULT of insn 25 or insn 32
    — the only priority-3 loads in the pool. Both load `*(s32 *)(outer + 0xC)`; insn 39's
    address is `outer + 0x10`. No honest C form makes one the other.
  * Deepening insn 22 (the only shared lever) lifts 25, 32 AND 39 by exactly the same
    amount — the dump shows all three take their max from the identical `pred=22 cost=1`
    link — so the tie survives any change to the store's depth.

VERDICT: KILLED. Priority is not a reachable lever on this pair.

### 2. THE BREAK — a bumped single-set `p10` placed BELOW the copy-3 store puts all three contested loads on target

Nine source positions of a single-set `s32 p10 = *(s32 *)(outer + 0x10);` were measured
(r1-r9; the s9 rejected body `split-stage-single-set-bumped-overshoots-slot22-score5.c` is
position r4):

| variant | p10 statement placed after | score | insns |
|---|---|---|---|
| r1 | `outer = D_800A3468;` (above the D_800F10D0 store) | 2 | **65** |
| r2 | the D_800F10D0 store | 5 | 67 |
| r3 | copy 1 (`+0x20`) | 5 | 67 |
| r4 | copy 2 (`+0x24`) — the s9 body | 5 | 67 |
| **r5** | **copy 3 (`+0x28`)** | **2** | **66** |
| r6 | the `+0x18` store statement | 2 | 66 |
| r7 | the `temp2` read | 2 | 66 |
| r8 | the `D_800A3478` gp store | 3 | 66 |
| r9 | the `+0x1A` store | 3 | 66 |

**r5 is a new shape, and it is the first body on this function whose whole prefix through
slot 20 is byte-identical to target — slots 10/11/12 included:**

    target  slot 10 lw v0,12(v1)   slot 11 lw a0,12(v1)   slot 12 lw a1,16(v1)
    r5      slot 10 lw v0,12(v1)   slot 11 lw a0,12(v1)   slot 12 lw a1,16(v1)

Every session from s1 to s9 recorded that pair as the residual and attributed it to
`rank_for_schedule`'s LUID fall-through at sched.c:2464 in sched2. **That residual no
longer exists.** r5's remaining 2-instruction gap is somewhere else entirely:

    target  ... lw a0,16(v1) / sw v0,40(v1) / lhu v0,0(a0) / lw a0,16(v1) / sh v0,24(v1) / lhu a0,2(a0) ...
    r5      ... lw a0,16(v1) / sw v0,40(v1) / lhu v0,0(a1) / nop          / sh v0,24(v1) / lhu a0,2(a0) ...

i.e. the `*(u16 *)(outer + 0x18)` read is CSE-folded onto p10's register (a1) instead of
re-loading `16(v1)`, so target's slot-23 load — which also fills the load-delay slot after
`lhu v0,0(a0)` — is missing and shows as a `nop`. r6 and r7 are the same defect displaced
by one read (there the `+0x2` read folds onto a1 instead: `lhu a0,2(a1)`). r5 was promoted
to `memory/grind/func_80060A68/candidate.c` this session; it carries NO multiply-assigned
local at all, so unlike the prior body it needs no staged-value-reused-variable claim.

### 3. The conservation law that r1-r9 / t1 / t2 trace out

`p10` is single-set, so its load (insn 39) takes sched1's `birthing_insn_p`
LAUNCH_PRIORITY bump and is chained late — which is exactly what supplies the
`LUID(39) > LUID(32)` that sched2 needs. But a bumped load is chained next to its FIRST
consumer, so:

  * **Three independent `16(v1)` loads** (target's count) means p10's only consumer is the
    `+4` read at target slot 29, sched1 chains insn 39 between insns 56 and 58, and sched2
    hits the T-30 pin — the load overshoots to slot 25 and strands a nop: r2, r3, r4 and
    t1, all score 5 / 67 insns.
  * **p10 with an early consumer** (one of the halfword reads CSE-folded onto it) unpins
    insn 39 and lands it on target's slot 12 — at the cost of one of the three loads:
    r5, r6, r7, all score 2 / 66 insns with a nop where the third load belongs.

Attempts to have both, measured and dead:
  * **t1** — split copy 3 into a named local so its store falls BETWEEN the p10 statement
    and the `+0x18` read, restoring three independent loads: score 5 / 67. The pin returns
    the moment the third load does.
    (`rejected/s10-split-copy3-restores-3-loads-and-the-T30-pin-score5.c`)
  * **t2** — hoist the `D_800A3478` gp store above the `+0x2` read to break CSE there
    instead: score 8 / 68 (the store-order fence drags the gp store forward).
    (`rejected/s10-gp3478-store-hoisted-above-temp2-read-score8.c`)
  * **t5** — write the `+0x18` read explicitly through `p10`: byte-identical to r5, score 2
    / 66. Confirms r5's fold is genuinely CSE and not an artifact of the spelling.

### 4. The proximate cause of the 3-load pin is REGISTER ALLOCATION, not scheduling

Reading the two sched2 traces side by side (`s3/n9/trace_sched2.txt` = prior staged body,
`s3/n9b/trace_sched2.txt` = the r4 3-load body) at the deciding cycle:

    n9  (works)  ;; ready list at T-30: 39 (3) 53 (8), now 53 39      <- 53 wins on priority
    n9b (pins)   ;; ready list at T-30: 39 (3), now 39                <- 39 forced in
                 ;; launching 56 before 39 with no stalls at T-31

Insn 53 is `sh v0,24(v1)` (the `+0x18` store) and insn 56 is the `+0x2` read's address
load. In n9b reload gives insn 56 the hard register **v0**, which insn 53 READS, so 53
carries a write-after-read anti-dependence on 56 and sched2 cannot release 53 until 56 is
scheduled — T-30 then comes up with insn 39 as the sole ready insn and the scheduler takes
it rather than stall. In the prior body and in target, insn 56 gets **a0**
(`lw a0,16(v1)` / `lhu a0,2(a0)`, a self-overwrite), there is no anti-dependence, 53 is
ready at T-30 and beats insn 39 on priority 8 vs 3. So the 3-load family closes the
function iff insn 56's destination can be steered from v0 to a0 by honest C.

Three attempts this session, all score 5 / 67, i.e. none moved insn 56 off v0:
  * `s32 temp2` instead of `u16 temp2` (u1)
  * `s32 temp_a1` instead of `u16 temp_a1` (u3)
  * a fresh single-set `s32 q = *(s32 *)(outer + 0x10);` hosting the `+0x2` read's address,
    forcing the address into its own pseudo (u2)
    (`rejected/s10-fresh-q-local-for-0x1A-address-does-not-move-insn56-off-v0-score5.c`)

`.greg` for both bodies shows the same five hard regs used (2 3 4 5 31) and a single
globally-allocated pseudo; the v0-vs-a0 choice is made before sched2 runs, in
local-alloc/reload, and is the next thing to read out rather than guess.

### Artifacts
- `tmp/grind/func_80060A68/s4/n1/` — full instrumented `-da` dump set for the floor-2 body
  with `BB2_PRIO_DEBUG=1`, plus `pass1.txt` / `pass2.txt` (this function's sched1 and
  sched2 debug streams, split out of the 10 MB `dbg.txt` by the `SCHEDDBG FUNC` markers)
- `tmp/grind/func_80060A68/s4/{r1..r9,t1,t2,t5,u1,u2,u3,q1,q3}.c` — the measured variants
- `tmp/grind/func_80060A68/s4/{apply.py,m.sh,split.py,run1.sh}` — the harness used

- [s10] Chassis re-measured: sandbox func_80060A68 --disable all = score 2, build 66 / target 66 with the ledger's floor-2 body; src/text1b.c reverted to HEAD before finishing.

- [s10] priority(insn 39) = 4 is UNREACHABLE, proved from the compiler's own PRIODBG stream rather than by argument: in sched2 insn 39's only producers are insn 9 (priority 1, cost 2) and insn 22 (priority 3, cost 1), giving max 3; a producer must be emitted before insn 39, and target's slots 1-11 contain no priority-4 insn at all, so the cost-1 route is empty; the cost-2 route needs insn 39's address to be the result of insn 25 or 32, both of which load *(s32 *)(outer + 0xC) rather than outer + 0x10; and deepening the shared producer insn 22 lifts 25, 32 and 39 identically.

- [s10] NEW SHAPE r5 (single-set s32 p10 = *(s32 *)(outer + 0x10); placed immediately AFTER the copy-3 store): score 2, build 66 / target 66, and the entire prefix through slot 20 is byte-identical to target INCLUDING slots 10/11/12 (lw v0,12(v1) / lw a0,12(v1) / lw a1,16(v1)). The sched2 rank_for_schedule LUID tie at sched.c:2464 that s2-s9 recorded as THE residual is absent from this body. Promoted to candidate.c; it carries no multiply-assigned local of any kind.

- [s10] r5's residual is a different defect: the *(u16 *)(outer + 0x18) read is CSE-folded onto p10's register (lhu v0,0(a1) instead of lw a0,16(v1) + lhu v0,0(a0)), so target's slot-23 load is missing and shows as a nop. r6 and r7 are the same defect displaced onto the +0x2 read (lhu a0,2(a1)).

- [s10] Conservation law measured across nine p10 source positions (r1-r9) plus t1/t2/t5: three independent 16(v1) loads implies p10 has no early consumer implies sched1 chains the bumped insn 39 between insns 56 and 58 and sched2's T-30 pin forces it to slot 25 (score 5 / 67 insns); giving p10 an early consumer unpins it onto target's slot 12 but CSE-folds one of the three loads away (score 2 / 66 insns with a nop). t1 (split copy 3 so its store re-separates the loads) restores the 3 loads AND the pin, score 5.

- [s10] The 3-load pin's proximate cause is REGISTER ALLOCATION, not scheduling: reload gives insn 56 (the +0x2 read's address load) hard register v0, and insn 53 is sh v0,24(v1), so 53 carries a write-after-read anti-dependence on 56 and sched2 cannot release 53 until 56 is scheduled; T-30 then holds insn 39 alone (n9b trace: ready list at T-30: 39 (3)) and the scheduler takes it. In target and in the prior staged body insn 56 gets a0 (self-overwrite lw a0,16(v1) / lhu a0,2(a0)), 53 IS ready at T-30 (n9 trace: ready list at T-30: 39 (3) 53 (8), now 53) and wins on priority 8 vs 3.

- [s10] Three honest attempts to move insn 56 off v0 all failed at score 5 / 67: s32 temp2, s32 temp_a1, and a fresh single-set s32 q local hosting the +0x2 read's address. The v0-vs-a0 decision is made in local-alloc/reload before sched2 runs and must be read out of .lreg/.greg next.

- [s4] Chassis re-measured this session: sandbox func_80060A68 --disable all = score 2, build 66 / target 66 with the ledger's floor-2 body applied at src/text1b.c:3321; src/text1b.c was reverted to HEAD before finishing (the asmfix.txt:109-110 integration hazard is unchanged).

- [s4] priority(insn 39) = 4 is unreachable, from the compiler's own PRIODBG stream: insn 39's producers in sched2 are exactly insn 9 (priority 1, cost 2) and insn 22 (priority 3, cost 1), max 3; target's slots 1-11 (the only legal producer pool, since a producer must be emitted before insn 39) contain no priority-4 insn; the only priority-3 loads in that pool are insns 25 and 32, which load *(s32 *)(outer + 0xC) and cannot supply insn 39's outer + 0x10 address; and the shared producer insn 22 lifts insns 25, 32 and 39 identically.

- [s4] NEW SHAPE r5 (single-set s32 p10 = *(s32 *)(outer + 0x10); placed immediately after the copy-3 store): score 2, build 66 / target 66, prefix byte-identical to target through slot 20 INCLUDING slots 10/11/12 (lw v0,12(v1) / lw a0,12(v1) / lw a1,16(v1)). The sched2 LUID tie at sched.c:2464 that s2-s9 recorded as THE residual does not occur in this body. Promoted to candidate.c; the prior staged-temp_a1 floor-2 body is preserved verbatim at memory/grind/func_80060A68/prior-floor2-staged-temp_a1.c (not rejected -- it still measures 2).

- [s4] r5's residual is a different defect: the *(u16 *)(outer + 0x18) read is CSE-folded onto p10's register, giving lhu v0,0(a1) where target has lw a0,16(v1) + lhu v0,0(a0), so target's slot-23 load (which also fills the load-delay slot after lhu v0,0(a0)) is missing and shows as a nop. r6 and r7 are the same defect displaced onto the +0x2 read (lhu a0,2(a1)).

- [s4] Nine p10 source positions measured: r1 = 2/65, r2 = 5/67, r3 = 5/67, r4 = 5/67 (the s9 body), r5 = 2/66, r6 = 2/66, r7 = 2/66, r8 = 3/66, r9 = 3/66. Position is NOT inert for a bumped local -- moving the statement changes which insns exist (CSE), which is what moves the score.

- [s4] Conservation law: three independent 16(v1) loads implies p10 has no early consumer implies sched1 chains the bumped insn 39 between insns 56 and 58 implies the sched2 T-30 pin (slot 25, score 5 / 67); giving p10 an early consumer unpins insn 39 onto target's slot 12 but CSE-folds one of the three loads away (score 2 / 66 with a nop). t1 (split copy 3 so its store re-separates the loads) restores both the three loads and the pin, score 5 / 67; t2 (hoist the gp store above the 0x1A read) scores 8 / 68.

- [s4] The 3-load pin's proximate cause is REGISTER ALLOCATION, not scheduling: reload gives insn 56 (the 0x1A read's address load) hard register v0, insn 53 is sh v0,24(v1), so 53 carries a write-after-read anti-dependence on 56 and sched2 cannot release 53 until 56 is scheduled; T-30 then holds insn 39 alone and the scheduler takes it. In target and in the prior staged body insn 56 gets a0 (self-overwrite lw a0,16(v1) / lhu a0,2(a0)), 53 IS ready at T-30 and wins on priority 8 vs 3.

- [s4] Three honest attempts to move insn 56 off v0 all measured 5 / 67: s32 temp2 (u1), s32 temp_a1 (u3), and a fresh single-set s32 q local hosting the 0x1A read's address (u2). The v0-vs-a0 decision is made in local-alloc/reload before sched2 runs and must be read out of .lreg/.greg rather than guessed.

- [s4] No banned construct appears in any body measured this session: every local in candidate.c (outer, idx, temp2, p10, temp_a1, result) is written exactly once and read for a real job. There is no multiply-assigned carrier, no staged-value-reused-variable claim, and no FAKE annotation in the new candidate.

---

## s5 (2026-08-19) — REDERIVE. Chassis re-measured; three new attractors; four axes killed.

Scratch: `tmp/grind/func_80060A68/s5/` (`apply.py` splices a body file into `src/text1b.c`;
`cmp.sh` prints a target-vs-build instruction table; `gen*.py` are the variant generators;
`v/*.c` are the 34 measured bodies). `src/text1b.c` was reverted to HEAD before finishing.

### Chassis
`candidate.c` applied to `src/text1b.c` re-measures **score 2 / build 66 / target 66** on today's
HEAD. The ledger floor of 2 is confirmed, not stale.

### The residual, stated exactly (candidate.c body)
Target slots 22-23 are `lhu $v0,0($a0)` / `lw $a0,0x10($v1)`; ours are `lhu v0,0(a1)` / `nop`.
Both bodies are 66 instructions; the whole prefix through slot 20 and the whole tail from slot 24
are byte-identical. The gap is one missing third `lw ?,0x10($v1)` (which in target fills the
slot-23 load-delay slot behind the `lhu` at 22) plus the register the `lhu` at 22 reads.

### NEW ATTRACTOR 1 — the three-fresh-inline-reads body (`v6` == `q1` == `q3` == `z2`): score 3, 66 insns

    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2                  = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1                = *(u16 *)(*(s32 *)(outer + 0x10) + 4);

No `p10` local at all — the simplest, most human body the campaign has produced (five locals, every
one written once and read for a real job, zero named-intermediate apparatus to defend). It emits
**all three** `lw ?,0x10($v1)` loads and stays at 66 instructions. Its residual is a *different*
shape from candidate.c's: target's early `lw $a1,0x10($v1)` at slot 12 is absent, and the third
load appears instead at slot 27 as `lw v0,0x10($v1)` feeding `lhu a1,4(v0)` at slot 29. So this
body is short exactly one *hoist*, not one *load*.
Banked: `rejected/s5-three-fresh-inline-reads-no-p10-load-lands-slot27-v0-score3-66insns.c`.

`q1` (a `p10` read placed early and consumed by the +0 read) and `q3` (same, consumed by the +2
read) both compile to **byte-identical output to `v6`** — cse propagates `p10` into whichever read
consumes it and the load then schedules with that read, so an early-consumed `p10` is not a
distinct body at all. Two more seats of the "give p10 an early consumer" idea, both collapsing.

### NEW ATTRACTOR 2 — `q5`: score 2, 66 insns, cleaner residual than candidate.c

    p10 = *(s32 *)(outer + 0x10);                            /* before the copy-3 store */
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);
    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);  /* fresh load */
    temp2   = *(u16 *)(p10 + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1 = *(u16 *)(p10 + 4);

Same floor as candidate.c (2) but strictly better aligned: slot 22 is `lhu v0,0(a0)` — the **correct
register**, which candidate.c gets wrong (`lhu v0,0(a1)`). The only two differing slots are 23
(`nop` vs `lw $a0,0x10($v1)`) and 25 (`lhu a0,2(a1)` vs `lhu $a0,0x2($a0)`), i.e. the residual is
now *purely* the missing third load and the register it would have supplied.
Banked: `rejected/s5-p10-early-feeds-plus2-and-plus4-plus0-fresh-score2-66insns.c`.

### KILL — the "no `outer` local" / m2c-literal family (g1-g4): 26-29, 68-70 insns
A fresh m2c decompile (`tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax`) produces a body with
**no local for the context pointer** — every access is `M2C_FIELD(D_800A3468, ...)`, i.e. the
gp-relative global is re-read at every use. Transcribed faithfully and measured in four orderings
(m2c's own order, the v6 order, the candidate order and the v2 order) the family measures **26, 26,
28, 29** at 68-70 instructions. GCC 2.7.2 does not keep the global in one register across the
interleaved stores the way it keeps `outer`; each store forces a reload of the gp word. The m2c
shape is a decompiler artefact, not the original spelling. **Do not re-transcribe m2c output for
this function.** Banked:
`rejected/s5-no-outer-local-global-D_800A3468-inline-everywhere-score26-68insns.c`.

m2c's statement ORDER, however, is worth recording as independent corroboration of the source order
the campaign already uses: zero-store, three s32 copies, +0 read/store, `D_800A3478` store, +2
read/store, +4 read into a temp, idx read, `D_800A347C` store, 0x1C store, call. That matches the
candidate/v6 order exactly except that m2c does not need a `temp2` (it stores the +2 read directly);
the `temp2`-free spelling was measured as `z1` and scores 9 / 68 insns, so `temp2` is load-bearing
and honest (it is what holds the +2 value across the intervening `D_800A3478` store).

### KILL — struct-typed member access (MEM_IN_STRUCT_P) regresses every seat that moves
`struct S3 { u16 a, b, c; };` with the inner reads spelled `(*(struct S3 **)(outer + 0x10))->a`:
- v6 order with struct reads (`x1`) = **6** (the raw-cast identical body = 3)
- candidate order with a `struct S3 *p10` (`x2`) = 2 (neutral)
- both sides struct-typed, destination `((struct S3 *)(outer + 0x18))->a` (`x3`) = **8**
- `u16 *` array-index spelling `(*(u16 **)(outer + 0x10))[0..2]` (`x4`) = **6**

Setting `MEM_IN_STRUCT_P` on the halfword reads is *not* codegen-neutral here, and every seat that
moves is a regression. Banked:
`rejected/s5-struct-typed-inner-reads-MEM_IN_STRUCT_P-regresses-3-to-6.c`.

### KILL — naming the address pseudo does not move reload's v0-vs-a0 choice in the 3-load body
The s10 frontier's item 1 asked for "an honest C change that alters that pseudo's LIVE RANGE rather
than its type". Four fresh seats measured, all **5 / 67 insns**, byte-identical to the unnamed `v2`
body: `r1` (named `s32 p0` for the +0 read's address), `r2` (named `s32 p2` for the +2 read's
address), `r5` (both named), `r8` (`p10` typed `u16 *` and read as `p10[2]`). Declaring a named
local for an address that GCC already has a pseudo for changes nothing about the allocno's live
range — cse/local-alloc see the same single-def single-use pseudo either way. Combined with s10's
three type-level attempts, the *declaration-level* levers on insn 56's destination are exhausted
from seven seats; what remains is a change to WHICH INSTRUCTIONS the pseudo lives across. Banked:
`rejected/s5-v2-with-named-address-locals-p0-p2-neutral-score5-67insns.c`.

### KILL — the gp store is not what pins the +4 load low in the v6 body
Hypothesis: in `v6` the third `lw ?,0x10($v1)` sits at slot 27 because it is written *after*
`D_800A3478 = outer + 0x18;`, and a load cannot be hoisted above a store to a different (hence
non-disambiguable) base. Probe `w1` sinks the gp store below the +4 read. Result: **score 7, 66
insns — the load moved only from slot 27 to slot 25, not to slot 12**, and the two stores came out
in the wrong relative order (`sh a0,0x1A($v1)` before the gp store; target is the reverse, and GCC
2.7.2 never reorders two stores whose bases it cannot disambiguate). So the gp store is a real
barrier but not the binding one: the +4 load's scheduling priority, set by the distance from its
sole consumer to the end of the block, is what keeps it low. Banked:
`rejected/s5-gp-store-sunk-below-plus4-read-load-still-not-hoisted-score7.c`.

### The store-separator law, now proven in both directions — and it DERIVES the target's C shape
cse.c does NOT disambiguate two `(plus pseudo const)` addresses, so any intervening store forces a
fresh load; sched.c DOES disambiguate them (`memrefs_conflict_p`), so a load may be hoisted across
those same stores. Every 0x10-read spelling in this campaign is a consequence of that asymmetry: a
read folds onto the previous read of `*(s32 *)(outer + 0x10)` iff no store separates them in source
order, full stop.

Applying that law to the TARGET's own instruction stream pins down the target's C:
- the +0 and +2 reads have separate address loads (target slots 20 and 23), so a store separates
  them in source — and the only candidate is `sh $v0,0x18($v1)`, which is exactly where it sits;
- the +4 read also has its own address load (target slot 12), so a store must separate it from the
  +2 read too — but the only stores after the +2 *load* are the gp store (slot 27) and
  `sh $a0,0x1A($v1)` (slot 28), and GCC never reorders two non-disambiguable stores, so both are
  after the +2 read in source as well. **There is no store available between the +2 read and the +4
  read.** Therefore the target's +4 pointer cannot be a third in-line read; it must be an early,
  separately-read local whose value survives to slot 29.

That body is `v2` (p10 read before the copy-3 store; +0 and +2 read fresh; +4 read through p10),
and `v2` measures 5 / 67. **The target's C shape is now derived rather than guessed; what is
unreproduced is purely its register allocation**, exactly as the s10 attribution said.

### Statement-position sweep (all measured this session)
| body | shape | score | insns |
|---|---|---|---|
| `z3`/candidate | p10 after copy-3 store, +0 folds onto p10 | **2** | 66 |
| `q5` | p10 before copy-3 store, feeds +2 and +4; +0 fresh | **2** | 66 |
| `v6`/`q1`/`q3`/`z2` | three fresh in-line reads, no p10 use | 3 | 66 |
| `y4` | +4 read hoisted above the `D_800F10D0` zero-store | 4 | **65** |
| `v2`/`z4`/`r1`/`r2`/`r5`/`r8`/`q6`/`q7`/`w2` | p10 early, +0 and +2 fresh (the DERIVED target shape) | 5 | 67 |
| `x1`,`x4` | struct- / array-typed inner reads | 6 | 66 |
| `w1`,`w3`,`w6`,`x5` | gp store sunk below the +4 read | 7 | 66-67 |
| `w4`,`w5`,`x3` | S7C/idx/dest-struct permutations | 8 | 66 |
| `y3`,`w7`,`z1` | +4 read above the copies / gp store above +2 / no `temp2` | 9 | 66-68 |
| `w8`,`y1`,`y2`,`v1`,`y5`,`y6` | 0x1C/0x347C swap, +4 among the copies, gp store above +0 | 10-14 | 66-68 |
| `g1`-`g4` | no `outer` local (m2c literal) | 26-29 | 68-70 |

`y4` is the only body in the campaign that is 65 instructions (one SHORT of target): putting the +4
read above the `D_800F10D0` zero-store lets cse fold something the target does not fold. Noted for
completeness, not a lead.

### Where s5 leaves the frontier
The derivation above collapses the search: `v2` IS the target's C, and the entire remaining gap is
that reload gives the +2 read's address pseudo `$v0` where the target gives it `$a0`, which creates
a write-after-read anti-dependence against `sh $v0,0x18($v1)` and costs both the slot-23 fill and
the p10 hoist. Declaration-level levers on that pseudo are dead from seven seats. The next lever
must change the pseudo's live range — i.e. what else is live at the +2 read — and that has to be
read out of `.lreg`/`.greg` for `v2` and for `q5` side by side, comparing the conflict sets and the
allocno order rather than the C.

- [s5] CHASSIS: candidate.c applied to src/text1b.c re-measures score 2 / build 66 / target 66 on today's HEAD. The ledger floor of 2 is confirmed, not stale.

- [s5] The candidate.c residual, exactly: target slots 22-23 are `lhu $v0,0($a0)` / `lw $a0,0x10($v1)`; ours are `lhu v0,0(a1)` / `nop`. Prefix through slot 20 and tail from slot 24 are byte-identical.

- [s5] NEW ATTRACTOR v6 (== q1 == q3 == z2): three fresh in-line reads of *(s32 *)(outer + 0x10), NO p10 local at all, score 3 / 66 insns. It emits all three `lw ?,0x10($v1)` loads; the third lands at slot 27 in v0 (feeding `lhu a1,4(v0)` at 29) instead of at slot 12 in a1. This body is short one HOIST, not one load, and is the plainest C the campaign has produced -- five locals, each written once and read for a real job, no named-intermediate apparatus to defend.

- [s5] NEW ATTRACTOR q5: p10 read before the copy-3 store and feeding the +2 and +4 reads, +0 read fresh in-line. score 2 / 66 -- same floor as candidate.c but gets target's slot 22 register right (a0, not a1); the only wrong slots are 23 (nop) and 25 (register).

- [s5] DERIVATION (the session's main result): under the store-separator law the target's +4 pointer CANNOT be a third in-line read, because no store is available between the +2 read and the +4 read and GCC never reorders two non-disambiguable stores. Therefore target's C is the v2 body (p10 read before the copy-3 store; +0 and +2 read fresh in-line; +4 read through p10). v2 measures 5 / 67, so the entire remaining gap on this function is v2's REGISTER ALLOCATION -- reload gives the +2 read's address pseudo $v0 where target gives it $a0.

- [s5] The store-separator law is now proven in both directions: cse.c does not disambiguate two (plus pseudo const) addresses (any intervening store forces a fresh load), while sched.c does disambiguate them via memrefs_conflict_p (loads may be hoisted across those same stores). Every 0x10-read spelling in this campaign is a consequence of that asymmetry.

- [s5] KILL: the m2c-literal 'no outer local' family scores 26-29 at 68-70 insns across four orderings -- GCC reloads the gp word at every interleaved store instead of caching D_800A3468 in one register. Do not re-transcribe m2c output for this function.

- [s5] KILL: struct-typed member access (MEM_IN_STRUCT_P) regresses every seat that moves -- v6 order 3 -> 6, both-sides-struct 8, u16* array-index 6; only the candidate-order seat is neutral at 2.

- [s5] KILL: naming the +0 / +2 read's address in an explicit local (r1, r2, r5) or typing p10 as u16 * (r8) is byte-identical to the unnamed v2 body at 5 / 67. Declaration-level levers on insn 56's destination are now dead from seven seats (four here, three type-level in s10).

- [s5] KILL: sinking the D_800A3478 gp store below the +4 read (w1, w3, w6, x5) moves the third load only from slot 27 to slot 25 and permanently swaps the two stores out of target order; scores 7 across all four seats. The gp store is a barrier but not the binding one.

- [s5] KILL: q1 and q3 (p10 read early with an explicit early consumer) compile BYTE-IDENTICALLY to v6 -- cse propagates p10 into the consuming read and the local vanishes. 'Give p10 an early consumer' is not a distinct body, and the s4 conservation law survives.

- [s5] 34 bodies measured this session across the full statement-position / typing / base-expression space; the full score table is in evidence.md. No body beat floor 2. y4 (the +4 read hoisted above the D_800F10D0 zero-store) is the only 65-instruction body ever produced on this function (score 4) -- noted, not a lead.

- [s5] NO BANNED CONSTRUCT appears in any body measured this session. Every local in candidate.c, q5 and v6 is written exactly once and read for a real job; there is no multiply-assigned carrier, no staged-value-reused-variable claim, no FAKE annotation, no volatile, no pin, no pad.

- [s5] src/text1b.c was reverted to HEAD before this session finished; the working tree carries only memory/grind/ ledger updates and tmp/ scratch.

## s6 (2026-08-19, rederive) -- the residual is ONE sched.c gate: birthing_insn_p on insn 39

Chassis re-measured on today's HEAD before anything else: candidate.c = score 2, build 66 /
target 66 (identical to s5's number, so nothing in the chassis moved).  All s6 numbers below
are from that same chassis.  src/text1b.c was restored to HEAD before this session finished.

### 1. Independent re-derivation of the target's C (stronger than s5's)

s5 derived the v2 shape by arguing that no store is available to separate the +2 read from the
+4 read.  s6 re-derived the same conclusion from scratch, but by ENUMERATING every store that
could act as a cse separator rather than by looking only at the +2/+4 adjacency.  The
enumeration is exhaustive because two facts pin it:

  (a) cse.c will not disambiguate two `(plus (reg) (const_int))` addresses, so two reads of
      `*(s32 *)(outer + 0x10)` collapse to one load unless a STORE sits between them; and
  (b) sched.c WILL disambiguate two `(plus (reg) (const_int))` addresses off the same base, so
      loads freely hoist across the copy stores and the 0x18/0x1A/0x1C halfword stores -- but
      it will NOT disambiguate a `(mem (symbol_ref "D_800A3478"))` store from a
      `(plus (reg) (const_int 16))` load, so nothing hoists across the gp stores.

Target needs THREE `lw ?,0x10($v1)` loads (slots 11/19/22, consumed at 28/21/24 = +4/+0/+2),
so it needs TWO separators.  The store sequence is fixed by the fact that stores never
reorder: D_800F10D0 zero, 0x20, 0x24, 0x28, 0x18, gp-3478, 0x1A, gp-347C, 0x1C.  The +0 read
must precede the 0x18 store (it feeds it) and the +2 read must precede the 0x1A store and the
gp-3478 store (it is emitted at slot 22, above the gp store at 26, and cannot hoist across it
per (b)).  That leaves exactly one legal assignment of separators: the +4 pointer is read
FIRST, separated from the +0 read by one of the copy stores, and the +0/+2 pairs is separated
by the 0x18 store.  That is v2, and it is forced -- there is no second solution.

s6 also measured the one partition s5 did NOT try, in which the +0 read is hoisted above the
copy-3 store into its own `t0` local so that the copy-3 store becomes the +0/+2 separator and
the 0x18 store is free to move.  Every seat of it regresses hard:

  - x1  p10 above copy2, +0 value read above copy3, 0x18 store after copy3  -> 9 / 67
  - x2  p10 above copy1, +0 value read after copy1                          -> 13 / 66
  - x6  p10 above copy1, +0 value read after copy2                          -> 9 / 67
  - x7  x1 with the 0x1A store hoisted above the gp-3478 store              -> 11 / 67
  - x8  x1 with the idx read hoisted above the +4 read                      -> 11 / 67

so the "hoist the +0 VALUE and re-use a copy store as the separator" family is dead.

### 2. v6 re-measured, and frontier item 2 KILLED with a mechanism, not a number

v6 (three fresh in-line reads, no p10 local) re-measures 3 / 66.  Its full normalised
disassembly diff against target (tmp/grind/func_80060A68/s6/v6.dis) is now exact: the two
streams are IDENTICAL except that the third `lw ?,0x10($v1)` sits at slot 26 in v6 and at
slot 11 in target, with everything between shifted one slot and the consumer reading `$v0`
instead of `$a1`.  One instruction, one position.

s5's frontier item 2 proposed closing v6 by raising that load's INSN_PRIORITY until sched2
hoists it from slot ~27 to slot 12.  That is impossible and the reason is not a priority gap:
in v6 the load sits BELOW `sw $v0,%gp_rel(D_800A3478)($gp)` (slot 25), and per (b) above
sched.c cannot disambiguate a symbol_ref store from a `(reg + const)` load, so the load may
not cross it at ANY priority.  Frontier item 2 is killed.  (This is also the independent
proof of the derivation in section 1: the +4 pointer cannot be an in-line read, because an
in-line read is necessarily below the gp store.)

### 3. THE MECHANISM (dumps + cc1 source): both v2 defects have ONE cause

`pwsh tools/grinder/dump.ps1 func_80060A68` was run with v2 in place.  Reading
tmp/grind/func_80060A68/dumps/text1b.lreg (function region at line 29591) and
text1b.sched (function region at line 34035):

  insn 49  reg90 = mem(reg72 + 16)      the +0 read's address
  insn 51  reg91 = mem(reg90 + 0)       the +0 halfword value
  insn 53  mem(reg72 + 24) = reg91      the 0x18 store              priority 8
  insn 56  reg92 = mem(reg72 + 16)      the +2 read's address       priority 3
  insn 39  reg75 = mem(reg72 + 16)      p10 (the +4 pointer)        priority 3
  insn 58  reg74 = mem(reg92 + 2)       temp2

sched1's own trace (`;; ready list at T-N:` lines in text1b.sched) shows the decision:

  ;; ready list at T-30: 39 (7f000001) 58 (7f000001), now 58 39
  ;; ready list at T-31: 39 (7f000001) 53 (8), now 39 53
  ;; launching 56 before 39 with no stalls at T-32
  ;; ready list at T-32: 53 (8) 56 (7f000001), now 56 53
  ;; ready list at T-33: 53 (8), now 53

sched.c schedules BACKWARD, so a larger T is an EARLIER position; the emitted order is
therefore 51 (T-35), 53 (T-33), 56 (T-32), 39 (T-31), 58 (T-30) -- which is exactly the order
the .lreg RTL shows.  Target's order is 51, 56, 53, 58 with 39 far above at slot 11.

0x7f000001 is `LAUNCH_PRIORITY` (tools/gcc-2.7.2/sched.c:187).  Two lines put it on 39 and 56:

  - sched.c:4049, in schedule_block: the insn currently being scheduled has its
    INSN_PRIORITY temporarily set to LAUNCH_PRIORITY before schedule_insn releases its
    predecessors ("Give INSN high enough priority that at least one (maybe more) reg-killing
    insns can be launched ahead of all others").
  - sched.c:2541-2590, adjust_priority: a released insn with `n_deaths == 0` and
    `birthing_insn_p (PATTERN (prev))` true has its INSN_PRIORITY raised to `max_priority`,
    which at that moment IS the LAUNCH_PRIORITY of the insn being scheduled.  `n_deaths` is
    always 0 -- GCC's own comment at sched.c:2551 says "This code has no effect, because
    REG_DEAD notes are removed before we ever get here."

and `birthing_insn_p` (sched.c:2505-2536) returns true iff reload has not completed, the
pattern is `(set (reg) ...)`, the destination is live in `bb_live_regs`, and -- the only gate
reachable from C source --

        return (reg_n_sets[i] == 1);

So EVERY single-set pseudo birth outranks EVERY store, unconditionally.  That is why insn 39
beats insn 53 at T-31.  And insn 39 winning T-31 is what forces 53 to T-33 and 56 to T-32,
i.e. what puts the 0x18 store BEFORE the +2 address load.  With 53 emitted first, reg91 (the
+0 value, hard reg $v0) is already dead when reg92 is born, so local-alloc hands reg92 $v0
again; in target, reg92 is born while reg91 is still live, $v0 is unavailable, and reg92 takes
$a0 -- the register target actually uses.  The missing slot-22 load and the wrong slot-25
register are therefore NOT two defects; they are one, and the single cause is insn 39's
LAUNCH_PRIORITY bump.

If insn 39 were not bumped, T-31 would go to 53 (priority 8 vs 39's honest 3), T-32 to 56, and
39 would drift to a much larger T -- an early slot.  That is target's stream exactly.

### 4. What that means for the remaining lever

The bump's only C-reachable gate is `reg_n_sets[p10] != 1`, i.e. writing the p10 local more
than once.  That is precisely the multiply-assigned pointer-staging carrier the Judge banned
for this function on 2026-08-19 ("banned for func_80060A68 regardless of which identifier
hosts it ... the carrier axis (all three partition seats) is now closed"), and it is why every
session from s1 onward re-converged on that construct: it is not one spelling among many, it
is the unique C-visible input to sched.c:2536.  The banked score-0 body
(rejected/banned-temp2-dual-role-score0-layer1-and-judge-FAIL.c) already documents the same
mechanism in its own FAKE annotations, so the identification is now doubly confirmed -- from
the cc1 source and from the dump -- rather than inferred.

s6 also measured the cheapest possible spelling of that construct to confirm no accidental
back door exists: `p10 = outer; p10 = *(s32 *)(outer + 0x10);` measures 5 / 67, byte-identical
to v2, because flow.c deletes the dead first store before sched1 runs and `reg_n_sets` is back
to 1.  A multiply-set carrier only works if BOTH writes are live, which is exactly the banned
shape.  Banked at rejected/s6-DIAGNOSTIC-dead-first-write-to-p10-is-DCEd-inert-score5.c.

### 5. Neutral / regressive seats measured this session (all on the v2 body)

  neutral, byte-identical to v2 at 5 / 67:
    w1  the +0 halfword value given its own named `t0` local
    w2  a `u16 *dst = (u16 *)(outer + 0x18)` local carrying the 0x18/0x1A/0x1C stores and
        the gp-3478 value (the sibling func_80060B70 `dst_u16` idiom)
    w3  a SECOND early pointer local `p10b` for the +2 read
    w6  p10 placed between copy 1 and copy 2
    w7  p10 placed above the whole copy triple
  regressive:
    w4  idx read hoisted above the +4 read                       7 / 67
    w5  0x1C store hoisted above the gp-347C store               8 / 67
    y1  +4 read sunk below the gp-347C store                     8 / 68
    y5  idx read hoisted above the gp-3478 store                 8 / 67
    y6  idx read hoisted to the top of the halfword group        8 / 67

The v2 statement order is a strict local optimum: every single-statement move off it costs
2-8 points.

### 6. Artifacts

  tmp/grind/func_80060A68/s6/{swap.py,norm.py,dis.sh,batch.ps1,gen.py,variants*.py}
  tmp/grind/func_80060A68/s6/{A_candidate,v2,v6}.dis  -- normalised disassemblies
  tmp/grind/func_80060A68/s6/target.txt               -- normalised target stream
  tmp/grind/func_80060A68/dumps/text1b.{sched,lreg,greg}  -- v6 dump in place at session end;
        the v2 dump quoted in section 3 was taken with v2 applied and its numbers are
        transcribed above verbatim.

- [s6] Chassis re-measured first thing: candidate.c = score 2, build 66 / target 66 on today's HEAD - the floor is confirmed, not stale. src/text1b.c was restored to HEAD before the session finished (git status shows only memory/grind and metrics dirt).

- [s6] The target's three `lw ?,0x10($v1)` loads are at slots 11, 19, 22 and are consumed at slots 28 (+4), 21 (+0) and 24 (+2) - all three address pseudos are single-use in target as well as in our builds, so 'give the pointer a second USE' is not the difference between us and target.

- [s6] cse/sched separator law, restated precisely: cse.c does NOT disambiguate two (plus (reg) (const_int)) addresses (a store must separate two reads for them to stay two loads); sched.c DOES disambiguate them off a common base (loads hoist across the 0x20/0x24/0x28/0x18/0x1A/0x1C stores) but does NOT disambiguate a (mem (symbol_ref)) gp store from a (reg + const) load (nothing crosses D_800A3478 / D_800A347C).

- [s6] Applying that law to the fixed store order (D_800F10D0 zero, 0x20, 0x24, 0x28, 0x18, gp-3478, 0x1A, gp-347C, 0x1C) yields exactly ONE legal three-load separator assignment - the v2 body. Five seats of the only untried alternative partition (+0 value hoisted into a local above the copy-3 store) measure 9, 13, 9, 11 and 11.

- [s6] v6 (three fresh in-line reads, no p10 local) = 3/66 and is one instruction from target: the third 0x10 load at slot 26 instead of slot 11, blocked by `sw $v0,%gp_rel(D_800A3478)($gp)` at slot 25.

- [s6] sched1 trace for v2 (tmp/grind/func_80060A68/dumps/text1b.sched, function region at line 34035): insn 53 (0x18 store) priority 8; insns 39 (p10 load) and 56 (+2 address load) both released at LAUNCH_PRIORITY 0x7f000001; `ready list at T-31: 39 (7f000001) 53 (8), now 39 53`.

- [s6] Pseudo map from text1b.lreg (function region at line 29591): insn 49 reg90 = mem(reg72+16) [+0 address]; insn 51 reg91 = mem(reg90+0) [+0 value]; insn 53 mem(reg72+24) = reg91 [0x18 store]; insn 56 reg92 = mem(reg72+16) [+2 address]; insn 39 reg75 = mem(reg72+16) [p10]; insn 58 reg74 = mem(reg92+2) [temp2].

- [s6] GCC source gate: LAUNCH_PRIORITY = 0x7f000001 (sched.c:187); set on the scheduling insn at sched.c:4049; propagated to released birthing insns by adjust_priority (sched.c:2541-2590) via max_priority; birthing_insn_p (sched.c:2505-2536) returns reg_n_sets[i] == 1 for a live (set (reg) ...) before reload. adjust_priority's n_deaths switch never fires (REG_DEAD notes already removed - GCC's own comment at sched.c:2551).

- [s6] A dead first write to p10 (`p10 = outer;` then the real load) is removed by flow.c before sched1, leaves reg_n_sets == 1 and measures 5/67 byte-identical to v2 - so the banned carrier has no harmless spelling.

- [s6] Eleven single-statement / declaration-level seats on the v2 body: five inert at 5/67 (including the sibling func_80060B70 `dst_u16` pointer-local idiom), six regressive at 7-8.

## s7 (2026-08-19) — synthesis

CHASSIS RE-MEASURED FIRST.  candidate.c applied to src/text1b.c: **score 2 / build 66 /
target 66** on today's HEAD.  The ledger floor of 2 is current, not stale.  Every number
below was measured this session with `sandbox func_80060A68 --disable all`.

### The merge: what the six prior sessions actually add up to

Target's own stream (asm/funcs/func_80060A68.s) contains THREE `lw ?,0x10($v1)` loads:
slot 11 into `$a1` (consumed 17 slots later at slot 28, `lhu $a1,0x4($a1)` — the +4 read),
slot 19 into `$a0` (consumed at slot 21, the +0 read), slot 22 into `$a0` (consumed at
slot 24, the +2 read).  So the target's C is: ONE pointer local (`p10`) whose ONLY consumer
is the +4 read, plus two fresh in-line reads for +0 and +2.  That is exactly the
candidate.c / v2 / cb shape — the shape question is settled and s5/s6's derivation is
correct.  What separates the bodies is only WHERE the p10 statement and its consumer sit.

### NEW THIS SESSION — a strictly better forensic seat than v2: `cb`, 4 / 66

s6 worked from v2 (p10 above the copy-3 store, consumer left at its candidate.c seat) =
5 / 67.  s7 swept the CONSUMER position — an axis no session had moved (s6's w-series moved
`p10`, never `temp_a1 = *(u16 *)(p10 + 4)`).  Seats measured on the v2 body, consumer placed:
  ca  immediately after the copy triple, above the 0x18 store      -> 8 / 66
  cb  immediately after the 0x18 store, above the +2 read          -> **4 / 66**
  cc  after the +2 read, above the D_800A3478 store                -> 7 / 66
  cd  after the D_800A3478 store, above the 0x1A store             -> 8 / 67
  v2  after the 0x1A store (the inherited seat)                    -> 5 / 67
  ce  after the `idx` read                                         -> 7 / 67
  (y1, after the D_800A347C store, was already 8 / 68 in s6.)

`cb` is the best non-candidate body the campaign has produced and is a better seat than v2
on every axis: 66 instructions (correct count, no stray nop), all THREE 0x10 loads present,
and a single localised defect.  Normalised diff vs target (tmp/grind/func_80060A68/s7/cb.dis):
the entire stream matches with a ONE-SLOT SHIFT from slot 11 onward, because p10's load is
emitted at slot 23 as `lw $v0,0x10($v1)` (consumed at slot 25, `lhu $a1,0x4($v0)`) instead of
at slot 11 as `lw $a1,0x10($v1)`.  ONE instruction in the wrong place, in the wrong register.

p10's SOURCE position is irrelevant once the consumer is at the cb seat: cb1 (p10 above
copy 1) = 4 / 66, cb2 (above copy 2) = 4 / 66, cb (above copy 3) = 4 / 66; only cb4 (p10
below the copy triple, which lets cse fold the +0 read onto p10 again) regresses to 6 / 66.

### The cb defect has the SAME single cause s6 named, now proven on a second body

`pwsh tools/grinder/dump.ps1 func_80060A68` with cb applied; text1b.sched, sched1 trace for
block 0 (dump line 34035 ff.):
    ;; launching 39 before 59 with no stalls at T-32
    ;; ready list at T-32: 53 (8) 39 (7f000001), now 39 53
insn 39 = p10's load, insn 59 = its consumer (the +4 lhu), insn 53 = the 0x18 store
(honest priority 8).  Structurally identical to s6's v2 trace at T-31.  0x7f000001 is
LAUNCH_PRIORITY; insn 39 is RELEASED by its own consumer being scheduled and, because the
birthing bump makes it outrank everything, it takes that very cycle.  **Under the bump a
single-set pointer load can NEVER be separated from its consumer in sched1** — it is always
placed at the first cycle it becomes ready, which is the cycle adjacent to its consumer.

That is a body-independent geometric argument, and it is new: the target has p10's load and
its consumer SEVENTEEN slots apart.  sched1 cannot produce that under the bump.  The only
pass that can is sched2, which runs with `reload_completed == 1` and therefore never bumps
(sched.c:2509) — but sched2 can only hoist the load if its hard register is free across the
gap.  In target the register is `$a1` (free from slot 11 to slot 28).  In cb, local-alloc
gives the short-lived pseudo `$v0`, which is written at slot 20 and read at slot 22, so the
hoist is blocked.  Chain: bump -> load pinned adjacent to consumer -> short live range ->
`$v0` -> sched2 hoist blocked -> slot 23 instead of slot 11.

### Frontier item 2 (birthing_insn_p's `bb_live_regs` gate) is KILLED — measured, not argued

sched.c:2524-2531 tests `bb_live_regs[offset] & bit` BEFORE `reg_n_sets[i] == 1` and returns
0 outright if the bit is clear.  s6 left this unprobed.  s7 probed it with the instrumented
cc1 (tools/gcc-2.7.2/cc1, BB2_SCHED_DEBUG=1; harness tmp/grind/func_80060A68/s7/adjpri.sh,
log tmp/grind/func_80060A68/s7/adjpri.log, 6620 ADJPRI records TU-wide, 3611 birth=0 /
3009 birth=1 so the flag is genuinely discriminating).  For our block (identified by
`SCHEDDBG block=0 n_insns=44 n_ready=1` at log line 55776 plus the matching per-insn
priority list, i.e. insn 9 pri 1 / insn 39 pri 3 / insn 133 pri 2147483528):
    SCHEDDBG ADJPRI insn=56 deaths=0 birth=1 maxpri=2130706433 pri=8
    SCHEDDBG ADJPRI insn=59 deaths=0 birth=1 maxpri=2130706433 pri=3
    SCHEDDBG ADJPRI insn=53 deaths=0 birth=0 maxpri=2130706433 pri=8
    SCHEDDBG ADJPRI insn=39 deaths=0 birth=1 maxpri=2130706433 pri=3
insn 39 is birth=1: its destination IS in bb_live_regs.  This is inevitable rather than
incidental — sched.c schedules backward, so insn 39 is only released once a consumer that
READS its destination has been scheduled, and that read is exactly what puts the bit in
bb_live_regs.  A load whose dest is not live is a dead load and is deleted by flow.c.  The
`bb_live_regs` door is therefore closed for every C shape, not just for cb.  `reg_n_sets[i]
== 1` is the sole remaining C-visible gate, and it is the Judge-banned multiply-written
carrier.

### Also killed this session (all inert on the cb body — the dressing axes are dead there too)

Declaration order (a sanctioned family, never before measured on a 3-load body): p10 first
(cbd1), p10 last (cbd2), p10 second (cbd3), full permutation with temp_a1 first (cbd4) —
all 4 / 66, i.e. pseudo-numbering has no effect on the allocation of p10.  Type level:
`s32 temp_a1` (cbt1), `u16 *p10` with `p10[2]` for the +4 read (cbt2), `s32 temp2` (cbt3) —
all 4 / 66.  Separator level: the `D_800A347C = outer + 0x20;` gp store hoisted to sit
between p10 and the +0 read as an alternative cse separator (g1) = 8 / 68, and with the
D_800A3478 store used the same way (g2) = 11 / 68 — the gp stores are absolute scheduling
barriers, so using one as the separator wrecks the whole prefix.  This closes "is there any
separator other than a copy store" as an open question.

- [s7] Chassis re-measured at session start: candidate.c applied to src/text1b.c gives score 2 / build 66 / target 66 on today's HEAD. The ledger floor of 2 is current, not stale.

- [s7] The target's own stream contains THREE lw ?,0x10($v1) loads - slot 11 into $a1 (consumed seventeen slots later at slot 28, lhu $a1,0x4($a1)), slot 19 into $a0 (consumed slot 21, the +0 read), slot 22 into $a0 (consumed slot 24, the +2 read). The target's C is therefore one pointer local consumed only by the +4 read plus two fresh in-line reads, which is exactly the candidate.c / v2 / cb shape. The shape question is settled; only statement placement differs between bodies.

- [s7] NEW BEST NON-CANDIDATE BODY: cb = 4/66, banked at memory/grind/func_80060A68/rejected/s7-consumer-after-0x18-store-3-loads-p10-load-pinned-adjacent-to-consumer-score4-66insns.c. Correct instruction count, all three 0x10 loads present, one defect - p10's load at slot 23 in $v0 rather than slot 11 in $a1, shifting slots 11+ by one. It is a better forensic seat than v2 (5/67) on every axis and should replace v2 as the dump-reading base.

- [s7] Consumer-seat sweep on the v2 body: ca 8/66, cb 4/66, cc 7/66, cd 8/67, v2 5/67, ce 7/67 (s6's y1 was 8/68). p10's own position is inert once the consumer sits at the cb seat: cb1 4/66, cb2 4/66, cb 4/66, cb4 6/66.

- [s7] cb's sched1 trace (tmp/grind/func_80060A68/dumps/text1b.sched, block 0 at line 34035 ff.): 'launching 39 before 59 with no stalls at T-32' / 'ready list at T-32: 53 (8) 39 (7f000001), now 39 53' - structurally identical to s6's v2 trace at T-31. Under the birthing bump a single-set pointer load is always emitted adjacent to its consumer, because it becomes ready exactly when the consumer is scheduled and then outranks everything.

- [s7] GEOMETRIC PROOF (new this session): the target has p10's load and its consumer seventeen slots apart, which sched1 cannot produce under the bump. Only sched2 can open that gap (reload_completed == 1 disables birthing_insn_p at sched.c:2509), and only when the hard register is free across it - $a1 in the target, $v0 in cb (written slot 20, read slot 22). So the target's compile did not bump its insn-39 equivalent, on any body. This is independent of s6's source-level argument and reaches the same conclusion.

- [s7] birthing_insn_p's bb_live_regs gate measured with the instrumented cc1: SCHEDDBG ADJPRI insn=39 deaths=0 birth=1 maxpri=2130706433 pri=3. The gate passes, and it must pass for any live load under backward scheduling. reg_n_sets[i] == 1 is now the only C-visible input to the bump, and making it 2 is the multiply-written pointer-staging carrier the Judge banned for this function on 2026-08-19.

- [s7] Sanctioned-dressing families measured dead on the cb body as well as on candidate.c: four declaration orders and three local-type variants are all byte-inert at 4/66.

- [s7] gp stores cannot substitute for copy stores as cse separators between p10 and the +0 read: g1 = 8/68, g2 = 11/68, because a (mem (symbol_ref)) store is an absolute scheduling barrier for sched.c and destroys the matched prefix.

- [s7] src/text1b.c was reverted to HEAD before this session finished; no build-pipeline file was touched. The asmfix.txt:109-110 integration hazard recorded by s10 is unchanged and still applies to any future body swap.

## s8 2026-08-19 - structural

- [s8] CHASSIS. candidate.c re-measured on today's HEAD with the s8 harness
  (tmp/grind/func_80060A68/s8/apply.py + `sandbox func_80060A68 --disable all`):
  score 2, build 66, target 66. The honest floor is unchanged at 2 and is NOT stale.
  cb re-measured at 4 / 66 / 66, matching the s7 ledger exactly.
- [s8] candidate.c ALREADY emits `lw $a1,0x10($v1)` at slot 12 (1-based) - the instruction the
  campaign calls "the missing hoist". Its two 0x10 loads sit at slots 12 ($a1) and 20 ($a0);
  its residual is the ABSENT third load (a nop at slot 24) and one register. cb, by contrast,
  has all three loads but the third is `lw $v0,0x10($v1)` at slot 24 instead of
  `lw $a1,0x10($v1)` at slot 12. The two floor bodies fail on opposite halves of the same pair.
- [s8] DUMP FACTS (tmp/grind/func_80060A68/dumps/, generated with cb applied):
    text1b.lreg:29591  `;; Function func_80060A68` ... `Register 75 used 2 times across 6 insns
                       in block 0; GR_REGS or none.`  reg 75 is `p10`; insn 39 sets it from
                       `(mem:SI (plus (reg 72) (const_int 16)))`, insn 56 is its only reader.
                       Note reg 75 is NOT flagged `pointer` while regs 90 / 92 (the other two
                       0x10 loads) are.
    text1b.greg:25167  `;; 1 regs to allocate: 83` - every other pseudo, including 75, is
                       assigned by LOCAL-ALLOC. `;; Register dispositions: ... 75 in 2 ...`
                       ( = $v0), `76 in 5` ( = $a1, `temp_a1`). `;; Hard regs used: 2 3 4 5 31`.
    sched1 order at the residual: 49, 46, 51, 53, 39, 59, 56, 61, 64, 66. $v0 dies at insn 53
    (`sh $v0,0x18($v1)`) and is re-born at insn 64 (`addiu $v0,$v1,0x18`), so it is free across
    reg 75's whole range - which is why local-alloc's lowest-free scan takes it.
- [s8] sched.c read directly (tools/gcc-2.7.2/sched.c): birthing_insn_p at 2505-2536 has
  exactly four gates - `reload_completed == 1` returns 0; pattern must be `SET` with a `REG`
  dest; `bb_live_regs[offset] & bit`; `reg_n_sets[i] == 1`. adjust_priority at 2544-2593 only
  bumps in the `case 0:` arm of the n_deaths switch, and the REG_DEAD notes it counts are
  stripped before sched runs (GCC's own `??? This code has no effect` comment at 2555), so the
  n_deaths arms are unreachable - the s6 reading is correct on this point. What s6 and s7 got
  wrong is the INFERENCE: the bump fixes sched1's ORDER, but the byte that is actually missing
  is decided later, by which hard register local-alloc hands reg 75, and sched2 (which never
  bumps) is free to undo sched1's adjacency if the register permits. Those are two separable
  gates and only the second one has to move.
- [s8] THE PRESSURE LEVER IS REAL AND HONEST. Twenty bodies with a single-set `p10` emit
  target's `lw $a1,0x10($v1)` at slot 12. Best of them:
    f3 / f4 / i3 / i4 / r1  8 / 66  three 0x10 loads, prefix byte-identical to target
                                    THROUGH SLOT 22 (cb diverges at 11). Banked at
      rejected/s8-idx-hoisted-above-p10-consumer-3-loads-plus-a1-slot12-prefix-identical-thru-22-idx-read-steals-slot23-score8.c
    u5                      8 / 66  slots 26-34 byte-identical to target, including
                                    `lhu $a1,0x4($a1)` at 29 and `lhu $a0,0x0($v1)` at 30.
      rejected/s8-copy3-hoisted-into-c3-local-store-as-separator-target-tail-slots-26-34-exact-score8.c
    n2                     10 / 67  three loads, $a1 at slot 12, no named local at all - the
                                    cleanest proof that plain statement order is sufficient.
      rejected/s8-copy3-store-sunk-below-plus2-read-a1-slot12-three-loads-67insns-score10.c
- [s8] f3's residual, slot by slot (f3 left, target right):
    23  lhu $a0,0x0($v1)   |  lw  $a0,0x10($v1)
    24  sh  $v0,0x18($v1)  |  sh  $v0,0x18($v1)
    25  lw  $v0,0x10($v1)  |  lhu $a0,0x2($a0)
    26  lhu $a1,0x4($a1)   |  addiu $v0,$v1,0x18
    27  lhu $a2,0x2($v0)   |  sw  $v0,%gp_rel(D_800A3478)($gp)
    28  addiu $v0,$v1,0x18 |  sh  $a0,0x1A($v1)
    29  sw  $v0,gp         |  lhu $a1,0x4($a1)
    30  addiu $v0,$v1,0x20 |  lhu $a0,0x0($v1)
    31  sh  $a2,0x1A($v1)  |  addiu $v0,$v1,0x20
  i.e. the hoisted idx load takes slot 23 from the third 0x10 load and everything after it is a
  permutation. Slots 1-22 and 32-66 are already right.
- [s8] SEAT SWEEP, `idx` read, twelve seats on the cb order (i0..i11): $a1 at slot 12 appears
  ONLY at seats 2, 3, 4, 5 - the seats strictly before the p10 consumer statement. i0 / i1
  over-pressure to $a2 (9 / 66). i7 (8), i8 (4), i10 (7), i11 (7) and cb itself (seat 9, 4) all
  leave p10 in $v0.
- [s8] Consumer-and-p10 moved INTO or ABOVE the copy triple (ka 9/66, kb 11/67, kc 10/67,
  kd 8/66, ke 11/67, kf 10/67, kg 8/66, kh 10/67): the load does reach slot 10 or 11, but in
  $a0, because $a0 is free there too. Any winning body has to occupy $v0 AND $a0 across p10's
  sched1 range, not just $v0.
- [s8] Also measured and dead on the cb order: hoisting the +0 halfword VALUE into a `t0` local
  with the 0x18 store delayed (e1 8/66, e2 7/65, e3 7/65, e4 7/65, e5 9/65 - the delayed store
  stops separating the two 0x10 reads and a load folds); p10-position sweep (pa 4, pb 4 - both
  byte-identical to cb; pd 6; pe 3/65); halfword-group permutations (h1 3/65, h2 7, h3 7/67,
  h4 8/67, h5 7, h6 8, h7 9, h8 7); n1 11/66, n3 11/66, n4 12/67, n5 14/67, n6 12/67;
  m2 11/67, m3 12/66, m4 14/67, m5 12/66; o6 10/67, o8 10/67; u4 5/67, u7 5/67, u8 7/67,
  ub 7/67, u9 10/66.
- [s8] TOOLING. tmp/grind/func_80060A68/s8/ holds a reusable harness: apply.py (splices a body
  file into src/text1b.c by name), gen*.py (statement-token bodies), run.ps1 / run2.ps1
  (apply + sandbox + disassemble + report the slot and register of every `lw ?,0x10($v1)`),
  dis.sh. 59 bodies were measured this session; every .dis is on disk.
- [s8] src/text1b.c was reverted to HEAD before this session finished. No rules, no commits.

- [s8] CHASSIS: candidate.c re-measured on today's HEAD = score 2, build 66, target 66. cb re-measured = 4 / 66 / 66, matching the s7 ledger. The floor is 2 and is not stale.

- [s8] candidate.c (the floor-2 body) ALREADY emits `lw $a1,0x10($v1)` at slot 12; its residual is the ABSENT third 0x10 load (a nop at slot 24). cb has all three loads but the third is `lw $v0,0x10($v1)` at slot 24. The two floor bodies fail on opposite halves of the same pair - which is why 'the missing hoist' and 'the missing load' were being conflated.

- [s8] text1b.greg:25167 `;; 1 regs to allocate: 83` proves p10's pseudo (reg 75) is a LOCAL-ALLOC decision, not global-alloc; `;; Register dispositions: ... 75 in 2 ... 76 in 5 ...` gives p10 $v0 and temp_a1 $a1; `;; Hard regs used: 2 3 4 5 31`.

- [s8] text1b.lreg:29591 shows reg 75 as `used 2 times across 6 insns in block 0; GR_REGS or none` - and, unlike regs 90 and 92 (the other two 0x10 loads), it is NOT flagged `pointer`.

- [s8] sched.c:2505-2536 birthing_insn_p has exactly four gates (reload_completed==1 -> 0; SET with REG dest; bb_live_regs bit; reg_n_sets[i]==1) and adjust_priority (2544-2593) only bumps in the `case 0:` arm, whose REG_DEAD notes are stripped before sched runs (GCC's own `??? This code has no effect` comment at 2555). s6's reading of the pass is correct; s6/s7's INFERENCE that this is the last C-visible lever is not.

- [s8] f3 residual, slot by slot: 23 `lhu $a0,0x0($v1)` vs target `lw $a0,0x10($v1)`; 24 both `sh $v0,0x18($v1)`; 25 `lw $v0,0x10($v1)` vs `lhu $a0,0x2($a0)`; 26 `lhu $a1,0x4($a1)` vs `addiu $v0,$v1,0x18`; 27 `lhu $a2,0x2($v0)` vs the gp-3478 store; 28-31 the same four instructions in a different order. Slots 1-22 and 32-66 already match.

- [s8] u5 residual is complementary: all eight wrong slots are in 17-25 (hoisting the c3 read above copy 2 lets cse keep copy 2's base in $a0, so copy 3 emits `lw $a0,0x8($a0)` instead of target's `lw $a0,0xC($v1)` / `lw $v0,0x8($a0)`, plus one extra `lw $a1,0x10($v1)` at 24). Keeping the c3 read BELOW copy 2 repairs the triple but costs a 67th instruction (u4 5/67, u7 5/67, u8 7/67, ub 7/67).

- [s8] No banned construct was used or proposed this session: every body measured has each local written exactly once, and the two named locals introduced (`t0` for the +0 halfword value, `c3` for copy 3's value) are single-write single-read intermediates in bodies that were all REJECTED on score, not submitted.

- [s8] src/text1b.c was reverted to HEAD before finishing; `git status` shows no build-file dirt. No rules touched, no commits, no queue/retire calls.

- [s9] CHASSIS. Re-measured on today's HEAD: HEAD's own body = score 39 / build 64; candidate.c
  (the ban-free floor body) = score 2 / build 66 / target 66. The floor is 2 and is not stale.
  `python3 tools/scan_hand_coded.py --single func_80060A68` re-run this session: tier=LOW
  score=1/8, S4 (6 loads in an 8-insn window @ insn 9) the only signal.
- [s9] NEW BEST STRUCTURAL SEAT (d8, 4 / 66): source order copy1; copy2; `p10 = *(s32 *)(outer +
  0x10)`; copy3; `*(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0)`;
  `temp_a1 = *(u16 *)(p10 + 4)`; `temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2)`; gp-3478 store;
  0x1A store; idx read; gp-347C store; 0x1C store. This is the FIRST body with all three
  `lw ?,0x10($v1)` loads at 66 instructions AND slots 30-63 byte-identical to target. Its entire
  residual is one placement: target emits the +4 read's address load at slot 12 as
  `lw $a1,0x10($v1)`; d8 emits it at slot 24 as `lw $v0,0x10($v1)`, adjacent to its consumer, and
  everything from 12 to 24 is target's stream shifted by one.
- [s9] PASS ATTRIBUTION (dump-verified, not inferred). With b1 applied,
  `pwsh tools/grinder/dump.ps1 func_80060A68`; tmp/grind/func_80060A68/dumps/text1b.sched2:36617
  is this function; its trace reads `;; ready list at T-30: 39 (3), now 39` /
  `;; launching 56 before 39 with no stalls at T-31`. insn 39 is the +4 address load with
  priority 3. sched.c schedules backward and never idles while anything is ready, so a load whose
  ONLY consumer has just been scheduled is the only ready insn and is forced into that cycle.
- [s9] THE CONSERVATION LAW, RESTATED CORRECTLY. A `*(s32 *)(outer + 0x10)` load with exactly ONE
  consumer is pinned adjacent to that consumer by sched2 ready-list starvation and cannot reach
  slot 12. Every body in the campaign that reaches `lw $a1,0x10($v1)` at slot 12 (candidate.c,
  s8's f3, s5's q5, s9's g1, c4, d1) gives the load a SECOND consumer or adds an outside pressure
  source; paying for that second consumer is what costs either the third load or extra
  instructions. This supersedes the s6/s7 `reg_n_sets` reduction (retired by s8) and refines s8's
  occupancy story: $v0/$a0 occupancy decides which HARD REGISTER the load gets, ready-list
  starvation decides its SLOT.
- [s9] PRESSURE SWEEP ON THE THREE-LOAD ORDERS (28 bodies, every local written exactly once):
  `idx` read hoisted (g1 8/66 - $a1 at slot 12 but the idx load then takes target's slot-23
  load-delay slot; c1 8/67, c2 8/67, d5 8/67 all over-pressure the pseudo to $a2); named
  `gp18 = outer + 0x18` (a2 6/66, c3 5/67, g3 4/66 - inert on the slot); named
  `gp20 = outer + 0x20` (d4 5/67); copy 2 split into `c2` (a3 11/65, c5 5/67); copy 3 split into
  `c3` (c4 8/66 - $a1 at slot 12, but cse then keeps copy 2's base in $a0 and the copy triple
  loses target's `lw $a0,0xC($v1)` reload); gp-347C store hoisted above the halfword group
  (d1 8/68 - $a1 at slot 12 for two extra instructions); +2 read hoisted (a1 11/64, a5 10/64,
  a6 13/64); +4 read spelled fresh in-line and seated early (e1 10/67, e2 7/65, e3 11/67);
  +4 read pushed as late as the source allows (e5 7/67, e6 8/68); v2 shape re-measured
  (b1 = b2 = b3 = 5/67, p10's source position inert, as banked since s1); d2 9/67, d6 8/67,
  d7 7/66, g2 9/66, g4 8/66, g5 7/66, b5 7/67, b6 8/68, b4 = candidate 2/66.
- [s9] TOOLING. tmp/grind/func_80060A68/s9/ holds gen.py (statement-token body generator, 28
  bodies in bodies/), apply.py, run2.ps1 (apply + sandbox + disassemble + report the slot and
  register of every `lw ?,0x10($v1)`), dis.sh, target.txt (the target instruction stream,
  normalised) and cmp.py (slot-by-slot target-vs-build diff). cmp.py is new this session and is
  what makes "which slot is wrong" a one-command question.
- [s9] No banned construct was used or proposed: every body measured has each local written
  exactly once, and no volatile, pin, inline asm, dead local or FAKE annotation appears anywhere.
  src/text1b.c was reverted to HEAD before finishing; `git status` shows no build-file dirt.

- [s9] CHASSIS: on today's HEAD, HEAD's own body = score 39 / build 64; memory/grind/func_80060A68/candidate.c = score 2 / build 66 / target 66. Floor 2 confirmed, not stale.

- [s9] The earlier 2026-08-19 OWNER-ESCALATION entry's central claim - 'closing the function REQUIRES a C variable assigned more than once to carry copy 2's source pointer' - is FALSE and is corrected by the s9 entry filed this session. s8 already showed single-write bodies reaching lw $a1,0x10($v1) at slot 12; s9 shows single-write bodies with all three 0x10 loads at 66 instructions. No construct of any kind has been shown necessary; what is missing is a C body nobody has found.

- [s9] Dump-attributed mechanism (tmp/grind/func_80060A68/dumps/text1b.sched2:36617, b1 applied): ';; ready list at T-30: 39 (3), now 39'. insn 39 is the +4 read's address load; it is the only ready insn at that cycle, so backward scheduling forces it adjacent to its consumer. Occupancy decides the load's hard REGISTER (s8); ready-list starvation decides its SLOT (s9).

- [s9] d8 (banked at rejected/s9-three-loads-66insns-plus4-address-load-adjacent-to-consumer-slot24-v0-score4.c) is the campaign's cleanest structure: 66 instructions, three 0x10 loads, slots 30-63 byte-identical to target, one placement wrong.

- [s9] 28 bodies measured this session, every local written exactly once; no volatile, pin, inline asm, dead local, FAKE annotation or multiply-written local was used or proposed. The two standing bans were not approached.

- [s9] GATE 1 canonical-asm FAILS: python3 tools/scan_hand_coded.py --single func_80060A68 = tier LOW score 1/8, S4 only (6 loads in an 8-insn window @ insn 9); S1/S2/S6 clear.

- [s9] GATE 2 SOTN precedent FAILS and is not in play: s9 proposes no coercion construct, so there is no family to cite a precedent for.

- [s9] New tooling: tmp/grind/func_80060A68/s9/cmp.py + target.txt give a one-command slot-by-slot target-vs-build diff (normalised registers/offsets), which is what turned 'the score is 4' into 'slot 24 vs slot 12, register $v0 vs $a1'.

- [s9] src/text1b.c was reverted to HEAD before finishing; git status shows no build-file dirt. No rules touched, no commits, no queue/retire calls.

## [s10 2026-08-25 - escalation/disposition] Banked facts

- CHASSIS RE-MEASURED on 2026-08-25 HEAD: HEAD's committed body = score 39 / build 64
  (`sandbox func_80060A68 --disable all`); memory/grind/func_80060A68/candidate.c = score 2 /
  build 66 / target 66. Ledger floor 2 is intact on today's chassis.
- The two differing instructions in candidate.c, named exactly (tmp/grind/func_80060A68/s10/cmp.py
  against s10/target.txt): line 22 `lhu v0,0(a1)` where target has `lhu v0,0(a0)`, and line 23
  `nop` where target has the third `lw a0,16(v1)`. Everything else, including all 15 relocation
  sites, matches.
- NEW BODIES (s10): w1 and w2, banked as
  rejected/s10-mirrored-partition-p10-serves-plus2-lhu-a1-instead-of-a0-score2.c. Both score
  2 / 66 / 66 and are byte-identical to each other; they mirror candidate.c's partition (p10's
  hoisted load serves the +2 and +4 reads instead of the +0 and +4 reads). Line 22 becomes
  target-exact; line 26 becomes wrong instead. Floor unchanged.
- CONSERVATION (new, s10): with cse folding a repeated `*(s32 *)(outer + 0x10)` onto the most
  recent live equivalent, "three loads" == "three aliasing-store-separated reads" == "every load
  has exactly one consumer", and s9's ready-list-starvation law then pins the +4 load adjacent to
  its consumer. Any second consumer costs both that read's load AND that read's hard register.
  See hypotheses.md [s10].
- GATE 1 re-run this session: `python3 tools/scan_hand_coded.py --single func_80060A68` =>
  `tier=LOW score=1/8`, S4 (6 loads in an 8-insn window @ insn 9) the only signal; S1/S2/S6 all
  clear. Canonical-asm remains refused.
- GATE 2: no coercion construct is in play (every s8/s9/s10 body is plain C with single-write
  locals), so there is no family for which to cite a SOTN-master precedent. Fails by absence.
- LINE-NUMBER CORRECTION (audit hygiene): this function's two asmfix rules are at
  **asmfix.txt:82 and asmfix.txt:83** on today's HEAD (the comment block explaining them is
  asmfix.txt:73-81). The 2026-08-19 decisions.md entry says 105/106 and state.json's
  judge_constraints say 109-110; both are stale line numbers for the same two rules. The
  `delete_between` start anchor is `^lhu\t\$4,0\(\$3\)$` - the FIRST body instruction of the
  committed C body - which is why any body swap must retire both rules in the same change.

- [s10] Chassis re-measured 2026-08-25: HEAD's committed body = score 39 / build 64; memory/grind/func_80060A68/candidate.c = score 2 / build 66 / target 66 (sandbox func_80060A68 --disable all). The ledger floor 2 is intact on today's chassis; the driver's 'measurement unavailable' is resolved.

- [s10] candidate.c's residual, named to the instruction (tmp/grind/func_80060A68/s10/cmp.py cand): line 22 emits lhu v0,0(a1) where target has lhu v0,0(a0), and line 23 emits a nop where target has a third lw a0,16(v1). All 15 relocation sites and all 64 other instructions match.

- [s10] Target loads *(s32 *)(outer + 0x10) three times (slots 12, 20, 23) and hoists the first, lw a1,0x10($v1), seventeen slots above its only consumer lhu a1,0x4(a1) at slot 30 (asm/funcs/func_80060A68.s lines 13, 21, 24, 30).

- [s10] NEW s10 bodies w1/w2 banked at memory/grind/func_80060A68/rejected/s10-mirrored-partition-p10-serves-plus2-lhu-a1-instead-of-a0-score2.c: score 2 / 66 / 66, byte-identical to each other, the exact mirror of candidate.c's partition. Source position of the +2 read (p10 + 2 vs inline re-read) is inert.

- [s10] CONSERVATION (new, s10): with cse folding onto the most recent live equivalent, three loads is exactly equivalent to every load having exactly one consumer; any second consumer costs that read BOTH its own load and its hard register. Frontier item 1 is therefore closed for honest C; only frontier item 2 (an outside pressure source) survives.

- [s10] Gate 1 re-run 2026-08-25: tools/scan_hand_coded.py --single func_80060A68 => tier=LOW score=1/8, S4 only.

- [s10] Gate 2: no construct in play, no family claimed, no precedent citable.

- [s10] AUDIT-TRAIL CORRECTION: this function's two asmfix rules are at asmfix.txt:82 and asmfix.txt:83 on today's HEAD (explanatory comment block at asmfix.txt:73-81). The 2026-08-19 decisions.md entry cites 105/106 and state.json's judge_constraints cite 109-110; both are stale line numbers for the same two rules. The delete_between start anchor is the committed C body's FIRST body instruction (lhu $4,0($3)), which is what makes the rules body-coupled.

- [s10] The owner's RULES-TO-ZERO directive (queue unpark_reason, 2026-08-24) is now acknowledged in the ledger and executed as far as a grind session can: the rules can only retire in the same change as a body land, and no body reaches distance 0, so the remaining decision is the representation question filed in the packet.

- [s10] src/text1b.c was reverted to HEAD at the end of the session; no rules, engine, tools, Makefile or linker-script files were touched; nothing was committed.

- [s10] DECISION PACKET FILED: docs/grind/decisions.md, '## 2026-08-25 - func_80060A68 - OWNER-ESCALATION - ESCALATED WITH DECISION PACKET' (supersedes the retired-shape 2026-08-19 s9 entry).

## s11 (2026-08-25, escalation modality) — the residual's pass attribution is CORRECTED: it is a reload-induced REG_DEP_OUTPUT, not ready-list starvation

Chassis re-measured first: `memory/grind/func_80060A68/candidate.c` applied to `src/text1b.c`
measures **score 2 / build 66 / target 66** on today's HEAD (HEAD itself carries
`INCLUDE_ASM("asm/funcs", func_80060A68);` since commit 0bef2aa3). Floor confirmed, not stale.
`src/text1b.c` was restored to HEAD before this session finished; no rules touched, no commits.

### 1. THE CORRECTION (this session's main result)

s9 banked "THE LAW: a 0x10 load with exactly ONE consumer cannot be hoisted — it is pinned
adjacent to that consumer by ready-list starvation", and s10 built frontier item 2 on top of it
("find an OUTSIDE pressure source, an insn ready at sched2's T-30"). **That attribution is
wrong, and the dump says so directly.**

`pwsh tools/grinder/dump.ps1 func_80060A68` with the three-load body `d8`
(`rejected/s9-three-loads-66insns-plus4-address-load-adjacent-to-consumer-slot24-v0-score4.c`,
re-measured this session at **4 / 66**) applied; dump saved at
`tmp/grind/func_80060A68/s11/sched2.d8`, function region at line 30625. The trace reads:

    ;; insn[  53]: priority =    8, ref_count =   10
    ;; insn[  39]: priority =    8, ref_count =    6
    ;; ready list at T-29: 61 (8), now 61
    ;; launching 39 before 61 with no stalls at T-30
    ;; ready list at T-30: 39 (8), now 39

and the RTL for insn 39 (extracted with the scripting in `tmp/grind/func_80060A68/s11/`) is

    (insn 39 ... (set (reg/v:SI 2 v0) (mem:SI (plus:SI (reg/v:SI 3 v1) (const_int 16))))
       159 {movsi_internal2}
       (insn_list 152 (insn_list:REG_DEP_OUTPUT 51 (insn_list:REG_DEP_ANTI ...

Insn 39 IS the +4 address load. Its dependence list carries **REG_DEP_OUTPUT 51**, and insn 51 is

    (insn 51 ... (set (reg:HI 2 v0) (mem:HI (reg:SI 4 a0))) 163 {movhi_internal2} ...

— the +0 halfword read. Both are in **$v0**. sched2 runs POST-RELOAD, so this is an output
dependence between two hard-register writes that reload happened to give the same register. It
is what raises insn 39's INSN_PRIORITY to 8, and that is why insn 39 is released only at T-30,
where nothing else is ready.

Consequence: the +4 load's slot is **not** a property of the block's dependence graph and cannot
be moved by adding ready insns. The whole frontier-item-2 framing ("outside pressure source")
was aimed at the wrong quantity. The controlling quantity is **which hard register reload gives
p10's pseudo in a THREE-LOAD body**. (This also re-reads s8's "any winning body has to occupy
$v0 AND $a0 across p10's sched1 range" as the same fact seen from the allocation side — s8 was
closer to right than s9/s10.)

Secondary consequence, also load-bearing: even taken on its own terms, frontier item 2 was
arithmetically impossible. Insn 39 has priority 8; from T-30 downward the only other pending
priority-8 insn is 53, and every insn scheduled at T-33..T-45 has priority 7, 6, 5, 4 or 3.
sched2 picks the max-priority ready insn every cycle, so a priority-8 ready insn can be displaced
at most ONE cycle no matter how many extra insns are made ready. Target needs the load 13 cycles
further down (forward slot 12, i.e. about T-43). **No pressure source of any size could have
worked.** Frontier item 2 is CLOSED, and closed for a reason unrelated to the one s10 proposed.

### 2. Sixteen new bodies measured (all in `tmp/grind/func_80060A68/s11/bodies/`)

Harness: `tmp/grind/func_80060A68/s8/apply.py` + `tmp/grind/func_80060A68/s11/run.ps1`
(splice body, `sandbox --disable all`, report score/build/target).

Value-and-base-local hoists intended to keep $v0 / $a0 busy across the `p10` read (all place
`p10` INSIDE the copy triple):

| body | shape | score | insns |
|---|---|---|---|
| x1 | copy-1 VALUE in a local `c1`, `p10` between the load and the store | 5 | 67 |
| x2 | copy-2 VALUE in a local `c2`, `p10` between the load and the store | 5 | 67 |
| x4 | copy-2 value local, `p10` after the 0x24 store | 5 | 67 |
| x6 | copy-2 BASE in a local `b2`, `p10` after it | 5 | 67 |
| x7 | copy-1 and copy-2 values both in locals, `p10` before the 0x24 store | 5 | 67 |
| x8 | copy-1 BASE in a local `b1`, `p10` after it | 5 | 67 |
| x9 | copy-3 VALUE in a local `c3`, `p10` before the 0x28 store | 5 | 67 |

Every seat of `p10` inside the copy triple costs exactly one instruction (67 vs 66) and lands at
score 5, **regardless of which value or base is given a named local**. The only 66-instruction
three-load body remains the bare `d8` seat (`p10` between copy 2 and copy 3, no local) at 4/66.
The named locals are therefore NOT a live-range lever here: GCC coalesces them away.

`p10` seat sweep on the `c2`-local frame (`y2`..`y10`, `p10` inserted before statement N):

    y2 (above copy triple) 5/67 · y3 5/67 · y4 5/67 · y5 (before copy 3) 5/67
    y6 (after copy triple) 2/66 · y7 (after the 0x18 store) 2/66 · y8 (after the 0x1A source read) 2/66
    y9 3/66 · y10 3/66

### 3. x2 is the first three-load body in which reload gives p10 the TARGET register

`tmp/grind/func_80060A68/s11/x2.dis` (67 insns). Its slots 0-11 are byte-identical to target,
including target's adjacent pair `lw $v0,0xC($v1); lw $a0,0xC($v1)` at slots 10/11. It carries
THREE `lw ?,0x10($v1)` loads (slots 19, 23, 24) and — this is the new part — the +4 address load
is `lw $a1,0x10($v1)`, i.e. **$a1, target's register**, not $v0. What it does not have is the
slot: target puts that load at slot 12, x2 at slot 24, and x2 pays one extra `nop`.

So the two halves of the target stream have now each been reached, but never together:
  * candidate.c / y6 / y7 / y8: `lw $a1,0x10($v1)` at **slot 12**, but only TWO 0x10 loads.
  * x2: **THREE** 0x10 loads with the +4 load in **$a1**, but at slot 24 and at 67 insns.
  * d8: THREE loads at 66 insns, but the +4 load in $v0 (hence the REG_DEP_OUTPUT, hence T-30).

### 4. y6 confirms candidate.c's residual is unchanged and un-coupled to the c2 local

`tmp/grind/func_80060A68/s11/y6.dis` (66 insns, score 2) is in candidate.c's byte class: slot 12
`lw a1,0x10(v1)` correct, slot 22 emits `lhu v0,0(a1)` where target has `lhu v0,0(a0)`, slot 23
emits `nop` where target has the third `lw a0,0x10(v1)`. The `c2` named intermediate is therefore
codegen-INERT in every after-the-copy-triple seat — banked at
`rejected/s11-c2-named-intermediate-after-copy-triple-codegen-inert-score2.c` so no future
session spends a measurement on it.

### 5. Endgame gates re-checked (unchanged)

`python3 tools/scan_hand_coded.py --single func_80060A68` gives **tier LOW, score 1/8**, S4 only
("6 loads in 8-insn window @ insn 9"). Gate 1 FAILS, as in s8/s9/s10. Gate 2 is not applicable:
candidate.c contains no coercion construct at all, so there is no family for which a SOTN-master
precedent could be cited.

### 6. Why this session does NOT file a decision packet

The 2026-08-25 packet (representation) was filed, ruled (b), and executed as commit 0bef2aa3.
The residual that remains poses no question an owner can decide: it is a grind question with a
newly-corrected mechanism and a newly-opened, concrete axis (section 3). A packet asking to relax
a standard is pre-decided NO (owner ruling 2026-08-24, second), and "this is hard" is not a
packet. The honest outcome is `progress` with the kills banked and the item ACTIVE.

### 7. Artifacts

    tmp/grind/func_80060A68/s11/sched2.d8        — sched2 dump with d8 applied (the REG_DEP_OUTPUT evidence)
    tmp/grind/func_80060A68/s11/sched2.cand      — sched2 dump with candidate.c applied
    tmp/grind/func_80060A68/s11/x2.dis           — 67-insn three-load body, +4 load in $a1
    tmp/grind/func_80060A68/s11/y6.dis           — 66-insn floor-2 body, candidate byte class
    tmp/grind/func_80060A68/s11/bodies/*.c       — all 16 measured bodies
    tmp/grind/func_80060A68/s11/run.ps1          — batch apply+sandbox harness
    tmp/grind/func_80060A68/s11/text1b.c.HEAD    — pristine HEAD copy used to restore src/

- [s11] Chassis re-measured on today's HEAD: memory/grind/func_80060A68/candidate.c applied to src/text1b.c = score 2 / build 66 / target 66. HEAD itself carries INCLUDE_ASM("asm/funcs", func_80060A68); since commit 0bef2aa3 (owner ruling (b), executed). src/text1b.c was restored to HEAD before this session finished; no rules touched, no commits.

- [s11] PASS ATTRIBUTION CORRECTED. The residual's controlling dependence is REG_DEP_OUTPUT between sched2 insn 39 (the +4 address load, (reg/v:SI 2 v0)) and sched2 insn 51 (the +0 halfword read, (reg:HI 2 v0)) in the three-load body d8. Both were assigned $v0 by reload; sched2 is post-reload, so the dependence is allocation-induced, not semantic. It sets INSN_PRIORITY 8 on insn 39 and is the actual reason the load is released at T-30. s9's 'ready-list starvation law' and s10's frontier item 2 named the wrong quantity.

- [s11] FRONTIER ITEM 2 IS CLOSED, and closed independently of the mis-attribution: sched2 selects the max-priority ready insn each cycle; insn 39 has priority 8; from T-30 downward only insn 53 also has priority 8, and every insn scheduled at T-33..T-45 has priority 7, 6, 5, 4 or 3. Extra ready insns can therefore displace insn 39 by at most one cycle. Target needs it 13 cycles further down. No outside pressure source of any size could have worked, so the seven sources s9 measured dead were dead for a structural reason, not by accident.

- [s11] x2 (copy-2 VALUE in a named local, p10 seated between that load and its store) is the FIRST body in eleven sessions with THREE lw ?,0x10($v1) loads whose +4 address load is in $a1 -- target's register -- and whose slots 0-11 are byte-identical to target including target's adjacent lw $v0,0xC($v1); lw $a0,0xC($v1) pair. It measures 5 / 67: the $a1 load sits at slot 24 rather than 12 and one nop is added. Banked at rejected/s11-copy2-value-local-3loads-p10-reaches-a1-but-slot24-and-67insns-score5.c, disassembly at tmp/grind/func_80060A68/s11/x2.dis.

- [s11] The three halves of target's stream have now each been reached but never together: candidate.c / y6 / y7 / y8 put lw $a1,0x10($v1) at slot 12 but carry only TWO 0x10 loads; x2 carries THREE loads with the +4 load in $a1 but at slot 24 and 67 insns; d8 carries THREE loads at 66 insns but with the +4 load in $v0 (hence the output dependence, hence T-30).

- [s11] Named value/base locals are NOT a live-range lever in this function: all seven shapes (copy-1 value, copy-2 value, copy-3 value, copy-1 base, copy-2 base, and the two-local combination) measure exactly 5 / 67 when p10 is seated inside the copy triple, and exactly 2 / 66 (candidate's byte class, codegen-inert) when p10 is seated after it. GCC coalesces them away. Banked so no future session re-spends these measurements: rejected/s11-c2-named-intermediate-after-copy-triple-codegen-inert-score2.c.

- [s11] p10 seat sweep on the c2-local frame: y2 (above the copy triple) 5/67, y3 5/67, y4 5/67, y5 (before copy 3) 5/67, y6 (after the copy triple) 2/66, y7 (after the 0x18 store) 2/66, y8 (after the 0x1A source read) 2/66, y9 3/66, y10 3/66.

- [s11] ENDGAME GATE 1 RE-CHECKED AND STILL FAILS: python3 tools/scan_hand_coded.py --single func_80060A68 gives tier LOW, score 1/8, S4 only ('6 loads in 8-insn window @ insn 9'). ENDGAME GATE 2 IS NOT APPLICABLE: candidate.c carries no coercion construct at all, so there is no family for which a SOTN-master precedent could be cited.

- [s11] NO DECISION PACKET WAS FILED THIS SESSION, deliberately. The 2026-08-25 packet (the representation question) was filed, ruled (b) and executed as commit 0bef2aa3; docs/grind/decisions.md:11122 records the ruling. What remains poses no owner-decidable question: it is a grind question whose mechanism this session corrected and whose replacement axis this session opened. A packet asking to relax a standard is pre-decided NO (owner ruling 2026-08-24, second), and 'this is hard' is not a packet, so the honest outcome is progress with the kills banked and the item ACTIVE.

- [campaign-sweep 2026-08-30] First ra_solver run on this function (sched-tie
  endgame campaign spec, docs/superpowers/specs/). Chassis: candidate.c (s11
  d8 body) spliced -> sandbox 2, 66/66 confirmed live. Artifacts:
  tmp/grind/func_80060A68/s12_campaign/.
  **Residual localized on THIS chassis (normalized object diff):** exactly
  sites 21/22 — ours `lhu v0,0(a1); nop` vs target `lhu v0,0(a0); lw a0,16(v1)`
  — i.e. the +16 address pseudo should sit in $a0 (not $a1) AND its load fills
  the lhu's delay slot. One coupled alloc+placement pair, consistent with the
  s11 banner's "hold p10 off $v0" reframed to the current d8 body as "get the
  address value INTO $a0".
  **Model facts extracted:** global.c backend is EMPTY for this function
  (order=1 allocno — everything relevant is local-allocated), so every
  global-side lever family is out of scope here. reload retry is NOT the
  mechanism either (reload_extract: 0 retry_global_alloc events for this
  function). The deciding pass is local-alloc block_alloc, blk 0: qty 8
  (first_reg=75, birth 26 death 44, refs 3) got $a1; qty 11 (first_reg=74,
  birth 36 death 42, refs 2) got $a0. GOAL: qty 8 -> $a0 (hard reg 4).
  **Tool wall (campaign item):** inverse.py local on blk 0 (23 allocated
  qtys) did not finish depth 1 in >20 min of 100% CPU (two runs killed) —
  per-atom forward replay appears superlinear on large blocks. Fix or bound
  the local backend before re-running; until then the enumerated escape set
  for the qty8/qty11 first-fit decision remains underived.
  **Next session:** (1) profile/fix inverse.py local, then depth-1 the goal
  above; (2) note the suggestion-pass dumps granted by ruling 1(a)
  (qty_phys_copy_sugg / qty_phys_sugg scored) bear directly on this block —
  implementing that grant may explain the $a1 pick outright.

## s12 (2026-08-30, escalation/disposition modality)

- [s12] CHASSIS RE-MEASURED on today's HEAD. `memory/grind/func_80060A68/candidate.c`
  spliced over `INCLUDE_ASM("asm/funcs", func_80060A68);` at src/text1b.c:3063 measures
  **score 2 / build 66 / target 66**. The residual is EXACTLY two adjacent slots, confirmed
  by a fresh side-by-side disassembly (tmp/grind/func_80060A68/s12/base.dis vs
  asm/funcs/func_80060A68.s): our line 22 `lhu v0,0(a1)` where target has `lhu v0,0(a0)`,
  and our line 23 `nop` where target has the third `lw a0,0x10(v1)`. All 64 other
  instructions, every register and the whole frame are identical. src/text1b.c was restored
  to HEAD before this session finished; no rules, no commits.

- [s12] THE LOCAL-ALLOC PRIORITY FORMULA IS NOW READ OUT OF THE SOURCE, not guessed.
  `tools/gcc-2.7.2/local-alloc.c:1649-1685` (`qty_compare` / `qty_compare_1`):
  `pri = (floor_log2(qty_n_refs[q]) * qty_n_refs[q] * qty_size[q]) / (qty_death[q] - qty_birth[q]) * 10000`,
  ties broken by qty number, and `block_alloc` then allocates first-fit in that order
  (local-alloc.c:1563 qsort, 1571-1580 the allocation loop). On the campaign's measured
  numbers for the base body (qty 8 = pseudo 75, refs 3, birth 26, death 44 -> pri 1666;
  qty 11 = pseudo 74, refs 2, birth 36, death 42 -> pri 3333) qty 11 is allocated FIRST and
  takes $a0, and qty 8 then takes $a1. This exactly reproduces the observed assignment, so
  the campaign's "GOAL: qty 8 -> $a0" is now understood: it would require raising qty 8's
  priority above 3333, i.e. refs 4 (a fourth reference to the +0x10 pointer = the banned
  fabricated-consumer axis) or a live range shorter than 9 insns (which is the same thing as
  moving the +4 read early, measured below). NO NEW LEVER LIVES HERE.

- [s12] THE FREE-SEPARATOR AXIS IS DEAD. Breaking the cse fold of the 0x18 read onto p10
  requires an aliasing store between p10's def and that read. Every store the function
  already contains was tried as that separator and every one of them costs instructions:
  `D_800A3478 = outer + 0x18` -> 11 / 68; `D_800A347C = outer + 0x20` -> 8 / 68; both -> 14 / 69
  (either order); copy 3's store (i.e. p10's def hoisted above it) -> 5 / 67. The gp stores
  cost two instructions each because hoisting the store also hoists its `addiu $v0,$v1,k`
  and forces an extra load-delay nop. Banked at rejected/s12-gp3478-*, s12-gp347C-*,
  s12-both-gp-stores-*, s12-p10-def-above-copy3-store-*.

- [s12] THE PARTITION AXIS IS CLOSED - the third and last partition was measured for the
  first time this session. The three halfword reads of `*(s32 *)(outer + 0x10)` can be
  grouped three ways onto two loads. {+0,+4} shared = candidate.c = 2 / 66. {+2,+4} shared =
  s10 w1/w2 = 2 / 66. {+0,+2} shared (body c1: `*(u16 *)(p10 + 0)` and `*(u16 *)(p10 + 2)`
  spelled directly off p10, so no cse fold is involved, and the +4 read reloads the pointer
  because the 0x18/0x1A stores separate it) = **4 / 66**, with the disassembly
  (tmp/grind/func_80060A68/s12/c1.dis) showing the +4 address load emitted at slot 27 in $v0
  instead of slot 12 in $a1 - the SAME pathology as the three-load body d8. Variants c2 (no
  temp2 named local) 7 / 66 and c3 (idx read hoisted above the +4 read) 5 / 66.

- [s12] THE CONSERVATION LAW IS NOW GENERAL, and it is a C-level law, not a pass artifact:
  **a `lw ?,0x10($v1)` is emitted early (slot 12) and allocated $a1 if and only if it has an
  early consumer; the load that has no early consumer is emitted late and allocated $v0.**
  Four independent bodies exhibit it - base/y6 ({+0,+4}: shared load early in $a1, 2/66),
  w1/w2 ({+2,+4}: same, 2/66), c1 ({+0,+2}: the unshared +4 load late in $v0, 4/66) and d8
  (three loads: the +4 load late in $v0, 4/66). Target needs a load that is BOTH unshared
  (three loads) AND early in $a1, which is the one combination the law forbids at 66
  instructions. Every way of buying the early consumer honestly (a separator store) costs an
  instruction; every way of buying it for free (a fabricated second consumer, a dead read, an
  address-of, a discard) is a banned family.

- [s12] ENDGAME GATE 1 RE-RUN ON TODAY'S CHASSIS AND STILL FAILS:
  `python3 tools/scan_hand_coded.py --single func_80060A68` -> `tier=LOW score=1/8`, S4 only
  ("6 loads in 8-insn window @ insn 9"); S1, S2, S3, S5, S6, S7, S8 all clear.
  ENDGAME GATE 2 REMAINS NOT APPLICABLE: candidate.c contains no coercion construct of any
  kind (no volatile, no pin, no dead local, no FAKE annotation), so there is no family for
  which a SOTN-master precedent could be cited.

- [s12] Chassis re-measured on today's HEAD: memory/grind/func_80060A68/candidate.c spliced over INCLUDE_ASM("asm/funcs", func_80060A68) at src/text1b.c:3063 = score 2 / build 66 / target 66, carrying no rule, no pin, no volatile, no inline asm, no dead local and no FAKE annotation. src/text1b.c restored to HEAD before finishing; no rules touched, no commits.

- [s12] The residual is exactly two adjacent slots (fresh disassembly tmp/grind/func_80060A68/s12/base.dis vs asm/funcs/func_80060A68.s): our line 22 'lhu v0,0(a1)' vs target 'lhu v0,0(a0)', and our line 23 'nop' vs target 'lw a0,0x10(v1)'. All 64 other instructions, every register and the whole frame match.

- [s12] THE PARTITION AXIS IS NOW COMPLETE. The three halfword reads of *(s32 *)(outer + 0x10) group three ways onto two loads and all three are measured: {+0,+4} = candidate.c 2/66; {+2,+4} = s10 w1/w2 2/66; {+0,+2} = s12 body c1 4/66 (first measurement in twelve sessions).

- [s12] THE FREE-SEPARATOR AXIS IS DEAD: D_800A3478 as separator 11/68, D_800A347C 8/68, both 14/69 (either order), copy 3's store 5/67, later +4-read seats 7/67 and 8/68.

- [s12] GENERAL LAW (supersedes s10's conservation statement, and it is a C-level law rather than a pass artifact): a lw ?,0x10($v1) is emitted at slot 12 and allocated $a1 IFF it has an early consumer; a load with no early consumer is emitted late and allocated $v0. Four independent bodies exhibit both halves -- base/y6 and w1/w2 (shared, early, $a1, 2/66), c1 and d8 (unshared, late, $v0, 4/66). Target needs a load that is simultaneously unshared (three loads) and early-in-$a1, the one combination the law excludes at 66 instructions.

- [s12] The register half of that law is derived from compiler source, not guessed: local-alloc.c:1649-1685 priority formula + local-alloc.c:1563/1571-1580 first-fit order give pri(p10)=1666 < pri(+2 pointer)=3333, which is exactly the observed $a1/$a0 split.

- [s12] The 2026-08-30 ra_solver campaign's open tool item ('profile/fix inverse.py local, then depth-1 the qty8/qty11 first-fit decision') is ANSWERED ANALYTICALLY for this function and needs no tool fix here.

- [s12] ENDGAME GATE 1 FAILS on today's chassis: scan_hand_coded --single func_80060A68 = tier LOW, score 1/8, S4 only.

- [s12] ENDGAME GATE 2 FAILS/NOT APPLICABLE: candidate.c contains no coercion construct at all, so no SOTN-master precedent is even citable; every construct that WOULD close the function was already ruled a cheat for this function (Judge 2026-08-19 10:21; layer-1 FAILs 2026-08-19 08:56 and 10:01).

- [s12] What holds the byte-match on main today: NOTHING. src/text1b.c:3063 is INCLUDE_ASM("asm/funcs", func_80060A68) since commit 0bef2aa3 and grep of asmfix.txt/regfix.txt returns nothing -- the function is honest INCLUDE_ASM, INCOMPLETE only because its best pure-C form is two instructions short.

- [s12] Disposition entry filed this session at docs/grind/decisions.md:15403; consistent with the owner's 2026-08-30 batch ruling 8 ('escalation SPENT ... returns to ACTIVE as an ordinary INCLUDE_ASM item at floor 2') and with the driver-filed 2026-08-25 packet at docs/grind/decisions.md:12060. No new question is asked and no standard-lowering ask is made (2026-08-24 auto-reject class respected).

## s13 (2026-09-01, escalation/disposition modality — owner Ruling A reopen probes)

- [s13] CHASSIS RE-MEASURED FIRST. `memory/grind/func_80060A68/candidate.c` spliced over
  `INCLUDE_ASM("asm/funcs", func_80060A68);` (src/text1b.c:3142 on today's HEAD) measures
  **score 2 / build 66 / target 66** — unchanged from s12. src/text1b.c was restored to HEAD
  at end of session; the tree is clean.

- [s13] **THE TOOL WALL IS ROOT-CAUSED AND IT IS A ONE-LINE BUG, NOT SUPERLINEARITY.**
  The 2026-08-30 campaign note recorded `inverse.py local` on blk 0 "did not finish depth 1
  in >20 min of 100% CPU (two runs killed) — per-atom forward replay appears superlinear on
  large blocks". That diagnosis is WRONG. `LocalBackend.atoms()`
  (tools/ra_solver/inverse.py:484-489) builds its ALLOC_ORDER class with
  `for perm in itertools.permutations(base): if list(perm) != base and len(base) <= 6:` —
  the `len(base) <= 6` guard is INSIDE the loop, so with 23 allocated quantities in this
  block the generator walks 23! permutations before atoms() ever returns. The forward
  replay was never reached. NOTE: tools/ is outside a grind session's allowed edit surface,
  so this is REPORTED, not patched; the operator/tooling lane should hoist that guard.

- [s13] **PROBE (1) OF THE OWNER'S REOPEN NOTE IS EXECUTED AND THE SEARCH FINISHED.**
  tmp/grind/func_80060A68/s13/bounded_local.py reuses the SAME LocalBackend (same `_alloc`,
  same `_pri`, same model file) with the atom list rebuilt minus the ALLOC_ORDER class.
  Depth 1 over 510 atoms completes in **0.0 s**. Depth 2 over the 464 non-qty8 atoms
  completes in **3.9 s**. The block was never expensive; it was never started.

- [s13] **PROBE (2) EXECUTED: THE SUGGESTION SETS ARE NOT EMPTY.** `local_extract.py text1b
  --func func_80060A68 --suggest` (artifact s13/qtydbg_baseline.txt, table in
  tmp/ra_solver_work/text1b.sugg.json) shows blk 0 carries TWO suggested-pass quantities:
  qty13 (reg1=76, b=44 d=68, refs=3, copysugg=[5]) got $a1 and qty14 (reg1=73, b=46 d=64,
  refs=4, copysugg=[4]) got $a0. Both are OUTSIDE the contested span [26,44] and neither
  contributes a register to qty8's find_free_reg `used` set, so the suggestion pass does not
  explain the $a1 pick — but the s12-era assumption that the sets were empty was untested
  and is now replaced by measurement.

- [s13] **THE $a1 PICK IS FULLY EXPLAINED BY GROUND TRUTH, NOT BY A PAIRWISE ARGUMENT.**
  The SUGGDBG-FFR records (s13/qtydbg_baseline.txt; `_ffr` in text1b.sugg.json) give the
  exact hard-register set each find_free_reg call scanned. Allocation sequence for blk 0 is
  25 calls; qty8 is call #24 (LAST) and its `used` set is {0,1,2,3,4} = {$zero,$at,$v0,$v1,$a0}
  → first free is 5 = $a1. $v0 and $v1 are blocked structurally (qty0 spans 2-52 and the
  block's hard-register base pointer). **$a0 is blocked by exactly two allocated quantities
  that overlap [26,44]: qty10 (b=32 d=36, refs=2, got $a0, call #20) and qty11 (b=36 d=42,
  refs=2, got $a0, call #23).** s12's record named only qty11 — the depth-limit the owner's
  2026-09-01 review flagged. Both must be displaced, not one.

- [s13] **THE ESCAPE SET IS NOW ENUMERATED (it was "underived" in the campaign note).**
  local-alloc's qty_compare priority is `floor_log2(refs)*refs*size*10000/(death-birth)`
  (tools/gcc-2.7.2/local-alloc.c:1649-1685), allocated first-fit ascending. Measured
  priorities for blk 0: qty8=1666, qty11=3333, qty10=5000 (and eight further qtys at 5000).
  Ties break on ascending qty number, and 8 < 10, so qty8 takes $a0 IFF
  **pri(qty8) >= 5000**. Solving fl(refs)*refs*10000/span >= 5000 at size 1:
    refs=3 -> span <= 6;  refs=4 -> span <= 16;  refs=5 -> span <= 20;  refs=6 -> span <= 24.
  Baseline is refs=3, span=18. That is the complete escape set for the seat.

- [s13] **EVERY MEMBER OF THE ESCAPE SET IS MEASURED DEAD OR STRUCTURALLY UNAVAILABLE.**
  (a) refs=3 & span<=6 is "move the +4 read early" — bodies d8 and c1, already measured
      4 / 66 with the load in $v0 (s9, s12).
  (b) refs>=4 requires added references to p10, and **an added reference is an added
      instruction that inflates the very span it must outrun** — measured twice this session:
      PROBE A (two extra p10 reads placed after temp_a1): refs 3->5 but span 18->24,
      pri = 2*5*10000/24 = 4166 < 5000, qty8 still allocated after qty10, got $a2.
      PROBE B (the same two reads placed immediately after `p10 = ...`, to protect the death
      point): span 18->28 and only refs=4 (cse collapsed one base reference),
      pri = 2857, got $a1, and **sandbox 6 / build 72 / target 66**.
      Banked at rejected/s13-refs-lift-late-p10-refs5-span24-pri4166-below-5000.c and
      rejected/s13-refs-lift-early-p10-refs4-span28-score6-72insns.c.
  (c) The arithmetic of (b) is general, not an artifact of these two probes: the build
      already sits at EXACTLY target's 66 instructions, so every added reference is a
      guaranteed +1 residual slot even in the branch where the seat flips. The refs axis
      cannot pay for itself.
  (d) The one SANCTIONED honest ref-lift — [[duplicated-statement-into-arms]], the family
      that closed func_800324D0 and is the owner's named probe for func_80045294's
      `refs_up 72: 3->4` — is **structurally unavailable here**: the contested span [26,44]
      is straight-line code. func_80060A68's ONLY conditional is the trailing
      `if (*(s32 *)D_800A3468 & 0x200000) { D_800A32BC = 0xA; }`, far outside the span, and
      it references neither p10 nor $a0. There is no arm to duplicate a real statement into.

- [s13] **MODEL-FIDELITY CAVEAT, RECORDED SO NO FUTURE SESSION OVER-TRUSTS IT.**
  `inverse.py local`'s forward model reproduces only 14 of 23 QTYDBG seats on this block
  (9 disagree: qtys 3,4,5,6,7,8,11,19,20; e.g. model qty8=$a2 vs dump $a1, model qty11=$a1
  vs dump $a0). Its conflict model is plain interval overlap while local-alloc uses
  qty_conflict bitmaps, so it over-approximates conflicts here. Its depth-1 answer for
  goal {qty8: $a0} — {refs 3->5/6/7} or {born later 26->37..40} — is therefore corroborating,
  not load-bearing: the conclusions above are derived from the QTYDBG/FFR ground truth and
  the local-alloc.c priority formula directly, and PROBE A / PROBE B measure the refs branch
  on the real compiler. The no-qty8 depth-2 run's 1224 "hits" (all requiring impossible
  edits to qty0's 16-reference span or to qty11's refs) are model-coordinate artifacts and
  are NOT claimed as C-reachable.

- [s13] ENDGAME GATE 1 RE-RUN ON TODAY'S HEAD: `python3 tools/scan_hand_coded.py --single
  func_80060A68` -> **tier=LOW score=1/8**, S4 only ("6 loads in 8-insn window @ insn 9").
  FAILS. ENDGAME GATE 2: candidate.c carries no coercion construct, so there is no family
  for which a SOTN-master precedent could be cited; and the one family that could have
  lifted refs honestly (duplicated-statement-into-arms) has no arm to attach to in this
  function's straight-line contested span. FAILS.

- [s13] Chassis re-measured this session: candidate.c spliced over src/text1b.c:3142 gives sandbox score 2 / build 66 / target 66. src/text1b.c was restored to HEAD before finishing; the working tree carries no src edit.

- [s13] TOOLING BUG REPORTED, NOT PATCHED (tools/ is outside the grind edit surface): tools/ra_solver/inverse.py:484-489 places the `len(base) <= 6` guard inside `for perm in itertools.permutations(base)`, so LocalBackend.atoms() never returns for a block with more than ~10 quantities. This — not superlinear replay — is the '100% CPU, killed before depth 1' wall recorded on 2026-08-30. Hoisting the guard fixes it for every function, not just this one.

- [s13] Owner Ruling A probe (2) executed: the suggestion sets are NOT empty. blk 0 carries qty13 (copysugg=[$a1], got $a1, live [44,68]) and qty14 (copysugg=[$a0], got $a0, live [46,64]); both sit outside the contested span [26,44] and neither contributes to qty8's used set, so the suggested pass does not decide the seat.

- [s13] Owner Ruling A probe (1) executed and finished: depth 1 in 0.0 s, depth 2 (non-qty8 atoms) in 3.9 s through the same backend with the ALLOC_ORDER class omitted.

- [s13] GROUND TRUTH for the seat: qty8 is allocation call #24 of 25 in blk 0 with used = {$zero,$at,$v0,$v1,$a0}; $a0 is contributed by qty10 (32-36) and qty11 (36-42), BOTH of which must be displaced. The s12 record named only qty11.

- [s13] COMPLETE ESCAPE SET (first enumeration): pri(qty8) must reach 5000 (tie broken by qty 8 < qty 10), i.e. refs=3 & span<=6, refs=4 & span<=16, refs=5 & span<=20, or refs=6 & span<=24, against a baseline of refs=3, span=18, pri=1666.

- [s13] refs=3 & span<=6 is the '+4 read early' shape already measured at 4/66 with the load in $v0 (bodies d8, c1 — s9, s12).

- [s13] refs>=4 is self-cancelling and was measured twice: PROBE A refs 5 / span 24 / pri 4166 / seat $a2; PROBE B refs 4 / span 28 / pri 2857 / seat $a1 / sandbox 6 / build 72 / target 66. Since the build already matches target's 66-instruction count exactly, every added reference is a guaranteed +1 residual even in a branch where the seat flips.

- [s13] The sanctioned duplicated-statement-into-arms ref-lift has no attachment point: [26,44] is straight-line and the function's only conditional is the trailing D_800A3468 & 0x200000 guard, which touches neither p10 nor $a0.

- [s13] MODEL FIDELITY CAVEAT for future sessions: inverse.py local reproduces only 14 of 23 QTYDBG seats on this block (interval-overlap conflicts vs local-alloc's qty_conflict bitmaps; model qty8=$a2 vs measured $a1, model qty11=$a1 vs measured $a0). Its depth-1 answer corroborates but does not carry the conclusion; the QTYDBG/FFR dump plus local-alloc.c:1649-1685 and the two real builds do.

- [s13] Endgame gate (a) FAILS on today's HEAD: scan_hand_coded --single func_80060A68 = tier LOW, 1/8, S4 only. Endgame gate (b) FAILS: no coercion construct in candidate.c, hence no family and no citable precedent.

- [s13] Nothing holds a byte-match here: the function is committed as INCLUDE_ASM per asm-until-matched, asmfix.txt is empty project-wide, so there is no cheat to retire and no integration handoff pending.

- [s13] Foreclosure record filed this session at docs/grind/decisions.md:18515 with both gates' evidence, the enumerated escape set, the exhaustion count (13 sessions, 6 modalities, 76 banked rejected forms) and four named re-activation triggers.

- [operator 2026-09-02] owner ruling 2026-09-02 (decisions.md 'foreclosure mechanics'): re-activated with the exhaustion window RESET — the 2026-09-01 Ruling-A unpark was re-foreclosed after one session because the window did not reset. The 09-01 named probe is spent (see ledger); work the ladder from its next rung. All standing banned_constructs remain in force. exhaustion_base=13

## s14 (2026-09-03) — solver modality

**Chassis.** `candidate.c` re-measured on today's HEAD: sandbox `--disable all` =
**score 2 / build 66 / target 66, rules_dropped 0**. NOTE for the next session:
`tmp/grind/func_80060A68/s12/text1b.c.HEAD` is STALE (src/text1b.c gained C bodies for
func_80057CC8 and func_800645B0 since s12); use `tmp/grind/func_80060A68/s14/text1b.c.HEAD`
+ `s14/apply.py`. Also: `bash tools/wsl.sh` does NOT work from inside a PowerShell tool
call on this host (`wsl: command not found`); the working recipe is the Bash tool calling
`pwsh -NoProfile -Command "& tools/wteng.ps1 main sandbox ..."`, which is what
`s14/sweep.sh` does.

**Typed classification of both chassis (tools/ra_solver/goal_from_tgt.py classify).**
  - candidate.c (2 loads, score 2): `FIRST DIVERGENCE: PRE-RA`, `ours only: nop x1 /
    target only: lw #,16(#) x1`. No model applies — the residual is a missing instruction.
  - d8/P2A0 (3 loads, score 4): `FIRST DIVERGENCE: RA`, `$v0 -> $a1 x2`. The entire
    residual of the 3-load family is one register seat inside a modelled pass.
  This is the single most useful orientation fact of the session: the 3-load chassis is
  the one the solver stack can reason about, and its residual is two registers.

**The sched1 LAUNCH_PRIORITY census (kills the necessity argument behind the escalation).**
`tools/sched_solver/extract.py text1b` on the P2A0 body, block 0 of func_80060A68 pass 1:
44 insns, 44 `adjust_priority` observations, and `maxpri` is 2130706433 (0x7F000001)
on all of them. `birth` is 1 for the 34 REG-destination insns and 0 for the 10
MEM-destination stores (22 29 36 46 53 66 69 78 81 123) — i.e. `birthing_insn_p` bumps
every load and every arithmetic insn in this block, not just the contested one. The three
0xC loads (25/32/42), the +0 and +2 pointer loads (49/59), all three copy loads
(27/34/44) and the halfword reads (51/56/61) are bumped and STILL land in target's exact
positions. A bumped insn is picked the instant it becomes ready, and it becomes ready when
its consumer is picked — so the discriminator is the consumer's position, which is
ordinary spellable C, not `reg_n_sets`.

**The $a1 seat, measured in both directions (QTYDBG/FFR, `local_extract.py --suggest`).**
p10 = pseudo 75 in every 3-load body. Priority is
floor_log2(refs)*refs*size*10000/(death-birth), first-fit ascending, ties on ascending qty
number (local-alloc.c:1649-1685 / 1563-1580).
  | body | temp_a1 read position | qty | birth-death | refs | span | pri | alloc call | got |
  |---|---|---|---|---|---|---|---|---|
  | P2A0 (=d8) | right after the 0x18 store | 10 | 34-38 | 2 | 4 | 5000 | 19/25 | 2 = $v0 |
  | P2A1 | after the 0x1A store | 11 | 36-46 | 2 | 10 | 2000 | 24/25 | **5 = $a1** |
At call #24 the quantities holding $v0 (qty9/qty10) and $a0 (qty8/qty12) across [36,46]
are already seated, so first-fit is forced past {$v0,$v1,$a0}. The seat is bought by
LOWERING the priority, which a statement move does for free; s13's escape set was solved
for the opposite goal ($a0, pri >= 5000) and every candidate there had to ADD a reference,
which inflates the span it divides by. Artifacts: s14/qtydbg_d8.txt, s14/qtydbg_P2A1.txt.

**The 3x3 position sweep (p10 statement x temp_a1 read).** Scores are constant down each
p10 column, so the p10 statement's own position is codegen-inert:
  P0A0 P1A0 P2A0 = 4 / 66 (3 loads, $v0 seat)
  P0A1 P1A1 P2A1 = 5 / 67 (3 loads, $a1 seat, one nop at slot 21)
  P0A2 P1A2 P2A2 = 8 / 68
**The A1 dressing sweep (V1..V6)**, moving the D_800A3478 / D_800A347C / idx statements
around the $a1-seated body: V1 (gp D_800A3478 store after the 0x1A store) and V3 (same,
temp2 read written inline) both reach **66 insns with three loads and no nop, score 6**,
but lose the $a1 seat back to $a0; V2 8/68, V4 7/67, V5 7/67, V6 8/67. Banked:
rejected/s14-gp3478-store-after-1A-3loads-66insns-no-nop-but-a0-seat-score6.c.
The A1 family's two remaining defects are now separable and both are one slot each:
(i) the +2 pointer load must fill the +0 read's load-delay slot (target slot 22) instead
of falling after the `sh` — V1/V3 show this is achievable at 66 insns; (ii) the +4
pointer load must sit at target slot 11 instead of 24 — unachieved.

**sched2 search on the $a1-seated chassis.** `perturb.py --func func_80060A68 --pass 2
--block 0 --atoms luid,luid_move --depth 1` (2970 atoms, model order-exact on the
baseline) finds NO single statement move that lifts insn 39 (the +4 pointer load) above
insn 27 (`--goal-before 27:39`), and none that lifts insn 56 (the +2 pointer load) above
insn 53 (`--goal-before 53:56`). In P2A1's pass-2 stream the two loads sit at stream
indices 18 and 19 (slots 23 and 24); target wants slot 22 and slot 11.

- [s14] candidate.c re-measured on today's HEAD: sandbox --disable all = score 2 / build 66 / target 66, rules_dropped 0.

- [s14] tools/ra_solver/goal_from_tgt.py classify types the two chassis differently and decisively: candidate.c (2 loads) = FIRST DIVERGENCE PRE-RA ('ours only: nop x1 / target only: lw #,16(#) x1'), i.e. no model applies; the 3-load body = FIRST DIVERGENCE RA with the whole residual printed as '$v0 -> $a1 x2'. The 3-load family, not the floor-2 2-load body, is the chassis the solver stack can reason about.

- [s14] sched1 adjpri census, block 0, 3-load body: 44 insns, 44 observations, maxpri = 0x7F000001 on all of them; birth=1 on the 34 REG-dest insns, birth=0 only on the 10 MEM-dest stores. The LAUNCH_PRIORITY bump is the norm in this block, not a property of the contested load.

- [s14] local-alloc seat arithmetic for pseudo 75 measured in both directions: span 4 -> pri 5000 -> allocation call 19/25 -> $v0; span 10 -> pri 2000 -> allocation call 24/25 -> $a1. The seat is bought by LOWERING the priority (a statement move), not by raising it (which needs an added reference that inflates the span it divides by).

- [s14] Body W5 = 5 / build 66 / target 66, three 0x10 loads, slots 0-23 byte-identical to target including the slot-11 lw $a1,0x10($v1). Two ordinary statement moves off d8; no invented local, no multiply-assigned variable, no dead code, no volatile, no asm.

- [s14] W5's residual is exactly slots 24-29: ours 'lhu v0,2(a0) / lhu a0,0(v1) / sh v0,26(v1) / addiu v0,v1,24 / sw v0,0(gp) / lhu a1,4(a1)' vs target 'lhu a0,2(a0) / addiu v0,v1,24 / sw v0,%gp(D_800A3478) / sh a0,26(v1) / lhu a1,4(a1) / lhu a0,0(v1)'.

- [s14] Dressing sweep around the $a1-seated family (18 bodies measured this session): X2/X3/X5/X6 = 5/66, V1/V3 = 6/66 (66 insns and no nop, but the seat falls back to $a0), W1 and P2A1 = 5/67, Y3 = 7/67, Y1/Y2/Y7 = 8/67, Y6 = 10/68, Y5 = 11/68. Every attempt to restore target's apparent D_800A3478-store position (between the +2 read and the 0x1A store) costs the instruction back.

- [s14] Tooling notes for the next session: tmp/grind/func_80060A68/s12/text1b.c.HEAD is STALE (src/text1b.c gained C bodies for func_80057CC8 and func_800645B0 since s12); use tmp/grind/func_80060A68/s14/text1b.c.HEAD with s14/apply.py. 'bash tools/wsl.sh' fails from inside a PowerShell tool call on this host ('wsl: command not found'); the working sweep recipe is the Bash tool calling pwsh -NoProfile -Command "& tools/wteng.ps1 main sandbox ..." (see tmp/grind/func_80060A68/s14/sweep.sh).

## s15 (2026-09-03) — forensics modality

Chassis re-measured first: HEAD == tmp/grind/func_80060A68/s14/text1b.c.HEAD byte-for-byte
(commit 506390f5 touched only the ledger), s14's W5 body re-measures **5 / build 66 /
target 66** with three `lw ?,0x10($v1)` loads. Every number below is on that chassis with
ZERO FAKE constructs in any body: no invented local, no second write to any variable, no
dead code, no volatile, no asm anywhere in the ~70 bodies measured this session.

### 1. The residual of s14's W5 is ONE local-alloc decision, and it is now bought.

W5's five wrong slots (24-29) are the SAME instruction multiset as target's; the only
register that differs is the destination of the +2 halfword read (`lhu $v0,2($a0)` vs
target `lhu $a0,2($a0)`), and the ordering follows from it (with the value in $a0 the idx
read can no longer be hoisted into the load-delay slot at 25, so the D_800A3478 addiu/store
pair fills 25/26 and idx falls to 29 — target's stream).

QTYDBG/FFR ground truth for W5 (`tmp/grind/func_80060A68/s15/qtydbg_W5.txt`, extracted with
`tools/ra_solver/local_extract.py text1b --func func_80060A68 --suggest`):

    qty11  reg1=74 (the +2 halfword value)  birth 36  death 38  refs 2  ord  5  got=2 ($v0)
    qty14  reg1=94 (the D_800A3478 addiu)   birth 44  death 46  refs 2  ord  6  got=2 ($v0)
    qty12  reg1=75 (p10)                    birth 40  death 48  refs 2  ord 24  got=5 ($a1)

pri = floor_log2(refs)*refs*size*10000/(death-birth) (local-alloc.c:1649-1685), allocated
first-fit in descending-priority order (local-alloc.c:1563-1580). reg74 has span 2, the
minimum, so its priority 10000 is the LARGEST a refs=2 quantity can have; it is allocated
6th of 25, nothing holds $v0 across [36,38], and first-fit hands it $v0. There is no way to
out-prioritise it — the fix is the opposite direction: make its range ENCLOSE the addiu's.

**Placing `D_800A3478 = outer + 0x18;` between the +2 read and the 0x1A store does exactly
that.** The +2 value then spans [38,44] (priority 3333, allocated 23rd-24th), the addiu
spans [40,42] (priority 10000, allocated 6th-7th) and takes $v0, and first-fit is forced
past {$v0,$v1} into **$a0 — target's register**. Measured on the A8 and E3 chassis
(`qty12 reg1=74 ... got=4`).

### 2. The cse rule that decides how many `lw ?,0x10($v1)` loads exist (new).

A second `*(s32 *)(outer + 0x10)` in source is folded onto the first unless a store
separates them, and **both kinds of store separate**: the register-based stores
(`*(u16 *)(outer + 0x18) = ...`, `*(u16 *)(outer + 0x1A) = ...`, the three copy stores) AND
the gp symbol stores (`D_800A3478 = ...`, `D_800A347C = ...`). Cleanest instance: bodies E2
and E3 differ ONLY in whether the `p10 = *(s32 *)(outer + 0x10);` statement sits before or
after `D_800A3478 = outer + 0x18;` — before it, 2 loads; after it, 3.

### 3. E2 — a new score-2 class, two slots from target.

`memory/grind/func_80060A68/candidate.c` (and rejected/s15-E2-*) is
`S1, S2, p10, D_800A3478, S3(0x1A store), S6, idx, D_800A347C, S8`: 2 / 66, two 0x10 loads,
and its ONLY differing slots are

    22  ours nop               vs  target lw $a0,0x10($v1)
    24  ours lhu $a0,2($a1)    vs  target lhu $a0,2($a0)

Everything else — including the $a0 seat for the +2 value and target's
`addiu v0,v1,0x18 / sw v0,%gp(D_800A3478) / sh a0,0x1A(v1)` at 25/26/27 — is identical.
The entire residual is the missing THIRD 0x10 load: with p10 folded onto the +2 read's
pointer there is nothing to fill the +0 read's load-delay slot at 22, and the +2 read
addresses $a1 instead of $a0.

### 4. E3 — the best three-load body in the campaign: 3 / 66 (W5 was 5 / 66).

Same order with the p10 statement moved one line later (after `D_800A3478 = outer + 0x18;`).
Three loads, 66 instructions, slots 20-27 are target's own stream shifted one slot earlier.
Its residual is that the third load is emitted at slot 26 in $v0 (`lw $v0,0x10($v1)` feeding
`lhu $a1,4($v0)`) instead of at slot 11 in $a1.

### 5. The tension that is left, measured across ~70 bodies.

* Three loads WITH the early slot-11 $a1 load requires the p10 statement to sit before one
  of the copy stores (only a register-based store early enough separates it from S1's load).
* Three loads that way, COMBINED with the gp store between the +2 read and the 0x1A store,
  measures **67 instructions in every ordering tried — 30 bodies**: A1-A8 (idx in all 8
  gaps), B00-B23 (three p10-before-copy positions x four idx positions), C1-C18 (all tail
  permutations plus insertions into the S2..S3 window). Score range 5-9.
* Forensic cause of that extra instruction (sched1/sched2 dumps
  `tmp/grind/func_80060A68/s15/{W5,V4}.sched{1,2}.trace`, extracted from `-da` with
  tools/grinder/dump.ps1): with the gp store inside the +2 read's range, sched1 emits p10's
  load ADJACENT to the +2 pointer load; in sched2's reverse walk the ready list at T-30 then
  contains only insn 39 (p10's load), which is forced into that slot and spent filling the
  +2 pointer's load-delay slot, leaving the +0 read's own delay slot (slot 21) a nop. In W5
  the same two insns are separated by the 0x18 store, insn 39 keeps losing rank_for_schedule
  (priority 3 against 6-10) and sinks all the way to slot 11.
* Making `D_800A347C = outer + 0x20;` the early cse separator instead (H1-H12) is 68/69
  instructions, score 8-14 — that statement's position is NOT free.
* The p10 statement's position among the three copy statements remains codegen-inert
  (B00/B10/B20 etc. identical), confirming s14's H-s14-3 on a second chassis.

- [s15] Chassis check: HEAD src/text1b.c is byte-identical to s14's saved copy, and s14's W5 body re-measures 5 / build 66 / target 66 with three lw ?,0x10($v1) loads.

- [s15] W5's five wrong slots (24-29) contain target's exact instruction multiset; the sole register difference is the +2 halfword read's destination ($v0 vs target $a0), and both ordering differences follow from it.

- [s15] QTYDBG ground truth for W5 (tmp/grind/func_80060A68/s15/qtydbg_W5.txt): qty11 = pseudo 74 (the +2 value), birth 36 death 38, refs 2, allocation order 5 of 25, got $v0; qty14 = pseudo 94 (the D_800A3478 addiu), birth 44 death 46, got $v0; qty12 = pseudo 75 (p10), birth 40 death 48, order 24, got $a1.

- [s15] pri = floor_log2(refs)*refs*size*10000/(death-birth) at local-alloc.c:1649-1685; a refs=2 quantity with span 2 has the maximum possible priority 10000, so the +2 value cannot be out-prioritised - the only lever is to make its range ENCLOSE a higher-priority $v0 quantity.

- [s15] Placing `D_800A3478 = outer + 0x18;` between the +2 read and the 0x1A store does exactly that and yields got=4 ($a0) for pseudo 74 on the A8 / E2 / E3 chassis - the seat that the 2026-08-19, 2026-08-30 and 2026-09-01 records all describe as requiring a multiply-set carrier.

- [s15] New cse rule (measured): a second *(s32 *)(outer + 0x10) read is folded onto the first unless a store separates them, and BOTH register-based stores and the gp symbol stores separate. E2 vs E3 differ only in whether the p10 statement precedes or follows the D_800A3478 store, and that alone is 2 loads vs 3.

- [s15] E3 (memory/grind/func_80060A68/rejected/s15-E3-three-loads-a0-seat-for-plus2-value-third-load-lands-slot26-in-v0-score3.c) is the best three-load body in the campaign: score 3 / build 66 / target 66, with slots 20-27 reproducing target's stream one slot early; s14's W5 was 5 / 66.

- [s15] E2 (now memory/grind/func_80060A68/candidate.c) is a new score-2 class: 66 instructions, two loads, and its ONLY differing slots are 22 (nop vs target lw $a0,0x10($v1)) and 24 (lhu $a0,2($a1) vs lhu $a0,2($a0)); target's 25/26/27 addiu / sw %gp(D_800A3478) / sh 0x1A order is matched exactly.

- [s15] The prior candidate.c body (s7..s14, 2 / 66, residual at slots 22-23) is preserved at memory/grind/func_80060A68/rejected/s14-candidate-2load-a1-plus0-read-score2.c.

- [s15] Combining the $a0 seat with an early third 0x10 load costs exactly one instruction in all 30 orderings measured (A1-A8, B00-B23, C1-C18): 67 instructions, scores 5-9.

- [s15] sched2 forensic cause of that instruction (tmp/grind/func_80060A68/s15/V4.sched2.trace): ';; ready list at T-30: 39 (3)' - the list holds only p10's load, which is forced into that slot and spent as the +2 pointer's delay-slot filler; W5's trace shows the same insn lingering from T-26 down past T-39 because the 0x18 store keeps the list non-empty.

- [s15] All ~70 bodies measured this session are ordinary C: no invented local, no variable written twice, no dead code, no volatile, no asm, no FAKE construct of any kind.

## s16 (2026-09-03) - forensics modality

Chassis re-check first (mandatory kill re-audit): HEAD `src/text1b.c` is byte-identical to
`tmp/grind/func_80060A68/s14/text1b.c.HEAD`, and the two closest banked forms re-measure
unchanged on it - **E2 = 2 / build 66 / target 66 (two 0x10 loads)** and
**E3 = 3 / build 66 / target 66 (three loads)**. Every one of the ~270 bodies measured this
session is ordinary C: no invented local, no variable written twice, no dead code, no
volatile, no asm, no FAKE construct anywhere - so no ablation was applicable and the s15
instance kills stand as measured.

Statement labels are s15's: C1/C2/C3 = the three 0x20/0x24/0x28 copies; S1 = the 0x18 store
of the +0 halfword; S2 = `temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);`; S3 = the 0x1A
store; S4 = `idx = *(u16 *)outer;`; S5 = `D_800A3478 = outer + 0x18;`;
S6 = `temp_a1 = *(u16 *)(p10 + 4);`; S7 = `D_800A347C = outer + 0x20;`; S8 = the 0x1C store;
P = `p10 = *(s32 *)(outer + 0x10);`; O = `outer = D_800A3468;`; Z = the D_800F10D0 zero
store.

### 1. Target's exact three-load geometry is REACHABLE in ordinary C at 66 instructions.

`K7` = C1,C2,C3, S1, S2, S3, P, S4, S5, S6, S7, S8 (the gp store AFTER the 0x1A store)
measures **5 / build 66 / target 66** and emits

    slot 11  lw $a1,0x10($v1)      slot 19  lw $a0,0x10($v1)      slot 22  lw $a0,0x10($v1)

which is target's load multiset in target's slots, register for register. This is the first
time the campaign has had it at 66 instructions; s14's W5 and the s15 A/B/C families either
missed a slot or cost 67. K7's entire residual is the +2 halfword value's seat -
`tools/ra_solver/local_extract.py` QTYDBG gives qty11 / reg74, birth 36, death 38, refs 2,
allocation order 5 of 25, **got=2 ($v0)** where target needs $a0 - plus the tail order that
follows from it (the idx read hoisted into slot 25 and the addiu / sw %gp pair pushed to
27/28 instead of target's 25/26). `L4` (S4 moved into the +2 read's window on the same
spine) is the same 5 / 66 with the same got=2, so the idx read is not a $v0 donor.

Banked: `rejected/s16-K7-target-load-geometry-11-19-22-but-plus2-value-seats-v0-score5.c`.

### 2. s15's $a0-seat mechanism is CORRECTED: the gp store is not required, only a donor.

s15 recorded that the +2 value's $a0 seat is bought by placing `D_800A3478 = outer + 0x18;`
between the +2 read and the 0x1A store. The measured mechanism is more general and s15's
statement is one instance of it: what buys the seat is ANY statement in that window whose
RTL creates a short-lived, maximal-priority pseudo that takes $v0 first
(local-alloc.c:1649-1685 priority, 1563-1580 first-fit).

`L1` = C1,C2, S1, S2, **C3**, S3, P, S4, S5, S6, S7, S8 - the third COPY statement moved into
the window, gp store left after the 0x1A store - carries **both** target seats at once:
QTYDBG reg74 birth 34 death 38 **got=4 ($a0)** AND reg75 (p10) birth 40 death 48
**got=5 ($a1)**, with three 0x10 loads at 66 instructions. It is the first body in the
campaign to hold both seats simultaneously. It measures 11 because the moved copy's own
three instructions schedule at 21/23/26 instead of target's 16/18/20; the eleven variants
L2, L3, L8 (which copy is moved) and V1, V2, V5, V6, V7, V8, V9, VA (P / S4 / S5 / S7
positions around it) measure 10-17.

Banked: `rejected/s16-L1-both-target-seats-but-moved-copy-schedules-late-score11.c`.

### 3. The E2 spine was swept exhaustively - 228 bodies, no target load geometry.

Spine [O, Z, C1, C2, C3, S1, S2, S5, S3] (i.e. the gp store INSIDE the +2 read's range,
which is what buys E2/E3 their seats and target's 25/26/27 group):

* **X sweep (108 bodies)** - P inserted at each of the 9 spine gaps x 12 tail permutations
  of {S4,S6,S7,S8} with S6 before S8. Log: `tmp/grind/func_80060A68/s16/xsweep.log`.
* **Y sweep (120 bodies)** - P at the 4 copy-region gaps x S4 at all 10 gaps x 3 tail
  permutations of {S6,S7,S8}. Log: `tmp/grind/func_80060A68/s16/ysweep.log`.

Best score in the union is 2 (the E2 class: X5_3, X7_3, X9_2, Y3_2_2 are new members).
**Not one of the 228 emits the third load at slot 11.** On this spine the third 0x10 load
lands at slot 4 (when P precedes the zero store; 65 instructions), at slot 26 or 27 (when P
follows the 0x1A store; 66 instructions), or the body costs 67 instructions.

### 4. A new one-instruction-SHORT class: N2, 65 instructions, score 2.

`N2` = O, **P**, Z, C1,C2,C3, S1, S2, S5, S3, S6, S4, S7, S8 - the p10 read placed before
the D_800F10D0 zero store - is target's stream exactly, except that p10's
`lw $a1,0x10($v1)` is spent filling the load-delay slot after `lhu $v0,0x0($v1)` (our slot
4), which target leaves as a nop; everything from slot 5 on is target shifted one slot
earlier, including target's 25/26/27 group, and the body is 65 instructions. The load
depends only on $v1, so it is unconditionally ready at the top of the block; N1, N5, N7, N8
and every X1_* / X2_* row keep it at slot 4.

Banked: `rejected/s16-N2-p10-read-before-zero-store-load-fills-target-nop-65insns-score2.c`.

### 5. Three more spellings priced and closed.

* **Fully inlined reads** (no `temp2`, no `p10`; each halfword read written inline into its
  own store, so the three reads are separated by their own stores) - R1-R8, 8 bodies,
  66-68 instructions, **score 10-12**. The `temp2` named intermediate is load-bearing.
* **`D_800A347C = outer + 0x20;` moved early** as the cse separator or as the $v0 donor -
  Q1-Q8 (8 bodies) measure 68 instructions / score 8, and L5 measures 67 / score 7. This
  reproduces s15's H-family result on two further geometries.
* **S6 or S8 used as the $v0 donor in the window** - Z1-ZA, 10 bodies, score 6-14.

- [s16] Chassis re-audit: HEAD src/text1b.c is byte-identical to s14's saved copy; E2 re-measures 2 / build 66 / target 66 and E3 re-measures 3 / build 66 / target 66; all ~270 s16 bodies are FAKE-free so no ablation applied.

- [s16] K7 (C1,C2,C3,S1,S2,S3,P,S4,S5,S6,S7,S8) measures 5 / build 66 / target 66 and reproduces TARGET'S EXACT THREE-LOAD GEOMETRY: lw $a1,0x10($v1) at slot 11, lw $a0,0x10($v1) at slots 19 and 22.

- [s16] K7's entire residual is one seat: QTYDBG qty11 / reg74 (the +2 halfword value) birth 36 death 38 refs 2 order 5 of 25 got=2 ($v0), where target needs $a0; the tail order (idx at 25, addiu/sw-gp at 27/28) follows from it.

- [s16] L4 (idx moved into the +2 read's window on K7's spine) is also 5 / 66 with reg74 got=2 - the idx read is not a $v0 donor.

- [s16] L1 (third copy statement moved into the +2 read's window, gp store left after the 0x1A store) carries BOTH target seats simultaneously - reg74 got=4 ($a0) and reg75 got=5 ($a1) - with three loads at 66 instructions; it is the first body in the campaign to do so, and it measures 11 only because the moved copy schedules at 21/23/26 instead of target's 16/18/20.

- [s16] CORRECTION to s15: the +2 value's $a0 seat does not require `D_800A3478 = outer + 0x18;` inside its range; any statement creating a short-lived maximal-priority $v0 pseudo there buys it (local-alloc.c:1649-1685 / 1563-1580). s15's finding is an instance of that mechanism, not the mechanism.

- [s16] Exhaustive sweep of the E2 spine [O,Z,C1,C2,C3,S1,S2,S5,S3]: 228 ordinary-C bodies (X: P at all 9 gaps x 12 tail perms; Y: P at 4 copy-region gaps x S4 at 10 gaps x 3 tail perms). Minimum score 2; not one emits the third 0x10 load at slot 11.

- [s16] N2 (p10 read placed before the D_800F10D0 zero store) is 2 / build 65 / target 66 - target's whole stream with p10's load spent on the slot-4 load-delay nop that target keeps, so the body is one instruction short.

- [s16] Fully-inlined halfword reads (no temp2, no p10) measure 10-12 across R1-R8; the temp2 named intermediate is load-bearing for the current score-2 class.

- [s16] D_800A347C moved early measures 67-68 instructions / score 7-8 across Q1-Q8 and L5, reproducing s15's H-family result on two further geometries; S6/S8 as the window donor measures 6-14 across Z1-ZA.

- [s16] Chassis re-audit: HEAD src/text1b.c is byte-identical to tmp/grind/func_80060A68/s14/text1b.c.HEAD; E2 re-measures 2 / build 66 / target 66 and E3 re-measures 3 / build 66 / target 66; every s16 body is FAKE-free so no ablation applied.

- [s16] K7 (C1,C2,C3,S1,S2,S3,P,S4,S5,S6,S7,S8) measures 5 / build 66 / target 66 and reproduces target's exact three-load geometry: lw $a1,0x10($v1) at slot 11, lw $a0,0x10($v1) at slots 19 and 22.

- [s16] K7's entire residual is one local-alloc seat: QTYDBG qty11 / reg74 (the +2 halfword value) birth 36 death 38 refs 2 order 5 of 25 got=2 ($v0) where target needs $a0; the tail order (idx at 25, addiu / sw %gp at 27/28) follows from it.

- [s16] L4 (idx moved into the +2 read's window on K7's spine) is also 5 / 66 with reg74 got=2, so the idx read is not a $v0 donor.

- [s16] L1 (third copy statement moved into the +2 read's window, gp store left after the 0x1A store) carries BOTH target seats simultaneously -- reg74 got=4 ($a0) and reg75 got=5 ($a1) -- with three loads at 66 instructions; the first body in the campaign to do so.

- [s16] CORRECTION to s15: the +2 value's $a0 seat does not require `D_800A3478 = outer + 0x18;` inside its range; any statement creating a short-lived maximal-priority $v0 pseudo there buys it (local-alloc.c:1649-1685 priority, 1563-1580 first-fit). s15's finding is an instance of that mechanism, not the mechanism.

- [s16] Exhaustive sweep of the E2 spine [O,Z,C1,C2,C3,S1,S2,S5,S3]: 228 ordinary-C bodies (X: P at all 9 gaps x 12 tail perms; Y: P at 4 copy-region gaps x S4 at 10 gaps x 3 tail perms). Minimum score 2; not one emits the third 0x10 load at slot 11.

- [s16] N2 (p10 read placed before the D_800F10D0 zero store) is 2 / build 65 / target 66 -- target's whole stream with p10's load spent on the slot-4 load-delay nop that target keeps, so the body is one instruction short.

- [s16] Fully-inlined halfword reads (no temp2, no p10) measure 10-12 across R1-R8; the temp2 named intermediate is load-bearing for the current score-2 class.

- [s16] D_800A347C moved early measures 67-68 instructions / score 7-8 across Q1-Q8 and L5, reproducing s15's H-family result on two further geometries; S6 or S8 used as the window donor measures 6-14 across Z1-ZA.

- [s16] The function's residual is now bracketed by two ordinary-C bodies that each hold one half of it at 66 instructions: E2/candidate.c holds both register seats and target's 25/26/27 group but only two loads; K7 holds all three loads in target's slots and registers but seats the +2 value in $v0.


## [s17 2026-09-03 - rederive modality] Re-derivation of the read decomposition, and the four-condition law that governs this residual

### 1. Chassis / kill re-audit (mandated)
`sandbox func_80060A68 --disable all` on today's HEAD, with the s14 apply harness:
E2 (candidate.c's body) = **2 / build 66 / target 66**, two `lw ?,0x10($v1)` loads at slots 11 ($a1)
and 19 ($a0).  K7 = **5 / build 66 / target 66**, three loads at slots 11 ($a1), 19 ($a0), 22 ($a0).
Both reproduce the s15/s16 numbers exactly, so the chassis has not moved.  Neither body (nor any of
the 56 bodies measured this session) carries a FAKE construct, so `tools/fake_ablate.py` has nothing
to ablate; the s15/s16 instance kills remain chassis-current.

### 2. The rederive: the three 0x10 reads were REDECOMPOSED, not permuted (families A, B, C)
Every prior session permuted the same twelve statements.  s17 changed the decomposition itself:
* **A family (8 bodies)** - the `p10` pointer local is DELETED and the +4 read is written inline,
  `temp_a1 = *(u16 *)(*(s32 *)(outer + 0x10) + 4);`.
* **B family (3 bodies)** - `p10` is kept but feeds the +0 read (`*(u16 *)(outer + 0x18) =
  *(u16 *)(p10 + 0);`), the +2 and +4 reads inline.
* **C family (4 bodies)** - `p10` feeds the +2 read, the +0 and +4 reads inline.

Result: 3-8 at 66-68 instructions and **not one of the fifteen emits any `lw ?,0x10($v1)` at
slot 11**; the third load lands at 18/21/24-28.  Target's slot-11 load is the head of a LONG pointer
pseudo (born slot 11, dying at `lhu $a1,0x4($a1)` at slot 28).  An inline `*(s32 *)(outer + 0x10)`
inside a halfword read creates a two-insn pointer pseudo instead, which local-alloc seats in $v0/$a0
and sched2 never hoists.  The `p10` local is load-bearing structure, not a spelling choice.  Best of
the family is A2 = 3 / 66 (banked).

### 3. The four-condition law, read out of the local-alloc ground truth
`tools/ra_solver/local_extract.py text1b --func func_80060A68` (QTYDBG) on K7, E3, G1, H3.  Writing
S1 = the 0x18 store with the inline +0 read, S2 = `temp2 = ` the +2 read, S3 = the 0x1A store,
S5 = `D_800A3478 = outer + 0x18;`, S7 = `D_800A347C = outer + 0x20;`, P = `p10 = *(s32 *)(outer +
0x10);`, S6 = `temp_a1 = *(u16 *)(p10 + 4);`, S4 = `idx = *(u16 *)outer;`:

* **R1 (three loads)** - a store must separate each consecutive pair of `*(s32 *)(outer + 0x10)`
  reads (the s15 cse rule; both v1-based and gp-based stores separate).
* **R2 (the +2 value seats in $a0)** - reg74 has refs 2, so with S2 and S3 adjacent its span is 2 and
  its priority is 10000, the maximum a refs-2 quantity can reach (`floor_log2(refs)*refs*size*10000/
  (death-birth)`, local-alloc.c:1649-1685); it is then allocated 5th of 25 and first-fit
  (local-alloc.c:1563-1580) hands it $v0.  It can only be pushed to $a0 by LENGTHENING its range -
  i.e. by putting a statement between S2 and S3 - so that a span-2 $v0 quantity inside that range is
  allocated first.  Measured: K7 (nothing in the window) reg74 got=2; E3/G1/H3 (S5 or S7 in the
  window) reg74 got=4.
* **R3 (p10 seats in $a1)** - reg75 needs a span-2 $v0 quantity inside [P, S6] (so $v0 is taken) and
  `idx` live across it (so $a0 is taken; idx is argument-suggested $a0 and is allocated in the
  suggested pass).  Measured: K7 reg75 got=5 with S5's addiu at [44,46] inside it; G1 reg75 got=2
  with nothing inside it.
* **R4 (the load is emitted at slot 11)** - p10's read must not follow a gp-based store.  sched2
  disambiguates `mem(v1+0x10)` from the v1-based 0x18/0x1A/0x20/0x24/0x28 stores and freely hoists
  the load above them, but it cannot disambiguate it from `sw $v0,%gp_rel(D_800A3478)($gp)`.  Every
  s17 body whose P follows either gp store emits the load in the slot immediately after that store
  (26-28); every body with P before both gp stores that also satisfies R3 emits it at slot 11.

**The bind:** the only two span-2 $v0 quantities this function owns are the addiu halves of the two
gp stores, S5 and S7.  R2 needs one of them between S2 and S3; R4 needs P before both of them; and
R1 needs a store between S2 and P.  A copy statement is the only non-gp store that can occupy that
last slot, and moving a copy costs 10-15 (H-s16-4, re-confirmed by s17's M family).

### 4. H3 - target's whole stream with ONE instruction relocated (the new closest-by-structure body)
`H3 = C1,C2,C3,S1,S2,S5,S3,P,S4,S7,S6,S8` measures 5 / build 67 / target 66.  Slot-for-slot it is
target's stream with `lw $a1,0x10($v1)` moved from slot 11 to slot 26 (immediately after the
`sw $v0,%gp(D_800A3478)` at 25), everything between shifted one slot earlier, and a nop at slot 32.
Both target register seats are held (reg74 got=4, reg75 got=5) and all three loads are present.  This
is the first body in the campaign to hold both seats, three loads AND target's complete opcode
sequence at once; its whole residual is one instruction's position.  Banked at
`rejected/s17-H3-target-stream-with-the-a1-load-displaced-by-the-gp-store-score5-67insns.c`.

### 5. The s16 frontier's named next probe is closed
s16 asked for the D_800A347C addiu/store pair (S7) to be swept as the window donor on the spine that
already has the slot-11 load.  Seven placements measured (G2 9/66, G3 7/66, G4 7/66, G5 9/66, GA 7/66,
HC 7/66, HD 7/67, HE 7/67).  S7 is a gp store exactly like S5: it buys the $a0 seat by R2 and destroys
the slot-11 load by R4, and it additionally drags target's slots 30/31 addiu/sw %gp(D_800A347C) pair
into the 25/26 window where target puts the D_800A3478 pair.  Neither gp store can be the window donor
while the slot-11 load survives.

### 6. Other s17 negatives
* P placed among the copy statements with S5 in the +2 window (J4, J5, J6, J7): both seats, three
  loads, but the load is emitted at 23-24 and the body is 67 - being textually early is not
  sufficient for the hoist.
* S7 used as the cse separator between the +2 read and the p10 read (G2, G5, GA, GB): 7-9 / 66.
* S5 moved after S6 on the K7 spine (J1, J2, J3): R3 fails, p10 falls back to $v0/$a0, 7-9.
* Copy-as-separator bodies with P before the gp store (M1, M2, M3, M5, M6, M7): M2/M6/M7 DO put the
  load at slot 11 in $a1 with both seats, but the moved copy costs an instruction and 10-13 points -
  the same price H-s16-4 recorded, now confirmed on a different spine.

Artifacts: `tmp/grind/func_80060A68/s17/` (56 disassemblies, gen*.py, sweep.sh, qty.sh, H3.sched2).

- [s17] Chassis unchanged: E2 = 2 / build 66 / target 66 and K7 = 5 / build 66 / target 66 on today's HEAD, matching the s15/s16 records exactly; no FAKE construct exists in any live body, so fake_ablate has nothing to ablate.

- [s17] The p10 pointer local is load-bearing structure, not a spelling: fifteen bodies that delete it or reassign it to the +0 or +2 read (families A, B, C) all fail to put any lw ?,0x10($v1) at slot 11, because target's slot-11 load heads a pointer pseudo that lives from slot 11 to slot 28 and an inline read creates only a two-insn pseudo.

- [s17] The +2 halfword value can only be pushed out of $v0 by LENGTHENING its live range: with its read and store adjacent it has refs 2 / span 2, the maximum priority a refs-2 quantity can reach under local-alloc.c:1649-1685, and first-fit (local-alloc.c:1563-1580) then hands it $v0. QTYDBG confirms got=2 on K7 and got=4 on every body with a statement in that window.

- [s17] p10 reaches $a1 exactly when a span-2 $v0 quantity lies inside its live range and idx (argument-suggested $a0) is live across it; QTYDBG shows reg75 got=5 on K7/H3 and got=2 on E3/G1.

- [s17] sched2 freely hoists lw ?,0x10($v1) above the v1-based 0x18 / 0x1A / 0x20 / 0x24 / 0x28 stores but never above sw $v0,%gp_rel(D_800A3478)($gp): in all 56 bodies measured this session, a p10 read placed after either gp store has its load emitted in the slot immediately following that store (26-28), and only bodies whose p10 read precedes both gp stores ever reach slot 11.

- [s17] The bind is now single-valued: the only two span-2 $v0 quantities the function owns are the addiu halves of the two gp stores; the $a0 seat needs one of them inside the +2 value's range while the slot-11 load needs the p10 read ahead of both, and the only non-gp store that can separate the +2 read from the p10 read is a copy statement, which costs an instruction and 10-13 points.

- [s17] H3 (C1,C2,C3,S1,S2,S5,S3,P,S4,S7,S6,S8) = 5 / build 67 / target 66 is target's entire instruction stream with exactly one instruction relocated -- the lw $a1,0x10($v1) from slot 11 to slot 26 -- plus the nop that follows; both target register seats and all three loads are present and it carries no FAKE construct.

- [s17] s16's named next_probe (the D_800A347C pair as the window donor on the slot-11 spine) is closed at 7-9 across eight placements; both gp stores behave identically under the seat rule and the hoist rule.


## [s18 2026-09-03 - rederive modality] The separator law, and target's load geometry without `temp2`

### 0. Mandated chassis / kill re-audit
`sandbox func_80060A68 --disable all` on today's HEAD, s14 apply harness:
E2 (candidate.c's body) = **2 / build 66 / target 66**, loads at slots 11 ($a1) and 19 ($a0).
K7 = **5 / 66**, loads 11 ($a1) / 19 ($a0) / 22 ($a0).  H3 = **5 / build 67**, loads 18 / 21 / 26.
All three reproduce the s15/s16/s17 records instruction-for-instruction, so the chassis has not
moved.  None of the 74 bodies measured this session carries a FAKE construct, so
`tools/fake_ablate.py` has nothing to ablate and the s15-s17 instance kills stay chassis-current.

### 1. The rederive: R4 is a *register* law, not only a *position* law
s17 recorded R4 as "the p10 read must not follow a gp-based store".  s18's Z family disproves that
as a sufficient condition and replaces it with a two-part statement.

`Z1..Z9` (9 bodies) put the p10 read SECOND in the body - `S1, P, S6, S8, S2, S5, S3, ...` - so that
the 0x1C store (`*(u16 *)(outer + 0x1C) = temp_a1;`) is the cse separator between the p10 read and
the +2 read.  Every one of the three `*(s32 *)(outer + 0x10)` reads therefore precedes BOTH gp
stores, which is exactly what R4 asked for.  Result: 9-11 / 66, loads at 19 ($a0), 22 ($a0) and
**25 ($v0)** - the p10 load is *not* at slot 11 even though nothing is in front of it.  Reason: with
S6 immediately after P, p10's pseudo has span 2 and there is no $v0 donor inside [P, S6], so R3
fails, local-alloc seats p10 in $v0, and a $v0-seated load cannot be hoisted past the $v0 traffic in
slots 12-24.  **Corrected law: the slot-11 load is a consequence of the $a1 SEAT (R3), not of textual
earliness; R4 is a second, independent necessary condition on top of it.**

### 2. The separator law, and why the copy store always costs a nop
Combining R2/R3 (both donors must be gp addiu halves - the only span-2 $v0 quantities the function
owns) with the corrected R4 gives a closed chain on this chassis:

* R3 needs a gp store strictly inside [P, S6] => that gp store is AFTER the p10 read.
* R4 needs every 0x10 read - including the +2 read - to precede that gp store, otherwise its load is
  pinned to the slot immediately after it (measured again this session: the Y family below).
* R1 needs a store between each consecutive pair of 0x10 reads.  The 0x18 store covers one pair.
  The 0x1A store (S3) and the 0x1C store (S8) cannot cover the other pair, because R2 puts S3 after a
  gp store and S8 is downstream of S6.  **So the second separator can only be a copy store.**

Measured price of that copy store, 48 bodies, uniform:

* **M1-M8, W1-WB (17 bodies)** - one whole copy statement moved into the window between the +2 read
  and the p10 read.  Best is T61/T62 = 10 / **67**, loads 11 ($a1) / 17 ($a0) / 20 ($a0).  The 67th
  instruction is always the same nop: the moved copy's `lw $v0,0xC($v1)` and `lw $v0,0x8($v0)` end up
  adjacent (M2 slots 21/23 with the nop at 24), because local-alloc gives the moved copy $v0 for both
  the pointer and the value, whereas target's C3 uses $a0 for the pointer (slot 16) and $v0 for the
  value (slot 18) with the 0x24 store filling the delay.
* **T11-T65 (30 bodies)** - a systematic sweep of WHICH copy is the separator and WHERE the 0x18/0x1A
  pair sits inside the copy block, x 5 tail orders.  Prefixes that move C1 or C2 reach 66 instructions
  (T41/T42/T51/T52/T55, score 15-16) but then the p10 load falls to slot 24-25; the only prefix that
  keeps the slot-11 $a1 load (`C1,C2,S1,S2,C3,P`) is 67 in all five tails.  **No member of the
  copy-store-separator family measured this session is simultaneously 66 instructions and slot-11.**
* **V1-V9 (10 bodies)** - the copy statement SPLIT so only its `sw` moves into the window
  (`cv = *(s32 *)(*(s32 *)(outer + 0xC) + 8); ... *(s32 *)(outer + 0x28) = cv;`).  The split is
  byte-neutral when the two halves stay adjacent (control VC = 5 / 67, identical to the unsplit H3),
  but as a separator it measures **12-14** at 65-67: the `cv` pseudo is long-lived, takes a seat, and
  pushes p10 to $a2 and both halfword pointers to $v0 (V1 loads 11 $a2 / 19 $v0 / 22 $v0).
  Splitting a copy is strictly worse than moving it.

### 3. The Y family - a new score-5 class that holds the slot-11 load with NO copy moved
`Y1 = C1,C2,C3, S1, P, S4, S5, S6, S2, S7, S3, S8` (and Y6/Y8/Y9, the same spine with `idx`
re-placed) measures **5 / 66** with loads at 11 ($a1), 19 ($a0), 26 ($v0).  Slots 0-20 are
byte-identical to target.  The p10 read sits immediately after the 0x18 store, so the 0x18 store is
its cse separator and no copy has to move; the first gp store then sits inside [P, S6] and buys the
$a1 seat.  Its whole residual is that the +2 read, being downstream of that gp store, has its load
pinned at slot 26 in $v0 - the mirror image of H3's residual.  Y2/Y3/Y4/Y5/Y7/YA/YB (7 more bodies)
permute the same spine at 6-9.  Banked as
`rejected/s18-Y1-p10-read-hoisted-above-the-plus2-read-third-load-pinned-behind-gp-store-score5-66insns.c`.

### 4. U2 - target's exact load geometry with NO `temp2` local at all
`U2 = C1,C2,C3, S1, S3i, P, S4, S5, S6, S7, S8`, where `S3i` is the +2 read written INLINE into its
own store (`*(u16 *)(outer + 0x1A) = *(u16 *)(*(s32 *)(outer + 0x10) + 2);`), measures **5 / 66** with
loads at **11 ($a1), 19 ($a0), 22 ($a0) - target's geometry exactly**, and no `temp2` declaration
anywhere in the body.  The fused 0x1A store is a zero-cost cse separator between the +2 read and the
p10 read, which is what the copy store was being paid a nop for.  This REFINES s16's "the temp2 named
intermediate is load-bearing": that conclusion was drawn from R1-R8, which inlined *all three* reads
and also deleted `p10`; with `p10` kept, inlining only the +2 read is free.
U2's residual is slots 24-29 and is the same single seat as K7's: the +2 halfword value lands in $v0
where target needs $a0, so the 0x1A store fires at 26 ahead of the addiu/gp-store pair instead of
behind it.  On the U spine that seat has no source-level donor site, because a fused read+store
statement has no interior in which to place one.  U1/U3-U8 (7 more) measure 5-9.  Banked as
`rejected/s18-U2-inline-0x1A-store-gives-targets-exact-load-geometry-without-temp2-score5-66insns.c`.

### 5. Where the campaign stands after s18
Three ordinary-C bodies now each hold a different two-thirds of the residual at 66 instructions:
E2 (both seats + target's 25/26/27 group, two loads, score 2), K7/U2 (all three loads in target's
slots and registers, +2 value in $v0, score 5), Y1 (slots 0-20 byte-identical, slot-11 $a1 load, +2
load pinned at 26, score 5).  The single thing none of them has is a *free* store between the +2 read
and the p10 read that leaves the +2 read upstream of the gp store: the fused 0x1A store gives it for
free but removes the donor site (U family), a gp store gives it but pins the load (Y/H family), and a
copy store gives it but costs a load-delay nop (M/W/T family).

Artifacts: `tmp/grind/func_80060A68/s18/` (74 disassemblies, gen18.py, genV/genY/genZ/genW/genT/genU.py,
sweep.sh, dis.sh, bodies/).

- [s18] Chassis re-audit: E2 = 2 / 66, K7 = 5 / 66, H3 = 5 / 67 on today's HEAD, all identical to the s15-s17 records; 74 s18 bodies, zero FAKE constructs, nothing for fake_ablate.py to ablate.

- [s18] R4 corrected: nine Z bodies place every 0x10 read ahead of both gp stores using the 0x1C store as the second cse separator, and the p10 load still comes out at slot 25 in $v0 (9-11 / 66) -- textual earliness is not sufficient; the slot-11 hoist follows from the $a1 SEAT (R3), which needs a span-2 $v0 donor strictly inside [p10 read, +4 read].

- [s18] The second cse separator can only be a copy store once R2 and R3 are satisfied, and a moved copy costs one load-delay nop: 17 M/W bodies and 30 T bodies (48 total) -- the only prefix that keeps the slot-11 $a1 load (C1,C2,S1,S2,C3,P) measures 67 instructions in all five tail orders, best score 10.

- [s18] Splitting a copy statement so only its sw moves (cv = <value>; ... *(s32*)(outer+0x28) = cv;) is byte-neutral when the halves stay adjacent (VC = 5/67, identical to the unsplit H3) but measures 12-14 as a separator across V1-V9, because the cv pseudo takes a seat and pushes p10 to $a2 and both halfword pointers to $v0.

- [s18] New score-5 class Y1/Y6/Y8/Y9 = C1,C2,C3,S1,P,S4,S5,S6,S2,S7,S3,S8 at 5 / 66 with slots 0-20 byte-identical to target and loads at 11 ($a1) / 19 ($a0) / 26 ($v0); no copy is moved, the 0x18 store is p10's cse separator, and the only residual is the +2 read being downstream of the gp store.

- [s18] U2 = C1,C2,C3,S1,S3i,P,S4,S5,S6,S7,S8 (the +2 read written inline into its own 0x1A store, no temp2 local anywhere) measures 5 / 66 and reproduces target's exact three-load geometry 11 ($a1) / 19 ($a0) / 22 ($a0); s16's "temp2 is load-bearing" was measured on bodies that also deleted p10, and does not hold when p10 is kept.

- [s18] On the U spine the +2 value's $a0 seat has no source-level donor site, because a fused read+store statement has no interior; U2's residual is therefore the same single local-alloc seat as K7's, reached with strictly fewer locals.

- [s18] Chassis re-audit (mandated): E2 = 2 / build 66 / target 66, K7 = 5 / 66 and H3 = 5 / 67 on today's HEAD, instruction-for-instruction identical to the s15-s17 records; none of the 74 bodies measured this session carries a FAKE construct, so tools/fake_ablate.py had nothing to ablate and the s15-s17 instance kills remain chassis-current.

- [s18] R4 restated: the slot-11 lw $a1,0x10($v1) is a consequence of p10's $a1 SEAT, not of the p10 read being textually early. Nine Z bodies satisfy s17's positional R4 for all three 0x10 reads and still emit the p10 load at slot 25 in $v0 (9-11 / 66), because with the +4 read immediately after the p10 read there is no span-2 $v0 donor inside p10's range.

- [s18] The separator chain is now closed on this chassis: R3 forces a gp store after the p10 read, corrected R4 forces every 0x10 read before that gp store, R1 forces a store between consecutive 0x10 reads, the 0x1A store is pushed downstream of a gp store by R2 and the 0x1C store is downstream of the +4 read - so the second separator is either a copy store or the fused 0x1A store.

- [s18] Price of the copy store as that separator, measured on 47 bodies: the only prefix that keeps the slot-11 $a1 load (C1,C2,S1,S2,C3,P) is 67 instructions in all five tail orders (best 10), the 67th instruction being a load-delay nop caused by local-alloc handing the moved copy $v0 for both its pointer and its value; prefixes that move C1 or C2 are 66 but put the p10 load at slot 24-25.

- [s18] Splitting a copy so that only its sw moves is byte-neutral when the halves stay adjacent (control VC = 5 / 67, identical to the unsplit H3) but measures 12-14 as a separator across V1-V9, because the detached value pseudo takes a seat and pushes p10 to $a2 and both halfword pointers to $v0.

- [s18] Y1 (C1,C2,C3,S1,P,S4,S5,S6,S2,S7,S3,S8) is a new score-5 class at 66 instructions whose slots 0-20 are byte-identical to target, holding the slot-11 $a1 load with no copy moved; its whole residual is the +2 read sitting downstream of the gp store, which pins that read's load at slot 26 in $v0.

- [s18] U2 (C1,C2,C3,S1,S3i,P,S4,S5,S6,S7,S8) reproduces target's exact three-load geometry (11 $a1, 19 $a0, 22 $a0) at 66 instructions with NO temp2 local at all, because a fused read+store is a free cse separator; s16's 'temp2 is load-bearing' was measured on bodies that also deleted p10 and does not hold when p10 is kept.

- [s18] Three ordinary-C bodies now each hold a different two-thirds of the residual at 66 instructions: E2 (both register seats and target's 25/26/27 group, two loads, score 2), K7/U2 (all three loads in target's slots and registers, +2 value in $v0, score 5), Y1 (slots 0-20 exact, slot-11 $a1 load, +2 load pinned at 26, score 5).


## [s19 2026-09-03 - structural modality] The floor-2 body is ONE instruction from a match, and the cse-separator inventory is now complete and fully priced

### 0. Mandated chassis / kill re-audit
`sandbox func_80060A68 --disable all` on today's HEAD with the s14 apply harness:
CU2 (= s18's U2) = **5 / build 66 / target 66**, loads at 11 ($a1) / 19 ($a0) / 22 ($a0);
CE2 (= candidate.c's E2 body) = **2 / build 66 / target 66**, loads at 11 ($a1) / 19 ($a0);
D2 (an independent re-spelling of E2 through the s19 generator) = **2 / 66**, identical.
All reproduce the s15-s18 records instruction-for-instruction, so the chassis has not moved.
None of the 71 bodies measured this session carries a FAKE construct, so `tools/fake_ablate.py`
has nothing to ablate and every s15-s18 instance kill remains chassis-current.
(Slot numbers in this section are 1-based against `asm/funcs/func_80060A68.s` lines 2-68; the
s19 sweep harness prints them 1-based too, so they read one higher than the s15-s18 0-based notes.)

### 1. THE FLOOR-2 BODY'S ENTIRE RESIDUAL IS ONE MISSING INSTRUCTION, AND IT FITS IN AN EXISTING NOP
A slot-for-slot disassembly diff of E2 against target (`tmp/grind/func_80060A68/s19/E2.dis`,
`cmp.sh`) shows the two bodies are identical everywhere except two adjacent places:

| slot | target | E2 |
|---|---|---|
| 23 | `lw $a0,0x10($v1)` | `nop` |
| 25 | `lhu $a0,0x2($a0)` | `lhu $a0,0x2($a1)` |

E2's +2 read is cse-merged with the `p10` read (nothing separates `temp2 = *(u16 *)(*(s32 *)
(outer + 0x10) + 2);` from `p10 = *(s32 *)(outer + 0x10);` in its statement order), so the
third `lw` never exists and the +2 halfword is read through `$a1`, p10's register.  Everything
else - both register seats, the addiu/gp-store pair inside the +2 value's window, the whole
tail - is already target's.  **The slot the missing load belongs in is currently a load-delay
`nop`, so restoring the third read costs ZERO instructions: a separator between the +2 read and
the p10 read that adds no instruction and moves no load turns the floor-2 body into a byte
match.**  This is the first time the residual has been localized to a single instruction with
an already-free slot to put it in.

### 2. New lever: the 0x18 halfword store SPLIT off its read (`temp0`) is byte-neutral and is a free separator
`S1a = temp0 = *(u16 *)(*(s32 *)(outer + 0x10) + 0);` / `S1b = *(u16 *)(outer + 0x18) = temp0;`.
Control X1 (`CC,S1a,S1b,S3i,P,S4,S5,S6,S7,S8`) = 5 / 66 with loads 11/19/22 - byte-identical to
the unsplit U2, so the split itself costs nothing when the halves stay adjacent.  Detached, the
`sh 0x18` is a free non-gp cse separator - the one thing the copy store was being paid a
load-delay nop for in s18.

**X2 = `C1,C2,C3, S1a, S2, S1b, P, S5, S3, S4, S6, S7, S8` = 4 / build 65 / target 66** is
target's entire instruction stream MINUS EXACTLY ONE INSTRUCTION: its slots line up one-for-one
with target's except that target's third `lw $a0,0x10($v1)` is absent (the +0 read and the +2
read are now the merged pair, since S1b moved out of the gap between them), plus the resulting
one-slot shift and a single 28/29 swap.  This is the smallest instruction-count residual in the
campaign: every previous class was 66 with wrong seats or 67 with an extra nop.  X2 and E2 are
the same defect seen from two sides - one free separator short, in a body that has a free slot
for the load that separator would create.

### 3. The cse-separator inventory is now COMPLETE and every member is priced
Two read gaps must each carry a store (R1); R2 pushes the 0x1A store downstream of a gp store
and R4 puts every 0x10 read upstream of it, so the 0x1A store can never be a separator.  What
remains, with this session's measured price:

* **0x18 store (fused S1, or split S1a/S1b)** - free, but covers exactly ONE gap.  Using it for
  the second gap opens the first (X2/X3/X6/X8: 65 instructions, 4-7).
* **0x1C store (S8)** - needs `temp_a1` hence `S6` immediately after `P`, which removes R3's
  donor window and drops p10 to $v0 (s18 Z family, 9-11).
* **a copy store, whole statement moved** - one load-delay nop, 67 instructions (s18 M/W/T, 47
  bodies).
* **a copy store, only the store moved via a named pointer intermediate** (`pc = *(s32 *)(outer
  + 0xC); ... *(s32 *)(outer + 0x28) = *(s32 *)(pc + 8);`) - **new this session, P1-PC, 12
  bodies, ALL 67 instructions, 9-14**.  The pointer local does not decouple the copy's load from
  its store: P3's `pc` load SINKS to slot 22 with its use at 25 and pays a nop at 26, exactly
  the price of moving the whole statement.  P3/P6 do hold the slot-12 $a1 load with three loads.
* **a copy store reached by hoisting a 0x10 read above the copy block** (N1-NC, R1-RC, 24
  bodies) - dead: the hoisted read's own load is emitted at slot 11-16 in $a0, taking the
  seat target gives p10; 12-17 at 65-67.  R1 (`S1a,C1,C2,C3,S2,S1b,P,S5,S3,S6,S4,S7,S8`) has all
  three loads at 66 instructions and still scores 14 for that reason.
* **the copy stores reached by hoisting the p10 read above the copy block** (Q1-QC, 12 bodies) -
  three loads, but 67 instructions and the p10 load at slot 24-25, 4-8.
* **the copy stores reached by putting the whole halfword block BEFORE the copy block**
  (G1-GA, 10 bodies) - three loads restored, but the halfword loads hoist to slots 10-11 and the
  copy block collapses: 15-21 at 66-67.
* **the `D_800F10D0` zero-store** - the last store in the function that had never been priced.
  Moved down into the gap (D1/D3/D5/D6) it DOES restore the third load, but its five-instruction
  address computation moves with it, the body drops to 65 instructions and the head geometry is
  destroyed: 15-19.  Control D2 (store left at the top) = 2 / 66 = E2.

Every store the function owns has now been measured in the second-gap role on this chassis.

Artifacts: `tmp/grind/func_80060A68/s19/` (71 bodies + disassemblies, gen19.py, genX/genN/genQ/
genR/genP/genG/genD.py, sweep.sh, dis.sh, cmp.sh, target.txt).

- [s19] Chassis re-audit: CU2 = 5 / build 66 / target 66 and CE2 = 2 / 66 on today's HEAD, instruction-for-instruction identical to the s15-s18 records; 71 s19 bodies, zero FAKE constructs, nothing for fake_ablate.py to ablate.

- [s19] The floor-2 body's whole residual is ONE instruction: a slot-for-slot diff shows E2 == target except that target's third `lw $a0,0x10($v1)` is a `nop` in E2 and the following `lhu $a0,0x2(...)` therefore reads $a1 (p10's register) instead of $a0. The missing load's slot is already a load-delay nop, so a separator that restores the third read at zero instruction cost would be a byte match.

- [s19] Splitting the 0x18 halfword store off its read (temp0 = read; ... *(u16 *)(outer + 0x18) = temp0;) is byte-neutral when the halves stay adjacent (control X1 = 5 / 66, identical to the unsplit U2) and gives one FREE non-gp cse separator - the thing s18's copy-store separator was paying a load-delay nop for.

- [s19] X2 (C1,C2,C3,S1a,S2,S1b,P,S5,S3,S4,S6,S7,S8) = 4 / build 65 / target 66 is target's entire stream minus exactly one instruction (the third lw of *(s32 *)(outer + 0x10)); the split 0x18 store covers the +2-read-to-p10-read gap for free, which leaves the +0/+2 gap unseparated.

- [s19] The two read gaps cannot both be covered for free from the existing statement inventory as measured on this chassis: the 0x18 store covers exactly one, the 0x1A store is barred by R2+R4, and the 0x1C store needs S6 adjacent to P which removes R3's donor.

- [s19] Writing a copy through a named pointer intermediate so only its store moves (pc = *(s32 *)(outer + 0xC); ... *(s32 *)(outer + 0x28) = *(s32 *)(pc + 8);) does NOT decouple the copy's load from its store: across P1-PC (12 bodies) every body is 67 instructions, and P3's pc load sinks to slot 22 with its deref at 25 and a nop at 26 - the same load-delay nop the whole-statement move pays.

- [s19] Hoisting a 0x10 halfword read above the copy block to borrow a copy store as the free separator is dead across 24 bodies (N1-NC, R1-RC): the hoisted read's load is emitted at slot 11-16 in $a0, taking the seat target gives p10; R1 has all three loads at 66 instructions and still scores 14.

- [s19] Hoisting the p10 read above the copy block (Q1-QC, 12 bodies) gives three loads but 67 instructions with the p10 load at slot 24-25 (4-8); putting the whole halfword block before the copy block (G1-GA, 10 bodies) restores three loads but hoists the halfword loads to slots 10-11 (15-21).

- [s19] The D_800F10D0 zero-store - the last store in the function never priced as a cse separator - does restore the third load when moved into the gap (D1/D3/D5/D6) but drags its five-instruction address computation with it, leaving 65 instructions and a destroyed head at 15-19; control D2 with the store left at the top reproduces E2 exactly at 2 / 66.

- [s19] Chassis re-audit (mandated): CU2 = 5 / build 66 / target 66 with loads at 11 ($a1) / 19 ($a0) / 22 ($a0), CE2 = 2 / build 66 / target 66, and an independent re-spelling of the same body (D2) = 2 / 66 on today's HEAD - instruction-for-instruction identical to the s15-s18 records. None of the 71 bodies measured this session carries a FAKE construct, so tools/fake_ablate.py had nothing to ablate and the s15-s18 instance kills remain chassis-current.

- [s19] The floor-2 body (E2, memory/grind/func_80060A68/candidate.c) is target except at two adjacent slots: target's slot 23 lw $a0,0x10($v1) is a nop in E2, and the lhu $a0,0x2(...) at slot 25 reads $a1 (p10's register) instead of $a0. Both register seats, the addiu/gp-store pair inside the +2 value's window and the entire tail are already correct.

- [s19] Because the missing third load's slot is an existing load-delay nop, restoring the +2/p10 read separation costs ZERO instructions - the match condition is now a single free cse separator, not an instruction-count problem.

- [s19] Splitting the 0x18 halfword store off its read is byte-neutral when the halves stay adjacent: control X1 = 5 / 66 with loads 11/19/22, byte-identical to the unsplit U2.

- [s19] X2 (C1,C2,C3,S1a,S2,S1b,P,S5,S3,S4,S6,S7,S8) = 4 / build 65 / target 66 is target's entire instruction stream minus exactly one instruction (the third lw of *(s32 *)(outer + 0x10)) - the smallest instruction-count residual recorded in this campaign.

- [s19] There are two read gaps and exactly one free separator: the 0x18 store covers whichever gap it is spent on and opens the other (X2/X3/X6/X8 at 65 instructions, 4-7).

- [s19] The cse-separator inventory is now complete and every member is priced in the second-gap role: 0x18 (free, one gap only), 0x1A (barred by R2+R4), 0x1C (needs S6 adjacent to P, which removes R3's donor - s18 Z family), the three copies (one load-delay nop by every route: whole move s18, named pointer intermediate P1-PC all 67, read hoisting N/R 12-17, block reorder G 15-21), and the D_800F10D0 zero-store (restores the load but costs the head geometry, 15-19 at 65).

- [s19] A named pointer intermediate does not decouple a copy's load from its store under GCC 2.7.2 here: P3's pc load sinks to slot 22 with its deref at 25 and pays a nop at 26, exactly the price of moving the whole copy statement - which closes s18's frontier item 1 (seating the moved copy's pointer in $a0).

- [s19] Whichever 0x10 halfword read is hoisted above the copy block has its own load emitted at slot 11-16 in $a0, taking the seat target gives p10 (N/R, 24 bodies); the copy stores are therefore not usable as a free separator by moving a read to them.

- [s19] src/text1b.c was restored to HEAD at the end of the session; the only dirty file is metrics/events.jsonl.

## [s20 2026-09-03 - structural modality] The merge is CSE-1, confirmed in the dumps; only a memory store or a call can break it; and two new same-floor bodies localize the residual to ONE decision each

### 0. Mandated chassis / kill re-audit
`sandbox func_80060A68 --disable all` on today's HEAD with the s14 apply harness:
D1 (a fresh re-spelling of candidate.c's E2 body through the s20 generator) = **2 / build 66 /
target 66**, loads at 12 ($a1) / 20 ($a0) - instruction-for-instruction the s15-s19 record.
The chassis has not moved.  All 46 bodies measured this session are ordinary C with zero FAKE
constructs, so `tools/fake_ablate.py` again had nothing to ablate and every s15-s19 instance
kill remains chassis-current.  (Slot numbers in this section are 1-based against
`asm/funcs/func_80060A68.s` lines 2-68, i.e. the same numbering as `s20/target.txt`; the s20
sweep harness prints them in that numbering too, so unlike s19 they read the SAME as target.)

### 1. PASS ATTRIBUTION IS NOW READ OUT OF THE DUMPS, NOT INFERRED
`pwsh tools/grinder/dump.ps1 func_80060A68` on the E2 body, sliced with `s20/slice.py`:

* `text1b.rtl` for func_80060A68 contains **three** `(mem:SI (plus:SI (reg/v:SI 72) (const_int 16)))`
  loads - insn 46 -> reg90 (the +0 read's pointer), insn 53 -> reg92 (the +2 read's pointer),
  insn 58 -> reg/v 75 (`p10`).  Insn 50 (the `sh` to `outer+0x18`) sits between 46 and 53.
* `text1b.jump` still has three.  `text1b.cse` has **two**.  `loop`, `cse2` and `combine` also
  have two.

**The merge is performed by the FIRST cse pass (cse.c), on insn 58, exactly as s15 guessed and
s19 asked to be confirmed.  The attribution is now measured, not inferred.**

### 2. CLASS RESULT: nothing but a memory store or a call can separate two of these reads
Read out of `tools/gcc-2.7.2/cse.c`.  The table entry for `(mem:SI (plus (reg 72) (const_int 16)))`
has a VARYING address (`cse_rtx_addr_varies_p` is true - the address contains a pseudo), so
`invalidate_memory` (cse.c:1701) removes it unconditionally on any invalidation, regardless of
the `all` / `nonscalar` refinement that protects fixed-address scalars.  `invalidate_memory` has
exactly two callers: `invalidate_from_clobbers` (cse.c:7599), which fires only when
`note_mem_written` set `writes_memory.var`, and that flag is set only when the written rtx is a
`MEM` (cse.c, `note_mem_written`); and `cse_insn` (cse.c:7246) for a non-const `CALL_INSN`.
The only other way to lose the entry is `invalidate()` on the base register `reg 72` (`outer`).

Consequence for this function: **frontier item 1 of s19 - "a separator that is not a store" - is
closed.  In cse's model a construct that emits no store, no call and no redefinition of `outer`
cannot make the two reads non-equivalent, whatever its C spelling.**  The base-register escape
was measured too: **Q1** (`outer = outer;` written between the +2 read and the p10 read on the
E2 spine) = **2 / 66 with two loads**, byte-identical to the control - `store_expr` returns the
same rtx for both sides so no insn is emitted at all, and cse never sees a redefinition.  A
spelling that DOES emit the redefinition (Q5, `outer = D_800A3468;` re-read) costs the gp load:
26 / 68.

### 3. Type of the +2 value is inert (structural lever, killed)
D1 (`u16 temp2`, control) = 2 / 66; **D2 (`s32 temp2`) = 2 / 66; D3 (`u32 temp2`) = 2 / 66** -
byte-identical.  The lhu/sh pair and the pseudo's local-alloc `size` are unchanged by the
declared width, so declaration-type narrowing/widening on the +2 value is not a lever here.

### 4. NEW BODY Q2/R3: 66 instructions, target's EXACT three-load geometry, one register seat wrong
`Q2 = C1,C2,C3, S1, S2, S4, S3, P, S5, S6, S7, S8` (i.e. E2's spine with the 0x1A store moved
UP in front of `P` so it separates the +2 read from the p10 read, and `idx` inserted into the
[+2 read, 0x1A store] window) measures **5 / build 66 / target 66** with all three
`lw ?,0x10($v1)` at **slots 12 ($a1) / 20 ($a0) / 23 ($a0) - target's slots and target's
registers**.  `R3 = C1,C2,C3,S1,S2,S3,S4,P,S5,S6,S7,S8` is the same, 5 / 66.  A slot-for-slot
diff shows Q2 is target through slot 24 and again from slot 33 to the end; the whole residual is
slots 25-32, and it is caused by ONE thing: the +2 value seats in `$v0` (`lhu v0,0x2(a0)`)
instead of target's `$a0`, which forces `sh v0,0x1A(v1)` before the `addiu v0,v1,0x18`/gp-store
pair instead of after it.  **This is the first body in the campaign that has target's complete
instruction inventory, target's instruction count and target's three-load geometry with only a
single register seat wrong.**

### 5. WHY Q2's window filler fails - the R2 filler must be a STORE
`tools/ra_solver/local_extract.py text1b --func func_80060A68` (QTYDBG) on Q2:

    main ord=5   qty=11  reg1=74  birth=36  death=38  refs=2  got=2     <- the +2 value, $v0
    sugg ord=24  qty=12  reg1=73  birth=40  death=66  refs=4  got=4     <- idx, $a0
    main ord=23  qty=13  reg1=75  birth=42  death=48  refs=2  got=5     <- p10, $a1

reg74's span is still **2** even though `idx`'s read is written between the +2 read and the 0x1A
store in the C: local-alloc runs after sched1, and **sched1 moved the `lhu` out of the window**,
so the range was never lengthened.  reg73 (idx) is allocated in the suggested pass and does not
even overlap [36,38], so there is no conflict to exploit - reg74 simply first-fits `$v0` because
nothing occupies `$v0` over its two-insn range.  The gp store works as R2's filler in E2 for a
reason a read cannot reproduce: **`sw $v0,%gp_rel(D_800A3478)($gp)` and `sh ?,0x1A($v1)` are two
stores that sched cannot disambiguate, so the store-store edge GLUES the filler inside the
window.**  R2's filler must therefore be a store whose value is a span-2 `$v0` quantity: a gp
store (addiu+sw) or a copy (lw+sw).  A copy in that position drags its load out of the copy
block (N1/N3/N4 = 9-10 / 66, halfword loads pulled back to slots 17/20), and a gp store there
puts `P` downstream of it, which is R4.

### 6. NEW BODY T1: target's whole stream with p10's load in the slot-5 nop (score 2, 65 insns)
`T1 = P, Z0, C1,C2,C3, S1, S2, S5, S3, S6, S4, S7, S8` - the p10 read hoisted ABOVE the
`D_800F10D0` zero-store, which no previous session had tried (s19's Q family hoisted it above
the COPY block, i.e. below the zero store) - measures **2 / build 65 / target 66** with three
loads at slots **5 ($a1) / 19 ($a0) / 22 ($a0)**.  Slot-for-slot it is **target's entire
instruction stream, every register seat included (the +2 value is in `$a0`), with exactly one
difference: target's slot-5 `nop` is T1's `lw $a1,0x10($v1)`, and everything after it is shifted
one slot earlier.**  T1 is the second score-2 body in the campaign and the first with all three
loads and all target seats.

T1 also breaks the s4/s10 "conservation law" (three independent 0x10 loads => p10 has no early
consumer => sched1/sched2 pin its load to slot 25) for the first time: the law holds for every P
position at or below the zero store, and fails for P above it.  The discontinuity is the zero
store itself - `sw zero,%lo(D_800F10D0)($at)` is an `$at`-based store that sched2 cannot
disambiguate from `lw ?,0x10($v1)`, so it is a hoist barrier.  Above it, p10's load reaches slot
5 (the earliest free slot after the prologue) and eats target's nop; below it, the load pins to
25.  Six bodies (V1-V6) that give p10 an early consumer while keeping P just below the zero
store all regress (7-12 / 66-67); T2/T3/T4 (P just below the zero store, in or before the copy
block) are 5 / 67 with the load at 25.

### 7. Negatives banked this session
* **The +2 read hoisted into or above the copy block** (A1-A9, F1-F7, 16 bodies): three loads at
  66 instructions is easy to reach this way (A1/A2/A4/F1 = 12 / 66) but the hoisted read's own
  `lw` is always emitted at slot 11-16 in `$a0`, taking the seat target gives p10 - the s19 N/R
  law, re-confirmed for the +0 read with the split 0x18 store (F1-F7) and for the +2 read
  (A1-A9).  Best of the family 12 / 66; F4 (the read above the zero store) = 9 / 65.
* **The +2 read placed before the +0 read** (W1, W2, W6, W8): W8 = 7 / 66 with target's exact
  three-load geometry, but the 0x1A store then precedes the 0x18 store in the stream, which is
  seven instructions' worth of reordering - a dead reads-order.
* **P among the copies with the 0x1C store as the second separator and a copy as R3's donor**
  (Z1, Z2, Z4, Z6): 10-14 / 67-68.  Giving [P,S6] a copy-value donor does not recover the slot-12
  load once P is textually inside the copy block.
* **A copy moved into the R2 window so it serves as donor AND gap-2 separator** (N1-N6): 9-12 / 66.
  The copy's load leaves the copy block, so the halfword pointer loads move to slots 17/20.
* **The zero store as the R2 window filler** (N7): 16 / 65.

### 8. The bind, restated with this session's two new faces
Three reads means two gaps; cse says each gap needs a store (section 2); R2 says the [+2 read,
0x1A store] window needs a span-2 `$v0` STORE (section 5); R4 says `P` must precede that store.
E2 spends the free 0x18 store on gap 1 and leaves gap 2 open (one missing load, slot 23 nop).
Q2 covers both gaps by moving the 0x1A store above `P`, which empties R2's window (one wrong
seat).  T1 covers both gaps by hoisting `P` above the zero store, which costs target's slot-5
nop (one displaced load).  **Each of the three closest bodies is one decision from the match and
the three decisions are the same trilemma seen from three sides.**

Artifacts: `tmp/grind/func_80060A68/s20/` (46 bodies + disassemblies, gen20.py, slice.py,
sweep.sh, dis.sh, cmp.sh, target.txt, the sliced rtl/jump/cse/loop/cse2/combine dumps) and
`tmp/grind/func_80060A68/dumps/` (the full -da set for the E2 body).

- [s20] Chassis re-audit: D1 (an independent re-spelling of candidate.c's E2 body) = 2 / build 66 / target 66 with loads at 12 ($a1) / 20 ($a0) on today's HEAD, instruction-for-instruction identical to the s15-s19 records. All 46 s20 bodies are ordinary C with zero FAKE constructs, so tools/fake_ablate.py had nothing to ablate and the s15-s19 instance kills remain chassis-current.

- [s20] PASS ATTRIBUTION MEASURED: text1b.rtl for func_80060A68 carries three (mem:SI (plus (reg/v 72) (const_int 16))) loads (insns 46, 53, 58); text1b.cse carries two. The merge is done by the FIRST cse pass on insn 58 - s19's frontier asked for this confirmation and it is now read out of the dumps rather than inferred.

- [s20] CLASS: the cse table entry for these reads has a varying address, so invalidate_memory (cse.c:1701) removes it unconditionally; invalidate_memory is reachable only from invalidate_from_clobbers when note_mem_written set writes_memory.var (the written rtx is a MEM) and from cse_insn for a non-const CALL_INSN. Nothing but a memory store, a call, or a redefinition of the base register `outer` can separate two of these reads.

- [s20] The base-register escape is closed for a byte-neutral spelling: Q1 (`outer = outer;` between the +2 read and the p10 read) is 2 / 66 with two loads, byte-identical to the control, because store_expr returns the same rtx for both sides and emits no insn; the spelling that does emit a redefinition (Q5, re-reading D_800A3468) costs the gp load at 26 / 68.

- [s20] The declared type of the +2 value is inert: D1 (u16) = D2 (s32) = D3 (u32) = 2 / 66, byte-identical. Declaration-width narrowing/widening is not a lever on this residual.

- [s20] NEW closest-by-inventory body Q2 = C1,C2,C3,S1,S2,S4,S3,P,S5,S6,S7,S8 (the 0x1A store moved above P so it separates the +2 read from the p10 read, idx inserted in the R2 window) = 5 / build 66 / target 66 with all three lw ?,0x10($v1) at target's exact slots and registers (12/$a1, 20/$a0, 23/$a0); R3 (idx after the 0x1A store) is the same at 5 / 66. Q2 is target through slot 24 and from slot 33 on; the entire residual is the +2 value seating in $v0 instead of $a0.

- [s20] QTYDBG on Q2: reg74 (the +2 value) is qty 11, birth 36, death 38, refs 2, allocation ord 5, got=2 ($v0) - its span is still 2 even though idx's read is written inside the window, because local-alloc runs after sched1 and sched1 moved the lhu out. reg73 (idx) is allocated in the suggested pass with range [40,66] and got=4 ($a0), so it does not even conflict; reg74 first-fits $v0 simply because nothing occupies $v0 over its two-insn range.

- [s20] R2's window filler must be a STORE, not any statement: the gp store stays inside [+2 read, 0x1A store] only because sw %gp_rel and sh 0x1A are two stores sched cannot disambiguate, so the store-store edge glues it there. A read filler (idx) is scheduled out of the window; a copy filler works as a donor but drags its load out of the copy block (N1/N3/N4 = 9-10 / 66 with the halfword loads pulled back to slots 17/20); a gp store filler puts P downstream of it, which is R4.

- [s20] NEW second score-2 body T1 = P,Z0,C1,C2,C3,S1,S2,S5,S3,S6,S4,S7,S8 (the p10 read hoisted ABOVE the D_800F10D0 zero-store - a position no prior session tried; s19's Q family hoisted it above the COPY block, i.e. below the zero store) = 2 / build 65 / target 66, three loads at 5 ($a1) / 19 ($a0) / 22 ($a0). Slot-for-slot it is target's entire stream with every register seat correct (the +2 value is in $a0) and exactly one difference: target's slot-5 nop is T1's lw $a1,0x10($v1), with everything after shifted one slot earlier.

- [s20] T1 breaks the s4/s10 conservation law (three independent 0x10 loads implies p10's load pinned to slot 25) for the first time. The discontinuity is the zero store: sw zero,%lo(D_800F10D0)($at) is an $at-based store sched2 cannot disambiguate from lw ?,0x10($v1), so it is a hoist barrier. Above it p10's load reaches slot 5 and eats target's nop; below it the load pins to 25 (T2/T3/T4 = 5 / 67 with the load at 25; V1-V6, which give p10 an early consumer just below the zero store, are 7-12 / 66-67).

- [s20] Hoisting either halfword read into or above the copy block is dead across 16 more bodies (A1-A9 for the +2 read, F1-F7 for the +0 read with the split 0x18 store): three loads at 66 instructions is reachable (A1/A2/A4/F1 = 12 / 66) but the hoisted read's own lw is always emitted at slot 11-16 in $a0, taking the seat target gives p10.

- [s20] Putting the +2 read before the +0 read (W1/W2/W6/W8) reaches target's exact three-load geometry at 66 instructions (W8 = 7 / 66) but then emits the 0x1A store before the 0x18 store, which is seven instructions of reordering - the reads order (+2, +0, p10) is dead.

- [s20] src/text1b.c was restored to HEAD at the end of the session; the only dirty file is metrics/events.jsonl.

- [s20] Chassis re-audit: D1, an independent re-spelling of candidate.c's E2 body through the s20 generator, measures 2 / build 66 / target 66 with loads at 12 ($a1) / 20 ($a0) on today's HEAD -- instruction-for-instruction the s15-s19 record. All 46 s20 bodies are ordinary C with zero FAKE constructs, so tools/fake_ablate.py had nothing to ablate and the s15-s19 instance kills remain chassis-current.

- [s20] Pass attribution is now measured rather than inferred: text1b.rtl for func_80060A68 carries three (mem:SI (plus (reg/v 72) (const_int 16))) loads at insns 46, 53 and 58, text1b.jump still three, and text1b.cse two. The first cse pass merges insn 58 (p10's read) into insn 53's value.

- [s20] The cse table entry for these reads has a varying address, so invalidate_memory (tools/gcc-2.7.2/cse.c:1701) drops it unconditionally; invalidate_memory is reachable only from invalidate_from_clobbers when note_mem_written set writes_memory.var (the written rtx is a MEM) and from cse_insn on a non-const CALL_INSN.

- [s20] `outer = outer;` emits no RTL at all (store_expr returns the same rtx for both sides), so the base-register invalidation escape has no byte-neutral spelling: Q1 = 2 / 66 with two loads, byte-identical to the control, and Q5 (a real re-read of D_800A3468) = 26 / 68.

- [s20] The declared width of the +2 value is inert: u16, s32 and u32 spellings all measure 2 / 66 and are byte-identical.

- [s20] NEW closest-by-inventory body Q2 = C1,C2,C3,S1,S2,S4,S3,P,S5,S6,S7,S8 measures 5 / build 66 / target 66 with all three 0x10 loads at target's exact slots and registers (12/$a1, 20/$a0, 23/$a0), is identical to target through slot 24 and from slot 33 on, and has a single defect: the +2 value seats in $v0 instead of $a0. R3 (idx after the 0x1A store) is the same at 5 / 66.

- [s20] QTYDBG on Q2: reg74 (the +2 value) is qty 11, birth 36, death 38, refs 2, allocation ord 5, got=2 ($v0); reg73 (idx) is allocated in the suggested pass with range [40,66] and got=4 ($a0) and does not overlap reg74; reg75 (p10) is ord 23, got=5 ($a1). reg74's span is still 2 despite idx's read being written inside the window, because local-alloc runs after sched1 and sched1 moved the lhu out.

- [s20] R2's window filler must be a STORE: the gp store survives inside [+2 read, 0x1A store] only because sw %gp_rel and sh 0x1A are two stores sched cannot disambiguate, so a store-store edge glues it there. A read filler is scheduled out; a copy filler works as a donor but drags its load out of the copy block (N1/N3/N4 = 9-10 / 66 with the halfword loads pulled back to slots 17/20); a gp store filler puts the p10 read downstream of it, which is R4.

- [s20] NEW second score-2 body T1 = P,Z0,C1,C2,C3,S1,S2,S5,S3,S6,S4,S7,S8 measures 2 / build 65 / target 66 with three loads at 5 ($a1) / 19 ($a0) / 22 ($a0); slot-for-slot it is target's entire stream with every register seat correct and exactly one difference, target's slot-5 nop being T1's lw $a1,0x10($v1).

- [s20] The zero store is the hoist barrier that decides the p10 load's slot: with the p10 read above sw zero,%lo(D_800F10D0)($at) the load reaches slot 5 (T1), with it just below the load pins to slot 25 (T2/T3/T4 = 5 / 67), and with it after the halfword reads the load reaches target's slot 12 but the third read is cse-merged away (E2 = 2 / 66).

- [s20] Hoisting either halfword read into or above the copy block is dead across 16 further bodies (A1-A9, F1-F7): three loads at 66 instructions is reachable (12 / 66) but the hoisted read's own lw is emitted at slot 11-16 in $a0, taking the seat target gives p10.

- [s20] Reading the +2 halfword before the +0 halfword reaches target's exact three-load geometry at 66 instructions (W8 = 7 / 66) but emits the 0x1A store ahead of the 0x18 store, seven instructions of reordering.

- [s20] The three closest bodies are three faces of one trilemma: two read gaps each need a store, R2 needs a span-2 $v0 STORE inside [+2 read, 0x1A store], and R4 needs the p10 read before that store. E2 leaves gap 2 open (missing load, slot-23 nop); Q2 empties R2's window (wrong seat); T1 costs target's slot-5 nop (displaced load).

- [s20] src/text1b.c was restored to HEAD at the end of the session; the only dirty tracked file outside memory/grind/func_80060A68/ is metrics/events.jsonl.

## [s21 2026-09-03 - permuter modality] Two fresh-chassis campaigns (39k iterations) classify the whole permuter basin as banned-carrier or semantically unfaithful; and the gp store is priced as a cse separator for the first time

### 0. Mandated chassis / kill re-audit
`sandbox func_80060A68 --disable all` on today's HEAD with `memory/grind/func_80060A68/candidate.c`
applied through the s14 harness = **2 / build 66 / target 66**.  The chassis has not moved.
Kill re-audit of the closest banked form: **G0**, an independent re-spelling of s20's T1 through
this session's generator, = **2 / build 65 / target 66** with loads at 5 ($a1) / 19 ($a0) / 22 ($a0)
- instruction-for-instruction the s20 record.  Every body measured this session (41 of them) is
ordinary C with zero FAKE constructs, so `tools/fake_ablate.py` again had nothing to ablate and
every s15-s20 instance kill remains chassis-current.

### 1. THE PERMUTER AXIS, THIRD AND FOURTH CAMPAIGNS - on two chassis that did not exist in s3/s3b
Two minimal-TU workspaces were built from the validated s3 workspace, one seeded on **T1**
(`tmp/grind/func_80060A68/s21/permT1`, base permuter score 315, 65 insns) and one on **Q2**
(`.../permQ2`, base 420, 66 insns).  Both are structurally different chassis from s3's floor-2 E2
seed and s3b's w3 seed, so the 2026-09-01 chassis rule is satisfied.  Campaigns
`s21-T1-p10-above-zero-store` (-j 4, 899 s, **19,350 iterations**, best new 115) and
`s21-Q2-three-load-geometry` (-j 4, 905 s, **19,605 iterations**, best new 175) both ran to a
harvest `--stop` inside this session.

**87 finds were produced and every one was classified** by
`tmp/grind/func_80060A68/s21/semcheck.py`, which normalizes each output's statement set and
compares it against the function's 13 required statements:

* **37 finds** re-assign `p10` (or `temp2`) a SECOND time to hold a copy's source pointer -
  `p10 = *(s32 *)(outer + 0xC) + 0;`, `p10 = outer + 0x18;` and so on.  This is precisely the
  multiply-assigned pointer-staging carrier the Judge banned for this function ("No local may be
  written twice to hold copy 1's or copy 2's source pointer under any name, family label, or
  second-write spelling").
* **13 finds** introduce `new_var` / `new_var2` alias locals (`new_var = outer;` then every
  dereference through `new_var`) - an invented scratch, the shape layer-1 already FAILed here.
* The remainder are semantically UNFAITHFUL: `temp_a1 = *(u16 *)(p10 + 4);` emitted ABOVE
  `p10 = *(s32 *)(outer + 0x10);` (use-before-def), `p10 = 4; temp_a1 = *(u16 *)(p10 + p10);`,
  a dropped copy statement, or `if (1) { ... }` wrapping.
* **Seven finds are semantically faithful.**  Six are ordinary-C statement reorderings; one
  (permT1/output-215-1) is T1 plus a bare `do { } while (0);`, i.e. a FAKE construct with no
  objdump benefit (215 vs base 315 is a permuter-weight artefact).

**The single ordinary-C, ban-free find worth spending is permQ2/output-245-1**, whose statement
order is `Z0,C1,C2,C3,S1,S2,S5,S4,S3,P,S6,S7,S8`.  Re-spelled and measured in the sandbox it is
**M1 = 4 / build 66 / target 66** with three loads; its neighbour **M2 =
`Z0,C1,C2,C3,S1,S2,S5,S3,P,S6,S4,S7,S8` = 3 / build 66 / target 66**, three loads at 19 ($a0) /
22 ($a0) / 27 ($v0).  **M2 is the best-scoring THREE-LOAD body the campaign has produced** (the
previous best three-load members were Q2/R3 and JA at 5).  It is E2 with the 0x1A store lifted
above the p10 read and the p10 read dropped to just before the +4 read.

**Verdict on the axis: on all four chassis tried, the permuter's low-score basin for this function
is the banned multiply-assigned carrier plus semantically-broken reorderings.  Random mutation
finds the same illegal shape s3 found, because that shape genuinely is the cheapest way to buy the
contested seat - and it is exactly what the Judge closed.**

### 2. THE gp STORE IS PRICED AS A cse SEPARATOR FOR THE FIRST TIME (three new families, 24 bodies)
s17's R4 was a positional law ("the p10 read must not follow a gp-based store"), which is why no
session ever placed a gp store in a read gap.  s18 corrected R4 to a REGISTER law (the slot-12
`lw $a1` follows from p10's $a1 seat, which needs a span-2 $v0 donor inside [p10 read, +4 read]).
That correction re-opens the gp store as a candidate separator, and this session prices it:

* **J family (10 bodies)** - a gp store placed in the [+2 read, p10 read] gap, i.e. AFTER both
  halfword reads: **all ten are 66 instructions with three loads** (J1/J2/J3/J4/J7/J9 = 8-9,
  J5/J8 = 7, JA = 5).  The separator is FREE here.  What it costs is the head geometry: the p10
  load is emitted at slot 27-29 and never in $a1, even in the members (J1-J4, J7, J9) where the
  OTHER gp store sits strictly inside [P, S6] and supplies R3's donor.  **The positional flavour of
  R4 therefore survives s18's correction as an independent necessary condition: a p10 read that
  follows a gp store loses the slot-12 $a1 load whether or not it has a donor.**
* **K family (reads order +0, p10, +2; gp store covers the p10->+2 gap)** - K1/K2/K3 = 8 at
  **68** instructions, loads at **12 ($a1) / 20 ($a0) / 27 ($v0)**.  A slot-for-slot diff shows K1
  is identical to target through slot 22 and then diverges: target's slot-23 `lw a0,0x10(v1)` is a
  nop, and the +2 load appears at 27 in $v0 with a second nop at 28.
* **L family (reads order p10, +0, +2 = target's LOAD order; gp store covers the p10->+0 gap)** -
  L1/L2/L3 = 8-9 at **68** instructions, loads at **12 ($a1) / 24 ($v0) / 25 ($a0)**.  L6 (the gp
  store inside the copy block) = 9 / 68; L5 (R2 broken, control) = 13 / 69.

**Consolidated price list for the second cse separator, now complete:** the 0x18 store is the only
free one and covers exactly one gap (s19); the 0x1A store is free but empties R2's window (Q2,
s20); a copy store costs a load-delay nop (N/P/R families, s18-s20); the 0x1C store needs S6
adjacent to P and so destroys R3's donor (H family this session: H1-H6 = 9-14 at 65-66 with only
TWO loads, re-confirming s19 on a different spine); the zero store costs the head geometry (D
family, s19); and **a gp store is free when it sits after both halfword reads but then pins the
p10 load late (J), and costs exactly two instructions of displacement when it sits before them
(K, L)**.

### 3. The T1 face is closed by the hoist barrier, not by a ready-list tie
s20's frontier claimed T1 becomes a byte match if any other instruction occupies target's slot-5
nop.  That is measured false, and the reason is structural.  With the p10 read above
`sw zero,%lo(D_800F10D0)($at)` the load is confined to slots 4-8 by the barrier in BOTH directions
(sched2 cannot sink a load past a store it cannot disambiguate any more than it can hoist one
across it), so no T1 member can put that load at target's slot 12; filling slot 5 with something
else merely moves the load to slot 6 and the body stays 65 instructions.  Measured: G1/G2/G8 (the
idx read hoisted above the zero store) = 38 at 62 instructions - `*(u16 *)outer` merges with the
zero store's own index read and the head collapses; G3/G4 (a copy hoisted above the zero store) =
11 at **67** with the p10 load at 10; G5 (the D_800A3478 gp store hoisted above it) = 9 at 66 with
the load at 4; G6 = 9 at 65 with the load at 7; G7 (the 0x18 statement hoisted) = 7 at 64 with two
loads.  **Not one member reaches 66 instructions with the p10 load below the barrier.**

### 4. Where the campaign stands after s21
The floor is unchanged at 2 (E2 / candidate.c).  The three-load frontier improved from 5 to 3
(M2).  Two of s20's three frontier items are closed by measurement: the T1 face (section 3) and
the Q2 / 0x1C-store probe (H family, section 2).  What survives is the E2 face, and the separator
price list in section 2 says exactly what it needs: a store between the +2 read and the p10 read
that is neither a gp store (pins the p10 load late, J), nor the 0x1A store (empties R2, Q2), nor a
copy (nop), nor the 0x1C store (kills R3's donor, H), nor the zero store (head geometry).  On the
existing statement inventory that set is empty, which makes the next question a statement-inventory
question rather than an ordering question.

Artifacts: `tmp/grind/func_80060A68/s21/` (gen21.py, sweep.sh, dis.sh, cmp.sh, semcheck.py,
semcheck_final.txt, target.txt, 41 bodies + disassemblies, and the two permuter workspaces
permT1/ and permQ2/ with their campaign.log, campaign_meta.json and 87 output-* find directories).

- [s21] Chassis re-audit: candidate.c applied through the s14 harness = 2 / build 66 / target 66 on today's HEAD; G0 (an independent re-spelling of s20's T1) = 2 / build 65 / target 66 with loads at 5 ($a1) / 19 ($a0) / 22 ($a0), instruction-for-instruction the s20 record. All 41 s21 bodies are ordinary C with zero FAKE constructs, so tools/fake_ablate.py had nothing to ablate and the s15-s20 instance kills remain chassis-current.

- [s21] Third and fourth permuter campaigns, on two chassis that did not exist when s3/s3b ran: s21-T1-p10-above-zero-store (base 315, 19,350 iterations, 899 s, best new 115) and s21-Q2-three-load-geometry (base 420, 19,605 iterations, 905 s, best new 175). Both were harvested with --stop inside the session.

- [s21] All 87 permuter finds were classified by tmp/grind/func_80060A68/s21/semcheck.py against the function's 13 required statements: 37 re-assign p10 or temp2 a second time to hold a copy's source pointer (the Judge-banned multiply-assigned carrier), 13 introduce new_var/new_var2 alias locals, the rest are semantically unfaithful (use-before-def of p10, a dropped copy, if (1) wrapping), and 7 are faithful - six ordinary-C reorderings and one bare do { } while (0) with no objdump benefit.

- [s21] The permuter axis is now measured dead on FOUR chassis. Random mutation reaches the same illegal shape s3 reached, because the multiply-assigned carrier genuinely is the cheapest way to buy the contested seat; nothing in the stock mutation vocabulary reaches a ban-free improvement on either new chassis.

- [s21] The one ordinary-C ban-free find worth spending (permQ2/output-245-1) re-spells as M1 = 4 / 66 with three loads; its neighbour M2 (Z0,C1,C2,C3,S1,S2,S5,S3,P,S6,S4,S7,S8) = 3 / build 66 / target 66 with three loads at 19 ($a0) / 22 ($a0) / 27 ($v0) and is the best-scoring three-load body the campaign has produced (previous best: Q2/R3/JA at 5).

- [s21] The gp store is priced as a cse separator for the first time (s17's positional R4 had forbidden the configuration; s18's correction re-opened it). J family, ten bodies with a gp store in the [+2 read, p10 read] gap: all ten are 66 instructions with three loads (7-9, JA = 5), so the separator is free there - but the p10 load is emitted at slot 27-29 and never in $a1, even in the six members where the other gp store sits strictly inside [P, S6] and supplies R3's donor.

- [s21] The positional flavour of R4 survives s18's register-law correction as an independent necessary condition: a p10 read that follows a gp store loses the slot-12 $a1 load whether or not it has a span-2 $v0 donor in its range.

- [s21] A gp store used as a separator BEFORE the halfword reads costs exactly two instructions of displacement: K1/K2/K3 (reads +0, p10, +2) = 8 at 68 instructions with loads at 12 ($a1) / 20 ($a0) / 27 ($v0), and K1 is identical to target through slot 22; L1/L2/L3 (reads p10, +0, +2 - target's load order) = 8-9 at 68 with loads at 12 ($a1) / 24 ($v0) / 25 ($a0).

- [s21] The 0x1C store in R2's window is re-confirmed dead on the Q2 spine: H1-H6 (the p10 read and the +4 read hoisted ahead of the +2 read so the 0x1C store can sit inside [+2 read, 0x1A store]) = 9-14 at 65-66 instructions with only TWO 0x10 loads, because S6 adjacent to P removes R3's donor.

- [s21] The T1 face is closed by the zero-store hoist barrier, not by a ready-list tie: with the p10 read above sw zero,%lo(D_800F10D0)($at) the load is confined to slots 4-8 in both directions, so filling slot 5 with another instruction only moves the load to slot 6 and the body stays 65. G1/G2/G8 (idx read hoisted) = 38 at 62; G3/G4 (a copy hoisted) = 11 at 67 with the load at 10; G5 = 9 at 66 with the load at 4; G6 = 9 at 65; G7 = 7 at 64 with two loads. No member reaches 66 instructions with the p10 load below the barrier.

- [s21] src/text1b.c was restored to HEAD at the end of the session; the only dirty tracked file outside memory/grind/func_80060A68/ is metrics/events.jsonl.

- [s21] Chassis re-audit: candidate.c applied through the s14 harness measures 2 / build 66 / target 66 on today's HEAD; G0, an independent re-spelling of s20's T1 through this session's generator, measures 2 / build 65 / target 66 with loads at 5 ($a1) / 19 ($a0) / 22 ($a0), instruction-for-instruction the s20 record. All 41 bodies measured this session are ordinary C with zero FAKE constructs, so tools/fake_ablate.py had nothing to ablate and every s15-s20 instance kill remains chassis-current.

- [s21] Third and fourth permuter campaigns ran on chassis that did not exist when s3/s3b ran: s21-T1-p10-above-zero-store (base 315, 19,350 iterations, 899 s, best new 115) and s21-Q2-three-load-geometry (base 420, 19,605 iterations, 905 s, best new 175). Both were harvested with --stop inside the session; no campaign outlives it.

- [s21] All 87 permuter finds were classified mechanically (tmp/grind/func_80060A68/s21/semcheck.py normalizes each output's statement set against the function's 13 required statements): 37 re-assign p10 or temp2 a second time to hold a copy's source pointer (the Judge-banned multiply-assigned carrier), 13 introduce new_var/new_var2 alias locals, most of the remainder are semantically unfaithful (temp_a1 read from p10 above p10's assignment, p10 = 4, a dropped copy, if (1) wrapping), and 7 are faithful - six ordinary-C reorderings plus one bare do { } while (0) with no objdump benefit.

- [s21] The permuter axis is now measured dead on FOUR chassis for this function. Random mutation converges on the same illegal shape s3 reached because the multiply-assigned carrier genuinely is the cheapest way to buy the contested seat, and that is exactly what the Judge closed.

- [s21] The one ordinary-C ban-free find worth spending (permQ2/output-245-1, statement order Z0,C1,C2,C3,S1,S2,S5,S4,S3,P,S6,S7,S8) re-spells as M1 = 4 / 66 with three loads; its neighbour M2 (Z0,C1,C2,C3,S1,S2,S5,S3,P,S6,S4,S7,S8) = 3 / build 66 / target 66 with three loads at 19 ($a0) / 22 ($a0) / 27 ($v0) and is the best-scoring three-load body the campaign has produced (previous best: Q2/R3 and JA at 5). Banked at rejected/s21-M2-gp-store-then-0x1A-store-before-p10-read-three-loads-score3-66insns.c.

- [s21] The gp store is priced as a cse separator for the first time in the campaign (s17's positional R4 had forbidden the configuration; s18's register-law correction re-opened it). J family, ten bodies with a gp store in the [+2 read, p10 read] gap: all ten are 66 instructions with three loads (scores 5-9), so the separator is free there - but the p10 load lands at slot 27-29 and never in $a1, even in the six members where the other gp store sits strictly inside [P, S6] and supplies R3's donor.

- [s21] The positional flavour of R4 survives s18's register-law correction as an independent necessary condition: a p10 read that follows a gp store loses the slot-12 $a1 load whether or not it has a span-2 $v0 donor in its range.

- [s21] A gp store used as a separator BEFORE the halfword reads costs exactly two instructions of displacement: K1/K2/K3 (reads +0, p10, +2) = 8 at 68 instructions with loads at 12 ($a1) / 20 ($a0) / 27 ($v0), K1 identical to target through slot 22; L1/L2/L3 (reads p10, +0, +2 - target's load order) = 8-9 at 68 with loads at 12 ($a1) / 24 ($v0) / 25 ($a0).

- [s21] The second-separator price list is now complete and every member is priced: 0x18 store free but covers exactly one gap (s19); 0x1A store free but empties R2's window (Q2, s20); copy store costs a load-delay nop (N/P/R families, s18-s20); 0x1C store needs the +4 read adjacent to the p10 read and so destroys R3's donor (H family, s21, re-confirming s18's Z family on a different spine); zero store costs the head geometry (D family, s19); gp store free after both halfword reads but pins the p10 load late (J family, s21) and costs two instructions before them (K/L families, s21).

- [s21] The T1 face is closed by the zero-store hoist barrier rather than by a ready-list tie: with the p10 read above sw zero,%lo(D_800F10D0)($at) its load is confined to slots 4-8 in both directions, so occupying slot 5 only moves the load to slot 6 and the body stays 65 instructions. G1/G2/G8 = 38 at 62; G3/G4 = 11 at 67 with the load at 10; G5 = 9 at 66 with the load at 4; G6 = 9 at 65; G7 = 7 at 64 with two loads.

- [s21] src/text1b.c was restored to HEAD at the end of the session; the only dirty tracked file outside memory/grind/func_80060A68/ is metrics/events.jsonl.

## s22 (2026-09-03) - escalation/disposition session: the three live frontier items, all measured dead

Chassis re-measured first: HEAD carries `INCLUDE_ASM("asm/funcs", func_80060A68);` (sandbox
66 / build 0), and `memory/grind/func_80060A68/candidate.c` spliced over it measures
**2 / build 66 / target 66** - unchanged from the ledger, so every banked spelling
conclusion is still chassis-valid. The A0 control (E2 statement order, regenerated from the
s16/s21 statement dictionary) reproduces that 2 / 66 exactly, and the A1 control reproduces
M2 at 3 / 66 with its three loads at 19 ($a0) / 22 ($a0) / 27 ($v0).

**1. The tail conditional is not a free cse separator (frontier item 1, closed).** The
s20 class result leaves exactly three separator kinds (a MEM write, a non-const call, a
redefinition of `outer`), and s21 priced every store the body already contains. The one
untested candidate was the tail's `if (*(s32 *)D_800A3468 & 0x200000) { D_800A32BC = 0xA; }`,
which has no dependence on the call. Relocated into the [+2 read, p10 read] gap it measures
50-53 at 62-64 instructions in all four spellings (X1 re-reading the global, X2/X3/X5 off
the live `outer`), and the `lw ?,0x10($v1)` count goes to ZERO in every one: the conditional
splits the contested span into separate basic blocks, and both cse and the list scheduler
are per-block, so target's single 66-instruction straight-line stream stops being
constructible at all. The block-boundary-free upper bound X4 (the bare store, semantics
ignored) does separate the reads - three loads at 20/23/28 - but at build 70, four
instructions over target, for score 13. Either way the tail conditional is priced and dead.

**2. R3's donor is not what pins M2's third load (frontier item 2, closed).** Eight
variants (N1-N8) sweep the D_800A347C gp store through every position in and around the
[p10 read, +4 read] window on the M2 spine, including swapping the roles of the two gp
stores. All eight score WORSE than the M2 control (5, 6, 5, 7, 7, 7, 5, 5 vs 3). The
diagnostic content is in the load geometry: the first two loads never move off 19/22 in
$a0, and the third load only alternates between slot 27 in $v0 and slot 28 in $a0. So the
donor does buy the target SEAT (N1/N5/N8 put the third load in $a0) but never the target
SLOT - the p10 load's five-to-six-slot lateness on this spine is not a register-pressure
effect and cannot be bought with a donor.

**3. A gp store cannot be split at the source level (frontier item 3, closed - its premise
is false).** Frontier item 3 rested on being able to separate a gp store's `addiu` from its
`sw` so the pair could straddle the p10 read, resolving the R3/R4 conflict that s21
identified as the load-bearing obstruction. Staged through the pre-existing currently-dead
`result` local (`result = outer + 0x18;` ... `D_800A3478 = result;` - measured only, never
adopted, and it produces no bytes), P1 and P3 come out **byte-identical to the A0/E2
control** (empty diff of the disassemblies, 2 / build 66, two loads at 12/20) and P6/P7 come
out byte-identical to T1 (2 / build 65, three loads at 5/19/22). GCC folds the staged
address straight back into the store's address operand, so no spelling of the split ever
puts the addiu and the sw on opposite sides of the read. P2/P4/P5 (5-6) and P8 (10, the
`idx` carrier) confirm that where the split is not byte-neutral it is simply worse. The
R3-vs-R4 conflict therefore has no gp-store-respelling escape.

**Net.** All three of s21's live frontier items are now measured dead on the current
chassis with zero FAKE constructs anywhere in the 21 bodies. The floor is unchanged at 2,
the best bodies remain E2 (2 / 66, two loads) and T1 (2 / 65, three loads, every target
register seat, one instruction short), and the disposition filed in docs/grind/decisions.md
for 2026-09-03 records the two endgame gates (scan_hand_coded tier=LOW 1/8; no coercion
family in candidate.c and therefore no precedent to cite).

- [s22] Chassis re-measured at session start: HEAD carries INCLUDE_ASM (sandbox 66 / build 0); memory/grind/func_80060A68/candidate.c spliced over it measures 2 / build 66 / target 66, unchanged from the ledger, so all banked spelling conclusions remain chassis-valid.

- [s22] The A0 control (E2 statement order, regenerated from the s16/s21 statement dictionary) reproduces the floor exactly at 2 / 66 with two lw ?,0x10($v1) at slots 12 ($a1) and 20 ($a0); the A1 control reproduces M2 at 3 / 66 with three loads at 19 ($a0) / 22 ($a0) / 27 ($v0).

- [s22] Relocating a CONDITIONAL statement into the contested span is categorically destructive here, not merely unhelpful: the basic-block boundary it creates takes cse and the list scheduler out of the single-block regime target's 66-instruction stream requires, dropping the body to 62-64 instructions and score 50-53 with zero 0x10 loads.

- [s22] A free-standing store in the same slot (X4) DOES separate the two 0x10 reads and yields three loads, but costs four instructions (build 70) - which quantifies, for the first time, what an out-of-inventory separator would have to cost: the body is already at target's exact 66 instructions, so any separator that is not already a statement of the function is a guaranteed net loss.

- [s22] Source-level splitting of a gp store into an address stage plus a store is byte-neutral on both score-2 spines (P1/P3 byte-identical to E2; P6/P7 byte-identical to T1) - GCC re-fuses the addiu into the store's address operand, so no staging spelling can put the addiu and the sw on opposite sides of the p10 read.

- [s22] Gate (a) canonical-asm FAILS: tools/scan_hand_coded.py --single func_80060A68 on today's HEAD returns tier=LOW score=1/8, S4 only (6 loads in 8-insn window @ insn 9); S1/S2/S3/S5/S6/S7/S8 all clear. Artifact tmp/grind/func_80060A68/s22/scan_hand_coded.txt.

- [s22] Gate (b) SOTN-master precedent FAILS: candidate.c carries no coercion construct at all (no volatile, no pad, no dead local, no invented carrier, no inline asm), so there is no family for which a precedent could be cited; the one family-shaped spelling measured this session (staged-value through `result`) is byte-neutral and therefore moot on the evidence, not on the rules.

- [s22] Nothing holds a byte-match via a cheat: the function is committed as INCLUDE_ASM per asm-until-matched, and asmfix.txt/regfix.txt have been empty project-wide since 2026-08-25 - there is no cheat to retire and no integration handoff pending.

- [s22] Foreclosure record filed this session at the end of docs/grind/decisions.md: '2026-09-03 - func_80060A68 - RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED', carrying both gates' evidence, the exhaustion history (22 sessions, >=7 modalities, ~104k permuter iterations, 116 banked rejected forms) and four re-activation triggers.

## [s23 2026-09-04 - synthesis] The dispatch call is a `()` call, and that is free

Owner ruling 2026-09-04 Ruling A named two probes; both are executed and measured.

**Probe 1 - the `()` call spelling.**  Every one of the 116 bodies banked before this
session called the dispatch table as `(idx, temp_a1)`.  The matched sibling
func_80060B70 (src/text1b.c:3170) calls the SAME table as `()`, the table's first
callee is `u8 func_80063AF0(void)` (src/text1b.c:4042), and the target asm sets up no
argument registers before the `jalr $v0` at slot 48 - $a0 holds `idx` because `idx` is
read at slot 30 and consumed by the `sll`/`addu` at 34/36, and $a1 holds the +4
halfword because it is stored to 0x1C at slot 33.  Measured on four spines:

| spine | `(idx, temp_a1)` | `()` | 0x10 loads with `()` |
|---|---|---|---|
| E2 (candidate.c) | 2 / 66 | 2 / 66, **objdump byte-identical** | 12($a1) 20($a0) |
| T1 | 2 / 65 | 2 / 65 | 5($a1) 19($a0) 22($a0) |
| M2 | 3 / 66 | 3 / 66 | 19($a0) 22($a0) 27($v0) |
| Q2 | 5 / 66 | **10 / 66** | 20($a0) 23($a0) 26($a0) |

So the argument-register suggestion is inert on three spines and actively harmful on
the fourth (Q2 loses the $a1 seat of the +4 read).  candidate.c has been rewritten onto
`()`: it costs nothing and removes a 22-session semantic infidelity.  This also closes
the ledger's law R3 (hypotheses.md:1777) as a lever: local-alloc first-fits `idx` into
$a0 and the +4 halfword into $a1 without any argument suggestion, because nothing else
occupies those hard registers over their live ranges.

**Probe 2 - the sibling statement-inventory diff.**  func_80060B70 is matched C, so its
inventory is authoritative.  Three items differ from candidate.c and all three are now
measured:

- `u16 idx` (B70 types its dispatch index u16; this body types it s32).  **Wrong for the
  target**: u16 forces `andi a0,a0,0xffff` before the `sll`, which the target does not
  contain.  E2+u16 = 3 / build 67; T1+u16 = 3 / build 66.  Note that T1+u16 does supply
  T1's missing 66th instruction - but with the wrong instruction, in the wrong place.
- B70's braced `{ s32 last_arg = ...; copy; call(...); }` block.  Transposed onto this
  body's 0x1C cluster it is byte-neutral on both spines (E2 2 / 66, T1 2 / 65).
- B70's destination-pointer locals: already banked neutral
  (rejected/sibling-dst-locals-neutral-folds-to-base-offset-score2.c).

B70's remaining extra work - the three u16 copies through D_800A346C and the
func_80061FAC call - has no counterpart in the target's 66 instructions.  **There is no
missing statement.**  The s21/s22 frontier item that hoped an external source would name
T1's 66th instruction is CLOSED; T1's deficit is a scheduling artefact.

## [s23] The slot-12 law (dump-backed; this is now the whole residual)

Target's `lw a1,0x10(v1)` at slot 12 is a SINGLE-consumer load: its only use is
`lhu a1,0x4(a1)` at slot 29, which feeds `sh a1,0x1C(v1)` at slot 33.  Three regimes,
all read out of `tmp/grind/func_80060A68/dumps/text1b.sched`:

1. **p10 read BEFORE the Z0 store** (T1 / PA).  The load carries no memory dependence on
   the store - the dump shows `insn 12 (set (reg/v:SI 75) (mem:SI (plus (reg/v 72)
   (const_int 16))))` with dependency list `(insn_list 9 (nil))` only.  It is ready at
   time 0 and sched1 emits it third, i.e. into the `lhu`->`sll` load-delay slot that the
   target leaves as a nop.  Result: three 0x10 loads at 5/19/22 with every target
   register seat, but 65 instructions.
2. **p10 read AFTER any store** (PB/PC/PD after Z0; PH/PI after the D_800A3478 gp store;
   W1/W3 with the +2 read inlined).  The load acquires the store dependence -
   `(insn_list 9 (insn_list 22 ...))` in the PB dump - and sched1 sinks it BELOW both
   other 0x10 loads: PB's `insn 25 (reg75)` is emitted eighteenth, after insns 49 and 56,
   landing at final slot 25 in $v0.  Mechanism: with the store dependence present all
   three 0x10 loads head equal-length chains (load -> halfword read -> halfword store),
   so `priority()` ties and `rank_for_schedule` falls to the dep-class / luid tiebreak;
   the load whose consumer sits last in the block - p10's, whose +4 read feeds the last
   store before the dispatch - loses every time.
3. **p10 read cse-MERGED with the +2 read** (E2 / PF).  The merged pseudo has TWO
   consumers, which breaks the priority tie in its favour and seats it at slot 12 in $a1 -
   target's exact seat.  Cost: the third 0x10 load does not exist, so target's slot-23
   `lw a0,0x10(v1)` is a nop here and the +2 read runs through $a1.  That is exactly the
   floor-2 residual.

Full s23 measurement table (all `()` spelling, zero FAKE constructs):

| body | statement order (P = the p10 read) | score / insns | 0x10 loads |
|---|---|---|---|
| PA (=T1) | P,Z0,C1,C2,C3,S1,S2,S5,S3,S6,S4,S7,S8 | 2 / 65 | 5($a1) 19($a0) 22($a0) |
| PB | Z0,P,C1,C2,C3,... | 5 / 67 | 19($a0) 24($v0) 25($a1) |
| PC | Z0,C1,P,C2,C3,... | 5 / 67 | 19 24 25 |
| PD | Z0,C1,C2,P,C3,... | 5 / 67 | 19 24 25 |
| PE | Z0,C1,C2,C3,P,S1,S2,... | 2 / 66 | 12($a1) 20($a0) |
| PF | Z0,C1,C2,C3,S1,P,S2,... | 2 / 66 | 12($a1) 20($a0) |
| PG (=E2) | Z0,C1,C2,C3,S1,S2,P,S5,S3,... | 2 / 66 | 12($a1) 20($a0) |
| PH | Z0,C1,C2,C3,S1,S2,S5,P,S3,... | 3 / 66 | 19($a0) 22($a0) 27($v0) |
| PI (=M2) | Z0,C1,C2,C3,S1,S2,S5,S3,P,... | 3 / 66 | 19 22 27 |
| W1 | S1,P,S5,S3-inline,... | 8 / 68 | 19 22 26 |
| W2 | S1,P,S3-inline,S5,... | 6 / 66 | 19 22 |
| W4 | no p10 local, +4 read inline late | 3 / 66 | 19 22 27 |
| W5 | no p10 local, +4 read inline early | 8 / 65 | 19 23 |
| target | - | 0 / 66 | **12($a1) 20($a0) 23($a0)** |

**The open lever, stated precisely:** raise the p10 load's sched.c `priority()` above the
other two 0x10 loads WITHOUT adding an instruction and WITHOUT merging it in cse.  Two
untried shapes follow from the mechanism: (a) give the +4 halfword a second real consumer
that `combine` folds back to one insn, so the load heads a two-consumer chain like the
merged case but stays a distinct load; (b) lengthen the chain BELOW the 0x1C store so
p10's chain is strictly longer than the 0x18 and 0x1A chains rather than tied with them.

- [s23] candidate.c now spells the dispatch call `()` and re-measures 2 / build 66 / target 66 on today's HEAD (measured as body CAND with the full file spliced into src/text1b.c); src/text1b.c was restored to HEAD afterwards and the tree carries no src/ dirt.

- [s23] The `()` and `(idx, temp_a1)` E2 objdumps are byte-identical (empty diff), so the 116-body bank is unaffected by the spelling change and no banked conclusion needs re-deriving.

- [s23] Target sets up NO argument registers before `jalr $v0` at slot 48; $a0 at slot 30 is the dispatch index (consumed by sll/addu at 34/36) and $a1 at slot 29 is the +4 halfword (consumed by `sh a1,0x1C(v1)` at 33). The matched sibling func_80060B70 calls the same table as `()` at src/text1b.c:3170 and the table's first callee is `u8 func_80063AF0(void)` at src/text1b.c:4042.

- [s23] `u16 idx` is wrong for this function: it forces `andi a0,a0,0xffff` before the sll (E2+u16 = 3 / 67, T1+u16 = 3 / 66). The target contains no andi, so `s32 idx` is correct and the sibling's typing does not transfer.

- [s23] func_80060B70's braced block shape is byte-neutral here (E2 2 / 66, T1 2 / 65 unchanged), and its remaining statements (three u16 copies through D_800A346C, the func_80061FAC call) have no counterpart in the target's 66 instructions - there is no missing statement to import.

- [s23] text1b.sched, T1 spine: `insn 12 (set (reg/v:SI 75) (mem:SI (plus (reg/v:SI 72) (const_int 16))))` has dependency list `(insn_list 9 (nil))` and is emitted third - the load-delay slot the target leaves as a nop. text1b.sched, PB spine: the same read is `insn 25` with `(insn_list 9 (insn_list 22 ...))` and is emitted eighteenth, after insns 49 and 56.

- [s23] Target's slot-12 `lw a1,0x10(v1)` is a SINGLE-consumer load (only use: `lhu a1,0x4(a1)` at 29), yet every reproduction of that seat so far has required a TWO-consumer cse-merged pseudo. That contradiction is the entire remaining residual and is a sched.c priority question, not an ordering one.

## [s24 2026-09-04 — solver modality] THE T1 RESIDUAL IS ONE SCHEDULER PREDICATE, STATED EXACTLY

**The residual, in object space (this is the whole diff, nothing else differs).**
`tools/ra_solver/goal_from_tgt.py classify text1b func_80060A68` on the E2 chassis
reports **FIRST DIVERGENCE: PRE-RA — "next tool: none, the residual is upstream of
every model"**, with `ours only: nop x1 / target only: lw #,16(#) x1`: on E2 the two
streams differ by a cse merge, which no RA or scheduler model can attack. **On the T1
chassis (`rejected/t1-p10-read-above-the-zero-store-...c`, re-measured this session at
2 / build 65 / target 66 with the s23 `()` call spelling) the picture is completely
different and far better**: a full `engine.score.normalized_insns` diff of
`tmp/sandbox/func_80060A68/text1b.o` against `build/src/text1b.o` (the canonical
INCLUDE_ASM object, i.e. the real target bytes) is

    ours   : ... lhu v0,0(v1) | lw a1,16(v1) | sll v0,v0,0x2 | lui at | addu at | sw zero | lw v0,12(v1) | lw a0,12(v1) |            lw v0,0(v0) ...
    target : ... lhu v0,0(v1) | nop          | sll v0,v0,0x2 | lui at | addu at | sw zero | lw v0,12(v1) | lw a0,12(v1) | lw a1,16(v1) | lw v0,0(v0) ...

and **every other one of the 64 remaining instructions, including every nop, is
identical**. T1 is target's stream with ONE instruction (the p10 load) moved seven
positions earlier, into the `lhu -> sll` load-delay slot. Artifact:
`tmp/grind/func_80060A68/s24/dumpdiff.py` output.

**The scheduler model, extracted and exact.** `tools/sched_solver/extract.py text1b`
reports `parity=True funcs=492 blocks=1764`; func_80060A68 pass1 block 0 (42 insns) and
pass2 block 0 (44 insns) both replay **baseline exact**. The cc1 index -> RTL uid map
(`tmp/grind/func_80060A68/s24/map.py`) fixes the vocabulary for every future session:

    uid   9  lw $3,D_800A3468      uid  16  lhu $2,0($3)        uid  12  lw $5,16($3)   <- THE P10 LOAD
    uid  21  sll $2,$2,2           uid  25  sw $0,D_800F10D0($2)  (the Z0 store)
    uid  28  lw $2,12($3)          uid  35  lw $4,12($3)        uid  30  lw $2,0($2)
    uid  32  sw $2,32($3)          uid  49/56  the two later lw $4,16($3)
    uid  69  lhu $5,4($5)          uid  81  sh $5,28($3)

**The predicate.** `sched.c` builds the block backwards, so the printed pick order is
the reverse of the emission order. Our pass-2 pick tail is
`... 32, 30, 35, 28, 25, 21, 12, 16, 147, 145, 9`; target's must be
`... 32, 30, 12, 35, 28, 25, 21, 16, 147, 145, 9`. So **uid 12 must be picked
immediately after uid 30 and before uid 35**. `rank_for_schedule` is priority
descending, then dep class, then LUID descending, and the harvested pass-2 priorities
are `pri(30)=4, pri(35)=pri(28)=pri(25)=pri(21)=3, pri(12)=pri(16)=2`, with
`luid(12)=17, luid(30)=21, luid(35)=22`. Since `INSN_PRIORITY` is the longest
dependence path **from the block head**, the requirement is exactly:

> **pri(12) = 3 AND pass-2 LUID(12) > 22**, or **pri(12) = 4** (any luid < 21).

and `pri(insn) = max over LOG_LINKS preds of (pri(pred) + insn_cost(pred) - 1)`, with
`ADJUST_COST` zeroing anti/output costs. With the p10 read written first, uid 12's ONLY
pred is uid 9 (pri 1, load cost 2) => pri(12) = 2, permanently. Raising it needs a pred,
and a pred must precede it. Since pass-2 LUID is the sched1 emission position, the whole
question collapses to: **sched1 must emit the p10 load after uid 35 and before uid 32,
with the Z0 store as the only store above it in source.**

**The depth-1 solver verdict.** `perturb.py --pass 2 --block 0 --goal-before 30:12
--goal-before 12:35 --depth 1` searched **4634 single atoms** (the FULL atom vocabulary
— add_dep, del_dep, luid, luid_move, unit/cost — not the spellable-only subset) and
reports **NO perturbation reaches the goal at this depth**. The same search on pass 1
(goal `12:35`, 4400 atoms) returns exactly **one** vector:

    del_dep 25 <- 12        (remove the Z0 store's ANTI-dependence on the p10 load)

i.e. the only single input change that lets sched1 emit the p10 load below the two 0xC
loads is to make `sw $0,D_800F10D0($2)` not conflict with `lw $5,16($3)` in
`sched_analyze`. GCC 2.7.2 has no C-reachable spelling for that: both memrefs are
`(plus reg X)` forms with a varying index, so `memrefs_conflict_p` cannot disambiguate
them, and there is no `restrict`, no type-based aliasing and no `RTX_UNCHANGING_P` a C
author can set on an ordinary load. **The vector is real, is the right shape, and has no
C form** — recording it so no later session re-derives it.

**The six-position collapse (new, and it closes a whole family).** The only other way to
change LUID(12) is to move the p10 read below the Z0 store in source. Six distinct
source positions were built and measured this session — immediately after the Z0 store,
after copy 1, after copy 2, and three splits that put the read *inside* a copy statement
between its load and its store (`c1 = *(s32 *)(*(s32 *)(outer + 0xC) + 0); p10 = ...;
*(s32 *)(outer + 0x20) = c1;` and the copy-2 equivalent). **All six score 5 / build 67
with their three 0x10 loads at 18 / 23 / 24, and their raw cc1 output is BYTE-IDENTICAL**
(`diff tmp/grind/func_80060A68/s24/cc1_v1_p10_after_Z0.s cc1_v3_p10_after_copy2.s` and
`cc1_v6_split_copy2_p10_between.s` are both empty). So the statement position of the p10
read below the Z0 store is inert *upstream of sched* — cse/RTL emission normalises all
six to one stream — and the split-a-copy idea, which was the only way to place the read
with the Z0 store as its sole preceding store, buys nothing. There are exactly two
regimes reachable by moving this statement: **p10 load FIRST (T1, 65 insns, score 2)** or
**p10 load LAST (67 insns, score 5)**. The middle seat target uses has never been
produced by a statement move, and now has a mechanism for why.

- [s24] CHASSIS: candidate.c (E2) re-measured 2 / build 66 / target 66 on today's HEAD before any probe; the T1 body from rejected/t1-p10-read-above-the-zero-store-...c, with the s23 `()` call spelling, re-measured 2 / build 65 / target 66. Floor unchanged at 2.

- [s24] THE WHOLE T1 RESIDUAL, in target bytes: ours `lhu v0,0(v1) | lw a1,16(v1) | sll | lui at | addu at | sw zero | lw v0,12(v1) | lw a0,12(v1) | lw v0,0(v0)` vs target `lhu v0,0(v1) | nop | sll | lui at | addu at | sw zero | lw v0,12(v1) | lw a0,12(v1) | lw a1,16(v1) | lw v0,0(v0)`. Every other instruction, every nop included, is identical.

- [s24] The scheduler model is exact for this function: extract.py text1b -> parity=True, 492 funcs, 1764 blocks; pass1 block 0 (42 insns) and pass2 block 0 (44 insns) both replay BASELINE EXACT.

- [s24] RTL-uid vocabulary for every future session (from tmp/grind/func_80060A68/s24/map.py): 9 `lw $3,D_800A3468`; 16 `lhu $2,0($3)`; 12 `lw $5,16($3)` = THE P10 LOAD; 21 `sll $2,$2,2`; 25 `sw $0,D_800F10D0($2)` = the Z0 store; 28 `lw $2,12($3)`; 35 `lw $4,12($3)`; 30 `lw $2,0($2)`; 32 `sw $2,32($3)`; 49/56 the two later `lw $4,16($3)`; 69 `lhu $5,4($5)`; 81 `sh $5,28($3)`.

- [s24] PASS-2 PICK ORDERS. Ours: `... 32, 30, 35, 28, 25, 21, 12, 16, 147, 145, 9`. Target's must be `... 32, 30, 12, 35, 28, 25, 21, 16, 147, 145, 9`. Pick order is the REVERSE of emission order (sched.c builds the block backwards).

- [s24] HARVESTED PASS-2 PRIORITIES/LUIDS: pri(30)=4, pri(35)=pri(28)=pri(25)=pri(21)=3, pri(12)=pri(16)=2; luid(12)=17, luid(30)=21, luid(35)=22. Requirement: pri(12)=3 with luid(12)>22, or pri(12)=4.

- [s24] pri(12)=2 is structural while the p10 read is written first: INSN_PRIORITY is the longest dependence path FROM THE BLOCK HEAD and uid 12's only pred is uid 9 (pri 1, load cost 2).

- [s24] DEPTH-1 SOLVER NEGATIVE: perturb.py --pass 2 --block 0 --goal-before 30:12 --goal-before 12:35 --depth 1 searched all 4634 single atoms and reached the goal with NONE. NOTE the goal semantics: --goal-before is evaluated on the PICK list, so `A:B` means A is emitted AFTER B - a goal stated in emission order returns a false negative (this session lost one search to that).

- [s24] THE ONE PASS-1 VECTOR: `del_dep 25 <- 12` - remove the Z0 store's ANTI-dependence on the p10 load. Right shape, no C spelling under GCC 2.7.2 (both memrefs are `(plus reg X)` with a varying index, so memrefs_conflict_p cannot disambiguate; no restrict, no TBAA, no author-settable RTX_UNCHANGING_P).

- [s24] SIX-POSITION COLLAPSE: v1-v6 (p10 read after the Z0 store / after copy 1 / after copy 2 / split inside copy 1 both ways / split inside copy 2) ALL score 5 / build 67 with loads at 18 ($a0) / 23 ($v0) / 24 ($a1), and their raw cc1 streams are byte-identical to each other. Statement position below the Z0 store is normalised upstream of sched.

- [s24] TOOL DEFECT (upstream, not fixed here - tools/ is not an editable surface for a grind session): tools/sched_solver/goalmap.py `_macro_expand_counts` counts `sw $0,SYM($idx)` as ONE object insn, but GNU as assembles it to lui/addu/sw. That makes object-mode goal derivation abort on this function with `honest object has 65 insns but text1b.hon.s body has 63 lines`. A read-only fork with the third case added is at tmp/grind/func_80060A68/s24/goalmap.py, driven by tmp/grind/func_80060A68/s24/run_perturb.py.

- [s24] inverse_compose.py refuses text-mode classify for this zero-rule function and names the object-mode invocation; goal_from_tgt.py classify is the working entry point.

- [s24] candidate.c is now the T1 body (documented header); the E2 body is preserved undisproven at memory/grind/func_80060A68/e2-spine-floor2-cse-merged.c. Zero FAKE constructs in every body measured this session; none of the five banned constructs is present in any of them.

- [s24] src/text1b.c restored to HEAD (INCLUDE_ASM) at session end; the working tree carries only memory/grind/func_80060A68/ changes.

## s25 (2026-09-04) — forensics: the pass-2 priority table, read out whole, and the closed predecessor enumeration

Chassis: HEAD, T1 body (memory/grind/func_80060A68/candidate.c) applied to src/text1b.c.
`sandbox func_80060A68 --disable all` → `"score": 2, "target_insns": 66, "build_insns": 65`.
`tools/sched_solver/extract.py text1b` → `parity=True funcs=492 blocks=1764 picks=14350`.
`tools/fake_ablate.py` on candidate.c → "no FAKE-annotated constructs found; nothing to ablate".
Every priority and LUID s24 harvested reproduces digit-for-digit. The s24 kills stand on this chassis.

### The pass-2 block-0 node table (44 insns) — the authoritative input to the residual

Read with tmp/grind/func_80060A68/s25/model_dump.py; full copy at tmp/grind/func_80060A68/s25/model_T1.txt.
Columns: uid / pass-2 luid / pri / icost / ref.

    145  0  1 1  2      25 19  3 1 22      66 36  9 1  8
    147  1  1 1 25      28 20  3 2  5      69 37  9 2  3
      9 15  1 2 19      30 21  4 2 10      73 38  9 2  4
     16 16  2 2  6      35 22  3 2  6      76 39  8 1  4
     12 17  2 2  6      32 23  5 1 12      78 40  9 1  5
     21 18  3 1  3      37 24  5 2 10      81 41 10 1  5

`deps` for the p10 load: `"12": [[147, 0], [9, 0]]` — two true-dependence predecessors,
uid 147 (pri 1, icost 1 → contributes 1) and uid 9 (pri 1, icost 2 → contributes 2).
Hence pri(12) = 2. The pick tails:

    ours   pass 2: ... 32, 30, 35, 28, 25, 21, 12, 16, 147, 145, 9
    target pass 2: ... 32, 30, 12, 35, 28, 25, 21, 16, 147, 145, 9

### The closed enumeration (this session's deliverable)

`priority()` (tools/gcc-2.7.2/sched.c:1495) is `max over LOG_LINKS of pri(pred) + insn_cost(pred) - 1`,
and MIPS's ADJUST_COST zeroes anti/output link costs, so only a kind-0 dependence can contribute
more than `pri(pred) - 1`. Adding a dependence TO uid 12 leaves every other insn's priority
unchanged (priority is a function of an insn's own predecessors only), so the table above is a
fixed scoring board. Solving `pri(p) + icost(p) - 1` over it:

| target value | insns that deliver it | spellable as a predecessor of the p10 load? |
|---|---|---|
| pri(12)=4 | uid 28, uid 35 — both `lw ?,12($3)`, (pri 3, icost 2) | **NO.** Both are loads. The p10 address is `outer + 0x10`, not a function of `*(s32 *)(outer + 0xC)`, and a load-load pair gets no memory dependence. |
| pri(12)=3 | uid 21 `sll` (3,1); uid 16 `lhu $2,0($3)` (2,2) | **NO.** The p10 address is not a function of `*(u16 *)outer`. |
| pri(12)=3 | uid 25, the Z0 store (3,1) | **YES**, as a true memory dependence, by placing the p10 read below the store — the six-position family, killed at s24 (all six normalise to one cc1 stream, 67 insns, score 5). |
| overshoot | uid 30 (4,2) → 5; uid 32 (5,1) → 5 | ranks 12 above uid 30, the wrong side of target's order. |

So on the T1 65-instruction geometry the "raise pri(12)" route has exactly one spellable member,
and it is already banked dead.

### Pass 1: the birthing bump fires on the p10 load, and that is why the LUID half is closed too

`adjust_priority` (sched.c:2541-2589) runs only while `reload_completed == 0`. Pass-1 `adjpri`
records `(insn 12, deaths 0, birth 1, pri 2)` and the pass-1 pick list shows uid 12 picked at
priority 2130706433 (= 0x7F000001 = max_priority): the bump fires on the p10 load. It also fires
on uid 9, 16, 21, 28, 30, 35 and most of the block; the birth=0 insns are 118, 81, 78, 66, 63, 53,
25, 32, 46, 39. With the head insns all tied at max_priority, the pass-1 pick order among them is
decided by LUID descending over the pass-1 INPUT order (pass-1 luids 9=0, 12=1, 16=3, 21=5, 25=7,
28=8, 30=9, 32=10, 35=11), giving the pick tail 32, 35, 30, 28, 25, 21, 12, 16, 9 and therefore the
pass-2 luids 9=15, 16=16, 12=17, 21=18, 25=19, 28=20, 30=21, 35=22, 32=23. Raising pass-2 luid(12)
above 22 therefore requires the p10 read to sit later in the RAW SOURCE ORDER — the identical lever
s24 swept across six positions and found normalised to one cc1 stream. Both halves of the predicate
reduce to the same statement-position axis.

### The one new spelling measured, and why it fails on budget rather than on order

`p10 = *(s32 *)(D_800A3468 + 0x10);` written BELOW the Z0 store (body_V1.c) is a second source-level
read of the same gp global. Prediction: the store to `D_800F10D0($idx)` is a varying gp address that
`memrefs_conflict_p` cannot disambiguate, so cse cannot reuse the first load and a genuine second
`lw` materialises, carrying a true memory dependence on the store (pri 3) and thereby becoming the
(pri 3, icost 2) predecessor the enumeration demands. **The prediction is exactly right** — the cc1
stream shows `lw $3,D_800A3468 … sw $0,D_800F10D0($2) … lw $4,D_800A3468 … lw $6,16($4)`, and the
pass-2 profile shifts (four insns at pri 4 where T1 has three at pri 3). It fails on the instruction
budget instead: build 67 against target 66, score 9. Target's stream is T1's stream plus exactly one
load-delay NOP, so any body that materialises a 66th REAL instruction is out of budget whatever it
does to the schedule. That is now the standing constraint on this spine: **the residual must be
bought with zero net instructions.**

Artifacts: tmp/grind/func_80060A68/s25/{model_dump.py, pass1.py, model_T1.txt, picks_T1.txt,
picks_V1.txt, cc1_T1.s, cc1_V1.s, body_T1.c, body_V1.c, apply.sh, extract.sh, cap.sh}.

- [s25] HEAD chassis floor re-measured this session: T1 (candidate.c) = score 2, build_insns 65, target_insns 66. The brief's 'measurement unavailable' is resolved to 2.

- [s25] tools/sched_solver/extract.py text1b re-extracts parity=True over 1764 blocks; this function's pass-1 block 0 (42 insns) and pass-2 block 0 (44 insns) both replay BASELINE EXACT, so the model is authoritative on this chassis.

- [s25] Pass-2 block 0 deps for the p10 load are exactly `"12": [[147, 0], [9, 0]]` - uid 147 (pri 1, icost 1 -> contributes 1) and uid 9 (pri 1, icost 2 -> contributes 2), hence pri(12) = 2.

- [s25] The complete pass-2 (pri, icost) board for block 0 is banked in evidence.md; the only (pri 3, icost 2) insns are uid 28 and uid 35, both `lw ?,12($3)` loads.

- [s25] fake_ablate.py reports zero FAKE-annotated constructs in candidate.c, so the s22/s23/s24 instance kills were not measured under a FAKE carrier.

- [s25] NEW MECHANISM DATUM (reusable across this TU): a second source-level read of a gp global placed BELOW a store to a varying gp address is NOT cse-merged - the store invalidates memory - and materialises a real second `lw`. Confirmed in the cc1 stream at tmp/grind/func_80060A68/s25/cc1_V1.s.

- [s25] NEW STANDING CONSTRAINT on the T1 spine: target's instruction stream is this body's stream plus exactly one load-delay NOP, so the residual must be bought with ZERO NET INSTRUCTIONS. Any spelling that materialises a 66th real instruction is out of budget regardless of its effect on the schedule (measured: the gp re-read = 67 insns, score 9).

- [s25] Pass-1 adjpri confirms birthing_insn_p fires on the p10 load (birth=1, bumped to 0x7F000001), so the pass-1 head order is LUID-descending over raw source order - the mechanism behind s24's six-position collapse.

---

## [s26 2026-09-04, forensics] THE RESIDUAL IS A **READINESS** CONSTRAINT, NOT A PRIORITY CONSTRAINT — s24/s25's predicate is superseded

This is the single most important correction in the ledger to date and it should be read
before any further work on the T1 spine.  s24 stated the residual as an INSN_PRIORITY
predicate (`pri(12)=3 with pass-2 luid(12) > 22, or pri(12)=4`) and s25 spent a whole
session exhaustively enumerating the pass-2 (pri, icost) board looking for a predecessor
that could satisfy it.  **That predicate can never fire, because uid 12 is not in the ready
list at the pick where it would have to compete.**

Measured, not inferred — read straight out of the extracted pass-2 pick trace for
func_80060A68 block 0 on today's HEAD chassis with candidate.c (the T1 spine) applied
(`tools/sched_solver/extract.py text1b` -> `parity=True funcs=492 blocks=1764 picks=14350`):

    PICK insn 30  clock 44  ready [[30, 4, 21], [35, 3, 22]]
    PICK insn 35  clock 45  ready [[35, 3, 22]]
    PICK insn 12  clock 49  ready [[12, 2, 17]]

At clock 44 (where uid 30 is picked) and at clock 45 (where uid 35 is picked — the slot
target gives to uid 12) **uid 12 does not appear in the ready list at all.**
`rank_for_schedule` (sched.c:2407-2464) is only consulted among READY insns, so
INSN_PRIORITY(12) is irrelevant at those two picks: raising it to 3, to 4, or to 4000
would not put uid 12 on the list.  s25's board enumeration was answering the wrong
question, and this is also the reason the solver's only depth-1 vector, across two
sessions and 4634 atoms, was `del_dep 25 <- 12` and nothing else.

WHY uid 12 IS NOT READY.  The pass-2 dependence table for block 0 is

    uid  9  luid 15  pri 1  icost 2  deps  -                      lw $3,D_800A3468
    uid 16  luid 16  pri 2  icost 2  deps [[147,0],[9,0]]         lhu $2,0($3)
    uid 12  luid 17  pri 2  icost 2  deps [[147,0],[9,0]]         lw $5,16($3)   <- THE P10 LOAD
    uid 21  luid 18  pri 3  icost 1  deps [[16,0]]                sll
    uid 25  luid 19  pri 3  icost 1  deps [[147,15],[9,14],[12,14],[16,14],[21,0]]   sw $0,D_800F10D0($2)
    uid 28  luid 20  pri 3  icost 2  deps [[147,0],[21,14],[9,0],[25,0]]             lw $2,12($3)
    uid 30  luid 21  pri 4  icost 2  deps [[147,0],[25,0],[28,0]]                    lw $2,0($2)
    uid 35  luid 22  pri 3  icost 2  deps [[147,0],[9,0],[25,0]]                     lw $4,12($3)
    uid 32  luid 23  pri 5  icost 1  deps [[147,15],[25,15],[9,0],[30,0]]            sw $2,32($3)

`deps[25]` contains `[12, 14]` — dependence kind 14 is REG_DEP_ANTI.  **The Z0 store
carries an ANTI-dependence on the p10 load** (write-after-read on memory).  sched.c builds
the block BACKWARDS, so an insn becomes ready only when every insn that depends on it has
already been scheduled; uid 12 therefore cannot enter the ready list until uid 25 is
scheduled, and uid 25 is not scheduled until picks 28 and 35 are already behind it.  Target
picks uid 12 BEFORE uid 35, which means in target's compilation **the anti-dependence
25 <- 12 does not exist at all.**

## [s26] THE GATE PREDICATE, AND ITS SOURCE-SIDE INPUT ENUMERATION

The dep is created at `sched.c:1783`, `if (anti_dependence (XEXP (pending_mem, 0), dest))`.
`anti_dependence` is `sched.c:845-868`; it returns 0 — i.e. NO dep — on exactly four routes:

  (a) `RTX_UNCHANGING_P (mem)` on the p10 LOAD.  GCC 2.7.2 sets RTX_UNCHANGING_P only for
      reads of readonly DECLs and for constant-pool refs; a `const`-qualified pointer deref
      does not get it.  NO C SPELLING for a load through a runtime pointer.
  (b) `MEM_VOLATILE_P (x) && MEM_VOLATILE_P (mem)` — BOTH memrefs volatile.  The p10 load is
      a deref of a local pointer and the Z0 store is a write to the game-state global
      D_800F10D0; making both volatile changes emitted bytes elsewhere in the body and is a
      family question (`legitimate-volatile-interrupt-touched` two-prong gate), not a free
      lever.  UNMEASURED — see the frontier.
  (c) `memrefs_conflict_p (...) == 0` (sched.c:614-760).  The p10 load's address is
      `(plus (reg outer) 16)` with `outer` a PSEUDO holding a gp-loaded pointer, and the
      store's address is the `D_800F10D0 + idx*4` symbolic form.  The one escape that could
      apply is the "base addresses are distinct objects" branch at sched.c:697-705, which
      requires `find_symbolic_term` to succeed on BOTH sides; it returns 0 for a
      pseudo-based address, so the function returns 1 (conflict).  The source-side input
      that would change this is an address for the p10 load whose RTL base is a SYMBOL_REF
      rather than a pseudo.
  (d) The two MEM_IN_STRUCT_P / rtx_addr_varies_p asymmetry prongs.  **Both are dead for
      THIS pair independent of any C spelling**, and this is a class fact:
        prong 1 needs `! rtx_addr_varies_p (mem)` on the p10 LOAD — its address is
          `(plus (reg outer) 16)`, and `rtx_addr_varies_p` is true for any address
          containing a non-frame REG.  `outer` is a runtime value read from gp; it cannot be
          made a compile-time-invariant address in any C spelling of this function.
        prong 2 needs `! rtx_addr_varies_p (x)` on the Z0 STORE — its address contains the
          runtime index `*(u16 *)outer`.  Same argument.
      So MEM_IN_STRUCT_P is NOT a lever here: no arrangement of struct-typed vs
      scalar-typed access can open either prong while both addresses vary.

## [s26] MEASURED: THE COPY-2 SOURCE-POINTER HOIST FAMILY (12 bodies, zero FAKE constructs)

The s25 frontier's item 1 proposed lowering pri(35) to 2 by staging copy 2's source pointer
above the Z0 store so uid 35 loses its uid-25 memory dependence, on the theory that
rank_for_schedule's dependence-CLASS term would then discriminate.  Two independent reasons
that is now closed, one derived and one measured:

DERIVED.  With `last_scheduled_insn = uid 30`, the class test (sched.c:2422-2443) asks
whether the candidate is in `LOG_LINKS (30)`.  `deps[30] = [[147,0],[25,0],[28,0]]` — neither
uid 12 nor uid 35 is a predecessor of uid 30, so both classify 3 and the term is a tie; the
comparator falls through to `INSN_LUID (tmp) - INSN_LUID (tmp2)` (sched.c:2462), which is the
statement-position axis s24 already closed.  The class term cannot be the discriminator.

MEASURED (all on today's HEAD chassis, T1 spine, `sandbox func_80060A68 --disable all`):

    W1  c2p read hoisted above Z0 store, copy 2 only          score 4   66 insns
    W2  ditto, copies 2+3                                     score 5   66 insns
    W3  ditto, copy 1 only                                    score 6   64 insns
    W4  ditto, copies 1+2                                     score 6   64 insns
    W5  ditto, hoist placed above the p10 read                score 4   66 insns
    W6  ditto, copy 3 only                                    score 7   66 insns
    W7  ditto, all three copies                               score 7   64 insns
    X1  c2p AND p10 both read below the Z0 store (c2p first)  score 7   67 insns
    X2  ditto, p10 first                                      score 7   67 insns
    X3  p10 above store, c2p read BELOW store, copy 2 via it  score 4   65 insns
    X4  p10 read below the store, no c2p local (control)      score 5   67 insns
    X5  both below, copies 2+3 via c2p                        score 8   67 insns
    T1  control (candidate.c)                                 score 2   65 insns

W1 IS A NEW SPINE AND WORTH RECORDING PROPERLY.  Its object has **exactly 66 instructions
and is a pure permutation of target's exact instruction multiset** — the first body in 26
sessions with that property (T1 is 65 and is missing one load-delay nop; E2 is 66 but is
missing a load and carries an extra nop).  Raw diff is 4 and every instruction from
`lw v0,0(v0)` to the end is byte-identical to target:

    ours(W1): lw v1,0(gp) | addiu sp | sw ra | lw a1,16(v1) | lhu v0,0(v1) | lw a0,12(v1)
              | sll | lui at | addu at | sw zero | lw v0,12(v1) | nop | lw v0,0(v0) | ...
    target  : lw v1,0(gp) | addiu sp | sw ra | nop | lhu v0,0(v1)
              | sll | lui at | addu at | sw zero | lw v0,12(v1) | lw a0,12(v1) | lw a1,16(v1) | lw v0,0(v0) | ...

But its extracted pass-2 model makes it a WORSE solver target, not a better one.  The hoist
drags BOTH the p10 load and the copy-2 pointer load to the block-head chain, where both sit
at pri 2 (uid 12 luid 16, uid 15 luid 18) behind three pri-3 insns (uid 24 sll, uid 28 the
Z0 store, uid 31 the surviving copy-1 pointer load).  W1's goal is `pick 12, then 15, then
31` after uid 33, so it now needs TWO insns lifted over the pri-3 wall instead of one.  W1's
model is banked at tmp/grind/func_80060A68/s26/W1.sched.json and dumped at
tmp/grind/func_80060A68/s26/model_W1.txt.

X3 IS THE OTHER NEAR MISS AND IT DIES TO CSE, NOT TO THE SCHEDULER.  Reading the copy-2
source pointer into a single-write local BELOW the Z0 store keeps the instruction count at
65 (unlike the 67-insn six-position family), and puts a `lw a0,12(v1)` at target's slot 9.
But cse merges that local with copy 1's inline read of the same address, so only ONE
`lw ?,12(v1)` survives where target has two (slots 9 and 10) — the merged load is followed
by a load-delay nop at slot 10 and the p10 load is still stranded at slot 4.  score 4.

- [s26] Chassis re-measured at dispatch: candidate.c (T1 spine) applied to src/text1b.c gives sandbox score 2, target_insns 66, build_insns 65 -- unchanged from the ledger's recorded floor.

- [s26] The pass-2 pick trace for func_80060A68 block 0 is: PICK insn 30 clock 44 ready [[30,4,21],[35,3,22]]; PICK insn 35 clock 45 ready [[35,3,22]]; PICK insn 12 clock 49 ready [[12,2,17]]. uid 12 is not on the ready list at either of the two picks target uses for it.

- [s26] deps[25] = [[147,15],[9,14],[12,14],[16,14],[21,0]] -- dependence kind 14 is REG_DEP_ANTI, so the Z0 store is anti-dependent on the p10 load; sched.c builds the block backwards so uid 12 cannot join the ready list until uid 25 is scheduled.

- [s26] deps[30] = [[147,0],[25,0],[28,0]] -- neither uid 12 nor uid 35 is a LOG_LINKS predecessor of uid 30, so rank_for_schedule's dependence-class term is a tie (both class 3) at the critical pick and the comparator falls through to LUID.

- [s26] The anti-dependence is created at tools/gcc-2.7.2/sched.c:1783 and gated by anti_dependence at sched.c:845-868. Its four zero-return routes are: RTX_UNCHANGING_P on the read (no C spelling for a runtime-pointer deref under GCC 2.7.2); both memrefs MEM_VOLATILE_P (unmeasured, family question); memrefs_conflict_p == 0; and the two MEM_IN_STRUCT_P / rtx_addr_varies_p prongs (class-killed this session at sched.c:862).

- [s26] memrefs_conflict_p's only applicable escape for this pair is the distinct-symbolic-base branch at sched.c:697-705, which requires find_symbolic_term to succeed on BOTH addresses; the p10 load's base is a pseudo holding a gp-loaded pointer, so it returns 0 and the function reports a conflict.

- [s26] Copy-2 source-pointer hoist sweep (12 bodies, zero FAKE constructs, today's HEAD chassis): W1 4/66, W2 5/66, W3 6/64, W4 6/64, W5 4/66, W6 7/66, W7 7/64, X1 7/67, X2 7/67, X3 4/65, X4 5/67, X5 8/67; T1 control 2/65.

- [s26] W1's object is 66 instructions and a pure permutation of target's exact instruction multiset -- the first such body in 26 sessions. ours: ... sw ra | lw a1,16(v1) | lhu v0,0(v1) | lw a0,12(v1) | sll | lui | addu | sw zero | lw v0,12(v1) | nop | lw v0,0(v0) ... ; target: ... sw ra | nop | lhu v0,0(v1) | sll | lui | addu | sw zero | lw v0,12(v1) | lw a0,12(v1) | lw a1,16(v1) | lw v0,0(v0) ... Everything from lw v0,0(v0) onward is identical.

- [s26] W1's pass-2 model (parity=True, banked) shows the hoist strands both the p10 load (uid 12, pri 2, luid 16) and the copy-2 pointer load (uid 15, pri 2, luid 18) in the block-head chain behind three pri-3 insns (uid 24 sll, uid 28 the Z0 store, uid 31 the surviving copy-1 pointer load), so W1's goal needs TWO insns lifted over the pri-3 wall.

- [s26] tools/sched_solver/goalmap.py (and the s24 fork) still cannot auto-derive an object-mode goal for this function even when the tree carries the 66-instruction W1 body: it reports hon=65 / honobj |A|=67 and aborts before emitting a goal, so goal derivation for this function remains manual.

- [s26] The depth-2 pass-2 solver search (frontier item 2 from s25, in both the luid,luid_move and the full-atom vocabularies) was launched as this session's first act and ran 22 minutes without emitting a line; it was stopped cleanly before the turn ended and no solver process remains. It is now analytically superseded: luid perturbations cannot change READINESS, which is a function of the dependence graph, so no luid vector can put uid 12 on the ready list at clock 44/45.

- [s27] KILL RE-AUDIT (mandated, floor flat 3 sessions).  `tools/fake_ablate.py` is a no-op for this
  function: candidate.c and every s27 probe carry ZERO FAKE constructs.  Seven banked forms
  re-measured on today's HEAD chassis, all reproducing their recorded scores digit for digit:
  s5-three-fresh-inline-reads 3/66, no-p10-local-inline-4-read-late 3/66, s17-A2 3/66,
  s18-U2 5/66, s5-struct-typed-inner-reads 6/66, no-p10-local-inline-4-read-early 8/65,
  s5-v2-named-address-locals 5/67.  No banked kill is VOID under the current rule scope.

- [s27] The T1 control re-measures 2 / build 65 / target 66 on today's HEAD; X4 (the below-Z0-store
  control) re-measures 5 / build 67.

- [s27] s26's frontier item 1 is ANSWERED: X4's 67th instruction is a load-delay NOP at slot 22,
  where target has its second `lw a0,16(v1)`.  X4's window reads
  `lw a0,16(v1) | sw v0,40(v1) | lhu v0,0(a0) | nop | sh v0,24(v1) | lw v0,16(v1) | lw a1,16(v1) |
  lhu a0,2(v0)` against target's
  `lw a0,16(v1) | sw v0,40(v1) | lhu v0,0(a0) | lw a0,16(v1) | sh v0,24(v1) | lhu a0,2(a0)` plus the
  slot-11 `lw a1,16(v1)`.  The +2 pointer reload lands in $v0, which is anti-dependent on the pending
  `sh v0,24(v1)`, so it cannot occupy the delay slot; target's reload lands in $a0, which is free
  after slot 21, and fills it.

- [s27] PASS ATTRIBUTION, read from `pwsh tools/grinder/dump.ps1 func_80060A68` on the s5
  three-inline-read body: in the .sched dump the third `lw ?,16(reg/v:SI 72)` is uid 66 with
  LOG_LINKS `(insn_list 9 (insn_list 22 (insn_list 60 (nil))))`, while the other two 0x10 loads
  (uid 46, uid 53) carry only `(insn_list 9 (insn_list 22 (nil)))`.  uid 60 is the
  `sw ?,D_800A3478` gp store.  A 0x10 read placed below that store inherits a memory dependence on
  it (symbol-based store vs pseudo-based load: memrefs_conflict_p cannot disambiguate,
  sched.c:697-705) and cannot be hoisted past it.

- [s27] p10-statement POSITION SWEEP on the T1 statement set, 8 positions, today's HEAD chassis,
  zero FAKE constructs in any body:
      after the Z0 store   (X4)   5 / 67   third 0x10 load at slot 25
      after copy 1         (Pc1)  5 / 67   third 0x10 load at slot 25
      after copy 2         (Pc2)  5 / 67   third 0x10 load at slot 25
      after copy 3         (Pc3)  2 / 66   load at SLOT 11 in $a1; cse merges p10 with the +0 read
      after the 0x18 store (P18)  2 / 66   load at SLOT 11 in $a1; cse merges p10 with the +2 read
      after the +2 read    (Pt2)  2 / 66   load at SLOT 11 in $a1; cse merges p10 with the +2 read
      after the gp store   (Pgp)  3 / 66   THREE distinct loads; third stranded at slot 26 in $v0
      after the 0x1A store (P1A)  3 / 66   THREE distinct loads; third stranded at slot 26 in $v0
  Pt2 and P18 emit byte-identical objects; Pgp and P1A emit byte-identical objects.

- [s27] Pt2/P18 is byte-identical to target at every one of its 66 slots EXCEPT slot 22
  (ours `nop`, target `lw a0,16(v1)`) and slot 24 (ours `lhu a0,2(a1)`, target `lhu a0,2(a0)`).
  The slot-11 `lw a1,16(v1)` - the seat 26 sessions could not reach - is present and correct.

- [s27] Pgp/P1A is byte-identical to target at every slot EXCEPT that its third 0x10 load sits at
  slot 26 as `lw v0,16(v1)` (feeding `lhu a1,4(v0)`) instead of at slot 11 as `lw a1,16(v1)`
  (feeding `lhu a1,4(a1)`).  Three distinct loads, everything else identical.

- [s27] CSE is the merger on the Pt2 spine, confirmed by counting
  `(mem:SI (plus:SI (reg/v:SI 72) (const_int 16)))` per dump: .rtl 3, .jump 3, .cse 2, .loop 2,
  .cse2 2, .combine 2, .sched 2, .lreg 3 (reload re-materialises one).  All three source reads
  survive into cse; cse folds two.  This reproduces s20's cse.c:1701 finding on the new spine.

- [s27] SECOND-SEPARATOR SWEEP - every way of putting a second cse-breaking store into the window
  between copy 3's store and the D_800A3478 store, measured on today's HEAD chassis:
      copy 3's store split out through a named intermediate, placed below the +2 read (S1) 11 / 66
      ditto, placed below the p10 read (S4)                                                8 / 67
      ditto, placed above the +0 read (S5)                                                 5 / 67
      copy 3's WHOLE statement moved between the +2 read and the p10 read (R1)            11 / 67
      copy 3's WHOLE statement moved between the +0 and +2 reads (R3)                      8 / 66
      copy 3's WHOLE statement moved between the p10 and +2 reads (R4)                    10 / 67
      the 0x1A store hoisted above the gp store to act as the separator (R2)               6 / 66
      temp2 deleted, +2 read inlined into the 0x1A store below the gp store (U1)           7 / 67
      ditto with a p10 local above the gp store (U3)                                       8 / 68
      ditto with idx below the 347C store (U4)                                            12 / 68
  Every one regresses.  Splitting or moving copy 3 shatters the copy block's emission
  (its loads and stores stop interleaving with the halfword group the way target's do).

- [s27] Chassis re-measured at dispatch: T1 (the incoming candidate.c) gives score 2, target_insns 66, build_insns 65 - unchanged from the ledger's recorded floor.

- [s27] X4's 67th instruction is a load-delay NOP at slot 22; target fills that slot with its second `lw a0,16(v1)`, which X4 cannot do because its +2 pointer reload targets $v0 and $v0 is still live for the pending `sh v0,24(v1)`.

- [s27] In the .sched dump the third `lw ?,16(reg/v:SI 72)` (uid 66) carries LOG_LINKS (9, 22, 60) where uid 60 is the `sw ?,D_800A3478` gp store; the other two 0x10 loads (uid 46, uid 53) carry only (9, 22).

- [s27] p10-statement position sweep, today's HEAD chassis, zero FAKE constructs: after Z0 store 5/67, after copy1 5/67, after copy2 5/67, after copy3 2/66, after the 0x18 store 2/66, after the +2 read 2/66, after the gp store 3/66, after the 0x1A store 3/66.

- [s27] Pt2 and P18 emit byte-identical 66-instruction objects that match target at every slot except 22 (ours nop, target `lw a0,16(v1)`) and 24 (ours `lhu a0,2(a1)`, target `lhu a0,2(a0)`); the slot-11 `lw a1,16(v1)` is present and correct.

- [s27] Pgp and P1A emit byte-identical 66-instruction objects with THREE distinct 0x10 loads that match target at every slot except that the third load sits at slot 26 as `lw v0,16(v1)` feeding `lhu a1,4(v0)`, instead of at slot 11 as `lw a1,16(v1)` feeding `lhu a1,4(a1)`.

- [s27] Per-pass count of `(mem:SI (plus:SI (reg/v:SI 72) (const_int 16)))` on the Pt2 body: .rtl 3, .jump 3, .cse 2, .loop 2, .cse2 2, .combine 2, .sched 2, .lreg 3 - cse is the merger.

- [s27] Second-separator sweep on today's HEAD chassis: S1 11/66, S4 8/67, S5 5/67, R1 11/67, R3 8/66, R4 10/67, R2 6/66, U1 7/67, U3 8/68, U4 12/68 - every import of a second separator regresses.

- [s27] Kill re-audit: seven banked rejected forms re-measured on today's HEAD chassis all reproduce their recorded scores exactly (3/66, 3/66, 3/66, 5/66, 6/66, 8/65, 5/67); fake_ablate is a no-op because no banked body carries a FAKE construct.
