/* func_8001F938 — REJECTED (s3, structural). Pure UNSIGNED single-read of +0x270.
 * sandbox --disable all: floor 6, build_insns 106 (target 107). BEATS clean floor 8.
 *
 * THE +0x270 BLOCK MEASURED (rest of function = clean floor-8 form):
 *     u32 probe = *((u16 *)(arg0 + 0x270));      // SINGLE unsigned read, ONE deref
 *     s32 raw_or_3;
 *     if (probe >= 4U) { raw_or_3 = 3; } else { raw_or_3 = probe; }
 *     idx = ((raw_or_3 << 16) >> 15);
 *
 * WHY IT SCORES 6 (disasm tmp/grind/func_8001F938/s3/unsigned_read_disasm.txt):
 *   395c: lhu   v0, 0x270(a0)   -- ONE load (target has TWO: lh + lhu)
 *   3968: sltiu v0, v1, 4       -- UNSIGNED compare (target has slti, SIGNED)
 *   3970: sll   v0, v1, 0x10    -- FOLD DEFEATED: matches target's sll<<16 ...
 *   397c: sra   v0, v0, 0xf     -- ... and sra>>15. The (raw<<16)>>15 idiom is
 *                                  NOT folded to raw*2, unlike the clean s16 form.
 *
 * SIGNIFICANCE (this is a NEW result, not a re-spelling of a dead axis):
 *   - CORRECTS the s2 ledger over-claim "NO single-typed read defeats the fold;
 *     only a second TYPED memory view works" (evidence.md s2 P2b + s2c partial
 *     correction). FALSE. A plain UNSIGNED single read (u16, no cast, no dual
 *     view, no split, no branch-PHI) DEFEATS the fold outright -> floor 6. The
 *     mechanism: GCC cannot prove a u16-typed value (range [0,65535], bit15 may
 *     be set) fits SIGNED-16, so (x<<16)>>15 != x<<1 for GCC -> it keeps the
 *     unfolded shift pair. (The signed s16 form folds because s16 provably fits.)
 *   - TIGHTENS THE DICHOTOMY from a fresh angle: the unsigned read buys the
 *     unfolded index shape but FORFEITS the signed compare (sltiu, not target's
 *     slti). Target needs BOTH a signed compare (slti/lh) AND an unsigned-opaque
 *     index (lhu, unfolded). One typed read delivers exactly one of the two;
 *     getting both simultaneously requires two typed views of +0x270 = the
 *     pre-banned signedness-split family. Confirmed: single-typed reads give
 *     {signed=fold=floor 8} XOR {unsigned=unfold-but-sltiu=floor 6}; neither
 *     byte-matches, and the ONLY sub-6 forms are the banned dual views.
 *
 * WHY NOT A CANDIDATE:
 *   - Semantically DIFFERENT from target: target compares the field SIGNED
 *     (slti); this compares it UNSIGNED (sltiu). For a field value >= 0x8000
 *     the two branch differently (signed<0<4 -> use field; unsigned>=4 -> use 3).
 *     A byte-match MUST reproduce slti, i.e. a signed compare operand = a signed
 *     view of the field. Floor 6 has both the missing 2nd load AND the wrong
 *     compare; it cannot reach 0 without adding the signed view = dual read.
 *   - Strictly DOMINATED by the known distance-0 signed-cast-single-read.c
 *     (floor 0, pre-banned family) and does not open any clean path.
 *
 * VERDICT: KILLED. Eliminates the "unsigned single-read" branch of the search
 * space and re-proves the +0x270 signed-compare-vs-unsigned-index dichotomy.
 * src/ kept at the clean floor-8 form (candidate.c).
 */
