# Evidence bank — motion_SetMotion

- WIP rejected_form: {'form': 'case 9: case 11: sel = 0xD; (with rule removed)', 'score': 0, 'reason': 'WRONG: full build SHA1 mismatch 82e50f76. Cross_jump fires when both case9/11 and D_800A3207==3 non-10 have sel=0xD â€” minimum-2 (`li s0,13; j sel_dispatch`) met â†’ blocks merged â†’ different binary.'}

- WIP rejected_form: {'form': 'swap declaration order (sel2 before result)', 'score': 9, 'reason': "Masked-sandbox 11->9 noise; RA swap persisted. Tiebreaker (allocno order) doesn't help since priorities differ (300 vs 242), not equal."}

- WIP rejected_form: {'form': 'pins removed + regfix kept', 'score': 0, 'reason': 'PROVEN this session: SHA1 46e36200 (RA mismatch). resultâ†’$s2 (wrong) without pins.'}

- WIP rejected_form: {'form': 'func_8006BEC4(0xA, sel2) instead of func_8006BEC4(0xA, -1) at early exit', 'score': 0, 'reason': "Would add 4th ref to sel2 (raising priority to 645 > result's 300), but changes binary: oracle has `li a1, -1` at dcc NOT `move a1, s2`. Cannot use this lever."}

- == imported from memory/wip notes.md ==
# motion_SetMotion (code6cac_c_mid.c) — WIP, Wall 1 SOLVED, Wall 2 open

## TL;DR (2026-07-01)
The coupled wall split and half fell. **Wall 1 (RA priority) is SOLVED
in pure C** — both pins retired (commit: cheat-cleanup 2026-07-01); full
SHA1 == oracle with the 1 remaining rule applied. **Wall 2 (cross-jump
merge) remains**: 1 regfix rule (case-9/11 sel 12→13 subst @
regfix.txt:2484). Masked sandbox is BLIND here — full SHA1 only.

## Wall 1 recipe (landed — reusable for the cluster)
Duplicate load_sel2's `sel2 = D_800A3350;` into case 0 (FAKE-annotated)
and move the label to case 13/17's copy. Flow counts the REAL second def
(sel2 reg_n_refs 3→4 → pri 851 > result 412) → RA lands sel2→$s2 /
result→$s3 naturally; jump2 cross-jump re-merges the identical
[lbu; j] tails → zero byte delta. **Label placement steers merge
direction**: label on case 13/17 ⇒ cross-jump rewrites case 0's copy ⇒
target layout exactly. Mirror labeling (m6) emits a 4-diff flipped
layout. Measured this session: dead stores are INERT for global RA
(m1: cse folds const re-stores; m3: `sel2 = v0` deleted by flow WITHOUT
being counted — 2.7.2 counts refs post-deletion). Real duplication is
the working spelling.

## Wall 2 — cross-jump no-merge (open)
Target keeps BOTH `[li s0,13; j sel_dispatch]` sites (==3 arm at ea0 as
j-delay form; case 9/11 at fc0). Writing 0xD in C merges them (SHA1
82e50f76). Ruled out this session:
- consecutive-label aliasing (sel_dispatch2:) — jump1 unifies
  same-address labels, then merge fires ANYWAY and deletes 2 insns (m7).
- ==3 arm reshaped to the two-goto form (`sel=0xF; if(==10) goto disp;
  sel=0xD; goto disp;`) matching target's delay-slot structure — still
  merges (m8).
**REG_DEAD lead REFUTED (2026-07-02):** the death-note comparison in
find_cross_jump is `#ifdef STACK_REGS` only (jump.c:2436-2467) — MIPS
never compares them. **Pairing algorithm fully mapped** (jump.c:
1966-2001 + 2371-2533): for each simplejump, (a) minimum=1 attempt vs
code-before-its-own-label, (b) minimum=2 attempts vs every other jump
in the SAME label's jump_chain. Stream-1 hitting a CODE_LABEL does
`--minimum; break` — so a [jtbl-label; set13; j] block as stream 1
gets 1 match + label bonus = merge GUARANTEED against any same-label
[set13; j] partner. ⇒ the original compile must have had the two 0xD
jumps targeting DIFFERENT CODE_LABELs at jump2 time (labels are free
in bytes — they coalesce at assembly). Remaining lever family: a
second label for case 9/11's goto that is NOT adjacent to sel_dispatch
(adjacent → m7 showed unification/merge). Known cost problem: any
real-code separation between the labels either emits bytes or gets
cross-jump-merged itself leaving a thunk `j` (+1 insn). UNSOLVED
puzzle: what C makes GCC keep two labels apart through jump1's
tensioning yet emit them at the same final address. Also note: in the
CURRENT source's cc1 output the ==3 arm's 0xD/0xF sets are NOT visible
as separate li's near the dispatch (only case-10's 13 at a bne delay)
— map the arm's actual emission before theorizing further.

## Ruled out (session 1-2, do not re-derive)
- Declaration-order swap (masked noise, swap persisted).
- `func_8006BEC4(0xA, sel2)` at early exit (bytes: li a1,-1 vs move).
- Real 4th-ref forms that add bytes; livelen restructures (init-later).

## Pointers
- tmp/probe_msm.py (m0-m9 probe harness; regenerate from git if gone).
- Wall-1 mechanics: dead-store-fake-exception.md (chain-extender note),
  register-alloc-pure-c.md, register-alloc-deep-dive.md.
- Cluster siblings to apply the Wall-1 recipe to: saEft00Add,
  marionation_Exec, cpu_side_move_dir_4 (local-alloc variant — see its
  WIP), func_8007C97C.


- [s1] [fable-blitz 2026-07-07] Current state: 0 pins (Wall 1 SOLVED 2026-07-01 via duplicated-statement-into-arms - sel2->s2/result->s3 natural, sel2-store duplicated into case 0, label on case 13/17's copy), exactly 1 rule left: regfix.txt:2483-2484 `subst "addiu\t$16,$zero,12" -> 13 @ 149` on the case-9/11 site - a semantic-lie value subst (C says sel=0xC, binary says 0xD) whose sole purpose is preventing the jump2 cross-jump merge. Queue distance 10 is masked noise: the honest-0xD form scores masked 0 but full SHA1 = 82e50f76 != oracle. Only full-build SHA1 / raw function byte-diff are valid metrics on this wall.

- [s1] [fable-blitz 2026-07-07] The wall's target topology (asm/funcs/motion_SetMotion.s): TWO identical simplejump blocks [j .L80038EDC; delay addiu s0,0xD] - the ==3 arm at L85-86 and case-9/11's jtbl block .L80038EC8 at L170-172 - jumping to the SAME label. Writing 0xD honestly in both arms merges them (WIP m1, SHA1 82e50f76, 2 insns deleted). The ==3 arm's shape [beq s1,v0,.L80038EDC delay li s0,0xF; j .L80038EDC delay li s0,0xD] (L83-86) corresponds to the two-goto C `sel=0xF; if(v0==10) goto disp; sel=0xD; goto disp;` - m8 measured that form: still merges.

- [s1] [fable-blitz 2026-07-07] THE -1-PAIR PARADOX (new this recon, the key evidence): target ALSO contains a second byte-identical same-label pair - case 8 .L80038E10 [j .L80038EDC; delay addiu s0,-1] (L119-120) and the case-10 D_800A3350!=0 subpath (L158-159, same two insns). Our committed C writes BOTH honestly (`sel = -1; goto sel_dispatch;` at src/code6cac_c_mid.c:999 and :1020-1021) and the build BYTE-MATCHES with both blocks unmerged. So the original's blocking condition is already reproduced by our C at the -1 sites while absent at the 13 sites. The discriminating feature between our -1 blocks and our (honest-0xD) 13 blocks in the SAME RTL dump IS the transplantable lever.

- [s1] [fable-blitz 2026-07-07] find_cross_jump fully mapped from source (tools/gcc-2.7.2/jump.c:2371-2533): stream1 (before e1, the jump being processed) walks prev_nonnote_insn - labels NOT skipped; hitting a CODE_LABEL does `--minimum; break` (2406-2410) = the DIRECTIONAL label bonus. Stream2 (before the partner) skips notes AND labels (2392-2393). Chain-partner attempts start at minimum=2 (1993); each rtx_renumbered_equal_p-matched real insn decrements (2528); USE/CLOBBER patterns match on the win path but do NOT count (2524-2529); first pattern mismatch (GET_CODE or rtx_renumbered_equal_p, 2469-2470) breaks matching. Merge iff minimum<=0 AND at least one counted match (2532: last1 != 0). The jump_chain contains ONLY simplejumps (jump.c:217); partner walk is first-match-wins (1986-1994); do_cross_jump has NO additional guards (2536-2583) - it deletes the e1-side matched insns and redirects e1's jump into the partner's copy.

- [s1] [fable-blitz 2026-07-07] Model VALIDATED against landed bytes: the Wall-1 sel2-tail merge replays exactly under this trace - e1 = case-13/17's j (its lbu preceded by the load_sel2 label -> 1 match + label bonus -> minimum 0), the 13/17-side copy is deleted and its jump redirected into case 0's copy = target's .L80038DE0 [j .L80038DD4; delay li s0,6] with .L80038DD0 falling into .L80038DD4 [lbu s2; j]. Mechanically confirms 'label placement steers merge direction': the label-bonus side is the DELETED side. CONSTRAINT for any Wall-2 lever: it must be 13-pair-LOCAL - suppressing cross-jump globally would unmerge the sel2 tails and break the landed Wall-1 byte shape.

- [s1] [fable-blitz 2026-07-07] By the same validated model the -1 pair MUST merge (e1 = case-8's j: iter1 set-1 == set-1 counts 1, iter2 prev_nonnote = the jtbl CODE_LABEL -> bonus -> minimum 0 -> merge) - yet it does not merge, in our build NOR in the original. Therefore a real blocking condition sits outside jump.c-as-read: candidates visible in one dump = pattern divergence at jump2 time (one set-1 not `(set (reg s0) (const_int -1))`), an intervening real/byte-free insn adjacent to one jump post-sched2, REG-note structure, or the jumps not actually sharing a JUMP_LABEL rtx.

- [s1] [fable-blitz 2026-07-07] Two-label route CLOSED by source: mark_jump_label's consecutive-label canonicalization (jump.c:3177-3193) keeps labels separated by NOTE_INSN_LOOP_BEG distinct at jump1 (cross_jump=0, lines 3189-3192), but jump2 re-runs it with cross_jump=1 where ALL notes AND USE/CLOBBER insns are skipped (3183-3188) - so byte-free-separated labels UNIFY at exactly the pass that merges, and any real separator insn emits bytes. m7's failure (consecutive labels unified, merge fired anyway) is fully explained; NO byte-free label-separation spelling exists. The WIP's conclusion 'the original must have had two different CODE_LABELs at jump2 time' is therefore almost certainly WRONG - the -1 pair proves a different protection mechanism exists.

- [s1] [fable-blitz 2026-07-07] Pass-order facts: the ONLY cross-jump-enabled call is jump_optimize(insns, 1, 1, 0) at toplev.c:3142, AFTER sched2 (toplev.c:3125) and BEFORE reorg/final - so suffix adjacency at merge time is post-sched2 order; USE/CLOBBER insns still exist at that point, emit nothing at final, and reorg's backward delay-slot scan skips them - a [set13; CLOBBER; j] block still yields target's [j; delay set13] shape while breaking find_cross_jump's iter1 from BOTH directions (stream2 skips only notes/labels, so the CLOBBER is compared and GET_CODE(SET) != GET_CODE(CLOBBER) breaks at 2469 before any label bonus).

- [s1] [fable-blitz 2026-07-07] Byte-free C-reachable emitters located by grep: (a) standalone `(use return_reg)` emitted for every value-return (stmt.c:2532) - explains marionation's 'cross-jump does NOT fire' note (their 2-insn [move v0,a2; j epi] return tails: the USE matches but doesn't count -> 1 counted match < minimum 2 -> no merge; NOT a global config fact - cross-jump demonstrably fires here per m1 and per the landed sel2-tail merge); not deployable mid-function. (b) bare `(clobber target)` from union-typed constructor expansion (expr.c:2996, 'Inform later passes that the whole union value is dead') and struct-return paths (calls.c:1784) - the C-reachable insertable-break candidate; survival past flow1/local-alloc/reload to jump2 is UNMEASURED.

- [s1] [fable-blitz 2026-07-07] Rejected inventory (do not re-derive): m1 honest-0xD both arms = merge (82e50f76); m7 adjacent second label (sel_dispatch2:) = jump1 unifies, merge fires; m8 two-goto arm reshape = merge; REG_DEAD-note lead REFUTED (death comparison is #ifdef STACK_REGS only, jump.c:2433-2467); func_8006BEC4(0xA, sel2) at the early exit = wrong bytes (target has li a1,-1 at 0x80038CD4, not move a1,s2). Wall-1-era rejecteds (decl-order swap, pins-off+regfix-on 46e36200) are moot - Wall 1 landed. The 2026-06-24 owner-ruling levers: 'single-shared-store letting cross-jump RESPREAD' is impossible by source (do_cross_jump only merges/deletes, never duplicates - jump.c:2536-2583); 'sel2-lifetime + post-early-exit sequencing' was Wall-1-scoped (moot); 'USE/CLOBBER suffix-break' is refined into frontier F2 with the emitter sites now located.

- [s1] [fable-blitz 2026-07-07] do-while(0) (sanctioned 2026-07-06, POSTDATES m7/m8): direct note effects are inert on every comparison surface of this wall by source reading (prev_nonnote_insn at 2389, stream2 note-skip at 2392-2393, label-unification note-skip at 3187-3192) - any wrap effect on cross-jump would be INDIRECT (changed RTL emission of the wrapped arm / sched2 adjacency). Unmeasured on this function; cheap to sweep; low prior.

- [s1] [fable-blitz 2026-07-07] Executor metric warning: the masked sandbox CANNOT see this wall (m1 scored masked 0 while the SHA1 diverged) - every probe verdict needs the full-build SHA1 gate or a raw objdump byte-diff of the function window; sandbox-0 is necessary-not-sufficient and `retire` is the only proof.
