# motion_SetMotion (code6cac_c_mid.c) — WIP, BLOCKED, two coupled cheats

## TL;DR (2026-06-14)
402-insn motion-state dispatcher. HEAD byte-matches ONLY via TWO forbidden
cheats that are COUPLED:
1. **Register pins** `register s32 result asm("s3")`, `register s32 sel2 asm("s2")`.
2. **regfix subst** `motion_SetMotion: subst "addiu\t$16,$zero,12" "...,13" @149`
   — commented "anti-cross-jump fix".

No floor reduction this session — the work is diagnosis. CRITICAL: the
register swap is **masked by the sandbox** (a pure $s2<->$s3 swap scores 0 in
`--disable all`), so the sandbox shows 10 whether or not RA is right. Only the
full-build SHA1 (`retire`/`verify-oracle`) reveals the real gap. Do NOT trust
the masked sandbox score to guide this function.

## The two issues (both must be solved, in pure C)
### 1. Register allocation
Target: `result -> $s3`, `sel2 -> $s2` (target's `func_8006BEC4($s0,$s2)` =
`func_8006BEC4(sel, sel2)` confirms sel2=$s2; `move v0,$s3` at return =
result). With pins removed GCC flips to `result->$s2, sel2->$s3`. The
allocator gives `result` the LOWER reg even though `sel2`'s live range ends
earlier (sel2's last use is the mid-function call; result lives to the
return). Lever to try: register-alloc-pure-c Lever A (block-local split /
use-order) to push sel2 into $s2.

### 2. Cross-jump merge (the regfix)
The target has `li $s0,0xC` at 80038EB4 (case 10 false-branch: `sel=0xC; if
(...0xA000A000) D_800A3206=1; goto load_sel2`) and `li $s0,0xD` at 80038ECC
(in the delay slot of `j sel_dispatch` = case 9/11). The SOURCE writes
`sel=0xC` in BOTH case 10's load_sel2 path and case 9/11. GCC's jump2
`cross_jump` merges the two tails reaching sel_dispatch, and the regfix patches
the merged constant 12->13. PROVEN: setting `case 9/11: sel=0xD` and removing
the rule FAILS the full build (retire SHA1 mismatch 82e50f76...) — so 0xD is
NOT the literal source value; the 0xD is a cross-jump artifact.
Lever to try: cross-jump-store-tail-merge / cross-jump-call-merge — make the
two tails' ENDINGS not rtx_equal so jump2 won't merge.

## Resume steps
1. Remove ONLY the two pins (see candidate.c). Keep the body + `sel=0xC` as-is.
2. Work issue #1 (RA) and #2 (cross-jump) — verify EACH via full build
   (`retire` rolls back on mismatch), NEVER the masked sandbox.
3. If both plateau, escalate canonical-asm or document as coupled plateau.

## Ruled out (do not re-derive)
- `case 9/11: sel=0xD` + rule removed: full-build SHA1 MISMATCH. 0xD is wrong
  as a literal; it's a cross-jump artifact.
- declaration-order swap (sel2 first): masked-sandbox 11->9 is NOISE; the raw
  s2/s3 swap persisted. Not a real fix.
- Hand-rolled standalone pipeline for unmasked diffing: my tmp/realdiff.sh
  produced a 788-byte stub (pipeline reproduction failed). Use the engine's
  faithful build paths, not a hand-rolled cpp|cc1|maspsx chain.

## Pointers
- `.claude/rules/cross-jump-store-tail-merge.md`, `cross-jump-call-merge.md`
- `.claude/rules/register-alloc-pure-c.md` (Lever A; pins forbidden)
- `.claude/rules/proven-spelling-class-reconstruction.md` (NOT applicable — this
  is a value/cross-jump artifact, not a same-bytes respelling)
