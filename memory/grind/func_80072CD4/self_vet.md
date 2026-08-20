# SELF-VET — func_80072CD4

Diff under vet: `src/text1b.c:5865` — the `INCLUDE_ASM("asm/funcs", func_80072CD4);` line is
replaced by a `POLY_G4` typedef plus the function body (identical to
`memory/grind/func_80072CD4/candidate.c` with its header comment stripped, and byte-identical to
the banked `rejected/rederive_polyg4_struct_perarm_score0_banned_family.c` body that the Judge
rulings of 2026-08-20 06:35 and 06:54 examined). Nothing else in `src/` or `include/` is touched.

Measurements taken this session with that diff in place:
* `& tools/wteng.ps1 main sandbox func_80072CD4 --disable all` → **score 0**, build_insns 79 ==
  target_insns 79, scorable true, rules_dropped 0 (artifact
  `tmp/grind/func_80072CD4/s8/sandbox_struct_s8.json`).
* `& tools/wteng.ps1 main build` → `sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle,
  **MATCH** (the FINAL CALL required by the standing judge constraint).

CONSTRUCTS: none

## T1 semantic purpose: Every statement in the body is a live field write or a call that the
function's specification requires. `arg1` is a libgpu `POLY_G4` primitive (offsets touched are
exactly its four RGB triples 0x04/0x05/0x06, 0x0C/0x0D/0x0E, 0x14/0x15/0x16, 0x1C/0x1D/0x1E; the
function calls `SetPolyG4`/`SetSemiTrans` on it, `AddPrim`s it, and returns `arg1 + 0x24` ==
`sizeof(POLY_G4)`). Each inner branch assigns the complete gouraud colour of vertices 0 and 1 for
that game-state case; the unconditional statements assign vertices 2 and 3. Remove any one of the
12 stores and the primitive is drawn with a wrong colour channel — each has an observable effect
on the game's output. 12 C stores produce 12 emitted stores (build_insns 79 == target 79); there
is no dead store, no byte-neutral copy, no discarded value, no holder, and no local of any kind.

## T2 human-programmer: Yes. Given the specification "fill in a POLY_G4's four vertex colours,
choosing the colour of vertices 0 and 1 from a flag", a programmer writes each vertex's r/g/b
together in ascending field order — which is exactly what the COMPLETED-C `func_8003553C` in
`src/code6cac_b2_pre.c:167-176` does with the same typedef in this same repo. A reader asks no
"why is this here?" question about `->r0 = 0xFC; ->g0 = 0xC3; ->b0 = 0x1E;`. The construct that
WOULD draw that question is the opposite spelling — lifting the two reds out of the colour
assignments into a shared tail behind an `int fc_const` — and that is the previously banked
blob-model form, preserved as `fallback_floor4.c`, which is not what is landed.

## T3 GCC-internals justification: The spelling is not justified by a GCC pass. Its justification
is the data model: these are per-vertex colour components of a POLY_G4, so they belong in the
branch that chooses the colour. The ledger does RECORD a codegen consequence (jump2 cross-jumps
the arms' common tail to the join label, which is target's merge head), and the s5-forensics L1/L2
laws explain why the blob-model spellings are bounded away from target — but that is a post-hoc
explanation of a measurement, not the reason any statement is placed where it is. No allocator,
scheduler, `reg_n_refs`, `INSN_PRIORITY` or `label_num` reasoning is load-bearing for the shape:
the shape is the field layout, and the two red components sharing the value 0xFC is a property of
the artwork, not a lever.

## T4 permuter/search provenance: Not permuter output. The form was derived by reading the offset
set as the libgpu POLY_G4 layout and rewriting the function in the field order the COMPLETED-C
sibling `func_80072BC4` (`src/text1b.c:5822`) already uses in this same file. The permuter
campaigns of s4/s4b never produced it, and every construct it contains would survive any detector
because there is no construct: no local, no volatile, no asm, no barrier, no annotation.

## T5 family check: No forbidden family applies, and no sanctioned family is being spent, because
there is no construct to classify. Specifically not the duplicated-statement-into-arms family: a
duplicated-into-arms lever duplicates an UNCONDITIONAL common-tail statement into both arms so
that jump2 merges the copy away byte-neutrally. Here `->r0` and `->r1` are per-arm colour
components of two different colour sets that happen to share the red value 0xFC — they were never
unconditional statements, and the arms are not byte-neutral duplicates of each other (arm A writes
0xC3/0x1E and 0xC8/0x32, arm B writes 0xC3/0x50 and 0xDC/0x46). The banned
`rejected/dup4_0xc_into_arms.c` lever — an `int fc_const` holder plus `@4=fc_const; @0xC=fc_const`
injected mid-arm in a merge-order-driven sequence — is NOT present and remains rejected. The
banned_constructs entry (the self-issued 2026-08-20 05:46 decisions.md "ruling") is not re-declared,
not relied on, and this session wrote no ruling entry in docs/grind/decisions.md at all.
In-repo evidence that an un-hoisted repeated component on both arms is ordinary accepted C here:
`src/text1b.c:5840` and `src/text1b.c:5843` (COMPLETED-C `func_80072BC4`, zero rules, absent from
engine/queue.json) write the identical `*(u8 *)((s32)(arg1) + 0x1D) = 0xC3;` in both arms; and
this function's OWN target bytes keep the cross-arm repeat unmerged —
`asm/funcs/func_80072CD4.s:23-24` (arm A, 0x80072D28-2C) and `asm/funcs/func_80072CD4.s:32-33`
(arm B, 0x80072D48-4C) each emit `addiu $v0, $zero, 0xC3 / sb $v0, 0x5($s1)`.

## T6 naming-announces-intent: No name in the diff announces coercion intent. The only identifiers
introduced are the `POLY_G4` typedef and its libgpu field names (`tag`, `r0/g0/b0`, `code`,
`x0/y0`, `pad1..pad3`), copied verbatim from `src/code6cac_b2_pre.c:153-162`. There is no `pad`,
`dummy`, `unused`, `spill`, `tmp`, `holder` or `fc_const` variable — there is no variable at all.
(The struct's `pad1/pad2/pad3` members are the primitive's actual reserved bytes in the hardware
packet layout, never written by this function.)

SANCTIONED-FAMILY-CLAIMS: none
ANNOTATION-CONFORMANCE: n/a — no FAKE construct
