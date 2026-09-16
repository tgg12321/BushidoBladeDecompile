# SELF-VET — _SsSndCrescendo

Body under vet: `memory/grind/_SsSndCrescendo/candidate.c`, applied to
`src/main.c` in place of `INCLUDE_ASM("asm/funcs", _SsSndCrescendo);`.
Measured this session: `sandbox _SsSndCrescendo --disable all` =
**score 0, build_insns 200, target_insns 200**, `cheat_asm_stripped` unchanged
at 9 (all from other functions in the TU — this body contains no asm at all).

CONSTRUCTS: `s32 bank_no = a0;` (fresh named intermediate, FAKE-annotated); `s32 *score_tbl = (s32 *)&_ss_score;` (C-level pointer alias to a global, FAKE-annotated); `s32 *bank = score_tbl + bank_no;` (ordinary pointer local, 5 reads); `u8 *base = (u8 *)(*bank + (s16)a1 * 0xB0);` (ordinary pointer local); `u16 voll, volr;` (ordinary address-taken output locals); the `if (unk40 >= 0) {...} else {...}` handler and the `(unk98 == 0 || unk40 <= 0)` tail check written out in both arms (ordinary C — the SOTN reference's own structure); two different address spellings for the clear sites (`*bank`-based inside the arms, `((s32 *)&_ss_score)[a0]` at the two tail sites — again the reference's own two spellings, `score->unk90` vs `_ss_score[arg0][arg1].unk90`).

## T1 semantic purpose
- `bank_no`, `score_tbl`: NO independent observable effect — `bank =
  (s32 *)&_ss_score + a0` computes the identical value. They exist to place
  the `_ss_score` address insn between the two halves of the index shift.
  They FAIL T1 on their own and are therefore claimed under sanctioned
  families below, with `/* FAKE */` annotations, a dump-read mechanism and a
  measured lever-exhaustion ledger. They are not a first reach.
- `bank`, `base`: real consumed values, each read at 5+ sites; a human writes
  these (`score` in the SOTN reference is the same local). PASS.
- `voll`, `volr`: the two out-parameters of `_SsVmGetSeqVol`. PASS.
- Duplicated handler / duplicated tail check: each copy is semantically
  REQUIRED on its own arm — deleting either changes behaviour on that path.
  They are the literal structure of the matched SOTN reference
  (`tmp/sotn-decomp/src/main/psxsdk/libsnd/cres.c`, the `else { SpuVmSetSeqVol
  (...,0x7F,0x7F,0); _ss_score[arg0][arg1].unk90 &= ~0x10; }` blocks and the
  two `if ((score->unk98 == 0) || (score->unk40 == 0))` blocks). PASS.
- Two clear-site spellings: the reference has the same two spellings for the
  same reason (a member read off the cached `score` pointer vs a fresh
  `_ss_score[arg0][arg1]` subscript). PASS.

## T2 human-programmer
The whole body is the SOTN reference function's shape, statement for
statement, with four BB2-4.1-build divergences that s1/s2/s6 measured
individually (outer `--unk98 < 0` guard, literal-1 4th argument, `unk40 <= 0`
tail disjunct, `(u16)` truncation of the computed volume pairs). A human
decompiling `score = &_ss_score[bank][ch]` naturally writes a table pointer
and an index; that is what `score_tbl` / `bank_no` / `bank` are. The one
thing a reader would ask "why is this here?" about is that the table pointer
and the index are *separate statements in that order* — which is precisely
why both carry a `/* FAKE */` annotation stating the mechanism.

## T3 GCC-internals justification
Yes, and it is stated openly rather than hidden: the mechanism is combine.c's
`try_combine` folding the s16->int `ashiftrt:16` together with the scale-by-4
`ashift:2` into a single `ashiftrt:14` emitted at the LATER insn's slot, plus
sched.c's `rank_for_schedule` leaving the two independent insns in RTL/LUID
order. Read from the cc1 `-da` dumps this session
(`tmp/grind/_SsSndCrescendo/dumps/main.{rtl,cse,combine,sched}`), not guessed.
This is exactly the situation the `/* FAKE */` + sanctioned-family route
exists for; the two constructs are claimed under it, not presented as
ordinary C.

## T4 permuter/search provenance
None. No permuter was run this session. Every form was hand-derived from the
target disassembly and the cc1 dumps, and each of the 13 rejected
single-expression preamble spellings was measured individually (all score 2).

## T5 family check
- `score_tbl` → **C-level pointer alias to a global** (frozen list entry;
  `.claude/rules/pointer-alias-fake-exception.md`). Not an `asm("sym")`
  rename, not a volatile coercion, not a dead local.
- `bank_no` → **named-intermediate declaration order**
  (`.claude/rules/no-new-park-categories.md:204` + the 2026-08-17/2026-08-31
  clarification). Once-written; the value it holds (the sign-extended bank
  index) appears in the target's own bytes (`sll $v0,$a3,16` / `sra
  $v0,$v0,14`); byte-neutral (build_insns 200 == target_insns 200); a fresh
  local, not a borrow; its destination is not live-pre-initialized.
- No other construct in the diff matches any forbidden family: no register
  pins, no `__asm__`, no scheduling barrier, no volatile, no dead store, no
  unused array, no empty-`if`, no `goto` at all, no width-cast padding.

## T6 naming-announces-intent
`bank_no`, `score_tbl`, `bank`, `base`, `voll`, `volr` — every name is
domain-descriptive (LIBSND bank id, the `_ss_score` bank table, the selected
bank slot, the channel control block, left/right volume). No `pad`, `dummy`,
`unused`, `spill`, `tmp`, `slack` or `_frame_pad`. Both FAKE locals are read
on the very next line; neither is address-taken or discarded.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: C-level pointer alias to a global
  SCOPE: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): a C-level local pointer alias (second handle) to a global — `Type* t = &g_Thing;`, `s16 (*p)[] = &D_xxx;`, FakePrim-style reinterpret views — is allowed as a LAST-RESORT lever with `/* FAKE: ... */` annotation + lever-exhaustion. SOTN idiom incl. self-admitted `// n.b.! unused, required for PSP`. asm(\"sym\") alias-RENAMES stay forbidden."
  PRECEDENT: .claude/rules/pointer-alias-fake-exception.md:37

  FAMILY: Named-intermediate declaration order
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID. SOTN's `randy` chain in `src/weapon/w_037.c` is the same mechanism."
  PRECEDENT: .claude/rules/no-new-park-categories.md:204

ANNOTATION-CONFORMANCE:
  /* FAKE: named intermediate for the bank index, mechanism: combine.c
     try_combine folds the s16->int `ashiftrt:16` together with the scale
     `ashift:2` into one `ashiftrt:14` and emits it at the LATER insn's slot,
     and sched.c rank_for_schedule leaves the independent insns in RTL/LUID
     order - so the index conversion must be its own statement ahead of the
     table-address statement; lever-exhaustion: hypotheses.md H8 - 13
     single-expression spellings of this preamble measured, all score 2
     (v5,v6,v7,wa-we,x1,x2,y1-y4 in tmp/grind/_SsSndCrescendo/s7/). */
  /* FAKE: C-level pointer alias to the _ss_score global, mechanism: the
     `movsi` of the symbol_ref has to be emitted BETWEEN the surviving
     `ashift:16` and combine.c's folded `ashiftrt:14`; fold() moves the
     constant ADDR_EXPR to operand 1 of any single pointer-sum expression,
     which therefore evaluates the whole index first; lever-exhaustion:
     hypotheses.md H8 - the same 13 measured spellings, all score 2. */
Both carry what + a named GCC-pass mechanism + a lever-exhaustion pointer.
