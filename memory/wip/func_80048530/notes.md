# func_80048530 (text1b.c) — BLOCKED: coupled base-register routing + sched + c/d lhu

## TL;DR
HEAD "matches" only via 5 regfix rules + inline-move-aliasing cheat-asm + ""
barriers + 3 register pins. The fully cheat-FREE pure-C form (candidate.c) scores
12 (HEAD's partially-cheat-credited honest floor is 11), so candidate is NOT yet
below HEAD — src/ left at HEAD (cheated but oracle-green). A prior pure-C
cheat-cleanup was REVERTED (53a441c3) as an inline-asm-injection cheat. This is a
documented hard function.

## The three coupled diffs (clean floor 12, 43 vs 47 insns)
1. **arg0 base-register routing** — target: `move v1,a0` → use v1 for
   `a1=arg1*4+v1` & `lw v0,0(a1)` → `move t0,v1` (2nd copy of arg0) → `addu
   v1,v0,v1` (v1 becomes the walking pointer) → keeps `t0`=arg0 for the final
   `addu a0,a0,t0`. Our clean build keeps arg0 in ONE register and never makes
   the 2nd copy. (the subst @6/@9 + delete @8 + reorder @1-9 force this.)
2. **`move a1,a3` hoist** — target sets a1=arg3 right after `lw v0,0(a1)`
   (before the bounds-check `beqz`), reusing the freed a1; we emit it inside the
   taken branch. (reorder @27-35 forces this.)
3. **c/d `lh` vs `lhu`+`sll`+`sra`** — `c=(s32)(s16)*(u16*)p` folds to `lh`;
   target loads `lhu` then sign-extends with shifts (like a,b, which are passed
   `(s16)a` so they get lhu+sll+sra). Holding c/d as u16 + `(s16)` at the call
   DOES restore lhu+sll+sra but **cascades the whole arg-register setup to 22** —
   diff 3 is coupled to diffs 1/2.

## What was tried (don't re-derive — see meta.json rejected_forms)
4 structural variants: explicit u8* base, index-load temp, arg3-early-local,
c/d-u16-hold. All 12 except c/d-u16-hold which regressed to 22.

## Resume guidance
1. Apply candidate.c, confirm sandbox 12.
2. Best next steps (meta.json next_hypotheses): permuter from floor-12 base;
   a base-routing lever to force the `move t0,v1` 2nd-copy (try walking a `u8 *p`
   while keeping a separate `s32 base=arg0`); THEN re-try the c/d-u16-hold form
   (its cascade may vanish once the base registers are right).
3. The sibling func_800485EC (right below, also pinned) may share the routing.

Source at HEAD (oracle green; HEAD is the cheated form). Card BLOCKED. Needs a
permuter find or a base-register-routing lever, then the coupled c/d fix.
