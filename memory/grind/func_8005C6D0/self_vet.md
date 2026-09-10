# SELF-VET — func_8005C6D0

Measured this session: `sandbox func_8005C6D0 --disable all` => **score 0**, target_insns 118,
build_insns 118, with the C body in place in src/text1b.c; full `verify-oracle` reports
`"ok": true, "build_matches": true` (SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa).

CONSTRUCTS: two byte-offset locals (`entry_off`, `vol_off`, both `i * 8`, both read and consumed
in emitted addressing); symbol+addend addressing of three independent extern scalars
(`*(u16 **)((u8 *)&D_800EFB78 + entry_off)`, `*((u8 *)&D_800EFB7D + vol_off)`,
`*((u8 *)&D_800EFB7C + vol_off)`, `*(s32 *)((u8 *)&D_800EFB78 + i * 8) = 0`); block-scope
`extern s32 *D_800EFC38[];`; a pointer local `ev` dereferenced four times; `(s16)` narrowing casts
on HImode values; a top-tested `for` scan nested inside an explicit `if ((s16)voice < 0x18)` guard;
`next = (s16)(voice + 1);` written after the SsUtKeyOnV call. No inline asm, no register pins, no
volatile, no dead store, no dead local, no pad, no alias rename, no `do {} while (0)`, no
`/* FAKE */` construct of any kind.

## T1 semantic purpose: every construct changes what the function computes or how it addresses
real data, and none is a no-op.
- `entry_off` / `vol_off` carry the byte offset of pool entry `i`; each is dereferenced to load or
  store real data (the request pointer, the two volume bytes). Delete either and the function stops
  addressing the pool. They are not discards, not written-never-read, and not address-of-only.
  They are two distinct variables because the second is computed inside the voice-scan guard where
  the volume bytes are read; folding them into one was RE-MEASURED on this chassis and produces a
  different, shorter function (105 insns, and 99 insns when the clear site also shares it) — i.e.
  it is a genuinely different program shape, not a decoration on the same one.
- The symbol+addend reads/stores are how the function reaches the pool at all.
- `ev` is the address of the packed note word; all four argument expressions dereference it.
- The `for` scan and the `if` guard are the loop itself.
- `next = (s16)(voice + 1);` is the carrier that makes the voice search resume where the last
  key-on stopped; it persists across pool entries and is observable behaviour.

## T2 human-programmer: yes. The body reads as the obvious implementation of "flush the sound
request pool": walk 24 entries, skip empty ones, find a free SPU voice from where we left off,
remap VAB 6->3 in the alternate bank, unpack the note word, key it on, clear the entry. A reader
asking "why is this here?" gets a semantic answer for every line. The nearest thing to a codegen-
shaped choice is having a second offset local inside the voice guard rather than reusing the outer
one — and that is exactly what the already-byte-matching sibling writer of the same pool does one
function earlier in the same file (`s32 off = a3 * 8;` at src/text1b.c:2685, then
`(u8 *)&D_800EFB78 + off` / `&D_800EFB7C + off` / `&D_800EFB7D + off`).

## T3 GCC-internals justification: NO construct in the diff is justified by a GCC internal. The
mechanism notes in the ledger (jump.c duplicate_loop_exit_test rotation, reorg.c
fill_slots_from_thread, loop.c LICM) are how this session EXPLAINED and SEARCHED, and they are
recorded for the next reader; they are not the reason any line exists. Each line's reason is the
program: an offset, an address, a loop, an argument. Nothing is named "lever", nothing is inert,
and removing any construct changes the function's data flow, not merely its schedule.

## T4 permuter/search provenance: no permuter, no auto-search, no randomised spelling generator was
used. Every form was hand-written from the target's disassembly and measured one edit at a time
(`tools/grinder/dump.ps1` for pass attribution, the project cc1 for the RTL, `sandbox --disable all`
for the score). The winning form was reached by reading `tools/gcc-2.7.2/reorg.c` and the
BB2_DBR_DEBUG trace, and it survives detectors because there is nothing to detect.

## T5 family check: no forbidden family matches, even by analogy. Concretely against the catalog —
no register-asm pin, no hardcoded-$N asm, no scheduling barrier, no INLINE_MOVE_ALIASING, no
volatile in any spelling, no unused local array or frame coercion, no dead-param-assign, no
dead-conditional-store, no empty-body `if`, no `if (1)`, no dead-goto label pad, no DImode chain,
no goto-end accumulator, no param-local alias declaration trick, no `s32 one = 1;` opaque variable,
no lowercase asm block, no build-time assembly rewriting, no `asm("sym")` rename, no redundant
width cast (each `(s16)` narrows a value that is then used as a `short` argument or compared as
HImode, and removing them changes the emitted extends), and no linker/rodata reordering.
The `(u8 *)&SYM + off` addressing is the split-symbol object model, not an aggregate-merge dodge:
the target emits three INDEPENDENT `lui %hi(SYM) / addu $at,$at,<idx> / {lw,lbu,sw} %lo(SYM)`
triples against three distinct symbols and never shares a base register between them, which is
affirmative evidence against the merge rather than an absence of it. The merged/array declaration
was MEASURED this session and is worse and structurally different (loop.c hoists the symbol address
into a callee-saved register: frame 0x60, 112 insns) — banked as
memory/grind/func_8005C6D0/rejected/array-decl-licm-hoist-frame-0x60.c.

## T6 naming-announces-intent: no name in the diff announces coercion. There is no `pad`, `_pad`,
`dummy`, `unused`, `spill`, `sp_*`, `_buf`, `tail`, `slack` or `_frame_pad`. The locals are
`keys`, `i`, `voice`, `next`, `vab`, `p`, `vol_off`, `entry_off`, `ev` — each named for the value
it holds, and each read at least once for that value.

SANCTIONED-FAMILY-CLAIMS: none — the body is ordinary C. No construct in the diff belongs to any
sanctioned or unsanctioned coercion family, so no scope sentence is quoted and no family precedent
is claimed. (Informational, not a family claim: the identical addressing idiom against the identical
three symbols already ships on main in the byte-matching sibling func_8005C650,
src/text1b.c:2685.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
