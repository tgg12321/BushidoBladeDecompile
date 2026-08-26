# SELF-VET — func_80061250

Written by s4 (2026-08-26, recon modality) against the diff actually staged in the
working tree this session. All line numbers below were re-grepped AFTER the edits
were applied, so they resolve in the file a reviewer will read (the s1 layer-1 FAIL
was caused by pre-application line numbers; see evidence.md s2).

## The diff under review (three files, all inside the driver scope grant at
## tools/grinder/scope_allow.txt:34 — "func_80061250 src/text1b.c src/text1b_b.c
## undefined_syms_auto.txt", written by the driver on the Judge's
## integration-handoff ESCALATE, docs/grind/decisions.md:14520)

1. `src/text1b.c:3271-3298` — the `INCLUDE_ASM("asm/funcs", func_80061250);` line and
   the dead `extern volatile u8 D_800F1159;` above it are replaced by the C body of
   func_80061250.
2. `src/text1b_b.c:591` — the dead `extern volatile u8 D_800F1159;` declaration
   (zero uses in that TU) is deleted.
3. `undefined_syms_auto.txt:516` — the splat symbol line `D_800F1159 = 0x800F1159;`
   is deleted, so exactly one C handle survives for bytes 0x800F1159-0x800F115A.

Edits 2 and 3 are the "D_800F1159 retirement" the Judge recorded as a BINDING
constraint on integration ("Integrate the array-extern form ONLY together with the
D_800F1159 retirement", docs/grind/decisions.md:14543). They are pure deletions of
unreferenced declarations: after the INCLUDE_ASM disappears, the only remaining
references to D_800F1159 anywhere in the tree were those two dead externs and
asm/text1b.s, which is NOT linked (bb2.ld links build/src/text1b.o; verified s3 and
re-verified by the full clean build this session).

CONSTRUCTS: block-scope `extern u8 D_800F1154[];` array declaration; array indexing
`D_800F1154[5]` / `D_800F1154[6]` (reads, stores of 0, and two address publishes
`&D_800F1154[5]` / `&D_800F1154[6]`); a `s32 *v1 = (s32 *)&D_800F116C;` pointer local
to a global; a `goto check_one_zero` / `goto end` control shape with one inline fall-
through arm; a `s32 *p = arg0;` walking-pointer tail (`*p++` × 2 then `*p`).

## T1 semantic purpose
Every construct changes the function's observable behaviour.
- The array declaration + `D_800F1154[5]/[6]` accesses ARE the function's work: it
  tests two busy flags, clears them when both are set, and publishes the address of
  one of them to the text subsystem. Remove any of them and the behaviour changes.
- `v1` is dereferenced twice (`*v1 = 0x21000A` at the arm, and its value is stored to
  D_800A3468 which the other arm re-reads) — a live, read value, not a holder.
- The `goto` shape is the function's control flow (three distinct exits into a shared
  tail), not a wrapper: each arm publishes a different address and a different opcode
  constant (0x210009 vs 0x21000A).
- The `*p++` tail performs the three loads/stores that copy arg0[0..2] into
  D_800F1140/44/48. Nothing is dead; nothing is discarded.
No construct in the diff is byte-identical-with-or-without. PASS.

## T2 human-programmer
Yes — and demonstrably so, because human-written committed siblings in this same
translation unit already look exactly like this, and they predate every grind session
on this function:
- `src/text1b.c:3557` declares `extern u8 D_800F1154[];` and `src/text1b.c:3563`
  publishes `D_800F1154 + 3` from it (func_800619F0, committed and integrated).
- `src/text1b.c:3505` declares `extern u8 D_800F1152[];` and func_800618B4 indexes
  it [0]/[1] with the identical test-and-clear shape.
- `src/text1b.c:3589` declares `extern u8 D_800F1164[];` for the neighbouring block.
- `src/text1b.c:3370-3397` (func_8006156C, Match commit 450f69d1, Judge PASS
  2026-07-22) is the same function on a different flag pair: same `v1` pointer local,
  same D_800A3468/D_800F1178 publish, same nested if/goto flag test-and-clear, same
  `*p++` tail. Only the data differs (flags [5]/[6] vs [1]/[2], opcodes 0x210009/A vs
  0x210005/6, mask 0xFF0060 vs 0xFF8080).
A reader asks "why is this here?" about nothing in the body. PASS.

## T3 GCC-internals justification
The justification for the FORM is the object model, not a compiler pass. D_800F1154
is a byte buffer (naming census named_syms.txt:827 models 0x800F1154 as
`g_text1b_glyph_buf_b` — a buffer — and that census entry predates this function's
grind), the flags live at byte offsets 5 and 6 inside it, and the C says exactly
that. It is true that the ledger separately RECORDS a measured cse mechanism (a
compound-constant anchor keeps the base register instead of folding to absolute
addressing, evidence.md s1); that is a measurement of why the WRONG spellings lost,
not the reason this spelling is written. The array declaration would be the correct
declaration of this object even if the bytes did not depend on it. PASS.

## T4 permuter/search provenance
No permuter, no auto-search, no PERM_* macro was used on this function in any
session. The form came from reading the committed sibling func_8006156C and the
committed array declarations already in this TU, then from the layer-1 reviewer's own
prescription at docs/grind/decisions.md:14502 ("declare the D_800F1154 flag block as
a real header-level aggregate"). It survives detectors because it is honest, not
because of a spelling gap. PASS.

## T5 family check
Against the forbidden catalog, construct by construct:
- The array declaration + indexing is NOT the banned pointer-pun. The ban recorded
  for this function is `(&D_800F1154)[5]/[6]` (and `(&D_800F1154)+5/+6`) off a
  SINGLE-BYTE `extern u8 D_800F1154;` — a declaration that says "one byte" while the
  code walks five bytes past it. This diff contains no `(&D_800F1154)` expression at
  all; it declares the object as an array and indexes it in bounds of an incomplete
  array type, which is ordinary C. The grinder Judge ruled precisely this distinction
  at docs/grind/decisions.md:14506 ("That is not a respelling of the pun; it is the
  prescribed cure") and dispositioned it as an integration handoff rather than a
  family question.
- The pointer local `v1` is a live, dereferenced pointer that carries the function's
  own data — not a pointer-alias-for-codegen (and it is copied verbatim from the
  Judge-PASSed sibling at src/text1b.c:3371).
- No register-asm pin, no `__asm__` of any kind, no scheduling barrier, no volatile
  (the diff DELETES the two stale volatile externs), no alias rename, no dead store,
  no self-assign, no constant holder, no dead local, no local array, no
  do-while(0)/if(1) wrapper, no `(void)` discard, no naming-announces-intent
  identifier, no regfix/asmfix rule (0 rules keyed to this function).
PASS.

## T6 naming-announces-intent
Identifiers in the diff: `arg0`, `v1`, `p`, `check_one_zero`, `end`. `v1`/`p` are the
house style of every matched sibling in this TU; `check_one_zero`/`end` are
descriptive control labels. No `pad`, `dummy`, `unused`, `spill`, `tmp`, `slack`,
`_buf`, `_frame_pad`. Every declared name is read. PASS.

SANCTIONED-FAMILY-CLAIMS: none — this diff claims NO sanctioned-family exception. It
is ordinary C. The one family that could arguably be in play, "per-word splat symbol
-> aggregate merge", was examined by the grinder Judge and ruled INAPPLICABLE here on
the ground that nothing is being invented: the array declaration for this very symbol
is already committed on main and predates the grind (precedent
`src/text1b.c:3557`; ruling `docs/grind/decisions.md:14506`; sibling-shape precedent
`src/text1b.c:3370`; sibling Match commit `450f69d1`). The Judge additionally verified
that the full-block merge is mechanically impossible today because D_800F1158 /
D_800F115B / D_800F115C are still referenced by INCLUDE_ASM'd sibling .s files, which
is why this session performed the MINIMAL retirement (D_800F1159 only) the Judge's
constraint mandates and did not widen it.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. No construct in this diff belongs to
a family whose rule mandates a `/* FAKE */` annotation; every statement is semantic
program logic, and the sanctioned-family-claims section above is "none".

## Bytes, measured THIS session with these exact edits in place
- `& tools/wteng.ps1 main sandbox func_80061250 --disable all` → score 0,
  target_insns 59, build_insns 59, rules_dropped 0 (cheat_asm_stripped 169 is other
  functions' legacy debt; nothing was stripped from this body).
- `& tools/wteng.ps1 main build` → SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa ==
  oracle, MATCH — a full clean build WITH the D_800F1159 splat symbol deleted and both
  dead externs removed, which is what proves the retirement is safe.
