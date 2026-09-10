# Evidence bank — func_8005C6D0

## s1 (recon, 2026-09-10) — first C body ever written for this function

- [s1] CHASSIS. `canonical` = C, tier LOW (no S1/S2/S6 hand-asm signal), 118 target
  instructions. Honest floor at session start = 118 (`no_c_body: true` — src/text1b.c
  carried `INCLUDE_ASM("asm/funcs", func_8005C6D0);`). Honest floor at session end = **5**
  (build 119 insns vs target 118). Ten measured spellings, banked below.

- [s1] SEMANTIC MODEL (derived from asm/funcs/func_8005C6D0.s, 126 lines, plus the
  already-matching sibling func_8005C650 at src/text1b.c:2678). The function is the
  per-frame sound-request FLUSH:
  `SpuGetAllKeysStatus(sp+0x20)` into a 24-byte local, then for i in 0..23:
  read the 8-byte pool entry at `D_800EFB78 + i*8`; if its +0 pointer `p` is non-NULL
  and the VAB slot `D_800EFC38[*(u16 *)p]` holds a KSEG0 pointer (tested `bgez` => `< 0`
  as s32), scan SPU voices starting at `next` (a carrier that PERSISTS ACROSS POOL
  ENTRIES — it is only advanced when a key-on actually happened) for the first voice
  with `SpuGetKeyStatus(1 << voice) != 1`; remap VAB id 6 -> 3 when
  `D_800EFC50 == D_800EFC44`; fetch the packed note word
  `((u32 *)D_800EFC38[vab][0])[ ((u16 *)p)[1] ]` and call
  `SsUtKeyOnV(voice, vab, w & 0x7F, (w >> 7) & 0xF, (w >> 11) & 0x7F, (w >> 18) & 0x7F,
  D_800EFB7D[i*8], D_800EFB7C[i*8])`. Every iteration ends by clearing the pool
  entry's +0 word to 0. Sony prototype: `short SsUtKeyOnV(short voice, short vabId,
  short prog, short tone, short note, short fine, short voll, short volr)` — the two
  volume args come from the pool entry's +5 and +4 bytes IN THAT ORDER (note the swap:
  arg7 = D_800EFB7D = +5, arg8 = D_800EFB7C = +4).

- [s1] OBJECT MODEL: the brief flagged D_800EFB7D as a per-word splat piece of
  g_effect_pool_b_24_field_4 (@800EFB7C) and directed the aggregate-merge family as the
  sanctioned spelling. Verdict per flagged symbol, from the TARGET BYTES:
  * **D_800EFB78 — MATCHES the split/scalar-address spelling.** Target emits
    `lui $at,%hi(D_800EFB78) / addu $at,$at,<idx> / lw $s1,%lo(D_800EFB78)($at)` at
    0x8005C718 and the same shape for the `sw $zero` at 0x8005C854 — i.e. a symbol+addend
    relocation per use, exactly what `*(s32 *)((u8 *)&D_800EFB78 + off)` produces and
    exactly what the already-byte-matching sibling func_8005C650 (src/text1b.c:2686-2689)
    uses. Measured: this spelling reaches score 5.
  * **D_800EFB7C / D_800EFB7D — MATCH the split spelling (measured score 5).** Target
    0x8005C804 and 0x8005C818 each carry their OWN `lui %hi / addu $at,$at,$s2 / lbu %lo`
    triple against their own symbol. A merged 8-byte struct-array declaration is NOT
    required to reach 5, and the two lbu's do NOT share a base register in the target, so
    the merge buys nothing here. MISMATCH-unmeasured for the merged form itself: not
    tested, because the split form already reproduces the target's addressing exactly and
    a merge would additionally have to survive the known `sym+N` scoring blindspot
    (memory index: score-symtab-blind-to-asm-data-dlabels).
  * **D_800EFC38 — MATCHES an ARRAY declaration** (`extern s32 *D_800EFC38[];`, the same
    spelling func_8005BF78 at src/text1b.c:2599 already uses). Target hoists its base into
    `$s6` in the outer preheader (`lui/addiu` at 0x8005C704) and indexes `addu $v0,$v0,$s6`.
  * **D_800EFC44 / D_800EFC50 — MISMATCH for the aggregate merge (unmeasured, but the
    bytes decide it).** They are the +0xC and +0x18 words of D_800EFC38 (i.e. [3] and [6]),
    but the target loads them at 0x8005C790-0x8005C79C through their OWN `lui %hi / lw %lo`
    pairs, NOT through the already-live `$s6` base. Spelling them as `D_800EFC38[6]` /
    `D_800EFC38[3]` would have to emit s6-relative loads, which the target bytes contradict.
    The independent-scalar spelling (as src/text1b.c:2517 already uses) is what reaches 5.

- [s1] RESIDUAL AT SCORE 5 (fully characterised, single cause). Register allocation is
  IDENTICAL to the target for every value: s4=next, s3=i, s2=off, s1=p, s0=voice,
  s5=constant 1, s6=&D_800EFC38; frame is -0x58 with the same 8 callee-saved slots; every
  stack-arg slot (0x10/0x14/0x18/0x1C) and every %hi/%lo relocation matches. The only
  divergence is the delay slot of `beq $v0,$s5,.L8005C838` (target index 41):
  * target fills it with `addiu $v0,$s0,0x1` — reorg.c pulls the loop-continue increment
    out of the .L8005C838 thread and retargets the label past it — and then `li 6` lands in
    the `lhu $a1,0x0($s1)` load-delay slot;
  * our build has sched2 place `li $v0,6` ABOVE the branch, so reorg takes it for the
    branch slot, the `lhu` needs a following `nop`, and the increment stays split as
    `addiu $v0,$s0,1 / addu $s0,$v0,$zero` at the loop bottom (+1 instruction, 119 vs 118).
  Net: 3 mismatched instruction slots + 1 insertion = weighted score 5.

- [s1] SPELLING SENSITIVITY OF THE `i * 8` SITES (the load-bearing discovery; each
  measured on the same chassis, all other text identical). The function has four uses of
  the byte offset `i * 8`: (L) the pool-entry pointer load, (C) the pool-entry clear, and
  (V1,V2) the two volume lbu's. How each is spelled decides three separate GCC decisions:
  * ALL FOUR through one `off` local at the top of the outer loop -> loop.c does not hoist
    %hi(D_800EFB78) (correct) but jump.c cross-jumps the `if (voice < 0x18)` guard into the
    do-while bottom test (emits `j` to the shared test) so the target's duplicated guard is
    missing, and the clear site reuses the register instead of recomputing.
    **score 26 / 114 insns** (rejected/off-local-everywhere-guard-crossjumped.c).
  * L and C written inline as `i * 8`, V1/V2 through `off` declared INSIDE the voice guard
    -> the guard is correctly duplicated and the target's `addu $s2,$v1,$zero` copy appears,
    but with two inline uses of `(u8 *)&D_800EFB78 + i * 8` loop.c hoists the entire symbol
    address into a callee-saved register (extra `lui/addiu` preheader pair, extra
    save/restore, 0x60 frame). **score 20 / 121 insns**
    (rejected/inline-i8-everywhere-licm-hoist.c).
  * L through its own `o` local at the top, C inline as `i * 8`, V1/V2 through `off` inside
    the guard -> all three decisions come out right at once: no LICM hoist, guard duplicated,
    `off = i * 8` CSEs down to the copy `addu $s2,$v1,$zero`, and the clear site recomputes
    `sll 16 / sra 13` at the 4-way join because it sits at a basic-block join where GCC
    2.7.2's CSE has no available expression. **score 5 / 119 insns** — the banked candidate.

- [s1] OTHER MEASURED SPELLING FACTS (each isolated, one edit at a time):
  * `vab` must be **u16**, not s16: the target's `lhu $a1 / andi $v1,$a1,0xFFFF` before the
    `== 6` test is GCC 2.7.2's HImode zero-extend-before-compare. An s16 `vab` emits a
    second `lh` load. (118 -> 58 -> 57 chain.)
  * the packed note word must be **u32**: target uses `srl` for >>11, >>7 and >>18; an s32
    carrier emits `sra`.
  * the note word must be reached through a POINTER local
    (`ev = &((u32 *)D_800EFC38[vab][0])[p[1]]`) and dereferenced four times, not cached in a
    scalar local. The target loads `*ev` twice (0x8005C7DC and 0x8005C7F0) because the
    `sw $v0,0x10($sp)` argument store kills GCC's memory CSE, while the ADDRESS in `$v1`
    survives. A scalar `n = ...` local loads once (1 insn short); re-writing the whole
    `notes[p[1]]` expression four times reloads the `lhu $v0,0x2($s1)` index too (2 insns
    long). The pointer local reproduces exactly. This single change moved 57 -> 37.
  * the inner voice scan must be spelled `voice = next; if ((s16)voice < 0x18) { off = i * 8;
    do { ... } while ((s16)voice < 0x18); }`. A plain `for (voice = next; voice < 0x18; ...)`
    compiles to jump-to-test and loses the target's duplicated guard; `if ((s16)next < 0x18)
    { voice = next; ... }` keeps the duplicated guard but swaps the s3/s4 register roles and
    drops the `addu $s2,$v1,$zero` copy
    (rejected/guard-on-next-no-s2-copy.c, score 20 at exactly 118 insns).
  * `extern s32 D_800EFB78[];` indexed as `D_800EFB78[i * 2]` (array spelling for the pool)
    measured identically to the byte-offset spelling at the same chassis point (score 20) —
    it does NOT change the LICM hoist decision, so the hoist is driven by the number of
    distinct index pseudos, not by array-vs-pointer-arithmetic.
  * `D_800EFC38` must be declared at BLOCK scope inside the function
    (`extern s32 *D_800EFC38[];`) because src/text1b.c:2617 already carries a file-scope
    `extern s32 D_800EFC38;` that the array form would conflict with.
  * the later `extern s32 func_8005C6D0(void);` at src/text1b.c:5928 must become
    `extern void func_8005C6D0(void);` once the body is defined (the asm sets no return
    value); the candidate.c header assumes that one-line edit.

- [s1] SIBLINGS. Both auto-swept siblings (func_80070188 floor 696, func_800720FC floor 688,
  both src/text1b.c) have NO candidate.c and no overlapping block with this function — their
  ledgers record only s1 recon of unrelated camera/replay code. Nothing to transplant; no
  hypothesis is owed to them. The genuinely load-bearing neighbour is func_8005C650
  (src/text1b.c:2678), the already-byte-matching WRITER of the same pool, whose splat
  scalar-address spelling this session reused verbatim.

- [s1] Honest floor moved 118 -> 5 this session; the function had NO C body at all before (src/text1b.c carried INCLUDE_ASM, sandbox reported no_c_body: true). Build is 119 instructions against a 118-instruction target.

- [s1] Semantics: per-frame sound-request flush. SpuGetAllKeysStatus into a 24-byte local, then for i in 0..23 read the 8-byte pool entry at D_800EFB78 + i*8; if its +0 pointer p is non-NULL and D_800EFC38[*(u16 *)p] is a KSEG0 pointer (bgez test), scan SPU voices from a carrier `next` that PERSISTS ACROSS POOL ENTRIES for the first voice with SpuGetKeyStatus(1 << voice) != 1, remap VAB id 6 -> 3 when D_800EFC50 == D_800EFC44, unpack the note word ((u32 *)D_800EFC38[vab][0])[((u16 *)p)[1]] (bits 0-6 prog, 7-10 tone, 11-17 note, 18-24 fine) and SsUtKeyOnV it; every iteration ends by clearing the pool entry's +0 word.

- [s1] Sony prototype confirmed from the argument slots: short SsUtKeyOnV(short voice, short vabId, short prog, short tone, short note, short fine, short voll, short volr). The two volume arguments come from the pool entry's +5 then +4 bytes IN THAT ORDER (arg7 = D_800EFB7D, arg8 = D_800EFB7C).

- [s1] OBJECT MODEL (mandatory recon entry, per flagged symbol): D_800EFB78 MATCHES the split scalar-address spelling (measured, score 5); D_800EFB7C MATCHES (measured, score 5); D_800EFB7D MATCHES (measured, score 5) — the target's two lbu's each carry their own lui %hi / addu $at,$at,$s2 / lbu %lo triple and deliberately do NOT share a base register, so no aggregate merge is needed; D_800EFC38 MATCHES an `extern s32 *D_800EFC38[];` array declaration (base hoisted to $s6 in the outer preheader, exactly as the target does); D_800EFC44 and D_800EFC50 are MISMATCH-unmeasured for the aggregate merge — they are the [3] and [6] words of D_800EFC38 but the target loads them at 0x8005C790-0x8005C79C through their own lui %hi / lw %lo pairs rather than through the already-live $s6 base, so spelling them as D_800EFC38[3]/[6] would emit addressing the bytes contradict; the independent-scalar spelling already used at src/text1b.c:2517 is what reaches score 5.

- [s1] Register allocation at floor 5 is IDENTICAL to the target for every value: s4=next, s3=i, s2=off, s1=p, s0=voice, s5=constant 1, s6=&D_800EFC38. Frame is -0x58 with the same eight callee-saved slots at 0x38-0x54, the same outgoing stack-argument slots at 0x10/0x14/0x18/0x1C, and every %hi/%lo relocation in the same instruction index as the target.

- [s1] The entire score-5 residual is one delay-slot decision: the target fills the `beq $v0,$s5,.L8005C838` delay slot with `addiu $v0,$s0,0x1` (reorg.c pulls the loop-continue increment out of the .L8005C838 thread and retargets the label past it) and lets `li 6` fill the following `lhu $a1,0x0($s1)` load-delay slot; our build has sched2 place `li $v0,6` above the branch, so reorg takes it for the branch slot, a nop is required after the lhu, and the increment stays split as `addiu $v0,$s0,1 / addu $s0,$v0,$zero` at the loop bottom. That is 3 mismatched slots plus 1 extra instruction.

- [s1] Three distinct GCC decisions hang on how the byte offset `i * 8` is spelled at each of its four use sites, and they are NOT independent: all four through one top-of-loop local gives no LICM hoist but a cross-jumped guard (26); all inline gives the right guard but a LICM hoist of %hi(D_800EFB78) (20); load through its own local `o`, clear inline, volumes through `off` declared inside the voice guard gives all three right at once (5).

- [s1] Integration note for whoever lands this: src/text1b.c:5928 currently declares `extern s32 func_8005C6D0(void);` later in the same TU; it must become `extern void func_8005C6D0(void);` because the target sets no return value. `D_800EFC38` must be declared at BLOCK scope inside the function since src/text1b.c:2617 already has a conflicting file-scope `extern s32 D_800EFC38;`.

- [s1] Siblings func_80070188 (floor 696) and func_800720FC (floor 688) have no candidate.c and share no block with this function; nothing was transplantable. The load-bearing neighbour is func_8005C650 (src/text1b.c:2678), the already-byte-matching WRITER of the same pool, whose splat scalar-address spelling this session reused verbatim.

## s2 (structural, 2026-09-10) — MATCHED: honest floor 5 -> 0, oracle SHA1 confirmed

- [s2] RESULT. `sandbox func_8005C6D0 --disable all` => **score 0**, target_insns 118,
  build_insns 118. Full `verify-oracle` with the C body in src/text1b.c reports
  `"ok": true, "build_matches": true`. The final body is
  memory/grind/func_8005C6D0/candidate.c (installed at src/text1b.c:2700, with the later
  forward declaration at src/text1b.c:5976 changed from `extern s32 func_8005C6D0(void);`
  to `extern void func_8005C6D0(void);` exactly as the s1 integration note predicted).

- [s2] CHASSIS RE-MEASURE. s1's candidate re-measured at score 5 unchanged before any edit,
  so every s1 spelling conclusion was still chassis-valid at the start of this session.

- [s2] TOOLING (reusable on any text1b function, and the reason a 10-variant sweep was
  affordable):
  * `tmp/grind/func_8005C6D0/s2/probe.sh` + `mini_head.c` compile a MINIMAL TU containing
    only this function plus its externs, with the project's exact cpp/cc1 flags. The
    resulting assembly is IDENTICAL to the full-TU build for this function except for label
    numbering (verified by diff against tmp/grind/func_8005C6D0/dumps/text1b.s), and the
    RTL insn UIDs are identical too. A probe is ~3 seconds instead of a full sandbox build.
  * The instrumented cc1 at tools/gcc-2.7.2/cc1 already carries BB2_DBR_DEBUG=1 reorg.c
    tracing (added by an earlier campaign; see reorg.c:142-148). It prints, for every
    conditional jump, which thread reorg tried FIRST, whether it owned that thread, and for
    each candidate insn the exact reason it was rejected (`refset` / `setset` / `setneed` /
    `setsopp` / `trap`). That single trace replaced all delay-slot guessing.

- [s2] THE s1 RESIDUAL, ROOT-CAUSED FROM THE TRACE (not inferred). For the
  `beq $v0,$s5,.L8005C838` branch the s1 chassis produced:
  `DBRDBG thr insn=96 thread=100 opp=219 own=1 likely=0 tif=0`, then `trial=100 ... trap=1`
  (LOSE), then `trial=104 ... trap=0` WINNER. Reading tools/gcc-2.7.2/reorg.c:
  `fill_eager_delay_slots` (reorg.c:3812) calls `mostly_true_jump` (reorg.c:1379); for an EQ
  condition with no loop note adjacent to the target label that returns 0, so reorg tries
  the FALL-THROUGH thread first. In that thread the `lhu $a1,0($s1)` is rejected by the
  `! may_trap_p (pat)` guard and the very next insn, the `li $v0,6` compare constant, is
  stolen into the branch delay slot. That is the whole of the s1 score-5 residual: `li 6` in
  the branch slot, a `nop` after the `lhu`, and the loop-continue increment left split as
  `addiu $v0,$s0,1 / addu $s0,$v0,$zero` (+1 instruction).

- [s2] THE FIX, AND WHY IT IS ORDINARY C. `mostly_true_jump` returns 1 when
  `NEXT_INSN (target_label)` is a `NOTE_INSN_LOOP_VTOP`, and that note exists only where
  jump.c's `duplicate_loop_exit_test` (jump.c:2338) has ROTATED a top-tested loop. s1's inner
  scan was spelled `voice = next; if (voice < 0x18) { off = i*8; do { ... } while (voice <
  0x18); }` — an explicit do-while, which is never rotated, hence no VTOP note, hence
  prediction 0, hence fall-through-first. Spelling the same scan as a top-tested `for` INSIDE
  the same explicit guard — `voice = next; if ((s16)voice < 0x18) { vol_off = i*8; for (;
  (s16)voice < 0x18; voice = (s16)(voice + 1)) { ... } }` — gets the loop rotated, gets the
  VTOP note, flips reorg to try the loop-continue thread first, and fills the delay slot with
  `addiu $v0,$s0,0x1` exactly as the target does, with `li 6` landing in the `lhu` load-delay
  slot. Measured: **score 5 -> 2, build_insns 119 -> 118**. The explicit `if` guard is still
  required — it is what keeps the s1 `addu $s2,$v1,$zero` copy and stops the guard being
  cross-jumped: the same `for` with the offset assignment hoisted out of the guard emits
  `j <bottom test>` instead of the duplicated guard (106 insns; probe kept at
  tmp/grind/func_8005C6D0/s2/variants/v3_for_outer_off.c).

- [s2] THE LAST TWO INSTRUCTIONS. At score 2 the only divergence was the order of three insns
  in the argument-setup block: target `sll $a0,$s0,16 / sra $a0,$a0,16 / addiu $s4,$s0,0x1 /
  sll $a1,$a1,16`, build `addiu $s4,$s0,1 / sll $a0 / sra $a0 / sll $a1`. Cause:
  `next = (s16)(voice + 1);` was written BEFORE the SsUtKeyOnV call, so its RTL was emitted
  ahead of the argument setup and sched2 kept it there. Moving the statement to AFTER the
  call is semantically identical (`voice` is not touched by the call): the insn is then
  emitted after the call, and because `$s4` and `$s0` are both callee-saved it carries no
  dependency on the call, so sched2 hoists it back up into exactly the target's slot between
  the `$a0` sign-extension and the `$a1` shift. Measured: **score 2 -> 0**. Four other
  placements/shapes of the same statement (before `ev`, as the first statement of the
  if-body, `next = voice + 1` without the inner cast, and split into two assignments) all
  reproduce the score-2 order — the position relative to the CALL is the operative thing, not
  the expression's shape.

- [s2] OBJECT MODEL, RE-CONFIRMED BY MEASUREMENT ON THE MATCHING CHASSIS. The brief's
  SPLIT-AGGREGATE signal for D_800EFB7C / D_800EFB7D and its four flagged "declaration puns"
  point at the aggregate-merge family; the bytes contradict it, and this session MEASURED the
  merge-shaped alternative rather than only arguing from the disassembly. Declaring the pool
  as an array (`extern u16 *D_800EFB78[];` + `D_800EFB78[i*2]`, with `D_800EFB7C[]` /
  `D_800EFB7D[]` likewise) makes loop.c hoist `%hi(D_800EFB78)` into a callee-saved register,
  adding a preheader `la`, a save/restore pair and 8 frame bytes: 112 insns, frame 0x60 —
  structurally different from the target. Banked as
  rejected/array-decl-licm-hoist-frame-0x60.c. The split `(u8 *)&SYM + off` spelling, the
  same one the byte-matching sibling func_8005C650 already ships on main at
  src/text1b.c:2685-2689 against the same three symbols, is what reaches distance 0.

- [s2] THE TWO OFFSET LOCALS ARE LOAD-BEARING ON THE NEW CHASSIS TOO (re-measured, since s1's
  conclusion was taken on the do-while chassis). Folding `entry_off` and `vol_off` into one
  outer-loop local gives 105 insns (guard cross-jumped); additionally routing the clear site
  through it gives 99 insns. Both are structurally different functions, not reorderings. The
  winning arrangement is unchanged from s1: pool load through its own local, volumes through
  a local declared inside the voice guard, clear site inline `i * 8`.

- [s2] SIBLINGS. func_80070188 (floor 696) and func_800720FC (floor 688) still have no
  candidate.c and share no block with this function; nothing was transplantable in either
  direction, and nothing this session produced is spelling-specific to them. The transferable
  asset for them is the s2 TOOLING entry above (minimal-TU probe + BB2_DBR_DEBUG reorg
  trace), not any C form.
