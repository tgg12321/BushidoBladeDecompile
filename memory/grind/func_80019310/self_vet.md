# SELF-VET — func_80019310

CONSTRUCTS: (1) four `__asm__ volatile` GTE islands reproducing PsyQ SDK macro bodies — gte_SetRotMatrix, gte_SetTransMatrix, gte_ldv0 (lwc2 form) + 2 load-delay nops, cop2 MVMVA `.word 0x4A480012`, gte_stlvnl — each in the `move $12, %0` / hardcoded `$12-$15` macro-body spelling with `"$12".."$15"` clobbers; (2) a `"memory"` clobber on every island's clobber list (island 1's is byte-load-bearing: it forces the 0x8001934C reload of the MATRIX pointer; islands 3/4 read/write memory in fact); (3) a new prototype `void func_80019310(s16 *arg0, s32 *arg1);` at src/code6cac.c:314 (the caller predates the definition); (4) the ordinary C: `s32 out[3]` local read back after gte_stlvnl, `for (i = 0; i < ((s16 *)arg1)[2]; i++)` loop over `(s32 *)(arg1[0] + i * 8)`, six `dst[k]` stores with `dst = (s32 *)((u8 *)dst + 0x40)`, and the struct copy `*(MATRIX *)(arg1 + 5) = **(MATRIX **)(arg0 + 2)`. Zero register pins, zero free-standing GPR asm, zero empty-asm barriers, zero volatile, zero dead locals/stores, zero `/* FAKE */` constructs, zero regfix/asmfix rules (rules_dropped 0).

## T1 semantic purpose:
(1) The islands ARE the function's GTE work — the 14 cop2 insns have no C form (GCC 2.7.2's MIPS backend emits no cop2 mnemonics). (2) The "memory" clobber tells GCC the islands touch memory: island 3 reads the SVECTOR through $12, island 4 writes out[0..2] which the C reads back (without it GCC has no reason to order/keep those reads honestly); on island 1 its observable effect is the second read of the MATRIX pointer before gte_SetTransMatrix — a real emitted load the target contains. (3) The prototype only types the existing call. (4) Every C statement is program logic present in the bytes.

## T2 human-programmer:
A PsyQ programmer writes exactly `gte_SetRotMatrix(m); gte_SetTransMatrix(m); for (...) { gte_ldv0(v); gte_rtv0(); gte_stlvnl(out); dst->... = out.. << 7; ... } desc->mat = *m;` — the islands are the SDK macros spelled out (the repo's accepted func_800203B4 form), and a macro user never edits a macro's clobber list per call site, which is why the clobber is on all four islands uniformly rather than tuned to the one island where it changes bytes. Reading the pointer again after a macro that declares it clobbers memory is the compiler's business, not something the reader would question.

## T3 GCC-internals justification:
The candidate is explained by program logic + the SDK macro text. GCC internals (cse.c invalidate_memory on a `(clobber (mem:BLK (scratch)))`) are cited in evidence.md H2 only to explain WHY the target's reload exists, i.e. as evidence the original macros carried a memory clobber — not as a lever chosen to steer a pass. Honest caveat: no Sony `inline_c.h` is on disk to quote the 3.x macro's clobber list; the in-band evidence is the target reload itself. If the reviewer/Judge holds that the clobber list is outside the condition-3 template, the correct disposition is a ruling-request on that exact question, not a respelling (volatile-param / cast reload spellings are the banned forms of the same reload and were NOT tried).

## T4 permuter/search provenance:
No permuter, no search. Hand-written from the asm in six measured variants (v1..v6, tmp/grind/func_80019310/s1/); the only choices made were the local's size (frame 24) and the clobber list.

## T5 family check:
Not a coercion family. The islands fall under the COP2 addressing-preamble cluster canonical-asm path (owner grant 2026-09-01, widened anchor) — func_80019310 is named in that grant as a confirmed handwritten-tagged carrier; bucket is COMPLETED-INLINE-ASM-CANONICAL, never COMPLETED-C. The "memory" clobber matches the func_800300B4 Judge-PASSed island form (truthful clobber, same-file precedent code6cac_b.c:1708) and func_8002D320's islands (code6cac_b.c:935/942). It is NOT an `__asm__ volatile("" ::: "memory")` empty barrier (there is no empty asm anywhere), and no forbidden-catalog row (pins, hardcoded-$N injection outside a granted island, volatile coercion, dead locals, alias renames) is present.

## T6 naming-announces-intent:
Locals are `out`, `i`, `dst`; params `arg0`/`arg1` (splat defaults). No pad/dummy/spill/tail names.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: COP2 addressing-preamble cluster — canonical GTE macro islands (widened anchor grant 2026-09-01; condition 3 = the SDK macro body)
  SCOPE: "membership closes the TAIL ISLAND only, when the function's pure-C body independently reaches sandbox 0 under the normal grind gates."
  PRECEDENT: `.claude/rules/cop2-addressing-preamble-cluster.md:154`
  PRECEDENT: `docs/grind/decisions.md:18119`
  PRECEDENT: `inline_asm_canonical.txt:367`
  FAMILY: truthful "memory" clobber on a granted GTE island (Judge PASS precedent, func_800300B4)
  SCOPE: "island 4 adds a truthful \"memory\" clobber (the asm writes mac[], which the following C reads) with same-file precedent at src/code6cac_b.c:1708."
  PRECEDENT: `docs/grind/decisions.md:20650`
  PRECEDENT: `src/code6cac_b.c:935`

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. Every island carries the macro-name comment the cluster rule requires (gte_SetRotMatrix / gte_SetTransMatrix / gte_ldv0 / MVMVA 0x0480012 / gte_stlvnl) and the function header cites the 2026-09-01 grant record.
