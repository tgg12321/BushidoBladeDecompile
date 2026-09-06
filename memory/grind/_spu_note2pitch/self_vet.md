# SELF-VET — _spu_note2pitch  (session s2b, 2026-09-06)

Diff vs HEAD (src/main.c), in full:
  (1) `u32 _spu_2pitch(u32 atten, u32 rem) {`  ->  `inline u32 _spu_2pitch(...)`
      (the GNU89 `inline` keyword on the already-COMPLETED-C sibling defined
      immediately above; the sibling still emits out-of-line and still measures
      sandbox _spu_2pitch --disable all = 0)
  (2) `INCLUDE_ASM("asm/funcs", _spu_note2pitch);` replaced by the C body in
      memory/grind/_spu_note2pitch/candidate.c

CONSTRUCTS: (a) GNU89 `inline` keyword on the sibling `_spu_2pitch`;
(b) `u16 atten;` — a narrow local holding the octave attenuation, written in
each arm of the sign branch and read once after the join;
(c) `diff = atten;` — the attenuation staged through the pre-existing local
`diff` and consumed on the next line as `_spu_2pitch`'s first actual (FAKE-
annotated, family: staged-value-reused-variable);
(d) two ternary absolute values `(diff < 0) ? -diff : diff` and
`(rem < 0) ? -rem : rem` — ordinary C, each materialising a mips.md `abssi2`
that is present in the shipped bytes (0x8008BB4C and 0x8008BBC4);
(e) `oct = absdiff / 1536; rem = absdiff % 1536;` — ordinary C divmod;
(f) the `if (pitch >= 0x4000) pitch = 0x3FFF;` clamp — ordinary C, in the bytes.

## T1 semantic purpose
(a) `inline` changes the program's structure, not just its bytes: the target's
    tail IS an integrated copy of `_spu_2pitch` (the 0x103B curve walk, the
    `upper` spill to 0x8($sp), the 16-byte frame). Without it the call is a real
    `jal` and the bytes are nowhere near. It has an observable effect.
(b) `atten` carries the real octave attenuation value (0x1000 << oct or
    0x1000 >> oct) from the branch arms to the call. Removing it means writing
    the two expressions somewhere else; the value is genuinely consumed.
    Its `u16` type is the value's real width (the attenuation is a 16-bit
    fixed-point quantity and the shipped code truncates it: `andi $a2,$v0,0xFFFF`).
(c) `diff = atten;` is a real store of a real value that is READ on the very
    next line. It is not dead and it is not a self-assign. What it does NOT
    have is a purpose beyond that read: passing `atten` straight to the call
    would be behaviourally identical. That is exactly why it is FAKE-annotated
    and claimed under the staged-value-reused-variable exception rather than
    presented as ordinary C. Declared honestly, not hidden.
(d)(e)(f) all compute values the function returns; each is byte-visible.

## T2 human-programmer
(a) Yes — `_spu_note2pitch` and `_spu_2pitch` are two exported entry points of
    the same PsyQ LIBSPU module (S_N2P) sharing one curve walk; marking the
    shared helper `inline` in the same translation unit is what a 1997 library
    author would write, and it is why the original ships an integrated copy.
(b) Yes — a named 16-bit local for the attenuation is the obvious spelling.
(d)(e)(f) Yes — the ternary abs is the standard pre-`abs()` idiom, `/` and `%`
    on the cents distance is the natural divmod, and the clamp is the API's
    documented 14-bit pitch ceiling.
(c) NO. A reader would ask "why not pass `atten` directly?". This is the one
    construct in the diff that fails the human-programmer test on its face,
    and it is submitted under a named, frozen, owner-sanctioned exception whose
    ORIGIN is this exact mechanism, with the required FAKE annotation.

## T3 GCC-internals justification
(c) is justified by a GCC internal and says so: GCC 2.7.2 sched.c
`adjust_priority` -> `birthing_insn_p` (tools/gcc-2.7.2/sched.c:2504-2535),
whose test is literally `reg_n_sets[i] == 1` on a live destination. The
widening `andi` that produces `_spu_2pitch`'s `atten` actual is boosted to
LAUNCH_PRIORITY when its destination pseudo is single-set; the backward list
scheduler then picks it FIRST off the ready list and therefore EMITS it LAST,
after the inlinee's `addiu $a0,$zero,0x103B`. The target has the opposite
order. Staging through `diff` makes that pseudo two-set, the boost does not
fire, and the pair falls to the LUID tie-break, which is the target order.
This is a GCC-internals mechanism, which under T3 is a cheat signal for any
UNSANCTIONED construct — and it is precisely the mechanism the
staged-value-reused-variable rule was written to cover (see its Origin
section, which names `sched.c adjust_priority -> birthing_insn_p` and
`reg_n_sets[regno] == 1` verbatim). Constructs (a),(b),(d),(e),(f) need no
GCC-internals justification at all; they are explained by the program's logic.

## T4 permuter/search provenance
No permuter output is in this diff. This session's mandated modality was
`permuter`; per the brief's escape clause the frontier named FAKE-construct
removal / duplication-into-arms, which the permuter cannot express, so the
session used `tools/sweep_variants.py` over 24 hand-written forms
(tmp/grind/_spu_note2pitch/s2/sweep_results.txt). Every form in the sweep was
written by hand from a stated hypothesis, and the winning one is the spelling
the staged-value-reused-variable rule prescribes — it was predicted before it
was measured, not discovered by a search and rationalised afterwards. No
construct here passes review only because a detector misses this spelling.

## T5 family check
(a) `inline` on a same-TU helper is plain GNU89 C, not in any forbidden family.
(b) A narrow-typed local holding a real consumed value is plain C. It is NOT
    the banned per-arm `(u16)` narrowing: there is no cast anywhere in the body,
    the narrowing happens once (at `u16 atten`'s two arm assignments, which is
    the value's declared width, not a redundant width cast on a u32 receiver),
    and the receiver `diff` is not narrowed per arm.
(c) Matches the FROZEN list entry "Variable reuse for codegen control", gated by
    .claude/rules/staged-value-reused-variable.md. Checked against that rule's
    six bounds:
      1. value real and used — read on the next line as the call's first actual. OK
      2. the variable already exists for a real job — `diff` holds the signed
         cents distance and drives `if (diff >= 0)`; it is not invented. OK
         (`atten` is a fresh named intermediate, but `atten` is NOT the borrowed
         variable — it is ordinary C on its own merits, and it is present in the
         score-2 baseline form too.)
      3. borrow provably safe — `diff`'s last read is the `if (diff >= 0)` test
         that selects the arm; nothing reads `diff` after the join, and the
         staged value is consumed before `diff` is next assigned (it never is).
         OK
      4. annotated with what + mechanism + lever-exhaustion. OK (quoted below)
      5. last resort with receipts — s1 measured forms A/B/C/D/E/F/G/H/K/L/M,
         s2 measured the divmod and narrowing axes, s2b measured 24 further
         spellings; every cast-free, staging-free form measured >= 2.  OK
      6. everything else still applies — no dead stores, no unused variables, no
         arrays, no register pins, no inline asm, no volatile, no build-time
         editing. OK
    NOT cited: .claude/rules/defeat-licm-hoist-var-reuse.md — that rule is
    loop-scoped (loop.c movable admission) and its own Related section says not
    to cite it for straight-line code. This is straight-line code.
(d)(e)(f) ordinary C, no family.
BANNED-CONSTRUCT CHECK: this function's banned list is (i) `atten = (u16)(0x1000
>> oct);` in the else-arm and (ii) per-arm `(u16)` narrowing chosen to give
`atten` two static sets. Neither is present under any spelling: the body
contains no cast operator at all, both arms assign the *same* narrow variable
without a cast, and the two-set pseudo is `diff`, a pre-existing local, not the
call receiver. This is a different attack on the same residual, not a respelling
of the banned one — the banned form put the extra set on the receiver by
duplicating a redundant width cast; this one puts it on a borrowed live local
under a frozen family with its own rule file.

## T6 naming-announces-intent
No `pad`, `dummy`, `unused`, `spill`, `tmp`, `slack`, `_buf` or similar. The
locals are `cen`, `tgt`, `diff`, `absdiff`, `oct`, `rem`, `atten`, `pitch` —
every one names the quantity it holds, and every one is read. `atten`'s only
uses are its two arm writes and the staged read; `diff` is read by the sign
test and by the call.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: staged-value-reused-variable (variable reuse for codegen control)
  SCOPE: "A real, immediately-used value staged through an existing (currently-dead) local to fix instruction order; FAKE-annotated, lever-exhaustion required; zero dead code"
  PRECEDENT: .claude/rules/staged-value-reused-variable.md:1
  PRECEDENT: docs/reference/sotn-construct-index.md:51
  PRECEDENT: .claude/rules/no-new-park-categories.md:232

ANNOTATION-CONFORMANCE:
  /* FAKE: the octave attenuation is staged through `diff` (dead from the
     `diff >= 0` test above onward; nothing reads it after this point) and
     consumed on the very next line, mechanism: GCC 2.7.2 sched.c
     adjust_priority -> birthing_insn_p (reg_n_sets[regno]==1) — a two-set
     pseudo is not boosted to LAUNCH_PRIORITY, so the widening `andi` is
     emitted before the inlinee's `li 0x103B` as in the target,
     lever-exhaustion: memory/grind/_spu_note2pitch/hypotheses.md s1 H1/H5/H6
     and s2b (24 measured spellings, all >= 2 without this staging) */
  Carries all three: WHAT (the attenuation staged through `diff`, with the
  liveness argument), MECHANISM (sched.c adjust_priority -> birthing_insn_p,
  reg_n_sets==1), LEVER-EXHAUSTION (the hypotheses ledger + this session's
  24-form sweep, tmp/grind/_spu_note2pitch/s2/sweep_results.txt).

MEASUREMENTS THIS SESSION (HEAD chassis, sibling `inline`, no other FAKE):
  sandbox _spu_note2pitch --disable all = 0
  sandbox _spu_2pitch     --disable all = 0
  full-build verify-oracle              = ok:true
