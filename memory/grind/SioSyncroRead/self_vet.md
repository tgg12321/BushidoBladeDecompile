# SELF-VET — SioSyncroRead

CONSTRUCTS: volatile extern decls (D_800F1AFC, D_800F1AE0), volatile base pointer `flag` (+ loop-local copy `st`), FAKE pointer handles p_ae2 / p_ae0 / p_b04 / p_b04a / p_b04b, duplicated return compute into cleanup_A and cleanup_B arms, sibling SioAnsyncRead `flag` local aligned to `volatile s32 *`

## T1 semantic purpose
- **volatile decls D_800F1AFC / D_800F1AE0**: semantic, not coercive — both are Sony LIBCOMB COMB module state mutated at interrupt time (HandleSio); the Judge's PASS ruling (docs/grind/decisions.md:11002) verified the census prong (symbols referenced only by COMB members) and the ground-truth codegen prong (target's own unfolded lui/addiu/lhu-0 at SioSyncroRead.s:22-24 vs folded non-volatile sibling AE6; expand_increment dead-reload fingerprint at SioSyncroRead.s:118-121 for AFC). Volatile IS the original semantics.
- **`volatile s32 *flag`**: real multi-use base pointer over the AFC/B00/B04 state block (flag[0]=busy flag, flag[1]=buffer, flag[2]=remaining length); every access through it is a real read/write the function's spec requires. `st` is the loop-scoped copy of the same base — identical to the committed twin's spelling (SioSyncroWrite, main.c ~3055).
- **FAKE handles**: each is read/written exactly where the function's spec requires the underlying global access — the value flow is real; only the address-materialization spelling is match-motivated, which is why each carries a FAKE annotation.
- **duplicated return compute**: `return r_arg1 - *p_b04x;` is the function's real return value on each path (bytes read minus bytes remaining); duplication into the two cleanup arms is byte-neutral (cross-jump re-merges to the single .L8008C14C tail — word-level diff verified clean in s1, re-verified 160/160 score 0 this session).
- **SioAnsyncRead flag alignment**: removes a volatile-discard introduced by the shared decl change; measured byte-neutral (0, 24/24).

## T2 human-programmer
Yes for the structure: a Sony SIO driver author naturally writes a busy-flag check, a state-block base pointer, nested poll loops on the SIO status register, a retry callback, and `requested - remaining` as the return. The FAKE handles and the duplicated return are the match-motivated residue — each is annotated as such per the sanctioned-family conventions, which is exactly the disclosure those families require. The committed twin SioSyncroWrite (COMPLETED-C 2026-08-19) ships the same construct set.

## T3 GCC-internals justification
The FAKE constructs' justifications DO reference GCC internals (combine.c symbol-fold defeat via MEM_VOLATILE_P; flow.c reg_n_refs / global.c allocno_compare lift) — that is the defined disclosure format of their sanctioned families (pointer-alias-fake-exception prereq 2 REQUIRES naming the GCC-pass interaction; duplicated-statement-into-arms likewise). The volatile decls themselves are justified by program semantics (IRQ-mutated state), not internals.

## T4 permuter/search provenance
None. No construct came from permuter/auto-search: the body was hand-derived in s1 from the matched twin's committed constructs, and the s2 respelling was mandated verbatim by the Judge's ruling.

## T5 family check
- volatile decls → Ruling-4 owner-granted CLASS (docs/closer/rulings.md:68) via the function-specific PASS at docs/grind/decisions.md:11002; spelling conforms to its constraint (decl-level, pattern 3; nothing injected at pointer/cast over a non-volatile decl; the session did NOT touch volatile_extern_allowlist.txt — rows are driver-staged per the same constraint).
- pointer handles → pointer-alias-fake-exception (sanctioned family; forbidden variants avoided: no asm("Sym") renames, no volatile-cast over non-volatile decls — every handle copies volatility from a granted decl).
- duplicated return → duplicated-statement-into-arms (sanctioned family; not a dead store — the statement is real on both paths; byte-neutral re-merge verified).
- No forbidden-family construct present: no pins, no __asm__, no barriers, no dead stores, no invented locals, no unused arrays. The rule-era body's `la`-injection asm blocks and memory barriers were DELETED by this change.

## T6 naming-announces-intent
Names are descriptive of role (flag, st, count, retries, pkt_len, spu, cb, saved, p_ae0/p_ae2/p_b04 = address handles named for their symbol): no pad/dummy/unused/spill/slack names; no declaration-only or address-of-only uses.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: pointer-alias-fake-exception (p_ae2, p_ae0, p_b04, p_b04a, p_b04b)
  SCOPE: "a local pointer that provides a second C handle to a global — where using the global directly would be semantically identical — is a sanctioned last-resort matching lever under the prerequisites below."
  PRECEDENT: .claude/rules/pointer-alias-fake-exception.md:13
  (prereq 1 exhaustion: direct-global form measured 158i/24 — evidence.md [s1] variant A; non-volatile handle measured 159i — hypotheses.md [s1-H2]. prereq 2: combine.c fold defeat via MEM_VOLATILE_P, named in each annotation. prereq 3: every handle carries /* FAKE: ... */. Twin committed precedent: src/main.c:3045 SioSyncroWrite p_ae2 handle.)

  FAMILY: duplicated-statement-into-arms (return compute in cleanup_A + cleanup_B)
  SCOPE: "Writing the SAME real statement in two or more control-flow arms — instead of sharing one copy via a label/goto — is a legitimate matching technique, including when: GCC's jump2 cross-jump re-merges the copies so the final bytes are identical to the shared-label form, and the duplication's surviving effect is the extra `reg_n_refs` count flow.c records (allocno-priority lift for global RA)"
  PRECEDENT: .claude/rules/duplicated-statement-into-arms.md:14
  (prereq 1: the return compute is real on both paths. prereq 2 byte-neutrality: s1 word-level diff clean; s2 sandbox 0, 160/160. prereq 3 exhaustion: shared-label form measured 9/160 with the s3<->s4 seat swap; solver model tmp/ra_solver_work/SioSyncroRead.model.json shows the allocno_compare ordering no honest respelling changes — evidence.md [s1] mechanism 4. prereq 4: FAKE annotations present on both copies.)

  FAMILY: legitimate-volatile / Ruling-4 owner-granted class (extern volatile s32 D_800F1AFC; extern volatile u16 D_800F1AE0)
  SCOPE: "D_800F1AE0 qualifies under Ruling 4 (docs/closer/rulings.md:68, owner-granted CLASS). Both prongs verified independently, not taken from the ledger" — and, same ruling: "Same ruling extends to D_800F1AFC"
  PRECEDENT: docs/grind/decisions.md:11002
  (Spelling constraint of that ruling satisfied: volatility on the DECLs only; every pointer copies; allowlist rows are driver-staged at integration handoff, untouched by this session. Sibling grant precedent: D_800F1AE2 allowlist row 46; D_800F1B00/B04 rows 2026-07-10.)

ANNOTATION-CONFORMANCE: five /* FAKE: ... */ annotations emitted, each carrying what + mechanism + lever-exhaustion:
- "FAKE: redundant second handle to D_800F1AE0, mechanism: MEM_VOLATILE_P blocks combine.c's single-use symbol-address fold into the load (non-volatile handle measured folded: 159i vs target 160), lever-exhaustion: memory/grind/SioSyncroRead/hypotheses.md [s1-H2]; volatility copied from the granted decl (Ruling-4 grant, docs/grind/decisions.md 2026-08-25 10:34)"
- "FAKE: duplicated return compute (re-merged by cross-jump), mechanism: flow.c reg_n_refs priority lift for the saved-arg1 pseudo in global.c allocno_compare, lever-exhaustion: s1's shared-label form measured 9/160 with s3<->s4 seat swap (solver model tmp/ra_solver_work/SioSyncroRead.model.json: p74 3refs/len102 vs p78 3refs/len56)" (cleanup_A; cleanup_B carries the same with "as cleanup_A duplicate")
- "FAKE: redundant second handle to D_800F1AE2, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: twin precedent SioSyncroWrite s4-M1 (main.c:3044); s1's direct-global form measured 158i/24 (variant A)"
- "FAKE: redundant second handle to D_800F1B04, mechanism: combine.c symbol-fold defeat (address forced into a pseudo, so lui/%lo is not folded into the load base), lever-exhaustion: twin precedent SioSyncroWrite s4-M4 (main.c:3104); s1's direct-global form measured 158i/24 (variant A)"
The volatile decls carry no FAKE annotation by design: decl-level volatility under a Ruling-4 grant is semantic (mmio-volatile-type-level / legitimate-volatile convention — "NOT fake — no annotation"), tracked via the allowlist instead.
