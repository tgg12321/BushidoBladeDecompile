/* MIGRATION BANNER (asm-until-matched, owner ruling 2026-08-19): func_800480C0 is
 * committed on main as INCLUDE_ASM("asm/funcs", func_800480C0) in src/text1b.c. This
 * file is a CANDIDATE, not the state of HEAD; every 'measured on main' statement in the
 * headers below means 'measured with this body installed over that INCLUDE_ASM line'.
 * Install with tmp/grind/func_800480C0/s3/install.py. */
/* s10 (rederive, 2026-09-02, chassis HEAD a0198d09) - BODY UNCHANGED, floor
 * re-measured 20 (74/74, `sandbox func_800480C0 --disable all`). The mandated
 * rederive axes were all three run to the end and they CONVERGE on this body:
 *  1. FRESH m2c. `tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax` on
 *     asm/funcs/func_800480C0.s produces the same control flow this body has
 *     (the only deltas are m2c's s32-typed arg4/arg5 and a post- rather than
 *     pre-decrement loop test). Spelled out as a chassis it emits 71 insns with
 *     vars=0: dropping the second base carrier costs the `move $18,$16` the
 *     target ships, so the m2c shape is one insn SHORT, not a new lever.
 *  2. SIBLING TRANSPLANT - THE DECISIVE RESULT. func_80047FBC (src/text1b.c:82)
 *     is this same routine with 4 parameters instead of 6, and it is COMPLETED-C
 *     on main. Its body is line-for-line the body below (same `base_addr`/`p`
 *     init chain, same annotated `arg0 = 0;` dead param store, same in-loop
 *     cursor arithmetic, same `while ((count--) != 0)` tail). The ONLY
 *     structural difference between the accepted sibling and this candidate is
 *     its leading `volatile u32 pre_pad[8];`, granted by the 2026-08-20 owner
 *     ruling at engine/volatile_cheats.py:757-758. func_80047EE8 (src/text1b.c:35)
 *     and func_800481E8 carry the identical grant. So the rederive axis does not
 *     have a different shape to find: the shape is already on main three times.
 *  3. STRUCTURAL RESPELLINGS - 10 new forms measured (55 total on this body),
 *     every one vars=0 / unalloc=0:
 *       b1 fresh-m2c chassis (71 insns)      b2 u8* byte-cursor chassis (72)
 *       b3 `long long` loop counter (86, regs=10 - a DImode pseudo that IS
 *          allocated pays REGISTERS, not vars)
 *       b4 `unsigned long long` scale intermediate with a provably dead high
 *          word (76 - cc1 lowers it to SImode and orphans nothing)
 *       b5 12-byte struct-record chassis (63) b6 for-index chassis (62)
 *       b7 sub-word read of the two STACK-passed s16 params (68)
 *       b8 sub-word read of all four s16 params (68)
 *       b9 s16 round-trip temporaries, stream preserved (72)
 *  TWO LIVE FRONTIER ITEMS CLOSED. (a) s9 frontier 1 (DImode route to four
 *  phantoms) is dead: b3/b4 are the two ways a long long can enter this body and
 *  neither leaves an unallocated DImode pseudo. (b) s9 frontier 3 (the two
 *  stack-passed s16 args as a class-A site) is dead in the useful direction: the
 *  substitution DOES fire there - b7 drops 72 -> 68 insns because both
 *  `lw + sll + sra` triples fold into `lh` - but it produces unalloc=0 and it
 *  DELETES four instructions the target ships. Class A remains a byte-cost, not
 *  a phantom source, on this body.
 *  MANDATED FAKE RE-AUDIT (rejected/s10-candidate-minus-fake-reseats-registers.c):
 *  with the annotated `arg0 = 0;` physically removed the body still emits 72
 *  insns with vars=0/unalloc=0, but the register seating changes (base carrier
 *  $22 instead of $18/$s2, second base use binds $4) and the prologue reorders.
 *  The FAKE is load-bearing for the stream and masks no phantom lever - s8's
 *  ablation verdict re-confirmed on the current chassis.
 *  Instruments: tmp/grind/func_800480C0/s10/{probe.sh,runall.sh,runall2.sh,
 *  gen_bodies.py,gen_bodies2.py,bank.py,bodies/,bodies2/,bodies3/}. */
/* s9 (forensics, 2026-09-02, chassis HEAD 0c7f30e4) - BODY UNCHANGED, floor
 * re-measured 20 (74/74). Both s8 frontier items closed NEGATIVE, and the
 * class-A producer is now fully named:
 *  1. THE FAMILY HAS EXACTLY FOUR MEMBERS. grep for callers of func_800482C8
 *     over asm/funcs/*.s + src/*.c returns only the four known siblings; there
 *     is no fifth batch loader anywhere in the binary, so no live sibling can
 *     name the 32-byte object.
 *  2. func_80041AC8 IS NOT A THIRD PRODUCER CLASS. Its three orphan pseudos
 *     (115/105/85, .cse insns 171/106/26) and SetDrawEnv's three (140/137/128,
 *     .cse insns 217/209/166) are defined by the IDENTICAL RTL pattern
 *     `(set (reg:SI P) (ashift:SI (subreg:SI (reg:HI Q) 0) (const_int 16)))`
 *     followed by an `ashiftrt ... 16` carrying REG_EQUAL (sign_extend (reg:HI Q)).
 *     One class-A producer in this tree, not two.
 *  3. ITS PRECONDITION IS BYTE-VISIBLE AND ABSENT HERE. combine deletes that
 *     ashift only by substituting a MEMORY equivalent for the HImode pseudo and
 *     re-forming the extension as a signed narrow load - func_80041AC8 pays for
 *     each orphan with an emitted `lh $2,0($16)`, SetDrawEnv with `lh $5,22($sp)` /
 *     `lh $2,D_8009BE78`. The substitution is gated by can_combine_p's
 *     use_crosses_set_p (tools/gcc-2.7.2/combine.c:917). func_800480C0's shipped
 *     stream has ZERO `lh`, and each of its four `lhu $aN,0x0($s0)` loads carries
 *     an `addiu $s0,$s0,0x2` on the base register between the load and its
 *     sll/sra pair - which is precisely why those pairs survive into the target
 *     bytes. Class A is closed here on a cited predicate, not by analogy.
 *  Four new spellings measured (45 total on this body), all vars=0/unalloc=0/
 *  orphanUSE=0: immediate s32 sign-extends of the u16 loads, and three
 *  combine-foldable chain extensions of the arg1 scale (chain length is not the
 *  lever - shift merges rewrite i3 in place and orphan nothing).
 *  Instruments: tmp/grind/func_800480C0/s9/{dump.sh,dump2.sh,run.sh,bodies/,dumps/}.
 *  Full record: evidence.md / hypotheses.md s9. */
/* s8 (forensics, 2026-09-02, chassis HEAD 28583e8e) - BODY UNCHANGED, floor
 * re-measured 20 (74/74). Mandated FAKE re-audit: tools/fake_ablate.py finds one
 * FAKE unit (the annotated `arg0 = 0;`) and scores keep-all 20 / drop-1 32 - the
 * store is load-bearing and masks no lever. Three forensic results: (1) every
 * caller of the four text1b siblings passes scalars and func_800482C8 owns its
 * own locals, so the 32 bytes are not a caller-built record; (2) all four
 * siblings' shipped frames reserve the IDENTICAL untouched window 0x18-0x37 with
 * args=0x18 and callee-saved base 0x38 despite 2/4/6/2 parameters - the signature
 * of one shared source declaration, not of shape-dependent allocation residue;
 * (3) a filter-free census of all 1096 functions this tree compiles finds the
 * mult-free phantom ceiling is THREE (SetDrawEnv, SetDrawEnv2, func_80041AC8),
 * with only the two mult/div bodies at 6 - four phantoms on a mult-free body is
 * attested nowhere. Two further spellings measured inert (unsigned /4*4 for the
 * power-of-two scales is codegen-identical: GCC folds it at expand). Full record:
 * evidence.md / hypotheses.md s8. */
/* s7 (solver, 2026-09-02, chassis HEAD 7e18adc2) - BODY UNCHANGED, floor re-measured
 * 20 (74/74). goal_from_tgt.py classify returns PRE-RA, 'next tool: none - the residual
 * is upstream of every model': registers and instruction order already match exactly,
 * so neither ra_solver nor sched_solver has any purchase here. Three further class-A
 * spellings measured negative (s16* direct loads, four u16* copy temps, five copy
 * temps). The target's own narrow loads are lhu+sll+sra, NOT lh, so the HImode
 * sign-extend fold that makes class-A orphans elsewhere in this tree cannot fire on
 * this body without changing bytes. Full record: hypotheses.md / evidence.md s7. */
/* func_800480C0 - s3 (structural, 2026-09-02) BEST POLICY-LEGAL FORM.
 * sandbox --disable all = 20 (74/74 insns); .frame sp,56 (vars=0, regs=8,
 * args=24) vs target .frame sp,88 (vars=32, regs=8, args=24).
 *
 * TWO s2 LEDGER CORRECTIONS, both measured this session:
 *
 * 1. THE HONEST FLOOR IS 20, NOT 32. engine/volatile_cheats.py's
 *    find_dead_param_assigns strips an UN-ANNOTATED `arg0 = 0;` out of the
 *    scored TU (engine/test_engine.py:1094-1116 pins exactly this: an
 *    annotated dead param assign is bypassed, an un-annotated one is
 *    flagged/stripped). Every s1/s2 body carried the store WITHOUT a
 *    FAKE annotation comment, so the sandbox scored a body with the
 *    store deleted. Adding the annotation - which the dead-store family
 *    REQUIRES anyway (.claude/rules/dead-store-fake-exception.md) - moves the
 *    score 32 -> 20 with no other change (measured s3).
 *
 * 2. THERE IS NO CALLEE-SAVED SEAT ROTATION. s2 recorded base_addr in s6
 *    against the target's s2 and built a whole frontier (ra_solver /
 *    sched_solver) on it. That rotation only exists in the STRIPPED sandbox
 *    build, i.e. it is the downstream effect of correction 1. With the store
 *    honoured, cc1 emits `s2 = s0` (s2 = s0) and seats
 *    sx_arg2..5 in s6/s5/s4/s3 - identical to the target
 *    (tmp/grind/func_800480C0/s3/probe.s, BB2_ALLOC_DEBUG order in
 *    tmp/grind/func_800480C0/s3/alloc.err: pseudo 83 -> hardreg 18).
 *
 * RESIDUAL: purely the frame size. The instruction STREAM is byte-identical
 * to the split target listing for func_800480C0; the 20 differing insns are the 20 sp-relative
 * operands (1 addiu sp, 8 sw, 2 incoming stack-arg lw, 8 lw, 1 addiu sp).
 * The target reserves 32 bytes of `vars` at sp+0x18..0x37 that no instruction
 * touches. args+vars = 0x38 is all the binary fixes, so (args=24, vars=32) and
 * (args=56, vars=0) are indistinguishable layouts - but args=56 needs a
 * 14-word call, i.e. the fabricated-dead-call-site family REFUSED by owner
 * ruling 2026-08-17 (.claude/rules/no-new-park-categories.md:357-366).
 *
 * The pad remains banned here (Judge 2026-09-02 04:28,
 * docs/grind/decisions.md:20349); the bytes-proven pad body is preserved at
 * rejected/pad-judge-banned-2026-09-02.c. */
/* s5 (synthesis, 2026-09-02) - BODY UNCHANGED, floor re-measured 20 (74/74).
 * Two ledger corrections this session, both tree-wide measurements:
 *  1. ONE UNALLOCATED PSEUDO == 8 BYTES OF `vars`, NOT 4. s3 modelled the
 *     ST_REGS compare residue as a 4-byte alter_reg slot rounded up to 8.
 *     Correlating cc1's `# vars=` with the instrumented cc1's BB2_ALLOC_DEBUG
 *     hardreg=-1 count across all 32 TUs gives `vars = 8 * phantoms` exactly
 *     (func_80042874 6->48, func_80041E10 3->24, get_cs 2->16, ~30 at 1->8).
 *     So the target's 32 untouched bytes are FOUR phantoms, and every spelling
 *     in the 30-form ceiling supplies ONE of them.
 *  2. THE PAD IS NOT THE ONLY PRODUCER OF UNTOUCHED `vars`. 54 ordinary-C
 *     COMPLETED-C bodies on main reserve untouched frame bytes with no pad
 *     aggregate and no spill, and eight reach 16 bytes (two phantoms) -
 *     get_cs/get_ce (src/display.c:556), func_8003FECC, func_80038170,
 *     func_80040594, SsSeqCalledTbyT, _SsSeqPlay, SpuSetCommonAttr. Same
 *     ST_REGS residue class (display.lreg: Registers 85 and 99, blocks 1 and 6).
 *     What is missing here is multiplicity, not mechanism.
 *  Bound: among the 45 census candidates with no mult/div, untouched `vars`
 *  takes only {8, 16, 32} and every 32 is a sanctioned-pad function whose
 *  phantom count is ZERO (the 32 bytes are the declared array). Four
 *  folded-compare residues on a mult-free body has no precedent in this tree.
 *  Instruments: tmp/grind/func_800480C0/s5/{census.sh,census2.py,census3.py,
 *  alloc_tu.sh,dump2.sh}. */
/* s6 (synthesis, 2026-09-02, chassis HEAD ba593529) - BODY UNCHANGED, floor
 * re-measured 20 (74/74). Two ledger corrections, both dump-verified:
 *  1. THE PHANTOM PRODUCER HAS A NAME. s3/s5 called it "an ST_REGS-classed
 *     compare residue". It is an orphan `(insn (use (reg P)))` planted by
 *     combine.c's distribute_notes (tools/gcc-2.7.2/combine.c:10832-10841):
 *     combine rewrites/deletes the insn that DEFINED intermediate pseudo P,
 *     P's REG_DEAD note finds no home, the backward scan (combine.c:10757-10762)
 *     stops at the block's leading jump/label, and the USE is planted there.
 *     P then has no set and no constraint-bearing reference, so regclass leaves
 *     its printed class at the default `ST_REGS or none`, find_reg cannot seat
 *     it, and alter_reg pays 8 bytes of vars for zero emitted instructions.
 *     New instrument tmp/grind/func_800480C0/s6/count_uses.py counts these per
 *     function in a .combine dump; over six TUs it predicts the BB2_ALLOC_DEBUG
 *     phantom count exactly (SetDrawEnv 3/3, get_cs 2/2, ... ), the only
 *     disagreements being the mult/div DImode-HILO producer.
 *  2. s5's TREE-WIDE BOUND OF TWO IS REFUTED. SetDrawEnv (src/display.c:360)
 *     and SetDrawEnv2 (src/display.c:436) are mult-free ordinary C, COMPLETED-C
 *     on main, and each carry THREE orphan USEs / THREE phantoms:
 *     tmp/grind/func_800480C0/s5/asm/display.s:1243 reads
 *     `.frame $sp,64 # vars= 32, regs= 3/0, args= 16` (24 phantom + 8 for the
 *     live u16 buf[4]). s5's bound was an artifact of its census filter, which
 *     drops any function that also owns live stack traffic.
 *  AND THE REFRAME THAT MATTERS: this body's own phantom is the OTHER class.
 *  rejected/phantom-guard-vars8-ceiling.c and
 *  rejected/two-branch-guards-still-one-phantom.c both re-measure unalloc=1 with
 *  orphanUSE=0 on the current chassis, so the 30-form ceiling of s2-s5 bounds
 *  class B (non-combine) only; the combine-orphan producer that reaches 3 in
 *  this tree has never fired on func_800480C0 at all.
 *  Four new spellings measured class-A-negative (vars=0/unalloc=0/orphanUSE=0),
 *  banked in rejected/: s32-typed params with explicit (s16) casts, block-scope
 *  s16 intermediates, function-scope s16 intermediates, and a split shift
 *  ((word>>1)>>1)<<2 at the loop head.
 *  Instruments: tmp/grind/func_800480C0/s6/{probe.sh,runall.sh,dumpall.sh,
 *  count_uses.py}. probe.sh restores src/text1b.c from HEAD around every
 *  measurement, which the s3 probe did not. */
void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0; /* FAKE: breaks the $a0 == base_addr value association so cse2 cannot canonicalise the two base copies, mechanism: cse.c canonical-register substitution, lever-exhaustion: hypotheses.md s1-s3 */
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(base_addr + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        s32 sx_arg4;
        s32 sx_arg5;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        sx_arg4 = arg4;
        sx_arg5 = arg5;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = base_addr + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            func_800482C8(new_var,
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg4,
                          (s32)v0v + sx_arg5);
        } while ((count--) != 0);
    }
}
