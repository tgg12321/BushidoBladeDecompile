# SELF-VET — func_80041188

CONSTRUCTS: (1) `s16 two;` constant-holder local plus its live assignment `two = 2;` feeding loop1's halfword store; (2) `two = 3;` — a dead store to that local, never read, deleted by flow.c before any bytes are emitted.

Everything else in the diff is construct-free: the body is s36's form P1r (real-loop chassis, `pa4` param-alias deleted, target's honest block-2 `out3 = (s32 *)(((u8 *)a4) + 0x20);`), which measures sandbox 2 with ALL-TARGET callee-saved seats and zero constructs. No wrap, no alias, no variable reuse, no split increment, no volatile, no asm, no pin. The previously owner-sanctioned split-increment FAKE that the old candidate carried is GONE — this form does not need it.

## T1 semantic purpose
- Construct 1 (`s16 two;` + `two = 2;`): no semantic purpose beyond the literal. Byte-identical program behaviour to `*(s16 *)(...) = 2;`. Measured: s36 form C (same holder, no dead store) produces bit-identical output to the bare literal (both sandbox 2). It is a codegen carrier, and it is annotated as such.
- Construct 2 (`two = 3;`): no semantic purpose whatsoever. The value is never read; `flow.c` `propagate_block` deletes the insn (measured: pseudo 90 appears 3x in red.i.cse2 and 2x in red.i.flow, tmp/grind/func_80041188/s37/K/). Zero emitted bytes; build_insns stays 132.
Both FAIL T1 by design — which is precisely why each is declared, annotated `/* FAKE: ... */`, and claimed under a frozen sanctioned family below rather than presented as ordinary C.

## T2 human-programmer
No. A programmer writing this function from its specification would write `*(s16 *)(ents + i * 0x68 + 6) = 2;` and stop. A reader will ask "why is `two = 3;` here?" — that is the cheat smell, and the answer is the FAKE annotation, not a semantic story. I am not claiming otherwise.

## T3 GCC-internals justification
Yes, explicitly, and the internals are named end-to-end from dumps rather than guessed:
`loop.c` `count_loop_regs_set` records `n_times_set` per pseudo inside the loop; `scan_loop` builds a movable only when the destination's `n_times_set == 1` (the consecutive-invariant-sets special case does not apply — the two sets are separated by the store). With a second, non-consecutive set present, no movable exists, so `move_movables` cannot hoist loop1's `(set (reg:HI 90) (const_int 2))` to block 0. Left in loop1 the temp is block-local and call-free, and `local_alloc` seats it in `$v0` — target's register. Without it, the hoisted pseudo is live 92 insns across 7 calls, `global_alloc` has no free callee-saved seat, and reload rematerialises the REG_EQUIV constant into `$t0` (MIPS defines no `REG_ALLOC_ORDER`). Measured: red.i.loop for the accepted form has ZERO `moved to` lines; P1r's has exactly one (`Insn 152: regno 121 (life 1), move-insn savings 1  moved to 312`).
This is the cheat SIGNAL, acknowledged as such. It is admissible only because the construct sits inside a frozen sanctioned family whose whole premise is a named GCC-pass mechanism, with the annotation and the exhaustion ledger both present.

## T4 permuter/search provenance
No permuter, no auto-search, no campaign was run this session. Both constructs were derived by reading `loop.c`'s movable preconditions after s36 closed the other four dimensions (threshold, spelling, gates (1)(2)(3)), and were then confirmed against cc1 `-da` dumps. The spelling was not chosen to slip past detectors: three neighbouring spellings were measured and REJECTED this session with their mechanisms named (`two = two;` -> 2, expander elides the self-move; `two = 2;` re-store -> 2, cse1 deletes it before loop.c; `two = (s16)(i+1);` -> 77 at 135 insns, computed source materialises).

## T5 family check
Construct 2 is a dead store to a LOCAL — the literal first-listed shape of the frozen `dead-store` family, not an analogy to it. Construct 1 is a constant-holder scalar local — the literal first-listed shape of the frozen `constant-holder / dead scalar local` family. Neither is a respelling of a family forbidden for this function. Checked against this function's BANNED list: the banned construct is the `do { ... } while (0);` half-loop1 wrap entered by an external `goto` to an interior label — absent here (there is no `do { } while (0)` anywhere in the diff; loop1's `do { } while (i < 0x12)` is a REAL loop with a real condition, which is the chassis, not a wrap). The three banned precedent lines (do-while-zero-exception.md:99, staged-value-reused-variable.md:46, docs/reference/sotn-construct-index.md:582) are not cited here. Neither construct is a register pin, hardcoded-`$N` asm, scheduling barrier, volatile coercion, alias rename, dead local ARRAY, frame coercion, width cast, or a chain-extender.

## T6 naming-announces-intent
The local is named `two`. It is not in the announce-intent set (`pad`, `_pad`, `dummy`, `unused`, `spill`, `sp_*`, `_buf`, `tail`, `slack`, `_frame_pad`) and its use is a real store, not a discard, an unused declaration, or an address-of. `two` is a value-descriptive constant-carrier name of exactly the kind the constant-holder rule's own SOTN evidence ships (`s16 three = 3;`, `s16 one = 1;` — .claude/rules/named-local-fake-exception.md:38). The record field's semantics are not established in this repo, so an invented semantic name would be an unevidenced claim ([[names-require-evidence]]); a value-descriptive name is the honest choice.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: dead store / self-assignment to a LOCAL (construct 2, `two = 3;`)
  SCOPE: "Ordinary-C assignment statements inside a function body whose stored value is never read (GCC DCEs the store; its existence influences RA / scheduling / flow analysis upstream of DCE)"
  PRECEDENT: docs/reference/sotn-construct-index.md:65

  FAMILY: constant-holder / dead scalar local (construct 1, `s16 two;` + `two = 2;`)
  SCOPE: "a local variable whose only purpose is codegen influence — holding a constant in a register across calls/statements, or existing as a declaration that biases register allocation — is a sanctioned last-resort matching lever under the prerequisites below."
  PRECEDENT: .claude/rules/named-local-fake-exception.md:38

ANNOTATION-CONFORMANCE:
  /* FAKE: constant-holder carrying loop1's record-flag value; mechanism: gives loop.c's scan_loop a user pseudo it can count sets on. lever-exhaustion: memory/grind/func_80041188/hypotheses.md s36 (five literal/holder spellings A,C,E,F,G all measure 2) */
  /* FAKE: dead store, never read; mechanism: loop.c count_loop_regs_set sees n_times_set == 2 so scan_loop builds no movable and move_movables cannot hoist the constant out of loop1 (flow.c propagate_block then deletes this store, zero emitted bytes). lever-exhaustion: memory/grind/func_80041188/hypotheses.md s32-s36 */
Both carry what + named GCC pass + a lever-exhaustion pointer. Lever exhaustion for this function is 36 prior sessions of ledger: memory/grind/func_80041188/hypotheses.md (s36 closes four of the five movable dimensions by measurement; this construct is the fifth), memory/grind/func_80041188/evidence.md E-s36-4/5/6/7, and 132 banked rejected forms in memory/grind/func_80041188/rejected/.
