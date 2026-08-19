# SELF-VET — func_8001B748

CONSTRUCTS: `struct AB` type definition + `((AB *)a)->hN` field accesses; `struct DST` type
definition + `((DST *)dst)->wN/hN/bN` field accesses; named intermediate `zval` (once-written,
once-read); ordinary named temporaries `inv_frac`, `inv_s1`, `base`, `dx`, `dy`, `dz`, `dd`, `cur`,
`t`, `v`, `use_high`, `sum`, `avg`.

## T1 semantic purpose
- **struct AB / struct DST + field accesses**: YES, real semantic content. These declare the memory
  layout of the two data objects the function operates on (an interpolation source record with
  s16 x/y/z at +4/+6/+8, and a 0x40-byte destination record). Every field access replaces a manual
  `*((s32 *)(dst + 0x18))`-style byte-offset cast with the equivalent typed member reference. The
  observable effect is identical addressing with a *declared* type for each slot — this is the
  normal way a C program describes a record, and it is what the original 1998 source must have
  done, since the offsets are dense, aligned and consistently typed.
- **`zval`**: holds the interpolated +8 (Z) product sum, computed once and consumed once at the
  `->w8` store. Its value is real and appears in the target's own bytes (`mflo t0` / `addu v0,a1,t0`
  / `sra v0,v0,12` / `sw v0,8(s0)`). Naming it is what places the second product's computation
  before the 0x12/0x10/0x14/0x18 stores instead of after them; inlining it (variant
  `u7_nonewvar`, banked in `rejected/struct-form-inline-second-product-no-zval-44.c`) scores 44.
- All other locals are ordinary temporaries holding values that appear in the target's bytes.
- There is NO construct in the diff that is byte-identical with or without itself. Every construct
  carries a value or a type declaration that the emitted code uses.

## T2 human-programmer
YES for every construct. Given only the specification ("interpolate a source record's x/y/z into a
destination record by a 12-bit fraction, seeding it on first use and easing toward it afterwards"),
a human programmer would (a) declare the two records as structs and access them by member — this is
strictly *more* natural than the byte-offset casts it replaces, and (b) name the interpolated Z sum
before storing the other fields. Nothing in the diff would make a reader ask "why is this here?".
On the contrary, this session's change makes the function read closer to plausible original source
than the inherited form did: it removed the s7 `pa` split (a construct that existed only for
codegen) and removed a no-op `& 0xFFFFFFFF` mask, and both removals were *required* by nothing —
they simply became unnecessary once the types were declared.

## T3 GCC-internals justification
The struct typing was **found** by reasoning about `sched.c:832-839` (the `MEM_IN_STRUCT_P` /
`rtx_addr_varies_p` dependence-suppression rule), and that mechanism is recorded honestly in the
ledger. But the mechanism is not the *justification* for the construct, and this is the distinction
the test asks about: the program-logic explanation stands entirely on its own. "These two pointers
point at records, so declare the records" needs no reference to any GCC pass, would be written the
same way by someone who had never heard of `sched_analyze`, and produces a file that is easier to
read than the one it replaces. A construct whose only defensible explanation is a pass name is a
cheat; this one's C-level explanation is the primary one and the pass explanation is incidental
colour. Note also that the construct is *not* pass-targeted in shape — it is applied uniformly to
every access of both objects across the whole function, not surgically placed at the one
instruction that moved.

## T4 permuter/search provenance
NO permuter or auto-search involvement. No permuter campaign was launched this session. The form was
derived by reading GCC 2.7.2's `sched.c` dependence predicates directly and then hand-writing three
typed variants (`t1` a/b only, `t2` dst only, `t3` both) with a scripted rewrite of the inherited
body. `t1` and `t2` both scored 2 (unchanged); only the full `t3` scored 0. Nothing here passes a
detector by spelling — the diff contains no `__asm__`, no `register ... asm()`, no `volatile`, no
alias rename, no dead store, no empty block, no `do {} while (0)`, no `goto`, and no unused local.

## T5 family check
NO forbidden family matches, by analogy or otherwise. Walking the catalog: no register-asm pins; no
hardcoded-`$N` asm; no regfix/asmfix edits of any kind (both files untouched); no scheduling
barriers; no INLINE_MOVE_ALIASING; no volatile coercion in any of its five listed spellings (the
word `volatile` does not appear in the diff); no unused-local-array frame coercion; no dead-param
assign; no dead-conditional store; no empty-body `if`; no `if (1)` wrapper; no dead-goto label pad;
no DImode chain; no goto-end accumulator; no param-local-alias declaration trick; no `s32 one = 1;`
opaque variable; no lowercase `asm()`; no `asm("sym")` alias rename; no redundant width casts
(the session *removed* one no-op mask rather than adding any); no `bb2.ld` rodata reorder; no
`reg_n_refs` chain extender. Declaring a struct for a record is not a respelling of any of these —
it changes what the program says the memory *is*, which none of the catalogued families do.

## T6 naming-announces-intent
NO. There is no `pad`, `_pad`, `dummy`, `unused`, `spill`, `sp_*`, `_buf`, `tail`, `slack`, or
`_frame_pad` in the diff. Struct member names are offset-derived (`w0`, `w4`, `h10`, `b1F`, ...) —
a neutral, evidence-free layout description rather than an unevidenced semantic claim
([[names-require-evidence]]). Members not touched by this function (`wC`, `b1C`..`b1E`, `w20`..`w2C`,
`h16`, `h36`, `h3E`) exist to make the layout total and correctly sized; they are declared, not
referenced, which is the ordinary state of a record declaration and not an unused-local-array
coercion (they are struct members of a caller-owned object, not function-frame storage — no frame
byte is reserved for them). Locals are named for what they hold (`inv_frac`, `use_high`, `dx`).

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Named-intermediate declaration order
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID."
  PRECEDENT: .claude/rules/no-new-park-categories.md:189
  (Claimed only for `zval`, conservatively. It satisfies the 2026-08-17 clarification's
  conditions: once-written, once-read, and its value is real and present in the target's own
  bytes. The struct typing is NOT claimed under any coercion family — ordinary C record
  declaration is not a coercion construct and needs no carve-out.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. Nothing in the diff is annotated `/* FAKE */`
because nothing in the diff is a codegen-coercion construct requiring one.
