/* s87 UPDATE (2026-09-06, solver). BODY UNCHANGED (2/179/0, re-measured live; residual = slots
 * 55-57, chain A's shift 115 emitted before chain B's 126/128 in both passes by INSN_LUID).
 * Both owner-directive probes executed: (1) local_extract/local_alloc on a4 - NO re-pricing
 * reaches the seat exchange, hard reg 4 is in every block-3 used set because sched1 emits
 * `la a0` at position 8; (2) the honest-4 form's residual is ONE seat (t0 in $7 vs $4) on a
 * target-exact order, produced by a do-while(0) LOOP-NOTE BARRIER (sched.c:2081) at the a2i
 * shift. NEW BASE vB (progress/s87-vB-...-4.c): h4 with a1 passed as `*pp` directly - every
 * seat target-exact, order target-exact except the a1 load after the barrier. Measured dead
 * this session: vA/vC (a1 direct inside the wrap: precomputed copy beats la a0), F4/F5 (fresh
 * address pseudo boosts 120 but makes t0 single-set -> 111 boosted), P5a/P5b (t0 shared with
 * the callback byte: combine merges the callback load+copy, t0 single-set anyway). Exact
 * requirement for a match, from the floor's sched1/sched2 traces + qty table: pass-1 emission
 * 128 < 115 < 130 AND (Q_A span >= 8 | Q_V span <= 4 | Q_V refs +2). See evidence.md s87.
 */
/* s84 UPDATE (2026-09-04, synthesis). BODY UNCHANGED (2/179/0, re-measured live at the start
 * and again at the end of the session). What changed is the DIAGNOSIS, and it points away from
 * this body.
 *
 * 1. s83's floor-body headline is wrong. It scored scheduler atoms by "puts 143 before 120";
 *    the acceptance test is the EXACT pass-2 stream (the baseline with ONLY 143 and 120
 *    transposed). Under that test, 0 single atoms reach the target - over all 20 nodes x
 *    pri 1..9, ref 0..19, luid 0..31, all 190 pairwise luid swaps, all edge removals and all
 *    edge additions - and 0 of 729,150 two-atom combinations reach it either. s83's four named
 *    atoms each fix the one adjacency while displacing five or more other insns.
 *
 * 2. Two model corrections were needed to get that answer. (a) s83's edge-removal sweeps
 *    deleted an edge WITHOUT decrementing the predecessor's INSN_REF_COUNT, so those streams
 *    were not legal schedules - every "no dependence edit reaches the target" row from s83 is
 *    void; use tmp/grind/CD_ready/s84b/enum2.py. (b) `ref` in the solver json is INSN_REF_COUNT
 *    (successor count), NOT reg_n_refs - so s83's "ref(147)=3 is the do-while(0) refs lever"
 *    identification was a mis-attribution.
 *
 * 3. The priority arithmetic is now pinned: pri(x) = max over preds p of
 *    (pri(p) + insn_cost(p,kind,x) - 1), insn_cost = icost(p) on true edges, 1 on anti edges.
 *    Hence pri(143) = pri(138) + 1 >= 2 always (s83's pri(143)=1 atom is unreachable), and the
 *    pair is lost TWICE for two different reasons: in pass 1 pri(143) == pri(120) and the luid
 *    tie-break favours chain A; in pass 2 pri(138) is raised to 2 by the post-reload anti-deps
 *    138<-128 and 138<-130 ($2 reuse by the arg5 chain), so pri(143)=3 > pri(120)=2 and the
 *    tie-break is never consulted. A winning form must beat both at once.
 *
 * 4. AND THE RESIDUAL IS PRODUCIBLE IN C. Every form that gives the D_800A11DC element its own
 *    named intermediate, and every form that inlines chain A into the call, flips 143 ahead of
 *    120 in BOTH passes: a1 16, a3 14, a4 14, a7 14, a8/a9/a10 14 - all 179 insns / 0 rules.
 *    Non-flipping controls: a2 (names the index, not the element) 9, a6 (chain A moved after
 *    the arg5 chain) 7. The transposition is cheap; the +12 collateral is REGISTER ALLOCATION.
 *    Function-scope carriers are strictly worse than a fresh block-local (status 22, cnt 26,
 *    i 44) and are dead.
 *
 * 5. THE FRONTIER THEREFORE LEAVES THIS BODY. This body's scheduler neighbourhood is
 *    enumerated and empty; a4 (14/179/0, transposition already fixed) is the base with the
 *    live door, and its residual is an ra_solver question, not a sched_solver one. See
 *    state.json frontier[0].
 */
/* s82 UPDATE (2026-09-04, structural). BODY UNCHANGED (2/179/0, re-measured live). What
 * changed is that the s74/s81 blocking premise is GONE and the single-atom target is exact.
 *
 * 1. SCHED1 AND SCHED2 DO DIVERGE IN BLOCK 3 on the order-exact base (6/179/0):
 *      sched1 emission  117 120 126 122 139 131 148 144 154
 *      sched2 emission  117 120 126 122 139 131 144 148 154   <- the target
 *    s74 measured "sched1 == sched2 in block 3" on the FLOOR body; it is false here. Every
 *    live_extend / live_shrink atom the s81 solver returned was gated behind that premise.
 *
 * 2. THE CAUSE IS birthing_insn_p (sched.c:2505, driven by adjust_priority sched.c:2543,
 *    switched off in sched2 by reload_completed at sched.c:2510). In sched1 the insns
 *    120/122/126/139/144/154/156 are boosted to INSN_PRIORITY 0x7F000001; 131 and 148 are not.
 *      - insn 148 is a STORE, so sched.c:2513's `SET_DEST == REG` can never hold -> never
 *        birthing. pri 3 = pri(122) + LOAD latency 2 - 1.
 *      - insn 131 sets reg/v 104 (t0), which the chain-A byte load insn 111 also sets, so
 *        reg_n_sets == 2 fails sched.c:2526. pri 2 = pri(126) + ALU latency 1 - 1.
 *    The +1 that makes 148 outrank 131 in sched1 is exactly "chain B ends in a load, chain A
 *    ends in a shift".
 *
 * 3. THE ATOM, DERIVED OFFLINE ON THE VALIDATED sched_solver MODEL (60/60 blocks order- AND
 *    clock-exact in BOTH passes). birth(131)=1, pri(131)>=4 and pri(148)<=1 all give sched1
 *      117 120 122 148 126 139 131 144 154
 *    -> reg103 (arg5 value) born 18 span 2 pri 4.0, reg109 (chain A shift) born 22 span 4
 *    pri 2.0: the arg5 value becomes qty1 and allocates FIRST. That is the seat order the
 *    target needs. birth(144)=0 is a wrong turn (different, worse order).
 *
 * 4. AND THE PASS-2 ORDER IS INVARIANT TO THAT RELABEL (whatif2.py): pass-2 LUIDs are the
 *    pass-1 emission positions, and relabelling to the perturbed order still reproduces the
 *    target emission. So a form that perturbs ONLY sched1 keeps the byte order.
 *
 * 5. WHAT IS DEAD: seven spellings that realize birth(131)=1 by making reg/v 104 single-set
 *    (fresh s32 address local 9, three fresh locals 9, fresh s32* address 9, chain-A byte
 *    staged through the multi-set `status` 8 / `cnt` 8, and both `(s32)tbl + (x<<2)`
 *    respellings 8). All seven DO produce the predicted sched1 order - the mechanism fires -
 *    but each adds a pseudo, the allocation changes, sched2 moves with it, and the seats do
 *    not flip. rejected/s82-chainA-*.c.
 *
 * 6. THE FRONTIER IS THEREFORE: a spelling that raises pri(131) to >= 4 or lowers pri(148)
 *    to <= 1 WITHOUT adding a pseudo (these are priority-graph atoms, not liveness atoms, so
 *    they do not touch reg_n_sets); or a spelling that sets birth(131)=1 while keeping the
 *    pseudo count and the post-reload dep graph of the order-exact base.
 */
/* s81 UPDATE (2026-09-04, rederive). BODY UNCHANGED (still 2/179/0, re-measured live this
 * session); what changed is that the residual is now IDENTIFIED DOWN TO THE RTL INSN, and the
 * s80 frontier's arithmetic target has been corrected.
 *
 * 1. THE RESIDUAL, READ OFF THE ORDER-EXACT BASE (progress/s80-g7-...-6.c), IS EXACTLY SIX
 *    INSTRUCTIONS AND ONE TWO-WAY SEAT SWAP.  objdump slots 44-71, ours | target:
 *        51  lbu v1,0(s2)   | lbu a0,0(s2)          <- CD_intstr index byte  (chain A)
 *        57  sll v1,v1,2    | sll a0,a0,2           <- chain A shift
 *        58  lw  a0,0(v0)   | lw  v1,0(v0)          <- arg5 VALUE load       (chain B)
 *        61  addu v1,v1,s5  | addu a0,a0,s5         <- chain A address
 *        63  sw  a0,16(sp)  | sw  v1,16(sp)         <- arg5 stack store
 *        67  lw  a3,0(v1)   | lw  a3,0(a0)          <- chain A deref
 *    Slot 54 (`lw a1,8(a1)` vs `lw a1,0(a1)`) is NOT a residual: it is the CD_alarm struct
 *    model's %hi/%lo addend on D_800F19B8+8 against the target's own %hi/%lo on D_800F19C0,
 *    the same address after relocation, and the scorer counts it as equal.
 *
 * 2. THE TWO CONTESTED QUANTITIES ARE NAMED RTL INSNS NOW (post-sched1 dump
 *    tmp/grind/CD_ready/dumps/system.sched, taken on the order-exact base):
 *        qty1 = reg 109 = (set (reg 109) (ashift (reg/v 104) 2))  = insn 126, chain A's SHIFT
 *        qty2 = reg 103 = (set (reg/v 103) (mem (reg 107)))       = insn 122, the arg5 VALUE
 *      sched1 order is 120(reg107 = arg5 addr), 126(reg109), 122(reg103), 139, 131(chain A
 *      addr), 148(the sw), 144, 154 - and local-alloc's insn_number steps by 2, which maps
 *      exactly onto the measured births 16/18/20/22.  So s80's label "qty1 = the t0 chain" is
 *      right but imprecise: qty1 is only the SHIFT RESULT, and qty2 is only the LOADED VALUE.
 *      Each has exactly ONE set and ONE use.
 *
 * 3. THEREFORE THE s80 FRONTIER TARGET ("arg5 refs >= 5") WAS OFF BY THE PARITY OF THE WRAP.
 *    reg_n_refs = (number of references) x loop_depth, and the do_timeout block sits at
 *    loop_depth 2 inside the outer FAKE do-while(0), so refs are QUANTIZED TO EVEN VALUES:
 *    refs 5 is not a reachable state at all.  Proven by ablation - deleting the outer wrap
 *    halves every refs field exactly (4/4/4/8 -> 2/2/2/4, dump tmp/grind/CD_ready/s81/d0.qty.txt)
 *    and leaves births, deaths and the seats identical.  The real requirement, identical at
 *    either depth, is ONE ADDITIONAL SURVIVING REFERENCE TO reg103 (refs 4 -> 6 at depth 2,
 *    2 -> 3 at depth 1; either gives pri 2.0 or 0.5 against qty1's unchanged 1.3333/0.3333).
 *
 * 4. THE ARG5-SIDE C SPELLING IS BYTE-INERT ON THE ORDER-EXACT BASE, AND THE T0-SIDE IS NOT.
 *    24 forms this session (tmp/grind/CD_ready/s81/{a,b,c}*.c, logs a.log/c.log): every
 *    respelling of the arg5 chain - fresh split locals, pointer-typed intermediate, array
 *    subscript off tbl_125c, a relay local, a shared table pointer, staging through the
 *    function-scope status local - scores exactly 6, and three qty dumps confirm qty2 stays
 *    bit-identical at birth 20 / death 26 / refs 4.  Every t0-side respelling regresses
 *    (split-into-three 9, pointer-typed 9, array subscript 11, relay 11, staged through v0 11,
 *    both indices through function-scope locals 17).  Note the array-subscript spelling of
 *    arg5 costs 9+ on the FLOOR body (s80 f2 = 11) but is FREE here.
 *
 * 5. THE LOCAL-ALLOC INVERSE SOLVER SAYS REACHABLE, WITH 23 SINGLE-ATOM VECTORS
 *    (`python3 tools/ra_solver/inverse.py local tmp/ra_solver_work/system.local.json
 *      --func CD_ready --block 3 --swap 1,2 --depth 3`): refs_up on qty2 (4->5/6/7),
 *    refs_down on qty1 (4->3/2), live_extend on qty1, live_shrink on qty2, and six
 *    alloc_order atoms.  Of these, refs_down on qty1 is unreachable (a one-reference pseudo
 *    is dead), the odd-refs atoms are unreachable by the parity fact in (3), and EVERY
 *    live_extend/live_shrink atom moves an insn that is already at its target slot - so it is
 *    realizable only if the sched1 alloc-time order can be made to differ from the final
 *    sched2 emission order.  That leaves exactly two live openings, and they are the frontier.
 */
/* s80 UPDATE (2026-09-04, rederive). FLOOR UNCHANGED AT 2/179/0, but the BODY CHANGED: the
 * arg5 index chain is now spelled in TWO statements instead of three
 * (`v0 = idx_1494[1];` then `arg5 = *(s32 *)((v0 << 2) + (s32)tbl_125c);`), which retires the
 * separate `v0 <<= 2;` staging statement and one FAKE annotation unit at ZERO byte cost -
 * measured 2/179/0 with a residual byte-identical to the s78/s79 floor (the single adjacent
 * transposition at build slots 56/57; adiff2 prints exactly `ours[55] sll a0,a0,2` vs
 * `tgt[57] sll a0,a0,2`). FAKE unit count on the floor body: 7 -> 6.
 *
 * THE ONE STRUCTURAL RESULT OF s80 - READ THIS BEFORE PROBING.  A body whose instruction
 * ORDER is 100% target-exact, with the printf's a1 argument load STILL AT ITS TARGET SLOT and
 * with NO extra pseudo anywhere, exists and is reached by ORDINARY C statement reordering:
 * move the two t0-chain statements (`t0 *= 4;` and `t0 = (s32)((u8 *)tbl_125c + t0);`) to
 * AFTER the whole arg5 chain.  Banked as
 * progress/s80-g7-order-100pct-target-exact-a1-in-place-pure-seat-swap-6.c (score 6, 179).
 * Its residual is a PURE TWO-WAY SEAT SWAP and nothing else: every emitted opcode is the
 * target's, the t0 chain sits in $v1 where the target has $a0 and the arg5 value sits in $a0
 * where the target has $v1.  This is strictly better conditioned than the s79 y2/z_d0 bases
 * (which paid an a1 displacement or an extra a5a pseudo for the same order).
 *
 * THE SEAT SWAP IS ONE INTEGER AWAY, AND s80 MEASURED THE INTEGER.  Instrumented-cc1 dump
 * tmp/grind/CD_ready/s80/h_0_9_d1.qty.txt, block 3:
 *     qty=0 reg107 birth 16 death 20 span 4  refs 4  -> pri 2.0000 -> ord 1 -> $v0
 *     qty=1 reg109 birth 18 death 24 span 6  refs 4  -> pri 1.3333 -> ord 2 -> $v1  (t0 chain)
 *     qty=2 reg102 birth 20 death 26 span 6  refs 4  -> pri 1.3333 -> ord 3 -> $a0  (arg5 value)
 *     qty=3 reg116 birth 22 death 30 span 8  refs 8  -> pri 3.0000 -> ord 0 -> $v0
 * qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1660) is
 * floor_log2(refs)*refs*size/(death-birth), sorted DESCENDING, ties broken by
 * `return *q1 - *q2` - the LOWER QUANTITY NUMBER.  qty1 and qty2 are an exact tie at 1.3333
 * and qty1 (the t0 chain) wins the tie purely because it is born two insn-indices earlier.
 * To flip the seats the arg5 quantity must read STRICTLY greater than 1.3333 while its span
 * stays 6, i.e. refs >= 5 at span 6 (floor_log2(5)*5 = 10 > 8), or the t0 quantity must read
 * strictly less, i.e. refs <= 3 at span 6 (floor_log2(3)*3 = 3, pri 0.5).
 *
 * WHAT s80 MEASURED AGAINST THAT TARGET AND WHAT IS LEFT (75 whole-function builds):
 *   - 51 loop-note wrap placements/depths on the order-exact base (tmp/grind/CD_ready/s80/h.log,
 *     gen3.py): every one is >= 6.  Excluding the t0 statements from the wrap does lower the t0
 *     refs but costs tbl_125c its $s5 seat (14-19).
 *   - 24 NESTED inner-wrap forms (n.log, gen4.py) aimed exactly at "arg5 refs 5, span 6": the
 *     inner note DOES lift reg102's refs 4 -> 5, but it also moves reg102's birth 20 -> 18, so
 *     the span goes 6 -> 8 and the priority DROPS to 1.25 (dump
 *     tmp/grind/CD_ready/s80/n_o1_7_8_i1.qty.txt).  Best 9, most 10-29.  A loop note does not
 *     advance local-alloc's insn_number (local-alloc.c:1176), so the birth move is a SCHEDULER
 *     effect of the note, not a local-alloc one - that is the thing to defeat.
 *   - The "t0 refs 3" half needs the t0 shift's two references split across a note boundary
 *     while both arg5 references stay inside it.  On the order-exact statement order the four
 *     references nest as arg5-set < t0-sll < t0-addu < arg5-use, so NO contiguous wrap can do
 *     it.  The one intermediate order that would separate them (`t0 *= 4;` between `v0 <<= 2;`
 *     and the arg5 statement) was measured this session: it sits in the FLOOR basin, not the
 *     target one (p_0_9_d1 = 2 with the floor's wrong ALU order; the refs-split wraps on it are
 *     7).  So the remaining opening is a NON-CONTIGUOUS or statement-level restructuring that
 *     puts the t0 shift outside the arg5 pseudo's live note region.
 *   - The Sony bios.c v1.86 ARGUMENT SPELLINGS are dead on this chassis (10 forms, f*.c, 11-18):
 *     spelling the two CD_intstr arguments as array subscripts of tbl_125c instead of the
 *     manual shift-and-add costs 9+ points and rebuilds the whole block-3 register web.
 */
/* CD_ready CANDIDATE - s78 (2026-09-04, rederive). FLOOR UNCHANGED AT MASKED 2 (score 2,
 * build 179, target 179, rules_dropped 0, re-measured live this session), but this body is
 * STRICTLY CLEANER than the s60-s77 floor body it replaces: it retires TWO non-ordinary
 * constructs at zero byte cost.
 *
 * !! HOW TO APPLY THIS FILE !!  It carries a DECLARATION SURFACE (the //DROPALL, //REPLALL and
 * //INS_BEFORE directive lines below) as well as a body. tmp/grind/CD_ready/s63/splice.py
 * splices the BODY ONLY and will silently produce a body that does not compile / does not
 * measure 2. Use memory/grind/CD_ready/apply_s78.py (same file as tmp/grind/CD_ready/s78/apply.py):
 *     python3 memory/grind/CD_ready/apply_s78.py memory/grind/CD_ready/candidate.c
 * It re-applies the s63 RENAME map, replays the directives against
 * tmp/grind/CD_ready/s61/system.c.bak and writes src/system.c. The previous (scalar-model,
 * splice.py-compatible) floor body is banked verbatim at
 * memory/grind/CD_ready/progress/s77-floor-body-scalar-model-2.c.
 *
 * WHAT CHANGED vs the s77 floor body, and why each change is banked:
 *   1. SONY OBJECT MODEL for the alarm block (owner directive probe 1, transplanted from
 *      CD_datasync s58): D_800F19B8/BC/C0 are ONE Sony object - BIOS.OBJ XDEF `Alarm` at
 *      .bss+0x18 == 0x800F19B8 (memory/closer/libcd-groundtruth.md:42). Declared here as
 *      `typedef struct { s32 timeout; s32 count; void *func; } CD_alarm; extern CD_alarm
 *      D_800F19B8;`. MEASURED 2/179/0 - identical to the floor - AND it makes the
 *      `void **pp` pointer-alias FAKE non-load-bearing, exactly as CD_datasync s58 found on
 *      its twin. The pp FAKE is DELETED here (printf reads D_800F19B8.func directly).
 *      On the scalar model that same deletion costs 9 points (this ledger s53-s57).
 *      FAKE unit count on the floor body: 8 -> 7.
 *   2. THE asm("D_800A147C") ALIAS RENAME IS RETIRED. The s60-s77 chassis carried
 *      `extern volatile u8 *D_800A147C_2 asm("D_800A147C");` - an alias-rename spelling on the
 *      forbidden-family catalog. Replacing it with a plain `extern volatile u8 *D_800A147C;`
 *      (the decl the symbol already has, with the volatile moved onto it) and using the symbol
 *      directly MEASURES 2/179/0. The pointed-to object is the CD-ROM index register
 *      (0x1F801800 range, census g_cd_index_reg), so this is type-level MMIO volatile
 *      (.claude/rules/mmio-volatile-type-level.md) and needs no annotation and no allowlist row.
 *
 * WHAT DID NOT CHANGE - the residual is still s76's single adjacent ALU transposition
 * (target slots 56/57: `addu $v0,$v0,$s5` before `sll $a0,$a0,2`; this body emits them the
 * other way round) with every seat already correct. s78 re-measured the whole struct chassis
 * against it: seven statement orders (2/2/2/4/4/6/7), four birthing_insn_p de-boost carriers
 * (12/12/15/2) - the de-boost family reproduces its scalar-chassis scores instruction for
 * instruction, so removing pp's pseudo from block 3 does NOT change that coupling.
 *
 * THE ONE OPEN QUESTION IS STILL F3 (the `volatile u8 *idx_1496` spelling), and s78 measured
 * both halves of it for the first time:
 *   - Removing the volatile from the pointer local costs 2 points (4/178,
 *     rejected/s78-v10-idx1496-pointer-without-volatile-4.c). The volatile IS load-bearing.
 *   - Re-spelling it honestly at the DECLARATION (`extern volatile u8 D_800A1496;` read
 *     directly) costs 25 points (27/177) - but its non-volatile CONTROL scores the SAME 27
 *     (rejected/s78-v6-...-control-27.c), so the 25 points are the ADDRESSING change
 *     (losing the idx_1494+2 base-register form), not the volatile. A CD_intr struct with a
 *     volatile member, which keeps base-register addressing, scores 36 (v8) against its
 *     plain control's 37 (v9) - same conclusion, and the struct model for D_800A1494 is dead
 *     on this function regardless of volatility.
 *   So the honest declaration-level spelling of the D_800A1496 volatile is NOT byte-reachable
 *   on any access shape measured so far; the pointer-injected spelling is the only one that
 *   holds the floor. That is a Judge question (Ruling-4 / allowlist class), not a lever.
 */
//REPLALL:extern u8 *D_800A147C; => extern volatile u8 *D_800A147C;
//DROPALL:extern volatile u8 *D_800A147C_2 asm("D_800A147C");
//DROPALL:extern s32 D_800F19B8;
//DROPALL:extern s32 D_800F19BC;
//DROPALL:extern void *D_800F19C0;
//INS_BEFORE:extern s32 D_800161B8;|typedef struct { s32 timeout; s32 count; void *func; } CD_alarm;@@extern CD_alarm D_800F19B8;
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
  D_800F19B8.timeout = sys_VSync(-1) + 0x3C0;
  tbl_125c = D_800A125C;
  idx_1494 = (u8 *)&D_800A1494;
  idx_1495 = 1 + idx_1494;
  idx_1496 = idx_1494 + 2;
  D_800F19B8.count = 0;
  D_800F19B8.func = &D_80016248;
  loop:
  v0 = sys_VSync(-1);

  if (D_800F19B8.timeout < v0)
  {
    goto do_timeout;
  }
  cnt = D_800F19B8.count;
  D_800F19B8.count = cnt + 1;
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
    t0 = idx_1494[0];
    v0 = idx_1494[1]; /* FAKE: index staged through the (dead-here) v0 var per staged-value-reused-variable (owner-sanctioned 2026-07-03) */
    v0 <<= 2;
    v0 += (s32)tbl_125c;
    t0 <<= 2;
    arg5 = *(s32 *)v0;
    debug_printf(&D_800161C8, D_800F19B8.func, D_800A11DC[D_800A11D5], *(s32 *)(t0 + (s32)tbl_125c), arg5);
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
    saved = *D_800A147C & 3;
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
    *D_800A147C = saved;
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
