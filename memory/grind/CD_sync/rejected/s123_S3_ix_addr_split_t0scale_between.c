/* [s122] BODY UNCHANGED; floor still 2/160.  The s118-s121 NAMING of block 3's
 * contended local-alloc quantities was WRONG and is corrected here (post-sched1
 * RTL, tmp/grind/CD_sync/s122/P5_v2_control/gccdump.lreg:446-505):
 *   qty1 = reg113 = the t0 SCALE result (`sll reg113 = reg107 << 2`), live for
 *          three insns only (127 -> 132);
 *   qty2 = reg106 = the arg5 loaded value (123 -> the `sw ...,16(sp)` at 149);
 *   the t0 ADDRESS is reg107, the /v pseudo of the C variable `t0`, and it is
 *          NOT one of the four quantities local-alloc ranks in this block.
 * Consequence: every "live_extend qty 1" vector means MOVE THE INSNS, not
 * restructure the C variable -- and four structurally distinct spellings (the
 * `t0` variable reused for the ix index / for the arg5 value / a dedicated
 * `s32 *ap` pointer local / the control) produce a BYTE-IDENTICAL quantity
 * table on the V2 chassis.  Also: perturb.py --pass 1 --goal-before 132:149
 * returns 25 reaching vectors and ZERO luid atoms, so source-statement order
 * provably does not contain the span fix on that chassis.  See hypotheses.md
 * H122-1..H122-3.
 */
/* [s121] BODY UNCHANGED; floor still 2/160 (re-measured live this session,
 * bi 160 rd 0).  What changed is the DIAGNOSIS, and it moves the frontier off
 * the scheduler entirely.
 *
 * 1. This body's residual is ONE adjacent transposition in block 3, identical
 *    in sched1 and sched2: ours emits uid 120 (the t0 chain's `sll`, from
 *    `t0 *= 4;`) then uid 130 (the ix chain's reload-materialised `addu`, from
 *    the folded `*(s32 *)(ix + (s32)tbl_125c)`); the target emits 130 then 120.
 *    Both are INSN_PRIORITY 2 and ready together, so rank_for_schedule decides
 *    on INSN_LUID descending -- i.e. on SOURCE STATEMENT ORDER, nothing else.
 *    perturb.py finds 36 pass-2 / 31 pass-1 single luid atoms that reach the
 *    goal; all of them are plain statement moves.
 *
 * 2. AND ONE OF THEM IS ALREADY BANKED.  progress/s121-V2-order-exact-RA-only-6.c
 *    (= s120's V2_ixfirst_folded, which s120 recorded as a bare "6/160") is
 *    ORDER-EXACT in BOTH scheduler passes for block 3.  ra_solver's
 *    inverse_compose classify calls its FIRST DIVERGENCE `RA -- same
 *    instructions, different registers`, six pairs, the t0 address and the arg5
 *    value exchanged between $a0 and $v1.
 *
 * 3. So the s115-s120 "order-vs-seat is one binary variable" equation is RETIRED:
 *    a form exists with the order and not the seats.  The live question is a
 *    local-alloc tie -- blk=3 qty1 reg113 (t0 addr) birth 18 death 24 refs 2 and
 *    qty2 reg106 (arg5) birth 20 death 26 refs 2, equal pri, broken by qty
 *    number.  inverse.py local --swap 1,2 says REACHABLE at 1 atom, 21 vectors.
 *    Rank #1 (refs_down on the t0 address) is measured dead (folding the add
 *    into printf's 4th argument = 14/160, six spellings).  The live vectors are
 *    live_extend on qty 1 and refs_up on qty 2.
 *
 * 4. Do NOT use `goalmap.py --target asm/funcs/CD_sync.s`: asm_body() skips
 *    every `/*`-prefixed line, so the target parses as one instruction and every
 *    block falsely reports "GOAL == OURS".  Object mode only.
 */
/* CD_sync candidate - s118 (2026-09-04).  Honest floor 2/160, build_insns 160,
 * rules_dropped 0, measured live this session on HEAD's src/system.c.
 *
 * THIS BODY REPLACES the s117 scalar/pp body that used to sit here.  It is the
 * pp-FREE CD_alarm-struct chassis (s117 hypothesis H117-1, CONFIRMED): the
 * per-word D_800F19BC / D_800F19C0 externs are dropped and D_800F19B8 is
 * declared as `typedef struct { s32 timeout; s32 count; char *func; } CD_alarm;`
 * so the printf argument is a member MEM.  Same 2/160 as the scalar body, but
 *   - ONE FAKE construct instead of two (the `void **pp` pointer alias is gone),
 *   - no declaration pun for the dispatch auto-scan to flag.
 * The five R_MIPS_LO16 addend differences the struct model produces are all
 * masked by engine/score.py and were verified addend-only against objdump
 * records in s117, so 2 is 2.
 *
 * The leading //REPL: / //DROP: lines are the declaration-surface directives
 * consumed by tmp/grind/CD_sync/s117/apply.py (region-scoped so CD_datasync's
 * duplicate declaration block is never touched).  They are also valid C
 * comments, so this file reads as the body plus a decl-surface manifest.
 *
 * RESIDUAL (unchanged since s115): a single transposition at target indices
 * 54/55 - target `addu $v0,$v0,$s3 ; sll $a0,$a0,2`, ours the other way round.
 * All 158 other instructions match including every register.  See
 * hypotheses.md s117/s118 for the local-alloc quantity table that governs it.
 *
 * [s119] The floor form is still this one (2/160).  But the STRUCTURALLY
 * closest form in the ledger is now memory/grind/CD_sync/progress/
 * s119-T1-status-borrow-4.c (4/160): it is order-exact for all 160
 * instructions AND seats printf's 5th argument in $v1 (the target seat) with
 * refs=2 and no loop-note wrap, by carrying the do_timeout t0 address in the
 * existing function-wide local `status' so that pseudo is exiled from
 * local-alloc.  Its entire residual is ONE register: the t0 chain sits in $s0
 * where the target has $a0 (indices 49/55/59/65).  See hypotheses.md s119.
 *
 * [s120] Body unchanged; floor still 2/160.  The residual is now stated as a
 * closed equation (evidence.md s120): local-alloc.c:1660 qty_compare_1 gives
 * pri = floor_log2(refs)*refs*size/(death-birth)*10000, and block 3's two
 * contending quantities are the t0 address chain and the arg5 value, both at
 * refs 2 / size 4.  THIS body uses the FOLDED ix add
 * (`arg5 = *(s32 *)(ix + (s32)tbl_125c)`), so the add only materialises during
 * reload: t0's sll takes sched1 slot 8, t0 span 8 (pri 10000) vs arg5 span 6
 * (13333), arg5 is allocated first and takes $v1, t0 takes $a0 - the TARGET
 * seats - at the cost of the 54/55 transposition.  Writing the add as its own
 * statement (`ix += (s32)tbl_125c;`) makes it a sched1 insn, which fixes the
 * order for all 160 instructions but pushes t0's sll to slot 9, ties both
 * spans at 6, and swaps $a0/$v1 (6/160; banked as progress/
 * s120-V1-order-exact-seatswap.c).  Fifteen source orders were measured and
 * the two outcomes are exhaustive.  The closing predicate is now a sched1
 * ORDER predicate - make sched1 emit `sw arg5,16(sp)` before `lw a3,0(t0)`,
 * which is the relative order the TARGET's own final output has - and not the
 * refs>=3 chain-extender FAKE that s119's frontier called for.
 */
//REPL:extern s32 D_800F19B8; => typedef struct { s32 timeout; s32 count; char *func; } CD_alarm; extern CD_alarm D_800F19B8;
//DROP:extern s32 D_800F19BC;
//DROP:extern void *D_800F19C0;
//REPL:extern void D_80016240; => extern char D_80016240[];
s32 CD_sync(s32 a0, u8 *a1)
{
  int new_var;
  s32 v0;
  s32 cnt;
  u8 *idx_1494;
  u8 *idx_1495;
  s32 *tbl_125c;
  u8 saved;
  s32 status;
  u8 *src;
  u8 *dst;
  s32 i;
  u8 b;
  s32 temp;
  D_800F19B8.timeout = VSync(-1) + 0x3C0;
  tbl_125c = D_800A125C;
  idx_1494 = &D_800A1494;
  idx_1495 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1); /* FAKE: combine-foldable chain-extender (link-constant delta; folds to &D_800A1494 + 1 with ZERO emitted bytes, build_insns 160 == target), mechanism: flow.c records the extra reg_n_refs before combine.c folds the SYMBOL_REF difference (expr.c::expand_expr emits the subsi3/addsi3 pair, unfoldable in cse.c per s97); family: .claude/rules/dead-store-fake-exception.md:32-46 (owner ruling 2026-07-01); lever-exhaustion: memory/grind/CD_sync/hypotheses.md s96-s105 (~40 decompositions, 122 rejected forms); load-bearing: s105 measured all three honest respellings (idx_1494+1, &idx_1494[1], &D_800A1494+1) at masked=15 vs 2 */
  D_800F19B8.count = 0;
  D_800F19B8.func = D_80016240;
  loop:
  v0 = VSync(-1);

  if (D_800F19B8.timeout < v0)
  {
    goto do_timeout;
  }
  cnt = D_800F19B8.count;
  D_800F19B8.count = cnt - -1;
  if (!(0x3C0000 < cnt))
  {
    goto success;
  }
  do_timeout:
  puts(&D_800161B8);

  {
    s32 arg5;
    s32 t0;
    s32 ix;
    t0 = idx_1494[0];
    ix = idx_1494[1];
    ix <<= 2;
    ix = (s32)((u8 *)tbl_125c + ix);
    t0 *= 4;
    arg5 = *(s32 *)ix;
    t0 = (s32)((u8 *)tbl_125c + t0);
    printf(&D_800161C8, D_800F19B8.func, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
  }
  CD_flush();
  v0 = -1;
  goto check;
  success:
  v0 = 0;

  check:
  if (v0 != 0)
  {
    return -1;
  }

  new_var = 0xFF;
  if (CheckCallback() != 0)
  {
    saved = (*D_800A147C) & 3;
    poll:
    status = getintr();

    if (status != 0)
    {
      if (status & 4)
      {
        if (D_800A11B8 != 0)
        {
          ((void (*)(u8, void *)) D_800A11B8)(*idx_1495, &D_800F19A8);
        }
      }
      if (status & 2)
      {
        if (D_800A11B4 != 0)
        {
          ((void (*)(u8, void *)) D_800A11B4)(*idx_1494, &D_800F19A0);
        }
      }
      goto poll;
    }
    *D_800A147C = saved;
  }
  temp = (*idx_1494) & new_var;
  if (((temp == 2) || (temp == 5)) != 0)
  {
    *idx_1494 = 2;
    dst = a1;
    src = (u8 *) (&D_800F19A0);
    i = 7;
    if (a1 != 0)
    {
      do
      {
        b = *src;
        src++;
        i--;
        *dst = b;
        dst++;
      }
      while (i != (-1));
    }
    return temp;
  }
  if (a0 != 0)
  {
    return 0;
  }
  goto loop;
}
