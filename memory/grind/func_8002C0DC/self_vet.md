# SELF-VET — func_8002C0DC

Session 1 (recon modality). Diff vs HEAD: the function body's loop was rewritten
from a hand-rolled walking-pointer `do { } while` carrying three
`register T x asm("sN")` pins into an ordinary index-based `for (i = 0; i < 2; i++)`
loop over the 0x44C-stride array at `D_80101EC8`, with the three field reads taken
as ordinary offsets off the per-iteration element pointer. The three register pins
were DELETED. Nothing was added.

CONSTRUCTS: index-based `for` loop with a per-iteration element pointer
`e = (u8 *)&D_80101EC8 + i * 0x44C`; three plain local initializers (`ptr`,
`arg1`, `arg2`) holding the dereferenced pointer and the two coordinate deltas.
(No dead code, no dead stores, no self-assigns, no unused declarations, no
volatile, no aliases, no wrappers, no inline asm, no register pins — the diff
strictly REMOVES three `register asm("sN")` pins and replaces a pointer-walk with
an index walk.)

## T1 semantic purpose
Every construct is load-bearing.
- The `for` loop: the function genuinely iterates the two entries of the
  0x44C-stride array; the loop bound `i < 2` and the counter are the program's own
  iteration logic (they exist in the target bytes as `addiu $s1,$s1,1` /
  `slti $v0,$s1,2` / `bnez`).
- `e = (u8 *)&D_80101EC8 + i * 0x44C`: the address of the element being processed.
  Removing it makes the function unable to address entry 1.
- `ptr = *(u8 **)e`: the linked partner object read out of field 0. Both of its
  field reads (`+0xD8`, `+0xE0`) depend on it; it is in the target bytes as
  `lw $v0,0x0($s0)`.
- `arg1` / `arg2`: the X and Y deltas passed to `ratan2`. They are the call's
  actual arguments; the target emits both `subu`s.
Removing ANY construct changes observable behavior. None is byte-neutral padding.

## T2 human-programmer test
Yes — this is the shape a human writes from the specification "for each of the two
fighters, aim it at its opponent." `for (i = 0; i < 2; i++) { entry = &tbl[i];
opponent = entry->partner; angle = ratan2(opponent->x - entry->x,
opponent->z - entry->z); Aim(entry, angle); }` is the plainest possible spelling.
Nothing in the body would make a reader ask "why is this here?". By contrast the
form being REPLACED (a manual walking pointer pinned to `$s0` with the counter
pinned to `$s1` and the call result pinned to `$s2`) is the one that reads as
codegen coercion — this diff removes that smell rather than adding one.

## T3 GCC-internals justification test
The construct is justified by program logic alone (iterate two array entries; read
two fields off each; call ratan2 on the deltas), and I can describe the entire body
without naming a single GCC pass. GCC internals appear ONLY in my *diagnosis of why
the previous form was wrong*, which is diagnosis, not justification: the old
walking-pointer form gave loop.c two independent induction variables, so
`strength_reduce`/`combine_givs` materialized a second live pointer
(`$s1 = $s0 + 0xE0`, consumed at `-8($s1)` and `0($s1)`), which cost a fourth
callee-save register and an 0x28 frame. The index form has one basic induction
variable, so all three field references (+0x00, +0xD8, +0xE0) combine onto the
single derived pointer that the target uses. That is an explanation of the
MEASUREMENT, not the reason the code is written this way — the code is written this
way because it is the natural spelling of the loop, and it happens to be the
natural spelling that the original compiler saw too.

## T4 permuter/search provenance
No permuter, no auto-search, no directed PERM macros, no sweep script were run this
session. The form was derived by hand from an instruction-level read of
`asm/funcs/func_8002C0DC.s` against an objdump of the sandbox build: the extra
callee-save + the `addiu $s1,$s0,0xE0` / `addiu $s1,$s1,1100` pair identified the
surplus induction variable, and the index-loop rewrite was the direct first-try
consequence. It closed 23 -> 0 on the first measurement. It does not depend on any
detector failing to recognize a spelling.

## T5 family check
No forbidden family is touched, by name or by analogy. Checked against the full
catalog: no register-asm pin (three were REMOVED), no hardcoded-`$N` `__asm__`, no
lost-codegen regfix insert, no scheduling barrier, no INLINE_MOVE_ALIASING, no
volatile in any spelling (alias-rename, cast, plain extern, `(void)volatile`), no
local array (used or unused), no dead-param-assign, no dead-conditional-store, no
empty-body `if`, no `if (1)`, no dead-goto label pad, no DImode chain, no
goto-end accumulator, no param-local-alias declaration-order trick, no `s32 one = 1;`
opaque variable, no lowercase `asm(...)`, no build-time rewriting, no `asm("sym")`
rename, no redundant width casts, no `bb2.ld` change. Nor is any *new* family being
reached for: the diff adds no construct that requires a family at all, because every
line of it is ordinary program logic. The `(u8 *)` / `(s32 *)` / `(s16 *)` casts are
the file's existing pre-typed-struct addressing idiom, unchanged in kind from the
code being replaced and from the untouched post-loop block below it.

## T6 naming-announces-intent test
Names are `i` (the loop counter, read by the bound test), `e` (the element being
processed, dereferenced four times), `ptr` (the partner object, dereferenced twice),
`arg1` / `arg2` (the two `ratan2` arguments, both passed). No `pad`, `_pad`, `dummy`,
`unused`, `spill`, `sp_*`, `_buf`, `tail`, `slack`, or `_frame_pad`. Every name has
at least one real read; none is declaration-only, discard-only, or address-of-only.

SANCTIONED-FAMILY-CLAIMS: none

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in the diff.

## Integration note (not a vetting item)
The 13 regfix rules at `regfix.txt:229-244` for func_8002C0DC were written against
the OLD (wrong) codegen and are now actively harmful — they must be retired by the
operator/driver before the full-build oracle check. I am forbidden from touching
`regfix.txt` and from running `retire`, so this is flagged rather than done.
