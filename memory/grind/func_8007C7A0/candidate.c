/* func_8007C7A0 — BEST KNOWN FORM (s4, 2026-08-08): sandbox --disable all == 12,
 * build_insns 51/51 STREAM-EXACT.
 *
 * This supersedes BOTH prior banked forms:
 *   - old candidate.c        (12, build 50 — non-stream-exact, park insn missing)
 *   - candidate_stream51.c   (15, build 51 — stream-exact)
 * This form is simultaneously stream-exact AND at the overall floor: every one
 * of the 51 instructions matches target in opcode/operand-shape/order; the 12
 * masked-Levenshtein diffs are pure register renames on 5 roles:
 *   carrier(x)     a2 -> a3   (pseudo 76)
 *   xlim-save      v1 -> a2   (pseudo 83)
 *   sxt(arg1)      v1 -> a2   (pseudo 92)
 *   lo             a0 -> v0   (pseudo 79)
 *   const 0xE300   v0 -> a0   (pseudo 81)
 * Roles ALREADY CORRECT in this form (new vs stream51): X-join temp tx -> $v0
 * (the assignment s3's T1 declared unreachable — see below), ylim-save -> $a0.
 *
 * THE TWO LEVERS (both permuter-found s4, both semantically clean, both from
 * the frozen-list "variable reuse for codegen control" family — SOTN-sanctioned;
 * see the s4 evidence.md entry for the full cheat-vet discussion + the open
 * FAKE-annotation question that must be resolved before any candidate-ready):
 *   L1: tx (dead after `x = tx;`) reused as the dispatch discriminant:
 *       `tx = (u32)(D_8009BE74 - 1); ... if (tx >= 2U)`.
 *       Merging the dispatch pseudo into s16 tx changes the RTL pseudo GRAPH
 *       (not just prefs/conflicts of the old graph) -> X-join lands $v0.
 *       Equivalence: D in 0..255 => D-1 in [-1,254], all representable in s16;
 *       (u32)tx >= 2U identical to (u32)(D-1) >= 2U for every value.
 *       s32 holders (pkt) do NOT work: pkt-dispatch = 15, or breaks stream (50).
 *   L2: Y sign check staged through hi (dead until its real def):
 *       `hi = arg1; if (hi >= 0)` -> ylim-save lands $a0. Staging through pkt
 *       instead = 15; hi is load-bearing.
 * IMPORTANT: T1 (s3) is NOT refuted — it correctly closed perturbations of the
 * stream51 9-pseudo graph, but pseudo-MERGING spellings build a different graph
 * outside its vocabulary. The s3 closure does not bound this spelling class.
 *
 * Verify after applying to src/display.c (replace the whole body):
 *   & tools/wteng.ps1 main sandbox func_8007C7A0 --disable all   # expect 12, 51/51
 *
 * Twin: func_8007C86C (0xE4000000) — same levers should transfer; unverified.
 * Provenance: PsyQ 4.0 LIBGPU SYS: get_cs (static) — verbatim-linked Sony object
 * (census 2026-07-09); ground-up C (no published reference matches this build).
 */

s32 func_8007C7A0(s16 arg0, s16 arg1)
{
    s16 x = arg0;
    s16 tx;
    s32 hi;
    s32 lo;
    s32 pkt;

    if (arg0 >= 0) {
        if ((D_8009BE78 - 1) < arg0) {
            tx = D_8009BE78 - 1;
        } else {
            tx = x;
        }
    } else {
        tx = 0;
    }
    x = tx;

    hi = arg1;
    if (hi >= 0) {
        if ((D_8009BE7A - 1) < arg1) {
            arg1 = D_8009BE7A - 1;
        }
    } else {
        arg1 = 0;
    }

    tx = (u32)(D_8009BE74 - 1);
    hi = arg1 & 0xFFF;
    if (tx >= 2U) {
        hi = arg1 & 0x3FF;
        hi = hi << 10;
        lo = x & 0x3FF;
    } else {
        hi = hi << 12;
        lo = x & 0xFFF;
    }
    pkt = lo | 0xE3000000;
    return hi | pkt;
}
