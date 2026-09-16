/* func_8006CCC8 — src/text1b.c — session s3 (structural)
 * Sandbox --disable all floor at end of s3: 77 (target_insns=189, build_insns=183).
 * UNCHANGED from s2's floor (77) despite a real structural fix landing this
 * session (see H4 in hypotheses.md) — the inner j-loop restructure lowered
 * build_insns 185->183 and now matches target's disassembly insn-for-insn
 * in every region hand-walked this session, but the sandbox LEVENSHTEIN
 * score did not move, meaning the 77-worth of edit distance lives in a
 * region NOT yet objdump-diffed at the instruction level (see s3 evidence
 * CONCLUSION + the s4 frontier in hypotheses.md).
 *
 * This candidate IS applied to src/text1b.c as of end of s3 (2026-09-16) —
 * it is the resident WORKING-TREE body (never committed; asm-until-matched
 * keeps main as INCLUDE_ASM until sandbox reaches 0). NOTE: at s3 dispatch,
 * src/text1b.c STILL carried INCLUDE_ASM (a repeat of the s1->s2 stale-HEAD
 * pattern flagged by this session's CONSISTENCY WARNING) — the s2 body had
 * never actually been re-applied after s2 ended. Re-applied verbatim at s3
 * start, re-verified score 77 (matches ledger), then made the H4 change.
 *
 * s3 change from the s2 body (measured this session, same chassis):
 *   H4 (inner j-loop record-update restructure): hoisted the
 *   `*(rec+0x1A)&mask` / `*(rec+0x1D)&mask` and `*(rec+0x17)` reads OUT of
 *   the `if (i==0) {...} else {...}` arms into two fresh locals (`masked`,
 *   `byte17`) read ONCE before the branch, which now only selects which
 *   nibble mask (0xF0 vs 0xF) applies to `byte17` before adding `masked`
 *   and storing. Applied identically to both the field28==3 (+0x1A) and
 *   field28==4 (+0x1D) arms. This reproduces target's own observed
 *   unconditional-both-loads-then-select-by-branch shape
 *   (`asm/funcs/func_8006CCC8.s:130-146` / `:165-181`) instruction-for-
 *   instruction, including exact register-number identity (our raw $20/
 *   $23/$22/$21/$19/$18/$17/$16/$fp/$31 ARE target's $s4/$s7/$s6/$s5/$s3/
 *   $s2/$s1/$s0/$fp/$ra numerically). Ordinary C — no coercion, no dead
 *   code; both locals hold real, once-read consumed values.
 *
 * KNOWN REMAINING GAP (floor 77, UNCHANGED by the H4 fix): the sandbox
 * score is a Levenshtein edit-distance between our built .o and the
 * reference .o (engine/score.py score_func), not a raw instruction count —
 * build_insns dropping 185->183 while the score stays flat at 77 means
 * whatever region the remaining 77 comes from was NOT touched by this
 * session's fix. Both the inner j-loop (H4's target) and the function's
 * tail/epilogue/prologue were hand-walked this session and found to match
 * target's disassembly with no visible mismatch. The residual is most
 * likely in the outer loop's `*arg1 & (0x1000<<shift)` / `*arg1 &
 * (0x4000<<shift)` increment/decrement arms (`asm/funcs/func_8006CCC8.s`
 * roughly lines 55-95), which s2's evidence entry claimed "matched
 * structurally" by EYE only, never verified through the engine's actual
 * `score.normalized_insns` levenshtein tool — that tool could not be run
 * this session (needs WSL objdump; blocked from the Windows-side Bash tool
 * by worktree_contamination_guard's bare-`from engine import` block, and
 * objdump isn't on the Windows PATH for a direct PowerShell attempt either).
 * Next session's first move: run the diff FROM WSL to get the exact
 * opcode-level residual instead of continuing to hand-walk assembly (see
 * hypotheses.md s4 frontier item #1).
 *
 * Structure derived from m2c --valid-syntax over asm/funcs/func_8006CCC8.s
 * (s1 provenance, unchanged since). Field layout cross-checked against
 * sibling func_8006CBD4 (same TU, same D_800A34FC / D_800A3524 struct
 * idiom).
 */
s32 func_8006CCC8(s32 *arg0, s32 *arg1, s16 arg2) {
    s32 i;
    s16 lim;
    s32 shift;
    s32 mask;
    s32 fade;
    s32 j;
    u8 *rec;
    s32 ret;
    s32 t;
    s32 masked;
    u8 byte17;

    ret = 0;
    if ((*(s32 *)((u8 *)D_800A34FC + 0x28) == 0x50005) && (*arg1 & 0x400040)) {
        func_8005C650(1, 0x7F, 0x7F);
        ret = 1;
    }

    fade = 0;
    for (i = 0, shift = 0, mask = 0; i < 2; i++, shift += 0x10, mask += 2) {
        t = arg2;
        lim = ((t >> i) & 1) ? 4 : 5;

        if (*arg1 & (0x1000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) <= 0) {
                *(s16 *)((u8 *)D_800A34FC + mask + 0x28) = lim;
            } else {
                *(s16 *)((u8 *)D_800A34FC + mask + 0x28) = (s16)(*(s16 *)((u8 *)D_800A34FC + mask + 0x28) - 1);
            }
        } else if (*arg1 & (0x4000 << shift)) {
            func_8005C650(0, 0x7F, 0x7F);
            if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) >= lim) {
                *(s16 *)((u8 *)D_800A34FC + mask + 0x28) = 0;
            } else {
                *(s16 *)((u8 *)D_800A34FC + mask + 0x28) = (s16)(*(s16 *)((u8 *)D_800A34FC + mask + 0x28) + 1);
            }
        }

        if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) == 3) {
            if (*arg1 & (0x40 << shift)) {
                func_8005C650(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    masked = *(rec + 0x1A) & (0xF << fade);
                    byte17 = *(rec + 0x17);
                    if (i == 0) {
                        *(rec + 0x17) = (u8)((byte17 & 0xF0) + masked);
                    } else {
                        *(rec + 0x17) = (u8)((byte17 & 0xF) + masked);
                    }
                }
            }
        } else if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) >= 4) {
            if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) == 4) {
                if (*arg1 & (0x40 << shift)) {
                    func_8005C650(1, 0x7F, 0x7F);
                    for (j = 0; j < 3; j++) {
                        rec = (u8 *)D_800A3524 + j;
                        masked = *(rec + 0x1D) & (0xF << fade);
                        byte17 = *(rec + 0x17);
                        if (i == 0) {
                            *(rec + 0x17) = (u8)((byte17 & 0xF0) + masked);
                        } else {
                            *(rec + 0x17) = (u8)((byte17 & 0xF) + masked);
                        }
                    }
                }
            }
        } else if (*(s16 *)((u8 *)D_800A34FC + mask + 0x28) >= 0 && (*arg1 & (0xF0 << shift))) {
            func_8005C650(0, 0x7F, 0x7F);
            t = i;
            func_8006CBD4(t, *arg1);
        }
        fade += 4;
    }
    return ret;
}
