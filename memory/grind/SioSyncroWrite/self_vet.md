# SELF-VET — SioSyncroWrite  (s5, 2026-08-19, forensics)

Scope note: this session's outcome is `owner-gated` (INTEGRATION HANDOFF), not
`candidate-ready`; src/main.c was restored to HEAD. This vet documents the
banked body in candidate.c for the operator's layer-2 pass.

CONSTRUCTS: extern volatile s32 D_800F1AEC (file-scope declaration change);
volatile s32 *flag = &D_800F1AEC; volatile s32 *st = flag; volatile u16 *p_ae2;
volatile s32 *p_af8; volatile s32 *p_af4b; volatile s32 *remaining;
volatile s32 *p_af4; `for (;;)` + `break` outer loop; `if (D_800F1AF4 == 0)
goto done;` top test.

## T1 semantic purpose
- `extern volatile s32 D_800F1AEC`: yes, byte-observable. hypotheses.md [s5-F1]
  proves via the cc1 `.rtl` dump that MEM_VOLATILE_P is what makes
  expand_increment emit the store-then-reload that target contains twice
  (asm/funcs/SioSyncroWrite.s:106-117); [s5-K1] measures the non-volatile body
  at 154/159 insns. It is also semantically required: HandleSio (the IRQ8/SIO
  ISR) writes the word at 0x8008CCD4.
- `flag` / `st`: the LIBCOMB control-block BASE pointer. Target itself holds it
  in $s3 and indexes 0x4/0x8/0xC. Their `volatile s32 *` type is now simply the
  type of `&D_800F1AEC` — nothing is added. Dropping `st` costs 1 insn and 51
  differing lines ([s4-M5]).
- `p_ae2`/`p_af8`/`p_af4b`/`remaining`/`p_af4`: each measured load-bearing
  ([s4-M1..M4], [s4-H1/H2]); each carries a /* FAKE */ annotation.
- loop shape / top test: ordinary control flow, no annotation needed.

## T2 human-programmer
Yes. Sony's LIBCOMB source declares an IRQ-shared SIO control block volatile and
holds its base in a local pointer; the body reads as a plain synchronous
byte-blaster loop. Nothing in it prompts "why is this here?" on semantics — the
volatile qualifier is answered by "an interrupt handler mutates it", which is
true and cited.

## T3 GCC-internals justification
The volatile declaration is justified by PROGRAM SEMANTICS (ISR-mutated shared
state), not by a pass. The RTL/expand_increment analysis in [s5-F1] is
CORROBORATION that the original source must have been volatile — it explains the
target bytes, it is not the reason the construct is written. The five pointer
aliases DO rest on a pass mechanism (combine.c symbol fold) and are therefore
/* FAKE */-annotated under the sanctioned pointer-alias family, with their
lever-exhaustion measurements recorded.

## T4 permuter/search provenance
No construct here came from permuter output. s4's 63.6k-iteration campaign
returned zero finds; the body came from structural reasoning, and this session's
change came from a dump read.

## T5 family check
- `extern volatile s32 D_800F1AEC;` — the `extern volatile T G;` on IRQ-touched
  game-state globals family, whose gate is an entry in
  volatile_extern_allowlist.txt. THE ENTRY DOES NOT YET EXIST, which is exactly
  why this session returns owner-gated instead of submitting.
- The five single-global pointer aliases — the sanctioned C-level-pointer-alias
  family, /* FAKE */-annotated with named mechanism + exhaustion refs.
- Explicitly NOT used: the banned `volatile s32 *flag` over a non-volatile
  extern, any `(volatile T *)` cast, register pins, inline asm, dead locals,
  scheduling barriers.

## T6 naming-announces-intent
No `pad`/`dummy`/`unused`/`spill`/`tmp` names. `flag`, `st`, `retries`,
`pkt_len`, `i`, `cb`, `remaining` describe the values; the `p_*` aliases are
named for the symbol they alias and every one of them is read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: C-level pointer alias to a global (/* FAKE */, combine symbol-fold)
  SCOPE: "Each future proposed exception must clear its own SOTN-master-branch evidence bar (mirroring the 2026-06-02 borderline-research methodology, not blanket sanction). Agents must NOT generalize from any single sanctioned exception to treat the broader category as relaxed."
  PRECEDENT: .claude/rules/no-new-park-categories.md:172
  FAMILY: extern volatile T G; on IRQ-touched game-state globals (two-prong gate)
  SCOPE: "The DEFAULT posture for `extern volatile T D_xxxxxxxx;` is STILL FORBIDDEN. Adding a symbol to this file is an EXCEPTION grant, NOT a relaxation."
  PRECEDENT: volatile_extern_allowlist.txt:45
  STATUS: CLAIMED BUT NOT YET GRANTED for D_800F1AEC — the grant is the handoff.

ANNOTATION-CONFORMANCE: five /* FAKE: <what>, mechanism: combine.c symbol-fold
defeat ..., lever-exhaustion: hypotheses.md [s4-M1..M4]/[s4-H1/H2] */ lines, one
per pointer alias, as written in candidate.c. `flag`/`st` are deliberately NOT
annotated: they are the real control-block base pointer, which target holds in
$s3 — genuine program structure, not a codegen device.
