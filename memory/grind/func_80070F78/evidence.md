# Evidence bank — motion_ShiftControl

- [s1] Queue entry: distance=808, verdict=ASM-STRUCTURAL, rules=1, status=parked (2026-06-09 audit REJECTED auto-auth: standard GCC-compiled C). Single rule: asmfix.txt:187 replace_with_asmfile; src/text1b.c body is a stub. 808 = unwritten body. regfix.txt: zero rules.

- [s1] Structure (tmp/blitz/motion_ShiftControl_map.txt): 810 insns @0x80070F78-0x80071C1C, frame 0x70, saves s0-s7+fp+ra. Single jr $ra, void return. NO mult/div (all strength-reduced: *0x3F, *3, <<5/<<6 scaling), NO GTE, NO jump tables.

- [s1] Signature: void motion_ShiftControl(void *arg0, void *arg1). arg0 = draw-context struct (+0x18 prim/tex cursor advanced +0xC after initTexPage, +0x10 and +0x4 prim-list heads updated from func_8007352C/func_80073728 returns); arg1 = sprite work struct (+0x0 image ptr, +0x4 = img+0xC, +0x8/+0xC next links, +0x14 mode, +0x18/+0x1C screen x/y, +0x20/+0x24 scale 0x100->0x80, +0x28 flag, +0x29..0x2B RGB pulse from math_Sin).

- [s1] 3 loops, all over the same bound `D_800A35B0 + (D_800A3558 + 1)` (slot count = base count + extra when D_800A3558!=0): (1) small pre-pass 38-insn loop marking state-5/0x10 slots (sets D_800A3562[i*3]=6/7, sp18=1, timer[i]=0); (2) the 546-insn main per-slot loop (select/confirm/cancel handling); (3) the 90-insn draw loop (func_80073728 sprite per active slot). The bound expression is RELOADED each iteration (calls clobber memory) — write it inline in each loop condition.

- [s1] Loop counters are s16 (var_s3 patterns `var_s3_2 << 0x10 >> 0x10` everywhere; shift-amount computes `(s16)(i - D_800A3558) << 4`) — the pervasive sll/sra 0x10 pairs come from s16 counter locals; declaring them s32 would delete ~50 insns and never match.

- [s1] Pad-input decoding: D_800A354C is a button/trig word tested with per-slot nibble shifts: 0x2000<<(4*k) and 0x8000<<(4*k) = left/right, 0x40<<(4*k) = confirm, 0x10<<(4*k) = cancel, where k=(s16)(i - D_800A3558). Cursor per slot: D_800A3590[i] (s16 pairs, clamped to [var_a3..var_a2] = [0/1..4/5] depending on a D_8009BC7C[D_800A3561[i*3]] & 2 capability bit).

- [s1] State globals cluster (menu state machine): D_800A3560[i*3] slot state (5/0x10 = locked), D_800A3562[i*3] selected id (0xFF = none; 6/7 forced), D_800A3561[i*3] char id, D_800A3544[i] anim rate (7 or 9), D_800A3540[i] frame 0..0xC, D_800A35C4 timer array (s16, +4/+6 fields, set 0x1E), D_800A3578 mode latch (0->3->0x101), D_800A3584/D_800A3550/D_800A355C/D_800A3558/D_800A3565/D_800A3563 exit latches, D_800A3594[i] column, D_800A35BC game mode (2 = vs?, 3 = auto-confirm), D_800A3568->0x14 & 0x20000 flag, D_800A35A8 resource block (+0x74 image table, +0x7C VRAM x base, +0x60, +0x14 CLUT table indexed [id*8 + (capBit?0:4)]).

- [s1] Call pattern is the codegen-sensitive area: func_8005C650 (play SE) called 10x with VARYING arg counts — 4 args `(0,0x7F,0x7F,var_a3)` and `(2,0x7F,0x7F,var_a3)` vs 3 args `(1,0x7F,0x7F)`, `(2,0x7F,0x7F)`, `(6,0x7F,0x7F)` — an unprototyped K&R callee; preserving the per-site arg counts matters for a3 homing and for cross-jump-call-merge behavior (target may merge identical call suffixes).

- [s1] gpu_LoadImage x2 + gpu_DrawSync(0) x2 (confirm-path texture upload: src VRAM rect = D_800A35A8->0x7C + (i<<6) + id*8, data ptr = *(D_800A35A8 + id*8 + 0x14 + (capBit ? 0 : 4))) — the second instance (locked-slot re-upload at m2c lines 314-319) is nearly identical inline code, kept separate by control flow (no merge).

- [s1] Distinctive idioms for the draft: `(&D_800A3562)[-(var_s3_2 == 0) & 3]` = index (i==0 ? 3 : 0) via -(cond)&3 arithmetic; `((cond) == 0) * 4` CLUT-offset multiply of a comparison; RGB pulse `(math_Sin(((timer & 0x1F) << rate[i]) + i*0x1FF) * 0x3F >> 12) - 0x40` stored to 3 consecutive s8 fields (0x2B,0x2A,0x29 in that order); scale reset 0x100/0x100 inside loop vs 0x80/0x80 footer.

- [s1] Tail block (m2c 401-411): exit-condition check — if (D_800A3562 != 0xFF && timer[0]==0) { if (mode==2) D_800A3558=1; if (((D_800A3565!=0xFF && timer[1]==0) || (D_800A35B0+D_800A3558)==0) && sp18 != 1) { D_800A3578=0x101; D_800A3584=2; D_800A3550=1; D_800A355C=0; } } — sp18 is a genuine stack local (flag set 1 in pre-pass, 2 in locked-path).

- [s1] m2c decompiles CLEAN standalone (no jtbl needed): tmp/blitz/motion_ShiftControl_m2c.c (412 lines, --valid-syntax). Control flow fully structured; block_20/21/23 and block_94/95 are short-circuit condition joins (the mode==2 && !0x20000 && i==1 special-case check appears 3x — likely a repeated inline condition, not a helper).

- [s1] Function family: sibling UI functions func_8007352C / func_80073728 (sprite-emit helpers, both called here) and func_8005C650 (SE play) — check their completion status when drafting; the misapplied name is from Kengo matching but no kengo annotation exists on this function's stub.

- [s1] Ledger existed as empty skeleton (session_count=0); no WIP checkpoint.

## Manual session 2026-09-29 (s2, laneA) — from scratch, 808 -> 6 (record model, mini TU)

Tooling (memory/grind/func_80070F78/tools/, copies of tmp/func_80070F78/): sc.py (sandbox clone,
`--mini --pre=<prefix>`), mkpre.py (text1b.c prefix up to the INCLUDE_ASM line with bodies
stripped; sc_reps.py retypes the `func_80070F78` prototype + caller cast to DescF97C *), gen.py
(variant generator), dump.py (RTL dumps on the mini prefix), scr.sh (scores against
tmp/func_80070F78/mini/pre_rec.c = mini prefix with `u8 D_800A3560[]` replaced by a 3-byte
record array `{u8 unk0, unk1, unk2}` — the record model, NOT on main).

Measured path (mini TU, sandbox-equivalent score):
- First draft (m2c-guided, u8 array model `D_800A3560[i * 3 + k]`, per-byte scalars for constant
  offsets): 242/827. Same body in the record model: 116. Record-model bounds as siblings
  (`1 + D_800A35B0 + D_800A3558`, loop 2 `(port_ofs = D_800A3558)`): included.
- Loop 1: the two arms each carry their own `flag = 1; timer = 0;` (cross-jump re-merges them):
  cse sees the top block's i*2 in both arms (target's shared `a0`). 118 -> 75 with the timer
  if/else below and the tail order fixes.
- else-arm order x, y, ot_idx (as func_80070188).
- Loop 3 prologue: `s->header = sheets[0];` BEFORE the scale/has_color/ot_idx stores: any store
  through `s` invalidates cse's memory table (cse.c note_mem_written: varying address -> nonscalar;
  QImode -> all), so `cells = s->header + 0x24` reloads as the target does (70 -> 63).
- vram: `vram = *(u8 **)(D_800A35A8 + 0x7C); vram += i << 6;` (Ruling 4 compound split) at both
  sites: the load lands in vram's own register (target `lw s4,0x7C(v0)`); one-statement form puts
  it in a temp (expr.c binop subtarget is cleared inside loops, preserve_subexpressions_p). 63 -> 51,
  insn count 810/810.
- Draw tail order: colors, x, y, pad0C, ot_idx (permutation sweep of 5 items, 120 bodies: only
  CXYPO = 43).
- Image-table load `LoadImage(vram + id * 8, <slot>)`: the target computes id*8, +0x14, + base,
  + k*4, then lw 0 — every address-context spelling (INDIRECT_REF, ARRAY_REF, 2-D cast, struct view)
  expands under EXPAND_SUM, where expr.c both_summands floats the constant outward (lw 20(...)).
  Only an ASSIGNMENT (modifier 0 -> binop) keeps (id8 + 20) as its own insn: a pointer local
  `tim = (s32 *)(D_800A35A8 + 0x14 + id * 8 + k * 4); LoadImage(vram + id * 8, *tim);` (43 -> 21).
- Selector variables: ==3 site `k` (if/else -> jump.c store-flag, sltiu), confirm site `sel`
  (1 / special / 0), locked site `k2`: the target has three different registers (a1 local, a1 global,
  v1 local), so one shared selector cannot be it (l_* sweep).
- Locked site: `id` must be a single-set value there (inline `D_800A3560[i].unk2` twice, cse'd):
  sched1 adjust_priority birthing boost (reg_n_sets == 1) puts the unk2 load after the store-flag,
  as the target (21 -> 6).
- Remaining 6: (a) GPREL-name artifact `%gp_rel(D_800A35C8+2)` vs `D_800A35CA` (operand-only);
  (b) locked-arm other-player timer select: target `lw v0,D_800A35C4; bnez s0; addiu v1,v0,4;
  addiu v1,v0,6; lh v0,0(v1)` (base v0, pointer v1, beq delay slot filled from the target);
  ours base v1 / pointer v0. Forms measured: pointer local if/else both orders, ?: forms,
  `&[i==0?3:2]`, `+= ?:` all 27-30 (extra insns); `+2; if (i==0) +3` / `timer++` / `&[2]`/`&[3]`
  / block-local = 6.

Open construct questions for landing (not yet settled):
- Record model needs the 0x800A3560 record merge across all consumers + func_8006E534's word
  store (union word view = Q33, being encoded 2026-09-29) — or a per-byte fallback with per-site
  FAKE record-offset locals (70188 style), unmeasured on this body yet.
- `tim` pointer locals (three sites) and the selector variables `k`/`k2` separate from `sel`.

### s2 continued (2026-09-29) — per-byte model closes; record/union model does not land
- Timer select (locked arm, the other player's countdown): `((s16 *)D_800A35C4 + 2)[i == 0 ? 1 : 0]`
  (also `[i == 0]` / `[!i]`) = the target's single base load + `addiu +4` / `+6` select; every
  pointer-variable spelling (if/else, ?:, `+= ?:`, `&[..]`, block-local) seats base/pointer in v1/v0
  (global.c allocno_compare tie at pri 30000: the variable's pseudo 86 < the load's 707, ALLOCDBG
  `ord=6 pseudo=86` / `ord=8 pseudo=707`, dumps via tools/dump.py + BB2_ALLOC_DEBUG). Record model
  6 -> 1 (record_model_1.c; the 1 is the GPREL-name artifact `%gp_rel(D_800A35C8+2)` vs
  `D_800A35CA`, same bytes).
- Q33 union merge measured (mini prefix with `union { Unk800A3560Record rec[2]; s32 word; }`):
  67/822. Constant-offset member accesses (`.rec[0].unk2`, `.rec[1].unk0`, `.rec[1].unk2`,
  `.rec[0].unk1`) go through change_address -> explow.c memory_address, which force_regs every
  constant address at -G0; cse then shares one base (`lui s8; addiu s8,s8,5`, `sb zero,0(s8)` /
  `sb zero,-3(s8)`) where the target stores gp-direct to D_800A3565 / D_800A3562. Only a scalar
  VAR_DECL keeps (mem (symbol_ref)). So the record/union declaration cannot reproduce this function's
  constant-offset accesses without per-byte scalar handles beside it (prong (c) forbids both), and the
  landing stays on main's per-byte model (u8 D_800A3560[] + D_800A3561..D_800A3565 scalars).
- Per-byte model: inline `D_800A3560[i * 3 + k]` everywhere = 149/818. Per-site offset locals
  (70188 mechanism: an offset already in a pseudo keeps (plus off sym+k) legitimate in
  explow.c memory_address; inline, force_operand copies sym+k into a pseudo that cse shares / loop.c
  hoists). All 13 sites as locals: 13; + `other = i == 0 ? 3 : 0` for the other player's record: 1.
  Single-site ablation (inline one local, others kept; tools/ablate.py): needed alone o1 (loop 1) 52,
  o4 (sel test + else-arm cancel store) 10, o5 (==3 site) 23, o11 (locked LoadImage site) 26; o2 / o3 /
  o6-o9 / o12-o14 each 1. Keeping only o1/o4/o5/o11: 14 (the three `[i * 3]` offset-0 sites then share
  one hoisted `lui t1; addiu t1` symbol pseudo); adding back any ONE of o2 (loop-2 top) / o9 / o12 = 1.
  Chosen: o2 (loop-2 top). `other` inline (`[(i == 0 ? 3 : 0) + 2]`) 13 (fold distributes the PLUS over
  the COND_EXPR: two loads); in-condition assignment 1; block-scoped `s32 other = ...` 1.
- tim pointer locals (ablation on the block form): inline at ==3 9, confirm 10, locked 15.
  Selector per block (`sel` block-local at each LoadImage site) and `id` block-local per site: 1
  (a_idb). vram split into two variables: 13 (b_f1/b_f2); one-statement vram: 24.
- candidate.c (2026-09-29) = this body: mini TU 1, full TU (tools/sc.py) 1 (artifact only);
  func_80070C70 0, func_80070188 0, func_800720FC 0, func_80071C4C 0 with the landing edits
  (tools/sc_reps.py: prototype `DescF97C *`, func_80070C70's private PrimC70 typedef replaced by
  DescF97C (identical layout; field renames only) so the call passes `&prim` without a cast).
- PROOF (2026-09-29, laneA under the landing lock): tools/land.py splice + `lock.ps1 rebuild`:
  build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches true. Reverted and rebuilt green.
- Ruling 11 per-value splits on candidate.c (r11/): vram 13, sheets (0x60 value separate) 3,
  cells all 10 / else-arm only 4 / tail only 7 / pre-loop-3 only 10, everything split 24.
  Mechanism sketch (dumps to bank): the 0x60 sheets value and the else-arm/tail cells values sit in
  callee-saved fp / s0 only because they share the pseudo that crosses calls (loop 2/3); split, each is
  a one-block pseudo that local-alloc gives a call-clobbered register.
