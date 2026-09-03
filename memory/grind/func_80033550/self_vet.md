# SELF-VET — func_80033550  (s16b, 2026-09-03, solver modality)

Written for the INTEGRATION-HANDOFF form banked in candidate.c (canonical
aggregate-merge spelling). NOT a candidate-ready submission: the fix cannot be
staged from a grind session because it edits include/code6cac.h,
undefined_syms_auto.txt and named_syms.txt. The vet is written now so the
landing session / Judge inherits it complete.

CONSTRUCTS: (1) canonical aggregate declaration `typedef struct { s32 x, y, z; } LeafPos; extern LeafPos D_80107850[6];` in include/code6cac.h replacing three per-word `extern s32` splat scalars, with the per-word symbols deleted from undefined_syms_auto.txt and named_syms.txt; (2) parameter retyped `s32 *arg0` -> `LeafPos *arg0`; (3) `D_80107850[i] = *arg0;` (struct assignment); (4) `*(&D_800A3918 + i)` byte-flag indexing (pre-existing in every banked form since s1, unchanged).

## T1 semantic purpose: Every construct is load-bearing. (1) declares the actual storage layout the original binary uses — six 12-byte records, proven by func_800335D8's `addiu $s2,$s2,0xC` table walk; (2) gives the parameter the type the call site actually passes (a pointer to one such record); (3) IS the function's whole semantic payload — copy the caller's record into the free slot; deleting any of them changes what the function does. Nothing here is behaviour-neutral decoration.
## T2 human-programmer: Yes — this is the form a human writing the original would produce. A programmer with the spec ("find the first free leaf slot, mark it used, store the caller's position record there") writes a record type, an array of six, and `table[i] = *src;`. The three-scalar-load/three-scalar-store spelling the ledger ground for fifteen sessions is the LESS natural form; it exists only as an artifact of splat inventing one C symbol per word.
## T3 GCC-internals justification: The GCC mechanism (block-move expansion keeping the source address live, which grows hard_reg_conflicts[72] so find_reg pass-0 lands on $a3) EXPLAINS why the natural form matches, but it is not the reason the construct is written this way. The program logic alone justifies every line: a 12-byte record table indexed by slot, assigned from the caller's record. No construct is present that the semantics do not already require, and nothing is shaped to a pass's internals.
## T4 permuter/search provenance: No permuter, no automated search. The form came from reading asm/funcs/func_800335D8.s (stride-12 base-register walk) and named_syms.txt:1556 (committed 12-byte-stride census row) — i.e. from the object model, not from a spelling search. It passes detectors because there is nothing to detect, not because a detector missed a spelling.
## T5 family check: The declaration merge is squarely the sanctioned "per-word splat symbol -> aggregate merge" family (owner ruling 2026-08-17), claimed below with all five prongs satisfied. The rest (a struct assignment, a retyped parameter, an array subscript) is ordinary C and belongs to no coercion family. Explicitly NOT present: the BANNED per-use byte-pointer pun `*(Word3 *)(((u8 *)(&D_80107850)) + i * 12) = *(Word3 *) arg0;` — that spelling is what this form replaces, and the reason it is replaced is precisely that it dodged prongs (c) and (d) of this family.
## T6 naming-announces-intent: No pad/dummy/unused/spill/slack names anywhere. `LeafPos`, `x/y/z`, `arg0`, `i` — all name real values. Every declared entity is read and written.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: per-word splat symbol -> aggregate merge (owner ruling 2026-08-17)
  SCOPE: "two or more splat-invented `D_<addr>` scalars may be replaced by a single aggregate declaration."
  PRECEDENT: .claude/rules/no-new-park-categories.md:238
  PRONGS:
    (a) object model established by evidence independent of and predating this
        session: named_syms.txt:1556 (committed naming census — "12-byte stride
        per leaf, 6 entries = 72-byte position array") and base-register stride
        indexing in the original binary at asm/funcs/func_800335D8.s
        (&D_80107850 -> $s2, `addiu $s2,$s2,0xC` per iteration, six iterations
        bounded by D_800A391E = D_800A3918+6).
    (b) the merged declaration reflects that documented shape: a 6-entry table
        of 12-byte records, `extern LeafPos D_80107850[6];`. The stride is
        carried by the type, not by a magic number — the use site is
        `D_80107850[i]`, with no `i * 12` anywhere.
    (c) complete: D_80107854 and D_80107858 are removed from include/code6cac.h,
        from undefined_syms_auto.txt:995-996 and from named_syms.txt:2562-2563,
        leaving exactly one C handle per storage location. Measured: full build
        still links and SHA1s to the oracle with all three deletions applied.
    (d) spelled at the canonical declaration in the shared header
        (include/code6cac.h), never TU-local, no per-use pointer pun.
    (e) byte-neutral for every other consumer: the only other reader of the
        table (func_800335D8) is still INCLUDE_ASM and untouched; no C consumer
        of the per-word symbols exists (grep, this session); `verify-oracle`
        build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle.
        Layer-2 cheat-reviewer still owed at landing time.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. The aggregate-merge family does
not mandate a /* FAKE */ annotation (it is a declaration-correctness family, not
a last-resort codegen lever), and this form contains no dead store, no wrap, no
constant holder, no alias and no pad. The `do { } while (0)` wrap the previous
candidate carried is deleted along with the scalar-triple tail.
