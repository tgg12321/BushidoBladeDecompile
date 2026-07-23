# Evidence bank — func_8006156C

- s1 MATCH (2026-07-22): distance 0. The residual-6 was NOT a scheduler wall.
  The prior sweep tested array-index (`arg0[i]`) access only; the closing lever
  is the WALKING-POINTER form the COMPLETED-C sibling func_800618B4 already uses:
  `p=arg0; D_800F1140=*p++; D_800F1144=*p++; D_800F1148=*p; D_800A3464=0xFF8080;`
  Pure C, 0 rules, 0 cheat-asm. cheat-reviewer PASS. Retires HEAD's 2-pin cheat.
  Ref: .claude/rules/walking-pointer-serializes-parallel-loads.md
  Note: WIP's "everything else is 9 or 11" only covered array-index variants —
  the *p++ pointer-walk was never in that sweep.


- WIP rejected_form: {'form': 'register s32 t asm("$2"); register s32 mask asm("$3"); (HEAD\'s committed form)', 'score': 0, 'reason': 'FORBIDDEN cheat-asm register-asm pins — see .claude/rules/register-asm-pins / register-alloc-pure-c Lever D and inline-asm-policy. Byte-matches but is not COMPLETED-C.'}

- WIP rejected_form: {'form': 'mask stored mid (between arg0[1] and arg0[2]) — any of: plain const, separate var m, m live across loads', 'score': 9, 'reason': 'Not a cheat, just worse: floor 9. Mask-FIRST (floor 6) is strictly better. Do not re-derive the mid-store variants.'}

- WIP rejected_form: {'form': 'three separate load locals x0/x1/x2 (no reuse), mask mid', 'score': 11, 'reason': "Not a cheat, worse (floor 11). Separate locals let GCC hoist all 3 loads into distinct regs — the opposite of target. Single reused 't' is required."}

- == imported from memory/wip notes.md ==
# func_8006156C (text1b.c) — WIP, floor 6, BLOCKED for permuter/canonical review

## TL;DR (2026-06-14)
HEAD's committed "match" used two FORBIDDEN cheat-asm pins
(`register s32 t asm("$2")`, `register s32 mask asm("$3")`) — not COMPLETED-C.
Removed both → honest floor 11. Reusing ONE local `t` for the three tail
`arg0[i]` coordinate loads + storing the `0xFF8080` mask FIRST lowers the
floor to **6**. That candidate is clean (0 rules, 0 cheat-asm) and is saved
as `candidate.c`. Could NOT close to 0 with C levers.

## The residual 6 (the whole gap)
Control flow + the 3 coordinate loads MATCH (all three reuse one register, as
target does). The only difference is the `0xFF8080` mask stored to
`D_800A3464`:
- **Target:** `lui $v1,0xff` scheduled EARLY (into a load-delay slot), then
  `ori $v1,$v1,0x8080` + `sw $v1,%gp_rel` LATE — the const is kept live in
  `$v1` and its construction is interleaved across the coordinate loads.
- **Mine:** the whole `lui $v0 / ori $v0 / sw $v0` mask is hoisted as a block
  ahead of the loads (in `$v0`).

This is an intra-basic-block instruction-SCHEDULING choice (delay-slot fill /
which reg the const lives in). No C statement ordering controls it — verified
by sweeping ~16 tail variants (mask first/mid/last, mask as var vs const,
reused vs separate load temps, function-scope vs block-scope decls). Best is
always mask-FIRST + reused-`t` = 6; everything else is 9 or 11.

## Resume steps
1. Paste `candidate.c` over func_8006156C in src/text1b.c; confirm sandbox 6.
2. Next modality = decomp-permuter scheduling search from this base (the
   residual is exactly its specialty). VERIFY any candidate with the engine
   `sandbox` + `retire` (faithful full pipeline) — NOT the isolated permuter
   score (PERMUTER_PIPELINE.md: isolated match != survives full build).
3. If the permuter also plateaus, escalate: the only known byte-match is the
   2-pin cheat-asm form, so this likely needs canonical-asm authorization or
   acceptance as a documented scheduler plateau.

## Ruled out (do not re-derive)
- 2-pin cheat-asm form (HEAD): byte-matches but FORBIDDEN. floor 0 via cheat.
- mask stored mid / split / as live var: floor 9 (worse than mask-first 6).
- 3 separate load locals (no reuse): floor 11 (GCC hoists all 3 loads).
- mask declared function-scope before/after `t`: no change (still 6 or 9).

## Pointers
- `.claude/rules/register-alloc-pure-c.md` (Lever family; pins forbidden)
- `.claude/rules/inline-asm-policy.md` / `register-asm-pins` (why pins are cheats)
- `docs/PERMUTER_PIPELINE.md` (permuter setup + faithful-verify caveat)

