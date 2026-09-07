# Evidence bank — func_80035280

## s1 (2026-09-07, recon)

OBJECT MODEL: the two globals the DATA MODEL flagged, measured, not asserted.

- **D_80106A73 (census `g_file_flags_byte`, 8-bit flag byte; bits 0/1/2 read by
  file_GetFlag0/1/2) — MISMATCH (measured score 56; the flag block is 4
  instructions short of the target with the current declaration).**
  Declared `extern u8 D_80106A73;` at src/code6cac_b.c:128 (nothing in
  include/*.h). The target's flag block (asm/funcs/func_80035280.s:25A90-25AEC,
  24 instructions) emits THREE `lbu $v1,0x0($a1)` of D_80106A73 and THREE
  `sw $v0,0x20($t0)`. Compiled from the scalar declaration through a pointer
  (`u8 *f = &D_80106A73; ... *f & 1 ... *f & 2 ... *f & 4`), GCC emits ONE of
  each — a 20-instruction block, build_insns 107 vs target 108
  (rejected/s1-plain-deref-flag-cse-collapses-block.c). Mechanism, read from the
  compiler source this session:
  * expr.c:4567-4577 — an INDIRECT_REF gets MEM_IN_STRUCT_P only if its address
    tree is a PLUS_EXPR, or is an ADDR_EXPR of an aggregate, or the referenced
    type is aggregate. A plain `*f` is none of those, so the load is a
    non-in-struct MEM.
  * cse.c:7565-7576 — the `p[8] = v` store sets `writes_ptr->nonscalar = 1` and
    never `all`, because the guard is
    `if (! ((MEM_IN_STRUCT_P (written) || GET_CODE (XEXP (written,0)) == PLUS)
    && GET_MODE (written) != QImode)) writes_ptr->all = 1;`
    and the store's address IS a PLUS, in SImode.
  * cse.c:1701 invalidate_memory — with only `nonscalar` set it removes an entry
    solely when `p->in_struct` or `cse_rtx_addr_varies_p (p->exp)`; and
    cse_rtx_addr_varies_p returns 0 for a MEM whose address is a REG with a
    known constant qty, which `&D_80106A73` is. The flag load therefore survives
    all three stores, is CSE'd, and the two intermediate `sw`s then die.

  MEASURED FIX: spelling the same read so its address tree is a PLUS_EXPR —
  `src[3]` with `src = (u8 *)&D_80106A73 - 3` (the loop-1 walker base the target
  itself derives with `addiu $a2,$a1,-0x3`) — restores all six instructions:
  build_insns 111, and the emitted flag block becomes
  `lw / lbu 0(a0) / and / andi / or / sw / lbu / and / andi / or / sw / lbu /
  and / andi / or / sw`, structurally the target's block modulo register names
  and two scheduler slots.

  CONCLUSION: the declaration is under-specified. The honest form is an
  AGGREGATE covering 0x80106A70..0x80106A73 (the loop-1 source bytes A70/A71/A72
  plus the flag at A73), which makes the flag read an ARRAY_REF — expr.c:4888
  sets MEM_IN_STRUCT_P = 1 unconditionally on the component path — and
  reproduces the target with no pointer trick at all. NOTE the collision to
  resolve first: include/code6cac.h:472 already declares `extern u8
  D_80106A70[3];`, and the already-matched sibling func_80034F88
  (src/code6cac_b.c:3573+) reads `u8 *q = &D_80106A73;` as a separate scalar and
  its own INTEGRATION HANDOFF note explicitly asks for D_80106A70 to stay `[3]`
  with D_80106A73 "left as its own scalar". Widening D_80106A70 to [4] (or
  declaring a 5-byte object based at A73 — named_syms.txt:2560 records
  `g_file_flags_plus_5 = 0x80106A78`) must be measured against func_80034F88
  before it is proposed.

- **D_80106A58 (sub-symbol +4 of g_file_disc_type @80106A54) — MATCHES; no
  declaration change is needed or wanted.** The existing `extern s32
  D_80106A58;` (src/code6cac_b.c:131) with the use-site `base = (u8
  *)&D_80106A58;` reproduces the target's loop-2 addressing exactly: ONE
  induction register with `lw 0x4($a2)` / `lbu 0x0($a2)` / `addiu $a2,$a2,8`,
  provided the loop is spelled `s = base + i * 8` (candidate.c) rather than a
  self-incrementing `base += 8` walker. The walker spelling makes GCC form TWO
  givs (`lw 0(t0)` with `t0 = t2 + 4`, plus `lbu 0(t2)`, and an `addiu` on both)
  — 2 instructions the target does not have; banked at
  rejected/s1-walker-two-IV-floor56.c. Independent corroboration: sibling
  func_8003C714 (src/code6cac_c2.c), whose loop 2 is line-for-line this loop,
  reached a Judge PASS on 2026-09-05 with `extern s32 D_80106A58;` and
  `base = (u8 *)&D_80106A58;` UNTOUCHED, and the `extern u8 D_80106A58[24]`
  retype is on that function's banned_constructs list. So the SPLIT-AGGREGATE
  signal is a true description of the data (an 8-byte record array — byte at +0,
  s32 frame count at +4, three records at A58/A60/A68) but is NOT this
  function's residual: the aggregate-merge family buys nothing here.

FACT 1 — chassis. HEAD is `INCLUDE_ASM("asm/funcs", func_80035280);`
(src/code6cac_b.c:3741), so the dispatch-time sandbox reads score 108 /
build_insns 0 / no_c_body true. That 108 is the no-body sentinel, not a floor.
The first real C body measured 56. SESSION FLOOR = 56.

FACT 2 — semantics, fully recovered, nothing left ambiguous. The function calls
`p = func_80077D00();`, then merges bits 0/1/2 of the flag byte at 0x80106A73
into `p[8]` one bit at a time, re-storing `p[8]` after each bit; then copies the
three bytes at 0x80106A70..A72 into BOTH `((u8 *)p)[0x17+i]` and
`((u8 *)p)[0x1D+i]`; then, for each of three 8-byte records based at 0x80106A58,
formats the s32 frame count at record+4 as a clock — `dst[0x21] = x / 1800`
(minutes at 30 fps), `dst[0x22] = (x / 30) % 60` (seconds), `dst[0x23] =
(x % 30) * 100 / 30` (hundredths of a second), `dst[0x24] = record[0]` — writing
into `((u8 *)p) + i*4`. Every magic constant in the target is accounted for:
0x91A2B3C5 with `sra 10` plus add-back is signed /1800; 0x88888889 with `sra 4`
is signed /30 and with `sra 5` is signed /60; `((v<<4)-v)<<2` is *60,
`((v<<4)-v)<<1` is *30, `(((v<<1)+v)<<3 + v)<<2` is *100. No arithmetic is in
dispute — the entire residual is placement.

FACT 3 — the LICM residual, measured on THIS function, not inherited.
tmp/grind/func_80035280/dumps/code6cac_b.loop, func_80035280 slice:

    Loop from 109 to 255: 62 real insns.
    Insn 135: regno 112 (life 1), move-insn savings 1  moved to 273
    Insn 153: regno 118 (life 35), move-insn savings 1  moved to 275

regno 112 is the 0x91A2B3C5 (/1800) constant; regno 118 is 0x88888889 (/30).
The target hoists ONLY 0x88888889 (asm/funcs/func_80035280.s:25B1C/25B20, in the
preheader) and materialises 0x91A2B3C5 INSIDE the loop (25B30/25B38, where the
`ori` also fills the `lw 0x4($a2)` load-delay slot). The build hoists both,
which costs +2 preheader instructions and loses the delay-slot fill. Gate, read
from the compiler source this session:

    loop.c:1631  move iff `already_moved[regno]
                           || (threshold * savings * m->lifetime) >= insn_count
                           || (m->forces && m->forces->done
                               && n_times_used[m->forces->regno] == 1)`
    loop.c:532   threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)
    loop.c:897   savings = 1, HARD-CODED, for a `move_insn` (const-load) movable
    loop.c:791   m->lifetime = uid_luid[regno_last_uid] - uid_luid[regno_first_uid]
    loop.c:1609  `if (moved_once[regno]) insn_count *= 2;`
    loop.c prescan_loop — loop_has_call is set ONLY by a real CALL_INSN inside
                 the NOTE_INSN_LOOP_BEG..NOTE_INSN_LOOP_END range.

With no call, threshold = 122, so 122*1*1 = 122 >= 62 and it hoists. `savings`
cannot be 0 (hard-coded at loop.c:897) and `lifetime` cannot be below 1 for a
set plus a later use, so the product is pinned at >= 122. The only two ways to
keep the constant in the loop are (A) loop_has_call = 1, which drops threshold
to 61 so 61 < 62 by a margin of exactly one instruction, or (B) insn_count > 122
— reachable either as 123 raw RTL insns in the loop, or as 62 doubled to 124 by
loop.c:1609 when `moved_once[regno]` is already set.

FACT 4 — route (A) is PROVEN sufficient and PROVEN unavailable here.
rejected/s1-diagnostic-real-call-in-loop2-adds-jal.c puts a real
`func_800344B4()` call in the terminal arm of a `while (1)` loop-2. Measured:
build_insns 110 (= 108 + the jal + its delay slot), score 57, and the loop-2
preheader and body become instruction-for-instruction the target's:

    preheader: move a3,zero / lui 0x8888 / ori / lui %hi(D_80106A58) / addiu / move
    loop head: lui v0,0x91a2 / lw v1,4(base) / ori v0,0xb3c5 / mult v1,v0 /
               mfhi / addu / sra 0xa / sra 0x1f / subu / sb 0x21(dst)

compared against asm/funcs/func_80035280.s:25B18-25B54. Subtract the two call
instructions and the count is exactly 108 == 108. But func_80035280 has no
callee anywhere in or after loop 2 — its target epilogue at 25C20 is
`lw ra / addiu sp / jr ra / nop` — so there is no honest statement to relocate
into the loop. This is exactly where this function DIVERGES from sibling
func_8003C714, which closed on 2026-09-05 by moving its real
`func_8001CD68(buf)` call into the loop's terminal arm (docs/grind/decisions.md,
2026-09-05 19:41 final call PASS). The mechanism is shared; the fix is not
transferable.

FACT 5 — everything except the LICM lever already matches in SHAPE. With the
flag-block fix and the `s = base + i * 8` loop-2 spelling, build_insns is 109
against target 108, and the only structural delta is the 0x91A2B3C5 preheader
hoist. The remaining score (63) is a register-NAMING permutation across the
whole body (build a1/v0/v1/a0 where the target has v0/v1/a0/a1 in the flag
block; build t0/t2/t3 where the target has a2/t1/v0 in loop 2), and it is
DOWNSTREAM of the LICM decision — the diagnostic form, whose LICM is correct,
scores 57 while carrying two extra call instructions. Do NOT grind register
allocation before the LICM lever is settled; the naming distance is expected to
collapse once the preheader is right.

FACT 6 — provenance and scope facts worth not re-deriving.
* `func_80077D00` returns the same object func_8003504C and func_80034F88 use;
  `p[8]` is byte offset 0x20, and the three flag bits land there.
* The sandbox object disassembles the leading call as `func_800272FC` because
  the object is unlinked and the reloc is unresolved — that is an artefact, not
  a wrong callee.
* `pwsh tools/grinder/dump.ps1 func_80035280` prints roughly twenty
  conflicting-declaration warnings for OTHER functions in code6cac_b.c. They are
  pre-existing on HEAD and unrelated to this function.
* Helpers written this session and kept in scratch:
  tmp/grind/func_80035280/s1/slice.py (slices a whole-file cc1 -da dump on
  ";; Function ") and tmp/grind/func_80035280/s1/apply.py &lt;body.c&gt; (drops a
  body over the INCLUDE_ASM line, or replaces an existing body, LF-safe).

- [s1] OBJECT MODEL: D_80106A73 (census g_file_flags_byte) MISMATCH, measured score 56 - the scalar `extern u8 D_80106A73;` at src/code6cac_b.c:128 read as a plain deref yields build_insns 107 vs target 108 and a 20-instruction flag block against the target's 24 (asm/funcs/func_80035280.s:25A90-25AEC), because expr.c:4567-4577 leaves MEM_IN_STRUCT_P clear on a non-PLUS_EXPR INDIRECT_REF and cse.c:7565-7576 plus cse.c:1701 therefore never invalidate the load across the three p[8] stores; respelling the read with a PLUS_EXPR address (src[3], src = &D_80106A73 - 3) restores all three lbu and all three sw at build_insns 111. D_80106A58 (sub-symbol +4 of g_file_disc_type) MATCHES - the existing `extern s32 D_80106A58;` at src/code6cac_b.c:131 with `base = (u8 *)&D_80106A58;` already reproduces the target's single-induction lw 0x4($a2) / lbu 0x0($a2) / addiu 8 addressing when the loop is spelled `s = base + i * 8`, and sibling func_8003C714 took a Judge PASS with that declaration untouched while the `[24]` retype is on its banned list, so the flagged aggregate merge is not this function's residual.

- [s1] The dispatch-time 108 is the no-C-body sentinel (HEAD carries INCLUDE_ASM at src/code6cac_b.c:3741, build_insns 0, no_c_body true), not a floor. First real body measured 56; that is the session floor.

- [s1] Semantics fully recovered, nothing left ambiguous: merge bits 0/1/2 of the flag byte at 0x80106A73 into p[8] one bit at a time with a re-store after each; copy the three bytes at 0x80106A70..A72 into both ((u8 *)p)[0x17+i] and ((u8 *)p)[0x1D+i]; then for each of three 8-byte records based at 0x80106A58 format the s32 frame count at record+4 as a clock - dst[0x21] = x/1800 (minutes at 30 fps), dst[0x22] = (x/30)%60, dst[0x23] = (x%30)*100/30, dst[0x24] = record[0] - writing into ((u8 *)p) + i*4. Every magic in the target is accounted for: 0x91A2B3C5 with sra 10 plus add-back is signed /1800, 0x88888889 with sra 4 is /30 and with sra 5 is /60, ((v<<4)-v)<<2 is *60, ((v<<4)-v)<<1 is *30, (((v<<1)+v)<<3+v)<<2 is *100.

- [s1] The LICM gate is pinned by three hard numbers read from the compiler source, so only insn_count is movable: loop.c:897 hard-codes savings = 1 for a constant-load movable (it can never be 0), loop.c:791 makes lifetime at least 1 for a set with a later use, and loop.c:532 fixes threshold at 122 without a call. The product is therefore always >= 122, and the only ways to keep 0x91A2B3C5 in the loop are loop_has_call = 1 (threshold 61 < insn_count 62, margin one instruction) or insn_count > 122 - reachable as 123 raw RTL insns or as 62 doubled to 124 by loop.c:1609 `if (moved_once[regno]) insn_count *= 2;`.

- [s1] This function shares its residual mechanism with sibling func_8003C714 (src/code6cac_c2.c), whose loop 2 is line-for-line this loop, but NOT its fix: func_8003C714 closed on 2026-09-05 by relocating its real func_8001CD68(buf) call into the loop's terminal `if (i >= 3) { ...; break; }` arm, and func_80035280 has no callee to relocate. func_8003C714's rejected/ directory holds two banked insn_count-padding forms that reached 123 and measured sandbox 0 and were then refused as inadmissible padding - read those before re-deriving route (B) here.

- [s1] Once the LICM decision is correct the rest of the body already matches in shape: the call-in-loop diagnostic makes the loop-2 preheader (move a3,zero / lui 0x8888 / ori / lui %hi(D_80106A58) / addiu / move) and the loop head (lui 0x91a2 / lw 4(base) / ori 0xb3c5 / mult / mfhi / addu / sra 10 / sra 31 / subu / sb 0x21(dst)) instruction-for-instruction identical to asm/funcs/func_80035280.s:25B18-25B54. The residual 63 at 109/108 is a register-NAMING permutation downstream of the hoist, so register allocation should not be ground before the hoist is settled.

- [s1] Reusable scratch tooling written this session: tmp/grind/func_80035280/s1/slice.py splits a whole-file cc1 -da dump on ';; Function ' so a single function's pass output can be grepped without paging the whole file, and tmp/grind/func_80035280/s1/apply.py <body.c> drops a body over the INCLUDE_ASM line (or replaces an existing body) with LF endings preserved.

## s2 (2026-09-07, structural)

FACT 7 — THE FLOOR MOVED, 56/63 -> 39, FROM SPELLING ALONE. Writing every
loop-2 memory reference with its index inline (`((u8 *)p)[i * 4 + 0x21]`,
`*(s32 *)(base + i * 8 + 4)`) instead of introducing `u8 *s` / `u8 *dst` walker
locals measures `sandbox func_80035280 --disable all` = 39 at build 109 /
target 108 (memory/grind/func_80035280/candidate.c, this session's body). The
s1 candidate measured 63 on the same chassis this session, so the delta is real
and is entirely register naming: with the inline spelling GCC allocates
p -> $t0, the flag-byte pointer -> $a0 with its walker in $a2, the loop-2
counter -> $a3, the destination walker -> $a1, the record walker -> $a2 and
0x88888889 -> $t1, which is exactly the target's assignment
(asm/funcs/func_80035280.s:25B18-25B54); the s1 spelling put p in $t1 and the
counter in $t0 and permuted the whole loop-2 temp set. Two other natural
spellings measured worse and are banked:
rejected/s2-byte-field-first-adds-insn-score67.c (score 67, build 110 — moving
`dst[0x24] = *s` to the top of the body costs an instruction) and
rejected/s2-locals-collapse-three-lw-into-one.c (score 93, build 92 — reading
the frame count into three locals before any store lets cse1 merge the target's
three `lw 0x4($a2)` into one, which is a shape error, not a naming error).

FACT 8 — THE loop.c:1631 GATE IS NOW MEASURED, NOT DERIVED. s1 computed
threshold = 122 from loop.c:532 and the hard floors of savings and m->lifetime.
s2 measured the boundary directly by padding the loop-2 body with extra stores
and reading the .loop dump (tmp/grind/func_80035280/s2/pad*.c, generator
tmp/grind/func_80035280/s2/gen_pad.py):

    insn_count 117 -> "moved to ..."      (0x91A2B3C5 hoisted)
    insn_count 120 -> "moved to ..."      (hoisted)
    insn_count 123 -> "not desirable"     (STAYS IN THE LOOP)
    insn_count 135 -> "not desirable"
    at every one of those counts 0x88888889 (life 35) still hoists

So the product threshold * savings * m->lifetime is exactly 122 and the gate
flips between 120 and 123 — s1's arithmetic is confirmed by measurement. The
requirement is insn_count >= 123.

FACT 9 — NATURAL SPELLINGS DO NOT REACH insn_count 123, MEASURED. The .loop
dump's "Loop from N to M: K real insns" for loop 2, across the four natural
bodies tried in s1+s2:
    s1 candidate (s + dst walker locals)      62
    byte-field-first reorder                  62
    inline-index spelling (this candidate)    55
    three values into locals, store at end    44
The best natural spelling is 62 against a requirement of 123. Nothing in the
44-62 band is within a factor of two of the gate.

FACT 10 — THE s1 FRONTIER's `moved_once` DOUBLING IS SELF-DEFEATING, MEASURED.
The s1 frontier's cheapest untried route was loop.c:1609
`if (moved_once[regno]) insn_count *= 2;`, on the theory that 62 -> 124 > 122
would leave the constant in the loop. s2 built the only construct that can set
moved_once for that pseudo — an inner loop inside loop 2 containing the /1800
division, so the inner loop's scan_loop hoists it first (loops are scanned last
first, loop.c:430-433 "scan the loops, last ones first", so a loop nested inside
loop 2 is the only loop that can run before loop 2). Banked at
rejected/s2-nested-loop-movedonce-inflates-lifetime.c. The .loop dump shows the
doubling FIRING and the constant being hoisted anyway:

    Loop from 130 to 172: 13 real insns.
    Insn 144: regno 114 (life 1), move-insn savings 1  moved to 304
    Loop from 106 to 281: 67 real insns.
    Insn 304: regno 114 (life 18), move-insn savings 1 halved since already moved  moved to 306

The mechanism defeats itself: hoisting the constant out of the inner loop moves
its SET to the inner preheader while its USE stays put, so m->lifetime
(loop.c:791, a function-wide luid span) went 1 -> 18. The gate is then
122 * 1 * 18 = 2196 >= 134, which passes comfortably. Because any event that
sets moved_once[regno] for this pseudo is precisely an earlier move of that
pseudo's set to an enclosing preheader, lifetime is >= 2 afterwards, and
122 * lifetime >= 2 * insn_count holds for every insn_count <= 122 — the
doubling never lowers the bar below the raw insn_count >= 123 route. The s1
frontier item F1 collapses into F2 and buys nothing.

FACT 11 — THE RESIDUAL IS A COMPILER-CONFIGURATION DIFFERENCE, MEASURED
END-TO-END. The PS1 R3000 has no FPU. GCC 2.7.2's CONDITIONAL_REGISTER_USAGE
marks every FP hard register fixed under -msoft-float, which drops
n_non_fixed_regs by ~32 and therefore halves the loop.c:532 threshold
`(loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`. Our CC_FLAGS (Makefile:35,
engine/buildconfig.py:43) do NOT pass -msoft-float, so cc1 runs with the
hardware-FP register set and threshold 122. Measured, on the s1 candidate body
(loop-2 insn_count 62):

  * with -msoft-float the .loop dump prints
        Insn 135: regno 112 (life 1), move-insn savings 1 not desirable
        Insn 153: regno 118 (life 35), move-insn savings 1  moved to 273
    i.e. 0x91A2B3C5 STAYS IN THE LOOP and 0x88888889 still hoists — exactly the
    target's split;
  * the object built through the real pipeline with the flag added
    (tmp/grind/func_80035280/s2/build_o.sh, artifact
    tmp/grind/func_80035280/s2/soft.o) has build_insns 108 == target 108, with
    the loop head emitted as
        lui v0,0x91a2 / lw v1,4(base) / ori v0,0xb3c5 / mult v1,v0 / mfhi /
        addu / sra 0xa / sra 0x1f / subu / sb 0x21(dst)
    instruction-for-instruction the target's asm/funcs/func_80035280.s:25B18+,
    with NO call and NO padding. Its engine score is 55 — the +1 instruction and
    the entire structural residual are gone, and what remains is pure register
    naming (the s1 body's naming, since that was the body measured; the s2
    candidate's naming is 24 points better and has not yet been measured under
    the flag because its loop-2 insn_count of 55 falls below the soft-float
    threshold and would hoist again).

  ORACLE IMPACT, MEASURED ACROSS THE WHOLE PROJECT. cc1 output was generated for
  all 32 src/*.c stems with and without -msoft-float and compared line by line,
  ignoring only the cc1 flag-echo comment line
  (tmp/grind/func_80035280/s2/sf3.py):
        IDENTICAL: 31 of 32
        DIFFERS:   code6cac_b, 8 diff lines
  The single differing function is func_800324D0 (src/code6cac_b.c:2554), where
  the flag changes one `li 0x000000ff` from $8 to $2 and the matching `bne`.
  Scored against build/src/code6cac_b.o: func_800324D0 is 0 without the flag and
  3 with it. That function currently matches only through a /* FAKE */
  duplicated-statement-into-arms construct whose stated mechanism is forcing the
  walker pseudo into $v1 via reg_n_refs; its natural (un-duplicated) spelling
  was measured this session at score 27 under BOTH configurations, so the FAKE
  is not merely a hard-float artefact and the flag does not make it removable.

  NET: adopting -msoft-float would close func_80035280's structural residual and
  is codegen-neutral for every other function in the project except
  func_800324D0, which would need re-grinding under the new configuration from a
  residual of 3. That is a toolchain-configuration decision on surfaces a grind
  session may not touch (Makefile, engine/buildconfig.py) and is governed by
  .claude/rules/no-compiler-divergence.md. It is recorded here as evidence, NOT
  acted on, and NOT proposed as a candidate.

FACT 12 — DOORS IN loop.c CHECKED AND CLOSED, so no future session re-reads
them. A constant-load movable is always created for this insn: loop.c:695 skips
only pseudos created BY loop optimization (regno >= max_reg_before_loop); the
maybe_never / call_passed guards at loop.c:694-716 do not apply because
REG_USERVAR_P is false for a compiler temp and may_trap_p is 0 for a CONST_INT;
`savings` is n_times_used[regno], which loop.c:597 bcopies from n_times_set, so
it is the number of SETS in the loop and cannot be 0; and the only way to
may_not_optimize the pseudo (count_loop_regs_set, loop.c:3037-3048) is to set
the same pseudo in two basic blocks, or twice with a use between, both of which
materialise the constant twice and cost bytes. The gate at loop.c:1631
therefore has exactly one input left that C can move, insn_count.

FACT 13 — reusable s2 tooling, all under tmp/grind/func_80035280/s2/:
  loopslice.py       prints the func_80035280 slice of the .loop dump
                     (Loop-from / moved-to / halved / not-desirable lines only)
  gen_pad.py K out.c generates the body with K extra loop-2 stores, for
                     measuring the loop.c:1631 boundary
  dump_flags.sh "X"  regenerates all cc1 -da dumps with extra flags X
  build_o.sh "X" out builds code6cac_b.o through the REAL pipeline with extra
                     cc1 flags X (cpp | cc1 | prologue_fix | maspsx | multu_pad | as)
  score.sh obj ref   engine score_func for func_80035280 on any two objects
  sf3.py / sf6.py    the whole-project -msoft-float codegen parity sweep and the
                     func_800324D0 natural-spelling control

- [s2] Floor 39 on the HEAD chassis (build 109 / target 108) with the inline-index loop-2 spelling, measured this session; the s1 body re-measured at 63 and the s1 ledger floor of 56 came from a structurally worse two-giv walker form.

- [s2] Loop 2 in the new candidate is register-identical to asm/funcs/func_80035280.s:25B18-25B54 - dst $a1, record walker $a2, counter $a3, 0x88888889 $t1, p $t0 - with the only in-loop delta being the mfhi temp $t3 vs the target's $t2, which is displaced by the wrongly-hoisted 0x91A2B3C5 sitting in $t2.

- [s2] The loop.c:1631 desirability boundary is measured, not derived: 0x91A2B3C5 is hoisted at loop-2 insn_count 117 and 120 and refused at 123 and 135, so the product threshold * savings * m->lifetime is exactly 122 and the requirement is insn_count >= 123.

- [s2] Natural loop-2 spellings measure insn_count 62 (s1 walker locals), 62 (byte-field-first), 55 (inline index) and 44 (values into locals); the best is a factor of two below the 123 the gate requires.

- [s2] The loop.c:1609 moved_once doubling fires as advertised and still hoists: the nested-loop probe shows 'halved since already moved  moved to 306' because hoisting the constant out of the inner loop raised its m->lifetime from 1 to 18, and 122 * 18 = 2196 >= 134.

- [s2] Doors in loop.c checked and closed so they are not re-read: loop.c:695 (regno >= max_reg_before_loop) applies only to loop-created pseudos; the maybe_never / call_passed guards do not apply because REG_USERVAR_P is false for a compiler temp and may_trap_p is 0 for a CONST_INT; savings is n_times_used[regno] which loop.c:597 bcopies from the SET count and cannot be 0; and may_not_optimize (loop.c:3037-3048) requires the constant to be materialised twice, which costs bytes.

- [s2] -msoft-float halves the loop.c:532 threshold (PS1 has no FPU; CONDITIONAL_REGISTER_USAGE fixes the FP hard registers) and compiles func_80035280 to build_insns 108 == target 108 with the target's exact in-loop lui/lw/ori/mult sequence, no call and no padding, engine score 55 on the s1 body.

- [s2] -msoft-float is codegen-neutral for 31 of 32 src stems (cc1 output identical line for line ignoring the flag-echo comment); the only differing function project-wide is func_800324D0, which goes from score 0 to score 3, and whose natural un-FAKEd spelling scores 27 under both configurations.

- [s2] src/code6cac_b.c was restored to HEAD at the end of the session; no build file is left modified. All s2 diagnostics ran through scratch scripts under tmp/grind/func_80035280/s2/.

## s3 (2026-09-07, structural)

FACT 14 — THE FLOOR MOVED 39 -> 16, AND THE FLAG BLOCK IS NOW BYTE-EXACT.
Build insns 1..38 (prologue, the entire 24-instruction flag block, the entire
loop 1) are now IDENTICAL to asm/funcs/func_80035280.s, register for register
and slot for slot. `sandbox func_80035280 --disable all` = 16 at build 109 /
target 108 (memory/grind/func_80035280/candidate.c, this session's body). The
whole remaining 16 is the loop.c:1631 LICM residual described in FACT 3/8/11 —
the +2 preheader instructions for the wrongly-hoisted 0x91A2B3C5, the `nop` in
the `lw 0x4($a2)` load-delay slot the target fills with its `ori`, and the
$t2 -> $t3 displacement of the mfhi temp that the hoisted constant forces.

FACT 15 — THE MECHANISM OF THE FLAG-BLOCK SEAT PERMUTATION: local-alloc.c:472,
`reg_n_deaths[i] == 1`. This is the finding that moved the floor, and it is
read out of the compiler source and confirmed in two dumps, not inferred.

    tools/gcc-2.7.2/local-alloc.c:470-477
      for (i = FIRST_PSEUDO_REGISTER; i < max_regno; i++)
        {
          if (reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1
              && (reg_alternate_class (i) == NO_REGS
                  || ! CLASS_LIKELY_SPILLED_P (reg_preferred_class (i))))
            reg_qty[i] = -2;      /* eligible for LOCAL allocation */
          else
            reg_qty[i] = -1;      /* falls through to global-alloc */
        }

  With the s2 body's single reused accumulator `v`, the accumulator pseudo is
  SET FOUR TIMES, so it carries four REG_DEAD notes. The s2 .lreg dump says so
  literally: `Register 77 used 10 times across 16 insns in block 0; dies in 4
  places`. reg_n_deaths != 1, so reg_qty stays -1 and the accumulator is NOT a
  local quantity — the BB2_SUGG_DEBUG / BB2_QTY_DEBUG trace for block 0 lists
  exactly seven quantities (reg1 = 73, 78, 82, 83, 87, 88, 92) and the
  accumulator is not among them. local-alloc therefore hands $v0 and $v1 to the
  six short per-arm temps and $a0 to the address pointer (QTYDBG blk=0
  ord=0..6 ... got = 2,2,2,3,3,3,4), and only then does global-alloc run and
  find the accumulator conflicting with hard regs 2, 3 and 4 (the greg slice
  prints `;; 77 conflicts: 72 74 76 77 2 3 4 29`). It gets $a1, the only
  argument register left. That is the entire 4-cycle:
      build  (addr $a0, acc $a1, mask $v1, byte $v0)
      target (addr $a1, acc $v0, mask $a0, byte $v1)

  THE FIX: give the accumulator chain reg_n_deaths == 1 per pseudo by splitting
  it into one local per merged bit (flags / flags0 / flags1 / flags2). Each is
  then a block-0 quantity; local-alloc.c's qty_compare priority is
  floor_log2(n_refs) * n_refs * size / (death - birth), which ranks the
  accumulator chain ahead of the two-reference per-arm temps, so find_free_reg
  hands it $v0 first and everything else falls into the target's seats.
  MEASURED: 39 -> 18 from the split alone. Confirmation in the s3 dumps: the
  global list drops from `;; 13 regs to allocate` to `;; 12 regs to allocate`,
  and no block-0 pseudo in the .lreg slice says "dies in 4 places" any more.

FACT 16 — DECLARATION AND STATEMENT ORDER DO NOT MOVE THE SEATS; THE DEATH
COUNT DOES. Both of the obvious structural levers measured EXACTLY 39, i.e. no
change at all, on the single-accumulator body: declaring `v` first among the
locals, and hoisting `v = p[8];` above the `f`/`src` pointer setup. Dropping the
`f` local entirely and spelling `src = (u8 *)&D_80106A73 - 3;` measured 44 at
build 111 (two extra instructions — the target genuinely keeps both $a1 and
$a2). A TWO-way alternating split (fA/fB) measured 28: each pseudo then has
reg_n_deaths == 2, which still fails the `== 1` test, so it only helps
partially. Only the full one-local-per-bit split reaches 18. All four are
banked in memory/grind/func_80035280/rejected/.

FACT 17 — THE LOOP-1 COUNTER ZEROING IS A PLACEMENT LEVER WORTH 2 POINTS. The
target's first instruction after the jal's delay slot is `addu $a3,$zero,$zero`.
Spelling loop 1 as `for (; i < 3; i++)` with the initialisation written as a
statement measures, by placement:
    `i = 0;` immediately after `p = func_80077D00();`   16  (build 109)  <= target
    `i = 0;` immediately before `flags = p[8];`         18  (build 109)
    `i = 0;` immediately before loop 1 (== `for (i=0;`) 18  (build 109)
    `s32 i = 0;` declaration initialiser                29  (build 110)
The declaration-initialiser form is emitted BEFORE the call, so the value has to
survive it and GCC spends an extra instruction; that is a genuine +1 on
build_insns, not a naming difference.

FACT 18 — THE LICM RESIDUAL IS UNCHANGED BY THE NEW BODY, RE-MEASURED. The s3
.loop dump slice for this candidate reads
    Loop from 110 to 274: 55 real insns.
    Insn 137: regno 115 (life 1), move-insn savings 1  moved to 292
    Insn 165: regno 130 (life 31), move-insn savings 1  moved to 294
so loop-2 insn_count is 55 against the 123 the gate requires (s2 FACT 8), and
both constants are still hoisted. A re-read of tools/gcc-2.7.2/loop.c:1584-1633
this session confirms there is no further C-visible input: the guard reaching
the desirability test is `!m->done && (!m->cond || invariant_p(...)) &&
(!m->forces || m->forces->done)`, and the test itself is exactly
`already_moved[regno] || (threshold * savings * m->lifetime) >= insn_count ||
(m->forces && m->forces->done && n_times_used[m->forces->regno] == 1)`,
with `savings = m->savings`. Nothing new.

FACT 19 — -msoft-float RE-MEASURED ON THE NEW BODY: IT DOES **NOT** CLOSE THIS
BODY, and the reason sharpens s2's FACT 11 into a usable number. Built through
the real pipeline with `-msoft-float` (tmp/grind/func_80035280/s2/build_o.sh,
artifact tmp/grind/func_80035280/s3/soft_v6.o) the s3 candidate scores 16 at
build 109 — identical to the hard-float build. The reason is the threshold:
-msoft-float fixes the 32 FP hard registers, so loop.c:532's
`(loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` falls from 122 to about 58,
and this body's loop-2 insn_count of 55 is still BELOW it, so the constant is
hoisted again. The s1 walker body's insn_count of 62 is above it, which is why
s2 measured that body at 108 == 108 under the flag. So the configuration
question, if it is ever taken, is NOT "add the flag": it is "add the flag AND
find a loop-2 spelling whose insn_count lands in [59, 122] while keeping the
inline-index giv shape". The s1 walker spelling clears 59 but costs two giv
instructions (build 111). Bracketed both ways this session; do not re-derive.

FACT 20 — s3 tooling, all under tmp/grind/func_80035280/s3/:
  dis.py <out>   slices func_80035280 out of an objdump capture in raw.txt into
                 one-instruction-per-line form for pasting against target.txt
  target.txt     the 108 target instructions, one per line, from asm/funcs
  batch.ps1      applies each body listed in todo.txt and prints its sandbox
                 score and build_insns — one PowerShell call per batch
  qty.sh         rebuilds code6cac_b through cpp | cc1 with BB2_QTY_DEBUG=1 and
                 BB2_SUGG_DEBUG=1 into tmp/grind/func_80035280/s3/qty.txt.
                 NOTE: the instrumented cc1 is tools/gcc-2.7.2/cc1, NOT the
                 engine's tools/gcc-2.7.2/build/cc1 — the build/ binary emits
                 zero QTYDBG lines. Diagnostics only.
  v/v1..v9.c     the nine bodies measured this session
  soft_v6.o      the s3 candidate built with -msoft-float (FACT 19)

- [s3] Floor 39 -> 16 on the HEAD chassis (build 109 / target 108). Build instructions 1..38 - prologue, the whole 24-instruction flag block and the whole of loop 1 - are now identical to asm/funcs/func_80035280.s register for register; the entire remaining score is the loop.c:1631 LICM residual.

- [s3] The flag-block register permutation was caused by local-alloc.c:472's eligibility test `reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1`: a single reused accumulator `v` is set four times, so it carries four REG_DEAD notes (.lreg: "Register 77 ... dies in 4 places"), fails the test, and is left to global-alloc, which runs after local-alloc has already given $v0/$v1/$a0 to the short per-arm temps (greg: ";; 77 conflicts: 72 74 76 77 2 3 4 29"). Splitting the accumulator into one local per merged bit gives each pseudo reg_n_deaths == 1, makes them block-0 quantities, and local-alloc's qty_compare priority (floor_log2(refs)*refs*size/(death-birth)) then puts the chain on the target's $v0 seat. Measured 39 -> 18 from the split alone.

- [s3] Declaration order and statement order do NOT move the flag-block seats on this function: declaring `v` first and hoisting `v = p[8];` above the pointer setup both measured exactly 39, unchanged. Dropping the `f` pointer measured 44 at build 111. A two-way alternating split (fA/fB) measured 28 because each pseudo still has reg_n_deaths == 2. Only the full one-local-per-bit split reaches 18.

- [s3] The loop-1 counter zeroing is a placement lever worth 2 points: `i = 0;` written as a statement immediately after the func_80077D00() call with `for (; i < 3; i++)` measures 16, the same statement before the flag block or immediately before loop 1 measures 18, and the `s32 i = 0;` declaration initialiser measures 29 at build 110 because the value then has to survive the call.

- [s3] -msoft-float does NOT close the s3 body: it scores 16 at build 109 under the flag, identical to hard float, because the flag drops the loop.c:532 threshold only to about 58 and this body's loop-2 insn_count is 55. The configuration question is therefore "flag AND a loop-2 spelling with insn_count in [59,122] keeping the inline-index giv shape", not "flag alone"; the s1 walker spelling clears 59 but costs two giv instructions.

- [s3] The instrumented cc1 carrying the BB2_QTY_DEBUG / BB2_SUGG_DEBUG local-alloc hooks is tools/gcc-2.7.2/cc1; the engine's configured compiler tools/gcc-2.7.2/build/cc1 (engine/buildconfig.py:19) emits zero QTYDBG lines, so a diagnostic run must override CC1 explicitly.

- [s3] Floor 39 -> 16 on the HEAD chassis (build 109 / target 108). Build instructions 1..38 - prologue, the whole 24-instruction flag block and the whole of loop 1 - are now identical to asm/funcs/func_80035280.s register for register and slot for slot; the entire remaining score of 16 is the loop.c:1631 LICM residual (the +2 preheader instructions for the wrongly hoisted 0x91A2B3C5, the nop in the lw 0x4($a2) load-delay slot the target fills with its ori, and the mfhi temp displaced from $t2 to $t3).

- [s3] local-alloc.c:470-477 is the gate that decided the flag-block registers: reg_qty[i] = -2 (eligible for LOCAL allocation) only when reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1. A reused accumulator written four times has four REG_DEAD notes, fails the test, and is allocated by global-alloc after local-alloc has already given $v0/$v1/$a0 away - producing the exact 4-cycle build (addr $a0, acc $a1, mask $v1, byte $v0) vs target (addr $a1, acc $v0, mask $a0, byte $v1).

- [s3] Nine bodies measured this session, all at build 109 unless noted: s2 single accumulator 39; v declared first 39; v = p[8] hoisted 39; `f` dropped 44 (build 111); three-way accumulator split 18; two alternating accumulators 28; three-way split + `i = 0;` after the call 16; same with `s32 i = 0;` initialiser 29 (build 110); same with `i = 0;` just before loop 1 18.

- [s3] Declaration order and statement order are inert on this function's flag block - the death count is the lever. Only the full one-local-per-bit split reaches the target's seats; a two-way alternating split leaves reg_n_deaths == 2 and only gets to 28.

- [s3] The loop-1 counter zeroing is a placement lever worth 2 points, and the `s32 i = 0;` declaration-initialiser spelling is a real +1 on build_insns (110) because the value is initialised before the call and has to survive it.

- [s3] The LICM residual is unchanged by the new body: the s3 .loop slice reads 'Loop from 110 to 274: 55 real insns' with 'Insn 137: regno 115 (life 1), move-insn savings 1 moved to 292' and 'Insn 165: regno 130 (life 31), move-insn savings 1 moved to 294'. loop.c:1584-1633 re-read this session confirms no further C-visible input: the guard is !m->done && (!m->cond || invariant_p(...)) && (!m->forces || m->forces->done), and the test is already_moved[regno] || (threshold * savings * m->lifetime) >= insn_count || (m->forces && m->forces->done && n_times_used[m->forces->regno] == 1) with savings = m->savings.

- [s3] -msoft-float does NOT close the s3 body (16 at build 109 under the flag, same as hard float): the flag lowers the loop.c:532 threshold only to about 58 and this body's loop-2 insn_count is 55. This supersedes the s2 reading of FACT 11 - the flag alone is not the fix, it needs a loop-2 spelling with insn_count in [59,122] that keeps the inline-index giv shape.

- [s3] The instrumented cc1 carrying the BB2_QTY_DEBUG / BB2_SUGG_DEBUG local-alloc hooks is tools/gcc-2.7.2/cc1; the engine's configured compiler tools/gcc-2.7.2/build/cc1 (engine/buildconfig.py:19) emits zero QTYDBG lines, so a local-alloc diagnostic run must override CC1 explicitly.

- [s3] src/code6cac_b.c was restored to HEAD at the end of the session (git checkout); no build file is left modified. All s3 diagnostics ran through scratch scripts under tmp/grind/func_80035280/s3/.

- [s4] FLOOR 16 -> 15 on the HEAD chassis (build 109 / target 108). The single change against the s3 body is loop 1's two byte stores, respelled from `((u8 *)p)[0x17 + i] = *src;` to `((u8 *)p + i)[0x17] = *src;` (and the same for 0x1D). The point is the loop-1 address add: the target emits `addu $v1, $t0, $a3` (pointer first), the `0x17 + i` subscript emits `addu $v1, $a3, $t0`. Adding the index to the POINTER in the source and leaving a CONSTANT subscript flips the operand order to the target's.

- [s4] The two obvious neighbours of that fix are inert, measured on the s4 mini-TU harness (29 normalized diffs for the accepted form, 31 for each of these, 31 for the s3 body): `((u8 *)p)[i + 0x17]` (commuting inside the subscript) and `(((u8 *)p) + 0x17)[i]` (constant folded into the pointer, index as the subscript). Banked at rejected/s4-loop1-index-commute-nochange.c. Only "pointer + index, constant subscript" moves the add.

- [s4] PERMUTER CAMPAIGN 1 (tmp/perm_80035280, label s4-split-accum-chassis, 8 jobs, base_score 600): 29,979 iterations, exactly ONE improvement in the whole run - output-590-1, which is the SAME loop-1 transform spelled as `((u8 *)p)[(unsigned long long)(0x17 + i)]` plus inert noise (a `new_var = &D_80106A58` hoist, `int new_var2 = ~4`, `long flags0`). Independent machine confirmation that (a) the loop-1 address add was a real point and (b) nothing else in the s3/s4 basin is reachable by local AST mutation. Harvested and stopped; log at tmp/grind/func_80035280/s4/perm/campaign_split_accum.log, find at tmp/grind/func_80035280/s4/perm/find_590_source.c.

- [s4] PERMUTER CAMPAIGN 2 (tmp/perm_80035280_w, label s4-walker-loop2-chassis, 8 jobs, base_score 960): the structurally different seed required by the chassis rule - the s1 walker loop-2 spelling (three per-field locals, `dst`/`base` pointer walkers, loop-2 RTL insn_count 62, the highest ever measured here) grafted onto the s3/s4 flag-split prologue. 25,284 iterations, 14 finds, best 695 - never within reach of the split-accumulator chassis's 590, let alone of a body that keeps the /1800 magic inside loop 2. The walker basin is a strictly worse basin; the extra loop-2 RTL buys nothing because 62 is still far below the loop.c:1631 requirement of 123. Harvested and stopped; log at tmp/grind/func_80035280/s4/perm/campaign_walker.log.

- [s4] A 4-byte `struct { u8 m, s, c, t; }` record spelling of loop 2 (`q = (Clk *)((u8 *)p + 0x21); q[i].m = ...`) measures 39 normalized diffs against the accepted body's 29: GCC keeps the record pointer AND the `base` walker, so loop 2 gains an induction variable and loses the target's `addiu $a1, $a1, 4`. Banked at rejected/s4-struct-clock-record-score39.c.

- [s4] THE loop.c:1631 GATE, RE-DERIVED FROM SOURCE AND SHARPENED. The condition is `already_moved[regno] || (threshold * savings * m->lifetime) >= insn_count || (m->forces && m->forces->done && n_times_used[m->forces->regno] == 1)`. s1-s3 had established that `threshold` is pinned at 122. s4 read the other two factors out of loop.c and they are pinned too, in the WRONG direction:
    * `savings = m->savings = n_times_used[regno]` (loop.c:793), and `n_times_used` is a straight `bcopy` of `n_times_set` (loop.c:597) - i.e. the number of SETS of the constant pseudo INSIDE the loop, not the number of uses. A single constant load gives exactly 1, and any C change that adds a second set of that pseudo raises savings and makes the hoist MORE eager.
    * `m->lifetime = uid_luid[regno_last_uid[regno]] - uid_luid[regno_first_uid[regno]]` (loop.c:791). The const load sits immediately before its single `mult`, so this is already the minimum 1; spreading the use away from the set only raises it (that is exactly why the /30 magic, lifetime 31-35, hoists so easily).
  So the product is 122 * 1 * 1 = 122 and insn_count is the ONLY C-movable term - insn_count >= 123 (s2 measured the flip between 120 and 123).

- [s4] TWO MORE ESCAPE ROUTES OFF THAT GATE CHECKED AND CLOSED IN SOURCE, so no later session needs to look:
    * `loop_has_call` (loop.c:532, the factor that would halve threshold to 61) is set ONLY by a real `CALL_INSN` in prescan_loop (loop.c:2202); there is no non-emitting construct that sets it, and s1 already measured that a real call adds a `jal` to loop 2.
    * The movable is always CREATED for this pseudo: the skip test at loop.c:695-701 is `! ((! maybe_never && ! loop_reg_used_before_p(...)) || (! REG_USERVAR_P (dest) && ! REG_LOOP_TEST_P (dest)) || reg_in_basic_block_p (p, dest))`, and a compiler-generated constant temp satisfies the second disjunct unconditionally (and the third as well, since it is set and used in one basic block). There is no C spelling that stops the movable being recorded.
  `may_not_move` (loop.c:3038-3044, set when a pseudo is written in two basic blocks of the loop) WOULD block it, but it needs the constant's pseudo set twice under mutually exclusive control flow inside loop 2, and the target's loop 2 is a single straight-line basic block - any such construct changes the emitted bytes.

- [s4] THE EXACT RESIDUAL, INSTRUCTION BY INSTRUCTION (objdump diff of the accepted body against asm/funcs/func_80035280.s, normalized): +2 preheader insns (`lui $t2,0x91a2` / `ori $t2,$t2,0xb3c5`), -2 in-loop insns (the target's `lui $v0,0x91a2` at the loop-2 top and its `ori` in the `lw 0x4($a2)` load-delay slot), +1 `nop` in that now-empty delay slot, `mult $v1,$t2` for the target's `mult $v1,$v0`, and then ten register-name differences (five `mfhi $t3` for `mfhi $t2`, and the five `addu` insns that read the mfhi temp) because our hoisted constant occupies $t2 and displaces the mfhi temp to $t3. All 15 points are one decision; they die together or not at all.

- [s4] TOOLING BUILT THIS SESSION AND WORTH REUSING: tmp/perm_80035280/{base.c,compile.sh,settings.toml,target.o} is a minimal-TU permuter/measurement harness for this function - nine lines of typedefs plus three externs plus the body, compiled through the REAL stage chain (cc1 with the canonical CC_FLAGS including -mel, prologue_fix, maspsx with the full flag set including --prefill-label-funcs, multu_pad) and extracted between `.ent`/`.end`. VALIDATED this session: its object is byte-identical to the full-TU sandbox object for func_80035280 (only relocation targets differ). tmp/grind/func_80035280/s4/score.sh scores an arbitrary variant against the target in about two seconds, versus roughly a minute for a full `sandbox` run, and tmp/grind/func_80035280/s4/install.py swaps a body into src/code6cac_b.c at the INCLUDE_ASM line and restores it with --restore.

- [s4] src/code6cac_b.c was restored to HEAD at the end of the session; no build file is left modified. Both permuter campaigns were harvested with --stop and `permuter_campaign.py status` reports every workspace dead.

- [s4] Floor 16 -> 15 on the HEAD chassis (build_insns 109, target_insns 108). The one change against the s3 body is loop 1's two byte stores: ((u8 *)p + i)[0x17] / ((u8 *)p + i)[0x1D] in place of ((u8 *)p)[0x17 + i] / ((u8 *)p)[0x1D + i]. Saved as memory/grind/func_80035280/candidate.c and re-verified at 15 from that file.

- [s4] The exact residual, instruction by instruction, against asm/funcs/func_80035280.s: +2 preheader insns (lui $t2,0x91a2 / ori $t2,$t2,0xb3c5), -2 in-loop insns (the target's lui $v0 at the loop-2 top and its ori in the lw 0x4($a2) load-delay slot), +1 nop in that now-empty delay slot, mult $v1,$t2 for the target's mult $v1,$v0, and ten register-name differences (five mfhi $t3 for mfhi $t2 plus the five addu insns that read the mfhi temp) because our hoisted constant occupies $t2. All 15 points are downstream of the single loop.c:1631 hoist.

- [s4] Ten of the fifteen remaining points are pure register naming, not instruction differences: if the hoisted 0x91A2B3C5 pseudo could be allocated to something other than $t2, the mfhi temp would land back on $t2 and those ten collapse without touching the LICM decision at all. That axis has never been probed on this function.

- [s4] Permuter campaign 1 (tmp/perm_80035280, s4-split-accum-chassis, 8 jobs, base_score 600): 29,979 iterations, one find (score 590) which is the loop-1 address-add transform spelled with an (unsigned long long) index cast. Harvested with --stop.

- [s4] Permuter campaign 2 (tmp/perm_80035280_w, s4-walker-loop2-chassis, 8 jobs, base_score 960, the structurally different seed the chassis rule requires): 25,284 iterations, 14 finds, best 695. Harvested with --stop. Both workspaces confirmed dead by permuter_campaign.py status; no campaign outlives this session.

- [s4] A 4-byte struct { u8 m, s, c, t; } record spelling of loop 2 measures 39 normalized diffs against the accepted body's 29 - GCC keeps the record pointer AND the base walker, so loop 2 gains an induction variable and loses the target's addiu $a1, $a1, 4. Banked at rejected/s4-struct-clock-record-score39.c.

- [s4] loop.c:1631's move condition is now proven pinned on all three left-hand factors: threshold 122 (loop.c:532, no call in loop 2 and n_non_fixed_regs is compiler configuration), savings 1 (loop.c:793 + 597 - it counts SETS of the pseudo in the loop, not uses), lifetime 1 (loop.c:791 - the const load sits immediately before its single mult). Only insn_count is C-movable and it must reach 123; this body's loop 2 is 55 real insns and the largest natural spelling ever measured here is 62.

- [s4] New reusable tooling: tmp/perm_80035280/{base.c,compile.sh,settings.toml,target.o} is a minimal-TU harness for func_80035280 compiled through the real stage chain (cc1 with canonical CC_FLAGS incl. -mel, prologue_fix, maspsx with the full flag set incl. --prefill-label-funcs, multu_pad), VALIDATED byte-identical to the full-TU sandbox object this session; tmp/grind/func_80035280/s4/score.sh scores a variant in about two seconds versus about a minute for a sandbox run, and tmp/grind/func_80035280/s4/install.py swaps a body into src/code6cac_b.c and restores it with --restore.

- [s4] src/code6cac_b.c was restored to HEAD at the end of the session; git status shows only memory/grind ledger files and metrics/events.jsonl modified.
