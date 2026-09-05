# SELF-VET — func_8003C714

CONSTRUCTS: (1) one /* FAKE */-annotated dead store `v = ((long long)*(s32 *)(src + 4) * 100) / 30;` inside the loop, overwritten by `v = *src;` in the same iteration; (2) split-init spelling of the three time conversions (`a = *(s32 *)(src + 4); a = a / 1800; dst[0x21] = a;` and the two like it); (3) loop-invariant base-pointer hoist `base = D_80106A58;` before the loop with `src = base + i * 8;` inside it; (4) the TU-local declaration change `extern s32 D_80106A58;` -> `extern u8 D_80106A58[24];`; (5) the un-punned global read `*((u8 *)s0 + 0x30) = D_80101ED2;` (was `*(u16 *)&D_80101ED2`); (6) inherited, unchanged: `*(s32 *)(src + 4)` field reads through the record base and `*(u16 *)buf` on the local `u8 buf[4]` filled by `func_8001CD68(buf)`.

## T1 semantic purpose
(1) NO independent semantic purpose in the emitted bytes — this is the declared FAKE construct. Its stored value is dead; it is present as a codegen lever. Its RHS is nevertheless a truthful quantity of this function's own data (the record's elapsed time, a 30 Hz frame count, expressed in hundredths of a second, with the x100 scaling done in 64-bit so it cannot overflow a 32-bit intermediate). The function itself converts that same frame count to minutes (/1800), seconds ((/30)%60) and hundredths ((%30)*100/30), so both the quantity and the 100/30 conversion are the program's own.
(2)-(6) all have ordinary semantic purpose and are observable in the emitted bytes: (2) is the same arithmetic written in named steps; (3) hoists a loop-invariant address the loop then indexes; (4) declares the object the loop actually walks (three 8-byte records) instead of a scalar whose address is cast; (5) reads the halfword global directly instead of through a cast; (6) reads the record's time field and the two bytes the callee wrote.

## T2 human-programmer
(1) A reader WOULD ask "why is this value computed and then thrown away?" — which is exactly why it is annotated /* FAKE */ and submitted under the dead-store exception rather than as ordinary C. What a reader would NOT ask is "what is this expression?": it is a plain integer conversion of the record's own time field, not a synthetic cast chain. (The float/long-long chain the Judge FAILed on 2026-09-05 — `v = (long long)((float)*(s32 *)(src + 4) / 30.0f);` — is NOT present; this session found a spelling with no float, no unnecessary conversion, and the same measured result.)
(2) Yes — split-init is how a C programmer writes a multi-step conversion, and is ordinary C by the owner's 2026-06 provisional sanction (split-init accumulation).
(3) Yes — hoisting an invariant base pointer out of a loop is routine.
(4) Yes — an object walked with an 8-byte stride and read at +0 and +4 for three iterations IS an array, not a scalar; the array declaration is the honest one and the cast disappears.
(5) Yes — reading a declared global by name is strictly more natural than casting its address.
(6) Yes.

## T3 GCC-internals justification
Only construct (1) has a GCC-internals mechanism, and it is stated in the annotation because the rule requires it, not because the program logic is missing: `loop.c:532` computes `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`, `prescan_loop` (`loop.c:2202`) sets `loop_has_call` for any CALL_INSN between the loop notes, and `flow.c`'s `libcall_dead_p` (`flow.c:1827`) deletes the whole REG_LIBCALL/REG_RETVAL block after `loop_optimize` and before `combine` and register allocation. Constructs (2)-(6) are justified by program logic alone; no GCC pass is cited for any of them.

## T4 permuter/search provenance
No permuter or auto-search output is in this diff. The construct was derived from compiler source (loop.c / flow.c / cse.c read in sessions s10-s18) and confirmed by directed measurement: ten hand-written spellings swept through `tmp/grind/func_8003C714/s18/sweepg.sh`, with the loop dump, the emitted `.frame` line and `sandbox --disable all` read for each. The s4 permuter campaign (66,016 iterations) never produced it.

## T5 family check
Construct (1) is the dead-store family: a store to a LOCAL whose stored value is never read (`v` is unconditionally re-assigned `v = *src;` before any read — store-level deadness, owner ruling 2026-08-31). It is not a register pin, not inline asm, not a scheduling barrier, not volatile coercion, not a frame pad, not an alias rename, not a dead local (v is a live, read local), and not an invented statement of a shape outside the frozen list. The Judge's 2026-09-05 ruling on this function is directly on point and binding: "A dead store whose RHS is itself ordinary C a reader can justify from the program's own quantities — and which happens to touch DImode — is INSIDE the family and is adjudicated on its prerequisites ... not as a new family". Constructs (2)-(6) are ordinary C and claim no family.

## T6 naming-announces-intent
No new names are introduced by the lever. `v` is the pre-existing loop scratch local that also carries the record's leading byte into `dst[0x24]`; `a`, `b`, `c` are the three conversion accumulators; `base`/`src`/`dst` are pointers that are all read. There is no `pad`, `dummy`, `unused`, `spill`, `slack` or `tmp` in the diff, and no local whose only use is a discard or an address-of.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: dead-store (FAKE-annotated dead store to a local)
  SCOPE: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): a dead store / self-assignment to a LOCAL or PARAMETER, annotated `/* FAKE: ... */`, is allowed as a LAST-RESORT codegen lever after documented lever-exhaustion. SOTN ships `dest = val1; // fake`, `idxSub = idxSub;`; oot ships `rtile = rtile; // Fake match?`. Un-annotated dead stores remain forbidden; register pins remain forbidden."
  PRECEDENT: .claude/rules/dead-store-fake-exception.md:69

ANNOTATION-CONFORMANCE:
  /* FAKE: dead store -- the record's elapsed time expressed in hundredths of a
   * second (the record holds 30 Hz frames; the x100 scaling is done in 64-bit so
   * it cannot overflow), stored into v and unconditionally overwritten by
   * `v = *src;` below.  mechanism: the DImode divide expands to a __divdi3
   * libcall block, whose CALL_INSN makes prescan_loop set loop_has_call
   * (loop.c:2202) and so halves the LICM hoist threshold at loop.c:532
   * (122 -> 61), which is what leaves the 0x91A2B3C5 division magic materialised
   * in-loop as the target's lui/lw/ori/mult quartet; flow.c's libcall_dead_p
   * (flow.c:1827) then deletes the whole block after loop_optimize and before
   * combine and register allocation, so it costs zero emitted bytes.
   * lever-exhaustion: memory/grind/func_8003C714/hypotheses.md K1-K54 and
   * rejected/ (30 disproven forms) over 18 sessions. */
  WHAT: the dead store and the value it holds. MECHANISM: named GCC passes — prescan_loop/loop.c:2202, loop.c:532, flow.c libcall_dead_p/flow.c:1827, with the deletion ordered before combine and register allocation. LEVER-EXHAUSTION: memory/grind/func_8003C714/hypotheses.md (K1-K54 across 18 sessions) and the 31 forms in memory/grind/func_8003C714/rejected/.
