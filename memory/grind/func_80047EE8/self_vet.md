# SELF-VET - func_80047EE8 (grind s12, 2026-08-20)

CONSTRUCTS: `volatile u32 pre_pad[8];` (unwritten leading frame pad, /* FAKE: */ annotated);
`arg0 = 0;` (dead store to a PARAM, /* FAKE: */ annotated). Everything else in the body is
ordinary live C: one walking pointer `p`, a `saved` base copy, sub-word `*(u16 *)` reads of the
walked table, an `s32 first` precompute consumed by the call, and a bottom-tested do/while.

## T1 semantic purpose
- `pre_pad[8]`: none by itself - it is a FAKE construct and is declared as such. Its purpose is to
  reconstruct the ORIGINAL function's frame: target `.frame $sp,72` allocates a 32-byte vars region
  0x18-0x37 that the target's own code never touches (zero sw/lw in that range - evidence.md
  [s6]/[s7]). The honest reading is that the original Marionation source declared a ~32-byte local
  aggregate that GCC 2.7.2 DCE'd but function.c never reclaimed; the same shape recurs across all
  four members of the cluster with identical size. This is precisely the situation the 2026-08-18
  phantom-frame-slot family exists for.
- `arg0 = 0;`: none by itself - also FAKE-annotated. It is the sanctioned dead-store-to-a-PARAM
  lever, with lever-exhaustion discharged at s2 (6 pure spellings of the init chain measured dead
  on this body, banked in rejected/pure-*.c).
- All other statements emit instructions that appear 1:1 in target (53/53 stream identity since s1).

## T2 human-programmer
Yes for the body: a table walker that reads a packed record (word + four halfwords) and calls the
draw routine per record is exactly what a 1998 sound/effect helper looks like, and the pointer
arithmetic mirrors the target's register flow. NO for the two FAKE constructs - a reader would ask
"why is this here?", which is why both carry an explicit /* FAKE: ... */ annotation naming what,
mechanism, and lever-exhaustion, rather than being smuggled in as apparently-live code.

## T3 GCC-internals justification
Both FAKE constructs are justified by named GCC passes, and both are declared as such:
`function.c assign_stack_local` at RTL-expand (frame reservation from a source DECL, never
reclaimed after DCE) for the pad; `cse2` canonical-register substitution over the {arg0, p, saved}
equivalence class for the dead store. Under the standing rulings this is REQUIRED (the FAKE
template mandates a named mechanism), not disqualifying - the disqualifying case is an
UNANNOTATED construct outside a sanctioned family whose only explanation is a GCC internal. No
other construct in the diff has a GCC-internals explanation; the rest is program logic.

## T4 permuter/search provenance
The composite was NOT found by search. Permuter campaigns (s4: 9,451 iters; s5: 17,057 iters) are
banked as KILLS: the only frame-growth lever they ever found was an 8-byte volatile SCALAR pad,
which is the WRONG SIZE (target needs 32) and was correctly rejected as a cheat
(rejected/permuter-volatile-*.c). The pad here comes from the owner's 2026-08-18 general family
plus the s6/s7 frame forensics that pin the original aggregate at 7-8 words; the element count
(8 -> 32 bytes) is read off the cc1 size-pin probe, not off a search score.

## T5 family check
- `volatile u32 pre_pad[8];` -> **Phantom-frame-slot volatile pad local** (no-new-park-categories.md
  2026-08-18 general family). All FORM CONSTRAINTS met: ARRAY form, first-declaration position, no
  `(void)pre_pad;` shim, `volatile`-qualified, FAKE annotation. All PREREQUISITES met: frame
  forensics (evidence.md [s6]/[s7] - zero sw/lw in 0x18-0x37, decisive negative on the reload-spill
  hypothesis, positive control reproducing vars=32, cluster-wide invariance, size-pin calibration);
  honest producers measured inert first (s3 nine-variant .frame grid, s4/s5 permuter, s8/s9
  rederive). This construct explicitly REPLACES the pre-migration `s32 unused_slack[8];` +
  `(void)unused_slack;` cheat, which is the forbidden spelling of the same intent.
- `arg0 = 0;` -> **dead-store-fake-exception** (dead store to a PARAM). Prerequisite discharged s2.
- No register pins, no `__asm__` of any kind, no alias renames, no volatile coercion of a global,
  no regfix/asmfix rule, no scheduling barrier, no `(void)` discard shim.

## T6 naming-announces-intent
`pre_pad` is the exact identifier the engine allowlist and the three landed precedents use
(src/code6cac.c:1491, src/code6cac.c:1570, src/code6cac_c2.c:856) - it is the family's canonical
name, deliberately honest about what it is rather than disguised. The old `unused_slack` name is
deleted. No other local is named for intent (`p`, `saved`, `count`, `first`, `word` are ordinary).
Note: the annotation text deliberately does NOT contain the token `pre_pad`, so the engine's
find_unused_local_arrays reference check cannot be shielded by the comment (the hygiene finding
reported in the sibling's 2026-08-20 handoff entry is not exploited here).

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Phantom-frame-slot volatile pad local (owner ruling 2026-08-18)
  SCOPE: "**Phantom-frame-slot volatile pad local** (off-brief survey exhibit `src/st/sel/2C048.c:564` `volatile u32 pad; // !FAKE:` in an INCLUDE_ASM=0 file): an unused `volatile` pad local declared solely to reserve target's untouched stack bytes."
  PRECEDENT: .claude/rules/no-new-park-categories.md:390
  PRECEDENT: docs/reference/sotn-construct-index.md:101
  PRECEDENT: src/code6cac_c2.c:856

  FAMILY: dead-store / self-assign to a LOCAL or PARAM (owner ruling 2026-07-01)
  SCOPE: "a dead store or self-assignment written as ordinary C, targeting a **local variable or function parameter**, is a sanctioned last-resort matching lever - under the strict prerequisites below."
  PRECEDENT: .claude/rules/dead-store-fake-exception.md:12
  PRECEDENT: docs/grind/decisions.md:981

ANNOTATION-CONFORMANCE:
  /* FAKE: unwritten leading frame pad (phantom-frame-slot volatile pad local family, owner ruling 2026-08-18, .claude/rules/no-new-park-categories.md:390). Mechanism: GCC 2.7.2 function.c assign_stack_local reserves the array slot at RTL-expand from the source DECL and never reclaims frame_offset after DCE ... Lever-exhaustion: 9 structural .frame variants (s3), ~26,500 permuter iters across two distinct basins (s4/s5), forensics (s6/s7), rederive (s8/s9) ... SOTN-master precedent: volatile u32 pad; // !FAKE: at src/st/sel/2C048.c:564 */
  /* FAKE: dead store to a PARAM (dead-store-fake-exception family, .claude/rules/dead-store-fake-exception.md). Mechanism: defeats cse2's canonical-register substitution over the {arg0, p, saved} equivalence class so the second pointer binds addu $s0,$s2,$v0 rather than $a0. Lever-exhaustion: 6 pure spellings of this init chain measured dead on this body at s2 (rejected/pure-*.c). */
  Both carry what + named GCC-pass mechanism + lever-exhaustion pointer, in the landed
  src/code6cac_c2.c:855-856 form.

NOTE ON OUTCOME: this session returns **owner-gated (integration handoff)**, not candidate-ready.
`sandbox --disable all` prints 10 because the sandbox strips the sanctioned pad until
func_80047EE8 has its row in engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS, and engine/ is
outside a grind session's allowed surface. The bytes are proven by the full driver build:
SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (tmp/grind/func_80047EE8/s12/build_oracle.log).
