# Hypothesis ledger - func_8005C8A8

## OPEN (floor 33, 2026-09-30): the `size` 0x4F0 stack slot. Owner Q45 option B REFUSED the cancellation spelling
(`size = (s32)tile + 0x4F0 - arg2` and every cancel-against-a-second-name variant). See evidence.md s3c + s3d. Look
for another source form that keeps the once-set constant out of update_equiv_regs' REG_EQUIV rewrite.

## LEAD (orchestrator, 2026-09-30; not yet spent)
The target's `li/sw` at entry + `lw` at the return is a reload SPILL of a pseudo live across the whole body
(slot 0x70 in pseudo order between mode_off 0x68 and the xpos orphan 0x78), not a declared stack local. So
look for a source value GCC keeps live the whole body with no REG_EQUIV constant that is genuinely NOT a
constant in the C: a size derived from real prim-pointer arithmetic, a count of emitted prims times a sizeof,
or a value computed from arguments that happen to be the same at every call site -- provided it is not a
cancellation of one value against itself or a second name for it (owner Q45). Callers for the latter check:
src/code6cac_c2.c:748, src/code6cac_tu2.c:2082, src/ings.c:468/470 (arg2 = D_800A38B4 at every site).

## s4 (2026-09-30, laneC): KILLED volatile / s16 / one-member struct / one-element array / `return 0x4F0;`
(probes/s4/scores.txt). The slot needs a once-set pseudo with no REG_EQUIV constant, i.e. a set whose constant
cse cannot see (evidence.md s4: cse.c:6918-6934, local-alloc.c:1019-1032, reload1.c:567-586/2381-2385).
SOTN precedent search for such a spelling in matched PS1 code: negative (evidence.md s4).

## s3 RESOLVED (floor 0): the hole is a combine-orphan `(use)` of the dead sign-extension temp of an always-zero
s16 local read once after the case-0 label (Q27 (A)); see evidence.md s3. H1 (a)/(c) and H2 are moot; H1 (b)'s
family (combine orphans) was right, via the known-zero fold rather than a stale reg_n_refs.

## Was open (s2 frontier, floor 30 = only the 8-byte frame hole at sp+0x78)
H1. The hole is a MEM pseudo X (created after `size`, before the final-loop temps) whose every reference vanishes in
    reload. Mechanisms in tools/gcc-2.7.2/reload1.c that leave allocated-but-unreferenced frame space:
    (a) delete_output_reload "forget we had a stack slot": X LOCAL (one block, one death), set and single use with
        no label/jump between, the use served by the inherited reload reg;
    (b) REG_UNUSED output of a MEM pseudo (no store emitted) when X's uses were removed after flow (combine's stale
        reg_n_refs case, combine.c header note) so X still has n_refs>0 at global time;
    (c) setup_save_areas slot for a caller-saved hard reg that a later spill made unnecessary (needs X's slot AFTER
        the initial pass: contradicted by 0x78 < 0x80(ot*4) < 0x88((s16)k) unless those two were kicked out of
        different hard regs).
    Only visible t0-computed-and-consumed value in the target: `mflo t0; sll v0,t0,3` (icon product j*count). Fits
    (a) if the product pseudo (pref LO_REG, else GR_REGS; global because CLASS_LIKELY_SPILLED) were MEM, but LO has
    no competitor in this function. Unresolved.
H2. The target's value that nv2 approximates (a pseudo equal to mode near the final frame block) comes from a
    construct cse cannot see through, and its final-tile read is mode's own slot. Untried: the frame section as a
    separate block scope / helper with real work taking mode, a copy of mode read from memory, a different spelling
    of the final dim tile.

## Killed (s2): see evidence.md "Hole investigation" for the measured list.

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: active; INCLUDE_ASM (`src/text1b.c:4035`). QUEUE DISTANCE 752 IS STALE: real floor is 33 = `candidate.c` (literal `size = 0x4F0`) + `fix1-merges.patch` (5 merges into include/game.h + 6 undefined_syms rows; passed layer-2; `git apply --check` still clean). `rejected/size-tile-cancel-0.c` scores 0 + oracle SHA1 but is REFUSED. `state.json` is stale too (session_count 1, floor 752, frontier = resolved s1 recon list) - a Grinder session would restart from recon; sync it first. 5c543ce1d moved the function from text1b_tu1c.c back into text1b.c: re-measure every banked score.
- CONSTRAINTS: owner Q45 refuses `size = (s32)tile + 0x4F0 - arg2` and any cancel-against-a-second-name variant (`docs/grind/owner-rulings-2026-09-26.md:456-459`, `docs/grind/borderline.md:154-160`). Layer-2 r1 also failed the `end_off = arg2+0x4F0; size = end_off - arg2` round trip (evidence.md s3b item 2). `xpos` always-zero local admitted under Q27(A), already passed layer-2 (`.claude/rules/named-local-fake-exception.md:39-56`). No volatile (Q48), no dummy subscripts (Q22).
- BLOCKER: target sets 0x4F0 once at entry into spill slot 0x70 and reloads it at return. A literal gets REG_EQUAL in cse -> REG_EQUIV in update_equiv_regs (`tools/gcc-2.7.2/local-alloc.c:1019-1032`) -> reload rematerializes (`reload1.c:567-586`), slot vanishes. [I] a two-set accumulator can't escape either: flow deletes dead sets before counting, reg_n_sets ends at 1 (`flow.c:1490, :2079`).
- PLAN:
  1. Re-baseline: apply fix1-merges.patch, splice candidate.c into text1b.c, `sandbox --disable all`, confirm 33.
  2. NEW EVIDENCE never shown to the owner: finished siblings in the same file, func_8005D814 (`src/text1b.c:4151/4308`) and func_8005E098 (:4364/4456), use `end_off = arg2 + K; ... return end_off - arg2;` with the same frame slots (0x68 mode_off, 0x70 end_off), and their target bytes really compute it (`asm/funcs/func_8005D814.s:33-35` `addiu t4,t4,0x304; sw t4,0x70(sp)` + `subu` at return).
  3. Bank cheap measurements: the sibling-faithful `return end_off - arg2;` spelling (honest baseline) and a 3-step running-offset accumulator (`off = 0xF0; cur = arg2+off; off += 0x3E8; mode_off = arg2+off; off += 0x18`). Expect ~33 [I].
  4. If both miss: NEW borderline question framed as new evidence under proven-spelling-class-reconstruction.md (not a Q45 re-ask): "same author's siblings show the end-pointer idiom; may the subtraction sit at entry?" NEEDS OWNER RULING.
  5. If refused again: rotate with this mechanism proof.
- DEPENDS: fix1-merges.patch edits include/game.h (coordinate). `Env5C8A8` duplicates `Env5E54C` (`text1b.c:4462`) - unify on landing; same descriptor type as func_8005D554 and finished func_8005D46C / func_8005FA98. named_syms misnomers in evidence.md s3b need a naming_wave reset.
- ODDS/LANE: manual only (owner question + layer-2). ~10% to 0 without a new ruling.

## s5 (2026-10-01, laneA) — floor 33 confirmed; QUESTION filed (borderline.md 2026-10-01)
KILLED: running-offset accumulator (a1-a3, 33: flow deletes the dead earlier sets, reg_n_sets 1); sibling-faithful
return-site `end_off - arg2` (e1, 110); struct constructor (u3, 33); s64 (d1, 88). Union constructor (u2/u4) keeps the
slot via a clobber (4) but is a no-purpose wrapper. state.json synced. See evidence.md s5, probes/s5/scores.txt.
