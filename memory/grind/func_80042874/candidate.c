/* candidate.c — func_80042874 (src/text1a_c.c), grind session 1 (recon).
 *
 * STATUS: honest pure-C distance 0.  `sandbox func_80042874 --disable all`
 *         => score 0, build_insns 119 == target_insns 119, rules_dropped 10,
 *         and a raw-byte comparison of the sandbox object's 532-byte function
 *         region against asm/funcs/func_80042874.s shows ZERO non-relocation
 *         word mismatches (28 reloc words in range, all %hi/%lo(Judge)).
 *         Script: tmp/grind/func_80042874/s1/bytecmp.py.
 *
 * CHEAT-FREE: zero regfix/asmfix rules needed, zero register pins, zero
 * volatile, zero inline asm.  The HEAD form reached 21 only while carrying
 * BOTH a `register s32 angC asm("$3")` pin and a
 * `(s16)*(volatile u16 *)(&Judge[...])` coercion — the sandbox strips both,
 * so they were never buying anything; this form drops them outright.
 *
 * INTEGRATION NOTE for the operator: regfix.txt lines 888-903 still carry 10
 * rules for func_80042874 (mflo/sra/mult substs, a delete, two reorders, the
 * la->lui %hi(Judge) rewrite and an insert_after "nop").  They were written
 * against the OLD codegen and must be retired (`engine retire func_80042874`)
 * for the full build to match.  This session may not touch regfix.txt.
 *
 * ---------------------------------------------------------------- HOW IT WAS FOUND
 * The decisive observation is that func_80042A88, the function immediately
 * below this one in the same file, is the SAME rotation-matrix builder with a
 * different element order, and it is already COMPLETED-C (off the queue,
 * `sandbox --disable all` = 0, zero rules; match commit 11ecbfa8).  Its
 * accepted body is the template.  Three of its structural decisions transfer
 * verbatim, and each is load-bearing here:
 *
 *   1. cosA is read through a `u16 rawA` staging local and sign-extended with
 *      an explicit (s16) cast, with the a1[] store of sinA placed BETWEEN the
 *      load and the cast.  Mechanism (as recorded by the A88 sessions):
 *      combine's can_combine_p refuses to combine a MEM load into a later
 *      user across an insn that may WRITE memory, so simplify_shift_const
 *      never sees the ashiftrt(ashift(zero_extend(mem),16),16) chain it would
 *      otherwise fold into a single sign_extend (`lh`).  Target's three-insn
 *      shape survives: lui at,%hi(Judge); addu at,at,a3; lhu v1,%lo(Judge)(at)
 *      then sll v1,16 / sra v1,16 (target insns 57-63).  sched1 runs after
 *      combine and hoists the sh back out to its target position (insn 106),
 *      so the store costs nothing.
 *   2. `angC = a0[2]; sinC = Judge[angC & 0xFFF];` sits AFTER the combined
 *      `negSinAxsinB_12 = (sinA * -sinB) >> 12;` statement.  a0[2] is the last
 *      use of the parameter pointer, so its position decides where $a0 dies;
 *      read there, local-alloc hands angC target's $v1 and the cos-index temp
 *      target's $v0 (target insn 19: lhu v1,0x4(a0)).
 *   3. The cosB index is hoisted into a named intermediate
 *      `idxB = (s16)angB + 0x400;` declared/assigned right after sinB.
 *
 * Measured ladder this session: 21 (HEAD, pin+volatile stripped)
 *   -> 14  after (1) alone plus a SPLIT mult/shift for negSinAxsinB_12
 *          (residual: angC/idxC mirror-swapped $v0<->$v1, sra scheduled
 *          early into $a0, and one surplus nop => 120 insns)
 *   -> 0   after replacing the split with A88's COMBINED `(sinA * -sinB) >> 12`
 *          plus the hoisted idxB.  The split-vs-combined choice is exactly the
 *          fixpoint the A88 ledger described as unresolved at score 12-13;
 *          the idxB hoist is what lets the combined form keep BOTH halves.
 */
extern s16 Judge[];
void func_80042874(u16 *a0, s16 *a1) {
    s32 angA, angB;
    s16 sinA, sinB, sinC;
    s16 cosB, cosC;
    s32 negSinAxsinB_12;
    s32 prod_sinC, cosB_cosC;
    s32 sinAxcosB;
    s32 sinAxcosB_12;
    s32 cosA_negSinC;
    s32 prod2_sinC, sinB_cosC;
    s32 sinAxsinB;
    s32 sinAxsinB_12;
    s32 sinAxsinB_12_cosC;
    s32 cosB_sinC;
    s32 negSinA_cosB;
    s32 cosA_cosC;
    s32 negSinAxcosB_12;
    s32 negSinAxcosB_12_cosC;
    s32 cosA_negSinB;
    s32 cosA_cosB;
    s32 sinB_sinC;
    s32 cosA;
    s32 angC;
    s32 idxB;
    u16 rawA;

    angA = a0[0];
    angB = a0[1];

    sinA = Judge[angA & 0xFFF];
    sinB = Judge[angB & 0xFFF];

    idxB = (s16)angB + 0x400;

    negSinAxsinB_12 = (sinA * -sinB) >> 12;

    angC = a0[2];
    sinC = Judge[angC & 0xFFF];

    prod_sinC = negSinAxsinB_12 * sinC;

    cosB = Judge[idxB & 0xFFF];
    cosC = Judge[((s16)angC + 0x400) & 0xFFF];

    cosB_cosC = cosB * cosC;

    sinAxcosB = sinA * cosB;

    rawA = Judge[((s16)angA + 0x400) & 0xFFF];
    a1[7] = sinA;
    cosA = (s16)rawA;

    cosA_negSinC = cosA * -sinC;

    sinAxcosB_12 = sinAxcosB >> 12;
    prod2_sinC = sinAxcosB_12 * sinC;

    sinAxsinB = sinA * sinB;
    sinB_cosC = sinB * cosC;

    sinAxsinB_12 = sinAxsinB >> 12;
    sinAxsinB_12_cosC = sinAxsinB_12 * cosC;

    cosB_sinC = cosB * sinC;

    negSinA_cosB = -sinA * cosB;

    cosA_cosC = cosA * cosC;

    negSinAxcosB_12 = negSinA_cosB >> 12;
    negSinAxcosB_12_cosC = negSinAxcosB_12 * cosC;

    cosA_negSinB = cosA * -sinB;

    cosA_cosB = cosA * cosB;

    a1[1] = cosA_negSinC >> 12;
    a1[4] = cosA_cosC >> 12;
    a1[6] = cosA_negSinB >> 12;
    a1[8] = cosA_cosB >> 12;
    a1[0] = (prod_sinC + cosB_cosC) >> 12;
    a1[2] = (prod2_sinC + sinB_cosC) >> 12;
    a1[3] = (sinAxsinB_12_cosC + cosB_sinC) >> 12;

    sinB_sinC = sinB * sinC;
    a1[5] = (negSinAxcosB_12_cosC + sinB_sinC) >> 12;
}
