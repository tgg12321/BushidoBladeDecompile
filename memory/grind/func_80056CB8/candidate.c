/* =====================================================================
 * func_80056CB8 — CANDIDATE (s7 body, s9 2026-09-16 enumerate-modality
 * re-verified, NO CHANGE) — floor 58/204, NOT YET 0. (Prior: 81/204 s5/s6.)
 * s9 (enumerate): re-confirmed floor 58 chassis; killed the "hoist
 * 0x1F8002B8 above the loop" frontier hypothesis (60, worse, matches s8's
 * in-loop result); ran an exhaustive spelling_enum sweep (5/5 variants,
 * decl-order + full-inline) of the code==4 tail's dx/dz distance-check
 * sub-expression -- zero gradient, all byte-identical at 58. Body below is
 * UNCHANGED from s7; see hypotheses.md/evidence.md [s9] for the full
 * writeup and the still-open frontier (scratchpad single-materialization +
 * y-compare branch-topology split, both PRE-RA per s8's classify report).
 * ---------------------------------------------------------------------
 * ---------------------------------------------------------------------
 * s7 (structural modality). STALE-HEAD-CLAIM NOTE (same as every prior
 * session): src representation is INCLUDE_ASM between grind sessions;
 * nothing persists on main. Re-applied the s6-banked body (D_800F6610 fix
 * + s5 store-batching + func_80053614 s32-return fix) to src/text1b.c,
 * re-confirmed floor 81/204 (build_insns 197) exactly matches the s5/s6
 * record before any s7 change.
 *
 * THE WIN (structural, SOTN-sanctioned variable-reuse-for-codegen-control
 * family, .claude/rules/no-new-park-categories.md § SOTN-accepted /
 * .claude/rules/defeat-licm-hoist-var-reuse.md — borrowing an EXISTING
 * local for a second unrelated but REAL value, no new local invented):
 *
 * Read asm/funcs/func_80056CB8.s directly (not re-derived from a stale
 * dump) and found target keeps ONE hardware register ($s0) for THREE
 * successive, non-overlapping-lifetime roles across the loop body:
 *   1. `flags` (the D_8009A821 byte-table value, shifted << 8)
 *   2. `ang` — target computes this as `addu $s0,$s0,$v0` (asm lines
 *      .L80056D60/.L80056D90), i.e. literally `flags += ...`, NOT a
 *      separate register/variable. Our s6 candidate declared `ang` as
 *      a SEPARATE fresh local (`ang = flags + ratan2(...);`), forcing
 *      GCC to allocate a second pseudo across the identical live range.
 *   3. `code` — after the Judge-table lookups, target reuses the SAME
 *      $s0 for the final disposition code (`or $s0,$s0,$v0; addiu
 *      $s0,$s0,0x1` at .L80056ED0/.L80056ED4, then the 0/3/4/5 tail).
 *      Our s6 candidate declared a THIRD fresh local `code` for this,
 *      again forcing a separate pseudo across a range that, in target,
 *      is the very same register.
 *
 * `ang` is dead the instant the two Judge-table pointers are computed
 * (`sin_p`/`cos_p` derived from it); `flags`/`ang`'s combined value is
 * therefore genuinely dead from that point until the `flags = (r1 |
 * (r2<<1)) + 1;` line, exactly matching target's reuse window. This is
 * the textbook "reuse an EXISTING local for a second unrelated value"
 * SOTN family (bound 2 in the family-selection table: this is a BORROW
 * of an already-declared local, never an INVENTED one) — not a fresh
 * intermediate, not a dead store, not a cheat: every write is real,
 * every read consumes a real value, and the merge exactly mirrors an
 * observed hardware-register reuse in the target's own asm.
 *
 * CHANGE: removed the `ang` and `code` local declarations entirely;
 * every former `ang` use became `flags` (`flags += *(s16*)(obj+0x1CA);`
 * / `flags += ratan2(...);` instead of `ang = flags + ...;`, and
 * `sin_p = &Judge + (flags & 0xFFF);` instead of `(ang & 0xFFF)`); every
 * former `code` use became a re-assignment of `flags`
 * (`flags = (r1 | (r2 << 1)) + 1;` and the three `flags = 0;`/`flags =
 * 5;` tail assignments), with the final store reading `(s8)flags`
 * instead of `(s8)code`.
 *
 * MEASURED: sandbox func_80056CB8 --disable all: score 81 -> 58/204
 * (build_insns 197 -> 198, one MORE instruction than before, yet the
 * WEIGHTED score dropped by 23 points — the win is register-identity
 * quality, not raw instruction count). Re-verified twice (once
 * immediately after the edit, once again after an unrelated tail
 * experiment was reverted — see below) — reproducible, not a fluke.
 *
 * OBJDIFF EVIDENCE (tools/objdiff.py tmp/sandbox/func_80056CB8/text1b.o
 * build/src/text1b.o, captured tmp/grind/func_80056CB8/s7/objdiff_s7.txt):
 * the diff region SHRANK from the s6 shape to 112 lines covering (a) a
 * pure register-name/loop-counter rotation ($s3 vs $s6/$s7 for the loop
 * index and its derived store address — the standing register-rotation-
 * infrastructure class per no-new-park-categories.md, not a new C
 * lever), and (b) the SAME code==4 tail branch-topology difference noted
 * at s6 (target: `bltz`+`beqz` as two branches; ours: one `bgez`) —
 * EXCEPT NOW target's characteristic FOUR duplicated `addu v0,s7,s6`
 * (recomputing the shared store address into each exit branch's delay
 * slot) has a matching FOUR duplicated `addu v0,s3,s7` in OUR build too
 * (previously our build only had it appear differently) — the address-
 * recompute duplication itself is now reproduced; only the two-vs-one
 * branch split for the y-compare remains different.
 *
 * TRIED AND REVERTED (re-tested KILLED, instance, confirms s6's finding
 * survives the s7 chassis change): rewrote the code==4 tail's y-compare
 * (`y - hit1[1] >= 0` / `>= 0x3E9` nested if/else) using explicit `goto
 * store;`/`goto neg;` early exits mirroring target's two-branch
 * (bltz+beqz) topology instead of the nested if/else fallthrough.
 * Measured: BYTE-IDENTICAL to the nested-if form (58/198, no change) —
 * re-confirms s6's conclusion that this tail's own C control-flow shape
 * is not what gates the branch-topology mismatch; whatever decides it is
 * upstream (register pressure/scheduling earlier in the function).
 * Reverted to the simpler nested-if form (no reason to carry unnecessary
 * goto complexity when it measures no different, twice now, on two
 * different chassis).
 *
 * TRIED AND REJECTED (measured WORSE, instance kill, reverted before
 * finalizing): replacing the two `(&D_8009A821)[i*2]` / `(&D_8009A820)
 * [i*2]` byte-table reads with two PER-TABLE pointer locals (`u8 *pf =
 * &D_8009A821 + i*2; u8 *ps = &D_8009A820 + i*2;`) instead of array-index
 * syntax on the extern bytes. Measured: score 81 -> 83 (WORSE),
 * build_insns 197 -> 196 (fewer raw instructions, but worse weighted
 * register-match quality) — reverted before the flags/ang/code merge was
 * applied (measured on the PRE-merge s6/s7-baseline chassis). A further
 * variant computing `ps` as `pf - 1` (treating D_8009A820 and D_8009A821
 * as one array via cross-declaration pointer arithmetic) was drafted but
 * NEVER MEASURED and immediately reverted without running sandbox — it
 * is undefined-behavior C (pointer arithmetic across two independently-
 * declared objects) and would also deepen the exact declaration-pun the
 * grind brief's DATA MODEL section already flags for these two symbols;
 * not banked as a measured kill, just noted so a future session does not
 * re-derive and try it.
 * ---------------------------------------------------------------------
 * s6 (enumerate modality, PREVIOUS session) content preserved below for
 * provenance — object-model fix (D_800F6610) and the killed shared-idx-
 * local / goto-tail hypotheses are UNCHANGED by this session's edits;
 * see hypotheses.md/evidence.md [s6] entries for the full writeup.
 *
 * OBJECT-MODEL FIX (real, evidence-backed, carried forward unchanged):
 * the ratan2 branch's second argument reads the separate global
 * `D_800F6610` (== D_800F6608+8, same storage as the Rec44 `.w8` field,
 * but a genuinely separate symbol per the target asm's independent
 * lui/lw(D_800F6610) relocation pair) rather than `D_800F6608.w8`. See
 * s6 evidence for the full objdiff-verified derivation.
 * ---------------------------------------------------------------------
 * PREREQUISITE CHANGE IN THE SAME TU (func_80053614 void -> s32 return
 * type, byte-neutral standalone) — unchanged since s2, still required.
 *
 * REMAINING RESIDUAL (58) — register-rotation cluster ($s3 vs $s6/$s7
 * for the loop counter and its derived addresses — no new C lever found
 * yet for this specific rotation) PLUS the code==4 tail's branch-
 * topology difference (one `bgez` vs target's `bltz`+`beqz`), which two
 * independent sessions (s6, s7) now confirm is NOT gated by this tail's
 * own C shape.
 *
 * FRONTIER FOR s8: (1) the loop-counter register rotation ($s3 in ours,
 * $s6-ish role in target, entangled with $s7=arg0 and the derived
 * store-address computations) is the single largest remaining named
 * residual — re-dump .greg/.lreg on THIS (s7, floor-58) chassis (pseudo
 * numbers renumber on every structural edit; do not reuse the s3-era
 * map) and read the disposition table before proposing a lever; this is
 * better suited to rederive/solver modality than more structural
 * guessing. (2) The bgez-vs-bltz+beqz branch-topology tail is very
 * likely gated by whatever decides the loop-counter rotation above, not
 * an independent lever — do not re-try goto-vs-nested-if on this tail a
 * third time, it has now measured neutral twice. (3) Consider whether
 * OTHER existing locals can similarly be borrowed for a second dead-
 * then-reused role (the flags/ang/code merge this session suggests the
 * technique generalizes) — `r1`/`r2` and `dx`/`dz`/`y` were not tried
 * this session; check whether target reuses any of THEIR registers for
 * an unrelated purpose before inventing a merge (register reuse must be
 * observed in target's own asm, not guessed).
 *
 * PROVENANCE: body is the s2-authored draft + s5 store-batching + s6
 * D_800F6610 fix + func_80053614 signature fix + this session's
 * flags/ang/code variable-reuse merge. See earlier session headers
 * (git history of this file, or hypotheses.md) for the full derivation
 * chain back to memory/grind/func_80056CB8/authored-notes-2026-08-18.md.
 * ===================================================================== */

extern s16 Judge;
extern s32 ratan2(s32, s32);
extern u8 D_8009A820;
extern u8 D_8009A821;
extern s32 D_800F6610;

void func_80056CB8(s32 arg0) {
    s32 pt0[4];
    s32 pt1[4];
    s32 hit0[4];
    s32 hit1[4];
    s32 work[4];
    s32 start;
    s32 i;

    start = (*(u16 *)(arg0 + 0x3E8) & 3) * 2;
    for (i = start; i < start + 2; i++) {
        s32 obj;
        s32 flags;
        s32 scale;
        s16 *sin_p;
        s16 *cos_p;
        s32 x;
        s32 z;
        s32 r1;
        s32 r2;

        obj = arg0;
        flags = (&D_8009A821)[i * 2] << 8;
        if ((flags & 0x1000) != 0) {
            obj = *(s32 *)arg0;
        }

        if (*(u16 *)(arg0 + 0x6A) == 0x13 || *(u16 *)(arg0 + 0x6A) == 6) {
            flags += *(s16 *)(obj + 0x1CA);
        } else {
            flags += ratan2(D_800F6608.w0 - *(s32 *)(obj + 0xF4),
                             D_800F6610 - *(s32 *)(obj + 0xFC));
        }

        sin_p = &Judge + (flags & 0xFFF);
        cos_p = &Judge + ((flags + 0x400) & 0xFFF);
        scale = (&D_8009A820)[i * 2] << 8;
        x = *(s32 *)(obj + 0xB8) + ((scale * *sin_p) >> 12);
        z = *(s32 *)(obj + 0xC0) + ((scale * *cos_p) >> 12);
        pt0[0] = *(s32 *)(obj + 0xB8);
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt0[2] = *(s32 *)(obj + 0xC0);
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt1[2] = z;

        r1 = func_80053614(pt0, pt1, (s32)hit0, (s32)work, 0x1F8002B8);
        if (r1 != 0) {
            x += (*sin_p * 0x7D) >> 8;
            z += (*cos_p * 0x7D) >> 8;
        }

        pt0[0] = x;
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x834;
        pt0[2] = z;
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) + 0x1004;
        pt1[2] = z;

        r2 = func_80053614(pt0, pt1, (s32)hit1, (s32)work, 0x1F8002B8);

        flags = (r1 | (r2 << 1)) + 1;
        if (flags == 3) {
            if (hit1[1] - *(s32 *)(obj + 0xBC) < 5) {
                flags = 0;
            }
        } else if (flags == 4) {
            s32 dx = hit0[0] - *(s32 *)(obj + 0xB8);
            s32 dz = hit0[2] - *(s32 *)(obj + 0xC0);
            if (0x3D0900 < dx * dx + dz * dz) {
                s32 y = *(s32 *)(obj + 0xBC);
                if (y - hit1[1] >= 0) {
                    if (y - hit1[1] >= 0x3E9) {
                        flags = 5;
                    }
                } else {
                    if (hit1[1] - y >= 0x3E9) {
                        flags = 5;
                    }
                }
            }
        }
        *(s8 *)(arg0 + 0x444 + i) = (s8)flags;
    }
}
