# SELF-VET — CD_sync

Diff surface: `src/system.c` ONLY (the `INCLUDE_ASM("asm/funcs", CD_sync);` line at
old line 377 replaced by the C body). No header, Makefile, `*.ld`, pipeline `*.txt`,
`engine/`, `tools/` or `.claude/rules/` change. No declaration-surface change at all —
the extern block already on main immediately above the function
(`D_800A125C[]`, `g_cd_status_a`, `D_800A11DC[]`, `D_800A11D5`, `D_800F19B8`,
`D_800F19BC`, `D_800F19C0`, `D_800161B8`, `D_800161C8`, `D_80016240`) is used verbatim.

Bytes: `sandbox CD_sync --disable all` = **score 0**, target_insns 160, build_insns 160,
rules_dropped 0, and `verify-oracle` ok:true with
build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (== oracle). Measured THIS session
with these edits in place in src/.

CONSTRUCTS: pointer alias `tbl_125c = D_800A125C`, pointer alias `idx_1494 = &g_cd_status_a`,
pointer alias `idx_1495 = 1 + idx_1494`, pointer alias `pp = &D_800F19C0`, named intermediate
`t0 = idx_1494[0]`, named address intermediate `pB = (s32 *)((idx_1494[1] << 2) + (s32)tbl_125c)`,
named intermediate `arg5 = *pB`, staged-value reuse of the copy-loop local `src` for the
chain-A address, `do { ... } while (0);` wrap around the do_timeout printf block,
`do { ... } while (0);` wrap around the CheckCallback/poll block, and the poll loop written
as a real `do { ... if (status == 0) break; ... } while (1);` (ordinary C, not a FAKE).

## T1 semantic purpose
- The four pointer aliases each hold a real address that is really dereferenced; deleting one
  changes nothing semantically but is NOT byte-neutral (C1 = 31, C2 = 29, A7 = 12, A5 = 18),
  so each is declared FAKE and annotated. They are second handles to real globals — the
  sanctioned pointer-alias shape, not a coercion.
- `t0`, `pB`, `arg5` each hold a real value that is consumed exactly once by the printf call
  and that appears in the target's own bytes (`lbu $a0,0($s2)`, `addu $v0,$v0,$s3`,
  `lw $v1,0($v0)` at asm/funcs/CD_sync.s:49/56/60). They have observable purpose: they are
  the arguments.
- `src` carries the chain-A address that becomes printf's 4th argument. It is a real value,
  staged through an existing local that is dead at that point and re-used later by the
  result-copy loop. Zero dead code.
- The two `do { } while (0);` wraps have no semantic effect. They are declared FAKE under the
  do-while-zero family, which is sanctioned for exactly that (ANY codegen effect incl.
  register allocation, owner ruling 2026-07-06).
- The `do { ... } while (1);` poll loop DOES have semantic purpose and is not FAKE: the
  function really polls `getintr()` until it returns 0. Writing it as a loop instead of a
  backward `goto` is the ordinary spelling of the loop the function performs, and it is the
  spelling the matched sibling CD_ready ships on main (src/system.c:566).

## T2 human-programmer
Yes for the semantic parts. A human writing libcd's `CD_sync` writes a `do { } while (1)`
polling loop, names the four base pointers it dereferences repeatedly, and names the values it
passes to a 5-argument `printf`. A reader would ask "why?" about the two `do {} while (0);`
wraps — which is why both carry a FAKE annotation naming the pass and the ablation
number, as the family requires. Nothing here is unreadable or nonsensical: the body is one
timeout loop, one diagnostic printf, one interrupt poll, one 8-byte result copy.

## T3 GCC-internals justification
The two do-while(0) wraps and the placement choices ARE justified by GCC internals
(flow.c:2081 loop_depth-weighted `reg_n_refs` feeding local-alloc.c:1660 `qty_compare_1` and
global.c's allocno priority sort; sched.c:2081 loop notes; sched.c:2462 `rank_for_schedule`;
calls.c:1652-1664 `expand_call`; sched.c:2505 `birthing_insn_p`). That is the cheat SIGNAL,
and it is precisely why each of those constructs is annotated FAKE with a named pass
and lever-exhaustion rather than presented as ordinary C. The program-logic constructs
(the poll loop, the named printf arguments, the aliases) are explained by the program.

## T4 permuter/search provenance
None. No permuter or auto-search ran this session. Every form was hand-derived as a transplant
of two ALREADY-MATCHED sibling bodies that ship on main (CD_ready, CD_datasync — same TU, same
PsyQ libcd bios.c v1.86 family, instruction-for-instruction the same do_timeout window) and
then reduced by single-construct ablation. Two constructs the transplant carried
(the `new_var = 0xFF` constant holder and the `tb` ready-byte intermediate) ablated
byte-neutral at 0/160 and are DELETED from the submitted body.

## T5 family check
Every construct maps to a frozen family (claims below). The BANNED `CD_alarm` aggregate merge
of 0x800F19B8/BC/C0 (decisions.md 2026-09-06 11:38), which this candidate carried from s117
through s125, is ABSENT — HEAD's three flat externs are used. The refused cross-symbol
arithmetic idiom (owner ruling 2026-07-20) is ABSENT — the old combine-foldable chain-extender
`(u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1` is deleted and replaced by
`idx_1495 = 1 + idx_1494`, an offset within ONE object: D_800A1494/95/96 are the three bytes of
a single `static volatile CD_intr Intr` (memory/closer/libcd-identity.md:28), the identical
idiom the Judge cleared on CD_ready (docs/grind/decisions.md:24881) and that ships matched on
main at src/system.c:515 and src/system.c:766. No canonical-asm, no inline asm, no register
pin, no volatile coercion (the volatile is the TU's own pre-existing
`extern volatile u8 g_cd_status_a;`, not a local qualifier or cast), no dead local, no pad.

## T6 naming-announces-intent
No `pad`/`dummy`/`unused`/`spill`/`slack`/`_buf` names. `t0`, `pB`, `arg5` are
value-descriptive placeholders inherited from the sibling bodies; `idx_1494`/`idx_1495`/
`tbl_125c`/`pp`/`src`/`saved`/`status` name what they hold. Every named local is read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: pointer-alias-fake-exception (tbl_125c, idx_1494, idx_1495, pp)
  SCOPE: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): a C-level local pointer alias (second handle) to a global — `Type* t = &g_Thing;`, `s16 (*p)[] = &D_xxx;`, FakePrim-style reinterpret views — is allowed as a LAST-RESORT lever with `/* FAKE: ... */` annotation + lever-exhaustion. SOTN idiom incl. self-admitted `// n.b.! unused, required for PSP`. asm(\"sym\") alias-RENAMES stay forbidden."
  PRECEDENT: .claude/rules/pointer-alias-fake-exception.md:5

  FAMILY: pointer-alias-fake-exception (on-main matched precedent, same TU, same window)
  SCOPE: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): a C-level local pointer alias (second handle) to a global — `Type* t = &g_Thing;`, `s16 (*p)[] = &D_xxx;`, FakePrim-style reinterpret views — is allowed as a LAST-RESORT lever with `/* FAKE: ... */` annotation + lever-exhaustion. SOTN idiom incl. self-admitted `// n.b.! unused, required for PSP`. asm(\"sym\") alias-RENAMES stay forbidden."
  PRECEDENT: src/system.c:546

  FAMILY: do-while-zero-exception (both wraps, single level each)
  SCOPE: "SANCTIONED (owner ruling 2026-07-06, supersedes the 2026-06-04 mechanism-scoping): `do { ... } while (0);` (any body, incl. empty) is an allowed pure-C match device for ANY codegen effect incl. register allocation, with mandatory inline FAKE annotation; nested wraps need a single-level-insufficient justification. Hard line unchanged: no regfix/pins/inline-asm/semantic-lie C."
  PRECEDENT: .claude/rules/do-while-zero-exception.md:6

  FAMILY: staged-value-reused-variable (src)
  SCOPE: "SANCTIONED 2026-07-03 — a real, immediately-used value staged through an existing (currently-dead) local to fix instruction order; FAKE-annotated, lever-exhaustion required; zero dead code"
  PRECEDENT: .claude/rules/staged-value-reused-variable.md:3

  FAMILY: named-intermediate declaration order (t0, pB, arg5)
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID. SOTN's `randy` chain in `src/weapon/w_037.c` is the same mechanism."
  PRECEDENT: .claude/rules/no-new-park-categories.md:204

  Six-prong check for the named intermediates: (1) once-written — yes, each of t0/pB/arg5 is
  assigned exactly once; (2) real value present in the target's bytes — t0 = `lbu $a0,0($s2)`
  (asm/funcs/CD_sync.s:49), pB = `addu $v0,$v0,$s3` (asm/funcs/CD_sync.s:56), arg5 =
  `lw $v1,0($v0)` (asm/funcs/CD_sync.s:60); (3) byte-neutral chassis — the submitted body is
  160/160 with build_insns == target_insns; (4) fresh, not borrowed — all three are
  block-scope locals introduced for this purpose (`src` is the ONE borrowed carrier and is
  claimed under staged-value-reused-variable, not here); (5) destination not
  live-pre-initialized; (6) FAKE annotation + per-construct lever-exhaustion numbers present
  on each line.

ANNOTATION-CONFORMANCE:
  All ten FAKE constructs carry an inline `/* FAKE: <what>, mechanism: <named GCC pass>;
  lever-exhaustion: <measurement> */` annotation at src/system.c:392, 393, 394, 417, 418,
  419, 420, 421, 422, 438. The exact lines (abridged only where marked "..."; the full text is
  in src/system.c and in memory/grind/CD_sync/candidate.c):
  - src/system.c:392 — `/* FAKE: pointer alias (second handle) to the CD_intstr table per pointer-alias-fake-exception (owner ruling 2026-07-01, the `Type* t = &g_Thing;` redundant-second-handle shape), mechanism: global.c seats the base in $s3 across the whole function as the target does (asm/funcs/CD_sync.s:16-17); lever-exhaustion: s126 ablation C1 (direct D_800A125C[] subscript) = 31/160, plus the 125-session ledger in memory/grind/CD_sync/hypotheses.md */`
  - src/system.c:393 — `/* FAKE: pointer alias (second handle) to the libcd Intr status block per pointer-alias-fake-exception (owner ruling 2026-07-01); the volatile is the TU's own declaration `extern volatile u8 g_cd_status_a;` ...; mechanism: global.c seats the base in $s2 across the poll loop as the target does (asm/funcs/CD_sync.s:18-19); lever-exhaustion: s126 ablation C2 (direct (&g_cd_status_a)[n] subscript) = 29/160 */`
  - src/system.c:394 — `/* FAKE: second handle (+1) into the same 3-byte Intr block per pointer-alias-fake-exception (NOT cross-symbol arithmetic ...), mechanism: global.c seats the ready-byte base in $s4 (`addiu s4,s2,1`, asm/funcs/CD_sync.s:20); lever-exhaustion: s126 ablation A7 (idx_1494[1] read at the use site) = 12/160, s105 (three honest respellings of the old chain-extender) = 15 */`
  - src/system.c:417 — `/* FAKE: named intermediate for the sync byte, placed BEFORE the wrap (loop depth 1), mechanism: flow.c:2081 loop_depth-weighted reg_n_refs feeds local-alloc.c:1660 qty_compare_1 ...; lever-exhaustion: s126 probe B3 (read inlined into the src address) = 14/160, the 125-session t0 ledger in hypotheses.md s118-s125 */`
  - src/system.c:418 — `/* FAKE: do-while(0) wrap per do-while-zero-exception (owner ruling 2026-07-06), mechanism: sched.c:2081 loop-note barrier on the first insn inside (the ready-byte address chain) orders the sync-byte load ahead of it and every later register-argument load after it, and flow.c loop_depth ref weighting seats tbl_125c in $s3; lever-exhaustion: s126 ablation A8 (wrap removed) = 25/160 */`
  - src/system.c:419 — `/* FAKE: named address intermediate (fresh, once-written, once-read, real value = `addu $v0,$v0,$s3` at asm/funcs/CD_sync.s:56), mechanism: rank_for_schedule INSN_LUID tie-break (sched.c:2462) ...; lever-exhaustion: s126 probe B1 (folded back into the arg5 load) = 7/160 */`
  - src/system.c:420 — `/* FAKE: chain-A address staged through the (dead-here) src copy-loop variable per staged-value-reused-variable (owner-sanctioned 2026-07-03), mechanism: the multi-set destination keeps the addu unboosted (birthing_insn_p sched.c:2505) ...; lever-exhaustion: s126 ablation A6 (fresh local `ta` instead of the reused src) = 8/160 */`
  - src/system.c:421 — `/* FAKE: named intermediate for the fifth (stack) argument (fresh, once-written, once-read, real value = `lw $v1,0($v0)` at asm/funcs/CD_sync.s:60), mechanism: calls.c store_one_arg ...; lever-exhaustion: s126 probe B2 (passed as *pB directly) = 9/160 */`
  - src/system.c:422 — `/* FAKE: pointer alias (second handle) to the alarm callback slot per pointer-alias-fake-exception (owner ruling 2026-07-01, the `Type* t = &g_Thing;` redundant-second-handle shape), mechanism: calls.c:1652-1664 expand_call precomputes a register argument whose rtx_cost > 2 into a pseudo inside a loop (preserve_subexpressions_p) ...; lever-exhaustion: s126 ablation A5 (direct D_800F19C0 read) = 18/160; the CD_alarm struct spelling ... is BANNED (decisions.md 2026-09-06 11:38) and is removed here */`
  - src/system.c:438 — `/* FAKE: do-while(0) wrap per do-while-zero-exception (owner ruling 2026-07-06 - sanctioned for ANY codegen effect incl. register allocation), mechanism: flow.c:2081 loop_depth-weighted reg_n_refs lifts the idx_1494 / idx_1495 / saved allocnos in global.c's priority sort so they take $s2/$s4/$s1 instead of $s1/$s6/$s2; single level is sufficient here (no nested wrap needed); lever-exhaustion: s126 ablation V1 (wrap removed) = 18/160, all 18 being callee-saved register substitutions on an otherwise order-exact 160/160 stream */`
  Neither do-while(0) is nested, so the single-level-insufficient prerequisite does not apply.
