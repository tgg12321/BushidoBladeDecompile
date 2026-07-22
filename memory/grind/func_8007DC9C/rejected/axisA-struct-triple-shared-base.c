/* REJECTED (s3, structural) — score 11 (worse than floor 9).
 *
 * Hypothesis: reading BF68/BF6C/BF70 through a single struct pointer at
 * &D_8009BF68 gives the base address pseudo >1 use, so combine keeps it
 * materialized (la; lw 0(reg)) as target does for BF68 — the combine.c:1458
 * added_sets_2 multi-use retention mechanism (same as func_8007EDBC).
 *
 * Result: KILLED. build_insns 90 (unchanged), score 11 (was 9).
 * A shared struct base emits ONE materialized base + offset loads
 * (lw 0/4/8($base)), but TARGET reads BF6C and BF70 as SEPARATE folded
 * %lo(D_8009BF6C)/%lo(D_8009BF70) scalar symbols — i.e. the three are
 * INDEPENDENT scalars in the original, not a contiguous struct/array.
 * So no shared-base multi-use lever can materialize ONLY BF68's address
 * while leaving BF6C/BF70 as independent folded loads. Confirms the s2
 * "one-array" negative and closes the shared-base branch of axis A.
 */
struct bf68_triple { s32 a; s32 b; s32 c; };
/* ... in body: */
/* debug_printf(&D_80016044,
 *     ((struct bf68_triple *)&D_8009BF68)->a,
 *     ((struct bf68_triple *)&D_8009BF68)->b,
 *     ((struct bf68_triple *)&D_8009BF68)->c);  -> score 11 */
