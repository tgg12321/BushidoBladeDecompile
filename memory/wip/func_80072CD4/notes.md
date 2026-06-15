# func_80072CD4 — register-rotation plateau (text1b.c GameObj setter)

## TL;DR (2026-06-14)
Honest distance 12 (9 regfix rules stripped). The entire gap is a coupled
**$v0<->$v1 register rotation** plus a 10-store ELSE-block reorder. Target
carries `var_v0` (0x46 / 0x32) in **$v0** and the `0xFC` constant in **$v1**
(preset into the inner-if beqz delay slot, re-materialized for the +0x14
store). My build does the inverse and GCC's global allocator will not flip it
with any clean-C lever I tried. **No clean candidate beats HEAD's 12** — the
forms that reached 11 carry empty `do { } while(0)` scheduler barriers
(reviewer FAIL). Blocked on the board; not a fake WIP.

## Resume steps
1. `sandbox func_80072CD4 --disable all` on HEAD = **12** (9 rules stripped).
2. The 9 rules in regfix.txt (lines ~3274-3304) do: 6 `subst` (register
   $2<->$3 swaps + store-offset rewrites), 1 `delete`, 1 lost-codegen
   `insert_after`, 1 ELSE `reorder`. They are ALL the register/order swap.
3. `.greg` of the baseline standalone: pseudo 74 (`var_v0`) -> reg 3 ($v1);
   the 0xFC values -> reg 2 ($v0). Confirmed the inversion is allocator-driven.

## Live hypothesis (best next lever)
- **Lever A on the OTHER pseudo:** the inner byte-store constants
  (0xC3/0x1E/0xC8) all cycle through $v0 right where var_v0 is born. Try
  forcing those constants through a shared local so they vacate $v0 and
  var_v0 can claim it. (Not yet tried — out of budget this pass.)
- Instrumented cc1 ALLOC/PRIO dump ([[register-alloc-deep-dive]]) to read the
  allocno priority tiebreak that pins var_v0 to $v1.
- If clean levers stay negative this is the register-alloc-pure-c
  "confirmed limits" class → canonical-asm review (user decision).

## Ruled out (do not re-derive)
- Drop do-while + literals + reorder THEN/ELSE → 16 (insn count drifts to 77).
- `new_var = 0xFC` before the inner-if → 14 (GCC re-materializes anyway).
- `var_v0` as `s32` (Lever B wide type) → 12 (no change).
- +4-before-0x14 store order WITH do-while barriers → 11 but reviewer-FAIL
  (empty `do{}while(0)` = cheat); saved under rejected/plus4_first_dowhile.c.

## Pointers
- Rule: `.claude/rules/register-alloc-pure-c.md` (Step-0 + Levers A/B/C; the
  "confirmed limits" section is the relevant precedent class).
- The simpler matched family (func_8006133C / func_80061454 / func_800614E0,
  same session) is the SAME $v0/$v1 tie but single-store and was closable with
  a walking-pointer; this one is coupled across a branch merge + ELSE reorder.
