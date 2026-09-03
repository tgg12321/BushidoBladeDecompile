/* s74 UPDATE (2026-09-03, synthesis). This body is UNCHANGED and remains the floor at masked 2
 * (re-verified live at the start of s74 together with three side bases: candidate.c 2/179/0,
 * g06 6/179/0, s73's r6 5/179/0, s67's y02 6/179/0 - no drift). What a future session inherits:
 *   1. KILL RE-AUDIT (mandated) DISCHARGED on the closest-to-target instance kill that had never
 *      been ablated - s73's globalized-carrier r6 (score 5). The full 38-row fake_ablate grid on
 *      the current chassis reproduces keep-all at 5/179 and no subset improves it; the sanctioned
 *      tbl_125c wrap costs 8 points to remove even on the globalized body. s73's kill stands.
 *   2. CONFIRMED, and it closes the last unmeasured assumption of the s69-s73 closed form: sched1's
 *      OUTPUT order and sched2's EMISSION order are the same sequence in block 3. The pass-1 PICK
 *      stream reversed is 91 93 99 115 141 117 106 120 122 128 111 137 133 143 145 139 ... which is
 *      the emitted slot order instruction-for-instruction. Spans and quantity numbers really are
 *      pinned by the target's emitted order; they are not merely assumed to be.
 *   3. THE .lreg PSEUDO MAP, read for the first time: `reg/v 98` is the C variable `t0` and the
 *      contested "t0 shift temp" is `reg104`, a compiler intermediate BETWEEN two sets of reg98
 *      (insn 117 reg104 = reg98<<2, insn 122 reg98 = reg104 + reg81). reg98 has two deaths in the
 *      block, so local-alloc skips it and global-alloc seats it at $a0.
 *   4. KILLED (instance): moving that multi-death boundary one link DOWN - address into a fresh
 *      single-death local, shift written back into `t0` - which was predicted to free the arg5
 *      value to $v1 WITHOUT globalizing across basic blocks (and so without spending the
 *      statement-order lever that s73's carriers spent). Ten variants: m1 9, m2 9, m3 8, m6 9,
 *      n0 9, n1 9, n4 9, n2 10, n3 7. It scores 9 on EVERY statement order - order-invariant, and
 *      7 worse than the floor. Controls: m5 (shift in a fresh local) is byte-identical to g06, and
 *      n5 (an unused `s32 taddr;` and nothing else) is byte-identical to the floor, so an
 *      unreferenced scalar local is inert here.
 *   5. THE UNIFYING MECHANISM, from n0's .greg: once the t0 chain leaves local-alloc its allocno
 *      conflict set CONTAINS hard reg 4 (`;; 118 conflicts: ... 4 5 29`) and it is seated at $v0
 *      (`118 in 2`). With s73 that is three routes out of local-alloc - multi-block carrier ($s0 /
 *      $a3), multi-death boundary shift ($v0) - and none of them reaches $a0.
 *   6. NEW FRONTIER RUNG, from the same dump: global.c keeps HARD-REG PREFERENCES and this
 *      function has live ones - `;; 74 preferences: 2` (the `v0` variable, value arrives as a
 *      return value in $v0) and `;; 86 preferences: 5` (copied into printf's $a1). That answers
 *      s73's open question (the call-free `cnt` carrier took $a3 because it had no preference for
 *      4 and 4 was not free) and names an untried lever: give the t0 chain's allocno an $a0
 *      preference. Unprobed in 74 sessions.
 * The ONE open question on this body is still F3 (the volatile prong-2 / allowlist ruling for
 * idx_1496) - documented below and moot while the floor is 2.
 */
/* s73 UPDATE (2026-09-03, structural). This body is UNCHANGED and remains the floor at masked 2
 * (re-verified live at the start of s73: score 2, build 179, target 179, rules_dropped 0; the g06
 * order-perfect base re-verified at 6/179/0 in the same batch). Read hypotheses.md s73 before
 * working here. The four results that matter:
 *   1. The residual on THIS body is EXACTLY the two adjacent instructions at build slots 56/57 -
 *      `sll $a0,$a0,2` then `addu $v0,$v0,$s5` where the target has `addu $v0,$v0,$s5` then
 *      `sll $a0,$a0,2`. Every register in block 3 is already the target's. Nothing else in the
 *      179-instruction function differs.
 *   2. KILLED (class, sched.c:2429): rank_for_schedule's CLASS rung cannot separate the pair -
 *      insn_cost returns the CANDIDATE's result_ready_cost, 1 for any single-cycle ALU insn, so
 *      an `addu` and an `sll` are both class 3 on every call. With s72's INSN_PRIORITY kill the
 *      whole rank_for_schedule ladder above INSN_LUID is now closed: slot order = RTL order.
 *   3. CONFIRMED, a new working mechanism (the s72 frontier's item 1): carrying the t0 chain in a
 *      function-scope variable that is also referenced in ANOTHER basic block removes it from
 *      local-alloc entirely (block_alloc only makes quantities for single-block pseudos,
 *      local-alloc.c:1170-1180). Block 3 then has THREE quantities, the 1.3333-vs-1.3333 tie
 *      disappears, and the arg5 VALUE is seated at $v1 - the target's seat - for the first time in
 *      73 sessions on an order-perfect-derived body with no loop note (r6, dump s73/r6.qty.txt).
 *   4. KILLED (instance): but it does not close. The globalized carrier lands in $s0 (`status`,
 *      call-crossing) or $a3 (`cnt`) and never $a0, and - the decisive control - once the carrier
 *      is multi-block the statement-order lever goes completely inert: the same eight forms score
 *      identically whether generated on THIS body's statement order or on g06's, and all twenty
 *      forms emit slots 56/57 in the floor order. Best 5 against this body's 2.
 * The open frontier is therefore: find a multi-block carrier that global-alloc seats at $a0, or a
 * way to restore the LUID lever on a globalized body. Full battery in rejected/s73-*.
 */
/* s72 UPDATE (2026-09-03, structural). This body is UNCHANGED and remains the floor at masked 2
 * (re-verified live this session: score 2, build 179, target 179, rules_dropped 0). s72 executed
 * both open pass-level items of the s71 frontier and both are now discharged. What a future
 * session inherits:
 *   1. THE RESIDUAL IS NOW ONE NUMBER. The floor body's and the order-perfect g06 base's block-3
 *      quantity tables differ in exactly one respect - the t0-chain quantity's SPAN, 8 here and 6
 *      there. Span 8 gives it qty_compare_1 priority 1.0000, so it is allocated LAST and takes
 *      $a0 (correct); span 6 gives 1.3333, an exact tie with the arg5 value that
 *      local-alloc.c:1683 breaks on quantity number in its favour, so it takes $v1 (wrong). The
 *      target's own instruction order is what shortens it, by moving the t0 shift one slot later.
 *      Both tables are written out in hypotheses.md s72.
 *   2. KILLED: the SUGGESTION PASS route (s71's frontier item 1). It is REAL and reachable - this
 *      session produced the first non-empty copysugg in 72 sessions: naming printf's Nth argument
 *      in a fresh local makes GCC emit `move $aN, pseudo`, combine_regs records copysugg=$aN and
 *      local-alloc.c:1507-1526 seats that quantity before qty_compare_1 runs at all (measured for
 *      arguments 2/3/4 -> $a1/$a2/$a3). It does not reach THIS pair: the only hard registers in
 *      block 3 are printf's $a0-$a3, and the target seats the two contested values at $a0 and $v1
 *      - $a0 carries only a constant address that never occupies a pseudo, and $v1 is not an
 *      argument register. Scores 10 / 16 / 11.
 *   3. KILLED: qty_size (s71's frontier item 2). `long long arg5` builds 181 instructions against
 *      the target's 179. Every qty_compare_1 input is now measured: refs, span, quantity number,
 *      size.
 *   4. KILLED, with the dump: sched2's INSN_PRIORITY rung. All six insns in the contested window
 *      carry final_pri=2. sched.c:1497 adds `insn_cost - 1` per edge, so an ALU-to-ALU edge
 *      contributes zero and only a load-latency edge increments; each chain has exactly one load
 *      above the tied pair. Raising the arg5 chain to priority 3 needs a SECOND load above it,
 *      i.e. loading tbl_125c inside block 3 instead of inheriting it in $s5 - one extra insn.
 *   5. KILLED: single-assignment splitting of the t0 chain (so combine_regs ties it into one long
 *      quantity). It works mechanically - gm1's chain is one quantity, birth 18 death 32 - but
 *      refs scale with the chain exactly as the span does, so priority goes UP (refs 12 / span 14
 *      = 2.5714 against the split form's 1.3333). Chain-combining can never lower a priority here.
 *   6. KILLED (invariance extended, 24 measurements): operand commutation, u32 type narrowing,
 *      declaration-order swap, plain-integer address arithmetic, `&arr[i]` and `[0]` spellings are
 *      all byte-identical on BOTH bases. Commuting the arg5 address expression is the only one
 *      that changes bytes and it changes exactly one - the emitted `addu` operands commute too.
 *   7. Read at local-alloc.c:1176: `if (GET_CODE (insn) != NOTE) insn_number++;`. A loop NOTE does
 *      not advance insn_number and therefore cannot move a span directly - every span change
 *      s67-s71 measured came from sched1 re-ordering, which is why the note lever always paid the
 *      order cost.
 * The ONE open question on this body is still F3 (the volatile prong-2 / allowlist ruling for
 * idx_1496) - documented below and moot while the floor is 2.
 */
/* s71 UPDATE (2026-09-03, rederive). This body is UNCHANGED and remains the floor at masked 2
 * (re-verified live this session: score 2, build 179, target 179, rules_dropped 0). s71 closed the
 * s70 frontier's first two items and opened a pass-level one. What a future session inherits:
 *   1. KILL RE-AUDIT (mandated) DISCHARGED, and it kills the s70 frontier's item 2: stacking an
 *      in-block loop note on the q02 free-depth base is NOT cheaper. The printf-only wrap scores
 *      10 on the g06 base and 10 on q02; the nested version scores 10 on both. Note cost is
 *      independent of the enclosing depth.
 *   2. CONFIRMED, and this is the session's real result: the ASYMMETRIC reg_n_refs s70 named as
 *      the residual's only remaining requirement IS reachable - a note pair around the printf
 *      alone gives the arg5-value quantity refs 5 (or 6 when nested) against the t0-shift
 *      quantity's 4, the first asymmetry measured in 71 sessions. It does NOT flip the seat,
 *      because the same note is a sched1 region boundary and the re-order it causes moves the
 *      t0-shift span from 6 to 4 and the arg5-value span from 6 to 8. qty_compare_1 divides by
 *      that span: pri1 2.0000 vs pri2 1.2500 (single) / 1.5000 (nested). On this block the refs
 *      asymmetry and the span damage are the SAME event.
 *   3. KILLED: carrying the arg5 value in an already-multiply-set variable (`v0 = *(s32 *)(v0 +
 *      (s32)tbl_125c);`) really does delete reg97 from local-alloc - block 3 drops to three
 *      quantities and the tie ceases to exist - but it measures 12 on both bases, because `v0`
 *      already carries the arg5 index and reusing it serialises the block. The t0-side mirrors
 *      through `v0` measure 12 / 13 / 10.
 *   4. CONFIRMED (invariance): with the target's instruction sequence held, six byte-neutral
 *      respellings of the printf-argument block - named address pointer, shift folded into the
 *      address expression, inline `(t0 << 2)`, a fresh index local replacing the v0 staging, a
 *      dead prefix store - all reproduce block 3's quantity table row for row. Respelling the
 *      block does not move any qty_compare_1 input.
 *   5. KILLED: loop notes on THIS body (the floor). Five placements measured; four of them leave
 *      the 179-instruction basin outright (181 insns) and none improves on 2.
 *   6. NEW FRONTIER, read from the compiler: block_alloc has a SUGGESTION PASS
 *      (local-alloc.c:1507-1526) that seats every quantity carrying a hard-register suggestion
 *      BEFORE qty_compare_1 is consulted at all. Suggestions come only from combine_regs
 *      (local-alloc.c:1856-1885), reached for any insn with an `=` output operand 0 and a REG
 *      input operand when one of the two is a hard register. All four block-3 quantities print
 *      copysugg= EMPTY. A C spelling that puts the arg5 value or the t0 shift temp in an insn
 *      with a hard register bypasses the ten-session tie entirely - unprobed.
 * The ONE open question on this body is still F3 (the volatile prong-2 / allowlist ruling for
 * idx_1496) - documented below and moot while the floor is 2.
 */
/* s70 UPDATE (2026-09-03, rederive). This body is UNCHANGED and remains the floor at masked 2
 * (re-verified live this session: score 2, build 179, target 179, rules_dropped 0). s70 spent the
 * rederive rung and enumerated the last unread compiler predicate. What a future session inherits:
 *   1. KILLED (rederive): the ORIGINAL PsyQ libcd source shape is in hand -
 *      tmp/closer/sotn_bios.c:260-286 is CD_ready(int mode, u_char *result) with set_alarm (:95),
 *      get_alarm (:102) and callback (:210) inlined - and transplanted faithfully onto BB2 symbols
 *      it measures 57 at 174 build insns (r1, direct globals) and 55 at 178 (r2, Intr triple via
 *      one base pointer). Both are SHORT of the target's 179: the natural while(1) + short-circuit
 *      `||` control flow is a different basin, not a perturbation of this one. At block level the
 *      natural subscript spellings of the printf arguments bottom out at 7 (r4/r6; r3/r3b/r5 = 14,
 *      r7 = 8) - dropping the `pp` alias sinks the D_800F19C0 load from emitted slot 53/54 to
 *      61/62. Do not re-transplant the source shape.
 *   2. KILLED (class): flow.c has four `reg_n_refs[regno] += loop_depth;` sites, but :2329 and
 *      :2725 are inside `#ifdef AUTO_INC_DEC` and config/mips/mips.h:2175-2179 leaves
 *      HAVE_PRE/POST_INCREMENT commented out. The two live sites (:2081, :2515) are reached only
 *      from propagate_block, which walks PATTERN (insn) (flow.c:1584). REG_NOTES are NEVER walked.
 *      The s69 frontier's "which other RTL positions does flow.c count?" is answered: none.
 *   3. KILLED: the sanctioned F1 combine-foldable copy chain-extender (`a5b = arg5;` and a
 *      two-link version, plus the t0 mirror) is refs-INERT - byte-identical to the g06 base and
 *      block-3 quantity dumps line-for-line identical (refs 4/4/4/8). The copy is copy-propagated
 *      away before flow.c, a different mechanism from s69's self-stores with the same outcome.
 *   4. CONFIRMED, NEW AND POSITIVE: a do-while(0) whose note pair BRACKETS block 3 from outside
 *      (a second wrap opened above the `cnt` test and the `do_timeout:` label, NESTED over the
 *      existing in-block wrap) raises every block-3 qty_n_refs by one loop_depth level -
 *      4/4/4/8 -> 6/6/6/12 - at ZERO byte cost, instruction sequence unchanged. Every in-block
 *      placement s69 measured cost 4-9 points. Base:
 *      progress/s70-q02-note-outside-block3-byte-neutral-refs-lever-6.c. The un-nested form (just
 *      relocating the existing BEG) does NOT raise the depth at all.
 *   5. KILLED (class): that free depth lever cannot break the seat tie by itself. qty1 (reg104,
 *      the t0 shift temp) and qty2 (reg97, the arg5 value) have identical mention counts, spans
 *      and sizes, so a UNIFORM depth change multiplies both qty_n_refs equally and pri1 == pri2 at
 *      every depth; local-alloc.c:1683 then breaks the tie on quantity number, which the target's
 *      own instruction order pins in qty1's favour. Only an ASYMMETRIC refs change moves the seat.
 *   6. KILL RE-AUDIT (mandated) DISCHARGED: s69's k01 self-store kill re-measured across the full
 *      47-variant fake_ablate grid on the current chassis - keep-all and drop-[self-store] are
 *      both 6/179 and byte-identical, every other subset >= 7. No FAKE carrier was masking it.
 * The ONE open question on this body is still F3 (the volatile prong-2 / allowlist ruling for
 * idx_1496) - documented below and moot while the floor is 2.
 */
/* s69 UPDATE (2026-09-03, forensics). This body is UNCHANGED and remains the floor at masked 2
 * (re-verified live this session: score 2, build 179, target 179, rules_dropped 0). s69 read the
 * residual out of BOTH GCC passes with the instrumented cc1 and reduced it to closed form:
 *   1. CONFIRMED: GCC 2.7.2 schedules each block BACKWARD - the SCHEDDBG PICK stream is the
 *      REVERSE of the emitted order. Read any .sched dump on this project with that inversion.
 *   2. CONFIRMED: this body's two-instruction residual is exactly one comparison,
 *      `RANKDBG last=122 y=120 cls=3 x=106 cls2=3 val=0`; the class rung ties 3/3 and sched.c
 *      falls through to INSN_LUID. insn 106 = `sll $a0,$a0,2` (the t0 shift), insn 120 =
 *      `addu $v0,$v0,$s5` (the arg5 address add).
 *   3. CONFIRMED (the ordering half is SOLVED on this chassis): moving `t0 *= 4;` and
 *      `t0 = (s32)((u8 *)tbl_125c + t0);` to AFTER the arg5 load raises 106's LUID above 120's
 *      and emits the TARGET'S EXACT instruction sequence for the whole block. See
 *      progress/s69-g06-order-perfect-on-candidate-chassis-seats-swapped-6.c (score 6, 179, 0).
 *      Its residual is purely register naming.
 *   4. CONFIRMED (and this is why 3. does not close it): fixing the order shortens the t0-shift
 *      quantity from span 8 to span 6, raising its qty_compare_1 priority from 1.0000 to 1.3333 -
 *      an EXACT tie with the arg5-value quantity - and local-alloc.c:1683 breaks that tie on
 *      quantity number, which the target's own order pins in the t0 shift's favour. Order and
 *      seats are coupled BY CONSTRUCTION, which is the anti-correlation s61-s67 kept measuring.
 *   5. KILLED: loop notes cannot buy the one reference count that would break the tie on the
 *      order-perfect base (h01/h02/h03 = 12/12/15, j01-j04 = 10/12/10/10; the decisive control
 *      h04 - same split boundary, nothing bare - is also 12, so the whole loss is the note).
 *   6. KILLED: same-value re-stores (`arg5 = arg5;`) are REFS-inert, not just byte-inert -
 *      flow.c's delete_noop_moves runs before reg_n_refs is accumulated (k01 dump == g06 dump).
 * The ONE open question on this body is still F3 (the volatile prong-2 / allowlist ruling for
 * idx_1496) - documented below and moot while the floor is 2.
 */
/* s68 UPDATE (2026-09-01, escalation). This body is UNCHANGED and remains the floor at masked 2
 * (re-verified live this session: score 2, build 179, target 179, rules_dropped 0).
 * s68 killed BOTH of the s67 frontier axes with dump-level mechanism and discharged the owner's
 * 2026-09-01 Ruling A row for this function in full, so the item was FORECLOSED under the standing
 * ruling (docs/grind/decisions.md, 2026-09-01 CD_ready entry). What a future session must know:
 *   1. KILLED: `a1v` cannot be moved out of local-alloc by any C respelling. Declaring it at
 *      FUNCTION scope produces a local-alloc dump line-for-line identical to d01's (same
 *      quantities, refs, `used` sets, seats; only pseudo numbers shift) - block_alloc decides
 *      membership from REG_BASIC_BLOCK, not C scope. Hard reg 4 is in reg98's `used` because $a0
 *      is the printf call's first argument, NOT because of a1v (which takes $a1). The s67 frontier
 *      item's premise was false.
 *   2. KILLED: folding the arg5 address chain is byte-identical to d01 (7/179) and does not move
 *      the 51/52 lbu pair. Source-level folding cannot shorten an RTL dependence chain; removing a
 *      link costs an instruction (fully-inline = 180 insns), off the 179-parity basin.
 *   3. KILLED: the Ruling D CD_intr aggregate. Prong (c) is structurally unsatisfiable (five
 *      asm-only consumers of D_800A1494/95/96: getintr, CD_cw, func_800817A0, func_800819C4,
 *      func_80081E1C, plus the asm data definition), and the aggregate measures 41/173 (volatile)
 *      and 37/172 (plain) against the floor of 2/179.
 * The ONE open question on this body is still F3 (the volatile prong-2 / allowlist ruling for
 * idx_1496) - documented below and moot while the floor is 2.
 */
/* s67 UPDATE (2026-09-01, solver). This body is UNCHANGED and remains the floor at masked 2
 * (re-verified live this session: score 2, build 179, target 179, rules_dropped 0). It holds the
 * SEATS with a single adjacent ALU transposition left.
 *
 * s67 did NOT improve the floor, but it moved the frontier off this body onto a different base.
 * Read memory/grind/CD_ready/hypotheses.md s67 before working here. The three results that matter:
 *   1. KILLED: an EMPTY do{}while(0) is byte-neutral NOWHERE in the do_timeout block (six
 *      positions, 9/7/7/10/11/10 against the base's 6). The loop note is a sched1 region boundary
 *      here, so a reference count can never be bought for free on this function.
 *   2. CONFIRMED, new axis: SPLITTING the existing tbl_125c wrap so `t0 *= 4` falls into a bare
 *      loop-depth-1 gap LOWERS the t0 shift temp's reg_n_refs and flips the seats
 *      (memory/grind/CD_ready/progress/s67-y02-seats-correct-base.c, score 6).
 *   3. CONFIRMED by pre-registered prediction: on the order-perfect d01 base, raising the arg5
 *      VALUE quantity's refs to exactly 6 (two extra nesting levels on the arg5 load) gives it
 *      $v1, the target's seat (progress/s67-e02-arg5-value-v1.c, score 8).
 * The new frontier base is progress/s67-d01-order-perfect-base.c (score 7, 179 insns) whose
 * instruction SEQUENCE is the target's from insn 53 to the end; its residual is purely seats.
 */
/* CD_ready candidate - s60 (2026-09-01). HONEST FLOOR: masked 2, build 179 == target 179,
 * rules_dropped 0. This SUPERSEDES the 59-session vT40 body (masked 4, build 178), which is
 * retained beside this file as candidate-vT40-masked4-no-volatile.c.
 *
 * PROVENANCE. This is the Closer-phase form `marionation_vAT1_notailwrap.c`, recovered with
 *   git show 043e4b80^:memory/closer/candidates/marionation_vAT1_notailwrap.c
 * It was banked 2026-07-09 at masked 2 and then set aside on a premise the owner corrected in
 * the 2026-09-01 FORECLOSED-BUCKET REVIEW (decisions.md:17795, Ruling A row `CD_ready (d4)`):
 * "re-score the banked vAT1 form post-`-mel`". Executed this session; it re-scores at 2 on the
 * current post-`-mel`, post-naming-wave chassis. Floor 4 -> 2.
 *
 * NAMING MAP (this body predates the naming wave). To splice into the current src/system.c:
 *   marionation_Exec->CD_ready, sys_VSync->VSync, tslTm2LoadImage_2->puts, debug_printf->printf,
 *   cdrom_ClearIrq->CD_flush, sys_GetVblankCount->CheckCallback, func_80080828->getintr.
 * Harness: tmp/grind/CD_ready/s60/splice.py (marker: INCLUDE_ASM("asm/funcs", CD_ready); at
 * src/system.c:379). All needed externs already exist at src/system.c:353-375.
 *
 * THE ONE CONSTRUCT THAT DISTINGUISHES THIS FROM vT40 - and the open family question.
 * `volatile u8 *idx_1496;` (the pointer to the IRQ-set CD status byte 0x800A1496 = g_cd_status_c).
 * It is LOAD-BEARING and MEASURED so: strip the qualifier and nothing else (s60 v01) and the score
 * goes 2 -> 4 with build_insns 179 -> 178, i.e. GCC hoists the flag load out of the `goto loop`
 * polling loop and the target's 179th instruction disappears. The target therefore contains an
 * access GCC only emits when the object is volatile.
 *   PRONG 1 of .claude/rules/legitimate-volatile-interrupt-touched.md is SATISFIED WITH CITATIONS:
 *     - IRQ writer: `getintr` writes the byte - asm/funcs/getintr.s:304 `sb $v0, %lo(D_800A1496)($at)`
 *       (also :244 for 1494, :274/:309 for 1495); func_800819C4 likewise at asm/funcs/func_800819C4.s:71.
 *     - Installed handler: `getintr()` is called from `cdrom_IrqHandler` (src/system.c:770), whose
 *       entry carries glabel D_80081F1C (src/system.c:758-768) and which is installed as the IRQ-2
 *       (CD-ROM) callback by `InterruptCallback(2, &D_80081F1C);` at src/system.c:609 and :630.
 *   IN-TU PRECEDENT (matched, committed, byte-correct code declares these very bytes volatile):
 *     - src/system.c:549-551 `extern volatile u8 g_cd_status_a; ... _b; ... _c;`
 *     - src/system.c:748-749 the same, inside cdrom_IrqHandler's own declaration block
 *     - src/system.c:770-771 `volatile u8 *s1 = &g_cd_status_b; volatile u8 *s3 = s1 - 1;`
 *       i.e. a matched function derives a volatile byte pointer BY POINTER ARITHMETIC, the same
 *       shape used here (`idx_1496 = idx_1494 + 2`).
 *   PRONG 2 (use-site shape) IS THE OPEN QUESTION and must be settled before any candidate-ready:
 *   the rule's exact list is spin-wait / double-read-across-sequence-point / IRQ-mutated-loop-bound.
 *   This use-site is the `loop:` ... `tail: if (a0 == 0) goto loop;` poll whose body calls VSync()
 *   and getintr() between successive reads of `*idx_1496` - closest to
 *   double-read-across-sequence-point, but it is not verbatim any of the three. Also note
 *   D_800A1494/95/96 carry NO entry in volatile_extern_allowlist.txt, so a candidate needs either
 *   that grant or a ruling that the already-shipped in-TU declarations cover it. DO NOT submit
 *   candidate-ready on this body without resolving that; the honest move is a ruling-request.
 *
 * MEASURED-DEAD RESPELLINGS OF THE SAME VOLATILE (all s60, banked in rejected/):
 *   - type-level on the global, pointer taken directly (`extern volatile u8 g_cd_status_c;
 *     idx_1496 = &g_cd_status_c;`): score 4, build 180 - the separate lui/%lo materialisation
 *     destroys the single-base addressing (identical finding to CD_sync s107, decisions.md:18333).
 *   - all three Intr pointers volatile (idx_1494/1495/1496): score 8, build 179.
 *   - same, with the base taken as `&g_cd_status_a` in cdrom_IrqHandler's exact shape: score 8.
 *   Only the 1496 access path may be volatile; 1494/1495 must stay plain.
 *
 * REMAINING RESIDUAL (masked 2, 179/179) - CORRECTED AND FULLY LOCALISED IN s61 (2026-09-01).
 * The s60 header guessed a register mismatch here; the s61 disassembly (tmp/grind/CD_ready/s61/
 * show.py) proves otherwise. EVERY register in all 179 instructions already matches the target,
 * both seats included. The entire residual is a transposition of two independent ALU insns in the
 * do_timeout printf-argument block:
 *     idx  BUILD              TARGET
 *      55  sll  $v0,$v0,2     sll  $v0,$v0,2
 *      56  sll  $a0,$a0,2     addu $v0,$v0,$s5
 *      57  addu $v0,$v0,$s5   sll  $a0,$a0,2
 *      58  lw   $v1,0($v0)    lw   $v1,0($v0)
 * Attribution READ from tmp/grind/CD_ready/dumps/system.sched2: sched2 insns 106 (sll a0) and
 * 120 (addu v0) tie on INSN_PRIORITY and rank_for_schedule falls through to INSN_LUID, so C
 * statement order decides. Moving `t0 *= 4` after the arg5 chain DOES produce the target order
 * (s61 w04) but inverts the local-alloc seat via qty_compare_1 - order and seat are each
 * independently reachable and anti-correlated through that one lever. The arithmetic of the
 * inequality that would satisfy both is written out at the end of hypotheses.md (s61).
 * S65 CORRECTION TO THE PARAGRAPH ABOVE (read this before spending a session on the seats).
 * The s61-s63 model of the seat half (reg 98's ".lreg used 8 times across N insns" window) is a
 * PROXY and it is wrong: reg 98 is not one of local-alloc's block-3 quantities at all. The
 * instrumented cc1 (`bash tmp/grind/CD_ready/s63/qty.sh <out>`, BB2_QTY_DEBUG/BB2_SUGG_DEBUG)
 * prints the real thing - every quantity's qty_compare_1 inputs, its rank, and the hard register
 * find_free_reg gave it. Measured on THIS body: 104 arg5-ADDRESS 18-20 refs4 pri 4.00 -> $v0;
 * 110 22-30 refs8 3.00 -> $v0; 97 arg5-VALUE 20-26 refs4 1.33 -> $v1; 102 t0 16-24 refs4 1.00 ->
 * $a0 (all four seats as the target wants them). On the order-perfect base the arg5 address
 * quantity stretches to span 2 (pri 2.00) because sched1 fills the addu->load latency slot with
 * the t0 shift, and t0 then beats the arg5 value on a 1.33-vs-1.33 tie broken by quantity number.
 * s66 CORRECTION - do not spend the s65 frontier probe. s65 named the closing change as
 * "exchange sched1 insns 145 and 137"; reading k03.sched.txt against the QTYDBG columns shows
 * insn 145 is the death of NO local quantity (reg 98 = the `t0` variable is set twice in the
 * block, so REG_N_DEATHS == 2 and local_alloc skips it; global-alloc seats it, correctly, at
 * $a0). The tie is between reg 104 (the t0 SHIFT temp, insn 117 -> insn 122) and reg 97 (the
 * arg5 VALUE, insn 113 -> insn 137). With the target's instruction sequence held, both pseudos'
 * births, deaths and quantity numbers are forced and both carry the minimum two mentions, so
 * qty_n_refs - the loop-depth-weighted reg_n_refs - is the residual's ONLY free variable. s66
 * demonstrated that a do-while(0) loop note really does move it (arg5 value 4 -> 5 or 6), but
 * every placement tried also perturbs sched1 (best 8). See evidence.md/hypotheses.md s66.
 * Every t0-web / arg5 axis killed in s53-s59 was killed against the vT40 (masked-4) base; s61
 * re-ran the natural-C, source-position, decl-order, pseudo-split and variable-reuse families
 * against THIS base (all banked in evidence.md s61 and rejected/s61-*).
 */
s32 marionation_Exec(s32 a0, u8 *a1)
{
  s32 v0;
  s32 cnt;
  u8 *idx_1494;
  u8 *idx_1495;
  volatile u8 *idx_1496;
  int new_var;
  int new_var3;
  s32 *tbl_125c;
  u8 saved;
  s32 status;
  u8 *src;
  u8 *dst;
  u8 *dst2;
  s32 i;
  D_800F19B8 = sys_VSync(-1) + 0x3C0;
  tbl_125c = D_800A125C;
  idx_1494 = (u8 *)&D_800A1494;
  idx_1495 = 1 + idx_1494;
  idx_1496 = idx_1494 + 2;
  D_800F19BC = 0;
  D_800F19C0 = &D_80016248;
  loop:
  v0 = sys_VSync(-1);

  if (D_800F19B8 < v0)
  {
    goto do_timeout;
  }
  cnt = D_800F19BC;
  D_800F19BC = cnt + 1;
  if (!(0x3C0000 < cnt))
  {
    goto success;
  }
  do_timeout:
  do { /* FAKE: do-while(0) loop-note ref weighting seats tbl_125c in s5 (SOTN FAKE-class match device; do-while-zero-exception 2026-07-06) */
  tslTm2LoadImage_2(&D_800161B8);

  {
    s32 arg5;
    s32 t0;
    s32 a5a;
    s32 a2i;
    void **pp;
    t0 = idx_1494[0];
    pp = (void **)&D_800F19C0; /* FAKE: pointer-alias staging (staged-value-reused-variable / pointer-alias, owner-sanctioned 2026-07-03) */
    v0 = idx_1494[1]; /* FAKE: index staged through the (dead-here) v0 var per staged-value-reused-variable (owner-sanctioned 2026-07-03) */
    v0 <<= 2; /* FAKE: continued staging per staged-value-reused-variable */
    a5a = v0 + (s32)tbl_125c;
    t0 *= 4;
    arg5 = *(s32 *)a5a;
    a2i = D_800A11D5;
    t0 = (s32)((u8 *)tbl_125c + t0);
    do { /* FAKE: nested do-while(0) loop note raising ONLY the arg5 value pseudo reg_n_refs 4->5 (flow.c loop_depth weighting -> local-alloc.c qty_compare_1 tie break); single level insufficient - the outer wrap is load-bearing for tbl_125c in $s5 (8 points by fake_ablate). do-while-zero-exception 2026-07-06 */
    debug_printf(&D_800161C8, *pp, D_800A11DC[a2i], *(s32 *)t0, arg5);
    } while (0);

  }
  cdrom_ClearIrq();
  } while (0);
  v0 = -1;
  goto check;
  success:
  v0 = 0;

  check:
  if (v0 != 0)
  {
    return -1;
  }

  new_var = 0xFF;  /* FAKE: opaque mask variables (with new_var3) keep the target's redundant `andi ,0xff` alive (named-local constant-holder family). Alternatives exhausted and recorded in memory/wip/marionation_Exec/notes.md: u8-typed checks fold via PROMOTE_MODE+combine (measured 17), staged raw byte folds (proven byte); the symbolic mask is the one spelling combine cannot fold */
  new_var3 = 0xFF;
  do { /* FAKE: do-while(0) loop-note ref weighting seats idx_1494/idx_1495 in s2/s6 */
  if (sys_GetVblankCount() != 0)
  {
    saved = *D_800A147C_2 & 3;
    do
    {
    status = func_80080828();

    if (status == 0) break;
    {
      if (status & 4)
      {
        if (D_800A11B8 != 0)
        {
          ((void (*)(u8, void *)) D_800A11B8)(*idx_1495, &D_800F19A8);
        }
        ;
      }
      if (status & 2)
      {
        if (D_800A11B4)
        {
          ((void (*)(u8, void *)) D_800A11B4)(*idx_1494, &D_800F19A0);
        }
      }
    }
    }
    while (1);
    *D_800A147C_2 = saved;
  }
  } while (0);
  {
    s32 check;
    check = *idx_1496 & new_var;
    if (!check) goto check2;
    do { do { *idx_1496 = 0; } while (0); } while (0); /* FAKE: NESTED do-while(0) - double loop-note weighting lifts idx_1496's allocno priority to 1600, above arg1's 952. Single-level MEASURED insufficient 2026-07-06: i1496 pri 933 < arg1 952, i1496 falls s3->s4 (probe ledger, masked 4->14). Justification per do-while-zero-exception prerequisite 3 */
    src = (u8 *) (&D_800F19B0);
    dst = a1;
    if (a1 != 0)
    {
      i = 7;
      do
      {
        u8 bb;
        bb = *src;
        src++;
        i--;
        *dst = bb;
        dst++;
      }
      while (i != (-1));
    }
    return check;
    check2:
    check = *(idx_1496 - 1) & new_var3;
    if (!check) goto tail;
    do { *(idx_1496 - 1) = 0; } while (0); /* FAKE: do-while(0) loop-note weighting balances the check2 clear against check1's nested wrap */
    dst2 = a1;
    src = (u8 *) (&D_800F19A8);
    i = 7;
    if (dst2 != 0)
    {
      do
      {
        u8 bb;
        bb = *src;
        src++;
        i--;
        *dst2 = bb;
        dst2++;
      }
      while (i != (-1));
    }
    return check;
    tail:
    if (a0 == 0)
    {
      goto loop;
    }
    return 0;
  }
}
