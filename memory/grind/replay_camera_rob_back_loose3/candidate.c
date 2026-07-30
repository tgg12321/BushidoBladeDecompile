/* candidate.c - replay_camera_rob_back_loose3, session 4 (permuter).
 *
 * ***  MATCH.  engine `sandbox --disable all` = 0, build_insns 114 ==
 * ***  target_insns 114, rules_dropped 8.  CHEAT-FREE: zero regfix/asmfix
 * ***  rules, zero register pins, zero volatile, zero inline asm, zero dead
 * ***  code.  (HEAD's committed form scores 17 only via a register pin + a
 * ***  volatile coercion; s1's honest cheat-free baseline was 26.)
 *
 * ------------------------------------------------------------------ HISTORY
 * s2 (26 -> 13):  cosA is read through a `u16 rawA` staging local and
 *   sign-extended with an explicit (s16) cast, and `a1[5] = -sinA;` is MOVED
 *   to sit BETWEEN that load and the cast.  GCC 2.7.2 combine's can_combine_p
 *   refuses to combine a MEM load into a later user across an insn that may
 *   WRITE memory, so simplify_shift_const never sees the
 *   ashiftrt(ashift(zero_extend(mem),16),16) chain it would otherwise fold to
 *   `lh`.  Target's split shape (lui at,%hi; addu at,at,a3; lhu %lo; sll 16;
 *   sra 16) therefore survives, at zero instruction cost, because sched1 runs
 *   after combine and hoists the store back out.  BOTH halves are required.
 *
 * s3 (13 -> 12):  `angC = a0[2]; sinC = Judge[angC & 0xFFF];` moved from
 *   BEFORE the `sinAxsinB_12 = (sinA * sinB) >> 12;` statement to AFTER it.
 *   a0[2] is the LAST use of the parameter pointer a0, so its position decides
 *   where hard reg $a0 dies and puts the angC load after the mult in the
 *   pre-allocation stream; local-alloc then hands angC target's $v1 and its
 *   cos-index temp target's $v0.
 *
 * ------------------------------------------------- WHAT s4 ADDED (12 -> 0)
 * THE ONE CHANGE vs s3's candidate: the cosB index ADD is hoisted into a named
 * s32 local computed immediately after `sinB`, i.e. BEFORE the sinA*sinB
 * multiply, while the `& 0xFFF` MASK stays at the use site:
 *
 *     idxB = (s16)angB + 0x400;          <-- new, right after sinB
 *     ...
 *     cosB = Judge[idxB & 0xFFF];        <-- was Judge[((s16)angB + 0x400) & 0xFFF]
 *
 * Mechanism.  The residual after s3 was one value: `sinAxsinB_12`, which target
 * keeps in $v0 while our build was forced onto the leftover $a0, because our
 * mflo/sra were scheduled BEFORE the cosC index chain and therefore overlapped
 * the chain's live range on $v0 (s3 measured this precisely: target's stream is
 * mult @17 / mflo t0 @22 / sra v0,t0,12 @26, ours was mflo v0 @19 / sra a0 @20).
 * Materialising the cosB index early introduces an independent, already-computed
 * quantity into that window: sched1 now has a ready insn to place ahead of the
 * multiply's result, so mflo/sra slide past the cosC index chain into target's
 * positions, the chain's temp dies before sinAxsinB_12 is defined, and
 * local-alloc gives sinAxsinB_12 $v0.  Instruction count is unchanged at 114 --
 * the addiu that computes the index is in target too, it simply moves.
 *
 * Why this is not any of the previously-rejected idxB forms.  Two things
 * distinguish it from rejected/s32-masked-idxB-temp-keeps-sext-loses-registers.c
 * and rejected/narrow-idxB-temp-folds-away-sext-112-insns.c:
 *   (1) only the ADD is in the local; the `& 0xFFF` mask stays at the use site
 *       (folding the mask in as well changes what CSE canonicalises), and
 *   (2) the local is computed EARLY -- immediately after sinB, ahead of the
 *       multiply -- not late, after the sinC read, where the earlier attempts
 *       put it.  Both are load-bearing; s3's late/masked spellings scored 13.
 * The local is s32, matching the natural type of `(s16)angB + 0x400`; a narrowed
 * (u16/s16) carrier folds the sign-extend away and loses four instructions.
 *
 * Provenance: found by a decomp-permuter campaign (label lineswap-mid9,
 * tmp/grind/replay_camera_rob_back_loose3/s4/ws1) as a `new_var` hoist, 3438
 * iterations / 140 s; confirmed against the real build with
 * `sandbox --disable all` = 0.  The permuter also emitted an angA/angB
 * read-interleave alongside it; that half was measured INERT (0 with and
 * without), so the minimal form below keeps s3's declaration/read order.
 *
 * Purpose check: `idxB` is an ordinary named intermediate for a value the
 * function genuinely computes and uses (the cosB table index).  It is live in
 * the emitted code, has no coercion role, contains no dead store, and is
 * exactly the kind of local a human writing the 3x3 rotation-matrix idiom would
 * name.  Nothing here is a pin, a barrier, a volatile view, an alias, or a
 * dead holder.
 */
extern s16 Judge[];
void replay_camera_rob_back_loose3(u16 *a0, s16 *a1) {
    s32 angA, angB;
    s16 sinA, sinB, sinC;
    s16 cosB, cosC;
    s32 idxB;
    s32 sinAxsinB_12;
    s32 prod_sinC, prod_cosC;
    s32 cosB_cosC, cosB_negsinC;
    s32 cosA_sinB, cosA_sinC, cosA_cosC, cosA_cosB;
    s32 sinAxcosB;
    s32 sinAxcosB_12;
    s32 scb_sinC, neg_sinB_cosC;
    s32 scb_cosC;
    s32 sinB_sinC;
    s32 cosA;
    u16 rawA;
    s32 angC;

    angA = a0[0];
    angB = a0[1];

    sinA = Judge[angA & 0xFFF];
    sinB = Judge[angB & 0xFFF];
    idxB = (s16)angB + 0x400;

    sinAxsinB_12 = (sinA * sinB) >> 12;

    angC = a0[2];
    sinC = Judge[angC & 0xFFF];

    prod_sinC = sinAxsinB_12 * sinC;

    cosB = Judge[idxB & 0xFFF];
    cosC = Judge[((s16)angC + 0x400) & 0xFFF];

    cosB_cosC = cosB * cosC;
    prod_cosC = sinAxsinB_12 * cosC;

    rawA = Judge[((s16)angA + 0x400) & 0xFFF];
    a1[5] = -sinA;
    cosA = (s16)rawA;

    cosB_negsinC = cosB * -sinC;

    cosA_sinB = cosA * sinB;
    cosA_sinC = cosA * sinC;

    sinAxcosB = sinA * cosB;
    cosA_cosC = cosA * cosC;

    sinAxcosB_12 = sinAxcosB >> 12;

    scb_sinC = sinAxcosB_12 * sinC;
    neg_sinB_cosC = -sinB * cosC;

    scb_cosC = sinAxcosB_12 * cosC;

    cosA_cosB = cosA * cosB;

    a1[2] = cosA_sinB >> 12;
    a1[3] = cosA_sinC >> 12;
    a1[4] = cosA_cosC >> 12;
    a1[8] = cosA_cosB >> 12;
    a1[0] = (prod_sinC + cosB_cosC) >> 12;
    a1[1] = (prod_cosC + cosB_negsinC) >> 12;
    a1[6] = (scb_sinC + neg_sinB_cosC) >> 12;

    sinB_sinC = sinB * sinC;
    a1[7] = (scb_cosC + sinB_sinC) >> 12;
}
