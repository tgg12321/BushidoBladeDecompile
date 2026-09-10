# SELF-VET — func_8006DD94

CONSTRUCTS: function-local `EnvB` typedef (the EnvA descriptor layout at 0x34 with two
trailing unwritten words pad2C/pad30); `s32 semi = 0;` (once-written, read twice — the
semi-transparency mode); `s32 hdr` (once-written per iteration, read twice — the sprite
header pointer); `s32 c` (the computed pulse colour); `s16 i` (loop counter); `u16 rect[4]`
(four real stores, passed to func_80069898); `s32 *q` (the descriptor table); the chained
assignment `s.col_r = s.col_g = s.col_b = <v>;`; two forward declarations
(`extern s32 D_800A374C;`, `extern void func_8006D808(s32 *, s32 *, s32 *, s32, s32);`);
removal of the `INCLUDE_ASM("asm/funcs", func_8006DD94);` line.

## T1 semantic purpose
- `semi`: YES — it is the semi-transparency mode, and it is read by two different
  semi-transparency consumers. `s.semi` is EnvA offset 0x10, which func_8007352C passes
  straight to `SetSemiTrans((s32)sp, env->semi)` (src/text1b.c:6792). The second read is
  func_8006E480's second argument; that function (src/text1b.c:6171) computes
  `(a0[0] & 0xFE1F) + (a0[1] << 7) + a1`, a getTPage word in which the 0xFE1F mask clears
  bits 5-8 and the second argument refills bits 5-6 — the PS1 GPU's abr
  (semi-transparency) field. One variable, one physical quantity, two consumers that both
  mean "semi-transparency mode 0 / opaque". Its value IS materialised in the target's
  bytes: `addu $s5,$zero,$zero` in the prologue and `addu $a1,$s5,$zero` in the call's
  delay slot (asm/funcs/func_8006DD94.s:6 and :74).
- `hdr`, `c`, `i`, `q`, `rect`: YES — all carry values the target's bytes contain, all
  read after being written.
- `EnvB`'s pad0C/pad20/pad24/pad2C/pad30: these are unwritten members of a struct that IS
  used (its address is passed to func_8007352C every iteration). They are a DATA-MODEL
  claim, not a coercion: the target's frame puts this descriptor at sp+0x18 and the rect
  at sp+0x50, so the descriptor object in the original source is larger than the 0x2C
  EnvA the rest of the file uses. pad0C/pad20/pad24 are exactly the field names the
  already-accepted EnvA declaration uses for its own unwritten members
  (src/text1b.c:6719-6733, in COMPLETED-C func_8007352C's TU region); pad2C/pad30
  continue that convention for the two additional words.
- Removing the construct is NOT byte-neutral for `semi`: deleting the local and passing
  the literal `0` measures score 8 this session
  (memory/grind/func_8006DD94/rejected/no-local-literal-score8.c).

## T2 human-programmer
Yes. A programmer writing this renderer declares the semi-transparency mode once
(`s32 semi = 0;`), stores it into the descriptor the sprite emitter consumes, and passes
the same mode into the tpage-word helper. Nobody reading the body asks "why is `semi`
here?" — it is the answer to "what semi-transparency mode does this thing draw with?".
Likewise `hdr` names the per-entry sprite header that both `s.header` and `s.table` are
derived from. The `EnvB` typedef reads as the descriptor type this particular caller
builds.

## T3 GCC-internals justification
No. The MECHANISM claim in the ledger (a named local gives the allocator a loop-spanning,
call-crossing pseudo, which it seats in a call-saved register, so ALIGN8(gp_regs) goes
24 -> 32 and the frame 112 -> 120) is how I EXPLAINED a measurement and how I chose
between two equally-truthful spellings — it is not the justification for the construct's
existence. The construct's justification is semantic (T1/T2). Per
.claude/rules/ordinary-c-judge-decidable.md Ruling 1 prong 3, "scheduling-motivated
respelling is not a FAIL ground when the spelling is semantically truthful"; that is the
situation here and I am disclosing the method honestly rather than hiding it. There is no
`register`, no pin, no `__asm__`, no barrier, no volatile, no pragma, no gate-list or
build-file change: `git diff` touches src/text1b.c only among build files.

## T4 permuter/search provenance
No permuter, no automated search. Every form in this session was hand-written and
measured with `sandbox --disable all`; the winning form came from READING the two callees'
decompiled bodies (func_8007352C at src/text1b.c:6763-6805 and func_8006E480 at
src/text1b.c:6171) to find out what the value actually means. The detectors are irrelevant
to it: it would survive any spelling-based detector because there is nothing to detect —
it is a named local with a real value.

## T5 family check
No forbidden family matches. Not a register pin, not hardcoded-$N asm, not a scheduling
barrier, not INLINE_MOVE_ALIASING, no volatile of any spelling, no dead param assign, no
dead conditional store, no empty-body if, no `if (1)`, no dead goto/label pad, no DImode
chain, no goto-end accumulator, no param-local alias, no opaque arithmetic variable
(`semi` is a plain literal 0, not `s32 one = 1;`-style obfuscation), no `asm()` alias, no
redundant width cast, no linker/rodata reordering. Specifically NOT the FAILed
2026-09-10 05:24 body: that one was `s32 *clut; clut = NULL;`, a pointer-typed holder the
Judge correctly read as "a zero-holder in pointer clothing" because func_8006E480 never
dereferences its second argument. This body has no pointer, no `clut`, and the value has a
truthful reading in BOTH of its uses. It is also not the named-intermediate FAKE family:
that family exists for locals with NO semantic purpose; this one has one, so
.claude/rules/ordinary-c-judge-decidable.md Ruling 1 prong 2 ("every NO-semantic-purpose
construct must sit inside the frozen list") does not reach it. Prong 4
(simplest-known-form) is satisfied: of the byte-exact forms measured this session
(pointer holder, s32 holder read once, s32 `semi` read twice), the one that lands is the
one whose every read is semantically truthful.

## T6 naming-announces-intent
No local is named pad/dummy/unused/spill/slack/tail/_buf/_frame_pad, and no local's only
use is a discard, an unused declaration or an address-of. `semi`, `hdr`, `c`, `i`, `q`,
`rect` are all consumed. The only `pad`-prefixed identifiers are struct MEMBERS, copied
from the naming convention of the EnvA declaration already on main
(src/text1b.c:6719-6733) for the same descriptor's unknown fields; they are not locals and
they do not coerce a frame by address-of or by `(void)`.

SANCTIONED-FAMILY-CLAIMS: none — the body is ordinary C. No construct in it is a
no-semantic-purpose construct, so no frozen-list family entry is claimed or needed.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
