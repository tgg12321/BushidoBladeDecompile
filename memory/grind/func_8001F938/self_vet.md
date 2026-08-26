# SELF-VET — func_8001F938 — VOID (superseded 2026-08-25 by grind session s11b)

This file previously vetted the distance-0 body built on
`s16 dmg = *((s16 *)(arg0 + 0x270)); if (dmg >= 4) { dmg = 3; } idx = dmg * 2;`.

That construct is now a DRIVER-ENFORCED BANNED CONSTRUCT for this function (layer-1
cheat-reviewer FAILs of 2026-08-25 23:08 and 23:29), and so is the 2026-08-25 23:20
decisions.md entry that the old vet cited as its authorization. The vet is therefore VOID
and must not be reused, adapted, or cited by any later session.

Session s11b returned `owner-gated`, not `candidate-ready`, so no self-vet is required:
there is no diff to vet — `src/code6cac.c` is left at
`INCLUDE_ASM("asm/funcs", func_8001F938)`, and `memory/grind/func_8001F938/candidate.c`
holds the clean floor-8 form (zero constructs from any coercion family, sanctioned or not).

Disposition: docs/grind/decisions.md, 2026-08-25 entry "func_8001F938 — OWNER-ESCALATION —
RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE".

If a future owner ruling ever moves the frozen signedness-split family, write a FRESH vet
against the body in `memory/grind/func_8001F938/rejected/layer1-fail-0825-2329.c`, citing
that new ruling — do not resurrect this one.
