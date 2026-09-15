# SELF-VET — func_80054604
CONSTRUCTS: pointer-alias local `Unk800EFAE8Ctrl *s = &D_800EFAE8;` (FAKE-annotated at its declaration in src/text1b.c), compound-assignment split `a6 += ret; game_StageCleanup(n, a6);` (ordinary C, Ruling 4), aggregate merge `Unk800EFAE8Ctrl D_800EFAE8` declared in include/game.h replacing the splat per-word scalars D_800EFAE8/D_800EFB0C/D_800EFB14/D_800EFB18/D_800EFB1C/D_800EFB20/D_800EFB24/D_800EFB28 (consumers func_80054FDC / func_8005507C converted, src/text1b_b.c's unused per-word externs removed), integer-typed data base `s32 p = s->unk2C; *(s32 *)(*(s32 *)(p + 4) + p)` (ordinary C), sign test `if (s->unk4 >= 0)` (ordinary C, target semantics)

## T1 semantic purpose:
- pointer alias `s`: no semantic effect beyond `D_800EFAE8.field` (a second handle to the same global) — this is exactly the sanctioned pointer-alias shape and is why it carries the mandatory FAKE annotation; it is claimed ONLY under that family, not as ordinary C. Both handles are used on purpose: the two stores to `D_800EFAE8.unk2C` go through the global (the target stores that word via `lui $at / sw %lo`), every other field access goes through `s` (the target's `$s1` base register). Not first reach: the direct-global form was measured first (82 vs 26, s1, rejected/direct-global-no-pointer-local-82.c).
- `a6 += ret; game_StageCleanup(n, a6);`: a real statement with a real consumed value (the advanced buffer address is what the callee receives); it is the compound-assignment spelling of `game_StageCleanup(n, a6 + ret)`. No statement is dead (a6's new value is read by the call), no pad, no annotation needed (Ruling 4).
- aggregate merge: a declaration-level object model, not a match device; it is the header-canonical shape the 2026-08-17 ruling requires, and every consumer stays byte-identical (verify-oracle --rebuild SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa this session).
- `s32 p` + `offset + base` operand order and `>= 0`: ordinary expression spellings that state what the target computes (`addu $v0,$v0,$v1`; `bltz` skipping the store).

## T2 human-programmer:
- `buf += ofs; cleanup(n, buf);` is a normal way to advance a base and pass it. `p = s->unk2C; *(s32 *)(*(s32 *)(p + 4) + p)` is the ordinary "table-relative offset" read for a relocatable data blob whose base is stored as an integer address (the relocator func_80054FDC adds a base to that same word, so integer typing is the model the program itself uses). `if (s->unk4 >= 0)` is the natural sign test. The struct is what a programmer would declare for a 0x4C-byte control block the binary addresses through one base register. The pointer alias is the one construct a reader would question; it is annotated as FAKE for that reason.

## T3 GCC-internals justification:
- Pointer alias: the mechanism (address materialisation caching / base-register allocation) IS the reasoning — that is why it is FAKE-annotated and claimed under the pointer-alias family, whose rule requires exactly this mechanism naming (prerequisite 2, `.claude/rules/pointer-alias-fake-exception.md:54`).
- Compound split: I chose it after observing register allocation (local-alloc.c:1064 REG_EQUIV live-length doubling, global.c:642 allocno_compare, sched.c:2460); Ruling 4 (`.claude/rules/ordinary-c-judge-decidable.md:165-176`) states a reviewer may not FAIL a compound-assignment split on that ground. The program logic is also a complete explanation (advance the buffer, then pass it).
- Everything else: no GCC internal is needed to explain it.

## T4 permuter/search provenance:
No permuter or automated search was used. Every form was hand-written from the target listing and the pass dumps (tmp/grind/func_80054604/s2/dumps_base, dumps_a6zero) and measured with sandbox; the mechanism was read from the GCC source before the closing form was written. The mechanism probe `a6 = 0;` (rejected/a6-eq-zero-in-else-arm-4.c) was NOT submitted: it is a no-op store (T2 fail).

## T5 family check:
- Pointer alias → pointer-alias family (sanctioned, FAKE). Not an `asm("sym")` rename, not a volatile coercion.
- `a6 += ret` → compound-assignment split, ordinary C by owner Ruling 4; NOT a dead-store (the write is read), NOT a dead-param-assign (Lever D requires the store to be dead), NOT a param-local alias (no second variable).
- Aggregate merge → the 2026-08-17 aggregate-merge family: (a) base-register evidence in the binary (asm/funcs/func_80054604.s forms `$s1` = %hi/%lo(D_800EFAE8) once and reaches offsets 0x00..0x4A from it; asm/funcs/func_8005490C.s addresses the same block through the same base; func_80054FDC relocates the 0x2C..0x40 group together) — not symbol adjacency; (b) a struct because the evidence shows one record; (c) C half complete: no C code names D_800EFB0C/14/18/1C/20/24/28 (grep clean after integration). config half landed THIS session (s3) under the pipeline scope grant (tools/grinder/scope_allow.txt:76, integration-handoff-self-serve, owner ruling 2026-08-19): rows D_800EFB14/18/1C/20 in undefined_syms_auto.txt carry the 2026-09-03 alias suffix `/* alias of D_800EFAE8+0x2C..0x38; retire with func_8005490C (asm/funcs/func_8005490C.s is a live INCLUDE_ASM referrer) */` because asm/funcs/func_8005490C.s (INCLUDE_ASM at src/text1b.c) still names them; rows D_800EFB0C/24/28 are DELETED (their only namers are the unbuilt .s files of the already-C func_8005507C / func_80054FDC). grep of the seven names over src/ include/ after integration hits only the explanatory comment in include/game.h. Precedent rows: undefined_syms_auto.txt:301-303 (D_800A9CFA.. alias suffix form); (d) declared in include/game.h, not TU-local; (e) byte-neutral for every consumer — `verify-oracle --rebuild --allow-dirty` this session (s3, 2026-09-15, with the undefined_syms_auto.txt edits in place): ok true, build_matches true, build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa; sandbox --disable all = 0 (160/160, rules_dropped 0).
- Pre-existing on main and unchanged in construct: func_80054FDC's `s32 *p = &D_800EFB14;` became `s32 *p = &D_800EFAE8.unk2C;` by the merge only (same alias, same bytes); it was not introduced by this session.
- No banned construct in the brief (none listed).

## T6 naming-announces-intent:
Locals are `s`, `id`, `ret`, `t`, `p`, `v`, `n`; struct members follow the header's `unkNN` convention. No pad/dummy/unused/spill names. The alias `s` is used throughout the body (every field read/write except the two unk2C stores), not discard-only.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: pointer-alias (C-level pointer alias to a global)
  SCOPE: "a local pointer that provides a second C handle to a global — where using the global directly would be semantically identical — is a sanctioned last-resort matching lever under the prerequisites below."
  PRECEDENT: `.claude/rules/pointer-alias-fake-exception.md:13`
  PRECEDENT: `docs/reference/sotn-construct-index.md:96`

  FAMILY: aggregate merge (per-word splat symbol → aggregate)
  SCOPE: "two or more splat-invented `D_<addr>` scalars may be replaced by a single aggregate declaration."
  PRECEDENT: `.claude/rules/no-new-park-categories.md:241`
  PRECEDENT: `include/game.h:56`

  FAMILY: compound-assignment split (ordinary C, not a FAKE family — listed for completeness)
  SCOPE: "Splitting one assignment into consecutive compound assignments on the SAME variable — `ratio *= 0x103B; ratio >>= 12;` for `ratio = (ratio * 0x103B) >> 12;`, or `v = a; v += b;` for `v = a + b;` — is a semantically-truthful spelling under Ruling 1(3), provided no statement is dead, no annotation is needed and no pad is introduced."
  PRECEDENT: `.claude/rules/ordinary-c-judge-decidable.md:167`

ANNOTATION-CONFORMANCE: one FAKE construct, annotated at its declaration in src/text1b.c:
  /* FAKE: second C handle to the global ctrl block (pointer-alias family); mechanism: expand/cse address materialisation -- the pointer local seats %hi/%lo(D_800EFAE8) in one callee-saved base register ($s1) for the whole body, whereas the direct D_800EFAE8.field form re-materialises the address per extended basic block; lever-exhaustion: direct-global form measured 82 vs 26 (memory/grind/func_80054604/evidence.md s1, rejected/direct-global-no-pointer-local-82.c). */
  It carries what (second handle to the global ctrl block), mechanism (address materialisation / base-register seating), and lever-exhaustion (the measured direct-global form). The compound split and the aggregate carry no FAKE annotation because their families do not mandate one.
