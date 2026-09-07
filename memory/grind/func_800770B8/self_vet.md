# SELF-VET — func_800770B8

CONSTRUCTS: (1) `do { } while (0);` empty wrap in the prologue [FAKE-annotated];
(2) `p_old = prev;` same-value dead store to a local [FAKE-annotated];
(3) `sym = (u8 *)&D_800A35D0;` same-value dead store to the local `sym` inside the
`(arg2 & mask)` arm [FAKE-annotated]; (4) `u8 *sym;` — an ordinary named pointer
local holding the real, consumed base address `&D_800A35D0` (`dp = sym + (t0 * 4);`);
(5) the two byte-neutral caller-side edits the ledger has carried since s1 (prototype
`s32 func_800770B8(s32, s32, s32);` and call site `(s32)&D_8009BD24`).

## T1 semantic purpose:
(1) none beyond codegen — declared FAKE. (2) none beyond codegen — declared FAKE;
it restores `p_old`'s pre-call value, which is never read again. (3) none beyond
codegen — declared FAKE; `sym` already holds that value and is not read afterwards.
(4) REAL: `sym` is the base of the D_800A35D0 pair store and its value is consumed
by `dp = sym + (t0 * 4);` and thence by both `sh` instructions — it materialises in
the target's bytes at rows 49/50/51. (5) REAL: the target passes the address as a
scalar and the callee's return value is used; the declaration is the one the bytes
demand (banked in the ledger since s1, byte-neutral).

## T2 human-programmer: (4) yes — naming the base of a two-store pair is ordinary.
(5) yes — it is the prototype the call site actually needs. (1), (2) and (3) — no;
a reader would ask "why is this here?", which is exactly why each carries a
`/* FAKE: ... */` block naming what it is, the GCC pass it works through, and where
the lever-exhaustion is recorded. They are declared match devices, not disguised
program logic.

## T3 GCC-internals justification: yes, and it is stated openly in each annotation —
(1) sched.c second-pass scheduling-region bounds; (2) `local-alloc.c:472`
(`combine_regs`, `reg_n_deaths == 1`); (3) `loop.c:3040-3041`
(`count_loop_regs_set` setting `may_not_move` for a pseudo set in two basic blocks)
plus `loop.c:649` (`scan_loop` skip). Each is a sanctioned family whose whole
purpose is a codegen effect, so a named-pass mechanism is a REQUIREMENT of the FAKE
template here, not a concealed rationale. Constructs (4) and (5) rest on program
logic alone.

## T4 permuter/search provenance: none of the five came from the permuter. (3) was
derived by reading `tools/gcc-2.7.2/loop.c:3040-3041` after s38's pass
re-attribution named `move_movables` as the displacing pass, and was predicted
before it was measured; the s39 sweep then confirmed it (g1 = 0) and the controls
(g1d = 18, g6 = 18, h1/h2/h3 = 41/54/51) show it is the specific mechanism and not
a lucky spelling. (1) and (2) are inherited, previously-measured ledger units,
re-ablated this session (g1a = 2, g1b = 5, g1c = 7).

## T5 family check: (1) do-while(0) family. (2) and (3) dead-store family — both are
same-value stores to LOCALS, both annotated, both after documented exhaustion.
(4) is an ordinary named pointer local carrying a real consumed value. (5) is a
declaration correction. No register pin, no `__asm__`, no scheduling barrier, no
volatile coercion, no unused array, no empty-`if`, no `if (1)`, no goto pad, no
alias rename, no width-cast padding, no linker reorder. Nothing here reaches a
family outside the frozen list.

## T6 naming-announces-intent: `sym`, `dp`, `ap`, `ptr`, `base`, `p_old`, `prev`,
`q`, `p`, `idx`, `mask` — every name describes the value it holds. There is no
`pad`, `dummy`, `unused`, `spill`, `slack`, `_buf`, `tail` or `_frame_pad`. `sym`
is read (`dp = sym + (t0 * 4);`) as well as written, so it is not a
declaration-only / address-of-only holder.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: dead store / self-assignment to a LOCAL (constructs 2 and 3)
  SCOPE: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): a dead store / self-assignment to a LOCAL or PARAMETER, annotated `/* FAKE: ... */`, is allowed as a LAST-RESORT codegen lever after documented lever-exhaustion. SOTN ships `dest = val1; // fake`, `idxSub = idxSub;`; oot ships `rtile = rtile; // Fake match?`. Un-annotated dead stores remain forbidden; register pins remain forbidden."
  PRECEDENT: docs/reference/sotn-construct-index.md:65

  FAMILY: do { ... } while (0) wrap (construct 1)
  SCOPE: "SANCTIONED (owner ruling 2026-07-06, supersedes the 2026-06-04 mechanism-scoping): `do { ... } while (0);` (any body, incl. empty) is an allowed pure-C match device for ANY codegen effect incl. register allocation, with mandatory inline FAKE annotation; nested wraps need a single-level-insufficient justification. Hard line unchanged: no regfix/pins/inline-asm/semantic-lie C."
  PRECEDENT: docs/reference/sotn-construct-index.md:74

ANNOTATION-CONFORMANCE:
  /* FAKE: empty do-while(0) wrap. Effect: it anchors a NOTE_INSN_LOOP_BEG/END pair ... mechanism: GCC 2.7.2 sched.c list scheduler, second pass (sched2, post-reload); ... lever-exhaustion: hypotheses.md classes A/B/C; s3 (12 statement orderings), s5 (honest-loop fence hunt, +11 insns), s9 (exhaustive 3234-atom sched_solver depth-1 sweep ...), s10 (struct-typed rederive 178 insns), s11 (63-position single-wrap sweep + 18 nested-wrap variants). */
  /* FAKE: same-value dead store restoring p_old's pre-call value (p_old is never read again). Effect: it denies local-alloc's combine_regs its reg_n_deaths == 1 precondition on the p_old pseudo ... mechanism: GCC 2.7.2 local-alloc.c:472 (combine_regs / block-quantity grant gated on reg_n_deaths == 1). lever-exhaustion: hypotheses.md class B, s1-s31 (31 sessions of store-base spellings), re-measured negative on three differing chassis in s37 (k1-k6) and ablation-confirmed load-bearing here in s39 (removing it costs 2 points: g1 0 -> g1a 2). */
  /* FAKE: same-value dead store re-establishing sym's own value (sym is never read after the loop body's D stores). Effect: it gives the sym pseudo a SECOND set, in a different basic block from its first, which is what keeps the lui %hi/addiu %lo pair for D_800A35D0 inside the outer loop ... mechanism: GCC 2.7.2 loop.c:3040-3041 (count_loop_regs_set sets may_not_move[regno] when a set is the first in the current basic block but the reg was already set in the loop, i.e. it is set in two basic blocks); scan_loop then skips the insn at loop.c:649, so move_movables never sees it. ... lever-exhaustion: 38 prior sessions, 8 modalities, 188 banked rejected forms, 33,926 permuter iterations; ablation this session shows removing it costs 18 points (g1 0 -> g1d 18) and that no real-valued second write substitutes for it (h1 41/178, h2 54/170, h3 51/178) nor does a literal self-assign (g6 18). */
  All three carry what + named GCC pass + lever-exhaustion pointer, verbatim in
  memory/grind/func_800770B8/candidate.c (== the applied src/text1b.c body).
