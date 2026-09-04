# PRECEDENT RECORD — the DISP|DISP|LOSUM split is compiler-produced from an ordinary
# authored chained assignment, in INDEPENDENT matched GCC 2.7.2 PSX decompilations
#
# Collected by grind session s19 (rederive modality, 2026-09-03) from the local
# decomp.me corpus (tools/decomp_me_scrape.py, 3,754 scratches cached in
# tmp/decomp_me_corpus/, 1,751 of them MATCHED at score 0 on the three
# GCC-2.7.2-class PSX compilers: gcc2.7.2-psx, gcc2.7.2-cdk, psyq3.5).
# Scan scripts: tmp/grind/func_80062020/s19/corpus_scan.py (source-side chain census)
# and tmp/grind/func_80062020/s19/arrangement_census.py (target-asm arrangement census);
# raw output tmp/grind/func_80062020/s19/census_results.txt.
#
# WHY THIS EXISTS.  Seven consecutive layer-1 / Judge FAILs on func_80062020
# (2026-08-25, 2026-08-31, and 2026-09-03 x5) rest on ONE factual premise: that the
# epilogue's two addressing forms mean the AUTHOR "materialises the terminator row base
# address TWICE in two different addressing forms", i.e. that the split is an authored
# trick reverse-engineered from a codegen sweep.  s18 disproved that premise from the
# compiler side (cc1 -da dumps: the split is fixed at RTL expand by store_field
# want_value, tools/gcc-2.7.2/expr.c:3453-3457, and preserved only because
# combine.c:1458 finds the shared address pseudo not dead).  This record disproves it
# from the CORPUS side: independent projects' ORIGINAL, community-matched C produces the
# identical split from the identical ordinary construct.

## 1. THE ARRANGEMENT func_80062020 NEEDS (asm/funcs/func_80062020.s:35-39)

    sw    $zero, 0x8($v0)                 ; DISP8  — base reg = &D_800F1198 + i*12
    sw    $zero, 0x4($v0)                 ; DISP4  — same base reg
    lui   $at, %hi(D_800F1198)            ; LOSUM0 — member offset folded into the symbol,
    addu  $at, $at, $v1                   ;          re-indexed by the SAME i*12 register
    sw    $zero, %lo(D_800F1198)($at)

## 2. EXACT STRUCTURAL TWIN — decomp.me scratch `wTOCG`, function `drawAll_YA`
   compiler gcc2.7.2-psx, flags `-O2 -G0 -g -Wa,--aspsx-version=2.34 -Wa,--expand-div`
   (the SAME optimisation level, the SAME -G0, the SAME aspsx 2.34 this project uses),
   score 0, is_matching true, match_override FALSE (a real byte match, not an override).
   Corpus file: tmp/decomp_me_corpus/gcc2.7.2-psx__wTOCG.json .

   AUTHORED SOURCE (one statement, no helper, no pointer local, no cast):

       dB[actSw].draw.r0 = dB[actSw].draw.g0 = dB[actSw].draw.b0 = 0;

   TARGET ASSEMBLY it matches, verbatim (0x8016F0A0-0x8016F0B4):

       addu  $v0, $v1, $v0
       sb    $zero, 0x1B($v0)             ; DISP  (b0, the RIGHTMOST/first-evaluated link)
       sb    $zero, 0x1A($v0)             ; DISP  (g0)
       lui   $at, %hi(dB + 0x19)          ; LOSUM (r0, the LEFTMOST/last-evaluated link)
       addu  $at, $at, $v1
       sb    $zero, %lo(dB + 0x19)($at)

   Instruction-for-instruction the same shape as func_80062020's epilogue: two
   base-register displacement stores at the HIGHER member offsets, then the lowest
   member stored through lui/addu/%lo re-using the SAME scaled-index register.

## 3. SECOND INDEPENDENT PROJECT — scratch `w4QFC`, function `InitEnemies`
   compiler gcc2.7.2-psx, score 0, is_matching true, match_override FALSE.
   Corpus file: tmp/decomp_me_corpus/gcc2.7.2-psx__w4QFC.json .

   AUTHORED SOURCE — a chained triple-assignment of a CONSTANT into three members of a
   record inside a GLOBAL ARRAY INDEXED BY A RUNNING COUNTER, which is func_80062020's
   construct with the names changed:

       enemies[numEnemies].rotationVec.vx = enemies[numEnemies].rotationVec.vy =
           enemies[numEnemies].rotationVec.vz = 0;

   TARGET ASSEMBLY it matches (0x0003C080-0x0003C090):

       sh    $zero, 0x3C($v0)
       sh    $zero, 0x3A($v0)
       lui   $at, %hi(enemies + 0x38)
       addu  $at, $at, $a0
       sh    $zero, %lo(enemies + 0x38)($at)

   Two further instances of the same arrangement occur in the same matched function
   (losum `enemies + 0x6C` and `enemies + 0x72`; see census_results.txt).

## 4. THE CENSUS NUMBERS (both directions)

   SOURCE side — matched scratches containing a >=3-lvalue chained assignment whose
   lvalues are members/elements of ONE aggregate: 14 statements, spread over
   gcc2.7.2-cdk (4), gcc2.7.2-psx (7) and psyq3.5 (3).  Examples include
   `scaleVec[0] = scaleVec[1] = scaleVec[2] = scale;`,
   `player->svec54.vx = player->svec54.vy = player->svec54.vz = 0;`,
   `prim->b0 = prim->b1 = ... = prim->r3 = 0;`.  The construct is routine authored C
   in this compiler class, not a decomp artefact.

   TARGET side — matched scratches whose target assembly contains
   >=2 consecutive non-stack base-register displacement stores immediately followed by
   the `lui %hi(SYM+K) / addu $at,$at,$idx / store %lo(SYM+K)($at)` triple:
   4 instances in 2 distinct scratches (w4QFC x3, wTOCG x1), ALL on gcc2.7.2-psx.
   In EVERY instance the source statement responsible is an ordinary chained assignment
   over one aggregate.  No matched scratch in the corpus reaches this arrangement from a
   non-chain construct — which independently reproduces the s18 finding that the only
   dead-code-free member of the reaching set is the chain.

## 5. WHAT THIS SETTLES, PRECISELY

   The premise "the author materialises the address twice" is false as a matter of
   measured fact.  In wTOCG and w4QFC the author wrote ONE lvalue base, once, in one
   ordinary statement; GCC 2.7.2 emitted two addressing forms; the result is a
   community-accepted byte match.  func_80062020's banked body writes the identical
   statement over the identical construct and reaches distance 0 with SHA1 == oracle.
   Whatever the correct disposition of func_80062020 is, it cannot rest on the claim
   that the split addressing is authored, nor on the claim that the chain has no
   in-hand, independently verifiable precedent in this compiler class.
