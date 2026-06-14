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
