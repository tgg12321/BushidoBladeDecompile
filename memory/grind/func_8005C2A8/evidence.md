# Evidence bank — func_8005C2A8

## s1 (recon, 2026-09-14) — MATCHED (sandbox 0, full-build SHA1 == oracle)

OBJECT MODEL: the three DATA-MODEL-flagged symbols, resolved.
- D_800EFC38 @800EFC38 — MATCHES (measured: score 0 with this declaration).
  The census name/prose ("second effect state struct") is WRONG for this use
  site: it is the 16-entry VAB-header pointer table, indexed by vab id.
  Declared here as `extern s32 *D_800EFC38[];` — identical to the pre-existing
  declaration at src/text1b.c:2645 used by func_8005BF78, so no new object
  model was invented. Element field [3] (byte offset 0xC) is the VAB body size
  in SPU bytes; the function accumulates it across all 16 slots.
- D_800EFB38 @800EFB38 — MATCHES (measured: score 0). Parallel 16-entry s32
  array of SPU base addresses (SsUtGetVBaddrInSB results), indexed by the same
  vab id. Declared `extern s32 D_800EFB38[];` (same as src/text1b.c:2646). The
  census prose "16-entry s32 array; cleared in sync with g_effect_struct_b" is
  consistent with what this function does — it zeroes both slots together.
- D_800158CC @800158CC — MATCHES (measured: score 0). Already a real C object:
  `const char D_800158CC[20] = "vab id:%d mistake\n";` at
  src/text1a_b_pre_rodata.c:359. Declared here as `extern const char
  D_800158CC[];` and passed to printf as the format string.
No declaration fix was needed for any of the three, and no MISMATCH remained
once the body matched.

FACT (measured): the honest floor went 134 (whole-function INCLUDE_ASM) -> 16
(first C body) -> 13 (snd_VabOpen return-type fix) -> 0 (exit-arm orientation).
All four numbers are `sandbox func_8005C2A8 --disable all` on HEAD's chassis.

FACT (measured, s1): the sign-extension of the snd_VabOpen result is placed by
the RETURN TYPE OF THE CALLEE DECLARATION, not by the local's type.
  `extern s16 snd_VabOpen(s32 *, s16);` + `s16 id = snd_VabOpen(...)` =>
  no conversion at the assignment, so each read of `id` extends into a scratch
  register: `move s0,v0 / sll v0,s0,16 / sra v1,v0,16 / ... sll v0,v1,2`
  (3 instructions off target; total score 16).
  `extern s32 snd_VabOpen(s32 *, s16);` + `s16 id = snd_VabOpen(...)` => the
  narrowing conversion is part of the assignment and GCC 2.7.2 extends IN PLACE
  in the local's own pseudo: `move s0,v0 / sll s0,s0,16 / sra s0,s0,16 /
  ... sll v0,s0,2`, which is the target (score 13). Note src/text1b.c:2764
  defines snd_VabOpen as returning s16; the s32 prototype ahead of the use site
  is a deliberate local widening and is benign to the build (full-build SHA1
  unchanged, oracle verified this session).

FACT (measured, s1): the two exit arms are NOT interchangeable — physical block
order in the emitted body follows the SOURCE order of the arms exactly.
  Form A: `if (id == -1) { printf; return 0; } <tail>; return hdr[2]-hdr;`
     -> emits [tail][printf-block]: `beq` into the far printf block, plus a
        trailing `j` to the shared epilogue from the tail, printf falling
        through. 133 insns vs the target's 134. Score 13.
  Form B: `if (id != -1) { <tail>; return hdr[2]-hdr; } printf; return 0;`
     -> matches the target exactly (the target's `bne` at 8005C428 is this test
        with the arms as written here; the tail falls through into the epilogue
        and the printf arm carries the `j .L8005C494`). 134 insns. SCORE 0.
The two spellings differ by 13 instructions; only Form B matches. Form A is
banked at memory/grind/func_8005C2A8/rejected/inverted-exit-arms.c.

FACT: the signature is `s32 func_8005C2A8(s32 *hdr, s16 vabid, s32 arg2)`. The
`short` second parameter is load-bearing: the three separate `sll/sra 16` of
$s5 in the target (8005C2F0, 8005C30C, 8005C3C8) are the per-use extensions of
a promoted `short` param, with the raw incoming word parked in $s5.

FACT: the pre-existing conflicting prototypes for func_8005C2A8 in the same TU
(`extern s32 func_8005C2A8(s32, s32, s32);` at src/text1b.c:2446, 2493, 2538)
do NOT break the build under GCC 2.7.2 and do not perturb codegen — the oracle
build is byte-identical with the new three-argument pointer-typed definition in
place. Same for the snd_VabOpen prototype/definition return-type mismatch.

FACT (semantics, for future naming work): the function is "load a VAB into a
slot". It (1) returns 0 early when func_80077D00()[5] & 0xF == 3 and the vab id
is 5, (2) closes and zeroes any VAB already occupying the slot, (3) for a
non-zero id recomputes the SPU watermark D_800A3404 from D_800EFB38[0] plus
every live slot's body size and sets D_800A3408 = D_800A3404 - D_800A340C,
(4) calls func_8005C074(id, arg2) for non-zero ids, (5) self-relocates the
header's three internal offsets (hdr[0], hdr[1], hdr[2] += (s32)hdr), (6) calls
snd_VabOpen + SsVabTransCompleted(1), (7) on -1 prints "vab id:%d mistake" and
returns 0, else records the header in D_800EFC38[opened_id], advances the
watermark by hdr[3], stores SsUtGetVBaddrInSB(id) into D_800EFB38[id], and
returns hdr[2] - (s32)hdr (the header's byte size, which callers add to their
running pointer — see src/text1b.c:1128, 2510, 2516).

## s2 (recon, 2026-09-15) — re-matched WITHOUT either banned construct

CHASSIS: HEAD floor re-measured at dispatch = 134 (whole-function INCLUDE_ASM at
src/text1b.c:2662). The s1 ledger's "MATCHED" status did NOT survive: the s1 body
layer-1 FAILed (docs/grind/decisions.md, 2026-09-14 17:28) and main was reverted
to INCLUDE_ASM, so this session started from 134, not from 0.

OBJECT MODEL: the three DATA-MODEL-flagged symbols, re-resolved on the s2
chassis with the final body in place.
- D_800EFC38 @800EFC38 — MATCHES (measured: score 0 with this declaration).
  Declared `extern s32 *D_800EFC38[];`, identical to the pre-existing
  declaration at src/text1b.c:2645 used by func_8005BF78. It is the 16-entry
  VAB-header pointer table indexed by vab id (the census prose "second effect
  state struct" does not describe this use site); element [3] is the VAB body
  size in SPU bytes.
- D_800EFB38 @800EFB38 — MATCHES (measured: score 0). Declared
  `extern s32 D_800EFB38[];`, same as src/text1b.c:2646. Parallel 16-entry s32
  array of SsUtGetVBaddrInSB results indexed by the same vab id.
- D_800158CC @800158CC — MATCHES (measured: score 0). Already a real C object,
  `const char D_800158CC[20] = "vab id:%d mistake\n";` at
  src/text1a_b_pre_rodata.c:359; declared here `extern const char D_800158CC[];`
  and used as the printf format string.
No declaration fix was required for any of the three and none remained a
MISMATCH once the body matched.

FACT (measured, s2): the `vab` alias local banned after s1 was NEVER load-bearing.
Deleting `s16 vab; ... vab = vabid;` and reading the parameter directly at all
four former use sites holds `sandbox func_8005C2A8 --disable all` = 0.
Body banked at memory/grind/func_8005C2A8/candidate.c.

FACT (measured, s2): the second s1 construct — a prototype declaring the in-TU
VAB-open wrapper (defined at src/text1b.c:2707, glabel snd_VabOpen @8005C5A8) as
returning s32 while its definition said s16 — is removable as a CONTRADICTION
without losing the codegen it bought, because that callee's return type is not
decidable from its own bytes. Its body is
`return (s16)SsVabTransBody(a0[2], a1);` — the sll/sra $v0 at 8005C5F4 is emitted
by the EXPLICIT CAST, not by the return type. Measured: changing the definition's
return type s16 -> s32 (cast kept) leaves `sandbox snd_VabOpen --disable all` = 0.
So the definition and the forward prototype can both say s32, they agree, and
func_8005C2A8 still measures 0.

FACT (measured, s2): three probes, all `sandbox func_8005C2A8 --disable all` on
the s2 chassis with `verify-oracle` re-run on the last one.
  V1 = s1 body + definition widened to s32              -> score 0 (callee 0)
  V2 = V1 with the `vab` alias deleted                  -> score 0
  V3 = V2 with the `id = vabid` reuse also deleted      -> score 0
V3 is the banked candidate: `id` now carries ONLY the open call's result, every
vab-id use reads the parameter, and no local is bound to another local's value.
Full build with V3 in place: build_sha1 = 62efab4f73f992798c43e8c730aa43baa10bb4fa
== oracle, build_matches = true, 134/134 instructions.

FACT (methodological, s2): the s1 ledger recorded H2 as "the CALLEE's declared
return type places the sign-extension", which is true, but s1 never asked whether
that declared type could be made TRUE instead of fabricated. The check is one
command — read the callee's own asm and see whether the truncation is already
written in its C body. It was. This is the generalisable move for any layer-1
"fabricated prototype" FAIL in this project: a return type that both spellings
compile identically is not a fabrication question, it is a declaration choice the
CALL SITE's bytes decide.
