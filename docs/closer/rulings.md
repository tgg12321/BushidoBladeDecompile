# Closer campaign — owner rulings

Audit trail of owner (Trenton) rulings specific to the final-stretch campaign
on `marionation_Exec` + `cpu_side_move_dir_4`. Binding on all Closer sessions.

## Ruling 1 — cross-symbol lever (`&D_800A1494 - D_800A125C`) — 2026-07-09

**Status: DEFERRED — sanction refused pending evidence.**

Owner's words: "Do we have a compelling case to sanction this? Is it SOTN
standard to do so? I don't want to make an exception just because this has
been hard."

Disposition:
- The lever family is **NOT sanctioned** for new candidates. Hardness is not
  a justification; community standard (SOTN-class projects) treats
  cross-symbol arithmetic between unrelated symbols as a fakematch to be
  eliminated, not codified.
- The community-standard resolution exists IF the symbols are genuinely one
  object: **merge them into a single struct/array declaration**, after which
  base+offset indexing is honest single-object arithmetic. Layout is
  suggestive (D_800A1494/95/96 sit at +0x238..+0x23A from D_800A125C — exactly
  past 142 s32 entries) but UNPROVEN.
- Therefore: Phase 1 W4 (structure provenance) must answer, with evidence:
  does the game treat 0x800A125C..0x800A1497 as one data structure?
  - If YES: re-declare as one struct in a header, respell the windows as
    member/offset access, re-measure. No exception needed — the honest form
    may even preserve the p79 refs (to be measured, s98 caveat noted).
  - If NO (unrelated symbols): the current h5 chain-lever form is a
    fakematch. It may remain in-tree as work-in-progress but the function
    cannot be COMPLETED with it; endgame goes through Ruling 2.
- Interim: the committed h5 form stays as WIP scaffolding only. Nothing is
  submitted for completion using the unsanctioned lever.

## Ruling 2 — endgame standard if impossibility is proven — 2026-07-09

**Status: RULED.**

Owner's words: "I'd prefer a fake construct over asm. Unless we have a
strong confidence the underlying structure was asm canonically. I don't want
us wasting time trying to decompile to C what was once asm."

Disposition:
- If Phase 2's enumeration proves no honest C form closes a window, the
  preferred close-out is a **minimal FAKE-annotated C construct** (per the
  do-while(0) precedent), NOT canonical-asm acceptance.
- Canonical-asm acceptance is reserved for windows with **strong affirmative
  evidence the original was hand-written assembly** (e.g., register usage or
  scheduling provably outside the compiler's reachable output space for ANY
  C input — not merely outside the space we searched).
- Corollary: any Closer impossibility argument must state which side of that
  line the evidence falls on ("compiler could emit this, we can't find the C"
  vs "no C input can make this compiler emit this").
