# SELF-VET — func_8001979C  (session 8, rederive; sandbox --disable all == 0)

CONSTRUCTS: (1) per-iteration index address `dst = base + i * 2;` / `dst2 = base + i * 2;` computed inside each bit loop; (2) `nd` named intermediate for the width subtraction, copied into `needed`; (3) `hi` reused to hold its own `0x20 - bits_left` shift amount before holding the value; (4) `val` named intermediate for the new `bits_left`, reused at the end of the function to hold the final zero; (5) the OR written as four statements with a named low-fragment intermediate per loop (`lo`, `lo2`), operand order preserved; (6) `neg2` named constant holder for the third loop's fill value; (7) `out` as the third loop's own walking pointer.

## T1 semantic purpose
- (1) YES, fully semantic. It is the store address; the loop cannot write element `i` without it. It replaces the previous form's pre-loop init plus a `dst += 2` duplicated into both if-arms, i.e. this construct REMOVED a codegen-only construct rather than adding one. Deleting it is not possible without deleting the stores.
- (2) NO observable effect: `nd` holds `w - bits_left` and its only use is `needed = nd;`. Codegen-only (measured 0 -> 4 when deleted).
- (3) NO observable effect: `hi = 0x20 - bits_left; hi = cur >> hi;` computes the same value as `hi = cur >> (0x20 - bits_left);`. Codegen-only (0 -> 16 when deleted; a fresh local for the same value is also 16, so the effect is the reuse).
- (4) NO observable effect: `val = 0x20 - needed; bits_left = val;` equals `bits_left = 0x20 - needed;`, and `val = 0; store val;` equals `store 0;`. Codegen-only (0 -> 4, and build_insns 77 -> 75 — the target's own `subu $v0,$t2,$a0 ; addu $a3,$v0,$zero` pair disappears without it, so it is what materialises two REAL target instructions).
- (5) PARTIAL. `lo` / `lo2` name a real sub-value (the fragment of the newly refilled word that completes the field), which a human writing a bit-unpacker plausibly names; but the SPLIT itself (placing `cur <<= needed;` between the read and the OR) has no observable effect and is codegen-only (0 -> 55 when folded back into one statement).
- (6) NO observable effect: `neg2 = -2; *(s32 *)(out + 0x110) = neg2;` equals storing the literal. Codegen-only (0 -> 2 when deleted).
- (7) YES, fully semantic: the third loop needs a walking pointer and this is it; the previous form reused `dst` for the same role.

## T2 human-programmer
- (1) YES — `p = base + i * 2` / `arr[i]` addressing is the ordinary way to write an indexed store loop; arguably MORE natural than the previous duplicated-increment biv.
- (7) YES.
- (2), (3), (4), (5-split), (6) — NO. A reader would ask "why is this here?" of each. They are declared as codegen levers, not as program logic, and are annotated as such in the source (`/* FAKE: ... */` on (3), (4), (6); (2) and (5) are the same named-intermediate mechanism and are described in the candidate.c header).

## T3 GCC-internals justification
YES for (2)-(6), and this is stated openly rather than disguised: (3) global.c `allocno_compare` floor_log2(nrefs) bucket; (4) cse.c:7454 cheapest-register rewrite blocked by make_regs_eqv's last-use test at cse.c:856; (5) sched.c:2464's final `INSN_LUID` tie-break; (6) global.c `find_reg` conflict graph; (2) measured allocation pressure in the same allocno contest. NO for (1) and (7), whose justification is program logic (the address of element `i`). Because (2)-(6) are GCC-internals-justified they are submitted ONLY as members of the sanctioned families claimed below, with the lever-exhaustion ledger cited.

## T4 permuter/search provenance
NO permuter was run this session. (1) was derived by reading how loop.c places hoisted movables versus reduced-giv initialisations, then written by hand; (2) and (5) originate in sessions 4 and 7 (a permuter proposal in (2)'s case) but each was re-derived, re-minimised by hand and re-measured this session on the new chassis, and each survives deletion testing on its own. No construct here passes only because a detector misses its spelling; every one is disclosed.

## T5 family check
- (1) and (7): no family — ordinary C.
- (2), (5): "Named-intermediate declaration order" (frozen SOTN-accepted list).
- (3), (4): "Variable reuse for codegen control" (frozen SOTN-accepted list).
- (6): "Constant-holder / dead scalar locals" ([[named-local-fake-exception]], owner ruling 2026-07-01) — a LIVE constant-holder, i.e. the `s16 three = 3;` shape, not a dead declaration.
- NOT in the diff, and deliberately: no register pin, no `__asm__`, no volatile of any kind, no alias rename, no dead store to a local or param, no dead conditional store, no unused local (the generator prunes unused declarations), no `do {} while (0)` wrapper, no `if (1)` wrapper, no opaque constant variable (the s4/s5 `0x20` holder axis is measured dead and is absent), no OR-operand swap or reparenthesisation (forbidden by .claude/rules/or-tree-shape-shift.md — operand order in `hi | lo` is target's order and is preserved), and no duplicated statement into arms (session 8 REMOVED the duplicated `dst += 2` that sessions 2-7 carried).

## T6 naming-announces-intent
No name in the diff is `pad` / `dummy` / `unused` / `spill` / `tmp` / `slack` / `_buf`. `nd`, `val`, `lo`, `lo2`, `neg2`, `hi`, `dst`, `dst2`, `out` all name the value they hold. Every one of them is READ in the emitted code — there are no address-of uses, no `(void)` discards and no write-only locals in this function.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Variable reuse for codegen control  [constructs (3), (4)]
  SCOPE: "**Variable reuse for codegen control** ([[defeat-licm-hoist-var-reuse]]): reusing one C variable for two unrelated values to influence loop-invariant detection or RA. SOTN ships `idxSub = idxSub;` and `randy = basePoint.x; baseX = randy;` with "FAKE but makes register allocation work" comments."
  PRECEDENT: `.claude/rules/no-new-park-categories.md:170`

  FAMILY: Named-intermediate declaration order  [constructs (2), (5)]
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID. SOTN's `randy` chain in `src/weapon/w_037.c` is the same mechanism."
  PRECEDENT: `.claude/rules/no-new-park-categories.md:189`

  FAMILY: Constant-holder / dead scalar locals  [construct (6)]
  SCOPE: "**Constant-holder / dead scalar locals** ([[named-local-fake-exception]]) — SOTN `s16 three = 3;`, `s32 zero = 0; // needed for PSP`, constant-holder named `fake`; `new_var` in 9 committed files. Arrays / frame coercion remain forbidden."
  PRECEDENT: `.claude/rules/no-new-park-categories.md:229`

ANNOTATION-CONFORMANCE:
  /* FAKE: hi carries its own shift amount before the value, mechanism: global.c allocno_compare (hi crosses the floor_log2 nrefs bucket and keeps $v1), lever-exhaustion: memory/grind/func_8001979C/hypotheses.md [s2] H2-B, [s3] H3-A, [s8] v2_amt */
  /* FAKE: new bits_left routed through val, mechanism: cse.c:7454 cheapest-register rewrite blocked by make_regs_eqv's last-use test at cse.c:856, lever-exhaustion: hypotheses.md [s1] H-C, [s6] H6-A, [s8] v2_no_val */
  /* FAKE: named constant holder for the fill value, mechanism: global.c find_reg conflict graph (a separate allocno for -2 is what puts the fill pointer in $v0), lever-exhaustion: hypotheses.md [s5] H5-B, [s6] H6-A, [s8] v2_no_neg2 */
  Each carries what + a named GCC pass as mechanism + a lever-exhaustion pointer into the per-function ledger (7 prior sessions; the specific alternatives for each construct are enumerated with their measured scores in candidate.c's header and in hypotheses.md). Constructs (2) and (5) sit in the named-intermediate family, whose 2026-06-02 sanction carries no annotation prerequisite; they are documented in the candidate.c header rather than annotated inline, and the reviewer should treat that as a deliberate choice, not an omission — if the Judge wants inline `/* FAKE */` on them too it is a one-comment fix.
