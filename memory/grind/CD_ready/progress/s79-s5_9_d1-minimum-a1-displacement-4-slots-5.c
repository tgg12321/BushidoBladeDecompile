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
    s32 a5a;
    s32 a2i;
    t0 = idx_1494[0];
    v0 = idx_1494[1]; /* FAKE: index staged through the (dead-here) v0 var per staged-value-reused-variable (owner-sanctioned 2026-07-03) */
    v0 <<= 2; /* FAKE: continued staging per staged-value-reused-variable */
    a5a = v0 + (s32)tbl_125c;
    t0 *= 4;
    do { /* FAKE: do-while(0) loop-note refs weighting (do-while-zero-exception 2026-07-06) */
    arg5 = *(s32 *)a5a;
    a2i = D_800A11D5;
    t0 = (s32)((u8 *)tbl_125c + t0);
    debug_printf(&D_800161C8, D_800F19B8.func, D_800A11DC[a2i], *(s32 *)t0, arg5);
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
