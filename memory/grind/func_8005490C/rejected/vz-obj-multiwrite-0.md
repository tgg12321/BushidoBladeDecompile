# rejected/vz-obj-multiwrite-0.c — layer-2 FAIL (2026-09-25, orchestrator's cheat-reviewer)

Bytes: sandbox 0 (399/399) and full-build oracle SHA1 match, both measured with the body spliced.

The FAIL is on two multiply-written locals:
- `vz` (function scope, written in both rotation blocks): the Ruling 5 extension needs all of
  (A)-(D). (B) fails because no member store selects a record and the RHS reads block temps.
  Base Ruling 5 1(b) then needs a selector, and both blocks rotate the same `vec`. The
  function-scope declaration only buys the allocator effect.
- `obj` (init player 0, init player 1, loop player i):
  - Ruling 5 1(a) fails: the consumers differ.
  - Ruling 5 1(c) fails: each init write is read twice.
  - Ruling 6 (A) fails: the init falls through into the loop.
  - Ruling 6 (C) fails: the RHS is a call.
  - The name is generic.
  - func_80054604's `v` was never ruled on, so it is not precedent.

Rated sound, and landed as byte-neutral prep in a9c634304:
- the ctrl-block array reshape
- g_anim_func_table[]
- the VECTOR typedef move

Also rated sound, and carried in candidate.c:
- the 0x84 pose record
- `ang`
- the annotated `s` alias

When this function lands, the 11 undefined_syms rows marked "retire with func_8005490C" retire
with it. They stay while it is INCLUDE_ASM.
