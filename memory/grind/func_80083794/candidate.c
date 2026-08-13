/* func_80083794 — best honest pure-C form. Session 1 found it; session 2
 * re-measured it and swept 18 structural variants around it without moving it.
 * Honest sandbox floor: 18 (target_insns 28, build_insns 28).
 *
 * THIS FILE IS THE AUTHORITATIVE CARRIER OF THE FLOOR. At the start of session 2
 * `src/ings2.c` still held the old register-pin + hardcoded-`$17` `__asm__` body
 * (only s1's LEDGER was committed, in b26eadc6, not its src edit). Re-apply this
 * body to src/ings2.c before measuring anything.
 *
 * Identity: this is GCC's `__main` — the only caller is main() (src/ings.c:589,
 * first statement), and the body is libgcc2.c's `__main` with
 * `__do_global_ctors` inlined:
 *     static int initialized;
 *     if (!initialized) { initialized = 1; <walk ctor table, call each>; }
 * D_800A2668 is the `initialized` flag, D_8008D070 is the ctor table base,
 * D_00000000 is a link-time absolute symbol whose VALUE (not contents) is the
 * ctor count — hence the `la` (lui/addiu) rather than a load.
 *
 * The residual is instruction-for-instruction aligned with target (same 28 insns
 * in the same order) and splits into exactly three classes, two of which are now
 * PROVEN unreachable from any C source under the frozen pipeline:
 *
 *   A frame geometry, ~8 insns — we emit the mandatory 16-byte o32
 *     outgoing-argument block (calls.c:1246-1252 + mips.h:1822/1830 +
 *     mips.c:4464/4474: >= 16 bytes for ANY function that expands a call, with
 *     the MAYBE_REG_PARM_STACK_SPACE zeroing escape dead because that macro is
 *     undefined for MIPS). Target's whole frame is 16 bytes and already holds 12
 *     bytes of saves, so it does not contain the block. Corpus: 1/1437 calling
 *     functions lack it — this one.  [PROVEN UNREACHABLE]
 *
 *   B register roles, ~5 insns — target has $s0=p, $s1=count, temp $t0; we get
 *     $s0=count, $s1=p, temp $v0. cc1 -da: pseudo 72 (count) has 8 loop-weighted
 *     refs, pseudo 73 (p) has 7, and global.c:635 allocno_compare ranks
 *     floor_log2(n)*n/live_length, i.e. 24 vs 14. 18 semantics-preserving
 *     structural forms all leave it unchanged.  [STRUCTURAL AXIS DEAD]
 *
 *   C `ori $t0,$zero,1` vs our `li`, 1 insn — GNU as expands `li` to `ori` only
 *     for immediates needing zero extension (0x8000..0xFFFF) and to `addiu`
 *     otherwise; 1 takes the addiu path. All 3 small-immediate `ori $rX,$zero,imm`
 *     instances in the executable are hand-written asm.  [PROVEN UNREACHABLE]
 *
 *   D prologue callee-save EMISSION ORDER, ~3 insns (isolated by session 3's
 *     exact 28-instruction accounting; s1/s2 folded it into A) — target stores
 *     ASCENDING (s0@4, s1@8, ra@0xC) while its own epilogue loads DESCENDING.
 *     mips.c:4680 is ONE loop (GP_REG_LAST -> GP_REG_FIRST) serving both the
 *     prologue and the epilogue, so cc1 emits both descending; only the
 *     post-reload scheduler can reverse a run, and it does so only under an
 *     in-block anti-dependence (a body insn writing $sN). Measured: hoisting the
 *     two `la` pairs into the entry block DOES produce the ascending order, and
 *     costs 5 points (18 -> 23), because target emits those `la`s AFTER the
 *     bnez.  [C-REACHABLE IN ISOLATION, JOINTLY UNREACHABLE — see
 *     rejected/hoisted-la-flips-save-order-but-costs-5.c]
 *
 * SESSION 4 (permuter) re-measured this exact body at 18 and then ran 107,064
 * decomp-permuter iterations from two chassis (this floor form, and the s3
 * hoisted-`la` class-D form) without ever going below it. The three closing-form
 * families the search produced are banked in
 * rejected/permuter-s4-proposals-cheat-or-byte-neutral.c: a dead-`volatile`-local
 * cheat that is measurably WORSE honestly (29 insns vs 28), a `do {...} while (0)`
 * wrap measured byte-identical to this body (score 18), and staged-value
 * respellings that tie the base. This body remains the floor carrier.
 *
 * SESSION 5 (permuter) built a NORMALIZED-TARGET workspace — the shipped target
 * with ONLY the proven-unreachable classes A and C rewritten to the form our
 * pipeline can emit — so the search objective was exactly classes B + D with the
 * constant noise removed (tmp/grind/func_80083794/s5/mkws3.py; that target.o is
 * a MEASUREMENT INSTRUMENT, a 0 against it is NOT a byte match). Two more
 * campaigns (95,501 + 63,304 iterations) still never beat this body honestly.
 * The one new lead — ws4/output-478-1, the first form in five sessions to put
 * `p` in $s0 — flips the roles only by testing one variable while decrementing
 * another, i.e. by deleting a semantically required decrement (infinite loop).
 * All six semantics-preserving spellings of that ref-split measure 18-24 with
 * `p` in $s1 (rejected/count-ref-split-forms-do-not-flip-s0-s1.c).
 * Session 5 also closes the search modality by arithmetic: eight of target's 28
 * instructions reference the frame and one is the `ori`, so NO pure-C form can
 * score below 9 — distance 0 is unreachable regardless of iteration count, and
 * the most any search can buy is classes B + D.
 *
 * SESSION 6 (forensics) added a FIFTH residual class and closed the external-
 * corroboration probe. Class E — DELAY-SLOT PROVENANCE: target's `bnez $t0`
 * delay slot holds `ori $t0,$zero,1`, an insn that CLOBBERS the register the
 * branch tests, so it can only have come from the conditional arm via reorg.c's
 * fall-through scan (reorg.c:3075) — which reorg.c:3048 reaches ONLY when the
 * backward scan (reorg.c:2960) finds nothing, and target has three eligible
 * callee-saves immediately ahead of the branch. Instrumented cc1
 * (BB2_DBR_DEBUG=1) logs `DBRDBG simp insn=11 trial=73 ... elig=1`: our build
 * consumes the nearest preceding save (`sw $16,16($sp)`) into that slot, every
 * time. Corpus: the 39 shipped functions that DO keep an intact >=3 save run
 * with a non-save filler all use an entry-block computation as the filler, never
 * an arm insn clobbering the tested register.  [PROVEN UNREACHABLE, and
 * INDEPENDENT of class D — fixing the save order would not stop the theft.]
 * Session 6 also confirmed class D at pass level from the RTL dumps (the saves
 * are absent from .greg, appear in .jump2 as insns 69/71/73 descending, survive
 * .sched2 unchanged, and .dbr steals insn 73), KILLED the "unfilled return delay
 * slot is a hand-asm fingerprint" idea (838 of 1434 shipped functions have
 * exactly that shape), and KILLED the external-corroboration probe: the second,
 * independently linked PsyQ executable disc/STR/MOVOVL.EXE contains no __main,
 * no ctor-walk loop and no leaf-frame save signature at all.
 *
 * SESSION 7 (forensics) closed the one counter-explanation that six sessions of
 * mechanism arguments had left formally open — that all five classes are
 * decompals-fork-vs-cc1psx DIVERGENCE rather than evidence about provenance.
 * The ORIGINAL PsyQ compiler (tools/cc1psx_wrapper.sh -> cc1psx.exe, banner
 * "GNU C 2.7.2.SN.1 [AL 1.1, MM 40] Sony Playstation") was fed THIS EXACT BODY
 * and reproduces all five classes as our fork does: `.frame $sp,32,$31 #
 * args= 16` (A), $16=count / $17=p / temp $2 (B), `li $2,0x00000001` not `ori`
 * (C), saves descending sw $31,24 / sw $17,20 / sw $16,16 (D), and the branch
 * delay slot filled with the nearest preceding SAVE (E). `args= 16` holds at
 * -O0/-O1/-O2/-O3 on both compilers. cc1psx is DIAGNOSTIC-ONLY here
 * ([[cc1psx-calibration-only]]); no build change is implied.
 * Session 7 also killed the last alternative reading of class C — that ASPSX
 * 2.34 (not GNU as) expanded small `li` to `ori` — by census: 5537
 * `addiu $rX,$zero,imm<0x8000` against 3 `ori` in the shipped executable, two of
 * the three inside /* handwritten instruction */-annotated GTE bodies. And it
 * added a WITHIN-REGION control experiment for class E: the twin at 0x80083804
 * has the same branch shape behind the same three eligible saves and leaves its
 * delay slot EMPTY, while both compilers fill it with `sw $16,16($sp)`.
 *
 * SESSION 8 (rederive) spent the modality's own two probes and then brought in
 * the first evidence from OUTSIDE this repository. m2c reconstructs THIS BODY
 * verbatim from the target (the 28 instructions admit exactly one dataflow), and
 * the 3754-scratch decomp.me corpus contains no matched `__main`/ctor-walk in
 * any PS1 project, so there is no alternative shape and no transplant source.
 * Using the corpus's 1740 already-matched GCC-2.7.2/PsyQ functions as a control
 * group: class A has 0 counterexamples in 1246 matched CALLING functions (1019
 * put their lowest callee-save at exactly +16, none below it — target puts $s0
 * at +4), and class E has 0 counterexamples in 484 (the shape itself is
 * ordinary, but 480 of the 484 have NO adjacent preceding callee-save and none
 * has 2+; target has 3). Class D is confirmed C-reachable (33 matched ascending
 * runs), which is s3's result, so the operative constraint there stays s3's
 * joint-unreachability, not the emission order.
 * Session 8 also REVISED class C's meaning: 591 matched corpus scratches contain
 * `ori $rX,$zero,small` (84 of them in BB2's addiu-flavoured `la` regime), cc1
 * never emits `ori` for a constant (mips.md:1899-1908 iorsi3 needs a register;
 * mips.c mips_move_1word emits `li %0,%X1`), and our maspsx decides it in
 * expand_load_immediate (addiu for 0<imm<=0x7FFF, ori for 0x8000-0xFFFF) with
 * the in-source note "ori is actually addiu on ASPSX 2.56+". So the spelling is
 * an ASPSX-VERSION property: class C stays unreachable for us, but it is a
 * FOREIGN-ASSEMBLER fingerprint, not a hand-written-asm one. All five classes
 * then collapse into one claim — these bytes were linked in from a prebuilt
 * PsyQ/SN object (crt0/libgcc `__main`), not compiled from this project's C.
 *
 * Consequence: honest distance 0 is not reachable in pure C for this function.
 * See memory/grind/func_80083794/hypotheses.md §"Live frontier after session 2".
 *
 * NOTE: the 9 regfix rules for this function (regfix.txt:105-113) were calibrated
 * against the old register-pin form, so the integrated build is expected to
 * disagree until they are retired. The honest sandbox floor is the gradient.
 */
extern s32 D_800A2668;
extern void (*D_8008D070)(void);
extern s32 D_00000000;

void func_80083794(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 == 0) {
        D_800A2668 = 1;
        count = (s32)&D_00000000;
        p = &D_8008D070;
        while (count != 0) {
            (*p++)();
            count--;
        }
    }
}
