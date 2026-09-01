# SELF-VET — func_8002EA24

Session 21 (rederive).  `sandbox func_8002EA24 --disable all` = **0** at 104/104
insns, 0 rules; full build `verify-oracle` = `62efab4f73f992798c43e8c730aa43baa10bb4fa`
== the locked oracle.  `canonical func_8002EA24` = ASM-PARTIAL, 8/104 insns, all
four reasons "GTE/cop2 op".

CONSTRUCTS: (1) three canonical GTE cop2 `__asm__ volatile` islands (lwc2 x2 +
mvmva `.word 0x4A486012`; swc2 x3; mtc2 $30 / swc2 $31 LZCS); (2) `vin` / `vout`
pointer locals that compute the island operand addresses in C and bind them via
`%0`; (3) ordinary named locals `x`, `z`, `sq`, `a0_var`, `neg_threshold`,
`lzcr`, `shift`, `tbl`, `sp_var`, `min_y`, `max_y`, `y_low`, `y`; (4) the final
two bound checks written as one `||` short-circuit condition.

NOTE ON WHAT IS **ABSENT**.  Every coercion-class construct the previous 20
sessions' candidates carried has been deleted and measured unnecessary on this
chassis: L3 (staged boolean through `a0_var`) went in s18; L2 (`max_y` reused to
carry the rotated X) went in s20; L1 (`{ z = 0; return z; }`, a dead local
borrowed to carry the return constant) goes in THIS session, replaced by the
merged `||` test.  The v5 "preference-donation" carrier (`threshold = x * x;`,
the parameter reused as a named intermediate) is also gone.  This body claims no
FAKE-annotated family because it contains no construct from any of them.

## T1 semantic purpose
- GTE islands: yes.  `mvmva` / `lwc2` / `swc2` / `mtc2 $30` / `swc2 $31` are cop2
  opcodes with no C spelling whatsoever; without them the function computes
  nothing (no rotation, no leading-zero count).  Pure semantics, not codegen.
- `vin` / `vout`: yes.  They ARE the operand values the islands consume; the
  `"r"` constraint needs an rvalue and the address arithmetic is done in C
  precisely so the template does not hardcode one.  Removing them removes the
  island's input.
- Every named local: yes.  `x` and `z` are the rotated coordinates (read by two
  bound tests each and by their own `mult`); `sq` is the squared planar distance
  (compared against `r_sq`, then subtracted from it); `a0_var` is the remaining
  squared distance and then its square root; `neg_threshold` is the lower bound
  used by four comparisons; `lzcr` / `shift` / `tbl` are the LZC table-lookup
  intermediates; `sp_var` is the island's `"=m"` output, read on the next line;
  `min_y` / `max_y` / `y_low` / `y` are the vertical-extent values.  Delete any
  one and the function stops computing its result.
- The merged `||`: yes.  It is the function's rejection condition; it decides
  the return value.
- There is NO construct in this diff whose removal leaves behaviour identical.

## T2 human-programmer
Yes for all of them, and this is the point of the session.  A human writing an
object-vs-cylinder overlap test writes exactly
`sq = x*x + z*z; if (r_sq < sq) return 0; a0_var = r_sq - sq;` — two named
quantities because they mean two different things (the squared distance, and the
distance left over).  It would in fact be ODD to re-store the second value into
the variable holding the first.  Likewise the two vertical bound checks are the
same kind of test as the two horizontal ones this function already writes as
`if (x < neg_threshold || threshold < x) return 0;`, so writing them as
`if (max_y < y - a0_var || y + a0_var < min_y) return 0;` is the locally
consistent spelling, not a device.  Nothing here makes a reader ask "why is this
here?": there is no variable holding a second unrelated value, no store that is
never read, no re-store of a value that is already there, no cast that changes
nothing, no empty statement, no unused declaration.

## T3 GCC-internals justification
The candidate.c header DOES explain, in GCC terms (`expand_preferences`,
`prune_preferences`, `regs_someone_prefers`, `find_reg`), WHY the split spelling
also happens to land the target's register assignment — that explanation is the
session's evidence and is what the ledger is for.  But the internals are not the
JUSTIFICATION for the construct: the program logic fully explains both edits on
its own (two distinct quantities get two names; two like tests get the like
spelling), and each construct passes T1/T2 with no reference to a compiler pass.
Contrast the construct this session DELETED: `{ z = 0; return z; }` had no
program-logic explanation at all — its only account was "it stops jump.c folding
the 0/1 diamond", which is precisely the cheat signal.  That one is gone.

## T4 permuter/search provenance
No permuter was run this session and no auto-search produced these forms.  The
two edits were derived by reading GCC 2.7.2's `global.c` in this repo (lines
828-871 `expand_preferences`, 876-935 `prune_preferences`, 995-1030 `find_reg`)
and predicting, before building anything, which spelling would satisfy
`expand_preferences`' non-conflict requirement.  A hand-written 8-body sweep then
confirmed the prediction on the first try (r1), and a second sweep tested the
ordinary alternatives to L1.  Both sweep scripts, all inputs and both logs are
banked under `tmp/grind/func_8002EA24/s21/`.  Neither construct is "necessary
only because a search found it": both are the spelling a reader would defend from
the source alone, which is why the L1-replacement sweep deliberately kept looking
after `L1h` (a `min_y`-borrowed variant, score 0 but the SAME borrowed-local
family as the L1 it would have replaced) and used `L1n` instead.

## T5 family check
No forbidden family is present, by shape, by analogy, or by respelling:
no register-asm pin (the islands name no allocatable register in a constraint;
they clobber `$12`/`$t4`, the scratch the island itself writes, exactly as the
in-tree accepted siblings do); no hardcoded-`$N` injection recreating target
bytes (the islands contain only cop2 opcodes and their own `$t4` staging — every
GPR-visible instruction in this function is compiler-emitted); no scheduling
barrier; no volatile coercion of any kind (no `volatile` on any global, no
`(volatile T *)` cast, no `asm("sym")` alias); no unused local array or frame
pad; no dead-param-assign; no dead conditional store; no empty-body `if`; no
`if (1)`; no dead goto label; no DImode chain; no accumulator + shared-label
trick; no param-local-alias declaration-order trick; no `s32 one = 1;`; no
lowercase `asm(...)` block; no build-time assembly rewriting; no redundant width
cast; no linker-script reorder.  It also contains none of the SANCTIONED
families — no variable reuse, no opaque arithmetic variable, no sub-word param
read, no mixed exit forms, no duplicated read into arms, no `do {} while (0)`,
no dead store or self-assign, no constant holder, no pointer alias to a global,
no duplicated statement into arms, no written-never-read array — which is why
this vet claims only the canonical-inline-asm category below.
BANNED_CONSTRUCTS check: the Judge constraint on this function bans adding
func_8002EA24 to `inline_asm_canonical.txt` (not done — that file is untouched),
bans retiring regfix rules (none exist project-wide; `rules_dropped` = 0), and
prescribes the exact island shape used here.

## T6 naming-announces-intent
No name in this diff is `pad`, `_pad`, `dummy`, `unused`, `spill`, a `sp_*`
frame device, `_buf`, `tail`, `slack`, or `_frame_pad`.  `sp_var` is inherited
from the accepted func_800274BC island (where it is `sp_tmp`) and names the
stack temporary that the `swc2 $31, 0($t4)` island writes and the very next
statement reads — it is an output operand, not a pad, and it is read.  `a0_var`
is a legacy name from the s4 ledger describing the register the value lands in;
it is a real, four-times-read value (remaining squared distance, then its square
root) and could be renamed `d` with no byte change — it is not a coercion
device, and nothing about it is address-of'd, discarded, or unused.  Every other
name (`x`, `z`, `sq`, `neg_threshold`, `min_y`, `max_y`, `y_low`, `y`, `lzcr`,
`shift`, `tbl`, `vin`, `vout`) describes the quantity it holds.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: canonical inline asm (GTE/cop2 opcodes with no C form)
  SCOPE: "Inline `__asm__` in a C function body using opcodes that ONLY EXIST in asm form: GTE coprocessor ops (`ctc2`/`mtc2`/`mfc2`/`lwc2`/`swc2`/`.word 0x4XXXXXXX`), BIOS vector jumps (`j 0xA0`/`B0`/`C0`), cache/DMA register pokes (`0x1F8003xx`). Authentic — original devs wrote these."
  PRECEDENT: src/code6cac_b.c:869

  (That is the vector/mvmva island of the MATCHED in-tree twin func_8002D320 —
  byte-for-byte the same template, constraints and clobber list as the two used
  here; its swc2 island is `src/code6cac_b.c:878`.  The LZCS island's accepted
  form is `src/code6cac_b.c:293`, func_800274BC, user-authorized 2026-06-10.
  All three already sit inside the tree that builds to the locked oracle SHA1.
  The scope sentence above is quoted verbatim from
  `.claude/rules/inline-asm-policy.md:80`.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.  This body contains no construct
from any FAKE-requiring family (see T5); the only claimed family is canonical GTE
inline asm, which mandates no annotation.  The islands nonetheless carry
explanatory comments naming their accepted in-tree source.

DISPOSITION NOTE (not a claim; for the operator/driver).  Because 8 of the 104
insns are canonical GTE cop2 ops, `canonical` routes this function ASM-PARTIAL,
and the completion bucket (COMPLETED-C vs COMPLETED-INLINE-ASM-CANONICAL, which
would require an `inline_asm_canonical.txt` line this session is forbidden to
write) is the operator's call.  The bytes are proven either way.
