# func_800617C8 (text1b.c) — BLOCKED, member of the func_800611A4 RA-wall family

## TL;DR
Same wall as **func_800611A4** (read `memory/wip/func_800611A4/notes.md` first).
Honest pure-C floor = 9; all 9 diffs are a single **v0<->v1 register-name swap**
on the post-call `mask(0xC06013)/load` cluster. Build is otherwise byte-identical
to target. HEAD "matches" only via `register asm("$2")/asm("$3")` pins (cheat-asm).
The swap is unreachable from correct pure C — proven for the family via cc1psx
calibration (both our GCC port and the original PsyQ cc1psx put load-temp in $v1,
mask in $v0; target is the reverse).

## Family
text1b render-state cluster sharing the identical pin + post-call mask/load shape:
func_800611A4, **func_800617C8 (this)**, func_800618B4 (already PARKED), func_80061250,
func_80061454, func_800614E0, func_8006156C, func_80061658. The only per-function
difference is the mask constant and the pre-call branch logic — the wall is in the
shared post-call cluster. **Solve once, retire all.**

## Resume
Don't re-run the 12-lever sweep — it's recorded in func_800611A4. Pursue the
family-wide avenues in that WIP's next_hypotheses (permuter from floor-9 base;
non-constant-mask hypothesis; instrumented-cc1 PRIO dump), or user canonical-asm
authorization for the whole family. Source left at HEAD (oracle green).
