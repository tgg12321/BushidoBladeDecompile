# Evidence bank — func_800480C0

## s1 — recon (2026-09-02, chassis HEAD dd2808d5, -mel, 0 regfix/asmfix rules)

- **Shape.** 74-insn loop: `p = arg0 + (s16)arg1*4; p = arg0 + (*p & ~3);
  count = *p++; while (count--) { word, lhu*4 -> func_800482C8(arg0 + (word & ~3),
  a1v+arg2, a2v+arg3, a3v+arg4, v0v+arg5) }`. It is the 6-arg twin of COMPLETED-C
  `func_80047FBC` (4-arg, src/text1b.c:84) — identical prologue idiom (`addu $s0,$a0`;
  `addu $s2,$s0`), identical loop body, plus two extra stack args (`lw $v1,0x68($sp)`,
  `lw $a0,0x6C($sp)`) sign-extended into $s4/$s3 inside the `if`.
- **Frame forensics.** `.frame $sp,0x58`: outgoing args 0x00-0x17 (5-arg call; `sw $v0,0x10($sp)`
  is the only sp-relative store outside the register saves), vars 0x18-0x37 = 32 bytes with
  ZERO `sw`/`lw` anywhere in `asm/funcs/func_800480C0.s`, regs $s0-$s6+$ra at 0x38-0x57.
  Exactly the same allocated-but-untouched 32-byte region the three text1b siblings carry
  (`engine/volatile_cheats.py:753-767`: func_80047EE8, func_80047FBC, func_800481E8 all
  `("pre_pad", 8)`).
- **Attribution ladder** (`tmp/grind/func_800480C0/s1/ladder.txt`), all `sandbox --disable all`:
  | body | score |
  |---|---|
  | clean (no `arg0 = 0`; pad stripped by detector) | 32 (74/74) |
  | + `arg0 = 0; /* FAKE */` (pad still stripped) | **20** (74/74) — honest floor as the engine reads it today |
  | + `volatile u32 pre_pad[8]` honoured (real build) | **0** — full build SHA1 == oracle (`s1/build.log`) |
  All 20 residual insns at score 20 are sp-offset deltas only (`addiu $sp,-0x38` vs `-0x58`,
  8 saves, 8 restores, 2 stack-arg loads, final `addiu $sp`): the instruction STREAM is
  byte-identical (`s1/cand1_build.txt` vs target, diffed in-session).
- **Why the sandbox cannot read 0.** The volatile-cheat stripper removes every unused local
  array unless `_is_sanctioned_pad` finds an exact `(func, name, count)` row in
  `engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS` (line 746). `engine/` is on the
  scope-grant DENYLIST (`.claude/rules/integration-handoff-self-serve.md:63`,
  `tools/grinder/grindlib.py:402`) — owner-only. The siblings got their rows by owner ruling
  2026-08-20 (func_80047FBC) and the 2026-08-22 parked-but-proven audit (func_800481E8).
- **Canonical gate:** verdict C, distance 20. `scan_hand_coded --single`: LOW 1/8 (S4 only) —
  no canonical-asm signal; not wanted anyway, the C form is proven.
- **Duplicate leads:** `tmp/duplicates_leads.txt` has no entry for func_800480C0; scan S5
  reports no >0.5-jaccard sibling (the twin differs by the two stack args).
- **Retired chassis (2026-08-24 pin):** old pinned floor 36 came from a 33-rule regfix body
  with five register-asm pins (`retired-chassis-2026-08/body.c`); superseded entirely.

- [s1] target .frame 0x58: args 0x00-0x17, vars 0x18-0x37 with ZERO sw/lw in asm/funcs/func_800480C0.s, regs 0x38-0x57 - same layout as siblings func_80047EE8/func_80047FBC/func_800481E8 (engine/volatile_cheats.py:757-767)

- [s1] sandbox strips volatile pre_pad unless _SANCTIONED_UNWRITTEN_PADS (engine/volatile_cheats.py:746) has an exact row; engine/ is scope-grant denylisted (.claude/rules/integration-handoff-self-serve.md:63) - owner-only

- [s1] scan_hand_coded --single func_800480C0: LOW 1/8 (S4 only) - no canonical-asm signal

- [s1] retired 2026-08 chassis floor 36 (33 regfix rules, 5 register pins) is superseded; ledger floor now 20 (honest as the engine reads it) / 0 (bytes proven)

## s2 — structural (2026-09-02, chassis HEAD c8e81f83, -mel, 0 regfix/asmfix rules)

- **The s1 "honest floor 20" was a SANDBOX ARTIFACT and is retired.** The
  volatile-cheat stripper deletes the *declarator text* of `volatile u32
  pre_pad[8];` but leaves the dangling `volatile` qualifier, which then binds to
  the next declaration. The stripped source it actually scores
  (`tmp/sandbox/func_800480C0/src/text1b.c:160-161`) reads `volatile` /
  `u32 *p;` — i.e. the 20 was measured on a `volatile u32 *p` body that no
  session ever wrote. Confirmed by measuring the same body with the pad removed
  from source: 32. The **honest pad-free floor is 32** (73 scorable insns vs
  target 74). A plain non-volatile `u32 pre_pad[8]` also scores 32 (the stripper
  removes it cleanly, no dangling qualifier).
- **Frame gradient instrument** (`tmp/frame_probe.sh func_800480C0 text1b`,
  cpp | cc1 -mel, reads cc1's own `# vars=`): separates "wrong frame" from
  "wrong codegen" and is ~15 s per spelling. Baseline honest body:
  `.frame $sp,56 # vars= 0, regs= 8/0, args= 24`. Target:
  `.frame $sp,88 # vars= 32, regs= 8, args= 24`. **regs and args already match;
  the entire frame gap is vars=32.**
- **`volatile` is NOT load-bearing for the frame slot.** A plain unused
  `u32 pre_pad[8]` compiles to vars=32 exactly like the volatile one
  (`tmp/grind/func_800480C0/s2/frame_ladder.txt`). The volatile in the pad family
  buys engine allowlist eligibility, not codegen — so there is no "non-volatile
  respelling" escape from the ban; the plain form is the forbidden
  dead-vars-local-array family (`.claude/rules/dead-vars-local-array.md`).
- **Phantom-slot ceiling on this body is 8 bytes, not 32.** 13 distinct honest
  structural spellings measured (`tmp/grind/func_800480C0/s2/frame_ladder.txt`):
  the folded loop-guard compare in the func_8003D9A0 spelling
  (`guard = count - 1; if (guard != -1)`) and every derivative of it produce
  `vars= 8` and nothing more. Rotated-while (func_8003DBE4 shape), hoisted inner
  temporaries, HImode narrow accumulators, named pointer intermediates, unsigned
  count, added register pressure, 4 stacked independent guards, per-stack-arg
  guards, nested guards — all measured `vars= 0` or `vars= 8`. No spelling
  reached 16, let alone 32.
- **The 8-byte phantom slot is genuinely zero-cost**
  (`tmp/grind/func_800480C0/s2/v9.s`): the vars region 0x18-0x1F carries no
  `sw`/`lw`; the register saves start at 0x20. The lever is real; it simply
  saturates one quarter of the way to the target, and the best spelling carrying
  it scores 36 — worse than the vars=0 body's 32.
- **The residual at 32 is frame + a callee-saved SEAT ROTATION, and the two are
  coupled.** objdump of the honest build
  (`tmp/grind/func_800480C0/s2/v0_dis.txt`) matches the target
  instruction-for-instruction except (a) every sp-relative offset and (b) the
  callee-saved bindings: the target holds base_addr in `$s2` and sx_arg2..5 in
  `$s6/$s5/$s4/$s3`; the honest build holds them in `$s6` and `$s5/$s4/$s3/$s2`.
  Since the pad-carrying build byte-matches the oracle, adding the 32 frame bytes
  ALSO fixes the seats — the seat rotation is downstream of `get_frame_size`, not
  an independent residual. Attacking seats and frame separately is therefore the
  wrong decomposition, and it explains why s1 read the frame gap as "20 sp-offset
  deltas only": that 20 was measured on the accidental volatile-pointer body,
  which happens to seat correctly.

- [s2] honest pad-free floor = 32; the ledger's 20 was a stripper artifact (dangling `volatile` binds to `u32 *p`) - tmp/sandbox/func_800480C0/src/text1b.c:160

- [s2] target frame decomposition: vars=32, regs=8, args=24; the honest build already matches regs and args exactly - the entire gap is vars

- [s2] 13 structural spellings measured; phantom-slot production saturates at vars=8 (zero-cost, verified no sw/lw in the slot) - tmp/grind/func_800480C0/s2/frame_ladder.txt

- [s2] plain non-volatile unused u32 pre_pad[8] also gives vars=32, so `volatile` is not the codegen agent in the pad family

- [s2] honest build vs target differ by sp offsets AND a callee-saved seat rotation (base_addr $s6 vs target $s2); the pad build gets both right, so frame and seats are one coupled residual

- [s2] Honest pad-free floor of func_800480C0 is 32 (73 scorable insns vs target 74); the ledger's 20 is retired as a sandbox-stripper artifact - engine/cheats.py deletes the pad declarator text but leaves a dangling `volatile` that binds to `u32 *p` (tmp/sandbox/func_800480C0/src/text1b.c:160-161).

- [s2] Target frame decomposes as vars=32, regs=8, args=24 (.frame $sp,88). The honest build already matches regs and args exactly (.frame $sp,56 # vars= 0, regs= 8/0, args= 24) - the entire frame gap is the 32 vars bytes.

- [s2] A plain non-volatile unused `u32 pre_pad[8]` compiles to the same vars=32, so `volatile` buys engine-allowlist eligibility, not codegen; there is no non-volatile respelling that escapes the pad ban without landing in the dead-vars-local-array family.

- [s2] 13 honest structural spellings measured via the cc1 `# vars=` gradient saturate at vars=8; the 8-byte slot is verified zero-cost (no sw/lw in 0x18-0x1F, tmp/grind/func_800480C0/s2/v9.s) but its best sandbox score is 36 vs the baseline 32.

- [s2] The honest build's residual is sp offsets PLUS a callee-saved seat rotation (base_addr $s6 vs target $s2; sx_arg2..5 $s5/$s4/$s3/$s2 vs target $s6/$s5/$s4/$s3), and the pad-carrying build fixes both at once - frame and seats are coupled through get_frame_size.

- [s2] Reusable instrument for this function: tmp/frame_probe.sh func_800480C0 text1b prints cc1's own `# vars=` in ~15 s, separating 'wrong frame' from 'wrong codegen' far more cheaply than the sandbox score.

- [s2] The s1 bytes-proven body (volatile pre_pad[8] + arg0=0, full build SHA1 == oracle) is preserved at memory/grind/func_800480C0/rejected/pad-judge-banned-2026-09-02.c; it remains unusable under the Judge's 2026-09-02 04:28 ruling until an owner grant adds the engine/volatile_cheats.py row.
