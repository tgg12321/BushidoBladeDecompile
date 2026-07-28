# GRINDER CIRCUIT-BREAK — 2026-07-27 18:58

**Reason:** 3 consecutive invalid sessions on func_80052B44

git HEAD: 2d4ad890
git status:
```
M  metrics/events.jsonl

```
Last 20 log lines:
```
[grind 2026-07-27 15:45:31] func_80044098: progress applied — floor=13, 'recon s1: allocno mechanism fully quantified (counter pri 26250 vs pointer 21176); counter-split axis measured dead via combine guard-fold; frontier = pointer-side byte-neutral ref lift'
[grind 2026-07-27 15:45:55] func_80044098: session 2 starting, modality=structural
[grind 2026-07-27 16:14:45] func_80044098: judge FAILED the candidate — constraint banked, grind continues.
[grind 2026-07-27 16:14:46] func_80044098: session 2 starting, modality=structural
[grind 2026-07-27 16:59:55] func_80044098: progress applied — floor=3, 'structural s2: floor 13 -> 3 — peel+hdr-split flips v1/a0, in-arm const-holder fixes v0/a1/a2; residual = 3-insn sched1 li-placement stub'
[grind 2026-07-27 17:00:19] func_80044098: session 3 starting, modality=structural
[grind 2026-07-27 17:34:19] func_80044098: progress applied — floor=3, 'structural s3: stub root-caused (sched.c birthing_insn_p LAUNCH boost, knob = reg_n_sets!=1); pY hdr-borrow kills the stub entirely (3 @ 26/26, exact target structure); both holder families proven 3-locked by measurement + borrow-host enumeration'
[grind 2026-07-27 17:34:43] func_80044098: session 4 starting, modality=permuter
[grind 2026-07-27 17:57:10] func_80044098: judge ruling FAIL recorded.
[grind 2026-07-27 17:57:33] func_80044098: session 4 starting, modality=permuter
[grind 2026-07-27 18:32:43] func_80044098: OWNER-GATED — parked pending owner ruling (docs/grind/decisions.md — '2026-07-27 — func_80044098 (src/text1a_c.c) — OWNER-ESCALATION' (line 1789): endgame-lock disposition per the 2026-07-27 standing both-gates-fail auto-ruling (OWNER-ACCEPTED INCOMPLETE, terminal park, re-attempt eligible), following the 17:57 Judge ruling that both gates fail).
[grind 2026-07-27 18:32:45] func_80052B44: seeded ledger from memory/wip checkpoint.
[grind 2026-07-27 18:32:45] func_80052B44: session 1 starting, modality=recon
[grind 2026-07-27 18:41:51] func_80052B44: judge ruling PASS recorded.
[grind 2026-07-27 18:42:16] func_80052B44: session 1 starting, modality=recon
[grind 2026-07-27 18:47:40] func_80052B44: SCOPE VIOLATION —  M inline_asm_canonical.txt — session discarded.
[grind 2026-07-27 18:48:04] func_80052B44: session 1 starting, modality=recon
[grind 2026-07-27 18:53:08] func_80052B44: SCOPE VIOLATION —  M inline_asm_canonical.txt — session discarded.
[grind 2026-07-27 18:53:29] func_80052B44: session 1 starting, modality=recon
[grind 2026-07-27 18:58:55] func_80052B44: SCOPE VIOLATION —  M inline_asm_canonical.txt — session discarded.
```

## RESOLVED — 2026-07-27 (operator)

Root cause: func_80052B44 is a GTE leaf (LIBGTE-style SetRotMatrix; canonical
gate ASM-PARTIAL, 8/14 ctc2). The Judge GRANTED canonical-body authorization
(decisions.md 18:41 PASS) but sessions cannot edit inline_asm_canonical.txt
(outside their allowed surface) — each attempt tripped the scope check, three
discards, circuit-break. Operator applied the judge-authorized completion per
the gte-wrapper auto-authorize policy (2026-05-26): glabel canonical body (with
.set reorder/at restore tail — see canonical-asm-authorization-recipe gotcha),
inline_asm_canonical.txt entry, fill_delay rule retired, SHA1 == oracle,
queue done → COMPLETED-INLINE-ASM-CANONICAL. Grinder relaunched.
