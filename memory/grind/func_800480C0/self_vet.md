# SELF-VET — func_800480C0

Session 24 (synthesis), 2026-09-06. Body installed at src/text1b.c:129.
Measured THIS session: `sandbox func_800480C0 --disable all` -> `"score": 0`,
`target_insns` 74, `build_insns` 74, `rules_dropped` 0; `verify-oracle` -> `"ok": true`,
build/bb2.exe SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle
(`tmp/grind/func_800480C0/s24/verify_oracle.txt`).

CONSTRUCTS: (1) `volatile u32 pre_pad[8];` unwritten first-declared volatile pad array;
(2) `arg0 = 0;` dead store to a parameter. Nothing else: no asm, no register pin, no
scheduling barrier, no alias rename, no cast trickery, no dead conditional, no goto pad.

## T1 semantic purpose
(1) pre_pad: none semantically — it exists to reserve the 32 locals bytes the original
function reserves at sp+0x18..sp+0x37 and never touches. That is exactly what the
phantom-frame-slot volatile pad family IS; the family is sanctioned precisely because the
target's own frame carries an unobservable reservation that no live C value can produce
without also producing traffic (measured: every referenced producer costs >=1 store).
(2) arg0 = 0: no observable effect — `arg0` is dead after `base_addr = arg0;` and
`p = (u32 *)arg0;`. That is the definition of the sanctioned dead-store-to-a-PARAMETER
family, not a disguised pin. Both are declared FAKE rather than passed off as real logic.

## T2 human-programmer
No. A human writing this function from its specification would write neither line. Both
are declared as match hacks in their `// !FAKE:` annotations, with the what, the GCC
mechanism and the lever-exhaustion pointer, exactly as the two sanctioned families
require. The identical pair appears in the matched, committed sibling func_80047FBC
sixteen lines above (src/text1b.c:85 and :91) — this body is that body with 6 parameters
instead of 4.

## T3 GCC-internals justification
Yes, and that is required here rather than disqualifying: both families are codegen levers
whose rules MANDATE a named GCC-pass mechanism in the annotation. pre_pad works through
get_frame_size/expand_decl reserving declared locals (config/mips/mips.c:4443-4475);
`arg0 = 0;` works through cse.c canonical-register substitution, which without it folds
the {arg0, p, base_addr} equivalence class and emits one base copy where the target has
two. Neither is a register pin, an `__asm__` injection, or a scheduling barrier; neither
names a hard register, an insn slot, or a spill offset.

## T4 permuter/search provenance
Neither construct came from a permuter or auto-search. pre_pad came from frame forensics
(s18's $sp census over asm/funcs/func_800480C0.s finding zero sw/lw in sp+0x18..0x37, then
s19-s23 measuring every honest producer). `arg0 = 0;` came from an RTL read of the cse2
equivalence class in s6 and is the same construct already accepted at layer-2 in the
committed sibling. No detector-evasion spelling is involved: both are the canonical,
detector-VISIBLE spellings their rules name, which is why the engine strips pre_pad by
default and needs an explicit per-function allowlist row to score it.

## T5 family check
(1) Phantom-frame-slot volatile pad local — a sanctioned family
(.claude/rules/no-new-park-categories.md:422-434, owner ruling 2026-08-18, which
"Supersedes the per-function 2026-08-17/18 leading/trailing-pad carve-outs with a general
family"). Its FORM CONSTRAINTs are met verbatim: ARRAY form `volatile u32 pre_pad[8];`,
first-decl position, no `(void)pad;` shim, `// !FAKE` annotation, ledger frame-forensics
showing the slot genuinely untouched, honest producers measured inert first. Its
per-function engine row exists: engine/volatile_cheats.py:779, owner commit 661c01ef
(2026-09-06).
(2) Dead store to a PARAMETER — sanctioned family,
.claude/rules/dead-store-fake-exception.md, owner ruling 2026-07-01. The target is a
parameter (not a global, not memory), it is annotated, and it follows documented
lever-exhaustion.
Neither is an unsanctioned-family first reach; neither is a respelling of a banned
construct. The prior judge_constraint against the pad in this function is explicitly
narrowed and void per the 2026-09-05 ESCALATE ruling (docs/grind/decisions.md:24275) and
superseded by the owner's grant row.

## T6 naming-announces-intent
`pre_pad` does announce intent — deliberately, and that is the family's REQUIRED name
shape: engine/volatile_cheats.py:196 enumerates `pre_pad` among the recognised pad names
and the allowlist row is keyed on the literal tuple `("pre_pad", 8)`. Renaming it to a
neutral identifier would defeat the detector and void the grant, i.e. would be the actual
cheat. The other identifiers (`p`, `base_addr`, `count`, `new_var`, `sx_arg2..5`, `word`,
`a1v`, `a2v`, `a3v`, `v0v`) all name real values, all are read, none is discarded.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Phantom-frame-slot volatile pad local
  SCOPE: "an unused `volatile` pad local declared solely to reserve target's untouched stack bytes. Supersedes the per-function 2026-08-17/18 leading/trailing-pad carve-outs with a general family."
  PRECEDENT: .claude/rules/no-new-park-categories.md:422
  FAMILY: Dead store / self-assignment to a LOCAL or PARAMETER (FAKE-annotated)
  SCOPE: "a dead store / self-assignment to a LOCAL or PARAMETER, annotated `/* FAKE: ... */`, is allowed as a LAST-RESORT codegen lever after documented lever-exhaustion."
  PRECEDENT: .claude/rules/dead-store-fake-exception.md:6

ANNOTATION-CONFORMANCE:
  volatile u32 pre_pad[8]; // !FAKE: phantom-frame-slot volatile filler (owner ruling 2026-08-18, .claude/rules/no-new-park-categories.md:422-434; grant row engine/volatile_cheats.py:779, commit 661c01ef): the target reserves 32 locals bytes at sp+0x18..sp+0x37 that no instruction in asm/funcs/func_800480C0.s reads or writes; mechanism: GCC 2.7.2 get_frame_size/expand_decl reserves declared locals (config/mips/mips.c:4443-4475); lever-exhaustion: memory/grind/func_800480C0/hypotheses.md s1-s23, 104 rejected forms, every referenced producer costs >=1 store (flow.c:1740-1741 never deletes the last store to a frame object)
  arg0 = 0; // !FAKE: dead store to a PARAMETER (sanctioned dead-store family, .claude/rules/dead-store-fake-exception.md; identical construct in the matched sibling func_80047FBC at src/text1b.c:91). It defeats cse2's canonical-register substitution, which otherwise folds the {arg0, p, base_addr} equivalence class and emits one base copy instead of two; mechanism: GCC 2.7.2 cse.c canonical-reg substitution; lever-exhaustion: hypotheses.md s1-s3
  Both carry what + named GCC-pass mechanism + lever-exhaustion pointer, in the `// !FAKE:`
  spelling the pad family's FORM CONSTRAINT requires and the committed sibling ships.
