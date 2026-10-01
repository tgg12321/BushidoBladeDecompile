# Hypothesis ledger — func_800770B8

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: INCLUDE_ASM. `candidate.c` scores 0/175 via SelWork members but relies on: an empty `do { } while (0)` (sanctioned; 5 without), a `sym` row pointer + same-value re-set (pointer-alias / dead-store; 19 and 18 without), and two-valued `p_old` (list ptr, then new work ptr) + dead restore `p_old = prev;` - owner Q66 REFUSED that (`docs/grind/owner-rulings-2026-09-26.md:701-704`). Variant k0 (no restore) = 2/175: only the base reg of the 0x30/0x34 clears differs ($s1 vs target $v0), a cse effect. Separate list/work locals = 20/175 (one callee-saved reg lost).
- CONSTRAINTS: Ruling 11 (B)(1) bans a reused var with a dead write (`.claude/rules/reused-local-necessity.md:22`); Ruling 4 bans carriers whose extra write is dead. Q66: re-measure after Q65 lands. The f1C/f20 word clears (`sw` at 0x800772BC/C0) need Q46 unions, added only together with this function. The `D_8009BD21` row in undefined_syms_auto.txt retires on landing.
- BLOCKER: one register carries the list ptr then the result (`addu v1,s1,zero; addu s1,v0,zero` at 0x80077124). The 2 points are cse treating the reloaded D_800A36A0 as equal to p_old; Q66 records the gp model may change this.
- PLAN:
  1. Wait for Q65 (`memory/grind/q65-adoption/HANDOFF.md`; regenerate with series.sh; step 15 is the gp switch).
  2. On the merged text1b_b.c add the f1C/f20 unions (and f3C if not already landed - see func_80075F80); re-measure k0, v7/m1-m3 and candidate.c.
  3. If k0 reaches 0: Ruling 11 package (named-pass dump proof, honest generic name, no dead write).
  4. Otherwise: owner question (Q66 offered "Allow narrowly"), or new modalities - permuter from k0, cse-dump study of the equivalence class.
- DEPENDS: after Q65 and after the f3C union landing.
- ODDS/LANE: 2+ sessions post-Q65, ~30% [I]. Manual. Last in the text1b_tu2 cluster.
