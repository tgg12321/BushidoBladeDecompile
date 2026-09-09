# Hypothesis ledger -- func_800335D8

## s1 (2026-09-08, recon) -- floor 47 (body A, retired form on new chassis) -> 6 (scorer floor; true 0 by oracle SHA1)

CONFIRMED
1. Declaration fix (pair structs for EBFC/EBFD and EA44/EA45, u8[6] for EBF4, TU arrays for E914/EA00/A3750):
   47 -> 27 (bodies A -> C). Mechanism: no `idx = cat*2` pseudo across the call; cat stays in $s1.
2. `buf[i] = 1` (store the constant, not `type`) in the type-1 arm: the constant `1` becomes a 2-use movable and
   loop.c hoists it into $s5 (combine_movables match), 173 -> 176 insns. A -> B1: 47 -> 42.
3. Loop 1 index form: signed `slt` and constant bound D_800A3918+6. 47 -> 46 (B3); carried into D.
4. Loop 2 index form over pointer local `buf` + pointer biv `tbl` + per-iteration `data = tbl + 1` + `0x7B + i`:
   20 (D) -> 16 (F2) -> 14 (F3, data/tbl increment order) -> 14 (G1, counter giv) -> 12 (H1, data giv).
5. Inline ratio expression `((scount - limit) << 10) / (D_8008EA44[adj].b * 30)`: 12 -> 6 (I2/I3); the remaining 6
   are the unresolved `+1` addend pairs (scorer gap), oracle SHA1 matches with the merge applied.

KILLED (all kill_scope=instance; chassis -mel -msoft-float, pair-struct decls applied, no FAKE constructs)
- B4 loop 2 direct `D_800A3750[i]` + `tbl[i*2]`/`tbl[i*2+1]`: 62 (param giv folded to `lbu 1($tbl)`, no $18).
- E `for (i...; i++, p++, data += 2, tbl += 2)` with `*p`: 33 (i not eliminable, stays a counter).
- F1 three index givs (buf[i], tbl[i*2], data[i*2]): 41, 179 insns.
- G3 `tbl[i*2]` + `(tbl+1)[i*2]`: 38 (givs combined, loop dump `giv at 421 combined with giv at 463`).
- H2 `limit` computed before `scount`: 21 (block-wide seat shuffle).
- I1 `diff` computed before the branch: 40, 178 insns.
- A2 parallel u8 arrays `D_8008EBFC[cat*2]` / `D_8008EBFD[cat*2]` (the split-scalar spelling): 22 (`la D_8008EBFD`
  hoisted into $s4); also NOT the object model.
- K1 fully structured if/else dispatch with the default call duplicated into both arms: 19, 173 insns.
- D2 `(s32)` casts on the pointer compares: -1 each but cosmetic; superseded by the index forms.

FRONTIER (s1, now spent -- see s2)
1. Land the banked form via the aggregate-merge integration surface (include/code6cac.h, undefined_syms_auto.txt,
   named_syms.txt, TU externs) -- bytes already proven by full-build SHA1; needs the Judge/driver to accept
   verify-oracle as the byte proof because the sandbox cannot reach 0 on this reference (scorer gap).
2. If a scorer-visible 0 is required first: engine/score.py `_symtab()` must also learn dlabel addresses from
   asm/data/7D920.data.s (or build/bb2.map). Not a session surface.
3. Nothing else is open: 176/176 insns, every register and emission order matches target.

## s2 (2026-09-08, recon) -- floor 6 (scorer artifact, s1) -> 0 (sandbox AND oracle SHA1, edits in place)

CONFIRMED
6. The s1 "+6 scorer floor" was a symbol-TABLE gap, not an engine-code gap: adding the merged aggregates' BASE
   address rows (`D_8008EA44 = 0x8008EA44;`, `D_8008EBFC = 0x8008EBFC;`) to undefined_syms_auto.txt -- an in-scope
   splat-symbol-config declaration of the same address the dlabel defines -- lets score.py `_resolve_named_pair`
   normalise the C side's `%lo(D_8008EBFC+1)` and the reference's `%lo(D_8008EBFD)` to the same @lo(0xebfd) token.
   6 -> 0; ablation (base rows removed, nothing else changed) 0 -> 6; oracle SHA1 unchanged either way. The per-word
   rows stay with the prong (c) amendment-2026-09-03 alias suffix while asm/funcs/func_800335D8.s (their only
   referrer, the sandbox reference side) exists. This retires s1 frontier items 1 and 2 without touching engine/.
7. `&D_80107850[i].x` == `(s32 *)&D_80107850[i]` byte-for-byte (K3 vs K2: sandbox 0 / oracle SHA1 for both). K3 is
   the banked body and the body in src/ (removes the brief's auto-scan use-site-cast flag with ordinary C).
8. Sibling transplant (func_80033550 s17 body on main; func_800325E0 s1 canonical body on main): the shared
   declarations `D_800A3918[6]` / `LeafPos D_80107850[6]` and the `D_80107850[i]` record spelling were already the
   candidate's spelling; measured inside the 0.

KILLED: none this session (K2-with-cast is byte-identical, not disproven -- nothing added to rejected/).

FRONTIER: none -- candidate-ready (sandbox 0 this session, edits in place in src/ + the granted integration surfaces).
Post-landing housekeeping for the OPERATOR (not a session surface): once asm/funcs/func_800335D8.s is retired the
`D_8008EA45` / `D_8008EBFD` alias rows in undefined_syms_auto.txt can go (their suffix says so).
