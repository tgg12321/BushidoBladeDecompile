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
