# Hypothesis ledger — func_80036FD4

## Resolved in session 1

**H0 (CONFIRMED).** The honest 17 floor is two independent things: (a) 3 missing
instructions caused by cse folding the reload of D_80101E60 into the arg0
sign-extend chain, and (b) a register-allocation residual (the 8 regfix rules).
(a) is solved by giving cse a memory-flushing store between the `sh` and the
read. Mechanism, the measurement, and the killed alternatives: see evidence.md.

**H1 (CONFIRMED).** An aggregate (BLKmode-destination) assignment writing
D_80101E6C/D_80101E70 as one 8-byte record flushes cse's memory table
(`note_mem_written` → `writes->all`), restoring both target reloads of
D_80101E60 and target's `lw/lw/sw/sw` shape. Floor 17 → 9, build_insns 76 → 78.

**H2 (KILLED).** "An intervening `sw` to a different absolute global, or an
`extern volatile` on one of those globals, defeats the store-to-load forward."
It does not — cse's `invalidate_memory` only removes varying-address entries
unless `writes->all` is set, and a volatile store merely goes unrecorded. The
pre-session build had both and still folded.

## Live frontier (session 2+)

**H3 — declaration cleanup (owed before any completion claim).** The working
form uses the probe spelling `*(CamPair *)&D_80101E6C = *(CamPair *)entry;`.
That is a typed re-view of globals (pointer-alias family) and should not be
committed as-is. The clean spelling is to declare the record as a struct-typed
global in `include/code6cac.h` (the file already treats `&D_80101E6C` as a
buffer pointer at code6cac_b2_post.c:277) and assign it directly, with the same
treatment for the source side (`SpecialCam` as an array of that record type,
matching `(s32)&SpecialCam + idx*8` at code6cac_b2_post.c:365 and :508/:513).
*Next probe:* introduce the struct declaration, drop both casts, re-sandbox and
confirm the score stays at 9 (bytes must be unchanged); then the pre-existing
`extern volatile s32 D_80101E70;` cheat at code6cac_b2_post.c:45 can also be
dropped for this function.

**H4 — the reload's schedule position is the whole remaining gap.** Target's
`lh` sits after both `sw`s and pays a load-delay `nop` (the missing 79th
instruction); our cc1 sched1 hoists it above the two entry loads because it has
the longer critical path to the jal argument. In target the reload's destination
is `$a0`, which is anti-dependent on `sw a0, %lo(D_80101E6C)` — that
anti-dependence is what pins it late, and it is a *consequence* of the words
landing in `$a0/$a1`, so the register residual and the nop are one coupled
problem, not two.
*Next probe:* dump cc1's sched1 output (`-da`, keep the `.sched` / `.greg` dumps
in `tmp/grind/func_80036FD4/sN/`) and read the ready-list priorities for the
`lh` versus the two `lw`s. Then look for a C shape that lengthens the entry-copy
chain or shortens the reload's chain to the jal — e.g. splitting the
`D_80101E74 = BcdToFrames(...) + ... - 0x96` statement so the call argument is
computed in its own statement AFTER the record copy, or expressing the call
argument via the already-live `entry` pointer relationship instead of a fresh
index computation.

**H5 — force `emit_block_move` instead of `move_by_pieces`.** If the aggregate
copy stayed an opaque block-move insn through sched1 rather than being expanded
into two SImode moves at expand time, it would be a hard scheduling barrier and
the `lh` could not be hoisted above it — which is precisely target's shape. Our
8-byte, 4-byte-aligned struct takes the `move_by_pieces` path.
*Next probe:* measure whether a differently-shaped record (e.g. a declared
alignment or member layout that makes GCC take the `movstrsi` path) changes the
schedule — and, importantly, whether it does so WITHOUT changing the emitted
load/store shape. Reject immediately if the record shape has no independent
justification from the data layout; the record's real shape must come from how
the bytes at 0x80101E6C are used elsewhere, not from what schedules well.

## [s1] The honest 3-instruction shortfall (76 vs 79) is GCC cse forwarding the just-stored arg0 to the reload of the s16 global D_80101E60, replacing the target's lui/lh/nop/sll with a second 'sra (arg0<<16),13'.
- mechanism: cse.c records a store's MEM in the src's equivalence class; ordinary sw stores to distinct absolute symbols only set writes->var, which invalidate_memory (cse.c:1700) uses to remove varying-address entries only, so D_80101E60's constant-address entry survives and the read folds. The pre-session source bought the reload with asm volatile("" ::: "memory"), which the sandbox strips - hence the 17 floor.
- probe: Read asm/funcs/func_80036FD4.s against the cheat-stripped sandbox disassembly (tmp/grind/func_80036FD4/s1/dis.sh); read tools/gcc-2.7.2/cse.c note_mem_written (7539) / invalidate_memory (1700) / the dest-recording guard at 7310-7340.
- result: Target reloads D_80101E60 twice (once before the jal, once after); our build emitted the pre-jal one as a duplicate sra off arg0<<16. Exactly 3 instructions, exactly the shortfall.
- verdict: CONFIRMED

## [s1] Writing D_80101E6C and D_80101E70 as ONE aggregate assignment gives the store a BLKmode destination, which sets note_mem_written's writes->all, flushes cse's entire memory table, and restores the target reload in pure C.
- mechanism: note_mem_written (cse.c:7539) sets *writes_ptr = everything for a BLKmode written MEM; invalidate_memory then removes every in_memory entry including constant-address ones. move_by_pieces then emits the 8-byte aggregate as two SImode symbol-addressed stores, which is target's lw/lw/lui+sw/lui+sw shape (target's %lo(D_80101E70) == the aggregate's sym+4). Independent justification for the record: code6cac_b2_post.c:277 already passes &D_80101E6C to cdrom_BcdToFrames/cdrom_FramesToBcd as a buffer.
- probe: Replaced the two scalar stores + the asm memory barrier with a single 8-byte record assignment; sandbox func_80036FD4 --disable all.
- result: score 17 -> 9, build_insns 76 -> 78 (target 79). Both D_80101E60 reloads now present, load-load-store-store ordering matches target.
- verdict: CONFIRMED

## [s1] An intervening sw to a different absolute global, or an extern volatile qualifier on one of the stored globals, defeats the store-to-load forward.
- mechanism: Claimed cse memory invalidation from a plain or volatile store. In fact invalidate_memory only removes varying-address entries unless writes->all is set, and a volatile MEM store merely sets do_not_record for that store (canon_hash, cse.c:1943) without calling invalidate_memory.
- probe: The pre-session build already had both (two sw to distinct globals between the store and the read, and extern volatile s32 D_80101E70 at code6cac_b2_post.c:45) and was measured at floor 17 with the reload folded away.
- result: Fold survived both. Neither is a flush mechanism; the volatile on D_80101E70 is dead weight for this function and can be dropped.
- verdict: KILLED

## [s1] Computing the entry index from the just-stored global (D_80101E60 * 8) instead of from the parameter ((arg0 << 16) >> 13) changes the emitted first index.
- mechanism: cse forwards the first read to the stored arg0 and materialises sign_extend*8 as sll 16 / sra 13 either way, so both spellings are byte-identical.
- probe: Swapped the spelling with the aggregate store in place; sandbox --disable all.
- result: score unchanged at 9, identical bytes. Kept the D_80101E60 * 8 spelling as the single-source-of-truth form (matches the sibling at code6cac_b2_post.c:365).
- verdict: KILLED

## Resolved in session 2 (structural)

**H3 (PARTIALLY RESOLVED — reframed, still owed).** The declaration cleanup is still
owed, but its justification is now much stronger and its scope is now known to be
owner-level. The record is real (see evidence.md "Record justification"), and the
committable form declares it once in include/code6cac.h in place of the per-word
externs D_80101E60/E62/E64/E68/E6A/E6C/E70/E74. That refactor was deliberately NOT
done in-session: those symbols are referenced from other files, and unifying them into
one object changes their mutual aliasing, so it needs a full-build SHA1 check. It does
NOT change this function's score — the emitted relocation addends are identical
whether the record comes from a header declaration or from the in-function
`(ReplayCamRec *)&D_80101E60` cast. So the cast is a probe spelling that must be
replaced before the completion commit, not a blocker on the measurement.

**H4 (CONFIRMED, with s1's mechanism corrected).** The reload's schedule position was
indeed the whole remaining gap, but: (a) the actor is sched2, the POST-RELOAD
scheduler, not sched1; (b) the cause is not ready-list priority — every insn in the
region has priority 1-4 and the decision is made on availability. The reload cannot be
placed at T-14 (load latency 2 to its consumer at T-13), the block move is the only
insn ready in that slot, so sched2 sinks the block move past the reload. That single
decision costs both the missing `nop` AND the $a1/$a2-vs-$a0/$a1 register residual —
so s1 was right that they are one problem, and the direction is scheduling ->
allocation. Fixed by giving the block move and the reload a shared base symbol, which
creates a real `true_dependence` between them. Floor 9 -> 4, build_insns 78 -> 79, and
the registers land on target with no register-directed work at all.

**H5 (KILLED AT ITS PREMISE).** "If the aggregate copy took emit_block_move/movstrsi
instead of move_by_pieces it would survive as an opaque memory insn through sched1 and
hard-block the hoist." The copy has been a single `movstrsi_internal` block-move insn
the whole time (cc1 -da combine dump insn 53, pattern 177), and the scheduler sinks it
past the reload regardless — an opaque block-move insn is NOT a scheduling barrier, it
is just an insn with no dependence on a load from a different symbol. There is
therefore nothing to gain from reshaping the record (member layout, declared
alignment, padding): `memrefs_conflict_p` refuses to conflict two distinct symbol_refs
at ANY size or shape. Banked as
rejected/distinct-symbol-aggregate-copy-cannot-force-sched-dep.c.

**H6 (CONFIRMED, new).** The last two scored instructions were an `INSN_LUID`
tie-break, i.e. pure C statement order. `sched.c rank_for_schedule()` resolves equal
priority on original RTL order, and target materialises the `SpecialCam` base BETWEEN
the sign-extend's `sll` and its `sra`. Splitting the index read from the scaling
(`idx = rec->unk00;` then `cam = &SpecialCam;` then `idx * 8`) puts the symbol there,
because combine fuses the sra16 with the sll3 at the sll3's position. Three-form
measured sweep in evidence.md: sll,sra,sym -> 4; sym,sll,sra -> 4; sll,sym,sra -> 2.

**H7 (CONFIRMED, new — this is the session's disposition).** The residual 2 is not an
instruction difference. All 79 instruction words match; the two scored words are
R_MIPS_HI16/R_MIPS_LO16 symbol+addend pairs (`D_80101E60+12/+16` versus splat's
`D_80101E6C`/`D_80101E70`), which link to identical bytes because GNU ld resolves an
o32 HI16 using the following LO16's addend. And the addend spelling is FORCED: the
dependence that buys the nop requires a shared base symbol, and base 0x80101E60 is the
cheapest choice (2 words versus 3 for base 0x80101E6C). So no further pure-C search
can lower this number — the function is finished as a C problem. Completion is blocked
only on deleting the 8 now-harmful regfix rules (with rules enabled the score is 4,
i.e. they corrupt correct output), which is `retire`/regfix.txt and outside a grind
session's surface. Filed as an INTEGRATION HANDOFF in docs/grind/decisions.md with the
exact operator steps. NOT build-verified in-session; `retire` is the one command that
confirms or refutes the link-identity claim.

## Live frontier (session 3+ / operator)

Nothing on this function is grindable in C any more. The three open items are all
integration, in order:
1. `retire func_80036FD4` + full-build SHA1 — the decisive test of H7. If it
   auto-rolls-back, H7 is refuted, the residual is real, and the function returns to
   the queue with that measurement banked.
2. The include/code6cac.h record declaration replacing the per-word externs (H3),
   score-neutral for this function but needs a full-build check because it changes
   other files' aliasing.
3. Fresh layer-2 cheat-reviewer on the body; the only construct needing a ruling is
   the `(ReplayCamRec *)&D_80101E60` typed re-view, which item 2 removes.

## [s2] The aggregate copy must share a BASE SYMBOL with the halfword global it is ordered against; only then does a true_dependence exist between the BLKmode store and the reload, which is what keeps the block move out of the reload's load-delay slot and so restores target's nop AND target's $a0/$a1 allocation.
- mechanism: sched.c:817 true_dependence -> memrefs_conflict_p with SIZE_FOR_MODE(BLKmode) == 0; for two constant addresses the recursion bottoms out at `if (CONSTANT_P (y)) return rtx_equal_for_memref_p (x, y) && (xsize == 0 || ysize == 0 || ...)`. The size-0 clause makes a BLKmode store conflict with anything at the SAME base symbol, and rtx_equal_for_memref_p makes two DISTINCT symbol_refs never conflict at any size or shape. Without the dependence, sched2 (the post-reload scheduler, not sched1) finds the block move to be the only insn ready at T-14 — the reload cannot go there because its consumer sits at T-13 and the load latency is 2 — and sinks the block move past the reload. That one decision consumes the load-delay slot (killing the nop, 78 vs 79 insns) AND leaves the reload's pseudo live across the block move, so reload hands the movstrsi scratches $a1/$a2 instead of target's $a0/$a1.
- probe: Declared the block at 0x80101E60 as one record (s16 x6, then an 8-byte pair member at +0xC, then s32 at +0x14) and accessed both the halfword and the pair through it; sandbox func_80036FD4 --disable all. Read the sched/sched2 traces in tmp/grind/func_80036FD4/s2/dump-cur/ to confirm the pre-fix decision verbatim.
- result: Floor 9 -> 4, build_insns 78 -> 79 (== target). The nop appeared and the registers fell onto target's $a0/$a1 with zero register-directed work. sched2 trace before the fix: ';; ready list at T-14: 53 (1), now 53' then ';; launching 62 before 53 with no stalls at T-15'.
- verdict: CONFIRMED

## [s2] s1's H5 — 'if the aggregate copy took emit_block_move/movstrsi instead of move_by_pieces it would survive as an opaque memory insn and hard-block the hoist' — is false at its premise, and no record RESHAPING (member layout, declared alignment, padding) can help.
- mechanism: The copy has been a single movstrsi block-move insn all along, so there was never a move_by_pieces expansion to defeat. And an opaque block-move insn is not a scheduling barrier: it is simply an insn with no dependence on a load from a different symbol, so the scheduler reorders across it freely. Since memrefs_conflict_p refuses to conflict two distinct symbol_refs regardless of size, the record's SHAPE is not a variable at all — only its BASE is.
- probe: cc1 -O2 -G0 ... -da on the sandbox .i; read the combine dump for the copy insn (tmp/grind/func_80036FD4/s2/dump-cur/f-base.i.combine.txt) and the sched/sched2 traces for the reorder.
- result: Dump insn 53 is `(set (mem:BLK (symbol_ref "D_80101E6C")) (mem:BLK (reg 78)))`, pattern `177 {movstrsi_internal}` — already a block move — and sched2 still sank it past the reload. Banked as rejected/distinct-symbol-aggregate-copy-cannot-force-sched-dep.c.
- verdict: KILLED

## [s2] The last two scored instructions were an INSN_LUID tie-break, i.e. decided purely by C statement order: target materialises the SpecialCam base BETWEEN the sign-extend's sll and its sra.
- mechanism: sched.c rank_for_schedule() resolves equal-priority ties on `INSN_LUID (tmp) - INSN_LUID (tmp2)` (original RTL order). Splitting the index read from the scaling makes `idx = rec->unk00` expand as sll16+sra16, `cam = &SpecialCam` the next insn, and `idx * 8` as sll3 — and combine then fuses the sra16 with the sll3 AT THE sll3's position, i.e. after the symbol. Result: sll, sym, sra.
- probe: Three-form measured sweep of the entry-address computation, each measured with sandbox --disable all.
- result: `&SpecialCam + rec->unk00 * 8` -> emitted sll,sra,sym -> score 4. `u8 *cam = &SpecialCam;` as the first initialiser -> sym,sll,sra -> score 4. Split form (idx, then cam, then idx*8) -> sll,sym,sra -> score 2. Sweep table banked in evidence.md.
- verdict: CONFIRMED

## [s2] The residual 2 is not an instruction difference and cannot be lowered by any further pure-C work, because the addend spelling is forced by the very dependence that buys the 79th instruction.
- mechanism: All 79 instruction words match target; the two scored words differ only in their R_MIPS_HI16/R_MIPS_LO16 symbol+addend pairs (ours D_80101E60+12/+16, splat's D_80101E6C/D_80101E70 with addend 0). 0x80101E60+12 == 0x80101E6C and +16 == 0x80101E70, and GNU ld resolves an o32 R_MIPS_HI16 using the addend of the FOLLOWING R_MIPS_LO16, so both spellings link to identical bytes. The sandbox compares UNLINKED object words, and splat invented a separate symbol for every word at 0x80101E60.., so the difference is symbol attribution (cf. memory/project/splat-symbol-names-are-not-evidence.md). The addends are unavoidable because the dependence requires a shared base, and base 0x80101E60 is the cheapest base available.
- probe: objdump -dr on tmp/sandbox/func_80036FD4/code6cac_b2_post.o to read the relocations directly; normalised opcode+register diff via tmp/grind/func_80036FD4/s2/cmp.py; base-choice arithmetic for the alternative record bases.
- result: Exactly two differing words: `sw a0,12(at)` / `sw a1,16(at)` with R_MIPS_LO16 against D_80101E60, versus target's addend-0 relocs against D_80101E6C/D_80101E70. Base 0x80101E6C instead would cost 3 words (moving addends onto sw#2 at +4, the sh at -12 and the pre-jal lh at -12); varying-address pointer spellings of the store change the addressing to lui/addiu/sw/sw and cost more. So 2 is the measured minimum for the only dependence mechanism that exists.
- verdict: CONFIRMED
