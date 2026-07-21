/* func_80033550 — best honest form, sessions 1-4 (floor 4, zero cheat-asm).
 * s2 (structural, 24 variants): tail geometry closed. s3 (structural, 14
 * variants): loop-region census-invariance proven (6 spellings, identical
 * .greg), REG_EQUIV const-pointer pseudos deleted pre-RA, DImode pair
 * occupant mechanism confirmed but never byte-free. s4 (permuter, 4
 * campaigns, ~104k iters): whole-function stochastic search measured
 * near-dead — every basin (random, directed-staged, v07-flip-neighborhood,
 * c3-20-neighborhood) converges to the score-20 ptr=a1 attractor; zero
 * sub-20 finds. s5 (permuter, 2 fresh basins, ~34k iters): DImode-pair and
 * walker-pointer chassis basins ALSO converge to the same attractor —
 * permuter modality now fully dead (6 basins, ~138k cumulative iters).
 * s5 new fact: an uninit-read pseudo occupies a register byte-free
 * (rejected/permuter-uninit-dimode-read-garbage-a3-30.c) — the zero-byte
 * occupant channel exists but no valid spelling reaches it yet.
 * s6 (forensics): cc1psx output is INSTRUCTION-IDENTICAL to our fork on this
 * C (ptr in a1) — no compiler-fork divergence. Closure theorem proven at
 * source level (mips.c:3447 even-pair rule; global.c find_reg prefs empty,
 * set_preference needs a hard-reg SET = calls; final.c:1800 no-op deletion
 * needs same-reg; flow.c:1479 dead defs die pre-RA): no semantically-valid C
 * producing THIS 34-insn shape can home ptr in a3.
 * s7 (forensics): FAKE-family sweep measured — dead-store/named-local INERT
 * (jump1 deletes pre-RA, .rtl->.jump proven); duplicated-arms NOT inert:
 * NEW channel (f) = jump2 cross-jump merges identical duplicated arms whose
 * pseudos were real RA conflicts (dupU homed ptr in a3, first valid-C ever;
 * best family rotations honest 11). s6 theorem amended. Sanctioning of the
 * invented identical-arms branch is the open ruling-request; family residue
 * (a2-seated deletable temps, label-steered merges) pending that ruling.
 * The do-while(0) wrap is sanctioned per do-while-zero-exception (2026-07-06
 * owner ruling, any codegen effect, single-level, FAKE-annotated).
 * Residual 4 = arg0's pointer pseudo homed in $a1 (build) vs $a3 (target):
 * move + 3 lw base regs. See evidence.md for the RTL conflict analysis. */
void func_80033550(s32 *arg0)
{
  s32 new_var;
  s32 i;
  s32 w0;
  s32 w1;
  s32 w2;
  s32 idx;
  i = 0;
  loop:;
  if ((*(&D_800A3918 + i)) == 0) goto found;
  i++;
  if (i < 6) goto loop;
  found:;
  if (i == 6) return;
  *(&D_800A3918 + i) = 1;
  new_var = i * 12;
  /* FAKE: single-level wrap; loop notes fence sched1 so the final sll of
   * idx stays ahead of the three lw's, matching target order. */
  do { idx = new_var; w0 = arg0[0]; } while (0);
  w1 = arg0[1];
  w2 = arg0[2];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
