# Evidence bank — func_8003B10C

- WIP rejected_form: {'form': 'per-branch ed6 variable (separate read each arm)', 'score': 32, 'reason': 'not a cheat; GCC promotes ed6 to callee-saved, frame grows -32->-40'}

- WIP rejected_form: {'form': 'per-branch full pointer p = &TBL + idx*N + ed6read', 'score': 26, 'reason': 'not a cheat; worse'}

- WIP rejected_form: {'form': 'per-branch p = &TBL + idx*N, ed6 read in shared tail', 'score': 19, 'reason': 'not a cheat; still hoists + spill'}

- WIP rejected_form: {'form': 'fold ed6 into v0 per branch (v0 = idx*N + ed6read)', 'score': 26, 'reason': 'not a cheat; worse'}

- WIP rejected_form: {'form': 'v6 fn-scope s32 v1 declared FIRST (declaration-order LUID)', 'score': 33, 'reason': 'no improvement'}

- WIP rejected_form: {'form': 'v7 register s32 v1 storage-class hint', 'score': 33, 'reason': 'no improvement'}

- WIP rejected_form: {'form': 'v8 s8 *tbl with explicit u8 cast at lbu', 'score': 33, 'reason': 'no improvement'}

- WIP rejected_form: {'form': 'v10 fn-scope s32 arg carrying lbu result', 'score': 20, 'reason': 'partial cross-jump but loses elsewhere'}

- WIP rejected_form: {'form': 'v11 arm-local u8 *tbl AND s32 v1', 'score': 25, 'reason': 'no improvement'}

- WIP rejected_form: {'form': 'v12 v0 += ED6 inline per arm + per-arm call', 'score': 37, 'reason': 'no improvement'}

- == imported from memory/wip notes.md ==
# func_8003B10C — WIP (CSE-defeat plateau; identical-symbol read can't duplicate in pure C)

## TL;DR (2026-06-14)
HEAD carries 12 regfix rules (4 deletes + 6 inserts + 1 subst) whose net effect
is to INJECT a duplicated `lh %lo(D_80101ED6)($1)` (with `lui %hi`/`addu $1,$16`)
into BOTH arms of the `if (D_800A38DC == 5)` branch. Unpinned/un-ruled masked
floor = **10** and NO pure-C form found this session beats it. This is a
[[split-read-defeats-hoist]] case where the dup-read lever does NOT apply
cleanly: the shared read is the SAME symbol (`D_80101ED6`) in both arms, so
there is no "symbol-known arm" to index directly — GCC CSE-merges the identical
memory read no matter where it is written, and the only thing that would force
the duplication is the FORBIDDEN volatile coercion. Per the rule, this makes
the function not pure-C-matchable; block for canonical-asm authorization or a
confirmed plateau decision.

## The gap (the 10)
Target duplicates `*(s16*)((u8*)&D_80101ED6 + s0)` inside each branch (leaves it
in v1, caller-saved) and the shared tail is just `addu v0,v0,a0; addu v0,v0,v1;
lbu`. Pure-C GCC HOISTS that identical read to ONE shared load after the merge
(CSE), changing the branch shapes + the tail. Also a 1-insn operand-order diff
(`addu v0,a0,v0` vs target `addu v0,v0,a0`).

## Ruled out this session (all >= floor 10, none a cheat)
- per-branch `ed6` variable (separate read in each arm) — floor 32; GCC
  promotes ed6 to a callee-saved reg (sp grows -32 -> -40, extra s-reg save).
- per-branch full pointer `p = &TBL + idx*N + ed6read` — floor 26.
- per-branch pointer `p = &TBL + idx*N`, ed6 read in shared tail — floor 19
  (still hoists; basically baseline + a spill).
- fold ed6 into v0 per branch (`v0 = idx*N + ed6read`) — floor 26.

## Why this needs volatile (forbidden) or canonical
The dup-read structural lever defeats hoisting only when the duplicated reads
target DIFFERENT bases (direct symbol vs pointer) per arm. Here both arms read
the SAME `D_80101ED6` global, so CSE re-merges; the regfix inserts exist
precisely because pure C cannot keep the read duplicated. Marking D_80101ED6
volatile would force re-reads but is a forbidden codegen-coercion cheat
(volatile_cheats detector + split-read-defeats-hoist #3 FORBIDDEN 2026-05-31).

## Next hypotheses
- Canonical-asm authorization (the 6 inserts reconstruct duplicated absolute
  loads GCC's CSE removes — genuine no-pure-C-form construct for this shape).
- Permuter from baseline (unlikely: CSE on identical reads is a fixed-shape
  artifact the permuter can't restructure away).


- [s1] [fable-blitz 2026-07-07] Queue entry: func_8003B10C, file code6cac_c_ab, distance 10, verdict C, 12 rules, status active. Body at src/code6cac_c_ab.c:386-408.

- [s1] [fable-blitz 2026-07-07] The 12 rules decoded (regfix.txt:58-74): 1 subst (merge-tail addu operand order $4,$2 -> $2,$4 @44), 4 deletes (@39,41,42,43 = the post-merge hoisted ED6 read + the else-arm la E5CC), 7 inserts (3-insn lui/addu/lh %hi/%lo(D_80101ED6) into EACH arm + la $4,D_8008E5CC re-insert). Net effect: un-hoist the shared D_80101ED6 read into both branch arms - the exact thing GCC's CSE undoes from the current single-call C.

- [s1] [fable-blitz 2026-07-07] Target shape (asm/funcs/func_8003B10C.s, 64 insns, frame 0x20, saves s0/s1/s2/ra): s1=arg0 (prologue addu), s2=0x80190800 built lui/ori SPLIT across the gpu_EnableDisplay and EndADRSound jal delay slots, s0=arg0*1100 via 7-insn shift/add chain in v0 ending `sll s0,v0,2`. EDA read via s0 (assembler-macro $at addressing) -> jal func_800493E4 (delay NOP - nothing scheduled in). Branch `bne v1,5` on D_800A38DC; if-arm: lh ED2->v1, la E6A4->a0, v0=v1*3, lh ED6->v1, j .L8003B1D4 with `sll v0,v0,1` in the delay slot (x6 total); else-arm: la E5CC->a0, lh ED2->v0, lh ED6->v1, sll v0,v0,3. Shared tail: addu v0,v0,a0; addu v0,v0,v1; lbu a1,0(v0); jal 494D4 (delay addu a0,s1); jal 49584 (delay addu a0,s2).

- [s1] [fable-blitz 2026-07-07] Current C reads ED2 per arm (v0=ED2*6 / ED2*8) but ED6 exactly ONCE, post-merge, inside the 494D4 call arg - the hoisted shape. The whole 12-rule cluster exists only to duplicate that one read; the 2026-06-14 notes.md conclusion 'needs volatile (forbidden) or canonical' is SUPERSEDED: v9 proved the duplication IS pure-C-reachable via per-arm call duplication + jump2 cross-jump re-merge.

- [s1] [fable-blitz 2026-07-07] v9 (2026-06-15 adjudicator deep-dive) = arm-local `s32 v1` (the ED6 value) + func_800494D4 call duplicated INSIDE each if/else arm. Cross-jump merges the identical addu/addu/lbu/jal suffixes back to the single target call - operand order included, so the @44 subst dies too. Masked 13 = baseline+3, the closest structurally-aligned form ever recorded. CRITICAL GAP: the v9 BODY WAS NOT PRESERVED (no candidate.c in memory/wip or the grind ledger; rejected/ empty) - session 2 must reconstruct it from this description before anything else.

- [s1] [fable-blitz 2026-07-07] v9 residual (all 13 points): GCC emits `move s2,s0` in func_800493E4's jal delay slot (target has a NOP there); the else-arm addresses ED2/ED6 through s2 (asm 0x67c, 0x688) while the EDA read and if-arm keep s0 -> TWO live copies of the offset -> save set grows (s3 for addr), frame -32 -> -40. Branch bodies and merge tail already match. A `move s2,s0` copy REQUIRES two distinct pseudos both carrying arg0*1100: a single-pseudo web cannot emit it. Prime suspect: cse.c extended-basic-block canon_reg substitution (the fall-through if-arm's uses get folded to the older mult-result reg, the non-fallthrough else-arm keeps the variable's pseudo, splitting the web at the branch) - unconfirmed, needs the .greg/.cse RTL dump.

- [s1] [fable-blitz 2026-07-07] Rejected bank (10 forms, scores 19-37, imported to ledger): per-arm ed6 var (32), per-arm full pointer (26), per-arm p + shared read (19), fold-into-v0 (26), decl-order (33), register hint (33), s8* tbl cast (33), fn-scope arg carry (20), arm-local tbl+v1 (25), v0+=ED6 inline per arm + per-arm call (37). NONE of these predate-v9 forms used a do-while(0) wrap and none combined v9's chassis with a secondary RA lever - both axes genuinely unexplored HERE.

- [s1] [fable-blitz 2026-07-07] Policy updates postdating every session on this function: (a) do-while-zero-exception FINAL ruling 2026-07-06 - wraps sanctioned for ANY codegen effect incl. register allocation, single-level, inline FAKE annotation, exhaustion no longer a hard gate; (b) duplicated-statement-into-arms sanction 2026-07-01 covers per-arm duplication with byte-neutral cross-jump re-merge, though its Non-extension clause names 'duplicating CALLS (changes bytes)' - v9's call duplication is byte-neutral BY the re-merge, and the 2026-06-24 owner ruling already directed 'resume from v9', so proceed on v9 and let layer-2 apply the byte-neutrality logic; (c) split-init accumulation (var=a; var*=b) user-sanctioned 2026-06-13; (d) staged-value-reused-variable sanctioned 2026-07-03.

- [s1] [fable-blitz 2026-07-07] Transferable RA-tie mechanics from memory/grind/marionation_Exec/evidence.md: local-alloc.c qty_compare priority = floor_log2(refs)*refs*size/life, EXACT ties broken by qty BIRTH order (first-born wins the lower reg); fresh single-set temps and web GROWTH of existing pseudos both trigger the sched LAUNCH re-timing pathology; statement order controls BOTH the sched2 LUID tie AND the qty lives; decl order and s32/u32 narrowing are qty-INERT (pseudo birth follows RTL first-use). Apply to the s0-vs-s2 seat question, and beware: a wrap ANYWHERE near a call window re-timed marionation's head in 4/4 probes - place wraps away from the 493E4 window.

- [s1] [fable-blitz 2026-07-07] Owner ruling 2026-06-24 (WIP meta.json sessions[2]): park-permanently unavailable; canonical-asm signal WEAK (scan_hand_coded LOW, score 0, no STRONG signals); cycle until pure-C match or STRONG hand-coded signal; explicit direction = resume from v9 and attack the s2/s0 cascade.

- [s1] [fable-blitz 2026-07-07] Compile-verified sibling template: func_8003AF40 (same file, src/code6cac_c_ab.c:~350-385, COMPLETED-C - 0 regfix rules, not in queue) uses the identical `*(s16 *)((u8 *)&D_80101ED2 + s0)` / `*(tbl + ED2*8 + ED6)` expression spellings, same callees (493E4/494D4/49584), same addr=0x80190800 pattern with s2. Its accepted spellings are the syntax baseline for reconstructing v9.

- [s1] [fable-blitz 2026-07-07] Prologue is rule-free at HEAD: all 12 rules sit in the branch region (post-delete idx 34-44), so the current single-call C already lands s1=arg0, s2=addr, s0=offset with the correct lui/ori delay-slot split. v9's frame regression is therefore INDUCED by the arm/call restructure, not a pre-existing prologue problem - the fix must neutralize the web split without disturbing the (already correct) three callee-save assignments.
