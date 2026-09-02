# SELF-VET — func_800325E0

Diff scope: `src/code6cac_b.c` only — `INCLUDE_ASM("asm/funcs", func_800325E0);` replaced by
the C body in `memory/grind/func_800325E0/candidate.c` (identical text, diffed this session).
No other tracked file touched (rules / engine / tools / Makefile / *.ld / gate lists
untouched). Measured this session (s4, 2026-09-02; identical text re-measured in s3): `sandbox func_800325E0 --disable all`
== 0, 149/149 insns, rules_dropped 0; `canonical func_800325E0` = 2/149 canonical-asm (mtc2,
swc2), i.e. the island only.

Standing layer-1 constraint (2026-09-02 13:26) addressed: the FAIL was that the island
comment named no Sony PsyQ GTE macro / header line. The old comment ("Hand-written GTE
leading-zero-count block ... identical to the user-authorized block in the matched sibling
func_800274BC") is banned for this function, is NOT present, and is banked at
`rejected/layer1-fail-0902-1326.c`. The island's instructions are unchanged; only the comment
was rewritten to the Ruling A form (macro name + header lines, see ANNOTATION-CONFORMANCE).

CONSTRUCTS: (1) gte_Lzc SDK macro island (PsyQ 4.5 gtemac.h:174-178 = gte_ldlzc inline_c.h:228-231 + gte_nop x2 inline_c.h:1346-1347 + gte_stlzc inline_c.h:1318-1322), one `__asm__ volatile` statement whose only GPR insns are the cluster-grant cop2 addressing preamble (`addu $t4,%1,$zero`; `addiu $v0,$sp,0x10` / `addu $t4,$v0,$zero`); (2) compound-assignment split `pan_sign = ~(u32)projected_pan; pan_sign >>= 31;`; (3) in-place reuse of `pan_L` / `pan_R` for the distance-scaled volumes (`pan_L = (distance_scale * pan_L) >> 16;`). No register pins, no `move %0,%1` aliasing, no barriers, no dead stores, no do-while(0), no volatile globals, no pad locals, no FAKE constructs.

## T1 semantic purpose:
(1) The island computes the GTE leading-zero count of `dist_sq` (LZCS in, LZCR out) — the
only source of `clz`, which selects the log-table index and the result shift; without it
`dist_volume` is wrong for every distance >= 0x400. The target bytes contain the identical
seven instructions (asm/funcs/func_800325E0.s:50-56; splat tags the mtc2/swc2 "handwritten
instruction"). (2) `pan_sign` is the sign flag of the projected pan; both statements execute,
neither is dead, and the flag selects the L/R swap; the target computes it in place
(`nor $a3,$zero,$a1` / `srl $a3,$a3,31`). (3) `pan_L`/`pan_R` hold the left/right volume
through its two stages (unit pan, then distance-scaled) and are the values passed to
`func_8005C650`; every read consumes a real value. PASS.
## T2 human-programmer:
(1) A PsyQ programmer computing log2 of a 32-bit distance calls the SDK's `gte_Lzc` macro —
that is literally what the original author did (the bytes are the macro body). (2) Writing a
flag as "complement, then take the top bit" in two statements is ordinary C; owner Ruling 4
(2026-09-02) says consecutive compound assignments on the same variable are ordinary C.
(3) Scaling a volume in place (`vol = (scale * vol) >> 16;`) is the natural spelling; the
retired chassis's extra `L_scaled`/`R_scaled` locals were the unnatural form. PASS.
## T3 GCC-internals justification:
The program-logic explanation stands on its own for every construct (T1/T2). The codegen
observation that motivated choosing (2) and (3) over the retired spellings (global.c allocno
priority ordering; `.greg` dump `tmp/grind/func_800325E0/dumps/code6cac_b.greg` insn 208
`(set (reg/v:SI 7 a3) (not:SI ...))`) is the method of matching decompilation, not the
construct's justification; per ordinary-c-judge-decidable.md Ruling 1(3) / Ruling 4 a
semantically truthful spelling chosen after observing the allocator is not a FAIL ground.
The island is justified by the target's hand-written SDK-macro bytes, not by any GCC pass.
PASS.
## T4 permuter/search provenance:
No permuter or auto-search was run in any session. Every form was hand-derived from the
object-level diff (`tools/pairdiff.py`), the global.c priority formula (global.c:635-648)
and the matched siblings func_80032314 / func_8002E838. PASS.
## T5 family check:
(1) matches the sanctioned canonical-asm cluster (COMPLETED-INLINE-ASM-CANONICAL bucket,
enumerated by name in the owner grant registry `tools/grinder/owner_cluster_grants.txt:24`);
the 11-line `__asm__` text is character-identical to the func_8002E838 island at
`src/code6cac_b.c:1189-1199` (Judge PASS 2026-09-02, docs/grind/decisions.md:20264;
allowlisted `inline_asm_canonical.txt:373`) including the `addiu $v0,$sp,0x10` +
`addu $t4,$v0,$zero` addressing preamble and the `"$2"` clobber. Condition 3 as clarified by
Ruling A (`.claude/rules/cop2-addressing-preamble-cluster.md:163-176`): nothing outside the
named macro body (`gte_Lzc` = ldlzc / nop / nop / stlzc) plus the addressing preamble is in
the island, and the comment names the macro and its header lines. It matches no forbidden
construct class: no pin, no move-aliasing, no barrier, no free-standing GPR asm outside the island.
(2) and (3) are ordinary C (Ruling 4 / Ruling 1) and match no forbidden family — no dead
store, no self-assign, no invented holder, no alias, no width cast. PASS.
## T6 naming-announces-intent:
Names: `sp_tmp` (the LZCR store slot — the same name used by every authorized sibling
island), `dx/dy/dz`, `dist_sq`, `clz`, `v0_m/v1_m/idx/hi` (the sibling's log-table
arithmetic locals, copied from func_80032314), `dist_volume`, `distance_scale`,
`listener_angle`, `projected_pan`, `pan_sign`, `pan_L`, `pan_R`, `tmp` (the swap
temporary). No `pad`, `dummy`, `unused`, `spill`, `slack`, `_buf`, `tail` or any other
coercion-announcing name; every local is read. PASS.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: cop2-addressing-preamble cluster (GTE gte_Lzc leading-zero-count island; COMPLETED-INLINE-ASM-CANONICAL bucket)
  SCOPE: "A member inherits the disposition **only** when all of these hold. This is a check, not a lever — it does not lower anyone's distance:"
  PRECEDENT: `.claude/rules/cop2-addressing-preamble-cluster.md:104`
  PRECEDENT: `.claude/rules/cop2-addressing-preamble-cluster.md:163`
  PRECEDENT: `tools/grinder/owner_cluster_grants.txt:24`
  PRECEDENT: `inline_asm_canonical.txt:373`
  PRECEDENT: `docs/grind/decisions.md:20264`
  Per-function check (cluster rule items 1-4), run this session: (1) sandbox 0 at 149/149
    (rules_dropped 0); (2) zero pins / aliasing blocks / barriers in the body; (3) in-island
    GPR = `addu $t4,%1,$zero` (ldlzc operand preamble) + `addiu $v0,$sp,0x10` /
    `addu $t4,$v0,$zero` (stlzc store-address preamble) only, the same two preambles the
    Judge admitted for func_8002E838; (4) layer-2 cheat-reviewer + `verify-oracle --rebuild`
    is the driver/operator step. Honest bucket is COMPLETED-INLINE-ASM-CANONICAL: the driver
    writes the `inline_asm_canonical.txt` line via the owner-cluster grant door before
    `queue done` (never COMPLETED-C).
  FAMILY: compound-assignment split (ordinary C under owner Ruling 4; listed only because it was chosen after observing register allocation)
  SCOPE: "Splitting one assignment into consecutive compound assignments on the SAME"
  PRECEDENT: `.claude/rules/ordinary-c-judge-decidable.md:157`
  (The Ruling 4 sentence continues at :158-161: "variable — `ratio *= 0x103B; ratio >>= 12;`
    for `ratio = (ratio * 0x103B) >> 12;`, or `v = a; v += b;` for `v = a + b;` — is a
    semantically-truthful spelling under Ruling 1(3), provided no statement is dead, no
    annotation is needed and no pad is introduced." All three provisos hold here.)
ANNOTATION-CONFORMANCE: n/a — no FAKE construct. The island carries the Ruling A (2026-09-02) provenance comment naming the SDK macro and header lines: "PsyQ 4.5 SDK GTE macro body: gte_Lzc(dist_sq, &sp_tmp) (gtemac.h:174-178) = gte_ldlzc(r0) `mtc2 %0,$30` (inline_c.h:228-231) + gte_nop() x2 `nop` (inline_c.h:1346-1347) + gte_stlzc(r0) `swc2 $31,0(%0)` (inline_c.h:1318-1322)" plus the statement that the two addressing preambles are the cluster-grant idiom and nothing outside the macro body is in the island (src/code6cac_b.c:2429-2438). Header lines verified against the PsyQ 4.5 headers at tmp/grind/motion_SetMotion/s7/repos/rood-reverse/include/psx/{gtemac.h,inline_c.h} ($PSLibId: Run-time Library Release 4.5$).
