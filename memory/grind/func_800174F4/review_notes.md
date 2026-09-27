# Layer-2 review notes — func_800174F4 (session 7, 2026-09-27)

## Session 8 correction

Fresh review of the session-7 body FAILed its under-typed GPU API declarations
and pointer casts. The final source corrects those declarations and removes
the casts from `DrawEnv env` and `u32 ot[2]`; see `session8.md`. Because the
reviewed body changed, the earlier verdict does not carry forward.

Landing body: landing_body.c (typedef + function; candidate.c is the same plus
a header comment). Splice it over `INCLUDE_ASM("asm/funcs", func_800174F4);` in
src/ings.c, leaving one blank line after it. Verified 2026-09-27 under the lock:
sandbox --disable all 0 (136/136, 0 hunks), and the full build SHA1 was
62efab4f73f992798c43e8c730aa43baa10bb4fa. The splice was then reverted
(session closed before review). Commit message: landing_msg.txt. Precheck
(mechanically clean): precheck.txt.

Constructs for the reviewer, with where the evidence lives:
1. `temp` (fade-loop count / case-20 D_800A37A8 code) and `temp2` (switch
   selector / case-20 D_800A37A0): Ruling 11. Proof: evidence.md
   "Ruling 11 proof" + "Ruling 11 (D)(4) permuter harvest"; dumps:
   r11_dumps.txt.
2. Both-arms `D_800A38F8 = cur + 1`: F7 common-store duplication,
   FAKE-annotated (hoisted: 6, 131 insns; `next` hoisted: 1, 135).
3. `DrawEnv` typedef (PsyQ DRAWENV layout, 0x5C, isbg at +0x18) and
   `u32 ot[2]`. Frame: 8 + 96 + an 8-byte reload slot left by the for
   loop's combined entry test (BB2_FRAME_DEBUG spill_new_p128) = 112.
4. `(u32)temp2 < cur`: this cast is what makes the compare `sltu`. A
   `u32 idx = cur` local also scores 0.
5. `(D_800A36AC & 1) ? 0xF0 : 0`, and the literal 0xF0 at both uses (0).

Nothing else is non-obvious: the natural for loop, `u8 *prim` and
`++D_800A37C0` in the compare. The old body's two do-while(0) wraps and the
goto loop are gone.

Landing steps (worker brief): take the lock, splice, `lock.ps1 rebuild`,
sandbox, `git add src/ings.c`, precheck, then layer-2. If it passes: commit
with landing_msg.txt, `queue done`, commit queue.json, run the integrity
check, then write the ledger close.
