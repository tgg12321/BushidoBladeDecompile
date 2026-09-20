/* func_8006ECF4 — session 3 (structural), continuation of s2's floor-60 body.
 * Floor history this session: chassis-confirmed 60 at dispatch, then FOUR more
 * measured structural levers, each landed and measured immediately:
 *
 *   60 (s2 floor, re-confirmed)
 *   -> 58  H4: struct-init store REORDER. Read asm/funcs/func_8006ECF4.s lines
 *          1-17 directly (not the diff tool's register-renamed view): the real
 *          store order for the S46C fields set once before the loop is
 *          one14=0x14, c20=0x200, zero18=0, zero1C=0, c24=0x100 — i.e. the
 *          0x100 constant (c24) is loaded into a register early but its STORE
 *          is deferred until after zero18/zero1C (both zero-stores need no
 *          register, so they interleave for free), landing LAST. The s2
 *          candidate had c24 third in source (before zero18/zero1C); moving it
 *          to last in the initializer statement order matches. Ordinary
 *          named-intermediate / statement-order C, no annotation needed.
 *   -> 39  H5: rewrote the s.p1 dispatch region (asm offset ~0x8006EF24-EFB8,
 *          labels .L8006EF24/.L8006EFA4/.L8006EF98/.L8006EFB8) from an
 *          if/else-if CHAIN (s2's spelling) to explicit `goto`s mirroring the
 *          asm's REAL branch topology: THREE separate conditions (D_800A35B0
 *          != 0; D_8009BC7C[D_800A3560[i*3+1]] & 2; D_800A35BC == 2 && bit)
 *          all jump to ONE shared `p1_idx:` block computing
 *          `s.p1 = *(s32*)(s3 + i*4)`, and the two negative paths
 *          (D_800A35BC != 2, or ==2 but bit clear) both fall into ONE shared
 *          `p1_fallback:` computing `s.p1 = *(s32*)(s3+4)`. The s2 body wrote
 *          the shared formula as a DUPLICATED STATEMENT in three separate
 *          `if`/`else if` arms — GCC compiled each occurrence as its OWN
 *          block instead of merging them via cross-jump, costing ~15 extra
 *          instructions. Spelling the real shared-label topology as explicit
 *          `goto`s (the SOTN-sanctioned "mixed exit forms" family,
 *          [[cross-jump-store-tail-merge]] / [[no-new-park-categories]]) lets
 *          GCC emit the ACTUAL single shared block per side, matching target.
 *          Ordinary control-flow-topology C, not a cheat — this is literally
 *          what the disassembly does, transcribed faithfully instead of
 *          restated as an equivalent-but-differently-shaped if/else chain.
 *   -> 37  H6: fixed the D_8009BC7C[sel]&1==0 (else) branch's pad-byte STORE
 *          ORDER. Read asm/funcs/func_8006ECF4.s lines 71-76 (.L8006EE00
 *          block) directly: target stores the three pad bytes in DESCENDING
 *          offset order (+0x2B, +0x2A, +0x29) in this branch — the OPPOSITE
 *          of the if-branch's ascending order (+0x29,+0x2A,+0x2B, unchanged
 *          from s2). (While investigating this block I initially misread the
 *          delay-slot semantics and mis-hypothesized byte28's value should be
 *          0 in the else branch — measured that at floor 46 [WORSE], caught
 *          the delay-slot execute-always semantics of MIPS branches on
 *          re-reading `beqz v0,.L8006EE00` / delay `addiu v0,zero,1`: the
 *          delay slot's `v0=1` executes UNCONDITIONALLY including on the
 *          taken branch, so `sb v0,0x38(sp)` at .L8006EE00 really does store
 *          1 — reverted byte28 back to the s2 value of 1, kept only the
 *          store-order fix. See the KILLED entries below.) Ordinary
 *          statement-order C.
 *   -> 11  H7: rewrote the tail `func_80073728` call from a single call with
 *          a `(i != 0)` boolean-expression argument to an explicit `if (i !=
 *          0) { call(...,1); } else { call(...,0); }` — mirroring the asm's
 *          REAL structure (.L8006EFC4-EFDC: `beqz`/`j` choosing between
 *          `li a1,1` and `move a1,zero` via two literal branches, NOT a
 *          `sltu`-based boolean materialization). The s2/H4-H6 body's
 *          `(i != 0)` ternary-as-argument compiled to `sltu a1,zero,a1`
 *          (comparison-to-boolean idiom) which is a semantically-equivalent
 *          but STRUCTURALLY DIFFERENT GCC codegen path from an explicit
 *          if/else with two call sites — this single change dropped the
 *          floor from 37 to 11, the largest single lever this session. Same
 *          SOTN-sanctioned "mixed exit forms" / duplicated-call-in-both-arms
 *          family as H5 (two call sites differing only in one literal arg —
 *          [[no-new-park-categories]] "Unconditional-common-store
 *          duplication into both branch arms", F7 survey). Ordinary C.
 *
 * Current honest floor: 11 (target 209, ours 211), measured via
 * `sandbox func_8006ECF4 --disable all` this session (s3).
 *
 * Two hypotheses measured and KILLED this session (both reverted, not in this
 * body):
 *  - Loop-bound expression reorder: `for (i=0; i < D_800A3554+1+D_800A35B0;
 *    i++)` rewritten as `D_800A35B0 + D_800A3554 + 1` to try to match target's
 *    load order (target loads the 32-bit D_800A35B0 via `lw` BEFORE the
 *    16-bit D_800A3554 via `lh`, opposite of this body's `lh`-then-`lw`
 *    order) — measured WORSE (floor 18, then even worse at 9 source-level
 *    hunks vs the current 5) both times tried. The load-order mismatch
 *    (hunks 1/2 in the current diff, "target lw v1 / ours lh v1" and
 *    "target lh v0 / ours lw v0") is very likely a SCHEDULER interleaving
 *    artifact (these loads get scheduled alongside the unrelated
 *    `v0=*(s32*)arg0; s3=*(v0+0x54);` chain for latency-hiding), NOT a
 *    source-text evaluation-order question — KILLED for the naive expression
 *    reorder, instance scope, needs a `.sched` dump read next session before
 *    trying anything else here (see frontier).
 *  - `D_8009BC40[b2 + c*12]` vs `D_8009BC40[c*12 + b2]` (associativity swap
 *    on the two already-named intermediates, mirroring the H1-from-s2
 *    "b2 named" fix) — ZERO score change (still 11 either way). The
 *    `addu v1,v1,v0` vs `addu v0,v0,v1` operand-only tie (hunks 4/5 in the
 *    current diff) is stable under this lever — KILLED, instance scope.
 *
 * Remaining residual per `--diff` re-read at floor 11 (30 hunks): 5
 * source-level, 3 operand-only, 22 not-scored (masked cascade). ALL of the
 * operand-only hunks (1,2,4,28,29 in the floor-58 numbering; now the s2/s3
 * naming-swap hunks) are the SAME single root cause: target assigns `s2` to
 * arg0 and `s3` to the `*(s32*)arg0)->0x54` chain value; our build assigns
 * them the OPPOSITE way round. This is a callee-save register-SEAT tie, not
 * yet attacked this session (turn budget) — NEXT: try reordering this
 * function's local declarations (declare a dedicated `s16 i;` for the loop
 * counter BEFORE the `v0`/`s3`/`s0` chain-derivation locals, per the s2
 * header's original H-note "arg0->s2 first, loop counter->s1 second, THEN
 * the s3/s0 pair" — not yet tried this session) and re-measure; if that's
 * flat, this needs a `.greg`/`.lreg` dump read (which pseudo gets s2 vs s3
 * and why) rather than further blind reorder guesses.
 * The 5 source-level hunks remaining (hunks 1/2 lh/lw load-order per the
 * KILLED entry above, plus 3 more not yet individually triaged this
 * session) are the next frontier item.
 * jtbl_800159D0 cross-TU rodata-ownership residual (F3, unchanged, not yet
 * reached since score is not yet 0) — same class as func_8006B578
 * (sessions 7-9), do not re-derive, re-run its byte-certification method
 * once this function is otherwise at/near 0.
 *
 * SESSION 4 (permuter modality) ADDENDUM: chassis confirmed at floor 11
 * (dispatch chassis check matched exactly, no drift). Two honest structural
 * probes measured this session, both reverted:
 *   - Removed the `if (sel < 15) { switch {...} } else { s.p0 = ...; }`
 *     wrapper entirely (switch's own `default:` arm already computes the
 *     identical `s.p0 = (void*)(s0+sel*12)` and `goto skip_load`, so the
 *     outer guard is logically redundant for correctness) -> measured
 *     WORSE, floor 46. KILLED, instance scope: the redundant guard is
 *     apparently load-bearing for cc1's codegen shape even though it's
 *     logically redundant for program behavior; do not re-propose removing
 *     it without a new angle.
 *   - Reordered the `i` (s16 loop counter) local's declaration to FIRST
 *     in the local list (before v0/s3/s0/sel/a2), per the s2/s3 frontier
 *     note's untested suggestion -> measured FLAT, still floor 11. KILLED,
 *     instance scope: plain decl-order reordering of `i` alone does not
 *     flip the s2/s3 register-seat tie (hunks 4/5, the `addu v1,v1,v0` vs
 *     `addu v0,v0,v1` operand-only pair — same root cause as s3's
 *     `b2+c*12` associativity swap, also flat).
 * Directed permuter campaign (tmp/perm_ecf4/, validated workspace: base.o
 * 211 insns / target.o 209 insns, diff identical to the sandbox --diff
 * output) ran 15,077+ iterations, base permuter-score 725, best find 470.
 * EVERY closing-score find (output-525-1, output-470-1, output-625-1,
 * output-559-1, ...) converged on the SAME forbidden construct family: an
 * unused `new_var` local written once as a side effect inside an existing
 * comparison/arithmetic expression (`if (sel < (new_var = 15))`,
 * `D_800A3554 + (new_var = 1) + D_800A35B0`) — classic dead-store /
 * constant-holder codegen-steering, occasionally paired with a redundant
 * width mask (`0x12C & 0xFFFFu`, itself the separately-forbidden F2
 * redundant-width-cast family). VERIFIED against the real sandbox (not
 * just the permuter's own proxy score): the `new_var=15` form alone DOES
 * move the honest floor 11 -> 9. REJECTED anyway — see
 * memory/grind/func_8006ECF4/rejected/permuter-new_var-dead-store-cheat.c
 * for the full 6-test checklist writeup. It sits inside a nominally
 * sanctioned family (named-local-fake-exception / dead-store-fake-
 * exception) but this session did not document the lever-exhaustion that
 * family's prerequisites require (only 2 honest structural probes were
 * tried, both above), so it is not eligible for submission yet. A future
 * session that first exhausts honest alternatives for the operand-only
 * `b2+c*12` tie and the `sel<15` extra-insn hunk should re-evaluate this
 * construct WITH a documented exhaustion ledger and a `/* FAKE */`
 * annotation before considering it. No cheat construct was landed or left
 * in src/text1b.c this session; src was reverted to `INCLUDE_ASM` before
 * the session ended (unchanged vs `git show HEAD:src/text1b.c`).
 *
 * No cheat constructs: everything this session is either a plain statement-
 * order change (struct-init store order, H4/H6), a faithful goto-based
 * transcription of the disassembly's own real branch topology (H5/H7, the
 * SOTN-sanctioned mixed-exit-forms / duplicated-into-arms families — the
 * duplicated `*(s32*)(s3+i*4)` / `*(s32*)(s3+4)` computations and the two
 * `func_80073728(...)` call sites are all REAL, semantically-required
 * statements, not dead code), or a plain expression-order try that measured
 * neutral/negative and was reverted. Nothing dead, no annotation needed,
 * nothing claims a sanctioned FAKE family (none of this session's landed
 * constructs need one — they all have a truthful semantic reading tracking
 * the real control flow / real values, same posture as s2's header).
 */

extern s16 D_800A3554;
extern s32 D_800A35B0;
extern u8 D_8009BC7C[];
extern u8 D_800A3560[];
extern u8 D_8009BC40[];
extern s16 D_800A3588[];
extern s16 D_800A358C[];
extern u8 D_800A32E8;
extern u8 D_800A32E9;
extern s32 D_800A3568;
extern s32 D_800A35A8;
extern s32 D_800A35BC;
extern void *D_800A35C4;
typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} Rect_8006ECF4;
extern Rect_8006ECF4 D_800A32F4;

void func_8006ECF4(s32 arg0) {
    S46C s;
    s32 v0;
    s32 s3;
    s32 s0;
    s32 sel;
    s32 a2;
    s16 i;
    Rect_8006ECF4 rectbuf;

    s.one14 = 0x14;
    s.c20 = 0x200;
    s.zero18 = 0;
    s.zero1C = 0;
    s.c24 = 0x100;

    v0 = *(s32 *)arg0;
    s3 = *(s32 *)(v0 + 0x54);
    s0 = s3 + 0xC;

    for (i = 0; i < D_800A3554 + 1 + D_800A35B0; i++) {
        s32 b = D_800A3588[i];
        s32 c = D_800A358C[i];
        s32 b2 = b * 2;
        sel = D_8009BC40[b2 + c * 12];
        if (D_8009BC7C[sel] & 1) {
            s.zero10 = 0;
            if (*(s32 *)((s32)D_800A35C4 + 8) & 4) {
                s.byte28 = 1;
            } else {
                s.byte28 = 0;
            }
            *((u8 *)&s + 0x29) = 0x94;
            *((u8 *)&s + 0x2A) = 0x80;
            *((u8 *)&s + 0x2B) = 0x6E;
        } else {
            s.zero10 = 1;
            s.byte28 = 1;
            *((u8 *)&s + 0x2B) = 0;
            *((u8 *)&s + 0x2A) = 0;
            *((u8 *)&s + 0x29) = 0;
        }

        if (sel < 15) {
            switch (sel) {
            case 12:
                a2 = *(s32 *)((s32)D_800A35A8 + 0x84);
                s.p0 = (void *)(s0 + 0x108);
                break;
            case 13:
                a2 = *(s32 *)((s32)D_800A35A8 + 0x88);
                s.p0 = (void *)(s0 + 0x114);
                break;
            case 14:
                a2 = *(s32 *)((s32)D_800A35A8 + 0x8C);
                s.p0 = (void *)(s0 + 0x120);
                break;
            case 0:
                a2 = *(s32 *)((s32)D_800A35A8 + 0x90);
                s.p0 = (void *)(s0 + 0x12C);
                break;
            case 3:
                a2 = *(s32 *)((s32)D_800A35A8 + 0x94);
                s.p0 = (void *)(s0 + 0x138);
                break;
            default:
                s.p0 = (void *)(s0 + sel * 12);
                goto skip_load;
            }
            if (i != 0) {
                if (D_800A32E8 != sel || D_800A32E9 != D_800A3554) {
                    rectbuf = D_800A32F4;
                    LoadImage((s32)&rectbuf, a2);
                    DrawSync(0);
                }
            }
        skip_load:;
        } else {
            s.p0 = (void *)(s0 + sel * 12);
        }

        if (D_800A35B0 != 0) goto p1_idx;
        if (D_8009BC7C[D_800A3560[i * 3 + 1]] & 2) goto p1_idx;
        if (D_800A35BC != 2) goto p1_fallback;
        if (*(s32 *)((s32)D_800A3568 + 0x14) & 0x20000) goto p1_idx;
    p1_fallback:
        s.p1 = (s32 *)*(s32 *)(s3 + 4);
        goto p1_done;
    p1_idx:
        s.p1 = (s32 *)*(s32 *)(s3 + (s32)i * 4);
    p1_done:;

        s.ret = *(s32 *)(arg0 + 4);
        if (i != 0) {
            *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 1);
        } else {
            *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 0);
        }
    }
}
