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
