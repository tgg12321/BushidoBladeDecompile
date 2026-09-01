/* 2026-08-24 MIGRATION NOTE: HEAD is now INCLUDE_ASM (asm-until-matched
   wave 1, 2026-08-19); rules retired. Statements below about "HEAD",
   rules carried, or "applied to src" describe the PRE-MIGRATION tree
   (banked at retired-chassis-2026-08/body.c). Paste this body over the
   INCLUDE_ASM line before any sandbox re-measure. */
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
 * best family rotations honest 11). s6 theorem amended.
 * s8 (forensics, post-ruling): Judge FAIL 2026-07-21 00:19 — the invented
 * identical-arms branch is a cheat-by-spelling; channel (f) via manufactured
 * branches is CLOSED and the family residue dies with it. scan_hand_coded
 * LOW 0/8 measured. Every sanctioned axis measured dead -> OWNER-ESCALATION
 * filed in docs/grind/decisions.md (2026-07-21); awaiting owner ruling
 * (option (b) precedent: pin retained on main to hold the oracle match,
 * function INCOMPLETE-owner-accepted).
 * s10 (2026-08-20, forensics, post-unpark): the 2026-07-22 owner ruling is
 * SPENT - the 2026-08-19 stale-park re-audit (docs/grind/borderline.md:68)
 * unparked this function on the "F6+F7 seam" ground. Both halves measured
 * DEAD this session on the post-migration chassis (baseline re-verified 4):
 * F6 empty-if / redundant-condition = deleted by jump_optimize pass 1
 * (.rtl 35 -> .jump 24 == baseline), .greg byte-identical, INERT;
 * F6 cancellation pair = deleted by flow.c DCE when it does not overlap the
 * pointer's live range (inert), and when it DOES overlap it makes the entry
 * copy coalescable (.greg gains `72 preferences: 4`, loses the hard-4
 * conflict) so find_reg seats 72 in $a0 before the numeric scan - 33 insns,
 * distance 6-7. F7 has no pre-existing if/else arms here (tail is
 * straight-line; duplicating into the i==6 arm changes semantics). Both
 * banked seam forms re-measured chassis-invariant at 11 > 4, so the seam is
 * MOOT. See hypotheses.md [s10] and evidence.md s10.
 * The do-while(0) wrap is sanctioned per do-while-zero-exception (2026-07-06
 * owner ruling, any codegen effect, single-level, FAKE-annotated).
 * s9 (2026-08-20, ESCALATION modality, DISPOSITION): floor re-measured 4 on
 * this chassis (34/34 insns, 0 rules). Residual restated as arithmetic from
 * .greg: pseudo 72 (the pointer) has an EMPTY preference set and hard
 * conflicts {2,3,4,29}, so find_reg's numeric scan v0,v1,a0,a1,a2,a3 gives
 * $a1; the target's $a3 requires conflicts superset-of {2,3,4,5,6} = two
 * byte-free occupants in $a1 AND $a2 while the hard-4 conflict survives -
 * mutually exclusive per the s8 (A)/(B) regime measurement. Phantom-pad
 * family KILLED on prerequisite (target has NO stack frame at all). Both
 * endgame-lock AND-gates fail (scan_hand_coded LOW 0/8; SOTN-master census
 * negative for a byte-free REGISTER occupant). Standing 2026-07-27 ruling
 * applied: REFUSED / OWNER-ACCEPTED INCOMPLETE, docs/grind/decisions.md:8216.
 * s10 (2026-08-25, ESCALATION modality, owner directive 2026-08-24 = "F1
 * chain-extender family"): directive EXECUTED and KILLED in three measured
 * modes (rejected/s10-f1-chain-*.c) - a plain alias dies in cse before flow.c
 * counts it (inert); a folding detour survives combine only by REPLACING the
 * entry copy (addiu a1,a0,N where target has addu a3,a0,zero - count-neutral,
 * not byte-neutral); a partial detour splits the pointer into two global
 * allocnos at 34 insns (arg0->$a0, detour->$a1) but again pays with the
 * entry-copy slot and moves arg0 AWAY from target. Law: arg0 is a runtime
 * param and the three loads are already direct base+offset, so no detour is
 * both surviving and byte-free. s10 also banked the first ra_solver model for
 * this function (only TWO global allocnos: 74=i->$v1 pri 22500, 72=arg0->$a1
 * pri 6250, hard conflicts {2,3,4,29}, EMPTY preference sets) and the first
 * typed verdict: inverse.py --goal {"72": 7} is NEGATIVE at depth 2 AND 3 over
 * 48 atoms in 5 classes, with the preference route FORECLOSED ($a3 never
 * appears as a hard reg in the pre-RA RTL, so set_preference cannot name it).
 * Both endgame-lock gates re-measured failing (scan LOW 0/8; no SOTN
 * precedent). Standing 2026-07-27 ruling re-applied; see docs/grind/decisions.md.
 * s11 (2026-08-30, ESCALATION modality, post-ruling-10): floor re-measured
 * 4 on this chassis (34/34, 0 rules). NEW: first 1:1 position-aligned
 * disassembly correspondence banked (s11/insn_correspondence.txt) - 30 of 34
 * instructions identical, the 4 that differ (idx 0,20,21,22) differ in ONE
 * register field only ($a3 vs $a1). Instruction-budget closure: every target
 * slot is already spoken for, so an added register carrier must take a 35th
 * slot or substitute for one of the 34 -> no byte-free occupant CAN exist in
 * this shape. Preference route also closed by fidelity: the sole caller
 * (func_800290B8.s:203) sets only $a0, so a 4-ary signature (the one
 * construct that would put $a3 in the pre-RA RTL) is a false claim. Both
 * endgame-lock gates re-measured failing (scan LOW 0/8; no SOTN precedent).
 * Standing 2026-07-27 ruling re-applied: docs/grind/decisions.md:15517.
 * s12 (2026-09-01, STRUCTURAL, owner Ruling-A named probe): floor re-measured
 * 4 (34/34, 0 rules). Ruling A KILLED at both real branch sites: the loop's two
 * exits merge UPSTREAM of the i==6 test (cross_jump hoists the duplicate out of
 * the tail, pointer coalesces to $a0, 11-20), and the i==6 arm's duplicable
 * statements are all DCE-able (RA-inert, +1 sched artefact only) while the only
 * non-DCE-able ones (the stores) are semantically invalid on the not-found path.
 * dupU does not transfer: its byte-freeness needed SYMMETRIC arms, and both real
 * branches here are asymmetric. NEW measured RA arithmetic: dS4 (base + one
 * trailing store) moves the pointer $a1 -> $a2 with every other seat
 * target-exact (blocking $a1 == extending the pointer past the last load, which
 * otherwise reuses its register); dZ (diagnostic, 37 insns) reaches
 * `move a3,a0` + all three `lw ...(a3)` with FIVE block-locals live. Residual is
 * now a specified shopping list: a byte-free fifth block-local deleted after
 * local-alloc PLUS a byte-free pointer use after it. See evidence.md s12.
 * NOTE: this file is CRLF - normalise to LF after pasting into src/*.c.
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
