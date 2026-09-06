# SELF-VET — _spu_note2pitch  (s2, 2026-09-06)

Diff under review: `src/main.c` — the `INCLUDE_ASM("asm/funcs", _spu_note2pitch);`
line is replaced by the C body in `memory/grind/_spu_note2pitch/candidate.c`, and
the sibling `_spu_2pitch` (already COMPLETED-C, immediately above) gains the GNU89
`inline` keyword. Nothing else changes.

Measurements with this exact diff in place:
  sandbox _spu_note2pitch --disable all = 0
  sandbox _spu_2pitch      --disable all = 0   (sibling stays byte-identical)
  verify-oracle (full clean build + link SHA1) = ok:true

CONSTRUCTS: GNU89 `inline` keyword on the sibling _spu_2pitch; `(u16)` narrowing
cast on the octave base in BOTH arms of the sign branch; ternary abs
`(diff < 0) ? -diff : diff`; ternary abs `(rem < 0) ? -rem : rem` on the second
call actual; named locals cen/tgt/diff/absdiff/oct/rem/atten/pitch.

## T1 semantic purpose
- `inline` on _spu_2pitch: this is a language-level keyword, not a construct in
  the function body. It changes WHERE the helper's code lives, which is an
  observable property of the program the original shipped: the target's tail
  (0x103B curve walk, `upper` spilled to 0x8($sp), 16-byte frame) is literally an
  inlined copy of the helper, and the helper is ALSO exported out-of-line at
  0x8008BA94. Both facts are in the shipped binary.
- `(u16)` cast, up-shift arm: LOAD-BEARING. `absdiff` reaches (0xFFFF<<7)+0xFFFF
  ~= 8.4M cents, so `oct` reaches ~5461 and `0x1000 << oct` overflows 16 bits for
  every oct >= 4. Deleting this cast changes the returned pitch.
- `(u16)` cast, down-shift arm: `0x1000 >> oct` cannot exceed 0x1000, so this cast
  alone changes no value. It is not byte-neutral, though — it MATERIALIZES bytes.
  The target has exactly one `andi $a2,$v0,0xFFFF` and it sits AFTER the join
  label .L8008BBB0, i.e. it truncates the value produced by BOTH arms. That is
  direct byte evidence that the original narrowed in both branches. Measured:
  narrowing only the up arm (rejected/downarm-cast-omitted-score4.c) = 4;
  narrowing once after the join (`u16 base;` + widen at the call, the s1
  candidate) = 2. Only symmetric per-arm narrowing reproduces the target.
- `(rem < 0) ? -rem : rem`: materializes the target's
  `bgez $v1 / addu $v0,$v1,$zero / negu $v0,$v0` at 0x8008BBC4-0x8008BBCC (the
  mips.md abssi2 template). `rem` is non-negative on every reachable path, so this
  is defensive normalization in the original library source — but the three
  instructions it emits are IN the shipped function, so the construct is
  reproducing original semantics, not coercing a compiler.
- `(diff < 0) ? -diff : diff`: load-bearing (diff is genuinely signed) and
  materializes the abssi2 at 0x8008BB4C-0x8008BB54.
- Locals: every one is written once and read; `cen`/`tgt` feed `diff`, `absdiff`
  feeds the divmod, `oct`/`rem` feed the arms and the call, `atten` is the call
  actual, `pitch` is the clamped result. No dead local, no unused local, no
  written-never-read local, no local array, no address-of.

## T2 human-programmer
Yes. Reading the function as a specification — "convert a signed cents distance
into an octave count plus leftover cents, scale the 16-bit unity pitch 0x1000 by
the octave, interpolate the leftover along the curve, clamp to 14 bits" — every
line is what that specification asks for. `oct = absdiff / 1536; rem = absdiff %
1536;` is the textbook divmod spelling. The `(u16)` narrowing states that the
octave base is a 16-bit quantity, which is exactly what the SPU pitch register is.
There is no line a reader would ask "why is this here?" about: the only line whose
value is redundant on its own (the down-arm cast) is there because the up-arm cast
is mandatory and both arms assign the same 16-bit quantity, which is ordinary type
discipline, not an oddity.

## T3 GCC-internals justification
The construct is justified by the FUNCTION'S SEMANTICS and by TARGET BYTES first:
the target contains one post-join `andi` covering both arms, and the up arm
genuinely overflows u16. The GCC mechanism (two static sets of `atten` at sched1
time defeat sched.c `birthing_insn_p`'s `reg_n_sets == 1` gate, and jump2
cross-jumping re-merges the two identical `andi` tails) is recorded in the ledger
as the EXPLANATION of why the s1 spelling was 2 insns off — it is not the reason
the construct is in the source. Deleting the cast breaks the program, not just the
byte match. No lever naming, no pass named as the purpose.

## T4 permuter/search provenance
No permuter, no auto-search, no sweep tool was used this session. The form was
derived from the s1 ledger's stated residual (the andi/li ordering) by asking
where the truncation lives in the original, and confirmed by three measurements
that discriminate the spellings (2 / 4 / 0). It is also the form that a reader of
the target asm arrives at directly: the andi is after the join, so both arms feed
it.

## T5 family check
No forbidden family matches. Not a register pin; no `__asm__` of any kind; no
scheduling barrier; no volatile anywhere; no alias rename; no unused local array
or frame coercion; no dead-param assign, dead-conditional store, empty-body `if`,
dead goto, DImode chain, `if (1)` wrapper, `do {} while (0)` wrap, opaque
constant-holder, pointer alias to a global, duplicated same-value re-store, or
linker/rodata reorder. The nearest catalogued family is F2 "redundant width
casts"; the cast here is not that: it is required for correctness in one arm and
it materializes an instruction that exists in the target rather than being
byte-neutral, so it fails F2's premise on both prongs. The nearest sanctioned
family is duplicated-statement-into-arms, and I am deliberately NOT claiming it,
because the two arms do not duplicate a statement — each arm assigns its own
distinct value (`0x1000 << oct` vs `0x1000 >> oct`) to the one variable the call
consumes, which is a plain if/else initialization.

## T6 naming-announces-intent
Names are cen_note/cen_fine/note/fine (the parameters), cen, tgt, diff, absdiff,
oct, rem, atten, pitch. Every name states the quantity it holds. `atten` matches
the sibling's parameter name for the same value. No pad/dummy/unused/spill/slack/
tail/_buf naming anywhere, and no variable exists whose only uses are discards,
address-of, or a declaration.

SANCTIONED-FAMILY-CLAIMS: none — the body is ordinary C and claims no exception.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
