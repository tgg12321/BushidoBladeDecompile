/* func_8006ECF4 — session 2 (structural), continuation of s2's first-real-body draft.
 * Floor history this session: chassis discontinuity at dispatch (banked 113, ledger
 * claimed 209 — the s1 recon ledger predates any C body ever being applied/measured).
 * Re-applied the banked s2 draft, confirmed 113, then found three more source-level
 * levers, each measured immediately after landing:
 *
 *   113 (s2 draft, re-confirmed)
 *   -> 106  H1: stage `b*2` through its own named local (`b2`) BEFORE combining with
 *           `c*12`, matching the asm's addu-chain order (target: sll v1,b,1 THEN
 *           sll/addu/sll for c*12 THEN addu v1,v1,v0). The un-staged `b*2 + c*12`
 *           expression reordered one sll relative to target; naming the sub-term
 *           fixed LUID/emit order. Ordinary named-intermediate C
 *           ([[no-new-park-categories]] SOTN-accepted "named-intermediate
 *           declaration order" family — no annotation needed, real consumed value).
 *   -> 96   H2: the RECT struct copy (`rectbuf = D_800A32F4; LoadImage(&rectbuf,...)`)
 *           must be a 4x s16 struct (2-byte alignment), NOT a 2x s32 struct (4-byte
 *           alignment). Target emits lwl/lwr + swl/swr (unaligned 8-byte copy) —
 *           only happens when GCC doesn't know the struct is 4-aligned. The project
 *           already has this exact shape as `Rect` (src/ings.c:84-89, s16 x,y,w,h)
 *           used in the exact same `rect = *(Rect*)&GLOBAL; LoadImage((u8*)&rect,...)`
 *           idiom (src/ings.c func_80016A8C). Declared a TU-local `Rect_8006ECF4`
 *           with the identical shape (couldn't reuse ings.c's file-local typedef
 *           across TUs) and re-typed `D_800A32F4`/`rectbuf`.
 *   -> 60   H3: DELETED the manufactured `tookCase` flag entirely. Reading
 *           asm/funcs/func_8006ECF4.s directly (glabel func_8006ECF4, lines 84-138)
 *           showed the real control flow: the jump table's non-explicit entries
 *           (sel in {1,2,4-11} and the sel>=15 path) jump straight to the SHARED
 *           fallback `s.p0 = s0 + sel*12` and skip the `if (i != 0)` /
 *           LoadImage-guard block entirely — there is no runtime flag distinguishing
 *           "matched a case" at all, it's pure control-flow topology (jump-table
 *           target selects whether the guard code is even reached). Rewrote as a
 *           real `switch` whose `default:` computes the fallback and `goto
 *           skip_load` past the `if (i != 0) {...}` guard, while the 5 explicit
 *           cases fall through into the guard. This is the ordinary C shape a human
 *           reading the disassembly would write (mixed switch-fallthrough + goto,
 *           the SOTN-sanctioned "mixed exit forms" family, [[no-new-park-categories]]
 *           / [[cross-jump-store-tail-merge]] — ordinary goto, not a cheat) and
 *           removed the flag variable's `li v1,1` sets that were adding an extra
 *           instruction cluster per case (visible in the s2-diff hunk 17/18 hi
 *           residual, ~20 target insns' worth of source-level noise).
 *
 * Current honest floor: 60 (target 209, ours 213), measured via
 * `sandbox func_8006ECF4 --disable all` this session (s2 continuation).
 *
 * Remaining residual per `--diff` re-read at floor 60 (33 hunks, 33/33 last hunk
 * list read this session): still-open SOURCE-LEVEL hunks (not yet chased, next
 * session's frontier):
 *  - hunks 3/4 (asm offset ~12-17): local decl/store ordering in the entry block —
 *    target stores `zero18`/`zero1C` BEFORE the `c24=0x100` value register is
 *    reused for the arg0 dereference chain; likely GCC's own prologue-adjacent
 *    store scheduling rather than something C statement order controls directly —
 *    UNCONFIRMED, not yet measured either way.
 *  - hunks 24/25/27/29/30/31/32/33 (asm offset ~142-194): the tail `s.p1 =`
 *    4-way if/else-if chain and the trailing `func_80073728` call region. Still
 *    ~9 source-level hunks here after H1-H3; not yet read in detail this session
 *    (turn budget). NEXT: read asm/funcs/func_8006ECF4.s from label around
 *    offset 0x142 (.L8006EF24 onward) directly (faster than the interleaved
 *    diff) the way H3 was found, rather than guessing from the diff tool's
 *    register-renaming-obscured hunks.
 *  - jtbl_800159D0 cross-TU rodata-ownership residual (F3, unchanged, not yet
 *    reached since score is not near 0) — same class as func_8006B578
 *    (sessions 7-9), do not re-derive, re-run its byte-certification method
 *    once this function is otherwise at/near 0.
 *
 * No cheat constructs: ordinary struct type (matches an existing project-wide
 * shape, `Rect` in src/ings.c), an ordinary named intermediate (`b2`), and an
 * ordinary switch+goto+fallthrough control-flow shape mirroring the disassembly's
 * actual jump-table topology. Nothing dead, no annotation needed, nothing claims a
 * sanctioned FAKE family (this session's constructs don't need one — they all have
 * a truthful semantic reading tracking the real control flow / real values).
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
    s.c24 = 0x100;
    s.zero18 = 0;
    s.zero1C = 0;

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
            *((u8 *)&s + 0x29) = 0;
            *((u8 *)&s + 0x2A) = 0;
            *((u8 *)&s + 0x2B) = 0;
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

        if (D_800A35B0 != 0) {
            s.p1 = (s32 *)*(s32 *)(s3 + (s32)i * 4);
        } else if (D_8009BC7C[D_800A3560[i * 3 + 1]] & 2) {
            s.p1 = (s32 *)*(s32 *)(s3 + (s32)i * 4);
        } else if (D_800A35BC == 2 && (*(s32 *)((s32)D_800A3568 + 0x14) & 0x20000)) {
            s.p1 = (s32 *)*(s32 *)(s3 + (s32)i * 4);
        } else {
            s.p1 = (s32 *)*(s32 *)(s3 + 4);
        }

        s.ret = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, (i != 0));
    }
}
